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
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QIntValidator>
#include <QGridLayout>
#include <QScrollArea>
#include <QToolBox>
#include <QToolButton>
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
#include <QMimeData>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "LevelsetProps.h"
#include "Organizer.h"
#include "AdvancedMechanics.h"
#include "TestSetup.h"
#include "ErrorCheck.h"
#include "About.h"
#include "../IniFile.h"
#include "../ChipsHax.h"
#include "../GameLogic.h"

#define CCEDIT_TITLE "CCEdit 2.1"

CCEditMain::CCEditMain(QWidget* parent)
    : QMainWindow(parent), m_currentTileset(0), m_savedDrawMode(ActionDrawPencil),
      m_currentDrawMode(EditorWidget::DrawPencil),  m_levelset(0), m_useDac(false),
      m_subProc(0)
{
    setWindowTitle(CCEDIT_TITLE);
    QIcon appicon(":/icons/boot-48.png");
    appicon.addFile(":/icons/boot-32.png");
    appicon.addFile(":/icons/boot-24.png");
    appicon.addFile(":/icons/boot-16.png");
    setWindowIcon(appicon);
    setDockOptions(QMainWindow::AnimatedDocks);

    // Actions
    m_actions[ActionNew] = new QAction(QIcon(":/res/document-new.png"), tr("&New Levelset..."), this);
    m_actions[ActionNew]->setStatusTip(tr("Create new Levelset"));
    m_actions[ActionNew]->setShortcut(Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(QIcon(":/res/document-open.png"), tr("&Open Levelset..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a levelset from disk"));
    m_actions[ActionOpen]->setShortcut(Qt::CTRL | Qt::Key_O);
    m_actions[ActionSave] = new QAction(QIcon(":/res/document-save.png"), tr("&Save"), this);
    m_actions[ActionSave]->setStatusTip(tr("Save the current levelset to the same file"));
    m_actions[ActionSave]->setShortcut(Qt::CTRL | Qt::Key_S);
    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs] = new QAction(QIcon(":/res/document-save-as.png"), tr("Save &As..."), this);
    m_actions[ActionSaveAs]->setStatusTip(tr("Save the current levelset to a new file or location"));
    m_actions[ActionSaveAs]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionClose] = new QAction(tr("&Close Levelset"), this);
    m_actions[ActionClose]->setStatusTip(tr("Close the currently open levelset"));
    m_actions[ActionClose]->setShortcut(Qt::CTRL | Qt::Key_W);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport] = new QAction(tr("Generate &Report"), this);
    m_actions[ActionGenReport]->setStatusTip(tr("Generate an HTML report of the current levelset"));
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionExit] = new QAction(QIcon(":/res/application-exit.png"), tr("E&xit"), this);
    m_actions[ActionExit]->setStatusTip(tr("Close CCEdit"));

    m_actions[ActionUndo] = new QAction(QIcon(":/res/edit-undo.png"), tr("&Undo"), this);
    m_actions[ActionUndo]->setStatusTip(tr("Undo the last edit"));
    m_actions[ActionUndo]->setShortcut(Qt::CTRL | Qt::Key_Z);
    m_actions[ActionUndo]->setEnabled(false);
    m_actions[ActionRedo] = new QAction(QIcon(":/res/edit-redo.png"), tr("&Redo"), this);
    m_actions[ActionRedo]->setStatusTip(tr("Redo the last edit"));
    m_actions[ActionRedo]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Z);
    m_actions[ActionRedo]->setEnabled(false);
    m_actions[ActionSelect] = new QAction(QIcon(":/res/edit-select.png"), tr("&Select"), this);
    m_actions[ActionSelect]->setStatusTip(tr("Enter selection mode"));
    m_actions[ActionSelect]->setShortcut(Qt::CTRL | Qt::Key_A);
    m_actions[ActionSelect]->setCheckable(true);
    m_actions[ActionCut] = new QAction(QIcon(":/res/edit-cut.png"), tr("Cu&t"), this);
    m_actions[ActionCut]->setStatusTip(tr("Put the selection in the clipboard and clear it from the editor"));
    m_actions[ActionCut]->setShortcut(Qt::CTRL | Qt::Key_X);
    m_actions[ActionCut]->setEnabled(false);
    m_actions[ActionCopy] = new QAction(QIcon(":/res/edit-copy.png"), tr("&Copy"), this);
    m_actions[ActionCopy]->setStatusTip(tr("Copy the current selection to the clipboard"));
    m_actions[ActionCopy]->setShortcut(Qt::CTRL | Qt::Key_C);
    m_actions[ActionCopy]->setEnabled(false);
    m_actions[ActionPaste] = new QAction(QIcon(":/res/edit-paste.png"), tr("&Paste"), this);
    m_actions[ActionPaste]->setStatusTip(tr("Paste the clipboard contents into the levelset at the selection position"));
    m_actions[ActionPaste]->setShortcut(Qt::CTRL | Qt::Key_V);
    m_actions[ActionPaste]->setEnabled(false);
    m_actions[ActionClear] = new QAction(QIcon(":/res/edit-delete.png"), tr("C&lear"), this);
    m_actions[ActionClear]->setStatusTip(tr("Clear all tiles and mechanics from the selected region"));
    m_actions[ActionClear]->setShortcut(Qt::Key_Delete);
    m_actions[ActionClear]->setEnabled(false);

    m_actions[ActionDrawPencil] = new QAction(QIcon(":/res/draw-freehand.png"), tr("&Pencil"), this);
    m_actions[ActionDrawPencil]->setStatusTip(tr("Draw tiles with the pencil tool"));
    m_actions[ActionDrawPencil]->setShortcut(Qt::CTRL | Qt::Key_P);
    m_actions[ActionDrawPencil]->setCheckable(true);
    m_actions[ActionDrawLine] = new QAction(QIcon(":/res/draw-line.png"), tr("&Line"), this);
    m_actions[ActionDrawLine]->setStatusTip(tr("Draw tiles with the line tool"));
    m_actions[ActionDrawLine]->setShortcut(Qt::CTRL | Qt::Key_L);
    m_actions[ActionDrawLine]->setCheckable(true);
    m_actions[ActionDrawFill] = new QAction(QIcon(":/res/draw-box.png"), tr("&Box"), this);
    m_actions[ActionDrawFill]->setStatusTip(tr("Draw tiles with the box fill tool"));
    m_actions[ActionDrawFill]->setShortcut(Qt::CTRL | Qt::Key_B);
    m_actions[ActionDrawFill]->setCheckable(true);
    m_actions[ActionPathMaker] = new QAction(QIcon(":/res/draw-path.png"), tr("Path &Maker"), this);
    m_actions[ActionPathMaker]->setStatusTip(tr("Draw a directional path of tiles"));
    m_actions[ActionPathMaker]->setShortcut(Qt::CTRL | Qt::Key_M);
    m_actions[ActionPathMaker]->setCheckable(true);
    m_actions[ActionConnect] = new QAction(QIcon(":/res/cctools-rbutton.png"), tr("Button &Connector"), this);
    m_actions[ActionConnect]->setStatusTip(tr("Connect buttons to traps and cloning machines"));
    m_actions[ActionConnect]->setShortcut(Qt::CTRL | Qt::Key_T);
    m_actions[ActionConnect]->setCheckable(true);
    m_actions[ActionAdvancedMech] = new QAction(QIcon(":/res/cctools-teeth.png"), tr("&Advanced Mechanics"), this);
    m_actions[ActionAdvancedMech]->setStatusTip(tr("Manually manipulate gameplay mechanics for the current level"));
    m_actions[ActionAdvancedMech]->setShortcut(Qt::CTRL | Qt::Key_K);
    m_actions[ActionAdvancedMech]->setEnabled(false);
    m_actions[ActionToggleWalls] = new QAction(QIcon(":/res/cctools-gbutton.png"), tr("&Toggle Walls"), this);
    m_actions[ActionToggleWalls]->setStatusTip(tr("Toggle all toggle floors/walls in the current level"));
    m_actions[ActionToggleWalls]->setShortcut(Qt::CTRL | Qt::Key_G);
    m_actions[ActionToggleWalls]->setEnabled(false);
    m_actions[ActionCheckErrors] = new QAction(tr("Check for &Errors..."), this);
    m_actions[ActionCheckErrors]->setStatusTip(tr("Check for errors in the current levelset or a specific level"));
    m_actions[ActionCheckErrors]->setShortcut(Qt::CTRL | Qt::Key_E);
    m_actions[ActionCheckErrors]->setEnabled(false);
    QActionGroup* drawModeGroup = new QActionGroup(this);
    drawModeGroup->addAction(m_actions[ActionDrawPencil]);
    drawModeGroup->addAction(m_actions[ActionDrawLine]);
    drawModeGroup->addAction(m_actions[ActionDrawFill]);
    m_actions[ActionDrawPencil]->setChecked(true);

    m_actions[ActionViewButtons] = new QAction(tr("Show &Button Connections"), this);
    m_actions[ActionViewButtons]->setStatusTip(tr("Draw lines between connected buttons/traps/cloning machines in editor"));
    m_actions[ActionViewButtons]->setCheckable(true);
    m_actions[ActionViewMovers] = new QAction(tr("Show &Monster Order"), this);
    m_actions[ActionViewMovers]->setStatusTip(tr("Display Monster Order in editor"));
    m_actions[ActionViewMovers]->setCheckable(true);
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

    m_actions[ActionZoom100] = new QAction(tr("&100%"), this);
    m_actions[ActionZoom100]->setStatusTip(tr("Zoom to 100%"));
    m_actions[ActionZoom100]->setShortcut(Qt::CTRL | Qt::Key_1);
    m_actions[ActionZoom100]->setCheckable(true);
    m_actions[ActionZoom75] = new QAction(tr("&75%"), this);
    m_actions[ActionZoom75]->setStatusTip(tr("Zoom to 75%"));
    m_actions[ActionZoom75]->setShortcut(Qt::CTRL | Qt::Key_7);
    m_actions[ActionZoom75]->setCheckable(true);
    m_actions[ActionZoom50] = new QAction(tr("&50%"), this);
    m_actions[ActionZoom50]->setStatusTip(tr("Zoom to 50%"));
    m_actions[ActionZoom50]->setShortcut(Qt::CTRL | Qt::Key_5);
    m_actions[ActionZoom50]->setCheckable(true);
    m_actions[ActionZoom25] = new QAction(tr("&25%"), this);
    m_actions[ActionZoom25]->setStatusTip(tr("Zoom to 25%"));
    m_actions[ActionZoom25]->setShortcut(Qt::CTRL | Qt::Key_2);
    m_actions[ActionZoom25]->setCheckable(true);
    m_actions[ActionZoom125] = new QAction(tr("12.5&%"), this);
    m_actions[ActionZoom125]->setStatusTip(tr("Zoom to 12.5%"));
    m_actions[ActionZoom125]->setShortcut(Qt::CTRL | Qt::Key_3);
    m_actions[ActionZoom125]->setCheckable(true);
    m_actions[ActionZoomCust] = new QAction(tr("&Custom..."), this);
    m_actions[ActionZoomCust]->setStatusTip(tr("Zoom to custom percentage"));
    m_actions[ActionZoomCust]->setShortcut(Qt::CTRL | Qt::Key_9);
    m_actions[ActionZoomCust]->setCheckable(true);
    m_actions[ActionZoomFit] = new QAction(tr("&Fit window"), this);
    m_actions[ActionZoomFit]->setStatusTip(tr("Zoom to fit view area"));
    m_actions[ActionZoomFit]->setShortcut(Qt::CTRL | Qt::Key_0);
    m_actions[ActionZoomFit]->setCheckable(true);
    QActionGroup* zoomGroup = new QActionGroup(this);
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
    m_actions[ActionTestSetup] = new QAction(tr("&Setup Testing..."), this);
    m_actions[ActionTestSetup]->setStatusTip(tr("Setup testing parameters and options"));

    m_actions[ActionAbout] = new QAction(QIcon(":/res/help-about.png"), tr("&About CCEdit"), this);
    m_actions[ActionAbout]->setStatusTip(tr("Show information about CCEdit"));

    m_actions[ActionAddLevel] = new QAction(QIcon(":/res/list-add.png"), tr("&Add Level"), this);
    m_actions[ActionAddLevel]->setStatusTip(tr("Add a new level to the end of the levelset"));
    m_actions[ActionAddLevel]->setEnabled(false);
    m_actions[ActionDelLevel] = new QAction(QIcon(":/res/list-remove.png"), tr("&Remove Level"), this);
    m_actions[ActionDelLevel]->setStatusTip(tr("Remove the current level from the levelset"));
    m_actions[ActionDelLevel]->setEnabled(false);
    m_actions[ActionMoveUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move &Up"), this);
    m_actions[ActionMoveUp]->setStatusTip(tr("Move the current level up in the level list"));
    m_actions[ActionMoveUp]->setEnabled(false);
    m_actions[ActionMoveDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move &Down"), this);
    m_actions[ActionMoveDown]->setStatusTip(tr("Move the current level down in the level list"));
    m_actions[ActionMoveDown]->setEnabled(false);
    m_actions[ActionProperties] = new QAction(QIcon(":/res/document-properties.png"), tr("Levelset &Properties"), this);
    m_actions[ActionProperties]->setStatusTip(tr("Change levelset and .DAC file properties"));
    m_actions[ActionProperties]->setEnabled(false);
    m_actions[ActionOrganize] = new QAction(QIcon(":/res/level-organize.png"), tr("&Organize Levels"), this);
    m_actions[ActionOrganize]->setStatusTip(tr("Organize or import levels"));
    m_actions[ActionOrganize]->setEnabled(false);

    // Control Toolbox
    QDockWidget* toolDock = new QDockWidget(this);
    toolDock->setObjectName("ToolDock");
    toolDock->setWindowTitle(tr("Toolbox"));
    toolDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolTabs = new QTabWidget(toolDock);
    m_toolTabs->setObjectName("ToolTabs");
    m_toolTabs->setTabPosition(QTabWidget::West);
    toolDock->setWidget(m_toolTabs);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    QWidget* levelManWidget = new QWidget(toolDock);
    m_levelList = new QListWidget(levelManWidget);
    m_nameEdit = new QLineEdit(levelManWidget);
    QLabel* nameLabel = new QLabel(tr("&Name:"), levelManWidget);
    nameLabel->setBuddy(m_nameEdit);
    m_nameEdit->setMaxLength(254);
    m_passwordEdit = new QLineEdit(levelManWidget);
    QLabel* passLabel = new QLabel(tr("&Password:"), levelManWidget);
    passLabel->setBuddy(m_passwordEdit);
    m_passwordEdit->setMaxLength(9);
    QToolButton* passwordButton = new QToolButton(levelManWidget);
    passwordButton->setIcon(QIcon(":/res/view-refresh-sm.png"));
    passwordButton->setStatusTip(tr("Generate new random level password"));
    passwordButton->setAutoRaise(true);
    m_chipEdit = new QSpinBox(levelManWidget);
    QLabel* chipLabel = new QLabel(tr("&Chips:"), levelManWidget);
    chipLabel->setBuddy(m_chipEdit);
    m_chipEdit->setRange(0, 32767);
    QToolButton* chipsButton = new QToolButton(levelManWidget);
    chipsButton->setIcon(QIcon(":/res/view-refresh-sm.png"));
    chipsButton->setStatusTip(tr("Count all chips in the selected level"));
    chipsButton->setAutoRaise(true);
    m_timeEdit = new QSpinBox(levelManWidget);
    QLabel* timeLabel = new QLabel(tr("&Time:"), levelManWidget);
    timeLabel->setBuddy(m_timeEdit);
    m_timeEdit->setRange(0, 32767);
    m_hintEdit = new QLineEdit(levelManWidget);
    QLabel* hintLabel = new QLabel(tr("&Hint Text:"), levelManWidget);
    hintLabel->setBuddy(m_hintEdit);
    m_hintEdit->setMaxLength(254);

    QToolBar* tbarLevelset = new QToolBar(levelManWidget);
    tbarLevelset->addAction(m_actions[ActionAddLevel]);
    tbarLevelset->addAction(m_actions[ActionDelLevel]);
    tbarLevelset->addSeparator();
    tbarLevelset->addAction(m_actions[ActionMoveUp]);
    tbarLevelset->addAction(m_actions[ActionMoveDown]);
    tbarLevelset->addSeparator();
    tbarLevelset->addAction(m_actions[ActionProperties]);
    tbarLevelset->addAction(m_actions[ActionOrganize]);

    QGridLayout* levelManLayout = new QGridLayout(levelManWidget);
    levelManLayout->setContentsMargins(4, 4, 4, 4);
    levelManLayout->setVerticalSpacing(0);
    levelManLayout->setHorizontalSpacing(4);
    levelManLayout->addWidget(m_levelList, 0, 0, 1, 3);
    levelManLayout->addWidget(tbarLevelset, 1, 0, 1, 3);
    levelManLayout->addItem(new QSpacerItem(0, 8), 2, 0, 1, 3);
    levelManLayout->addWidget(nameLabel, 3, 0);
    levelManLayout->addWidget(m_nameEdit, 3, 1, 1, 2);
    levelManLayout->addWidget(passLabel, 4, 0);
    levelManLayout->addWidget(m_passwordEdit, 4, 1);
    levelManLayout->addWidget(passwordButton, 4, 2);
    levelManLayout->addWidget(chipLabel, 5, 0);
    levelManLayout->addWidget(m_chipEdit, 5, 1);
    levelManLayout->addWidget(chipsButton, 5, 2);
    levelManLayout->addWidget(timeLabel, 6, 0);
    levelManLayout->addWidget(m_timeEdit, 6, 1);
    levelManLayout->addItem(new QSpacerItem(0, 8), 7, 0, 1, 3);
    levelManLayout->addWidget(hintLabel, 8, 0);
    levelManLayout->addWidget(m_hintEdit, 8, 1, 1, 2);
    m_hintEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
    m_toolTabs->addTab(levelManWidget, tr("Level &Manager"));

    QWidget* tileWidget = new QWidget(toolDock);
    QToolBox* tileBox = new QToolBox(tileWidget);
    m_tileLists[ListStandard] = new TileListWidget(tileBox);
    m_tileLists[ListStandard]->addTiles(QList<tile_t>()
        << ccl::TileFloor << ccl::TileWall << ccl::TileChip << ccl::TileSocket
        << ccl::TileExit << ccl::TileHint << ccl::TileBarrier_N
        << ccl::TileBarrier_W << ccl::TileBarrier_S << ccl::TileBarrier_E
        << ccl::TileBarrier_SE << ccl::TileBlock << ccl::TileDirt
        << ccl::TileGravel << ccl::TilePlayer_N << ccl::TilePlayer_W
        << ccl::TilePlayer_S << ccl::TilePlayer_E);
    tileBox->addItem(m_tileLists[ListStandard], tr("Standard"));
    m_tileLists[ListObstacles] = new TileListWidget(tileBox);
    m_tileLists[ListObstacles]->addTiles(QList<tile_t>()
        << ccl::TileWater << ccl::TileFire << ccl::TileBomb << ccl::TileForce_N
        << ccl::TileForce_W << ccl::TileForce_S << ccl::TileForce_E
        << ccl::TileForce_Rand << ccl::TileIce << ccl::TileIce_NW
        << ccl::TileIce_NE << ccl::TileIce_SE << ccl::TileIce_SW
        << ccl::TileTrap << ccl::TileTrapButton << ccl::TilePopUpWall
        << ccl::TileAppearingWall << ccl::TileInvisWall);
    tileBox->addItem(m_tileLists[ListObstacles], tr("Obstacles"));
    m_tileLists[ListDoors] = new TileListWidget(tileBox);
    m_tileLists[ListDoors]->addTiles(QList<tile_t>()
        << ccl::TileDoor_Blue << ccl::TileDoor_Red << ccl::TileDoor_Green
        << ccl::TileDoor_Yellow << ccl::TileToggleFloor << ccl::TileToggleWall
        << ccl::TileToggleButton);
    tileBox->addItem(m_tileLists[ListDoors], tr("Doors"));
    m_tileLists[ListItems] = new TileListWidget(tileBox);
    m_tileLists[ListItems]->addTiles(QList<tile_t>()
        << ccl::TileKey_Blue << ccl::TileKey_Red << ccl::TileKey_Green
        << ccl::TileKey_Yellow << ccl::TileFlippers << ccl::TileFireBoots
        << ccl::TileIceSkates << ccl::TileForceBoots);
    tileBox->addItem(m_tileLists[ListItems], tr("Items"));
    m_tileLists[ListMonsters] = new TileListWidget(tileBox);
    m_tileLists[ListMonsters]->addTiles(QList<tile_t>()
        << ccl::TileBug_N << ccl::TileBug_W << ccl::TileBug_S << ccl::TileBug_E
        << ccl::TileFireball_N << ccl::TileFireball_W << ccl::TileFireball_S
        << ccl::TileFireball_E << ccl::TileBall_N << ccl::TileBall_W
        << ccl::TileBall_S << ccl::TileBall_E << ccl::TileTank_N
        << ccl::TileTank_W << ccl::TileTank_S << ccl::TileTank_E
        << ccl::TileTankButton << ccl::TileGlider_N << ccl::TileGlider_W
        << ccl::TileGlider_S << ccl::TileGlider_E << ccl::TileTeeth_N
        << ccl::TileTeeth_W << ccl::TileTeeth_S << ccl::TileTeeth_E
        << ccl::TileWalker_N << ccl::TileWalker_W << ccl::TileWalker_S
        << ccl::TileWalker_E << ccl::TileBlob_N << ccl::TileBlob_W
        << ccl::TileBlob_S << ccl::TileBlob_E << ccl::TileCrawler_N
        << ccl::TileCrawler_W << ccl::TileCrawler_S << ccl::TileCrawler_E);
    tileBox->addItem(m_tileLists[ListMonsters], tr("Monsters"));
    m_tileLists[ListMisc] = new TileListWidget(tileBox);
    m_tileLists[ListMisc]->addTiles(QList<tile_t>()
        << ccl::TileThief << ccl::TileBlueWall << ccl::TileBlueFloor
        << ccl::TileTeleport << ccl::TileCloner << ccl::TileCloneButton
        << ccl::TileBlock_N << ccl::TileBlock_W << ccl::TileBlock_S
        << ccl::TileBlock_E);
    tileBox->addItem(m_tileLists[ListMisc], tr("Miscellaneous"));
    m_tileLists[ListSpecial] = new TileListWidget(tileBox);
    m_tileLists[ListSpecial]->addTiles(QList<tile_t>()
        << ccl::TileIceBlock << ccl::TilePlayerSplash << ccl::TilePlayerFire
        << ccl::TilePlayerBurnt << ccl::TilePlayerExit << ccl::TileExitAnim2
        << ccl::TileExitAnim3 << ccl::TilePlayerSwim_N << ccl::TilePlayerSwim_W
        << ccl::TilePlayerSwim_S << ccl::TilePlayerSwim_E << ccl::Tile_UNUSED_20
        << ccl::Tile_UNUSED_36 << ccl::Tile_UNUSED_37);
    tileBox->addItem(m_tileLists[ListSpecial], tr("Special (Advanced)"));

    m_layer[0] = new LayerWidget(tileWidget);
    m_foreLabel[0] = new QLabel(tileWidget);
    m_backLabel[0] = new QLabel(tileWidget);

    QGridLayout* tileLayout = new QGridLayout(tileWidget);
    tileLayout->setContentsMargins(4, 4, 4, 4);
    tileLayout->setVerticalSpacing(4);
    tileLayout->addWidget(tileBox, 0, 0, 1, 2);
    tileLayout->addWidget(m_foreLabel[0], 1, 0);
    tileLayout->addWidget(m_backLabel[0], 2, 0);
    tileLayout->addWidget(m_layer[0], 1, 1, 2, 1);
    m_toolTabs->addTab(tileWidget, tr("&Tiles - Sorted"));

    QWidget* allTileWidget = new QWidget(toolDock);
    QScrollArea* allTileScroll = new QScrollArea(allTileWidget);
    m_allTiles = new BigTileWiget(allTileScroll);
    m_layer[1] = new LayerWidget(allTileWidget);
    m_foreLabel[1] = new QLabel(allTileWidget);
    m_backLabel[1] = new QLabel(allTileWidget);
    allTileScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setWidget(m_allTiles);

    QGridLayout* allTileLayout = new QGridLayout(allTileWidget);
    allTileLayout->setContentsMargins(4, 4, 4, 4);
    allTileLayout->setVerticalSpacing(4);
    allTileLayout->addWidget(allTileScroll, 0, 0, 1, 2);
    allTileLayout->addWidget(m_foreLabel[1], 1, 0);
    allTileLayout->addWidget(m_backLabel[1], 2, 0);
    allTileLayout->addWidget(m_layer[1], 1, 1, 2, 1);
    m_toolTabs->addTab(allTileWidget, tr("&All Tiles"));

    // Editor area
    m_editorTabs = new QTabWidget(this);
    QToolButton* newTabButton = new QToolButton(m_editorTabs);
    newTabButton->setIcon(QIcon(":/res/tab-new-sm.png"));
    newTabButton->setStatusTip(tr("Open a new editor tab"));
    newTabButton->setAutoRaise(true);
    m_editorTabs->setCornerWidget(newTabButton, Qt::TopLeftCorner);
#if QT_VERSION >= 0x040500
    m_editorTabs->setMovable(true);
    m_editorTabs->setTabsClosable(true);
#else
    QToolButton* closeTabButton = new QToolButton(m_editorTabs);
    closeTabButton->setIcon(QIcon(":/res/tab-close-sm.png"));
    closeTabButton->setStatusTip(tr("Close the current editor tab"));
    closeTabButton->setAutoRaise(true);
    m_editorTabs->setCornerWidget(closeTabButton, Qt::TopRightCorner);
#endif
    setCentralWidget(m_editorTabs);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNew]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionOpen]);
    fileMenu->addAction(m_actions[ActionSave]);
    fileMenu->addAction(m_actions[ActionSaveAs]);
    fileMenu->addAction(m_actions[ActionClose]);
    fileMenu->addSeparator();
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
    toolsMenu->addAction(m_actions[ActionDrawFill]);
    toolsMenu->addAction(m_actions[ActionPathMaker]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionConnect]);
    toolsMenu->addAction(m_actions[ActionAdvancedMech]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionToggleWalls]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionCheckErrors]);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_actions[ActionViewButtons]);
    viewMenu->addAction(m_actions[ActionViewMovers]);
    viewMenu->addAction(m_actions[ActionViewActivePlayer]);
    viewMenu->addAction(m_actions[ActionViewViewport]);
    viewMenu->addAction(m_actions[ActionViewMonsterPaths]);
    viewMenu->addAction(m_actions[ActionViewErrors]);
    viewMenu->addSeparator();
    m_tilesetMenu = viewMenu->addMenu(tr("Tile&set"));
    m_tilesetGroup = new QActionGroup(this);
    QMenu* zoomMenu = viewMenu->addMenu(tr("&Zoom"));
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
    testMenu->addSeparator();
    testMenu->addAction(m_actions[ActionTestSetup]);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_actions[ActionAbout]);

    // Tool bars
    QToolBar* tbarMain = addToolBar(QString());
    tbarMain->setObjectName("ToolbarMain");
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
    tbarTools->setObjectName("ToolbarTools");
    tbarTools->setWindowTitle(tr("Tools"));
    tbarTools->addAction(m_actions[ActionDrawPencil]);
    tbarTools->addAction(m_actions[ActionDrawLine]);
    tbarTools->addAction(m_actions[ActionDrawFill]);
    tbarTools->addAction(m_actions[ActionPathMaker]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionConnect]);
    tbarTools->addAction(m_actions[ActionAdvancedMech]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionToggleWalls]);

    // Show status bar
    statusBar();

    connect(m_actions[ActionNew], SIGNAL(triggered()), SLOT(onNewAction()));
    connect(m_actions[ActionOpen], SIGNAL(triggered()), SLOT(onOpenAction()));
    connect(m_actions[ActionSave], SIGNAL(triggered()), SLOT(onSaveAction()));
    connect(m_actions[ActionSaveAs], SIGNAL(triggered()), SLOT(onSaveAsAction()));
    connect(m_actions[ActionClose], SIGNAL(triggered()), SLOT(onCloseAction()));
    connect(m_actions[ActionGenReport], SIGNAL(triggered()), SLOT(onReportAction()));
    connect(m_actions[ActionExit], SIGNAL(triggered()), SLOT(close()));
    connect(m_actions[ActionSelect], SIGNAL(toggled(bool)), SLOT(onSelectToggled(bool)));
    connect(m_actions[ActionCut], SIGNAL(triggered()), SLOT(onCutAction()));
    connect(m_actions[ActionCopy], SIGNAL(triggered()), SLOT(onCopyAction()));
    connect(m_actions[ActionPaste], SIGNAL(triggered()), SLOT(onPasteAction()));
    connect(m_actions[ActionClear], SIGNAL(triggered()), SLOT(onClearAction()));
    connect(m_actions[ActionUndo], SIGNAL(triggered()), SLOT(onUndoAction()));
    connect(m_actions[ActionRedo], SIGNAL(triggered()), SLOT(onRedoAction()));
    connect(m_actions[ActionDrawPencil], SIGNAL(toggled(bool)), SLOT(onDrawPencilAction(bool)));
    connect(m_actions[ActionDrawLine], SIGNAL(toggled(bool)), SLOT(onDrawLineAction(bool)));
    connect(m_actions[ActionDrawFill], SIGNAL(toggled(bool)), SLOT(onDrawFillAction(bool)));
    connect(m_actions[ActionPathMaker], SIGNAL(toggled(bool)), SLOT(onPathMakerToggled(bool)));
    connect(m_actions[ActionConnect], SIGNAL(toggled(bool)), SLOT(onConnectToggled(bool)));
    connect(m_actions[ActionAdvancedMech], SIGNAL(triggered()), SLOT(onAdvancedMechAction()));
    connect(m_actions[ActionToggleWalls], SIGNAL(triggered()), SLOT(onToggleWallsAction()));
    connect(m_actions[ActionCheckErrors], SIGNAL(triggered()), SLOT(onCheckErrorsAction()));
    connect(m_actions[ActionViewButtons], SIGNAL(toggled(bool)), SLOT(onViewButtonsToggled(bool)));
    connect(m_actions[ActionViewMovers], SIGNAL(toggled(bool)), SLOT(onViewMoversToggled(bool)));
    connect(m_actions[ActionViewActivePlayer], SIGNAL(toggled(bool)), SLOT(onViewActivePlayerToggled(bool)));
    connect(m_actions[ActionViewViewport], SIGNAL(toggled(bool)), SLOT(onViewViewportToggled(bool)));
    connect(m_actions[ActionViewMonsterPaths], SIGNAL(toggled(bool)), SLOT(onViewMonsterPathsToggled(bool)));
    connect(m_actions[ActionViewErrors], SIGNAL(toggled(bool)), SLOT(onViewErrorsToggled(bool)));
    connect(m_tilesetGroup, SIGNAL(triggered(QAction*)), SLOT(onTilesetMenu(QAction*)));
    connect(m_actions[ActionZoom100], SIGNAL(triggered()), SLOT(onZoom100()));
    connect(m_actions[ActionZoom75], SIGNAL(triggered()), SLOT(onZoom75()));
    connect(m_actions[ActionZoom50], SIGNAL(triggered()), SLOT(onZoom50()));
    connect(m_actions[ActionZoom25], SIGNAL(triggered()), SLOT(onZoom25()));
    connect(m_actions[ActionZoom125], SIGNAL(triggered()), SLOT(onZoom125()));
    connect(m_actions[ActionZoomCust], SIGNAL(triggered()), SLOT(onZoomCust()));
    connect(m_actions[ActionZoomFit], SIGNAL(triggered()), SLOT(onZoomFit()));
    connect(m_actions[ActionTestChips], SIGNAL(triggered()), SLOT(onTestChips()));
    connect(m_actions[ActionTestTWorldCC], SIGNAL(triggered()), SLOT(onTestTWorldCC()));
    connect(m_actions[ActionTestTWorldLynx], SIGNAL(triggered()), SLOT(onTestTWorldLynx()));
    connect(m_actions[ActionTestSetup], SIGNAL(triggered()), SLOT(onTestSetup()));
    connect(m_actions[ActionAbout], SIGNAL(triggered()), SLOT(onAboutAction()));

    connect(m_actions[ActionAddLevel], SIGNAL(triggered()), SLOT(onAddLevelAction()));
    connect(m_actions[ActionDelLevel], SIGNAL(triggered()), SLOT(onDelLevelAction()));
    connect(m_actions[ActionMoveUp], SIGNAL(triggered()), SLOT(onMoveUpAction()));
    connect(m_actions[ActionMoveDown], SIGNAL(triggered()), SLOT(onMoveDownAction()));
    connect(m_actions[ActionProperties], SIGNAL(triggered()), SLOT(onPropertiesAction()));
    connect(m_actions[ActionOrganize], SIGNAL(triggered()), SLOT(onOrganizeAction()));

    connect(m_levelList, SIGNAL(currentRowChanged(int)), SLOT(onSelectLevel(int)));
    connect(m_nameEdit, SIGNAL(textChanged(QString)), SLOT(onNameChanged(QString)));
    connect(m_passwordEdit, SIGNAL(textChanged(QString)), SLOT(onPasswordChanged(QString)));
    connect(passwordButton, SIGNAL(clicked()), SLOT(onPasswordGenAction()));
    connect(m_chipEdit, SIGNAL(valueChanged(int)), SLOT(onChipsChanged(int)));
    connect(chipsButton, SIGNAL(clicked()), SLOT(onChipCountAction()));
    connect(m_timeEdit, SIGNAL(valueChanged(int)), SLOT(onTimerChanged(int)));
    connect(m_hintEdit, SIGNAL(textChanged(QString)), SLOT(onHintChanged(QString)));
    connect(toolDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(onDockChanged(Qt::DockWidgetArea)));
    connect(qApp->clipboard(), SIGNAL(dataChanged()), SLOT(onClipboardDataChanged()));

#if QT_VERSION >= 0x040500
    connect(m_editorTabs, SIGNAL(tabCloseRequested(int)), SLOT(onCloseTab(int)));
#else
    connect(closeTabButton, SIGNAL(clicked()), SLOT(onCloseCurrentTab()));
#endif
    connect(m_editorTabs, SIGNAL(currentChanged(int)), SLOT(onTabChanged(int)));
    connect(newTabButton, SIGNAL(clicked()), SLOT(onNewTab()));

    for (int i=0; i<NUM_TILE_LISTS; ++i) {
        connect(m_tileLists[i], SIGNAL(itemSelectedLeft(tile_t)), SLOT(setForeground(tile_t)));
        connect(m_tileLists[i], SIGNAL(itemSelectedRight(tile_t)), SLOT(setBackground(tile_t)));
    }
    connect(m_allTiles, SIGNAL(itemSelectedLeft(tile_t)), SLOT(setForeground(tile_t)));
    connect(m_allTiles, SIGNAL(itemSelectedRight(tile_t)), SLOT(setBackground(tile_t)));

    // Load window settings and defaults
    QSettings settings("CCTools", "CCEdit");
    resize(settings.value("WindowSize", QSize(1024, 768)).toSize());
    if (settings.value("WindowMaximized", false).toBool())
        showMaximized();
    if (settings.contains("WindowState"))
        restoreState(settings.value("WindowState").toByteArray());
    m_zoomFactor = settings.value("ZoomFactor", 1.0).toDouble();
    m_actions[ActionViewButtons]->setChecked(settings.value("ViewButtons", true).toBool());
    m_actions[ActionViewMovers]->setChecked(settings.value("ViewMovers", true).toBool());
    m_actions[ActionViewActivePlayer]->setChecked(settings.value("ViewActivePlayer", false).toBool());
    m_actions[ActionViewViewport]->setChecked(settings.value("ViewViewport", true).toBool());
    m_actions[ActionViewMonsterPaths]->setChecked(settings.value("ViewMonsterPaths", false).toBool());
    m_actions[ActionViewErrors]->setChecked(settings.value("ViewErrors", true).toBool());
    m_dialogDir = settings.value("DialogDir").toString();

    // Make sure the toolbox is visible
    if (toolDock->isFloating()) {
        QPoint dockPos = toolDock->pos();
        if ((dockPos.x() + toolDock->width() - 10) < qApp->desktop()->contentsRect().left())
            dockPos.setX(qApp->desktop()->contentsRect().left());
        if (dockPos.x() + 10 > qApp->desktop()->contentsRect().right())
            dockPos.setX(qApp->desktop()->contentsRect().right() - toolDock->width());
        if (dockPos.y() < qApp->desktop()->contentsRect().top())
            dockPos.setY(qApp->desktop()->contentsRect().top());
        if (dockPos.y() + 10 > qApp->desktop()->contentsRect().bottom())
            dockPos.setY(qApp->desktop()->contentsRect().bottom() - toolDock->height());
        toolDock->move(dockPos);
        toolDock->show();
    }

    findTilesets();
    if (m_tilesetGroup->actions().size() == 0) {
        QMessageBox::critical(this, tr("Error loading tilesets"),
                tr("Error: No tilesets found.  Please check your CCTools installation"),
                QMessageBox::Ok);
        exit(1);
    } else if (settings.contains("TilesetName")) {
        QString tilesetFilename = settings.value("TilesetName").toString();
        bool foundTset = false;
        for (int i=0; i<m_tilesetGroup->actions().size(); ++i) {
            CCETileset* tileset = (CCETileset*)m_tilesetGroup->actions()[i]->data().value<void*>();
            if (tileset->filename() == tilesetFilename) {
                m_tilesetGroup->actions()[i]->setChecked(true);
                loadTileset(tileset);
                foundTset = true;
                break;
            }
        }
        if (!foundTset) {
            m_tilesetGroup->actions()[0]->setChecked(true);
            loadTileset((CCETileset*)m_tilesetGroup->actions()[0]->data().value<void*>());
        }
    } else {
        m_tilesetGroup->actions()[0]->setChecked(true);
        loadTileset((CCETileset*)m_tilesetGroup->actions()[0]->data().value<void*>());
    }

    if (m_zoomFactor == 1.0)
        m_actions[ActionZoom100]->setChecked(true);
    else if (m_zoomFactor == 0.75)
        m_actions[ActionZoom75]->setChecked(true);
    else if (m_zoomFactor == 0.5)
        m_actions[ActionZoom50]->setChecked(true);
    else if (m_zoomFactor == 0.25)
        m_actions[ActionZoom25]->setChecked(true);
    else if (m_zoomFactor == 0.125)
        m_actions[ActionZoom125]->setChecked(true);
    else if (m_zoomFactor == 0.0)
        m_actions[ActionZoomFit]->setChecked(true);
    else
        m_actions[ActionZoomCust]->setChecked(true);

    setForeground(ccl::TileWall);
    setBackground(ccl::TileFloor);
    onSelectLevel(-1);
    onClipboardDataChanged();
}

void CCEditMain::loadLevelset(QString filename)
{
    if (!closeLevelset())
        return;

    ccl::LevelsetType type = ccl::DetermineLevelsetType(filename.toUtf8().data());
    if (type == ccl::LevelsetCcl) {
        ccl::FileStream set;
        if (set.open(filename.toUtf8().data(), "rb")) {
            m_levelset = new ccl::Levelset(0);
            try {
                m_levelset->read(&set);
            } catch (std::exception& e) {
                QMessageBox::critical(this, tr("Error reading levelset"),
                                      tr("Error loading levelset: %1").arg(e.what()));
                delete m_levelset;
                m_levelset = 0;
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
        FILE* dac = fopen(filename.toUtf8().data(), "rt");
        if (dac == 0) {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                  tr("Error: could not open file %1").arg(filename));
            return;
        }

        try {
            m_dacInfo.read(dac);
            fclose(dac);
        } catch (ccl::Exception& e) {
            QMessageBox::critical(this, tr("Error reading levelset"),
                                    tr("Error loading levelset descriptor: %1")
                                    .arg(e.what()));
            fclose(dac);
            return;
        }

        QDir searchPath(filename);
        searchPath.cdUp();

        ccl::FileStream set;
        if (set.open(searchPath.absoluteFilePath(m_dacInfo.m_filename.c_str()).toUtf8().data(), "rb")) {
            m_levelset = new ccl::Levelset(0);
            try {
                m_levelset->read(&set);
            } catch (std::exception& e) {
                QMessageBox::critical(this, tr("Error reading levelset"),
                                        tr("Error loading levelset: %1").arg(e.what()));
                delete m_levelset;
                m_levelset = 0;
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                tr("Error: could not open file %1")
                                .arg(m_dacInfo.m_filename.c_str()));
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
    m_levelset->makeClean();

    m_haveCcx = false;
    QString ccxName = filename.left(filename.lastIndexOf('.')) + ".ccx";
    if (m_ccxFile.ReadFile(ccxName, m_levelset->levelCount()))
        m_haveCcx = true;

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionTestChips]->setEnabled(true);
    m_actions[ActionTestTWorldCC]->setEnabled(true);
    m_actions[ActionTestTWorldLynx]->setEnabled(true);
    m_actions[ActionAddLevel]->setEnabled(true);
    m_actions[ActionDelLevel]->setEnabled(m_levelset->levelCount() > 0);
    m_actions[ActionProperties]->setEnabled(true);
    m_actions[ActionOrganize]->setEnabled(true);
    m_actions[ActionCheckErrors]->setEnabled(true);
}

void CCEditMain::doLevelsetLoad()
{
    // This can be used to refresh the level display as well as load a new
    // set, so we should only adjust existing items rather than re-adding
    // the whole list.  This will preserve selection states and scroll
    // position, which in turn makes level re-ordering faster while
    // maintaining correctness.

    for (int i=m_levelList->count(); i<m_levelset->levelCount(); ++i) {
        // Add items to match new count
        m_levelList->addItem(QString());
    }
    for (int i=m_levelset->levelCount(); i<m_levelList->count(); ++i) {
        // Remove extra items from list
        delete m_levelList->takeItem(i);
    }
    for (int i=0; i<m_levelset->levelCount(); ++i) {
        // Use iterator for level number since stored level numbers may
        // be meaningless until saved...
        m_levelList->item(i)->setText(QString("%1 - %2").arg(i + 1)
                .arg(QString::fromLatin1(m_levelset->level(i)->name().c_str())));
    }
    if (m_levelList->currentItem() == 0 && m_levelset->levelCount() > 0)
        m_levelList->setCurrentRow(0);
    m_toolTabs->setCurrentIndex(0);
}

void CCEditMain::setLevelsetFilename(QString filename)
{
    m_levelsetFilename = filename;
    QString displayName = filename.isEmpty() ? "Untitled"
                        : QDir(filename).absolutePath().section(QChar('/'), -1);
    setWindowTitle(CCEDIT_TITLE " - " + displayName);
}

void CCEditMain::saveLevelset(QString filename)
{
    if (m_levelset == 0)
        return;

    if (m_useDac) {
        FILE* dac = fopen(filename.toUtf8().data(), "wt");
        if (dac != 0) {
            try {
                m_dacInfo.write(dac);
                fclose(dac);
            } catch (ccl::Exception& e) {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error saving levelset descriptor: %1")
                                      .arg(e.what()));
                fclose(dac);
                return;
            }

            QDir searchPath(filename);
            searchPath.cdUp();

            ccl::FileStream set;
            if (set.open(searchPath.absoluteFilePath(m_dacInfo.m_filename.c_str()).toUtf8().data(), "wb")) {
                try {
                    m_levelset->write(&set);
                } catch (ccl::Exception e) {
                    QMessageBox::critical(this, tr("Error saving levelset"),
                                          tr("Error saving levelset: %1").arg(e.what()));
                    return;
                }
            } else {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error: could not write file %1")
                                      .arg(m_dacInfo.m_filename.c_str()));
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error saving levelset"),
                                  tr("Error: could not write file %1").arg(filename));
            return;
        }
    } else {
        ccl::FileStream set;
        if (set.open(filename.toUtf8(), "wb")) {
            try {
                m_levelset->write(&set);
            } catch (ccl::Exception e) {
                QMessageBox::critical(this, tr("Error saving levelset"),
                                      tr("Error saving levelset: %1").arg(e.what()));
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
    m_levelset->makeClean();
}

void CCEditMain::closeEvent(QCloseEvent* event)
{
    if (!closeLevelset()) {
        event->setAccepted(false);
        return;
    }

    if (m_subProc != 0) {
        // Don't handle events after we're exiting.
        // Note that MSCC temp file cleanup will not take place if this happens!
        m_subProc->disconnect();
    }

    QSettings settings("CCTools", "CCEdit");
    settings.setValue("WindowMaximized", (windowState() & Qt::WindowMaximized) != 0);
    showNormal();
    settings.setValue("WindowSize", size());
    settings.setValue("WindowState", saveState());
    settings.setValue("ZoomFactor", m_zoomFactor);
    settings.setValue("ViewButtons", m_actions[ActionViewButtons]->isChecked());
    settings.setValue("ViewMovers", m_actions[ActionViewMovers]->isChecked());
    settings.setValue("ViewActivePlayer", m_actions[ActionViewActivePlayer]->isChecked());
    settings.setValue("ViewViewport", m_actions[ActionViewViewport]->isChecked());
    settings.setValue("ViewMonsterPaths", m_actions[ActionViewMonsterPaths]->isChecked());
    settings.setValue("ViewErrors", m_actions[ActionViewErrors]->isChecked());
    settings.setValue("TilesetName", m_currentTileset->filename());
    settings.setValue("DialogDir", m_dialogDir);
}

void CCEditMain::resizeEvent(QResizeEvent* event)
{
    if (event != 0)
        QWidget::resizeEvent(event);

    if (m_zoomFactor == 0.0 && m_editorTabs->currentWidget() != 0) {
        QSize zmax = ((QScrollArea*)m_editorTabs->currentWidget())->maximumViewportSize();
        double zx = (double)zmax.width() / (32 * m_currentTileset->size());
        double zy = (double)zmax.height() / (32 * m_currentTileset->size());
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setZoom(std::min(zx, zy));
    }
}

bool CCEditMain::closeLevelset()
{
    if (m_levelset == 0)
        return true;

    int reply = m_levelset->isDirty()
              ? QMessageBox::question(this, tr("Close levelset"),
                        tr("Save changes to %1 before closing?")
                        .arg(m_levelsetFilename.isEmpty() ? "new levelset" : m_levelsetFilename),
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
    m_levelList->clear();
    onSelectLevel(-1);
    delete m_levelset;
    m_levelset = 0;
    setWindowTitle(CCEDIT_TITLE);

    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionTestChips]->setEnabled(false);
    m_actions[ActionTestTWorldCC]->setEnabled(false);
    m_actions[ActionTestTWorldLynx]->setEnabled(false);
    m_actions[ActionAddLevel]->setEnabled(false);
    m_actions[ActionDelLevel]->setEnabled(false);
    m_actions[ActionProperties]->setEnabled(false);
    m_actions[ActionOrganize]->setEnabled(false);
    m_actions[ActionCheckErrors]->setEnabled(false);

    return true;
}

void CCEditMain::loadTileset(CCETileset* tileset)
{
    m_currentTileset = tileset;
    m_layer[0]->setTileset(tileset);
    m_layer[1]->setTileset(tileset);
    for (int i=0; i<NUM_TILE_LISTS; ++i)
        tileset->imageTiles(m_tileLists[i]);
    m_allTiles->setTileset(tileset);
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setTileset(tileset);
    resizeEvent(0);
}

void CCEditMain::registerTileset(QString filename)
{
    CCETileset* tileset = new CCETileset(this);
    tileset->load(filename);
    QAction* menuItem = m_tilesetMenu->addAction(tileset->name());
    menuItem->setCheckable(true);
    menuItem->setStatusTip(tileset->description());
    menuItem->setData(qVariantFromValue((void*)tileset));
    m_tilesetGroup->addAction(menuItem);
}

void CCEditMain::findTilesets()
{
    m_tilesetMenu->clear();

    QDir path;
    QStringList tilesets;
#if defined(Q_OS_WIN32)
    // Search app directory
    path.setPath(qApp->applicationDirPath());
    tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
    foreach (QString file, tilesets)
        registerTileset(path.absoluteFilePath(file));
#else
    // Search install path
    path.setPath(qApp->applicationDirPath());
    path.cdUp();
    path.cd("share/cctools");
    tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
    foreach (QString file, tilesets)
        registerTileset(path.absoluteFilePath(file));

    // Search standard directories
    path.setPath("/usr/share/cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        foreach (QString file, tilesets)
            registerTileset(path.absoluteFilePath(file));
    }
    path.setPath("/usr/local/share/cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        foreach (QString file, tilesets)
            registerTileset(path.absoluteFilePath(file));
    }
#endif

    // User-space local data
    path.setPath(QDir::homePath());
    path.cd(".cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        foreach (QString file, tilesets)
            registerTileset(path.absoluteFilePath(file));
    }
}

void CCEditMain::selectLevel(int level)
{
    if (level < m_levelList->count())
        m_levelList->setCurrentRow(level - 1);
    else
        m_levelList->setCurrentRow(m_levelList->count() - 1);
}

EditorWidget* CCEditMain::getEditorAt(int idx)
{
    if (idx < 0 || idx >= m_editorTabs->count())
        return 0;

    QScrollArea* scroll = (QScrollArea*)m_editorTabs->widget(idx);
    return (EditorWidget*)scroll->widget();
}

EditorWidget* CCEditMain::addEditor(ccl::LevelData* level)
{
    QScrollArea* scroll = new QScrollArea(m_editorTabs);
    EditorWidget* editor = new EditorWidget(scroll);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidget(editor);
    if (m_actions[ActionViewButtons]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowButtons);
    if (m_actions[ActionViewMovers]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowMovement);
    if (m_actions[ActionViewActivePlayer]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowPlayer);
    if (m_actions[ActionViewViewport]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowViewBox);
    if (m_actions[ActionViewMonsterPaths]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowMovePaths);
    if (m_actions[ActionViewErrors]->isChecked())
        editor->setPaintFlag(EditorWidget::ShowErrors);
    editor->setDrawMode(m_currentDrawMode);
    editor->setTileset(m_currentTileset);
    editor->setLevelData(level);
    editor->setLeftTile(m_layer[0]->upper());
    editor->setRightTile(m_layer[0]->lower());
    if (m_zoomFactor != 0.0)
        editor->setZoom(m_zoomFactor);
    m_editorTabs->addTab(scroll, level->name().c_str());
    resizeEvent(0);

    connect(editor, SIGNAL(mouseInfo(QString)), statusBar(), SLOT(showMessage(QString)));
    connect(editor, SIGNAL(canUndo(bool)), m_actions[ActionUndo], SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(canRedo(bool)), m_actions[ActionRedo], SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(hasSelection(bool)), m_actions[ActionCut], SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(hasSelection(bool)), m_actions[ActionCopy], SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(hasSelection(bool)), m_actions[ActionClear], SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(makeDirty()), SLOT(onMakeDirty()));
    return editor;
}

void CCEditMain::closeAllTabs()
{
    while (getEditorAt(0) != 0) {
        delete m_editorTabs->widget(0);
        m_editorTabs->removeTab(0);
    }
}

void CCEditMain::onNewAction()
{
    if (!closeLevelset())
        return;

    m_levelset = new ccl::Levelset();
    doLevelsetLoad();
    setLevelsetFilename(QString());
    m_useDac = false;

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionTestChips]->setEnabled(true);
    m_actions[ActionTestTWorldCC]->setEnabled(true);
    m_actions[ActionTestTWorldLynx]->setEnabled(true);
    m_actions[ActionAddLevel]->setEnabled(true);
    m_actions[ActionDelLevel]->setEnabled(true);
    m_actions[ActionProperties]->setEnabled(true);
    m_actions[ActionOrganize]->setEnabled(true);
    m_actions[ActionCheckErrors]->setEnabled(true);
}

void CCEditMain::onOpenAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Levelset..."),
                            m_dialogDir, "All Levelsets (*.dat *.dac *.ccl)");
    if (!filename.isEmpty()) {
        loadLevelset(filename);
        QDir dir(filename);
        dir.cdUp();
        m_dialogDir = dir.absolutePath();
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
    QString filter = m_useDac ? "TWorld Levelsets (*.dac)"
                              : "CC Levelsets (*.dat *.ccl)";

    if (m_levelsetFilename.isEmpty())
        m_levelsetFilename = m_dialogDir;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Levelset..."),
                                                    m_levelsetFilename, filter);
    if (!filename.isEmpty()) {
        saveLevelset(filename);
        QDir dir(filename);
        dir.cdUp();
        m_dialogDir = dir.absolutePath();
    }
}

void CCEditMain::onReportAction()
{
    QString reportPath = m_levelsetFilename.isEmpty() ? m_dialogDir
                       : m_levelsetFilename.left(m_levelsetFilename.lastIndexOf('.')) + ".html";
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Report..."),
                                                    reportPath, "HTML Source (*.html)");
    if (filename.isEmpty())
        return;

    QProgressDialog proDlg(this);
    proDlg.setMaximum(m_levelset->levelCount() + 1);
    proDlg.setMinimumDuration(2000);
    proDlg.setLabelText(tr("Generating HTML report.  Please be patient..."));

    // Generate HTML output
    QFile report(filename);
    if (!report.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error creating report"),
                tr("Error creating report: Could not write HTML report"));
        return;
    }
    QString filebase = QDir(filename).dirName();
    filebase = filebase.left(filebase.lastIndexOf('.')) + ".ccl";
    report.write("<html>\n<head><title>Levelset Report: ");
    report.write(filebase.toUtf8().data());
    report.write("</title></head>\n\n");

    report.write("<body>\n<h1 align=\"center\">Levelset Report:");
    report.write(filebase.toUtf8().data());
    report.write("</h1>\n");

    QString dirbase = filebase.left(filebase.lastIndexOf('.')) + "_levimg";
    for (int i=0; i<m_levelset->levelCount(); ++i) {
        ccl::LevelData* level = m_levelset->level(i);
        report.write("<hr />\n<h2>Level ");
        report.write(QString("%1").arg(i + 1).toUtf8().data());
        report.write("</h2>\n<pre>\n");
        report.write("<b>Title:</b>    ");
        report.write(level->name().c_str());
        report.write("\n<b>Chips:</b>    ");
        report.write(QString("%1").arg(level->chips()).toUtf8().data());
        report.write("\n<b>Time:</b>     ");
        report.write(QString("%1").arg(level->timer()).toUtf8().data());
        report.write("\n<b>Password:</b> ");
        report.write(level->password().c_str());
        report.write("\n<b>Hint:</b>     ");
        report.write(level->hint().c_str());
        report.write("\n</pre>\n<img src=\"");
        report.write(QString("%1/level%2.png").arg(dirbase).arg(i + 1).toUtf8().data());
        report.write("\" />\n");
    }
    report.write("</html>\n");
    proDlg.setValue(1);
    if (proDlg.wasCanceled())
        return;

    // Generate level images
    dirbase = filename.left(filename.lastIndexOf('.')) + "_levimg";
    QDir dir;
    dir.mkdir(dirbase);
    if (!dir.cd(dirbase)) {
        QMessageBox::critical(this, tr("Error creating report"),
                tr("Error creating report: Could not create level image folder"));
        return;
    }
    {
        EditorWidget reportDummy;
        reportDummy.setVisible(false);
        reportDummy.setTileset(m_currentTileset);
        for (int i=0; !proDlg.wasCanceled() && i<m_levelset->levelCount(); ++i) {
            reportDummy.setLevelData(m_levelset->level(i));
            reportDummy.renderReport().save(dirbase + QString("/level%1.png").arg(i+1));
            proDlg.setValue(i+2);
        }
    }
}

void CCEditMain::onSelectToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawSelect) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawSelect;
        m_actions[ActionDrawPencil]->setChecked(false);
        m_actions[ActionDrawLine]->setChecked(false);
        m_actions[ActionDrawFill]->setChecked(false);
        m_actions[ActionPathMaker]->setChecked(false);
        m_actions[ActionConnect]->setChecked(false);

        for (int i=0; i<m_editorTabs->count(); ++i)
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
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor == 0 || editor->selection() == QRect(-1, -1, -1, -1))
        return;

    ccl::LevelData* copyRegion = new ccl::LevelData();
    ccl::LevelData* current = editor->levelData();
    copyRegion->map().copyFrom(editor->levelData()->map(),
        editor->selection().left(), editor->selection().top(),
        0, 0, editor->selection().width(), editor->selection().height());

    // Gather any mechanics that are completely encompassed by this region
    std::list<ccl::Trap>::const_iterator trap_iter;
    for (trap_iter = current->traps().begin(); trap_iter != current->traps().end(); ++trap_iter) {
        if (editor->selection().contains(trap_iter->button.X, trap_iter->button.Y)
            && editor->selection().contains(trap_iter->trap.X, trap_iter->trap.Y))
            copyRegion->trapConnect(trap_iter->button.X - editor->selection().left(),
                                    trap_iter->button.Y - editor->selection().top(),
                                    trap_iter->trap.X - editor->selection().left(),
                                    trap_iter->trap.Y - editor->selection().top());
    }

    std::list<ccl::Clone>::const_iterator clone_iter;
    for (clone_iter = current->clones().begin(); clone_iter != current->clones().end(); ++clone_iter) {
        if (editor->selection().contains(clone_iter->button.X, clone_iter->button.Y)
            && editor->selection().contains(clone_iter->clone.X, clone_iter->clone.Y))
            copyRegion->cloneConnect(clone_iter->button.X - editor->selection().left(),
                                     clone_iter->button.Y - editor->selection().top(),
                                     clone_iter->clone.X - editor->selection().left(),
                                     clone_iter->clone.Y - editor->selection().top());
    }

    std::list<ccl::Point>::const_iterator move_iter;
    for (move_iter = current->moveList().begin(); move_iter != current->moveList().end(); ++move_iter) {
        if (editor->selection().contains(move_iter->X, move_iter->Y))
            copyRegion->addMover(move_iter->X - editor->selection().left(),
                                 move_iter->Y - editor->selection().top());
    }

    try {
        ccl::BufferStream cbStream;
        cbStream.write32(editor->selection().width());
        cbStream.write32(editor->selection().height());
        cbStream.write16(0);    // Revisit after writing data
        cbStream.write32(0);
        copyRegion->write(&cbStream, true);
        cbStream.seek(8, SEEK_SET);
        cbStream.write16(cbStream.size() - 14); // Size of data buffer
        QByteArray buffer((const char*)cbStream.buffer(), cbStream.size());

        QMimeData* copyData = new QMimeData();
        copyData->setData("CHIPEDIT MAPSECT", buffer);
        qApp->clipboard()->setMimeData(copyData);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error"),
                tr("Error saving clipboard data: %1").arg(e.what()),
                QMessageBox::Ok);
    }
    copyRegion->unref();
}

void CCEditMain::onPasteAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor == 0)
        return;

    const QMimeData* cbData = qApp->clipboard()->mimeData();
    if (cbData->hasFormat("CHIPEDIT MAPSECT")) {
        QByteArray buffer = cbData->data("CHIPEDIT MAPSECT");
        ccl::BufferStream cbStream;
        cbStream.setFrom(buffer.data(), buffer.size());

        int width, height;
        ccl::LevelData* copyRegion = new ccl::LevelData();
        try {
            width = cbStream.read32();
            height = cbStream.read32();
            cbStream.read16();
            cbStream.read32();
            copyRegion->read(&cbStream, true);
        } catch (std::exception& e) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Error parsing clipboard data: %1").arg(e.what()),
                    QMessageBox::Ok);
            copyRegion->unref();
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
        if (destX + width > CCL_WIDTH)
            width = CCL_WIDTH - destX;
        if (destY + height > CCL_HEIGHT)
            height = CCL_HEIGHT - destY;

        editor->beginEdit(CCEHistoryNode::HistPaste);
        editor->selectRegion(destX, destY, width, height);
        onClearAction();
        editor->levelData()->map().copyFrom(copyRegion->map(),
            0, 0, destX, destY, width, height);
        ccl::LevelData* current = editor->levelData();

        std::list<ccl::Trap>::const_iterator trap_iter;
        for (trap_iter = copyRegion->traps().begin(); trap_iter != copyRegion->traps().end(); ++trap_iter) {
            if (current->traps().size() < MAX_TRAPS && trap_iter->button.X + destX < CCL_WIDTH
                && trap_iter->button.Y + destY < CCL_HEIGHT && trap_iter->trap.X + destX < CCL_WIDTH
                && trap_iter->trap.Y + destY < CCL_HEIGHT)
                current->trapConnect(trap_iter->button.X + destX, trap_iter->button.Y + destY,
                                     trap_iter->trap.X + destX, trap_iter->trap.Y + destY);
        }

        std::list<ccl::Clone>::const_iterator clone_iter;
        for (clone_iter = copyRegion->clones().begin(); clone_iter != copyRegion->clones().end(); ++clone_iter) {
            if (current->clones().size() < MAX_CLONES && clone_iter->button.X + destX < CCL_WIDTH
                && clone_iter->button.Y + destY < CCL_HEIGHT && clone_iter->clone.X + destX < CCL_WIDTH
                && clone_iter->clone.Y + destY < CCL_HEIGHT)
                current->cloneConnect(clone_iter->button.X + destX, clone_iter->button.Y + destY,
                                      clone_iter->clone.X + destX, clone_iter->clone.Y + destY);
        }

        std::list<ccl::Point>::const_iterator move_iter;
        for (move_iter = copyRegion->moveList().begin(); move_iter != copyRegion->moveList().end(); ++move_iter) {
            if (current->moveList().size() < MAX_MOVERS && move_iter->X + destX < CCL_WIDTH
                && move_iter->Y + destY < CCL_HEIGHT)
                current->addMover(move_iter->X + destX, move_iter->Y + destY);
        }

        editor->endEdit();
        copyRegion->unref();
        m_levelset->makeDirty();
    }
}

void CCEditMain::onClearAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor == 0 || editor->selection() == QRect(-1, -1, -1, -1))
        return;

    editor->beginEdit(CCEHistoryNode::HistClear);
    for (int y = editor->selection().top(); y <= editor->selection().bottom(); ++y) {
        for (int x = editor->selection().left(); x <= editor->selection().right(); ++x) {
            editor->putTile(ccl::TileFloor, x, y, EditorWidget::LayTop);
            editor->putTile(ccl::TileFloor, x, y, EditorWidget::LayBottom);
        }
    }
    editor->endEdit();
    editor->update();
    m_levelset->makeDirty();
}

void CCEditMain::onUndoAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor != 0) {
        editor->undo();
        m_levelset->makeDirty();
    }
}

void CCEditMain::onRedoAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor != 0) {
        editor->redo();
        m_levelset->makeDirty();
    }
}

void CCEditMain::onDrawPencilAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawPencil;
    m_currentDrawMode = EditorWidget::DrawPencil;
    m_actions[ActionSelect]->setChecked(false);
    m_actions[ActionPathMaker]->setChecked(false);
    m_actions[ActionConnect]->setChecked(false);
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawLineAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawLine;
    m_currentDrawMode = EditorWidget::DrawLine;
    m_actions[ActionSelect]->setChecked(false);
    m_actions[ActionPathMaker]->setChecked(false);
    m_actions[ActionConnect]->setChecked(false);
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onDrawFillAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawFill;
    m_currentDrawMode = EditorWidget::DrawFill;
    m_actions[ActionSelect]->setChecked(false);
    m_actions[ActionPathMaker]->setChecked(false);
    m_actions[ActionConnect]->setChecked(false);
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setDrawMode(m_currentDrawMode);
}

void CCEditMain::onPathMakerToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawPathMaker) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawPathMaker;
        m_actions[ActionSelect]->setChecked(false);
        m_actions[ActionDrawPencil]->setChecked(false);
        m_actions[ActionDrawLine]->setChecked(false);
        m_actions[ActionDrawFill]->setChecked(false);
        m_actions[ActionConnect]->setChecked(false);

        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setDrawMode(m_currentDrawMode);
    }
}

void CCEditMain::onConnectToggled(bool mode)
{
    if (!mode && m_currentDrawMode == EditorWidget::DrawButtonConnect) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = EditorWidget::DrawButtonConnect;
        m_actions[ActionSelect]->setChecked(false);
        m_actions[ActionDrawPencil]->setChecked(false);
        m_actions[ActionDrawLine]->setChecked(false);
        m_actions[ActionDrawFill]->setChecked(false);
        m_actions[ActionPathMaker]->setChecked(false);

        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setDrawMode(m_currentDrawMode);
    }
}

void CCEditMain::onAdvancedMechAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor == 0)
        return;

    AdvancedMechanicsDialog mechDlg(this);
    mechDlg.setFrom(editor->levelData());
    editor->beginEdit(CCEHistoryNode::HistEditMech);
    if (mechDlg.exec() == QDialog::Accepted) {
        editor->endEdit();
        m_levelset->makeDirty();
    } else {
        editor->cancelEdit();
    }
    editor->update();
}

void CCEditMain::onToggleWallsAction()
{
    EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
    if (editor == 0)
        return;

    editor->beginEdit(CCEHistoryNode::HistToggleWalls);
    ccl::ToggleDoors(editor->levelData());
    editor->endEdit();
    editor->update();
    m_levelset->makeDirty();
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

void CCEditMain::onZoom100()
{
    m_zoomFactor = 1.0;
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(m_zoomFactor);
}

void CCEditMain::onZoom75()
{
    m_zoomFactor = 0.75;
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(m_zoomFactor);
}

void CCEditMain::onZoom50()
{
    m_zoomFactor = 0.5;
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(m_zoomFactor);
}

void CCEditMain::onZoom25()
{
    m_zoomFactor = 0.25;
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(m_zoomFactor);
}

void CCEditMain::onZoom125()
{
    m_zoomFactor = 0.125;
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setZoom(m_zoomFactor);
}

void CCEditMain::onZoomCust()
{
    bool ok;
    double zoom = QInputDialog::getDouble(this, tr("Set Custom Zoom"),
                        tr("Custom Zoom Percentage"), m_zoomFactor * 100.0,
                        2.5, 800.0, 2, &ok);
    if (ok) {
        m_zoomFactor = zoom / 100.0;
        for (int i=0; i<m_editorTabs->count(); ++i)
            getEditorAt(i)->setZoom(m_zoomFactor);
    }
    if (m_zoomFactor == 1.0)
        m_actions[ActionZoom100]->setChecked(true);
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
    resizeEvent(0);
}

void CCEditMain::onTilesetMenu(QAction* which)
{
    CCETileset* tileset = (CCETileset*)which->data().value<void*>();
    loadTileset(tileset);
}

void CCEditMain::onTestChips()
{
    if (m_levelset == 0 || m_levelList->currentRow() < 0)
        return;

    if (m_subProc != 0) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CCEdit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings("CCTools", "CCEdit");
    QString chipsExe = settings.value("ChipsExe").toString();
    if (chipsExe.isEmpty() || !QFile::exists(chipsExe)) {
        QMessageBox::critical(this, tr("Could not find CHIPS.EXE"),
                tr("Could not find Chip's Challenge executable.\n"
                   "Please configure MSCC in the Test Setup dialog."));
        return;
    }
#ifndef Q_OS_WIN32
    QString winePath = settings.value("WineExe").toString();
    if (winePath.isEmpty() || !QFile::exists(winePath)) {
        // Try standard paths
        if (QFile::exists("/usr/bin/wine")) {
            winePath = "/usr/bin/wine";
        } else if (QFile::exists("/usr/local/bin/wine")) {
            winePath = "/usr/local/bin/wine";
        } else {
            QMessageBox::critical(this, tr("Could not find WINE"),
                    tr("Could not find WINE executable.\n"
                       "Please configure WINE in the Test Setup dialog."));
            return;
        }
    }
#endif

    m_tempExe = QDir::tempPath() + "/CCRun.exe";
    m_tempDat = QDir::tempPath() + "/CCRun.dat";
    QFile::remove(m_tempExe);
    if (!QFile::copy(chipsExe, m_tempExe)) {
        QMessageBox::critical(this, tr("Error Creating Test EXE"),
                tr("Error copying %1 to temp path").arg(chipsExe));
        return;
    }
    ccl::FileStream stream;
    if (!stream.open(m_tempExe.toUtf8().data(), "r+b")) {
        QMessageBox::critical(this, tr("Error Creating Test EXE"),
                tr("Error opening %1 for writing").arg(m_tempExe));
        return;
    }

    // Make a CHIPS.EXE that we can use
    ccl::ChipsHax hax;
    hax.open(&stream);
    if (settings.value("TestPGPatch", false).toBool())
        hax.set_PGChips(ccl::CCPatchPatched);
    if (settings.value("TestCCPatch", true).toBool())
        hax.set_CCPatch(ccl::CCPatchPatched);
    hax.set_LastLevel(m_levelset->levelCount());
    if (m_useDac && m_dacInfo.m_lastLevel < m_levelset->levelCount())
        hax.set_FakeLastLevel(m_dacInfo.m_lastLevel);
    else
        hax.set_FakeLastLevel(m_levelset->levelCount());
    hax.set_IgnorePasswords(true);
    hax.set_DataFilename("CCRun.dat");
    hax.set_IniFilename("./CCRun.ini");
    hax.set_IniEntryName("CCEdit Playtest");
    hax.set_DialogTitle("CCEdit Playtest");
    hax.set_WindowTitle("CCEdit Playtest");
    stream.close();

    // Save the levelset to the temp file
    if (!stream.open(m_tempDat.toUtf8().data(), "wb")) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error opening %1 for writing").arg(m_tempDat));
        return;
    }
    unsigned int saveType = m_levelset->type();
    if (settings.value("TestPGPatch", false).toBool())
        m_levelset->setType(ccl::Levelset::TypePG);
    else
        m_levelset->setType(ccl::Levelset::TypeMS);
    try {
        m_levelset->write(&stream);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.what()));
        m_levelset->setType(saveType);
        stream.close();
        return;
    }
    m_levelset->setType(saveType);
    stream.close();

    // Configure the INI file
    QString cwd = QDir::currentPath();
    QDir exePath = chipsExe;
    exePath.cdUp();

    m_tempIni = exePath.absoluteFilePath("CCRun.ini");
    FILE* iniStream = fopen(m_tempIni.toUtf8().data(), "r+t");
    if (iniStream == 0)
        iniStream = fopen(m_tempIni.toUtf8().data(), "w+t");
    if (iniStream == 0) {
        QMessageBox::critical(this, tr("Error Creating CCRun.ini"),
                tr("Error: Could not open or create CCRun.ini file"));
        QFile::remove(m_tempExe);
        QFile::remove(m_tempDat);
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream);
        ini.setSection("CCEdit Playtest");
        ini.setInt("Current Level", m_levelList->currentRow() + 1);
        ini.setString(QString("Level%1").arg(m_levelList->currentRow() + 1).toUtf8().data(),
                      m_levelset->level(m_levelList->currentRow())->password());
        ini.write(iniStream);
        fclose(iniStream);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error writing CCRun.ini"),
                tr("Error writing INI file: %1").arg(e.what()));
        fclose(iniStream);
        QFile::remove(m_tempExe);
        QFile::remove(m_tempDat);
        QFile::remove(m_tempIni);
        return;
    }

    QDir::setCurrent(exePath.absolutePath());
    m_subProc = new QProcess(this);
    m_subProcType = SubprocMSCC;
    connect(m_subProc, SIGNAL(finished(int)), SLOT(onProcessFinished(int)));
    connect(m_subProc, SIGNAL(error(QProcess::ProcessError)), SLOT(onProcessError(QProcess::ProcessError)));
#ifdef Q_OS_WIN32
    // Native execution
    m_subProc->start(m_tempExe);
#else
    // Try to use WINE
    m_subProc->start(winePath, QStringList() << m_tempExe);
#endif
    QDir::setCurrent(cwd);
}

void CCEditMain::onTestTWorld(unsigned int levelsetType)
{
    if (m_levelset == 0 || m_levelList->currentRow() < 0)
        return;

    if (m_subProc != 0) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CCEdit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings("CCTools", "CCEdit");
    QString tworldExe = settings.value("TWorldExe").toString();
    if (tworldExe.isEmpty() || !QFile::exists(tworldExe)) {
#ifndef Q_OS_WIN32
        // Try standard paths
        if (QFile::exists("/usr/games/tworld")) {
            tworldExe = "/usr/games/tworld";
        } else if (QFile::exists("/usr/local/games/tworld")) {
            tworldExe = "/usr/local/games/tworld";
        } else {
            QMessageBox::critical(this, tr("Could not find Tile World"),
                    tr("Could not find Tile World executable.\n"
                       "Please configure Tile World in the Test Setup dialog."));
            return;
        }
#else
        QMessageBox::critical(this, tr("Could not find Tile World"),
                tr("Could not find Tile World executable.\n"
                   "Please configure Tile World in the Test Setup dialog."));
        return;
#endif
    }

    // Save the levelset to the temp file
    m_tempDat = QDir::toNativeSeparators(QDir::tempPath() + "/CCRun.dat");
    ccl::FileStream stream;
    if (!stream.open(m_tempDat.toUtf8().data(), "wb")) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error opening %1 for writing").arg(m_tempDat));
        return;
    }
    unsigned int saveType = m_levelset->type();
    m_levelset->setType(levelsetType);
    try {
        m_levelset->write(&stream);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.what()));
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
    connect(m_subProc, SIGNAL(finished(int)), SLOT(onProcessFinished(int)));
    connect(m_subProc, SIGNAL(error(QProcess::ProcessError)), SLOT(onProcessError(QProcess::ProcessError)));
    m_subProc->start(tworldExe, QStringList() << "-pr" << m_tempDat
                                << QString("%1").arg(m_levelList->currentRow() + 1));
    QDir::setCurrent(cwd);
}

void CCEditMain::onTestSetup()
{
    TestSetupDialog dlg;
    dlg.exec();
}

void CCEditMain::onAboutAction()
{
    AboutDialog about;
    about.exec();
}

void CCEditMain::onAddLevelAction()
{
    if (m_levelset == 0)
        return;

    m_levelset->addLevel();
    doLevelsetLoad();
    m_levelList->setCurrentRow(m_levelList->count() - 1);
    m_levelset->makeDirty();
}

void CCEditMain::onDelLevelAction()
{
    if (m_levelset == 0 || m_levelList->currentRow() < 0)
        return;

    int result = QMessageBox::question(this, tr("Delete Level"),
                    tr("Are you sure you want to delete this level?\n"
                       "This action cannot be undone!"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    int idx = m_levelList->currentRow();
    m_levelList->setCurrentRow(idx - 1);
    ccl::LevelData* level = m_levelset->takeLevel(idx);
    for (int i=0; i<m_editorTabs->count(); ++i) {
        if (getEditorAt(i)->levelData() == level)
            onCloseTab(i);
    }
    doLevelsetLoad();
    m_levelset->makeDirty();

    // Checked here for the case where the last level is deleted.
    // The selection changes before the item is removed, so we can have
    // a state where the buttons are enabled erroneously.
    m_actions[ActionMoveDown]->setEnabled(m_levelList->currentRow() < m_levelList->count() - 1);
    m_actions[ActionDelLevel]->setEnabled(m_levelset->levelCount() > 0);
}

void CCEditMain::onMoveUpAction()
{
    if (m_levelset == 0 || m_levelList->currentRow() < 0)
        return;

    int idx = m_levelList->currentRow();
    if (idx > 0) {
        ccl::LevelData* level = m_levelset->takeLevel(idx);
        m_levelset->insertLevel(idx - 1, level);
        doLevelsetLoad();
        m_levelList->setCurrentRow(idx - 1);
        m_levelset->makeDirty();
    }
}

void CCEditMain::onMoveDownAction()
{
    if (m_levelset == 0 || m_levelList->currentRow() < 0)
        return;

    int idx = m_levelList->currentRow();
    if (idx < (m_levelList->count() - 1)) {
        ccl::LevelData* level = m_levelset->takeLevel(idx);
        m_levelset->insertLevel(idx + 1, level);
        doLevelsetLoad();
        m_levelList->setCurrentRow(idx + 1);
        m_levelset->makeDirty();
    }
}

void CCEditMain::onPropertiesAction()
{
    if (m_levelset == 0)
        return;

    LevelsetProps props(this);
    props.setLevelset(m_levelset);
    props.setDacFile(m_useDac ? &m_dacInfo : 0);
    while (props.exec() == QDialog::Accepted) {
        m_levelset->setType(props.levelsetType());
        if (props.useDac()) {
            if (props.dacFilename().isEmpty()) {
                QMessageBox::critical(this, tr("Error"),
                        tr("You must specify a levelset filename when using a DAC file"),
                        QMessageBox::Ok);
                continue;
            }
            m_dacInfo.m_filename = props.dacFilename().toUtf8().data();
            m_dacInfo.m_ruleset = props.dacRuleset();
            m_dacInfo.m_lastLevel = props.lastLevel();
            m_dacInfo.m_usePasswords = props.usePasswords();
        }
        if (props.useDac() != m_useDac) {
            // Fix filename
            QString fnameNoSuffix = m_levelsetFilename.left(m_levelsetFilename.lastIndexOf(QChar('.')));
            if (!fnameNoSuffix.isEmpty())
                m_levelsetFilename = fnameNoSuffix + (props.useDac() ? ".dac" : ".dat");
            setLevelsetFilename(m_levelsetFilename);
        }
        m_useDac = props.useDac();
        m_levelset->makeDirty();
        break;
    }
}

void CCEditMain::onOrganizeAction()
{
    if (m_levelset == 0)
        return;

    OrganizerDialog dlg(this);
    dlg.setTileset(m_currentTileset);
    dlg.loadLevelset(m_levelset);
    if (dlg.exec() == QDialog::Accepted) {
        doLevelsetLoad();
        for (int i=0; i<m_editorTabs->count(); ++i) {
            if (getEditorAt(i)->isOrphaned())
                delete m_editorTabs->widget(i);
        }
        m_levelset->makeDirty();
    }
}

void CCEditMain::onSelectLevel(int idx)
{
    if (m_levelset == 0 || idx < 0) {
        m_nameEdit->setEnabled(false);
        m_passwordEdit->setEnabled(false);
        m_chipEdit->setEnabled(false);
        m_timeEdit->setEnabled(false);
        m_hintEdit->setEnabled(false);
        m_nameEdit->setText(QString());
        m_passwordEdit->setText(QString());
        m_chipEdit->setValue(0);
        m_timeEdit->setValue(0);
        m_hintEdit->setText(QString());
        closeAllTabs();

        m_actions[ActionMoveUp]->setEnabled(false);
        m_actions[ActionMoveDown]->setEnabled(false);
        m_actions[ActionDelLevel]->setEnabled(false);
        m_actions[ActionAdvancedMech]->setEnabled(false);
        m_actions[ActionToggleWalls]->setEnabled(false);
    } else {
        ccl::LevelData* level = m_levelset->level(idx);
        m_nameEdit->setEnabled(true);
        m_passwordEdit->setEnabled(true);
        m_chipEdit->setEnabled(true);
        m_timeEdit->setEnabled(true);
        m_hintEdit->setEnabled(true);
        m_nameEdit->setText(QString::fromLatin1(level->name().c_str()));
        m_passwordEdit->setText(QString::fromLatin1(level->password().c_str()));
        m_chipEdit->setValue(level->chips());
        m_timeEdit->setValue(level->timer());
        m_hintEdit->setText(QString::fromLatin1(level->hint().c_str()));

        EditorWidget* editor = getEditorAt(m_editorTabs->currentIndex());
        if (editor == 0) {
            editor = addEditor(level);
        } else {
            editor->setLevelData(level);
            m_editorTabs->setTabText(m_editorTabs->currentIndex(), level->name().c_str());
        }

        m_actions[ActionMoveUp]->setEnabled(idx > 0);
        m_actions[ActionMoveDown]->setEnabled(idx < m_levelList->count() - 1);
        m_actions[ActionDelLevel]->setEnabled(true);
        m_actions[ActionAdvancedMech]->setEnabled(true);
        m_actions[ActionToggleWalls]->setEnabled(true);
    }
}

void CCEditMain::onPasswordGenAction()
{
    if (m_levelList->currentRow() < 0)
        return;
    m_passwordEdit->setText(QString::fromLatin1(ccl::Levelset::RandomPassword().c_str()));
}

void CCEditMain::onChipCountAction()
{
    if (m_levelList->currentRow() < 0)
        return;
    int chips = 0;
    const ccl::LevelMap& map = m_levelset->level(m_levelList->currentRow())->map();

    for (int x=0; x<32; ++x) {
        for (int y=0; y<32; ++y) {
            if (map.getFG(x, y) == ccl::TileChip)
                ++chips;
            if (map.getBG(x, y) == ccl::TileChip)
                ++chips;
        }
    }
    m_chipEdit->setValue(chips);
}

void CCEditMain::onNameChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;

    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    if (level->name() != value.toLatin1().data()) {
        level->setName(value.toLatin1().data());
        m_levelset->makeDirty();
    }
    m_levelList->currentItem()->setText(QString("%1 - %2")
                    .arg(m_levelList->currentRow() + 1).arg(value));

    for (int i=0; i<m_editorTabs->count(); ++i) {
        if (getEditorAt(i)->levelData() == level)
            m_editorTabs->setTabText(i, value);
    }
}

void CCEditMain::onPasswordChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;
    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    if (level->password() != value.toLatin1().data()) {
        level->setPassword(value.toLatin1().data());
        m_levelset->makeDirty();
    }
}

void CCEditMain::onChipsChanged(int value)
{
    if (m_levelList->currentRow() < 0)
        return;
    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    if (level->chips() != value) {
        level->setChips(value);
        m_levelset->makeDirty();
    }
}

void CCEditMain::onTimerChanged(int value)
{
    if (m_levelList->currentRow() < 0)
        return;
    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    if (level->timer() != value) {
        level->setTimer(value);
        m_levelset->makeDirty();
    }
}

void CCEditMain::onHintChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;
    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    if (level->hint() != value.toLatin1().data()) {
        level->setHint(value.toLatin1().data());
        m_levelset->makeDirty();
    }
}

void CCEditMain::setForeground(tile_t tile)
{
    m_layer[0]->setUpper(tile);
    m_layer[1]->setUpper(tile);
    m_foreLabel[0]->setText(tr("Foreground: ") + CCETileset::TileName(tile));
    m_foreLabel[1]->setText(m_foreLabel[0]->text());
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setLeftTile(tile);
}

void CCEditMain::setBackground(tile_t tile)
{
    m_layer[0]->setLower(tile);
    m_layer[1]->setLower(tile);
    m_backLabel[0]->setText(tr("Background: ") + CCETileset::TileName(tile));
    m_backLabel[1]->setText(m_backLabel[0]->text());
    for (int i=0; i<m_editorTabs->count(); ++i)
        getEditorAt(i)->setRightTile(tile);
}

void CCEditMain::onClipboardDataChanged()
{
    const QMimeData* cbData = qApp->clipboard()->mimeData();
    m_actions[ActionPaste]->setEnabled(cbData->hasFormat("CHIPEDIT MAPSECT"));
}

void CCEditMain::onNewTab()
{
    if (m_levelList->currentRow() >= 0) {
        addEditor(m_levelset->level(m_levelList->currentRow()));
        m_editorTabs->setCurrentIndex(m_editorTabs->count() - 1);
    }
}

void CCEditMain::onCloseTab(int tabIdx)
{
    if (m_editorTabs->count() == 1)
        return;
    delete m_editorTabs->widget(tabIdx);
}

void CCEditMain::onCloseCurrentTab()
{
    onCloseTab(m_editorTabs->currentIndex());
}

void CCEditMain::onTabChanged(int tabIdx)
{
    if (tabIdx < 0)
        return;
    EditorWidget* editor = getEditorAt(tabIdx);

    //m_levelList->setCurrentRow();
    editor->dirtyBuffer();
    editor->update();
    editor->updateUndoStatus();

    bool hasSelection = editor->selection() != QRect(-1, -1, -1, -1);
    m_actions[ActionCut]->setEnabled(hasSelection);
    m_actions[ActionCopy]->setEnabled(hasSelection);
    m_actions[ActionClear]->setEnabled(hasSelection);
}

void CCEditMain::onDockChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::RightDockWidgetArea)
        m_toolTabs->setTabPosition(QTabWidget::East);
    else
        m_toolTabs->setTabPosition(QTabWidget::West);
}

void CCEditMain::onProcessFinished(int)
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
    m_subProc = 0;
}

void CCEditMain::onProcessError(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart) {
        QMessageBox::critical(this, tr("Error starting process"),
                tr("Error starting test process.  Please check your settings "
                   "and try again"), QMessageBox::Ok);
    }
    onProcessFinished(-1);
}


int main(int argc, char* argv[])
{
    // Set random seed for password generation
    srand(time(NULL));

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    CCEditMain mainWin;
    mainWin.show();
    if (argc > 1)
        mainWin.loadLevelset(argv[1]);
    if (argc > 2)
        mainWin.selectLevel((int)strtol(argv[2], NULL, 10));
    return app.exec();
}
