/******************************************************************************
 * This file is part of CCTools.                                              *
 *                                                                            *
 * CCTools is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * CCTools is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with CCTools.  If not, see <http://www.gnu.org/licenses/>.           *
 ******************************************************************************/

#include "CCEdit.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QToolBox>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QStatusBar>
#include <QCloseEvent>
#include <QActionGroup>
#include <QClipboard>
#include <QSettings>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QDir>
#include <QStandardPaths>
#include <QMimeData>
#include <QElapsedTimer>

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "LevelsetProps.h"
#include "Organizer.h"
#include "TileWidgets.h"
#include "LayerWidget.h"
#include "LevelProperties.h"
#include "AdvancedMechanics.h"
#include "TestSetup.h"
#include "ErrorCheck.h"
#include "TileInspector.h"
#include "libcc1/IniFile.h"
#include "libcc1/ChipsHax.h"
#include "libcc1/GameLogic.h"
#include "CommonWidgets/CCTools.h"
#include "CommonWidgets/EditorTabWidget.h"
#include "CommonWidgets/LLTextEdit.h"

static const QString s_appTitle = QStringLiteral("CCEdit " CCTOOLS_VERSION);
static const QString s_clipboardFormat = QStringLiteral("CHIPEDIT MAPSECT");

enum TileListId {
    ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
    ListMisc, ListSpecial, NUM_TILE_LISTS
};

Q_DECLARE_METATYPE(CCETileset*)

CCEditMain::CCEditMain(QWidget* parent)
    : QMainWindow(parent), m_undoCommand(), m_currentTileset(),
      m_savedDrawMode(ActionDrawPencil), m_currentDrawMode(EditorWidget::DrawPencil),
      m_levelset(), m_dirtyFlag(), m_useDac(), m_subProc()
{
    setWindowTitle(s_appTitle);

    // Actions
    m_actions[ActionNew] = new QAction(ICON("document-new"), tr("&New Levelset..."), this);
    m_actions[ActionNew]->setStatusTip(tr("Create new Levelset"));
    m_actions[ActionNew]->setShortcut(Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(ICON("document-open"), tr("&Open Levelset..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a levelset from disk"));
    m_actions[ActionOpen]->setShortcut(Qt::CTRL | Qt::Key_O);
    m_actions[ActionSave] = new QAction(ICON("document-save"), tr("&Save"), this);
    m_actions[ActionSave]->setStatusTip(tr("Save the current levelset to the same file"));
    m_actions[ActionSave]->setShortcut(Qt::CTRL | Qt::Key_S);
    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs] = new QAction(ICON("document-save-as"), tr("Save &As..."), this);
    m_actions[ActionSaveAs]->setStatusTip(tr("Save the current levelset to a new file or location"));
    m_actions[ActionSaveAs]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionCloseTab] = new QAction(tr("Close &Tab"), this);
    m_actions[ActionCloseTab]->setStatusTip(tr("Close the currently open editor tab"));
    m_actions[ActionCloseTab]->setShortcut(Qt::CTRL | Qt::Key_W);
    m_actions[ActionCloseTab]->setEnabled(false);
    m_actions[ActionCloseLevelset] = new QAction(tr("&Close Levelset"), this);
    m_actions[ActionCloseLevelset]->setStatusTip(tr("Close the currently open levelset"));
    m_actions[ActionCloseLevelset]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_W);
    m_actions[ActionCloseLevelset]->setEnabled(false);
    m_actions[ActionProperties] = new QAction(ICON("document-properties"), tr("Levelset &Properties"), this);
    m_actions[ActionProperties]->setStatusTip(tr("Change levelset and .DAC file properties"));
    m_actions[ActionProperties]->setEnabled(false);
    m_actions[ActionGenReport] = new QAction(tr("Generate &Report..."), this);
    m_actions[ActionGenReport]->setStatusTip(tr("Generate an HTML report of the current levelset"));
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionExit] = new QAction(ICON("application-exit"), tr("E&xit"), this);
    m_actions[ActionExit]->setStatusTip(tr("Close CCEdit"));

    m_actions[ActionUndo] = new QAction(ICON("edit-undo"), tr("&Undo"), this);
    m_actions[ActionUndo]->setStatusTip(tr("Undo the last edit"));
    m_actions[ActionUndo]->setShortcut(Qt::CTRL | Qt::Key_Z);
    m_actions[ActionUndo]->setEnabled(false);
    m_actions[ActionRedo] = new QAction(ICON("edit-redo"), tr("&Redo"), this);
    m_actions[ActionRedo]->setStatusTip(tr("Redo the last edit"));
    m_actions[ActionRedo]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Z);
    m_actions[ActionRedo]->setEnabled(false);
    m_actions[ActionSelect] = new QAction(ICON("edit-select"), tr("&Select"), this);
    m_actions[ActionSelect]->setStatusTip(tr("Enter selection mode"));
    m_actions[ActionSelect]->setShortcut(Qt::CTRL | Qt::Key_A);
    m_actions[ActionSelect]->setCheckable(true);
    m_actions[ActionCut] = new QAction(ICON("edit-cut"), tr("Cu&t"), this);
    m_actions[ActionCut]->setStatusTip(tr("Put the selection in the clipboard and clear it from the editor"));
    m_actions[ActionCut]->setShortcut(Qt::CTRL | Qt::Key_X);
    m_actions[ActionCut]->setEnabled(false);
    m_actions[ActionCopy] = new QAction(ICON("edit-copy"), tr("&Copy"), this);
    m_actions[ActionCopy]->setStatusTip(tr("Copy the current selection to the clipboard"));
    m_actions[ActionCopy]->setShortcut(Qt::CTRL | Qt::Key_C);
    m_actions[ActionCopy]->setEnabled(false);
    m_actions[ActionPaste] = new QAction(ICON("edit-paste"), tr("&Paste"), this);
    m_actions[ActionPaste]->setStatusTip(tr("Paste the clipboard contents into the levelset at the selection position"));
    m_actions[ActionPaste]->setShortcut(Qt::CTRL | Qt::Key_V);
    m_actions[ActionPaste]->setEnabled(false);
    m_actions[ActionClear] = new QAction(ICON("edit-delete"), tr("C&lear"), this);
    m_actions[ActionClear]->setStatusTip(tr("Clear all tiles and mechanics from the selected region"));
    m_actions[ActionClear]->setShortcut(Qt::Key_Delete);
    m_actions[ActionClear]->setEnabled(false);

    m_actions[ActionDrawPencil] = new QAction(ICON("draw-freehand"), tr("&Pencil"), this);
    m_actions[ActionDrawPencil]->setStatusTip(tr("Draw tiles with the pencil tool"));
    m_actions[ActionDrawPencil]->setShortcut(Qt::CTRL | Qt::Key_P);
    m_actions[ActionDrawPencil]->setCheckable(true);
    m_actions[ActionDrawLine] = new QAction(ICON("draw-line"), tr("&Line"), this);
    m_actions[ActionDrawLine]->setStatusTip(tr("Draw tiles with the line tool"));
    m_actions[ActionDrawLine]->setShortcut(Qt::CTRL | Qt::Key_L);
    m_actions[ActionDrawLine]->setCheckable(true);
    m_actions[ActionDrawRect] = new QAction(ICON("draw-rect"), tr("&Rectangle"), this);
    m_actions[ActionDrawRect]->setStatusTip(tr("Draw tiles with the rectangle tool"));
    m_actions[ActionDrawRect]->setShortcut(Qt::CTRL | Qt::Key_R);
    m_actions[ActionDrawRect]->setCheckable(true);
    m_actions[ActionDrawFill] = new QAction(ICON("draw-box"), tr("&Box"), this);
    m_actions[ActionDrawFill]->setStatusTip(tr("Draw tiles with the box fill tool"));
    m_actions[ActionDrawFill]->setShortcut(Qt::CTRL | Qt::Key_B);
    m_actions[ActionDrawFill]->setCheckable(true);
    m_actions[ActionDrawFlood] = new QAction(ICON("draw-fill"), tr("&Flood Fill"), this);
    m_actions[ActionDrawFlood]->setStatusTip(tr("Draw tiles with the flood fill tool"));
    m_actions[ActionDrawFlood]->setShortcut(Qt::CTRL | Qt::Key_F);
    m_actions[ActionDrawFlood]->setCheckable(true);
    m_actions[ActionPathMaker] = new QAction(ICON("draw-path"), tr("Path &Maker"), this);
    m_actions[ActionPathMaker]->setStatusTip(tr("Draw a directional path of tiles"));
    m_actions[ActionPathMaker]->setShortcut(Qt::CTRL | Qt::Key_M);
    m_actions[ActionPathMaker]->setCheckable(true);
    m_actions[ActionConnect] = new QAction(ICON("cctools-rbutton"), tr("Button &Connector"), this);
    m_actions[ActionConnect]->setStatusTip(tr("Connect buttons to traps and cloning machines"));
    m_actions[ActionConnect]->setShortcut(Qt::CTRL | Qt::Key_T);
    m_actions[ActionConnect]->setCheckable(true);
    m_actions[ActionAdvancedMech] = new QAction(ICON("cctools-teeth"), tr("&Advanced Mechanics"), this);
    m_actions[ActionAdvancedMech]->setStatusTip(tr("Manually manipulate gameplay mechanics for the current level"));
    m_actions[ActionAdvancedMech]->setShortcut(Qt::CTRL | Qt::Key_K);
    m_actions[ActionAdvancedMech]->setEnabled(false);
    m_actions[ActionInspectTiles] = new QAction(ICON("draw-inspect"), tr("&Inspect Tiles"), this);
    m_actions[ActionInspectTiles]->setStatusTip(tr("Inspect tiles and make advanced modifications"));
    m_actions[ActionInspectTiles]->setShortcut(Qt::CTRL | Qt::Key_I);
    m_actions[ActionInspectTiles]->setCheckable(true);
    m_actions[ActionToggleWalls] = new QAction(ICON("cctools-gbutton"), tr("&Toggle Walls"), this);
    m_actions[ActionToggleWalls]->setStatusTip(tr("Toggle all toggle floors/walls in the current level"));
    m_actions[ActionToggleWalls]->setShortcut(Qt::CTRL | Qt::Key_G);
    m_actions[ActionToggleWalls]->setEnabled(false);
    m_actions[ActionCheckErrors] = new QAction(tr("Check for &Errors..."), this);
    m_actions[ActionCheckErrors]->setStatusTip(tr("Check for errors in the current levelset or a specific level"));
    m_actions[ActionCheckErrors]->setShortcut(Qt::CTRL | Qt::Key_E);
    m_actions[ActionCheckErrors]->setEnabled(false);
    m_drawModeGroup = new QActionGroup(this);
    m_drawModeGroup->addAction(m_actions[ActionDrawPencil]);
    m_drawModeGroup->addAction(m_actions[ActionDrawLine]);
    m_drawModeGroup->addAction(m_actions[ActionDrawRect]);
    m_drawModeGroup->addAction(m_actions[ActionDrawFill]);
    m_drawModeGroup->addAction(m_actions[ActionDrawFlood]);
    m_drawModeGroup->addAction(m_actions[ActionPathMaker]);
    m_actions[ActionDrawPencil]->setChecked(true);
    m_modalToolGroup = new QActionGroup(this);
    m_modalToolGroup->setExclusive(false);
    m_modalToolGroup->addAction(m_actions[ActionSelect]);
    m_modalToolGroup->addAction(m_actions[ActionConnect]);
    m_modalToolGroup->addAction(m_actions[ActionInspectTiles]);
    m_drawModeGroup->setEnabled(false);
    m_modalToolGroup->setEnabled(false);

    m_actions[ActionViewButtons] = new QAction(tr("Show &Button Connections"), this);
    m_actions[ActionViewButtons]->setStatusTip(tr("Draw lines between connected buttons/traps/cloning machines in editor"));
    m_actions[ActionViewButtons]->setCheckable(true);
    m_actions[ActionViewMovers] = new QAction(tr("Show &Monster Order"), this);
    m_actions[ActionViewMovers]->setStatusTip(tr("Display Monster Order in editor"));
    m_actions[ActionViewMovers]->setCheckable(true);
    m_actions[ActionViewCloners] = new QAction(tr("Show &Clone Buttons/Machines Numbers"), this);
    m_actions[ActionViewCloners]->setStatusTip(tr("Display clone button and related clone machine numbers in editor"));
    m_actions[ActionViewCloners]->setCheckable(true);
    m_actions[ActionViewTraps] = new QAction(tr("Show &Trap Buttons/Traps Numbers"), this);
    m_actions[ActionViewTraps]->setStatusTip(tr("Display trap button and related trap numbers in editor"));
    m_actions[ActionViewTraps]->setCheckable(true);
    m_actions[ActionViewActivePlayer] = new QAction(tr("Show &Player Starting Position"), this);
    m_actions[ActionViewActivePlayer]->setStatusTip(tr("Highlight the Player's start position"));
    m_actions[ActionViewActivePlayer]->setCheckable(true);
    m_actions[ActionViewViewport] = new QAction(tr("Show Game &Viewport"), this);
    m_actions[ActionViewViewport]->setStatusTip(tr("Show a viewport bounding box around the cursor"));
    m_actions[ActionViewViewport]->setCheckable(true);
    m_actions[ActionViewMonsterPaths] = new QAction(tr("Show Mo&nster Paths"), this);
    m_actions[ActionViewMonsterPaths]->setStatusTip(tr("Trace Projected Monster Paths (May be inaccurate)"));
    m_actions[ActionViewMonsterPaths]->setCheckable(true);
    m_actions[ActionViewErrors] = new QAction(tr("Show Tile Combo &Errors"), this);
    m_actions[ActionViewErrors]->setStatusTip(tr("Visually mark invalid tile combinations in the editor"));
    m_actions[ActionViewErrors]->setCheckable(true);
    m_actions[ActionViewButtonsOnMouse] = new QAction(tr("Display Button and Teleport Connections on &Hover"), this);
    m_actions[ActionViewButtonsOnMouse]->setStatusTip(tr("Visually show button connection when moving the mouse over a button or a trap/cloner"));
    m_actions[ActionViewButtonsOnMouse]->setCheckable(true);

    m_actions[ActionViewDRCloners] = new QAction(tr("Mark Known MSCC &Data Resetting Clone Buttons"), this);
    m_actions[ActionViewDRCloners]->setStatusTip(tr("Mark known MSCC data resetting clone buttons in editor"));
    m_actions[ActionViewDRCloners]->setCheckable(true);
    m_actions[ActionViewMultiTanks] = new QAction(tr("Mar&k Multiple Tank Locations (Possible Multiple Tank Glitch)"), this);
    m_actions[ActionViewMultiTanks]->setStatusTip(tr("Mark tiles where multiple tanks are inserted in the monster list (to possibly trigger the Multiple Tank Glitch)"));
    m_actions[ActionViewMultiTanks]->setCheckable(true);

    m_actions[ActionZoom200] = new QAction(tr("200%"), this);
    m_actions[ActionZoom200]->setStatusTip(tr("Zoom to 200%"));
    m_actions[ActionZoom200]->setShortcut(Qt::CTRL | Qt::Key_2);
    m_actions[ActionZoom200]->setCheckable(true);
    m_actions[ActionZoom150] = new QAction(tr("150%"), this);
    m_actions[ActionZoom150]->setStatusTip(tr("Zoom to 150%"));
    m_actions[ActionZoom150]->setShortcut(Qt::CTRL | Qt::Key_3);
    m_actions[ActionZoom150]->setCheckable(true);
    m_actions[ActionZoom100] = new QAction(tr("&100%"), this);
    m_actions[ActionZoom100]->setStatusTip(tr("Zoom to 100%"));
    m_actions[ActionZoom100]->setShortcut(Qt::CTRL | Qt::Key_1);
    m_actions[ActionZoom100]->setCheckable(true);
    m_actions[ActionZoom75] = new QAction(tr("75%"), this);
    m_actions[ActionZoom75]->setStatusTip(tr("Zoom to 75%"));
    m_actions[ActionZoom75]->setShortcut(Qt::CTRL | Qt::Key_4);
    m_actions[ActionZoom75]->setCheckable(true);
    m_actions[ActionZoom50] = new QAction(tr("50%"), this);
    m_actions[ActionZoom50]->setStatusTip(tr("Zoom to 50%"));
    m_actions[ActionZoom50]->setShortcut(Qt::CTRL | Qt::Key_5);
    m_actions[ActionZoom50]->setCheckable(true);
    m_actions[ActionZoom25] = new QAction(tr("25%"), this);
    m_actions[ActionZoom25]->setStatusTip(tr("Zoom to 25%"));
    m_actions[ActionZoom25]->setShortcut(Qt::CTRL | Qt::Key_6);
    m_actions[ActionZoom25]->setCheckable(true);
    m_actions[ActionZoom125] = new QAction(tr("12.5%"), this);
    m_actions[ActionZoom125]->setStatusTip(tr("Zoom to 12.5%"));
    m_actions[ActionZoom125]->setShortcut(Qt::CTRL | Qt::Key_7);
    m_actions[ActionZoom125]->setCheckable(true);
    m_actions[ActionZoomCust] = new QAction(tr("&Custom..."), this);
    m_actions[ActionZoomCust]->setStatusTip(tr("Zoom to custom percentage"));
    m_actions[ActionZoomCust]->setShortcut(Qt::CTRL | Qt::Key_9);
    m_actions[ActionZoomCust]->setCheckable(true);
    m_actions[ActionZoomFit] = new QAction(tr("&Fit window"), this);
    m_actions[ActionZoomFit]->setStatusTip(tr("Zoom to fit view area"));
    m_actions[ActionZoomFit]->setShortcut(Qt::CTRL | Qt::Key_0);
    m_actions[ActionZoomFit]->setCheckable(true);
    auto zoomGroup = new QActionGroup(this);
    zoomGroup->addAction(m_actions[ActionZoom200]);
    zoomGroup->addAction(m_actions[ActionZoom150]);
    zoomGroup->addAction(m_actions[ActionZoom100]);
    zoomGroup->addAction(m_actions[ActionZoom75]);
    zoomGroup->addAction(m_actions[ActionZoom50]);
    zoomGroup->addAction(m_actions[ActionZoom25]);
    zoomGroup->addAction(m_actions[ActionZoom125]);
    zoomGroup->addAction(m_actions[ActionZoomCust]);
    zoomGroup->addAction(m_actions[ActionZoomFit]);

    m_actions[ActionTestChips] = new QAction(tr("Test in &MSCC"), this);
    m_actions[ActionTestChips]->setStatusTip(tr("Test the current level in Chips.exe"));
    m_actions[ActionTestChips]->setShortcut(Qt::Key_F5);
    m_actions[ActionTestChips]->setEnabled(false);
    m_actions[ActionTestTWorldCC] = new QAction(tr("Test in &Tile World (MSCC)"), this);
    m_actions[ActionTestTWorldCC]->setStatusTip(tr("Test the current level in Tile World with the MSCC Ruleset"));
    m_actions[ActionTestTWorldCC]->setShortcut(Qt::Key_F6);
    m_actions[ActionTestTWorldCC]->setEnabled(false);
    m_actions[ActionTestTWorldLynx] = new QAction(tr("Test in Tile World (&Lynx)"), this);
    m_actions[ActionTestTWorldLynx]->setStatusTip(tr("Test the current level in Tile World with the Lynx Ruleset"));
    m_actions[ActionTestTWorldLynx]->setShortcut(Qt::Key_F7);
    m_actions[ActionTestTWorldLynx]->setEnabled(false);
    m_actions[ActionTestLexy] = new QAction(tr("Test in &Lexy's Labyrinth"), this);
    m_actions[ActionTestLexy]->setStatusTip(tr("Test the current level in Lexy's Labyrinth (opens browser)"));
    m_actions[ActionTestLexy]->setShortcut(Qt::Key_F8);
    m_actions[ActionTestLexy]->setEnabled(false);
    m_actions[ActionTestSetup] = new QAction(tr("&Setup Testing..."), this);
    m_actions[ActionTestSetup]->setStatusTip(tr("Setup testing parameters and options"));

    m_actions[ActionAbout] = new QAction(ICON("help-about"), tr("&About CCEdit"), this);
    m_actions[ActionAbout]->setStatusTip(tr("Show information about CCEdit"));

    m_actions[ActionAddLevel] = new QAction(ICON("list-add"), tr("&Add Level"), this);
    m_actions[ActionAddLevel]->setStatusTip(tr("Add a new level to the end of the levelset"));
    m_actions[ActionAddLevel]->setEnabled(false);
    m_actions[ActionDelLevel] = new QAction(ICON("list-remove"), tr("&Remove Level"), this);
    m_actions[ActionDelLevel]->setStatusTip(tr("Remove the current level from the levelset"));
    m_actions[ActionDelLevel]->setEnabled(false);
    m_actions[ActionMoveUp] = new QAction(ICON("arrow-up"), tr("Move &Up"), this);
    m_actions[ActionMoveUp]->setStatusTip(tr("Move the current level up in the level list"));
    m_actions[ActionMoveUp]->setEnabled(false);
    m_actions[ActionMoveDown] = new QAction(ICON("arrow-down"), tr("Move &Down"), this);
    m_actions[ActionMoveDown]->setStatusTip(tr("Move the current level down in the level list"));
    m_actions[ActionMoveDown]->setEnabled(false);
    m_actions[ActionOrganize] = new QAction(ICON("level-organize"), tr("&Organize Levels"), this);
    m_actions[ActionOrganize]->setStatusTip(tr("Organize or import levels"));
    m_actions[ActionOrganize]->setEnabled(false);

    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::canUndoChanged, m_actions[ActionUndo], &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, m_actions[ActionRedo], &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::cleanChanged, this, &CCEditMain::onCleanChanged);

    // Control Toolbox
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

    auto levelManWidget = new QWidget(this);
    m_levelList = new QListWidget(levelManWidget);
    m_levelProperties = new LevelProperties(levelManWidget);

    auto tbarLevelset = new QToolBar(levelManWidget);
    tbarLevelset->addAction(m_actions[ActionAddLevel]);
    tbarLevelset->addAction(m_actions[ActionDelLevel]);
    tbarLevelset->addSeparator();
    tbarLevelset->addAction(m_actions[ActionMoveUp]);
    tbarLevelset->addAction(m_actions[ActionMoveDown]);
    tbarLevelset->addSeparator();
    tbarLevelset->addAction(m_actions[ActionOrganize]);

    auto levelManLayout = new QVBoxLayout(levelManWidget);
    levelManLayout->setContentsMargins(4, 4, 4, 4);
    levelManLayout->setSpacing(0);
    levelManLayout->addWidget(m_levelList);
    levelManLayout->addWidget(tbarLevelset);
    levelManLayout->addItem(new QSpacerItem(0, 8));
    levelManLayout->addWidget(m_levelProperties);

    m_levelManDock = new QDockWidget(this);
    m_levelManDock->setObjectName(QStringLiteral("LevelManagerDock"));
    m_levelManDock->setWindowTitle(tr("Level Manager"));
    m_levelManDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_levelManDock->setWidget(levelManWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_levelManDock);

    auto tileWidget = new QWidget(this);
    auto tileBox = new QToolBox(tileWidget);
    TileListWidget *tileLists[NUM_TILE_LISTS];
    tileLists[ListStandard] = new TileListWidget(tileBox);
    tileLists[ListStandard]->addTiles(QVector<tile_t>{
        ccl::TileFloor, ccl::TileWall, ccl::TileChip, ccl::TileSocket,
        ccl::TileExit, ccl::TileHint, ccl::TileBarrier_N,
        ccl::TileBarrier_W, ccl::TileBarrier_S, ccl::TileBarrier_E,
        ccl::TileBarrier_SE, ccl::TileBlock, ccl::TileDirt,
        ccl::TileGravel, ccl::TilePlayer_N, ccl::TilePlayer_W,
        ccl::TilePlayer_S, ccl::TilePlayer_E
    });
    tileBox->addItem(tileLists[ListStandard], tr("Standard"));
    tileLists[ListObstacles] = new TileListWidget(tileBox);
    tileLists[ListObstacles]->addTiles(QVector<tile_t>{
        ccl::TileWater, ccl::TileFire, ccl::TileBomb, ccl::TileForce_N,
        ccl::TileForce_W, ccl::TileForce_S, ccl::TileForce_E,
        ccl::TileForce_Rand, ccl::TileIce, ccl::TileIce_NW,
        ccl::TileIce_NE, ccl::TileIce_SE, ccl::TileIce_SW,
        ccl::TileTrap, ccl::TileTrapButton, ccl::TilePopUpWall,
        ccl::TileAppearingWall, ccl::TileInvisWall
    });
    tileBox->addItem(tileLists[ListObstacles], tr("Obstacles"));
    tileLists[ListDoors] = new TileListWidget(tileBox);
    tileLists[ListDoors]->addTiles(QVector<tile_t>{
        ccl::TileDoor_Blue, ccl::TileDoor_Red, ccl::TileDoor_Green,
        ccl::TileDoor_Yellow, ccl::TileToggleFloor, ccl::TileToggleWall,
        ccl::TileToggleButton
    });
    tileBox->addItem(tileLists[ListDoors], tr("Doors"));
    tileLists[ListItems] = new TileListWidget(tileBox);
    tileLists[ListItems]->addTiles(QVector<tile_t>{
        ccl::TileKey_Blue, ccl::TileKey_Red, ccl::TileKey_Green,
        ccl::TileKey_Yellow, ccl::TileFlippers, ccl::TileFireBoots,
        ccl::TileIceSkates, ccl::TileForceBoots
    });
    tileBox->addItem(tileLists[ListItems], tr("Items"));
    tileLists[ListMonsters] = new TileListWidget(tileBox);
    tileLists[ListMonsters]->addTiles(QVector<tile_t>{
        ccl::TileBug_N, ccl::TileBug_W, ccl::TileBug_S, ccl::TileBug_E,
        ccl::TileFireball_N, ccl::TileFireball_W, ccl::TileFireball_S,
        ccl::TileFireball_E, ccl::TileBall_N, ccl::TileBall_W,
        ccl::TileBall_S, ccl::TileBall_E, ccl::TileTank_N,
        ccl::TileTank_W, ccl::TileTank_S, ccl::TileTank_E,
        ccl::TileTankButton, ccl::TileGlider_N, ccl::TileGlider_W,
        ccl::TileGlider_S, ccl::TileGlider_E, ccl::TileTeeth_N,
        ccl::TileTeeth_W, ccl::TileTeeth_S, ccl::TileTeeth_E,
        ccl::TileWalker_N, ccl::TileWalker_W, ccl::TileWalker_S,
        ccl::TileWalker_E, ccl::TileBlob_N, ccl::TileBlob_W,
        ccl::TileBlob_S, ccl::TileBlob_E, ccl::TileCrawler_N,
        ccl::TileCrawler_W, ccl::TileCrawler_S, ccl::TileCrawler_E
    });
    tileBox->addItem(tileLists[ListMonsters], tr("Monsters"));
    tileLists[ListMisc] = new TileListWidget(tileBox);
    tileLists[ListMisc]->addTiles(QVector<tile_t>{
        ccl::TileThief, ccl::TileBlueWall, ccl::TileBlueFloor,
        ccl::TileTeleport, ccl::TileCloner, ccl::TileCloneButton,
        ccl::TileBlock_N, ccl::TileBlock_W, ccl::TileBlock_S,
        ccl::TileBlock_E
    });
    tileBox->addItem(tileLists[ListMisc], tr("Miscellaneous"));
    tileLists[ListSpecial] = new TileListWidget(tileBox);
    tileLists[ListSpecial]->addTiles(QVector<tile_t>{
        ccl::TileIceBlock, ccl::TilePlayerSplash, ccl::TilePlayerFire,
        ccl::TilePlayerBurnt, ccl::TilePlayerExit, ccl::TileExitAnim2,
        ccl::TileExitAnim3, ccl::TilePlayerSwim_N, ccl::TilePlayerSwim_W,
        ccl::TilePlayerSwim_S, ccl::TilePlayerSwim_E, ccl::Tile_UNUSED_20,
        ccl::Tile_UNUSED_36, ccl::Tile_UNUSED_37
    });
    tileBox->addItem(tileLists[ListSpecial], tr("Special (Advanced)"));

    for (auto listWidget : tileLists) {
        connect(this, &CCEditMain::tilesetChanged, listWidget, &TileListWidget::setTileImages);
        connect(listWidget, &TileListWidget::itemSelectedLeft, this, &CCEditMain::setLeftTile);
        connect(listWidget, &TileListWidget::itemSelectedRight, this, &CCEditMain::setRightTile);
    }

    auto layerWidget = new LayerWidget(tileWidget);
    auto leftLabel = new QLabel(tr("Left Button: "), tileWidget);
    auto rightLabel = new QLabel(tr("Right Button: "), tileWidget);
    auto leftTileLabel = new QLabel(tileWidget);
    auto rightTileLabel = new QLabel(tileWidget);
    leftTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    rightTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(this, &CCEditMain::tilesetChanged, layerWidget, &LayerWidget::setTileset);
    connect(this, &CCEditMain::leftTileChanged, layerWidget, &LayerWidget::setUpper);
    connect(this, &CCEditMain::rightTileChanged, layerWidget, &LayerWidget::setLower);
    connect(this, &CCEditMain::leftTileChanged, this, [leftTileLabel](tile_t tile) {
        leftTileLabel->setText(CCETileset::TileName(tile));
    });
    connect(this, &CCEditMain::rightTileChanged, this, [rightTileLabel](tile_t tile) {
        rightTileLabel->setText(CCETileset::TileName(tile));
    });

    QGridLayout* tileLayout = new QGridLayout(tileWidget);
    tileLayout->setContentsMargins(4, 4, 4, 4);
    tileLayout->setVerticalSpacing(4);
    tileLayout->addWidget(tileBox, 0, 0, 1, 3);
    tileLayout->addWidget(leftLabel, 1, 0);
    tileLayout->addWidget(leftTileLabel, 1, 1);
    tileLayout->addWidget(rightLabel, 2, 0);
    tileLayout->addWidget(rightTileLabel, 2, 1);
    tileLayout->addWidget(layerWidget, 1, 2, 2, 1);

    auto sortedTilesDock = new QDockWidget(this);
    sortedTilesDock->setObjectName(QStringLiteral("SortedTilesDock"));
    sortedTilesDock->setWindowTitle(tr("Tiles - Sorted"));
    sortedTilesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    sortedTilesDock->setWidget(tileWidget);
    tabifyDockWidget(m_levelManDock, sortedTilesDock);

    auto allTileWidget = new QWidget(this);
    auto allTileScroll = new QScrollArea(allTileWidget);
    auto allTiles = new BigTileWidget(allTileScroll);
    allTileScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setWidget(allTiles);

    layerWidget = new LayerWidget(allTileWidget);
    leftLabel = new QLabel(tr("Left Button: "), allTileWidget);
    rightLabel = new QLabel(tr("Right Button: "), allTileWidget);
    leftTileLabel = new QLabel(allTileWidget);
    rightTileLabel = new QLabel(allTileWidget);
    leftTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    rightTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(this, &CCEditMain::tilesetChanged, allTiles, &BigTileWidget::setTileset);
    connect(allTiles, &BigTileWidget::itemSelectedLeft, this, &CCEditMain::setLeftTile);
    connect(allTiles, &BigTileWidget::itemSelectedRight, this, &CCEditMain::setRightTile);
    connect(this, &CCEditMain::tilesetChanged, layerWidget, &LayerWidget::setTileset);
    connect(this, &CCEditMain::leftTileChanged, layerWidget, &LayerWidget::setUpper);
    connect(this, &CCEditMain::rightTileChanged, layerWidget, &LayerWidget::setLower);
    connect(this, &CCEditMain::leftTileChanged, this, [leftTileLabel](tile_t tile) {
        leftTileLabel->setText(CCETileset::TileName(tile));
    });
    connect(this, &CCEditMain::rightTileChanged, this, [rightTileLabel](tile_t tile) {
        rightTileLabel->setText(CCETileset::TileName(tile));
    });

    QGridLayout* allTileLayout = new QGridLayout(allTileWidget);
    allTileLayout->setContentsMargins(4, 4, 4, 4);
    allTileLayout->setVerticalSpacing(4);
    allTileLayout->addWidget(allTileScroll, 0, 0, 1, 3);
    allTileLayout->addWidget(leftLabel, 1, 0);
    allTileLayout->addWidget(leftTileLabel, 1, 1);
    allTileLayout->addWidget(rightLabel, 2, 0);
    allTileLayout->addWidget(rightTileLabel, 2, 1);
    allTileLayout->addWidget(layerWidget, 1, 2, 2, 1);

    auto allTilesDock = new QDockWidget(this);
    allTilesDock->setObjectName(QStringLiteral("AllTilesDock"));
    allTilesDock->setWindowTitle(tr("All Tiles"));
    allTilesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    allTilesDock->setWidget(allTileWidget);
    tabifyDockWidget(m_levelManDock, allTilesDock);

    // Editor area
    m_editorTabs = new EditorTabWidget(this);
    setCentralWidget(m_editorTabs);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNew]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionOpen]);
    m_recentFiles = fileMenu->addMenu(tr("Open &Recent"));
    populateRecentFiles();
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionSave]);
    fileMenu->addAction(m_actions[ActionSaveAs]);
    fileMenu->addAction(m_actions[ActionCloseTab]);
    fileMenu->addAction(m_actions[ActionCloseLevelset]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionProperties]);
    fileMenu->addAction(m_actions[ActionGenReport]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionExit]);

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_actions[ActionUndo]);
    editMenu->addAction(m_actions[ActionRedo]);
    editMenu->addSeparator();
    editMenu->addAction(m_actions[ActionSelect]);
    editMenu->addAction(m_actions[ActionCut]);
    editMenu->addAction(m_actions[ActionCopy]);
    editMenu->addAction(m_actions[ActionPaste]);
    editMenu->addSeparator();
    editMenu->addAction(m_actions[ActionClear]);

    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(m_actions[ActionDrawPencil]);
    toolsMenu->addAction(m_actions[ActionDrawLine]);
    toolsMenu->addAction(m_actions[ActionDrawRect]);
    toolsMenu->addAction(m_actions[ActionDrawFill]);
    toolsMenu->addAction(m_actions[ActionDrawFlood]);
    toolsMenu->addAction(m_actions[ActionPathMaker]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionConnect]);
    toolsMenu->addAction(m_actions[ActionAdvancedMech]);
    toolsMenu->addAction(m_actions[ActionInspectTiles]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionToggleWalls]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionCheckErrors]);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_actions[ActionViewButtons]);
    viewMenu->addAction(m_actions[ActionViewMovers]);
    viewMenu->addAction(m_actions[ActionViewCloners]);
    viewMenu->addAction(m_actions[ActionViewTraps]);
    viewMenu->addAction(m_actions[ActionViewActivePlayer]);
    viewMenu->addAction(m_actions[ActionViewViewport]);
    viewMenu->addAction(m_actions[ActionViewMonsterPaths]);
    viewMenu->addAction(m_actions[ActionViewErrors]);
    viewMenu->addAction(m_actions[ActionViewButtonsOnMouse]);
    viewMenu->addSeparator();
    QMenu* advancedMSCCMenu = viewMenu->addMenu(tr("&Advanced (MSCC Only)"));
    advancedMSCCMenu->addAction(m_actions[ActionViewDRCloners]);
    advancedMSCCMenu->addAction(m_actions[ActionViewMultiTanks]);
    viewMenu->addSeparator();
    QMenu* dockMenu = viewMenu->addMenu(tr("&Toolbox"));
    dockMenu->addAction(m_levelManDock->toggleViewAction());
    dockMenu->addAction(sortedTilesDock->toggleViewAction());
    dockMenu->addAction(allTilesDock->toggleViewAction());
    viewMenu->addSeparator();
    m_tilesetMenu = viewMenu->addMenu(tr("Tile&set"));
    m_tilesetGroup = new QActionGroup(this);
    QMenu* zoomMenu = viewMenu->addMenu(tr("&Zoom"));
    zoomMenu->addAction(m_actions[ActionZoom200]);
    zoomMenu->addAction(m_actions[ActionZoom150]);
    zoomMenu->addAction(m_actions[ActionZoom100]);
    zoomMenu->addAction(m_actions[ActionZoom75]);
    zoomMenu->addAction(m_actions[ActionZoom50]);
    zoomMenu->addAction(m_actions[ActionZoom25]);
    zoomMenu->addAction(m_actions[ActionZoom125]);
    zoomMenu->addAction(m_actions[ActionZoomCust]);
    zoomMenu->addAction(m_actions[ActionZoomFit]);

    QMenu* testMenu = menuBar()->addMenu(tr("Te&st"));
    testMenu->addAction(m_actions[ActionTestChips]);
    testMenu->addAction(m_actions[ActionTestTWorldCC]);
    testMenu->addAction(m_actions[ActionTestTWorldLynx]);
    testMenu->addAction(m_actions[ActionTestLexy]);
    testMenu->addSeparator();
    testMenu->addAction(m_actions[ActionTestSetup]);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_actions[ActionAbout]);

    // Tool bars
    QToolBar* tbarMain = addToolBar(QString());
    tbarMain->setObjectName(QStringLiteral("ToolbarMain"));
    tbarMain->setWindowTitle(tr("Main"));
    tbarMain->addAction(m_actions[ActionNew]);
    tbarMain->addAction(m_actions[ActionOpen]);
    tbarMain->addAction(m_actions[ActionSave]);
    tbarMain->addSeparator();
    tbarMain->addAction(m_actions[ActionUndo]);
    tbarMain->addAction(m_actions[ActionRedo]);
    tbarMain->addSeparator();
    tbarMain->addAction(m_actions[ActionSelect]);
    tbarMain->addAction(m_actions[ActionCut]);
    tbarMain->addAction(m_actions[ActionCopy]);
    tbarMain->addAction(m_actions[ActionPaste]);
    QToolBar* tbarTools = addToolBar(QString());
    tbarTools->setObjectName(QStringLiteral("ToolbarTools"));
    tbarTools->setWindowTitle(tr("Tools"));
    tbarTools->addAction(m_actions[ActionDrawPencil]);
    tbarTools->addAction(m_actions[ActionDrawLine]);
    tbarTools->addAction(m_actions[ActionDrawRect]);
    tbarTools->addAction(m_actions[ActionDrawFill]);
    tbarTools->addAction(m_actions[ActionDrawFlood]);
    tbarTools->addAction(m_actions[ActionPathMaker]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionConnect]);
    tbarTools->addAction(m_actions[ActionAdvancedMech]);
    tbarTools->addAction(m_actions[ActionInspectTiles]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionToggleWalls]);

    // Show status bar
    statusBar();

    connect(m_actions[ActionNew], &QAction::triggered, this, &CCEditMain::createNewLevelset);
    connect(m_actions[ActionOpen], &QAction::triggered, this, &CCEditMain::onOpenAction);
    connect(m_actions[ActionSave], &QAction::triggered, this, &CCEditMain::onSaveAction);
    connect(m_actions[ActionSaveAs], &QAction::triggered, this, &CCEditMain::onSaveAsAction);
    connect(m_actions[ActionCloseTab], &QAction::triggered, this, [this] {
        const int index = m_editorTabs->currentIndex();
        if (index >= 0)
            m_editorTabs->widget(index)->deleteLater();
    });
    connect(m_actions[ActionCloseLevelset], &QAction::triggered, this, &CCEditMain::closeLevelset);
    connect(m_actions[ActionProperties], &QAction::triggered, this, &CCEditMain::onPropertiesAction);
    connect(m_actions[ActionGenReport], &QAction::triggered, this, &CCEditMain::onReportAction);
    connect(m_actions[ActionExit], &QAction::triggered, this, &CCEditMain::close);
    connect(m_actions[ActionSelect], &QAction::toggled, this, &CCEditMain::onSelectToggled);
    connect(m_actions[ActionCut], &QAction::triggered, this, &CCEditMain::onCutAction);
    connect(m_actions[ActionCopy], &QAction::triggered, this, &CCEditMain::onCopyAction);
    connect(m_actions[ActionPaste], &QAction::triggered, this, &CCEditMain::onPasteAction);
    connect(m_actions[ActionClear], &QAction::triggered, this, &CCEditMain::onClearAction);
    connect(m_actions[ActionUndo], &QAction::triggered, this, &CCEditMain::onUndoAction);
    connect(m_actions[ActionRedo], &QAction::triggered, this, &CCEditMain::onRedoAction);
    connect(m_actions[ActionDrawPencil], &QAction::toggled, this, &CCEditMain::onDrawPencilAction);
    connect(m_actions[ActionDrawLine], &QAction::toggled, this, &CCEditMain::onDrawLineAction);
    connect(m_actions[ActionDrawRect], &QAction::toggled, this, &CCEditMain::onDrawRectAction);
    connect(m_actions[ActionDrawFill], &QAction::toggled, this, &CCEditMain::onDrawFillAction);
    connect(m_actions[ActionDrawFlood], &QAction::toggled, this, &CCEditMain::onDrawFloodAction);
    connect(m_actions[ActionPathMaker], &QAction::toggled, this, &CCEditMain::onPathMakerToggled);
    connect(m_actions[ActionConnect], &QAction::toggled, this, &CCEditMain::onConnectToggled);
    connect(m_actions[ActionAdvancedMech], &QAction::triggered, this, &CCEditMain::onAdvancedMechAction);
    connect(m_actions[ActionInspectTiles], &QAction::triggered, this, &CCEditMain::onInspectTilesToggled);
    connect(m_actions[ActionToggleWalls], &QAction::triggered, this, &CCEditMain::onToggleWallsAction);
    connect(m_actions[ActionCheckErrors], &QAction::triggered, this, &CCEditMain::onCheckErrorsAction);
    connect(m_actions[ActionViewButtons], &QAction::toggled, this, &CCEditMain::onViewButtonsToggled);
    connect(m_actions[ActionViewMovers], &QAction::toggled, this, &CCEditMain::onViewMoversToggled);
    connect(m_actions[ActionViewCloners], &QAction::toggled, this, &CCEditMain::onViewClonersToggled);
    connect(m_actions[ActionViewTraps], &QAction::toggled, this, &CCEditMain::onViewTrapsToggled);
    connect(m_actions[ActionViewDRCloners], &QAction::toggled, this, &CCEditMain::onViewDRClonersToggled);
    connect(m_actions[ActionViewMultiTanks], &QAction::toggled, this, &CCEditMain::onViewMultiTanksToggled);
    connect(m_actions[ActionViewActivePlayer], &QAction::toggled, this, &CCEditMain::onViewActivePlayerToggled);
    connect(m_actions[ActionViewViewport], &QAction::toggled, this, &CCEditMain::onViewViewportToggled);
    connect(m_actions[ActionViewMonsterPaths], &QAction::toggled, this, &CCEditMain::onViewMonsterPathsToggled);
    connect(m_actions[ActionViewErrors], &QAction::toggled, this, &CCEditMain::onViewErrorsToggled);
    connect(m_actions[ActionViewButtonsOnMouse], &QAction::toggled, this, &CCEditMain::onViewButtonsOnMouseToggled);
    connect(m_tilesetGroup, &QActionGroup::triggered, this, &CCEditMain::onTilesetMenu);
    connect(m_actions[ActionZoom200], &QAction::triggered, this, [this] { setZoomFactor(2.0); });
    connect(m_actions[ActionZoom150], &QAction::triggered, this, [this] { setZoomFactor(1.5); });
    connect(m_actions[ActionZoom100], &QAction::triggered, this, [this] { setZoomFactor(1.0); });
    connect(m_actions[ActionZoom75], &QAction::triggered, this, [this] { setZoomFactor(0.75); });
    connect(m_actions[ActionZoom50], &QAction::triggered, this, [this] { setZoomFactor(0.5); });
    connect(m_actions[ActionZoom25], &QAction::triggered, this, [this] { setZoomFactor(0.25); });
    connect(m_actions[ActionZoom125], &QAction::triggered, this, [this] { setZoomFactor(0.125); });
    connect(m_actions[ActionZoomCust], &QAction::triggered, this, &CCEditMain::onZoomCust);
    connect(m_actions[ActionZoomFit], &QAction::triggered, this, &CCEditMain::onZoomFit);
    connect(m_actions[ActionTestChips], &QAction::triggered, this, &CCEditMain::onTestChips);
    connect(m_actions[ActionTestTWorldCC], &QAction::triggered, this, [this] {
        onTestTWorld(ccl::Levelset::TypeMS);
    });
    connect(m_actions[ActionTestTWorldLynx], &QAction::triggered, this, [this] {
        onTestTWorld(ccl::Levelset::TypeLynx);
    });
    connect(m_actions[ActionTestLexy], &QAction::triggered, this, &CCEditMain::onTestLexy);
    connect(m_actions[ActionTestSetup], &QAction::triggered, this, [] {
        TestSetupDialog dlg;
        dlg.exec();
    });

    connect(m_actions[ActionAbout], &QAction::triggered, this, [this] {
        AboutDialog about(QStringLiteral("CCEdit"),
                          QPixmap(QStringLiteral(":/icons/boot-48.png")), this);
        about.exec();
    });

    connect(m_actions[ActionAddLevel], &QAction::triggered, this, &CCEditMain::onAddLevelAction);
    connect(m_actions[ActionDelLevel], &QAction::triggered, this, &CCEditMain::onDelLevelAction);
    connect(m_actions[ActionMoveUp], &QAction::triggered, this, &CCEditMain::onMoveUpAction);
    connect(m_actions[ActionMoveDown], &QAction::triggered, this, &CCEditMain::onMoveDownAction);
    connect(m_actions[ActionOrganize], &QAction::triggered, this, &CCEditMain::onOrganizeAction);

    connect(m_levelList, &QListWidget::currentRowChanged, this, &CCEditMain::onSelectLevel);
    connect(m_levelList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        loadLevel(m_levelList->row(item));
        m_editorTabs->promoteTab();
    });

    connect(m_levelProperties, &LevelProperties::nameChanged,
            this, &CCEditMain::onNameChanged);
    connect(m_levelProperties, &LevelProperties::authorChanged,
            this, &CCEditMain::onAuthorChanged);
    connect(m_levelProperties, &LevelProperties::passwordChanged,
            this, &CCEditMain::onPasswordChanged);
    connect(m_levelProperties, &LevelProperties::chipsChanged,
            this, &CCEditMain::onChipsChanged);
    connect(m_levelProperties, &LevelProperties::chipCountRequested,
            this, &CCEditMain::onChipCountAction);
    connect(m_levelProperties, &LevelProperties::timerChanged,
            this, &CCEditMain::onTimerChanged);
    connect(m_levelProperties, &LevelProperties::hintChanged,
            this, &CCEditMain::onHintChanged);

    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &CCEditMain::onClipboardDataChanged);

    connect(m_editorTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        m_editorTabs->widget(index)->deleteLater();
    });
    connect(m_editorTabs, &QTabWidget::currentChanged, this, &CCEditMain::onTabChanged);

    // Load window settings and defaults
    QSettings settings;
    resize(settings.value(QStringLiteral("WindowSize"), QSize(1024, 768)).toSize());
    if (settings.value(QStringLiteral("WindowMaximized"), false).toBool())
        showMaximized();
    if (settings.contains(QStringLiteral("WindowState2")))
        restoreState(settings.value(QStringLiteral("WindowState2")).toByteArray());
    m_zoomFactor = settings.value(QStringLiteral("ZoomFactor"), 1.0).toDouble();
    m_actions[ActionViewButtons]->setChecked(
            settings.value(QStringLiteral("ViewButtons"), true).toBool());
    m_actions[ActionViewMovers]->setChecked(
            settings.value(QStringLiteral("ViewMovers"), true).toBool());
    m_actions[ActionViewCloners]->setChecked(
            settings.value(QStringLiteral("ViewCloners"), false).toBool());
    m_actions[ActionViewTraps]->setChecked(
            settings.value(QStringLiteral("ViewTraps"), false).toBool());
    m_actions[ActionViewDRCloners]->setChecked(
            settings.value(QStringLiteral("ViewDRClones"), false).toBool());
    m_actions[ActionViewMultiTanks]->setChecked(
            settings.value(QStringLiteral("ViewMultiTanks"), false).toBool());
    m_actions[ActionViewActivePlayer]->setChecked(
            settings.value(QStringLiteral("ViewActivePlayer"), false).toBool());
    m_actions[ActionViewViewport]->setChecked(
            settings.value(QStringLiteral("ViewViewport"), true).toBool());
    m_actions[ActionViewMonsterPaths]->setChecked(
            settings.value(QStringLiteral("ViewMonsterPaths"), false).toBool());
    m_actions[ActionViewErrors]->setChecked(
            settings.value(QStringLiteral("ViewErrors"), true).toBool());
    m_actions[ActionViewButtonsOnMouse]->setChecked(
            settings.value(QStringLiteral("ViewButtonsOnMouse"), false).toBool());

    // Make sure the toolbox docks are visible
    QDockWidget* docks[] = {m_levelManDock, sortedTilesDock, allTilesDock};
    for (QDockWidget* dock : docks) {
        if (dock->isFloating()) {
            QPoint dockPos = dock->pos();
            const QRect desktopRect = QApplication::desktop()->contentsRect();
            if ((dockPos.x() + dock->width() - 10) < desktopRect.left())
                dockPos.setX(desktopRect.left());
            if (dockPos.x() + 10 > desktopRect.right())
                dockPos.setX(desktopRect.right() - dock->width());
            if (dockPos.y() < desktopRect.top())
                dockPos.setY(desktopRect.top());
            if (dockPos.y() + 10 > desktopRect.bottom())
                dockPos.setY(desktopRect.bottom() - dock->height());
            dock->move(dockPos);
            dock->show();
        }
    }
    m_levelManDock->raise();

    m_scaleGroup = new QActionGroup(this);
    QAction* scale1x = m_scaleGroup->addAction(tr("Original Size"));
    scale1x->setData(1);
    scale1x->setCheckable(true);
    scale1x->setStatusTip(tr("Render tiles at 1X scale"));
    QAction* scale2x = m_scaleGroup->addAction(tr("Scale 2X"));
    scale2x->setData(2);
    scale2x->setCheckable(true);
    scale2x->setStatusTip(tr("Render tiles at 2X scale (for high DPI displays)"));

    for (QAction* scaleAction : m_scaleGroup->actions()) {
        int scale = scaleAction->data().toInt();
        connect(scaleAction, &QAction::triggered, this, [this, scale] {
            m_currentTileset->setUiScale(qreal(scale));
            if (m_zoomFactor != 0.0)
                setZoomFactor(m_zoomFactor);
            emit tilesetChanged(m_currentTileset);

            QSettings settings;
            settings.setValue(QStringLiteral("TilesetScale"), scale);
        });
    }

    populateTilesets();
    if (m_tilesetGroup->actions().size() == 0) {
        QMessageBox::critical(this, tr("Error loading tilesets"),
                tr("Error: No tilesets found.  Please check your CCTools installation"),
                QMessageBox::Ok);
        exit(1);
    } else {
        QString tilesetFilename = settings.value(QStringLiteral("TilesetName"),
                                                 QStringLiteral("WEP.tis")).toString();
        bool foundTset = false;
        for (int i = 0; i < m_tilesetGroup->actions().size(); ++i) {
            auto tileset = m_tilesetGroup->actions()[i]->data().value<CCETileset*>();
            if (tileset->filename() == tilesetFilename) {
                m_tilesetGroup->actions()[i]->setChecked(true);
                loadTileset(tileset);
                foundTset = true;
                break;
            }
        }
        if (!foundTset) {
            m_tilesetGroup->actions()[0]->setChecked(true);
            loadTileset(m_tilesetGroup->actions()[0]->data().value<CCETileset*>());
        }
    }

    if (m_zoomFactor == 1.0)
        m_actions[ActionZoom100]->setChecked(true);
    else if (m_zoomFactor == 0.0)
        m_actions[ActionZoomFit]->setChecked(true);
    else if (m_zoomFactor == 2.0)
        m_actions[ActionZoom200]->setChecked(true);
    else if (m_zoomFactor == 1.5)
        m_actions[ActionZoom150]->setChecked(true);
    else if (m_zoomFactor == 0.75)
        m_actions[ActionZoom75]->setChecked(true);
    else if (m_zoomFactor == 0.5)
        m_actions[ActionZoom50]->setChecked(true);
    else if (m_zoomFactor == 0.25)
        m_actions[ActionZoom25]->setChecked(true);
    else if (m_zoomFactor == 0.125)
        m_actions[ActionZoom125]->setChecked(true);
    else
        m_actions[ActionZoomCust]->setChecked(true);

    const int tilesetScale = settings.value(QStringLiteral("TilesetScale"), 1).toInt();
    for (QAction* scaleAction : m_scaleGroup->actions()) {
        if (scaleAction->data().toInt() == tilesetScale)
            scaleAction->setChecked(true);
    }

    setLeftTile(ccl::TileWall);
    setRightTile(ccl::TileFloor);
    onSelectLevel(-1);
    onClipboardDataChanged();
}

void CCEditMain::loadLevelset(const QString& filename)
{
    if (!closeLevelset())
        return;

    ccl::LevelsetType type = ccl::DetermineLevelsetType(filename);
    if (type == ccl::LevelsetCcl) {
        ccl::FileStream set;
        if (set.open(filename, ccl::FileStream::Read)) {
            m_levelset = new ccl::Levelset(0);
            try {
                m_levelset->read(&set);
            } catch (const ccl::RuntimeError& e) {
                QMessageBox::critical(this, tr("Error reading levelset"),
                                      tr("Error loading levelset: %1").arg(e.message()));
                delete m_levelset;
                m_levelset = nullptr;
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                  tr("Error: could not open file %1").arg(filename));
            return;
        }
        doLevelsetLoad();
        setLevelsetFilename(filename);
        m_useDac = false;
    } else if (type == ccl::LevelsetDac) {
        ccl::unique_FILE dac = ccl::FileStream::Fopen(filename, ccl::FileStream::ReadText);
        if (!dac) {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                  tr("Error: could not open file %1").arg(filename));
            return;
        }

        try {
            m_dacInfo.read(dac.get());
        } catch (const ccl::RuntimeError& e) {
            QMessageBox::critical(this, tr("Error reading levelset"),
                                  tr("Error loading levelset descriptor: %1")
                                  .arg(e.message()));
            return;
        }

        QDir searchPath(filename);
        searchPath.cdUp();

        ccl::FileStream set;
        if (set.open(searchPath.absoluteFilePath(m_dacInfo.m_filename), ccl::FileStream::Read)) {
            m_levelset = new ccl::Levelset(0);
            try {
                m_levelset->read(&set);
            } catch (const ccl::RuntimeError& e) {
                QMessageBox::critical(this, tr("Error reading levelset"),
                                      tr("Error loading levelset: %1").arg(e.message()));
                delete m_levelset;
                m_levelset = nullptr;
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                tr("Error: could not open file %1")
                                .arg(m_dacInfo.m_filename));
            return;
        }
        doLevelsetLoad();
        setLevelsetFilename(filename);
        m_useDac = true;
    } else {
        QMessageBox::critical(this, tr("Error reading levelset"),
                              tr("Cannot determine file type for %1").arg(filename));
        return;
    }

    m_haveCcx = false;
    QString ccxName = filename.left(filename.lastIndexOf(QLatin1Char('.'))) + QStringLiteral(".ccx");
    if (m_ccxFile.readFile(ccxName, m_levelset->levelCount()))
        m_haveCcx = true;

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionCloseLevelset]->setEnabled(true);
    m_actions[ActionProperties]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionAddLevel]->setEnabled(true);
    m_actions[ActionOrganize]->setEnabled(true);
    m_actions[ActionCheckErrors]->setEnabled(true);

    QSettings settings;
    addRecentFile(settings, filename);
    populateRecentFiles();
}

void CCEditMain::doLevelsetLoad()
{
    // This can be used to refresh the level display as well as load a new
    // set, so we should only adjust existing items rather than re-adding
    // the whole list.  This will preserve selection states and scroll
    // position, which in turn makes level re-ordering faster while
    // maintaining correctness.

    while (m_levelList->count() < m_levelset->levelCount()) {
        // Add items to match new count
        m_levelList->addItem(QString());
    }
    while (m_levelList->count() > m_levelset->levelCount()) {
        // Remove extra items from list
        delete m_levelList->takeItem(m_levelList->count() - 1);
    }
    for (int i = 0; i < m_levelset->levelCount(); ++i) {
        // Use iterator for level number since stored level numbers may
        // be meaningless until saved...
        m_levelList->item(i)->setText(QStringLiteral("%1 - %2").arg(i + 1)
                .arg(ccl::fromLatin1(m_levelset->level(i)->name())));
    }
    if (!m_levelList->currentItem() && m_levelset->levelCount() > 0)
        m_levelList->setCurrentRow(0);
    m_levelManDock->raise();
}

void CCEditMain::setLevelsetFilename(const QString& filename)
{
    m_levelsetFilename = filename;
    QString displayName = filename.isEmpty() ? tr("Untitled")
                        : QFileInfo(filename).fileName();
    if (m_dirtyFlag)
        setWindowTitle(s_appTitle + QStringLiteral(" - ") + displayName + QStringLiteral(" *"));
    else
        setWindowTitle(s_appTitle + QStringLiteral(" - ") + displayName);
}

void CCEditMain::populateRecentFiles()
{
    m_recentFiles->clear();

    QSettings settings;
    QStringList recent = recentFiles(settings);
    for (const QString& path : recent) {
        QFileInfo info(path);
        const QString label = QStringLiteral("%1 [%2]").arg(info.fileName(), info.absolutePath());
        auto recentFileAction = m_recentFiles->addAction(label);
        connect(recentFileAction, &QAction::triggered, this, [this, path] {
            loadLevelset(path);
        });
    }

    m_recentFiles->addSeparator();
    auto clearListAction = m_recentFiles->addAction(tr("Clear List"));
    connect(clearListAction, &QAction::triggered, this, [this] {
        QSettings settings;
        clearRecentFiles(settings);
        populateRecentFiles();
    });
}

void CCEditMain::saveLevelset(const QString& filename)
{
    if (!m_levelset)
        return;

    if (m_useDac) {
        ccl::unique_FILE dac = ccl::FileStream::Fopen(filename, ccl::FileStream::WriteText);
        if (dac) {
            try {
                m_dacInfo.write(dac.get());
            } catch (const ccl::RuntimeError& e) {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error saving levelset descriptor: %1")
                                      .arg(e.message()));
                return;
            }

            QDir searchPath(filename);
            searchPath.cdUp();

            ccl::FileStream set;
            if (set.open(searchPath.absoluteFilePath(m_dacInfo.m_filename), ccl::FileStream::Write)) {
                try {
                    m_levelset->write(&set);
                } catch (const ccl::RuntimeError& e) {
                    QMessageBox::critical(this, tr("Error saving levelset"),
                                          tr("Error saving levelset: %1").arg(e.message()));
                    return;
                }
            } else {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error: could not write file %1")
                                      .arg(m_dacInfo.m_filename));
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error saving levelset"),
                                  tr("Error: could not write file %1").arg(filename));
            return;
        }
    } else {
        ccl::FileStream set;
        if (set.open(filename, ccl::FileStream::Write)) {
            try {
                m_levelset->write(&set);
            } catch (const ccl::RuntimeError& e) {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error saving levelset: %1").arg(e.message()));
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error saving levelset"),
                                  tr("Error: could not write file %1").arg(filename));
            return;
        }
    }
    setLevelsetFilename(filename);
    m_checkSave = true;
    m_undoStack->setClean();
    m_dirtyFlag = 0;

    QSettings settings;
    addRecentFile(settings, filename);
    populateRecentFiles();
}

void CCEditMain::closeEvent(QCloseEvent* event)
{
    if (!closeLevelset()) {
        event->setAccepted(false);
        return;
    }

    if (m_subProc) {
        // Don't handle events after we're exiting.
        // Note that MSCC temp file cleanup will not take place if this happens!
        m_subProc->disconnect();
    }

    QSettings settings;
    settings.setValue(QStringLiteral("WindowMaximized"),
                      (windowState() & Qt::WindowMaximized) != 0);
    showNormal();
    settings.setValue(QStringLiteral("WindowSize"), size());
    settings.setValue(QStringLiteral("WindowState2"), saveState());
    settings.setValue(QStringLiteral("ZoomFactor"), m_zoomFactor);
    settings.setValue(QStringLiteral("ViewButtons"),
                      m_actions[ActionViewButtons]->isChecked());
    settings.setValue(QStringLiteral("ViewMovers"),
                      m_actions[ActionViewMovers]->isChecked());
    settings.setValue(QStringLiteral("ViewCloners"),
                      m_actions[ActionViewCloners]->isChecked());
    settings.setValue(QStringLiteral("ViewTraps"),
                      m_actions[ActionViewTraps]->isChecked());
    settings.setValue(QStringLiteral("ViewDRClones"),
                      m_actions[ActionViewDRCloners]->isChecked());
    settings.setValue(QStringLiteral("ViewMultiTanks"),
                      m_actions[ActionViewMultiTanks]->isChecked());
    settings.setValue(QStringLiteral("ViewActivePlayer"),
                      m_actions[ActionViewActivePlayer]->isChecked());
    settings.setValue(QStringLiteral("ViewViewport"),
                      m_actions[ActionViewViewport]->isChecked());
    settings.setValue(QStringLiteral("ViewMonsterPaths"),
                      m_actions[ActionViewMonsterPaths]->isChecked());
    settings.setValue(QStringLiteral("ViewErrors"),
                      m_actions[ActionViewErrors]->isChecked());
    settings.setValue(QStringLiteral("ViewButtonsOnMouse"),
                        m_actions[ActionViewButtonsOnMouse]->isChecked());
}

void CCEditMain::resizeEvent(QResizeEvent* event)
{
    if (!event)
        QWidget::resizeEvent(event);

    if (m_zoomFactor == 0.0 && m_editorTabs->currentWidget() != nullptr) {
        QSize zmax = ((QScrollArea*)m_editorTabs->currentWidget())->maximumViewportSize();
        double zx = (double)zmax.width() / (32 * m_currentTileset->size());
        double zy = (double)zmax.height() / (32 * m_currentTileset->size());
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setZoom(std::min(zx, zy));
    }
}

void CCEditMain::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Shift) {
        EditorWidget* editor = currentEditor();
        if (editor) {
            editor->setPaintFlag(EditorWidget::RevealLower);
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void CCEditMain::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Shift) {
        EditorWidget* editor = currentEditor();
        if (editor) {
            editor->clearPaintFlag(EditorWidget::RevealLower);
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

bool CCEditMain::closeLevelset()
{
    if (!m_levelset)
        return true;

    int reply = m_dirtyFlag
              ? QMessageBox::question(this, tr("Close levelset"),
                        tr("Save changes to %1 before closing?")
                        .arg(m_levelsetFilename.isEmpty() ? tr("new levelset") : m_levelsetFilename),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)
              : QMessageBox::No;
    if (reply == QMessageBox::Cancel) {
        return false;
    } else if (reply == QMessageBox::Yes) {
        m_checkSave = false;
        onSaveAction();
        if (!m_checkSave)
            return false;
    }

    closeAllTabs();
    m_undoStack->clear();
    delete m_undoCommand;
    m_undoCommand = nullptr;
    m_levelList->clear();
    delete m_levelset;
    m_levelset = nullptr;
    m_dirtyFlag = 0;
    setWindowTitle(s_appTitle);

    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionCloseLevelset]->setEnabled(false);
    m_actions[ActionProperties]->setEnabled(false);
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionAddLevel]->setEnabled(false);
    m_actions[ActionOrganize]->setEnabled(false);
    m_actions[ActionCheckErrors]->setEnabled(false);

    return true;
}

void CCEditMain::loadTileset(CCETileset* tileset)
{
    QSettings settings;
    const int tilesetScale = settings.value(QStringLiteral("TilesetScale"), 1).toInt();
    m_currentTileset = tileset;
    m_currentTileset->setUiScale(qreal(tilesetScale));
    if (m_zoomFactor != 0.0)
        setZoomFactor(m_zoomFactor);

    emit tilesetChanged(tileset);
    resizeEvent(nullptr);
}

void CCEditMain::registerTileset(const QString& filename)
{
    auto tileset = new CCETileset(this);
    bool valid = false;
    try {
        valid = tileset->load(filename);
    } catch (const ccl::RuntimeError& err) {
        qDebug("Error registering tileset %s: %s", qPrintable(filename),
               qPrintable(err.message()));
        valid = false;
    }
    if (!valid) {
        delete tileset;
        return;
    }

    QAction* menuItem = m_tilesetMenu->addAction(tileset->name());
    menuItem->setCheckable(true);
    menuItem->setStatusTip(tileset->description());
    menuItem->setData(QVariant::fromValue(tileset));
    m_tilesetGroup->addAction(menuItem);
}

void CCEditMain::populateTilesets()
{
    m_tilesetMenu->clear();

    QDir path;
    QStringList tilesets;
    const QStringList tilesetGlob { QStringLiteral("*.tis") };
#if defined(Q_OS_WIN)
    // Search app directory
    path.setPath(QApplication::applicationDirPath());
    for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
        tilesets << path.absoluteFilePath(file);
#else
    // Search install path
    path.setPath(QApplication::applicationDirPath());
    path.cdUp();
    path.cd(QStringLiteral("share/cctools"));
    for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
        tilesets << path.absoluteFilePath(file);

    // Search standard directories
    path.setPath(QStringLiteral("/usr/share/cctools"));
    if (path.exists()) {
        for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
            tilesets << path.absoluteFilePath(file);
    }
    path.setPath(QStringLiteral("/usr/local/share/cctools"));
    if (path.exists()) {
        for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
            tilesets << path.absoluteFilePath(file);
    }
#endif

    // User-space local data
    path.setPath(QDir::homePath());
    path.cd(QStringLiteral(".cctools"));
    if (path.exists()) {
        for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
            tilesets << path.absoluteFilePath(file);
    }

    tilesets.removeDuplicates();
    for (const QString& file : tilesets)
        registerTileset(file);

    m_tilesetMenu->addSeparator();
    for (QAction* scaleAction : m_scaleGroup->actions())
        m_tilesetMenu->addAction(scaleAction);
}

void CCEditMain::loadLevel(int levelNum)
{
    if (!m_levelset)
        return;
    if (levelNum < 0 || levelNum >= m_levelset->levelCount())
        return;

    loadLevel(m_levelset->level(levelNum));
}

void CCEditMain::loadLevel(ccl::LevelData* levelPtr)
{
    if (!m_levelset)
        return;

    for (int i = 0; i < m_editorTabs->count(); ++i) {
        if (getEditorAt(i)->levelData() == levelPtr) {
            if (m_editorTabs->currentIndex() == i)
                onTabChanged(i);
            else
                m_editorTabs->setCurrentIndex(i);
            return;
        }
    }
    addEditor(levelPtr);
}

int CCEditMain::levelIndex(ccl::LevelData* level)
{
    if (!m_levelset || !level)
        return -1;

    for (int i = 0; i < m_levelset->levelCount(); ++i) {
        if (m_levelset->level(i) == level)
            return i;
    }
    return -1;
}

EditorWidget* CCEditMain::getEditorAt(int idx)
{
    if (idx < 0 || idx >= m_editorTabs->count())
        return nullptr;

    auto scroll = qobject_cast<QScrollArea*>(m_editorTabs->widget(idx));
    return qobject_cast<EditorWidget*>(scroll->widget());
}

EditorWidget* CCEditMain::currentEditor()
{
    return getEditorAt(m_editorTabs->currentIndex());
}

EditorWidget* CCEditMain::addEditor(ccl::LevelData* level)
{
    auto scroll = new QScrollArea(m_editorTabs);
    auto editor = new EditorWidget(scroll);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidget(editor);
    if (m_actions[ActionViewButtons]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowButtons);
    if (m_actions[ActionViewMovers]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowMovement);
    if (m_actions[ActionViewCloners]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowCloneNumbers);
    if (m_actions[ActionViewTraps]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowTrapNumbers);
    if (m_actions[ActionViewDRCloners]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowDRCloneButtons);
    if (m_actions[ActionViewMultiTanks]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowMultiTankLocations);
    if (m_actions[ActionViewActivePlayer]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowPlayer);
    if (m_actions[ActionViewViewport]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowViewBox);
    if (m_actions[ActionViewMonsterPaths]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowMovePaths);
    if (m_actions[ActionViewErrors]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowErrors);
    if (m_actions[ActionViewButtonsOnMouse]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowConnectionsOnMouse);
    editor->setDrawMode(m_currentDrawMode);
    editor->setTileset(m_currentTileset);
    editor->setLevelData(level);
    editor->setLeftTile(m_leftTile);
    editor->setRightTile(m_rightTile);
    if (m_zoomFactor != 0.0)
        editor->setZoom(m_zoomFactor * m_currentTileset->uiScale());
    m_editorTabs->addFloatingTab(scroll, ccl::fromLatin1(level->name()));
    resizeEvent(nullptr);

    connect(editor, &EditorWidget::mouseInfo, statusBar(), &QStatusBar::showMessage);
    connect(editor, &EditorWidget::hasSelection, m_actions[ActionCut], &QAction::setEnabled);
    connect(editor, &EditorWidget::hasSelection, m_actions[ActionCopy], &QAction::setEnabled);
    connect(editor, &EditorWidget::hasSelection, m_actions[ActionClear], &QAction::setEnabled);
    connect(editor, &EditorWidget::tilePicked, this, &CCEditMain::onTilePicked);
    connect(editor, &EditorWidget::editingStarted, this, [this] {
        beginEdit(CCEditHistory::EditMap);
    });
    connect(editor, &EditorWidget::editingFinished, this, &CCEditMain::endEdit);
    connect(editor, &EditorWidget::editingCancelled, this, &CCEditMain::cancelEdit);
    connect(this, &CCEditMain::tilesetChanged, editor, &EditorWidget::setTileset);
    connect(this, &CCEditMain::leftTileChanged, editor, &EditorWidget::setLeftTile);
    connect(this, &CCEditMain::rightTileChanged, editor, &EditorWidget::setRightTile);

    m_editorTabs->setCurrentWidget(scroll);
    return editor;
}

void CCEditMain::closeAllTabs()
{
    while (getEditorAt(0)) {
        delete m_editorTabs->widget(0);
        m_editorTabs->removeTab(0);
    }
}

void CCEditMain::createNewLevelset()
{
    if (!closeLevelset())
        return;

    m_levelset = new ccl::Levelset();
    doLevelsetLoad();
    setLevelsetFilename(QString());
    m_useDac = false;

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionCloseLevelset]->setEnabled(true);
    m_actions[ActionProperties]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionAddLevel]->setEnabled(true);
    m_actions[ActionOrganize]->setEnabled(true);
    m_actions[ActionCheckErrors]->setEnabled(true);
}

void CCEditMain::onOpenAction()
{
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Levelset..."),
                            settings.value(QStringLiteral("DialogDir")).toString(),
                            tr("All Levelsets (*.dat *.DAT *.dac *.ccl)"));
    if (!filename.isEmpty()) {
        loadLevelset(filename);
        settings.setValue(QStringLiteral("DialogDir"),
                          QFileInfo(filename).dir().absolutePath());
    }
}

void CCEditMain::onSaveAction()
{
    if (m_levelsetFilename.isEmpty())
        onSaveAsAction();
    else
        saveLevelset(m_levelsetFilename);
}

void CCEditMain::onSaveAsAction()
{
    QSettings settings;
    QString filter = m_useDac ? tr("TWorld Levelsets (*.dac)")
                              : tr("CC Levelsets (*.dat *.DAT *.ccl)");

    if (m_levelsetFilename.isEmpty())
        m_levelsetFilename = settings.value(QStringLiteral("DialogDir")).toString();
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Levelset..."),
                                                    m_levelsetFilename, filter);
    if (!filename.isEmpty()) {
        saveLevelset(filename);
        settings.setValue(QStringLiteral("DialogDir"),
                          QFileInfo(filename).dir().absolutePath());
    }
}

void CCEditMain::onReportAction()
{
    QSettings settings;
    QString reportPath = m_levelsetFilename.isEmpty()
                ? settings.value(QStringLiteral("DialogDir")).toString()
                : m_levelsetFilename.left(m_levelsetFilename.lastIndexOf(QLatin1Char('.'))) + QStringLiteral(".html");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Report..."),
                                                    reportPath, tr("HTML Source (*.html)"));
    if (filename.isEmpty())
        return;

    QProgressDialog proDlg(this);
    proDlg.setMaximum(m_levelset->levelCount() + 1);
    proDlg.setMinimumDuration(2000);
    proDlg.setLabelText(tr("Generating HTML report.  Please be patient..."));

    QElapsedTimer timer;
    timer.start();

    // Generate HTML output
    QFile report(filename);
    if (!report.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error creating report"),
                tr("Error creating report: Could not write HTML report"));
        return;
    }
    const QString filebase = m_levelsetFilename.isEmpty() ? tr("Untitled.ccl")
                             : QFileInfo(m_levelsetFilename).fileName();
    report.write("<html>\n<head><title>Levelset Report for ");
    report.write(filebase.toUtf8().constData());
    report.write("</title></head>\n\n");
    report.write("<body>\n<h1 align=\"center\">Levelset Report for ");
    report.write(filebase.toUtf8().constData());
    report.write("</h1>\n");

    const QString dirbase = QFileInfo(filename).baseName() + QStringLiteral("_levimg");
    const QString imgdir = QFileInfo(filename).path() + QDir::separator() + dirbase;
    QDir dir;
    dir.mkdir(imgdir);
    if (!dir.cd(imgdir)) {
        QMessageBox::critical(this, tr("Error creating report"),
                              tr("Error creating report: Could not create level image folder"));
        return;
    }

    for (int i = 0; i < m_levelset->levelCount(); ++i) {
        ccl::LevelData* level = m_levelset->level(i);
        report.write("<hr />\n<h2>Level ");
        report.write(QString::number(i + 1).toUtf8().constData());
        report.write("</h2>\n<pre>\n");
        report.write("<b>Title:</b>    ");
        report.write(level->name().c_str());
        report.write("\n<b>Chips:</b>    ");
        report.write(QString::number(level->chips()).toUtf8().constData());
        report.write("\n<b>Time:</b>     ");
        report.write(QString::number(level->timer()).toUtf8().constData());
        report.write("\n<b>Password:</b> ");
        report.write(level->password().c_str());
        report.write("\n<b>Hint:</b>     ");
        report.write(level->hint().c_str());
        report.write("\n</pre>\n<img src=\"");
        report.write(QStringLiteral("%1/level%2.png").arg(dirbase).arg(i + 1).toUtf8().constData());
        report.write("\" />\n");
        report.write("<p style=\"page-break-after: always;\">&nbsp;</p>\n");

        // Write the level image
        EditorWidget reportDummy;
        reportDummy.setVisible(false);
        reportDummy.setTileset(m_currentTileset);
        reportDummy.setLevelData(level);
        QImage levelImage = reportDummy.renderReport();
        levelImage.save(QStringLiteral("%1/level%2.png").arg(imgdir).arg(i + 1), "PNG");

        proDlg.setValue(i + 1);
        QApplication::processEvents();
        if (proDlg.wasCanceled())
            return;
    }
    report.write("</body>\n</html>\n");

    QMessageBox::information(this, tr("Report Complete"),
            tr("Generated report in %1 sec")
            .arg(timer.elapsed() / 1000., 0, 'f', 2));
}

static void uncheckAll(QActionGroup* group, QAction* exceptFor = nullptr)
{
    for (QAction* action : group->actions()) {
        if (action != exceptFor)
            action->setChecked(false);
    }
}

void CCEditMain::onSelectToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawSelect) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawSelect;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionSelect]);

        for (int i = 0; i < m_editorTabs->count(); ++i)
            getEditorAt(i)->setDrawMode(m_currentDrawMode);
    }
}

void CCEditMain::onCutAction()
{
    onCopyAction();
    onClearAction();
}

void CCEditMain::onCopyAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor || editor->selection() == QRect(-1, -1, -1, -1))
        return;

    const QRect selection = editor->selection();
    ccl::ClipboardData cbData(selection.width(), selection.height());
    ccl::LevelData* copyRegion = cbData.levelData();
    ccl::LevelData* current = editor->levelData();
    copyRegion->map().copyFrom(editor->levelData()->map(),
                               selection.x(), selection.y(), 0, 0,
                               selection.width(), selection.height());

    // Gather any mechanics that are completely encompassed by this region
    for (const ccl::Trap& trap_iter : current->traps()) {
        if (selection.contains(trap_iter.button.X, trap_iter.button.Y)
            && selection.contains(trap_iter.trap.X, trap_iter.trap.Y))
            copyRegion->trapConnect(trap_iter.button.X - selection.x(),
                                    trap_iter.button.Y - selection.y(),
                                    trap_iter.trap.X - selection.x(),
                                    trap_iter.trap.Y - selection.y());
    }

    for (const ccl::Clone& clone_iter : current->clones()) {
        if (selection.contains(clone_iter.button.X, clone_iter.button.Y)
            && selection.contains(clone_iter.clone.X, clone_iter.clone.Y))
            copyRegion->cloneConnect(clone_iter.button.X - selection.x(),
                                     clone_iter.button.Y - selection.y(),
                                     clone_iter.clone.X - selection.x(),
                                     clone_iter.clone.Y - selection.y());
    }

    for (const ccl::Point& move_iter : current->moveList()) {
        if (selection.contains(move_iter.X, move_iter.Y))
            copyRegion->addMover(move_iter.X - selection.x(),
                                 move_iter.Y - selection.y());
    }

    try {
        ccl::BufferStream cbStream;
        cbData.write(&cbStream);
        QByteArray buffer((const char*)cbStream.buffer(), cbStream.size());

        auto copyData = new QMimeData;
        copyData->setData(s_clipboardFormat, buffer);
        copyData->setImageData(editor->renderSelection());
        QApplication::clipboard()->setMimeData(copyData);
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error"),
                tr("Error saving clipboard data: %1").arg(e.message()),
                QMessageBox::Ok);
    }
}

void CCEditMain::onPasteAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    const QMimeData* pasteData = QApplication::clipboard()->mimeData();
    if (pasteData->hasFormat(s_clipboardFormat)) {
        QByteArray buffer = pasteData->data(s_clipboardFormat);
        ccl::BufferStream cbStream;
        cbStream.setFrom(buffer.constData(), buffer.size());

        ccl::ClipboardData cbData;
        try {
            cbData.read(&cbStream);
        } catch (const ccl::RuntimeError& e) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Error parsing clipboard data: %1").arg(e.message()),
                    QMessageBox::Ok);
            return;
        }

        int destX, destY;
        if (editor->selection() == QRect(-1, -1, -1, -1)) {
            destX = 0;
            destY = 0;
        } else {
            destX = editor->selection().left();
            destY = editor->selection().top();
        }
        const int width = std::min(cbData.width(), CCL_WIDTH - destX);
        const int height = std::min(cbData.height(), CCL_HEIGHT - destY);

        beginEdit(CCEditHistory::EditMap);
        editor->selectRegion(destX, destY, width, height);
        onClearAction();
        ccl::LevelData* current = editor->levelData();
        ccl::LevelData* copyRegion = cbData.levelData();
        current->map().copyFrom(copyRegion->map(), 0, 0, destX, destY, width, height);

        for (const ccl::Trap& trap_iter : copyRegion->traps()) {
            if (current->traps().size() < MAX_TRAPS && trap_iter.button.X + destX < CCL_WIDTH
                && trap_iter.button.Y + destY < CCL_HEIGHT && trap_iter.trap.X + destX < CCL_WIDTH
                && trap_iter.trap.Y + destY < CCL_HEIGHT)
                current->trapConnect(trap_iter.button.X + destX, trap_iter.button.Y + destY,
                                     trap_iter.trap.X + destX, trap_iter.trap.Y + destY);
        }

        for (const ccl::Clone& clone_iter : copyRegion->clones()) {
            if (current->clones().size() < MAX_CLONES && clone_iter.button.X + destX < CCL_WIDTH
                && clone_iter.button.Y + destY < CCL_HEIGHT && clone_iter.clone.X + destX < CCL_WIDTH
                && clone_iter.clone.Y + destY < CCL_HEIGHT)
                current->cloneConnect(clone_iter.button.X + destX, clone_iter.button.Y + destY,
                                      clone_iter.clone.X + destX, clone_iter.clone.Y + destY);
        }

        for (const ccl::Point& move_iter : copyRegion->moveList()) {
            if (current->moveList().size() < MAX_MOVERS && move_iter.X + destX < CCL_WIDTH
                && move_iter.Y + destY < CCL_HEIGHT)
                current->addMover(move_iter.X + destX, move_iter.Y + destY);
        }

        endEdit();
    }
}

void CCEditMain::onClearAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor || editor->selection() == QRect(-1, -1, -1, -1))
        return;

    beginEdit(CCEditHistory::EditMap);
    for (int y = editor->selection().top(); y <= editor->selection().bottom(); ++y) {
        for (int x = editor->selection().left(); x <= editor->selection().right(); ++x) {
            editor->putTile(ccl::TileFloor, x, y, EditorWidget::LayTop);
            editor->putTile(ccl::TileFloor, x, y, EditorWidget::LayBottom);
        }
    }
    endEdit();
}

void CCEditMain::onUndoAction()
{
    m_undoStack->undo();
    updateForUndoCommand(m_undoStack->command(m_undoStack->index()));
}

void CCEditMain::onRedoAction()
{
    auto command = m_undoStack->command(m_undoStack->index());
    m_undoStack->redo();
    updateForUndoCommand(command);
}

void CCEditMain::updateForUndoCommand(const QUndoCommand* command)
{
    auto editorCmd = dynamic_cast<const EditorUndoCommand*>(command);
    if (editorCmd) {
        for (int i = 0; i < m_editorTabs->count(); ++i) {
            auto editor = getEditorAt(i);
            if (editor->levelData() == editorCmd->levelPtr())
                editor->dirtyBuffer();
        }
        loadLevel(editorCmd->levelPtr());
    } else if (dynamic_cast<const LevelsetUndoCommand*>(command)) {
        doLevelsetLoad();
        m_levelList->setCurrentRow(-1);
    }
}

void CCEditMain::onDrawPencilAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawPencil;
    m_currentDrawMode = EditorWidget::DrawPencil;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawLineAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawLine;
    m_currentDrawMode = EditorWidget::DrawLine;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawRectAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawRect;
    m_currentDrawMode = EditorWidget::DrawRect;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawFillAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawFill;
    m_currentDrawMode = EditorWidget::DrawFill;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawFloodAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawFlood;
    m_currentDrawMode = EditorWidget::DrawFlood;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onPathMakerToggled(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionPathMaker;
    m_currentDrawMode = EditorWidget::DrawPathMaker;
    uncheckAll(m_modalToolGroup);

    for (int i = 0; i < m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onConnectToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawButtonConnect) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawButtonConnect;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionConnect]);

        for (int i = 0; i < m_editorTabs->count(); ++i)
            getEditorAt(i)->setDrawMode(m_currentDrawMode);
    }
}

void CCEditMain::onAdvancedMechAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    AdvancedMechanicsDialog mechDlg(this);
    mechDlg.setFrom(editor->levelData());
    beginEdit(CCEditHistory::EditMap);
    if (mechDlg.exec() == QDialog::Accepted)
        endEdit();
    else
        cancelEdit();
}

void CCEditMain::onInspectTilesToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawInspectTile) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawInspectTile;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionInspectTiles]);

        for (int i = 0; i < m_editorTabs->count(); ++i)
            getEditorAt(i)->setDrawMode(m_currentDrawMode);
    }
}

void CCEditMain::onToggleWallsAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    beginEdit(CCEditHistory::EditMap);
    ccl::ToggleDoors(editor->levelData());
    endEdit();
}

void CCEditMain::onCheckErrorsAction()
{
    ErrorCheckDialog dlg(this);
    dlg.setLevelsetInfo(m_levelset, &m_dacInfo);
    dlg.exec();
}

void CCEditMain::onViewButtonsToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowButtons);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowButtons);
    }
}

void CCEditMain::onViewMoversToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowMovement);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowMovement);
    }
}

void CCEditMain::onViewClonersToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowCloneNumbers);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowCloneNumbers);
    }
}

void CCEditMain::onViewTrapsToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowTrapNumbers);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowTrapNumbers);
    }
}

void CCEditMain::onViewDRClonersToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowDRCloneButtons);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowDRCloneButtons);
    }
}

void CCEditMain::onViewMultiTanksToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowMultiTankLocations);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowMultiTankLocations);
    }
}

void CCEditMain::onViewActivePlayerToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowPlayer);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowPlayer);
    }
}

void CCEditMain::onViewViewportToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowViewBox);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowViewBox);
    }
}

void CCEditMain::onViewMonsterPathsToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowMovePaths);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowMovePaths);
    }
}

void CCEditMain::onViewErrorsToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowErrors);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowErrors);
    }
}

void CCEditMain::onViewButtonsOnMouseToggled(bool view)
{
    if (view) {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setPaintFlag(EditorWidget::ShowConnectionsOnMouse);
    } else {
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->clearPaintFlag(EditorWidget::ShowConnectionsOnMouse);
    }
}

void CCEditMain::setZoomFactor(double zoom)
{
    m_zoomFactor = zoom;
    const double adjustedZoom = zoom * m_currentTileset->uiScale();
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(adjustedZoom);
}

void CCEditMain::onZoomCust()
{
    bool ok;
    double zoom = QInputDialog::getDouble(this, tr("Set Custom Zoom"),
                        tr("Custom Zoom Percentage"), m_zoomFactor * 100.0,
                        2.5, 800.0, 2, &ok);
    if (ok)
        setZoomFactor(zoom / 100.0);
    if (m_zoomFactor == 1.0)
        m_actions[ActionZoom100]->setChecked(true);
    else if (m_zoomFactor == 2.0)
        m_actions[ActionZoom200]->setChecked(true);
    else if (m_zoomFactor == 1.5)
        m_actions[ActionZoom150]->setChecked(true);
    else if (m_zoomFactor == 0.75)
        m_actions[ActionZoom75]->setChecked(true);
    else if (m_zoomFactor == 0.5)
        m_actions[ActionZoom50]->setChecked(true);
    else if (m_zoomFactor == 0.25)
        m_actions[ActionZoom25]->setChecked(true);
    else if (m_zoomFactor == 0.125)
        m_actions[ActionZoom125]->setChecked(true);
}

void CCEditMain::onZoomFit()
{
    m_zoomFactor = 0.0;
    resizeEvent(nullptr);
}

void CCEditMain::onTilesetMenu(QAction* which)
{
    auto tileset = which->data().value<CCETileset*>();
    loadTileset(tileset);

    QSettings settings;
    settings.setValue(QStringLiteral("TilesetName"), m_currentTileset->filename());
}

void CCEditMain::onTestChips()
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;
    const int levelNum = levelIndex(editor->levelData());
    if (levelNum < 0)
        return;

    if (m_subProc) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CCEdit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings;
    QString chipsExe = settings.value(QStringLiteral("ChipsExe")).toString();
    if (chipsExe.isEmpty() || !QFile::exists(chipsExe)) {
        QMessageBox::critical(this, tr("Could not find CHIPS.EXE"),
                tr("Could not find Chip's Challenge executable.\n"
                   "Please configure MSCC in the Test Setup dialog."));
        return;
    }

    QString winePath = settings.value(QStringLiteral("WineExe")).toString();
#ifndef Q_OS_WIN
    if (winePath.isEmpty() || !QFile::exists(winePath)) {
        // Try standard paths
        winePath = QStandardPaths::findExecutable(QStringLiteral("wine"));
        if (winePath.isEmpty() || !QFile::exists(winePath)) {
            QMessageBox::critical(this, tr("Could not find WINE"),
                    tr("Could not find WINE executable.\n"
                       "Please configure WINE in the Test Setup dialog."));
            return;
        }
    }
#endif

    m_tempExe = QDir::tempPath() + QStringLiteral("/CCRun.exe");
    m_tempDat = QDir::tempPath() + QStringLiteral("/CCRun.dat");
    QFile::remove(m_tempExe);
    if (!QFile::copy(chipsExe, m_tempExe)) {
        QMessageBox::critical(this, tr("Error Creating Test EXE"),
                tr("Error copying %1 to temp path").arg(chipsExe));
        return;
    }
    ccl::FileStream stream;
    if (!stream.open(m_tempExe, ccl::FileStream::ReadWrite)) {
        QMessageBox::critical(this, tr("Error Creating Test EXE"),
                tr("Error opening %1 for writing").arg(m_tempExe));
        return;
    }

    // Make a CHIPS.EXE that we can use
    ccl::ChipsHax hax;
    hax.open(&stream);
    if (settings.value(QStringLiteral("TestPGPatch"), false).toBool())
        hax.set_PGChips(ccl::CCPatchPatched);
    if (settings.value(QStringLiteral("TestCCPatch"), true).toBool())
        hax.set_CCPatch(ccl::CCPatchPatched);
    hax.set_LastLevel(m_levelset->levelCount());
    if (m_useDac && m_dacInfo.m_lastLevel < m_levelset->levelCount())
        hax.set_FakeLastLevel(m_dacInfo.m_lastLevel);
    else
        hax.set_FakeLastLevel(m_levelset->levelCount());
    hax.set_IgnorePasswords(true);
    hax.set_DataFilename("CCRun.dat");
    hax.set_IniFilename(".\\CCRun.ini");
    hax.set_IniEntryName("CCEdit Playtest");
    hax.set_DialogTitle("CCEdit Playtest");
    hax.set_WindowTitle("CCEdit Playtest");
    stream.close();

    // Save the levelset to the temp file
    if (!stream.open(m_tempDat, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error opening %1 for writing").arg(m_tempDat));
        return;
    }
    unsigned int saveType = m_levelset->type();
    if (settings.value(QStringLiteral("TestPGPatch"), false).toBool())
        m_levelset->setType(ccl::Levelset::TypePG);
    else
        m_levelset->setType(ccl::Levelset::TypeMS);
    try {
        m_levelset->write(&stream);
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.message()));
        m_levelset->setType(saveType);
        stream.close();
        return;
    }
    m_levelset->setType(saveType);
    stream.close();

    // Configure the INI file
    QString cwd = QDir::currentPath();
    QDir exePath = QFileInfo(chipsExe).absoluteDir();

    m_tempIni = exePath.absoluteFilePath(QStringLiteral("CCRun.ini"));
    ccl::unique_FILE iniStream = ccl::FileStream::Fopen(m_tempIni, ccl::FileStream::ReadWriteText);
    if (!iniStream)
        iniStream = ccl::FileStream::Fopen(m_tempIni, ccl::FileStream::RWCreateText);
    if (!iniStream) {
        QMessageBox::critical(this, tr("Error Creating CCRun.ini"),
                tr("Error: Could not open or create CCRun.ini file"));
        QFile::remove(m_tempExe);
        QFile::remove(m_tempDat);
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream.get());
        ini.setSection("CCEdit Playtest");
        ini.setInt("Current Level", levelNum + 1);
        ini.setString(ccl::toLatin1(QStringLiteral("Level%1").arg(levelNum + 1)),
                      editor->levelData()->password());
        ini.write(iniStream.get());
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error writing CCRun.ini"),
                tr("Error writing INI file: %1").arg(e.message()));
        QFile::remove(m_tempExe);
        QFile::remove(m_tempDat);
        QFile::remove(m_tempIni);
        return;
    }
    iniStream.reset();      // Force close of the file

    QDir::setCurrent(exePath.absolutePath());
    m_subProc = new QProcess(this);
    m_subProcType = SubprocMSCC;
    connect(m_subProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CCEditMain::onProcessFinished);
    connect(m_subProc, &QProcess::errorOccurred, this, &CCEditMain::onProcessError);
    connect(m_subProc, &QProcess::readyReadStandardOutput, [this] {
        fputs(m_subProc->readAllStandardOutput().data(), stdout);
    });
    connect(m_subProc, &QProcess::readyReadStandardError, [this] {
        fputs(m_subProc->readAllStandardError().data(), stderr);
    });
    if (!winePath.isEmpty()) {
        // Launch with Wine (Unix) or WineVDM (Windows)
        m_subProc->start(winePath, QStringList{ m_tempExe });
    } else {
        // Native execution
        m_subProc->start(m_tempExe, QStringList());
    }
    QDir::setCurrent(cwd);
}

void CCEditMain::onTestTWorld(unsigned int levelsetType)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;
    const int levelNum = levelIndex(editor->levelData());
    if (levelNum < 0)
        return;

    if (m_subProc) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CCEdit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings;
    QString tworldExe = settings.value(QStringLiteral("TWorldExe")).toString();
    if (tworldExe.isEmpty() || !QFile::exists(tworldExe)) {
        // Try standard paths
        tworldExe = QStandardPaths::findExecutable(QStringLiteral("tworld2"));
        if (tworldExe.isEmpty() || !QFile::exists(tworldExe))
            tworldExe = QStandardPaths::findExecutable(QStringLiteral("tworld"));
        if (tworldExe.isEmpty() || !QFile::exists(tworldExe)) {
            QMessageBox::critical(this, tr("Could not find Tile World"),
                    tr("Could not find Tile World executable.\n"
                       "Please configure Tile World in the Test Setup dialog."));
            return;
        }
    }

    // Save the levelset to the temp file
    m_tempDat = QDir::toNativeSeparators(QDir::tempPath() + QStringLiteral("/CCRun.dat"));
    ccl::FileStream stream;
    if (!stream.open(m_tempDat, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error opening %1 for writing").arg(m_tempDat));
        return;
    }
    unsigned int saveType = m_levelset->type();
    m_levelset->setType(levelsetType);
    try {
        m_levelset->write(&stream);
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.message()));
        m_levelset->setType(saveType);
        stream.close();
        return;
    }
    m_levelset->setType(saveType);
    stream.close();

    QString cwd = QDir::currentPath();
    QDir exePath = tworldExe;
    exePath.cdUp();
    QDir::setCurrent(exePath.absolutePath());
    m_subProc = new QProcess(this);
    m_subProcType = SubprocTWorld;
    connect(m_subProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CCEditMain::onProcessFinished);
    connect(m_subProc, &QProcess::errorOccurred, this, &CCEditMain::onProcessError);
    connect(m_subProc, &QProcess::readyReadStandardOutput, [this] {
        fputs(m_subProc->readAllStandardOutput().data(), stdout);
    });
    connect(m_subProc, &QProcess::readyReadStandardError, [this] {
        fputs(m_subProc->readAllStandardError().data(), stderr);
    });
    m_subProc->start(tworldExe, QStringList{
        QStringLiteral("-pr"), m_tempDat, QString::number(levelNum + 1)
    });
    QDir::setCurrent(cwd);
}

void CCEditMain::onTestLexy()
{
    auto editor = currentEditor();
    if (!editor)
        return;

    // Thanks @eevee for making this easy :)
    ccl::BufferStream bs;
    try {
        // Generate a fake levelset...
        bs.write32(m_levelset->type());
        bs.write16(1);
        editor->levelData()->write(&bs);
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to write level data to memory buffer: %1")
                              .arg(err.message()));
        return;
    }
    QByteArray buffer(reinterpret_cast<const char *>(bs.buffer()), bs.size());
    QByteArray zdata = qCompress(buffer);
    zdata.remove(0, 4);     // Strip Qt's custom size header...
    QByteArray b64data = zdata.toBase64(QByteArray::Base64UrlEncoding);

    QSettings settings;
    QString lexyUrl = settings.value(QStringLiteral("LexyUrl"),
                                     DEFAULT_LEXY_URL).toString();

    QDesktopServices::openUrl(QUrl(lexyUrl + QStringLiteral("?level=")
                                   + QString::fromLatin1(b64data)));
}

void CCEditMain::onTilePicked(int x, int y)
{
    EditorWidget* editor = currentEditor();
    Q_ASSERT(editor);

    if (m_currentDrawMode == EditorWidget::DrawInspectTile) {
        TileInspector inspector(this);
        inspector.setTileset(m_currentTileset);
        ccl::LevelMap& map = editor->levelData()->map();
        inspector.setTile(map.getFG(x, y), map.getBG(x, y));
        if (inspector.exec() == QDialog::Accepted) {
            beginEdit(CCEditHistory::EditMap);
            map.setFG(x, y, inspector.upper());
            map.setBG(x, y, inspector.lower());
            endEdit();
        }
    }
}

void CCEditMain::beginEdit(CCEditHistory::Type type)
{
    EditorWidget* editor = currentEditor();
    Q_ASSERT(editor);

    if (m_undoCommand)
        m_undoCommand->enter();
    else
        m_undoCommand = new EditorUndoCommand(type, editor->levelData());

    // Any type of edit should promote the tab to non-floating status
    m_editorTabs->promoteTab();
}

void CCEditMain::endEdit()
{
    EditorWidget* editor = currentEditor();
    Q_ASSERT(m_undoCommand && editor);

    if (m_undoCommand->leave(editor->levelData())) {
        m_undoStack->push(m_undoCommand);
        m_undoCommand = nullptr;
        editor->dirtyBuffer();
    }
}

void CCEditMain::cancelEdit()
{
    Q_ASSERT(m_undoCommand);

    if (m_undoCommand->leave(nullptr)) {
        delete m_undoCommand;
        m_undoCommand = nullptr;
    }
}

void CCEditMain::onCleanChanged(bool clean)
{
    if (clean)
        m_dirtyFlag &= ~1;
    else
        m_dirtyFlag |= 1;
    setLevelsetFilename(m_levelsetFilename);
}

void CCEditMain::onAddLevelAction()
{
    if (!m_levelset)
        return;

    auto undoCommand = new LevelsetUndoCommand(m_levelset);
    m_levelset->addLevel();
    undoCommand->captureLevelList(m_levelset);
    m_undoStack->push(undoCommand);
    doLevelsetLoad();
    m_levelList->setCurrentRow(m_levelList->count() - 1);
}

void CCEditMain::onDelLevelAction()
{
    if (!m_levelset || m_levelList->currentRow() < 0)
        return;

    auto undoCommand = new LevelsetUndoCommand(m_levelset);
    int idx = m_levelList->currentRow();
    m_levelList->setCurrentRow(idx - 1);
    ccl::LevelData* level = m_levelset->takeLevel(idx);
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        if (getEditorAt(i)->levelData() == level)
            m_editorTabs->widget(i)->deleteLater();
    }
    undoCommand->captureLevelList(m_levelset);
    m_undoStack->push(undoCommand);
    doLevelsetLoad();

    // Checked here for the case where the last level is deleted.
    // The selection changes before the item is removed, so we can have
    // a state where the buttons are enabled erroneously.
    m_actions[ActionMoveDown]->setEnabled(m_levelList->currentRow() < m_levelList->count() - 1);
    m_actions[ActionDelLevel]->setEnabled(m_levelset->levelCount() > 0);
}

void CCEditMain::onMoveUpAction()
{
    if (!m_levelset || m_levelList->currentRow() < 0)
        return;

    int idx = m_levelList->currentRow();
    if (idx > 0) {
        auto undoCommand = new LevelsetUndoCommand(m_levelset);
        ccl::LevelData* level = m_levelset->takeLevel(idx);
        m_levelset->insertLevel(idx - 1, level);
        undoCommand->captureLevelList(m_levelset);
        m_undoStack->push(undoCommand);
        doLevelsetLoad();
        m_levelList->setCurrentRow(idx - 1);
    }
}

void CCEditMain::onMoveDownAction()
{
    if (!m_levelset || m_levelList->currentRow() < 0)
        return;

    int idx = m_levelList->currentRow();
    if (idx < (m_levelList->count() - 1)) {
        auto undoCommand = new LevelsetUndoCommand(m_levelset);
        ccl::LevelData* level = m_levelset->takeLevel(idx);
        m_levelset->insertLevel(idx + 1, level);
        undoCommand->captureLevelList(m_levelset);
        m_undoStack->push(undoCommand);
        doLevelsetLoad();
        m_levelList->setCurrentRow(idx + 1);
    }
}

void CCEditMain::onPropertiesAction()
{
    if (!m_levelset)
        return;

    LevelsetProps props(this);
    props.setLevelset(m_levelset);
    if (m_useDac)
        props.setDacFile(&m_dacInfo);
    while (props.exec() == QDialog::Accepted) {
        if (props.useDac() && props.dacFilename().isEmpty()) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("You must specify a levelset filename when using a DAC file"),
                                  QMessageBox::Ok);
            continue;
        }

        m_levelset->setType(props.levelsetType());
        if (props.useDac()) {
            m_dacInfo.m_filename = props.dacFilename();
            m_dacInfo.m_ruleset = props.dacRuleset();
            m_dacInfo.m_lastLevel = props.lastLevel();
            m_dacInfo.m_usePasswords = props.usePasswords();
        }
        if (props.useDac() != m_useDac) {
            // Fix filename
            QString fnameNoSuffix = m_levelsetFilename.left(m_levelsetFilename.lastIndexOf(QLatin1Char('.')));
            if (!fnameNoSuffix.isEmpty()) {
                m_levelsetFilename = fnameNoSuffix + (props.useDac() ? QStringLiteral(".dac")
                                                                     : QStringLiteral(".dat"));
            }
            setLevelsetFilename(m_levelsetFilename);
        }
        m_useDac = props.useDac();

        // TODO: Make this an undo-able action
        m_dirtyFlag |= 2;
        onCleanChanged(false);
        break;
    }
}

void CCEditMain::onOrganizeAction()
{
    if (!m_levelset)
        return;

    OrganizerDialog dlg(this);
    dlg.setTileset(m_currentTileset);
    dlg.loadLevelset(m_levelset);
    if (dlg.exec() == QDialog::Accepted) {
        auto undoCommand = new LevelsetUndoCommand(m_levelset);
        undoCommand->levelList() = dlg.getLevels();
        m_undoStack->push(undoCommand);
        doLevelsetLoad();
        for (int i = 0; i < m_editorTabs->count(); ++i) {
            if (levelIndex(getEditorAt(i)->levelData()) < 0)
                m_editorTabs->widget(i)->deleteLater();
        }

        // Move the selection in the levelList if there's an open editor
        EditorWidget* editor = currentEditor();
        if (editor)
            m_levelList->setCurrentRow(levelIndex(editor->levelData()));
    }
}

void CCEditMain::onSelectLevel(int idx)
{
    if (!m_levelset || idx < 0) {
        m_actions[ActionMoveUp]->setEnabled(false);
        m_actions[ActionMoveDown]->setEnabled(false);
        m_actions[ActionDelLevel]->setEnabled(false);
    } else {
        m_actions[ActionMoveUp]->setEnabled(idx > 0);
        m_actions[ActionMoveDown]->setEnabled(idx < m_levelList->count() - 1);
        m_actions[ActionDelLevel]->setEnabled(true);
    }
    loadLevel(idx);
}

void CCEditMain::onChipCountAction()
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    const ccl::LevelMap& map = editor->levelData()->map();
    m_levelProperties->countChips(map);
}

void CCEditMain::onNameChanged(const std::string& value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->name() != value) {
        beginEdit(CCEditHistory::EditName);
        level->setName(value);
        endEdit();
    }

    const QString qtTitle = ccl::fromLatin1(value);
    const int levelNum = levelIndex(editor->levelData());
    QListWidgetItem* levelListItem = m_levelList->item(levelNum);
    if (levelListItem)
        levelListItem->setText(QStringLiteral("%1 - %2").arg(levelNum + 1).arg(qtTitle));

    for (int i=0; i<m_editorTabs->count(); ++i) {
        if (getEditorAt(i)->levelData() == level)
            m_editorTabs->setTabText(i, qtTitle);
    }
}

void CCEditMain::onAuthorChanged(const std::string& value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->author() != value) {
        beginEdit(CCEditHistory::EditAuthor);
        level->setAuthor(value);
        endEdit();
    }
}

void CCEditMain::onPasswordChanged(const std::string& value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->password() != value) {
        beginEdit(CCEditHistory::EditPassword);
        level->setPassword(value);
        endEdit();
    }
}

void CCEditMain::onChipsChanged(int value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->chips() != value) {
        beginEdit(CCEditHistory::EditChips);
        level->setChips(value);
        endEdit();
    }
}

void CCEditMain::onTimerChanged(int value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->timer() != value) {
        beginEdit(CCEditHistory::EditTimer);
        level->setTimer(value);
        endEdit();
    }
}

void CCEditMain::onHintChanged(const std::string& value)
{
    EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    ccl::LevelData* level = editor->levelData();
    if (level->hint() != value) {
        beginEdit(CCEditHistory::EditHint);
        level->setHint(value);
        endEdit();
    }
}

void CCEditMain::setLeftTile(tile_t tile)
{
    // Return to normal drawing mode if necessary
    m_actions[m_savedDrawMode]->setChecked(true);

    m_leftTile = tile;
    emit leftTileChanged(tile);
}

void CCEditMain::setRightTile(tile_t tile)
{
    // Return to normal drawing mode if necessary
    m_actions[m_savedDrawMode]->setChecked(true);

    m_rightTile = tile;
    emit rightTileChanged(tile);
}

static bool haveClipboardData()
{
    const QMimeData* cbData = QApplication::clipboard()->mimeData();
    return cbData->hasFormat(s_clipboardFormat);
}

void CCEditMain::onClipboardDataChanged()
{
    m_actions[ActionPaste]->setEnabled(currentEditor() && haveClipboardData());
}

void CCEditMain::onTabChanged(int tabIdx)
{
    m_actions[ActionCloseTab]->setEnabled(tabIdx >= 0);

    EditorWidget* editor = getEditorAt(tabIdx);
    if (!editor) {
        m_levelProperties->setEnabled(false);
        m_levelProperties->clearAll();

        m_actions[ActionCut]->setEnabled(false);
        m_actions[ActionCopy]->setEnabled(false);
        m_actions[ActionPaste]->setEnabled(false);
        m_actions[ActionClear]->setEnabled(false);
        m_actions[ActionAdvancedMech]->setEnabled(false);
        m_actions[ActionToggleWalls]->setEnabled(false);
        m_actions[ActionTestChips]->setEnabled(false);
        m_actions[ActionTestTWorldCC]->setEnabled(false);
        m_actions[ActionTestTWorldLynx]->setEnabled(false);
        m_actions[ActionTestLexy]->setEnabled(false);
        m_drawModeGroup->setEnabled(false);
        m_modalToolGroup->setEnabled(false);
        return;
    }

    ccl::LevelData* level = editor->levelData();
    m_levelList->setCurrentRow(levelIndex(level));
    m_levelProperties->setEnabled(true);
    m_levelProperties->updateLevelProperties(level);

    const bool hasSelection = editor->selection() != QRect(-1, -1, -1, -1);
    m_actions[ActionCut]->setEnabled(hasSelection);
    m_actions[ActionCopy]->setEnabled(hasSelection);
    m_actions[ActionPaste]->setEnabled(haveClipboardData());
    m_actions[ActionClear]->setEnabled(hasSelection);
    m_actions[ActionAdvancedMech]->setEnabled(true);
    m_actions[ActionToggleWalls]->setEnabled(true);
    m_actions[ActionTestChips]->setEnabled(true);
    m_actions[ActionTestTWorldCC]->setEnabled(true);
    m_actions[ActionTestTWorldLynx]->setEnabled(true);
    m_actions[ActionTestLexy]->setEnabled(true);
    m_drawModeGroup->setEnabled(true);
    m_modalToolGroup->setEnabled(true);
}

void CCEditMain::onProcessFinished(int, QProcess::ExitStatus)
{
    // Remove temp files
    if (m_subProcType == SubprocMSCC) {
        QFile::remove(m_tempExe);
        QFile::remove(m_tempIni);
    }
    QFile::remove(m_tempDat);

    // Clean up subproc
    m_subProc->disconnect();
    m_subProc->deleteLater();
    m_subProc = nullptr;
}

void CCEditMain::onProcessError(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart) {
        QMessageBox::critical(this, tr("Error starting process"),
                tr("Error starting test process.  Please check your settings "
                   "and try again"), QMessageBox::Ok);
    }
    onProcessFinished(-1, QProcess::NormalExit);
}

int main(int argc, char* argv[])
{
    // Set random seed for password generation
    srand(time(nullptr));

    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("CCTools"));
    QApplication::setApplicationName(QStringLiteral("CCEdit"));

    QIcon appicon(QStringLiteral(":/icons/boot-48.png"));
    appicon.addFile(QStringLiteral(":/icons/boot-32.png"));
    appicon.addFile(QStringLiteral(":/icons/boot-24.png"));
    appicon.addFile(QStringLiteral(":/icons/boot-16.png"));
    QApplication::setWindowIcon(appicon);

    CCEditMain mainWin;
    mainWin.show();

    QStringList qtArgs = QApplication::arguments();
    if (qtArgs.size() > 1)
        mainWin.loadLevelset(qtArgs[1]);
    if (qtArgs.size() > 2) {
        bool ok = false;
        int levelNum = qtArgs[2].toInt(&ok);
        if (ok)
            mainWin.loadLevel(levelNum - 1);
    }

    return QApplication::exec();
}
