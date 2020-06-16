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

#include "CC2Edit.h"
#include "EditorWidget.h"
#include "ExtWidgets.h"
#include "TileWidgets.h"
#include "ScriptTools.h"
#include "TestSetup.h"
#include "About.h"

#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QAction>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QToolBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QGridLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

#define CC2EDIT_TITLE "CC2Edit 2.1"

Q_DECLARE_METATYPE(CC2ETileset*)

enum TileListId {
    ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
    ListLogic, ListGlyphs, ListMisc, NUM_TILE_LISTS
};

#define DIR_TILE_LIST(baseTile)                     \
    cc2::Tile(baseTile, cc2::Tile::North, 0),       \
    cc2::Tile(baseTile, cc2::Tile::East, 0),        \
    cc2::Tile(baseTile, cc2::Tile::South, 0),       \
    cc2::Tile(baseTile, cc2::Tile::West, 0)

#define DIR_LOGIC_GATE(baseGate) \
    cc2::Tile(cc2::Tile::LogicGate, baseGate##_N),  \
    cc2::Tile(cc2::Tile::LogicGate, baseGate##_E),  \
    cc2::Tile(cc2::Tile::LogicGate, baseGate##_S),  \
    cc2::Tile(cc2::Tile::LogicGate, baseGate##_W)

CC2EditMain::CC2EditMain(QWidget* parent)
    : QMainWindow(parent), m_currentTileset(), m_subProc()
{
    setWindowTitle(CC2EDIT_TITLE);
    setDockOptions(QMainWindow::AnimatedDocks);

    // Actions
    m_actions[ActionNewMap] = new QAction(QIcon(":/res/document-new.png"), tr("&New Map..."), this);
    m_actions[ActionNewMap]->setStatusTip(tr("Create new map"));
    m_actions[ActionNewMap]->setShortcut(Qt::Key_F2);
    m_actions[ActionNewScript] = new QAction(QIcon(":/res/document-new.png"), tr("N&ew Script..."), this);
    m_actions[ActionNewScript]->setStatusTip(tr("Create new game script"));
    m_actions[ActionNewScript]->setShortcut(Qt::SHIFT | Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(QIcon(":/res/document-open.png"), tr("&Open..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a game file from disk"));
    m_actions[ActionOpen]->setShortcut(Qt::CTRL | Qt::Key_O);
    m_actions[ActionSave] = new QAction(QIcon(":/res/document-save.png"), tr("&Save"), this);
    m_actions[ActionSave]->setStatusTip(tr("Save the current map to the same file"));
    m_actions[ActionSave]->setShortcut(Qt::CTRL | Qt::Key_S);
    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs] = new QAction(QIcon(":/res/document-save-as.png"), tr("Save &As..."), this);
    m_actions[ActionSaveAs]->setStatusTip(tr("Save the current map to a new file or location"));
    m_actions[ActionSaveAs]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionClose] = new QAction(tr("&Close Map"), this);
    m_actions[ActionClose]->setStatusTip(tr("Close the currently open map"));
    m_actions[ActionClose]->setShortcut(Qt::CTRL | Qt::Key_W);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport] = new QAction(tr("Generate &Report"), this);
    m_actions[ActionGenReport]->setStatusTip(tr("Generate an HTML report of the current map"));
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionExit] = new QAction(QIcon(":/res/application-exit.png"), tr("E&xit"), this);
    m_actions[ActionExit]->setStatusTip(tr("Close CC2Edit"));

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
    m_actions[ActionSelect]->setEnabled(false);
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
    m_actions[ActionToggleWalls] = new QAction(QIcon(":/res/cctools-gbutton.png"), tr("&Toggle Walls"), this);
    m_actions[ActionToggleWalls]->setStatusTip(tr("Toggle all toggle floors/walls in the current level"));
    m_actions[ActionToggleWalls]->setShortcut(Qt::CTRL | Qt::Key_G);
    m_actions[ActionToggleWalls]->setEnabled(false);
    auto drawModeGroup = new QActionGroup(this);
    drawModeGroup->addAction(m_actions[ActionDrawPencil]);
    drawModeGroup->addAction(m_actions[ActionDrawLine]);
    drawModeGroup->addAction(m_actions[ActionDrawFill]);
    m_actions[ActionDrawPencil]->setChecked(true);

    m_actions[ActionViewActivePlayer] = new QAction(tr("Show &Player Starting Position"), this);
    m_actions[ActionViewActivePlayer]->setStatusTip(tr("Highlight the Player's start position"));
    m_actions[ActionViewActivePlayer]->setCheckable(true);
    m_actions[ActionViewViewport] = new QAction(tr("Show Game &Viewport"), this);
    m_actions[ActionViewViewport]->setStatusTip(tr("Show a viewport bounding box around the cursor"));
    m_actions[ActionViewViewport]->setCheckable(true);
    m_actions[ActionViewMonsterPaths] = new QAction(tr("Show Mo&nster Paths"), this);
    m_actions[ActionViewMonsterPaths]->setStatusTip(tr("Trace Projected Monster Paths (May be inaccurate)"));
    m_actions[ActionViewMonsterPaths]->setCheckable(true);

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
    auto zoomGroup = new QActionGroup(this);
    zoomGroup->addAction(m_actions[ActionZoom100]);
    zoomGroup->addAction(m_actions[ActionZoom75]);
    zoomGroup->addAction(m_actions[ActionZoom50]);
    zoomGroup->addAction(m_actions[ActionZoom25]);
    zoomGroup->addAction(m_actions[ActionZoom125]);
    zoomGroup->addAction(m_actions[ActionZoomCust]);
    zoomGroup->addAction(m_actions[ActionZoomFit]);

    m_actions[ActionTest] = new QAction(tr("&Test"), this);
    m_actions[ActionTest]->setStatusTip(tr("Test the current level in Chip's Challenge 2"));
    m_actions[ActionTest]->setShortcut(Qt::Key_F5);
    m_actions[ActionTest]->setEnabled(false);
    m_actions[ActionTestSetup] = new QAction(tr("&Setup Testing..."), this);
    m_actions[ActionTestSetup]->setStatusTip(tr("Setup testing parameters and options"));

    m_actions[ActionAbout] = new QAction(QIcon(":/res/help-about.png"), tr("&About CC2Edit"), this);
    m_actions[ActionAbout]->setStatusTip(tr("Show information about CC2Edit"));

    m_actions[ActionReloadScript] = new QAction(QIcon(":/res/view-refresh.png"),
                                                tr("&Reload Game Script"), this);
    m_actions[ActionReloadScript]->setStatusTip(tr("Re-load the current game script from disk"));
    m_actions[ActionEditScript] = new QAction(QIcon(":/res/document-properties.png"),
                                              tr("&Edit Game Script"), this);
    m_actions[ActionEditScript]->setStatusTip(tr("Open the current game script for editing"));

    // Control Toolbox
    auto toolDock = new QDockWidget(this);
    toolDock->setObjectName("ToolDock");
    toolDock->setWindowTitle(tr("Toolbox"));
    toolDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_toolTabs = new QTabWidget(toolDock);
    m_toolTabs->setObjectName("ToolTabs");
    m_toolTabs->setTabPosition(QTabWidget::West);
    toolDock->setWidget(m_toolTabs);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    m_gameProperties = new QWidget(toolDock);
    m_gameName = new QLabel(m_gameProperties);
    m_gameName->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    auto gameNameFont = m_gameName->font();
    gameNameFont.setBold(true);
    gameNameFont.setPointSize((gameNameFont.pointSize() * 3) / 2);
    m_gameName->setFont(gameNameFont);
    auto tbarGameScript = new QToolBar(m_gameProperties);
    tbarGameScript->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    tbarGameScript->addWidget(m_gameName);
    tbarGameScript->addAction(m_actions[ActionReloadScript]);
    tbarGameScript->addAction(m_actions[ActionEditScript]);
    m_gameMapList = new QListWidget(m_gameProperties);

    connect(m_gameMapList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        QString filename = item->data(Qt::UserRole).toString();
        if (!filename.isEmpty())
            loadMap(filename);
    });

    auto gamePropsLayout = new QGridLayout(m_gameProperties);
    gamePropsLayout->setContentsMargins(4, 4, 4, 4);
    gamePropsLayout->setVerticalSpacing(4);
    gamePropsLayout->setHorizontalSpacing(4);
    gamePropsLayout->addWidget(tbarGameScript, 0, 0);
    gamePropsLayout->addWidget(m_gameMapList, 1, 0);
    m_toolTabs->addTab(m_gameProperties, tr("&Game"));
    m_gameProperties->setEnabled(false);

    m_mapProperties = new QWidget(toolDock);
    m_title = new QLineEdit(m_mapProperties);
    auto titleLabel = new QLabel(tr("&Title:"), m_mapProperties);
    titleLabel->setBuddy(m_title);
    m_author = new QLineEdit(m_mapProperties);
    auto authorLabel = new QLabel(tr("&Author:"), m_mapProperties);
    authorLabel->setBuddy(m_author);
    m_lockText = new QLineEdit(m_mapProperties);
    auto lockLabel = new QLabel(tr("&Lock:"), m_mapProperties);
    lockLabel->setBuddy(m_lockText);
    m_editorVersion = new QLineEdit(m_mapProperties);
    m_editorVersion->setReadOnly(true);
    auto editorVersionLabel = new QLabel(tr("&Version:"), m_mapProperties);
    editorVersionLabel->setBuddy(m_editorVersion);

    m_mapSize = new QLineEdit(m_mapProperties);
    m_mapSize->setEnabled(false);
    auto mapSizeLabel = new QLabel(tr("Map &Size:"), m_mapProperties);
    mapSizeLabel->setBuddy(m_mapSize);
    auto resizeButton = new QPushButton(tr("&Resize..."), m_mapProperties);
    resizeButton->setStatusTip(tr("Resize the level"));
    m_chipCounter = new QLineEdit(m_mapProperties);
    m_chipCounter->setEnabled(false);
    auto chipLabel = new QLabel(tr("&Chips:"), m_mapProperties);
    chipLabel->setBuddy(m_chipCounter);
    m_pointCounter = new QLineEdit(m_mapProperties);
    m_pointCounter->setEnabled(false);
    auto pointLabel = new QLabel(tr("&Points:"), m_mapProperties);
    pointLabel->setBuddy(m_pointCounter);
    m_timeLimit = new QSpinBox(m_mapProperties);
    m_timeLimit->setRange(0, 32767);
    auto timeLabel = new QLabel(tr("Ti&me:"), m_mapProperties);
    timeLabel->setBuddy(m_timeLimit);
    m_viewport = new QComboBox(m_mapProperties);
    m_viewport->addItem(tr("10 x 10"), cc2::MapOption::View10x10);
    m_viewport->addItem(tr("9 x 9"), cc2::MapOption::View9x9);
    m_viewport->addItem(tr("Split"), cc2::MapOption::ViewSplit);
    auto viewLabel = new QLabel(tr("Vie&w:"), m_mapProperties);
    viewLabel->setBuddy(m_viewport);
    m_blobPattern = new QComboBox(m_mapProperties);
    m_blobPattern->addItem(tr("Deterministic"), cc2::MapOption::BlobsDeterministic);
    m_blobPattern->addItem(tr("4 Patterns"), cc2::MapOption::Blobs4Pattern);
    m_blobPattern->addItem(tr("Extra Random"), cc2::MapOption::BlobsExtraRandom);
    auto blobPatternLabel = new QLabel(tr("&Blobs:"), m_mapProperties);
    blobPatternLabel->setBuddy(m_blobPattern);
    m_hideLogic = new QCheckBox(tr("&Hide Logic"), m_mapProperties);
    m_cc1Boots = new QCheckBox(tr("CC&1 Boots"), m_mapProperties);
    m_readOnly = new QCheckBox(tr("&Read-Only"), m_mapProperties);
    auto optionsLabel = new QLabel(tr("Options:"), m_mapProperties);

    m_clue = new QPlainTextEdit(m_mapProperties);
    m_clue->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    auto clueLabel = new QLabel(tr("C&lue:"), m_mapProperties);
    clueLabel->setBuddy(m_clue);
    m_note = new QPlainTextEdit(m_mapProperties);
    m_note->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    auto noteLabel = new QLabel(tr("&Notes:"), m_mapProperties);
    noteLabel->setBuddy(m_note);

    auto mapPropsLayout = new QGridLayout(m_mapProperties);
    mapPropsLayout->setContentsMargins(4, 4, 4, 4);
    mapPropsLayout->setVerticalSpacing(4);
    mapPropsLayout->setHorizontalSpacing(4);
    int row = 0;
    mapPropsLayout->addWidget(titleLabel, row, 0);
    mapPropsLayout->addWidget(m_title, row, 1, 1, 2);
    mapPropsLayout->addWidget(authorLabel, ++row, 0);
    mapPropsLayout->addWidget(m_author, row, 1, 1, 2);
    mapPropsLayout->addWidget(lockLabel, ++row, 0);
    mapPropsLayout->addWidget(m_lockText, row, 1, 1, 2);
    mapPropsLayout->addWidget(editorVersionLabel, ++row, 0);
    mapPropsLayout->addWidget(m_editorVersion, row, 1, 1, 2);
    mapPropsLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed),
                            ++row, 0);
    mapPropsLayout->addWidget(mapSizeLabel, ++row, 0);
    mapPropsLayout->addWidget(m_mapSize, row, 1);
    mapPropsLayout->addWidget(resizeButton, row, 2);
    mapPropsLayout->addWidget(chipLabel, ++row, 0);
    mapPropsLayout->addWidget(m_chipCounter, row, 1, 1, 2);
    mapPropsLayout->addWidget(pointLabel, ++row, 0);
    mapPropsLayout->addWidget(m_pointCounter, row, 1, 1, 2);
    mapPropsLayout->addWidget(timeLabel, ++row, 0);
    mapPropsLayout->addWidget(m_timeLimit, row, 1, 1, 2);
    mapPropsLayout->addWidget(viewLabel, ++row, 0);
    mapPropsLayout->addWidget(m_viewport, row, 1, 1, 2);
    mapPropsLayout->addWidget(blobPatternLabel, ++row, 0);
    mapPropsLayout->addWidget(m_blobPattern, row, 1, 1, 2);
    mapPropsLayout->addWidget(optionsLabel, ++row, 0);
    mapPropsLayout->addWidget(m_hideLogic, row, 1, 1, 2);
    mapPropsLayout->addWidget(m_cc1Boots, ++row, 1, 1, 2);
    mapPropsLayout->addWidget(m_readOnly, ++row, 1, 1, 2);
    mapPropsLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed),
                            ++row, 0);
    mapPropsLayout->addWidget(clueLabel, ++row, 0, Qt::AlignTop);
    mapPropsLayout->addWidget(m_clue, row, 1, 1, 2);
    mapPropsLayout->addWidget(noteLabel, ++row, 0, Qt::AlignTop);
    mapPropsLayout->addWidget(m_note, row, 1, 1, 2);
    m_toolTabs->addTab(m_mapProperties, tr("Map &Properties"));
    m_mapProperties->setEnabled(false);

    auto sortedTiles = new QWidget(toolDock);
    auto tileBox = new QToolBox(sortedTiles);
    TileListWidget* tileLists[NUM_TILE_LISTS];
    tileLists[ListStandard] = new TileListWidget(tileBox);
    tileLists[ListStandard]->setTiles({
        cc2::Tile(cc2::Tile::Floor),
        cc2::Tile(cc2::Tile::Wall),
        cc2::Tile(cc2::Tile::SteelWall),
        cc2::Tile(cc2::Tile::Chip),
        cc2::Tile(cc2::Tile::ExtraChip),
        cc2::Tile(cc2::Tile::Socket),
        cc2::Tile(cc2::Tile::Exit),
        cc2::Tile(cc2::Tile::Clue),
        cc2::Tile(cc2::Tile::DirtBlock),
        cc2::Tile(cc2::Tile::IceBlock),
        cc2::Tile(cc2::Tile::Dirt),
        cc2::Tile(cc2::Tile::Gravel),
        cc2::Tile::panelTile(cc2::Tile::PanelNorth),
        cc2::Tile::panelTile(cc2::Tile::PanelEast),
        cc2::Tile::panelTile(cc2::Tile::PanelSouth),
        cc2::Tile::panelTile(cc2::Tile::PanelWest),
        cc2::Tile::panelTile(cc2::Tile::Canopy),
        DIR_TILE_LIST(cc2::Tile::Player),
        DIR_TILE_LIST(cc2::Tile::Player2),
        cc2::Tile(cc2::Tile::StyledFloor, cc2::TileModifier::CamoTheme),
        cc2::Tile(cc2::Tile::StyledWall, cc2::TileModifier::CamoTheme),
        cc2::Tile(cc2::Tile::StyledFloor, cc2::TileModifier::PinkDotsTheme),
        cc2::Tile(cc2::Tile::StyledWall, cc2::TileModifier::PinkDotsTheme),
        cc2::Tile(cc2::Tile::StyledFloor, cc2::TileModifier::YellowBrickTheme),
        cc2::Tile(cc2::Tile::StyledWall, cc2::TileModifier::YellowBrickTheme),
        cc2::Tile(cc2::Tile::StyledFloor, cc2::TileModifier::BlueTheme),
        cc2::Tile(cc2::Tile::StyledWall, cc2::TileModifier::BlueTheme),
    });
    tileBox->addItem(tileLists[ListStandard], tr("Standard"));
    tileLists[ListObstacles] = new TileListWidget(tileBox);
    tileLists[ListObstacles]->setTiles({
        cc2::Tile(cc2::Tile::Ice),
        cc2::Tile(cc2::Tile::Ice_NE),
        cc2::Tile(cc2::Tile::Ice_SE),
        cc2::Tile(cc2::Tile::Ice_SW),
        cc2::Tile(cc2::Tile::Ice_NW),
        cc2::Tile(cc2::Tile::Water),
        cc2::Tile(cc2::Tile::Fire),
        cc2::Tile(cc2::Tile::FlameJet_Off),
        cc2::Tile(cc2::Tile::FlameJet_On),
        cc2::Tile(cc2::Tile::FlameJetButton),
        cc2::Tile(cc2::Tile::Force_N),
        cc2::Tile(cc2::Tile::Force_E),
        cc2::Tile(cc2::Tile::Force_S),
        cc2::Tile(cc2::Tile::Force_W),
        cc2::Tile(cc2::Tile::Force_Rand),
        cc2::Tile(cc2::Tile::Slime),
        cc2::Tile(cc2::Tile::RedBomb),
        cc2::Tile(cc2::Tile::Trap),
        cc2::Tile(cc2::Tile::TrapButton),
        cc2::Tile(cc2::Tile::PopUpWall),
        cc2::Tile(cc2::Tile::AppearingWall),
        cc2::Tile(cc2::Tile::InvisWall),
        cc2::Tile(cc2::Tile::Turtle),
        cc2::Tile(cc2::Tile::MaleOnly),
        cc2::Tile(cc2::Tile::FemaleOnly),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_NE),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_SE),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_SW),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_NW),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_NS),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::Track_WE),
        cc2::Tile(cc2::Tile::TrainTracks, cc2::TileModifier::TrackSwitch),
    });
    tileBox->addItem(tileLists[ListObstacles], tr("Obstacles"));
    tileLists[ListDoors] = new TileListWidget(tileBox);
    tileLists[ListDoors]->setTiles({
        cc2::Tile(cc2::Tile::Door_Blue),
        cc2::Tile(cc2::Tile::Door_Yellow),
        cc2::Tile(cc2::Tile::Door_Green),
        cc2::Tile(cc2::Tile::Door_Red),
        cc2::Tile(cc2::Tile::ToggleFloor),
        cc2::Tile(cc2::Tile::ToggleWall),
        cc2::Tile(cc2::Tile::ToggleButton),
        cc2::Tile(cc2::Tile::RevolvDoor_SW),
        cc2::Tile(cc2::Tile::RevolvDoor_NW),
        cc2::Tile(cc2::Tile::RevolvDoor_NE),
        cc2::Tile(cc2::Tile::RevolvDoor_SE),
        cc2::Tile(cc2::Tile::LSwitchFloor),
        cc2::Tile(cc2::Tile::LSwitchWall),
    });
    tileBox->addItem(tileLists[ListDoors], tr("Doors"));
    tileLists[ListItems] = new TileListWidget(tileBox);
    tileLists[ListItems]->setTiles({
        cc2::Tile(cc2::Tile::Key_Blue),
        cc2::Tile(cc2::Tile::Key_Yellow),
        cc2::Tile(cc2::Tile::Key_Green),
        cc2::Tile(cc2::Tile::Key_Red),
        cc2::Tile(cc2::Tile::Flippers),
        cc2::Tile(cc2::Tile::FireShoes),
        cc2::Tile(cc2::Tile::IceCleats),
        cc2::Tile(cc2::Tile::MagnoShoes),
        cc2::Tile(cc2::Tile::HikingBoots),
        cc2::Tile(cc2::Tile::SpeedShoes),
        cc2::Tile(cc2::Tile::Helmet),
        cc2::Tile(cc2::Tile::RRSign),
        cc2::Tile(cc2::Tile::Lightning),
        cc2::Tile(cc2::Tile::TimeBomb),
        cc2::Tile(cc2::Tile::BowlingBall),
        cc2::Tile(cc2::Tile::SteelFoil),
        cc2::Tile(cc2::Tile::Hook),
        cc2::Tile(cc2::Tile::Eye),
        cc2::Tile(cc2::Tile::Bribe),
        cc2::Tile(cc2::Tile::Disallow),
    });
    tileBox->addItem(tileLists[ListItems], tr("Items"));
    tileLists[ListMonsters] = new TileListWidget(tileBox);
    tileLists[ListMonsters]->setTiles({
        DIR_TILE_LIST(cc2::Tile::Ship),
        DIR_TILE_LIST(cc2::Tile::Ant),
        DIR_TILE_LIST(cc2::Tile::Centipede),
        DIR_TILE_LIST(cc2::Tile::FireBox),
        DIR_TILE_LIST(cc2::Tile::Ball),
        DIR_TILE_LIST(cc2::Tile::Blob),
        DIR_TILE_LIST(cc2::Tile::Walker),
        DIR_TILE_LIST(cc2::Tile::AngryTeeth),
        DIR_TILE_LIST(cc2::Tile::TimidTeeth),
        DIR_TILE_LIST(cc2::Tile::BlueTank),
        cc2::Tile(cc2::Tile::TankButton),
        DIR_TILE_LIST(cc2::Tile::YellowTank),
        cc2::Tile(cc2::Tile::YellowTankCtrl),
        DIR_TILE_LIST(cc2::Tile::Ghost),
        DIR_TILE_LIST(cc2::Tile::Rover),
        DIR_TILE_LIST(cc2::Tile::FloorMimic),
        DIR_TILE_LIST(cc2::Tile::MirrorPlayer),
        DIR_TILE_LIST(cc2::Tile::MirrorPlayer2),
    });
    tileBox->addItem(tileLists[ListMonsters], tr("Monsters"));
    tileLists[ListLogic] = new TileListWidget(tileBox);
    tileLists[ListLogic]->setTiles({
        cc2::Tile(cc2::Tile::LogicButton),
        cc2::Tile(cc2::Tile::RevLogicButton),
        cc2::Tile(cc2::Tile::AreaCtlButton),
        cc2::Tile(cc2::Tile::LSwitchFloor),
        cc2::Tile(cc2::Tile::LSwitchWall),
        cc2::Tile(cc2::Tile::Teleport_Blue),
        cc2::Tile(cc2::Tile::Teleport_Red),
        cc2::Tile(cc2::Tile::Transformer),
        cc2::Tile(cc2::Tile::Switch_Off),
        cc2::Tile(cc2::Tile::Switch_On),
        DIR_LOGIC_GATE(cc2::TileModifier::Inverter),
        DIR_LOGIC_GATE(cc2::TileModifier::AndGate),
        DIR_LOGIC_GATE(cc2::TileModifier::OrGate),
        DIR_LOGIC_GATE(cc2::TileModifier::XorGate),
        DIR_LOGIC_GATE(cc2::TileModifier::NandGate),
        DIR_LOGIC_GATE(cc2::TileModifier::LatchGateCW),
        DIR_LOGIC_GATE(cc2::TileModifier::LatchGateCCW),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_0),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_1),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_2),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_3),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_4),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_5),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_6),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_7),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_8),
        cc2::Tile(cc2::Tile::LogicGate, cc2::TileModifier::CounterGate_9),
    });
    tileBox->addItem(tileLists[ListLogic], tr("Logic"));
    tileLists[ListGlyphs] = new TileListWidget(tileBox);
    std::vector<cc2::Tile> glyphTiles;
    glyphTiles.reserve(cc2::TileModifier::GlyphMAX - cc2::TileModifier::GlyphMIN);
    for (uint32_t i = cc2::TileModifier::GlyphMIN; i < cc2::TileModifier::GlyphMAX; ++i)
        glyphTiles.emplace_back(cc2::Tile::AsciiGlyph, i);
    tileLists[ListGlyphs]->setTiles(glyphTiles);
    tileBox->addItem(tileLists[ListGlyphs], tr("Glyph Tiles"));
    tileLists[ListMisc] = new TileListWidget(tileBox);
    tileLists[ListMisc]->setTiles({
        cc2::Tile(cc2::Tile::ToolThief),
        cc2::Tile(cc2::Tile::KeyThief),
        cc2::Tile(cc2::Tile::BlueWall),
        cc2::Tile(cc2::Tile::BlueFloor),
        cc2::Tile(cc2::Tile::StayUpGWall),
        cc2::Tile(cc2::Tile::PopDownGWall),
        cc2::Tile(cc2::Tile::Teleport_Blue),
        cc2::Tile(cc2::Tile::Teleport_Red),
        cc2::Tile(cc2::Tile::Teleport_Green),
        cc2::Tile(cc2::Tile::Teleport_Yellow),
        cc2::Tile(cc2::Tile::Cloner, cc2::TileModifier::CloneNorth),
        cc2::Tile(cc2::Tile::Cloner, cc2::TileModifier::CloneEast),
        cc2::Tile(cc2::Tile::Cloner, cc2::TileModifier::CloneSouth),
        cc2::Tile(cc2::Tile::Cloner, cc2::TileModifier::CloneWest),
        cc2::Tile(cc2::Tile::CloneButton),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowEast),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowSouth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowEast),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowSouth | cc2::Tile::ArrowEast),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowSouth | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowSouth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowEast | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowEast | cc2::Tile::ArrowSouth),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowSouth | cc2::Tile::ArrowEast | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowSouth | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowEast | cc2::Tile::ArrowWest),
        cc2::Tile::dirBlockTile(cc2::Tile::ArrowNorth | cc2::Tile::ArrowEast
                                | cc2::Tile::ArrowSouth | cc2::Tile::ArrowWest),
        cc2::Tile(cc2::Tile::Transformer),
        cc2::Tile(cc2::Tile::GreenChip),
        cc2::Tile(cc2::Tile::GreenBomb),
        cc2::Tile(cc2::Tile::TimeBonus),
        cc2::Tile(cc2::Tile::TimePenalty),
        cc2::Tile(cc2::Tile::ToggleClock),
        cc2::Tile(cc2::Tile::Flag10),
        cc2::Tile(cc2::Tile::Flag100),
        cc2::Tile(cc2::Tile::Flag1000),
        cc2::Tile(cc2::Tile::Flag2x),
    });
    tileBox->addItem(tileLists[ListMisc], tr("Miscellaneous"));

    for (auto listWidget : tileLists) {
        connect(listWidget, &TileListWidget::itemSelectedLeft, this, &CC2EditMain::setForeground);
        connect(listWidget, &TileListWidget::itemSelectedRight, this, &CC2EditMain::setBackground);
        connect(this, &CC2EditMain::tilesetChanged, listWidget, &TileListWidget::setTileImages);
    }

    auto layerWidget = new LayerWidget(sortedTiles);
    auto foreLabel = new QLabel(tr("Foreground: "), sortedTiles);
    auto foreTileLabel = new QLabel(sortedTiles);
    auto backLabel = new QLabel(tr("Background: "), sortedTiles);
    auto backTileLabel = new QLabel(sortedTiles);
    foreTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    backTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(this, &CC2EditMain::tilesetChanged, layerWidget, &LayerWidget::setTileset);
    connect(this, &CC2EditMain::foregroundChanged, layerWidget, &LayerWidget::setUpper);
    connect(this, &CC2EditMain::backgroundChanged, layerWidget, &LayerWidget::setLower);
    connect(this, &CC2EditMain::foregroundChanged, this, [foreTileLabel](const cc2::Tile* tile) {
        foreTileLabel->setText(CC2ETileset::getName(tile));
    });
    connect(this, &CC2EditMain::backgroundChanged, this, [backTileLabel](const cc2::Tile* tile) {
        backTileLabel->setText(CC2ETileset::getName(tile));
    });

    auto tileLayout = new QGridLayout(sortedTiles);
    tileLayout->setContentsMargins(4, 4, 4, 4);
    tileLayout->setVerticalSpacing(4);
    tileLayout->addWidget(tileBox, 0, 0, 1, 3);
    tileLayout->addWidget(foreLabel, 1, 0);
    tileLayout->addWidget(foreTileLabel, 1, 1);
    tileLayout->addWidget(backLabel, 2, 0);
    tileLayout->addWidget(backTileLabel, 2, 1);
    tileLayout->addWidget(layerWidget, 1, 2, 2, 1);
    m_toolTabs->addTab(sortedTiles, tr("&Tiles - Sorted"));

    // Editor area
    m_editorTabs = new EditorTabWidget(this);
    setCentralWidget(m_editorTabs);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNewMap]);
    fileMenu->addAction(m_actions[ActionNewScript]);
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
    toolsMenu->addAction(m_actions[ActionToggleWalls]);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_actions[ActionViewActivePlayer]);
    viewMenu->addAction(m_actions[ActionViewViewport]);
    viewMenu->addAction(m_actions[ActionViewMonsterPaths]);
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
    testMenu->addAction(m_actions[ActionTest]);
    testMenu->addSeparator();
    testMenu->addAction(m_actions[ActionTestSetup]);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_actions[ActionAbout]);

    // Tool bars
    QToolBar* tbarMain = addToolBar(QString());
    tbarMain->setObjectName("ToolbarMain");
    tbarMain->setWindowTitle(tr("Main"));
    tbarMain->addAction(m_actions[ActionNewMap]);
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
    tbarTools->addAction(m_actions[ActionToggleWalls]);

    // Show status bar
    statusBar();

    connect(m_actions[ActionNewMap], &QAction::triggered, this, &CC2EditMain::createNewMap);
    connect(m_actions[ActionNewScript], &QAction::triggered, this, &CC2EditMain::createNewScript);
    connect(m_actions[ActionOpen], &QAction::triggered, this, &CC2EditMain::onOpenAction);
    connect(m_actions[ActionClose], &QAction::triggered, this, &CC2EditMain::onCloseAction);

    connect(m_tilesetGroup, &QActionGroup::triggered, this, &CC2EditMain::onTilesetMenu);
    connect(m_actions[ActionZoom100], &QAction::triggered, this, [this] { setZoomFactor(1.0); });
    connect(m_actions[ActionZoom75], &QAction::triggered, this, [this] { setZoomFactor(0.75); });
    connect(m_actions[ActionZoom50], &QAction::triggered, this, [this] { setZoomFactor(0.5); });
    connect(m_actions[ActionZoom25], &QAction::triggered, this, [this] { setZoomFactor(0.25); });
    connect(m_actions[ActionZoom125], &QAction::triggered, this, [this] { setZoomFactor(0.125); });
    connect(m_actions[ActionZoomCust], &QAction::triggered, this, &CC2EditMain::onZoomCust);
    connect(m_actions[ActionZoomFit], &QAction::triggered, this, &CC2EditMain::onZoomFit);

    connect(m_actions[ActionTest], &QAction::triggered, this, &CC2EditMain::onTestChips2);
    connect(m_actions[ActionTestSetup], &QAction::triggered, this, [] {
        TestSetupDialog dlg;
        dlg.exec();
    });

    connect(m_actions[ActionAbout], &QAction::triggered, this, [this] {
        AboutDialog about(this);
        about.exec();
    });

    connect(m_actions[ActionReloadScript], &QAction::triggered, this, [this] {
        if (!m_currentGameScript.isEmpty())
            loadScript(m_currentGameScript);
    });
    connect(m_actions[ActionEditScript], &QAction::triggered, this, [this] {
        if (!m_currentGameScript.isEmpty())
            editScript(m_currentGameScript);
    });

    connect(toolDock, &QDockWidget::dockLocationChanged, this, &CC2EditMain::onDockChanged);

    connect(m_editorTabs, &QTabWidget::tabCloseRequested, this, &CC2EditMain::onCloseTab);
    connect(m_editorTabs, &QTabWidget::currentChanged, this, &CC2EditMain::onTabChanged);

    // Load window settings and defaults
    QSettings settings("CCTools", "CC2Edit");
    resize(settings.value("WindowSize", QSize(1024, 768)).toSize());
    if (settings.value("WindowMaximized", false).toBool())
        showMaximized();
    if (settings.contains("WindowState"))
        restoreState(settings.value("WindowState").toByteArray());
    m_zoomFactor = settings.value("ZoomFactor", 1.0).toDouble();
    m_actions[ActionViewActivePlayer]->setChecked(settings.value("ViewActivePlayer", false).toBool());
    m_actions[ActionViewViewport]->setChecked(settings.value("ViewViewport", true).toBool());
    m_actions[ActionViewMonsterPaths]->setChecked(settings.value("ViewMonsterPaths", false).toBool());
    m_dialogDir = settings.value("DialogDir").toString();

    // Make sure the toolbox is visible
    if (toolDock->isFloating()) {
        QPoint dockPos = toolDock->pos();
        QDesktopWidget* desktop = QApplication::desktop();
        if ((dockPos.x() + toolDock->width() - 10) < desktop->contentsRect().left())
            dockPos.setX(desktop->contentsRect().left());
        if (dockPos.x() + 10 > desktop->contentsRect().right())
            dockPos.setX(desktop->contentsRect().right() - toolDock->width());
        if (dockPos.y() < desktop->contentsRect().top())
            dockPos.setY(desktop->contentsRect().top());
        if (dockPos.y() + 10 > desktop->contentsRect().bottom())
            dockPos.setY(desktop->contentsRect().bottom() - toolDock->height());
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
            CC2ETileset* tileset = m_tilesetGroup->actions()[i]->data().value<CC2ETileset*>();
            if (tileset->filename() == tilesetFilename) {
                m_tilesetGroup->actions()[i]->setChecked(true);
                loadTileset(tileset);
                foundTset = true;
                break;
            }
        }
        if (!foundTset) {
            m_tilesetGroup->actions()[0]->setChecked(true);
            loadTileset(m_tilesetGroup->actions()[0]->data().value<CC2ETileset*>());
        }
    } else {
        m_tilesetGroup->actions()[0]->setChecked(true);
        loadTileset(m_tilesetGroup->actions()[0]->data().value<CC2ETileset*>());
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

    // Set default editor tiles
    cc2::Tile defTile(cc2::Tile::Wall);
    setForeground(&defTile);
    defTile.set(cc2::Tile::Floor);
    setBackground(&defTile);
}

void CC2EditMain::createNewMap()
{
    auto map = new cc2::Map;
    map->mapData().resize(32, 32);
    addEditor(map, QString());
    map->unref();

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionSelect]->setEnabled(true);
    m_actions[ActionTest]->setEnabled(true);
}

void CC2EditMain::createNewScript()
{
    addScriptEditor(QString());

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionSelect]->setEnabled(false);
    m_actions[ActionTest]->setEnabled(true);
}

void CC2EditMain::loadFile(const QString& filename)
{
    QFileInfo info(filename);
    if (info.suffix().compare(QLatin1String("c2g"), Qt::CaseInsensitive) == 0) {
        loadScript(filename);
        m_toolTabs->setCurrentWidget(m_gameProperties);
    } else if (info.suffix().compare(QLatin1String("c2m"), Qt::CaseInsensitive) == 0) {
        loadMap(filename);
        m_toolTabs->setCurrentWidget(m_mapProperties);
    } else {
        QMessageBox::critical(this, tr("Invalid filename"),
                              tr("Unsupported file type for %1").arg(filename));
    }
}

void CC2EditMain::loadMap(const QString& filename)
{
    QFileInfo info(filename);
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2EditorWidget* editor = getEditorAt(i);
        if (editor && editor->filename() == info.canonicalFilePath()) {
            m_editorTabs->setCurrentIndex(i);
            return;
        }
    }

    ccl::FileStream fs;
    if (!fs.open(filename.toLocal8Bit().constData(), "rb")) {
        QMessageBox::critical(this, tr("Error loading map"),
                tr("Could not open %1 for reading.").arg(filename));
        return;
    }

    auto map = new cc2::Map;
    try {
        map->read(&fs);
        addEditor(map, filename);
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, tr("Error loading map"), ex.what());
    }
    map->unref();

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
    m_actions[ActionSelect]->setEnabled(true);
    m_actions[ActionTest]->setEnabled(true);
}

void CC2EditMain::loadScript(const QString& filename)
{
    m_gameMapList->clear();
    m_gameName->setText(QString());

    ScriptMapLoader mapLoader;
    connect(&mapLoader, &ScriptMapLoader::gameName, m_gameName, &QLabel::setText);
    connect(&mapLoader, &ScriptMapLoader::mapAdded, this, [this](const QString &filename) {
        cc2::Map map;
        ccl::FileStream fs;
        if (fs.open(filename.toLocal8Bit().constData(), "rb")) {
            try {
                map.read(&fs);
            } catch (const std::exception &err) {
                QMessageBox::critical(this, tr("Error processing map"),
                                      tr("Failed to load map data for %1: %2")
                                              .arg(filename).arg(err.what()));
            }
        }
        QString title = !map.title().empty() ? QString::fromStdString(map.title())
                                             : QFileInfo(filename).fileName();

        QString name = tr("%1 - %2").arg(m_gameMapList->count() + 1).arg(title);
        auto item = new QListWidgetItem(name, m_gameMapList);
        item->setData(Qt::UserRole, filename);
    });
    if (mapLoader.loadScript(filename)) {
        m_currentGameScript = filename;
        m_gameProperties->setEnabled(true);
    }
}

void CC2EditMain::editScript(const QString& filename)
{
    QFileInfo info(filename);
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2ScriptEditor* editor = getScriptEditorAt(i);
        if (editor && editor->filename() == info.canonicalFilePath()) {
            m_editorTabs->setCurrentIndex(i);
            return;
        }
    }

    QFile scriptFile(filename);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error opening file"),
                tr("Could not open %1 for reading").arg(filename));
        return;
    }
    QString text = QString::fromLatin1(scriptFile.readAll());
    auto editor = addScriptEditor(filename);
    editor->setPlainText(text);

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionSelect]->setEnabled(false);
    m_actions[ActionTest]->setEnabled(true);
}

void CC2EditMain::registerTileset(const QString& filename)
{
    auto tileset = new CC2ETileset(this);
    try {
        tileset->load(filename);
    } catch (const ccl::IOException&) {
        // CC1 tileset or invalid file -- skip it
        delete tileset;
        return;
    }

    QAction* menuItem = m_tilesetMenu->addAction(tileset->name());
    menuItem->setCheckable(true);
    menuItem->setStatusTip(tileset->description());
    menuItem->setData(QVariant::fromValue(tileset));
    m_tilesetGroup->addAction(menuItem);
}

void CC2EditMain::findTilesets()
{
    m_tilesetMenu->clear();

    QDir path;
    QStringList tilesets;
#if defined(Q_OS_WIN)
    // Search app directory
    path.setPath(QCoreApplication::applicationDirPath());
    tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
    for (const QString& file : qAsConst(tilesets))
        registerTileset(path.absoluteFilePath(file));
#else
    // Search install path
    path.setPath(QCoreApplication::applicationDirPath());
    path.cdUp();
    path.cd("share/cctools");
    tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
    for (const QString& file : qAsConst(tilesets))
        registerTileset(path.absoluteFilePath(file));

    // Search standard directories
    path.setPath("/usr/share/cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        for (const QString& file : qAsConst(tilesets))
            registerTileset(path.absoluteFilePath(file));
    }
    path.setPath("/usr/local/share/cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        for (const QString& file : qAsConst(tilesets))
            registerTileset(path.absoluteFilePath(file));
    }
#endif

    // User-space local data
    path.setPath(QDir::homePath());
    path.cd(".cctools");
    if (path.exists()) {
        tilesets = path.entryList(QStringList("*.tis"), QDir::Files | QDir::Readable, QDir::Name);
        for (const QString& file : qAsConst(tilesets))
            registerTileset(path.absoluteFilePath(file));
    }
}

void CC2EditMain::loadTileset(CC2ETileset* tileset)
{
    m_currentTileset = tileset;
    emit tilesetChanged(tileset);
}

CC2EditorWidget* CC2EditMain::getEditorAt(int idx)
{
    if (idx < 0 || idx >= m_editorTabs->count())
        return nullptr;

    auto scroll = qobject_cast<QScrollArea *>(m_editorTabs->widget(idx));
    if (scroll)
        return qobject_cast<CC2EditorWidget*>(scroll->widget());
    return nullptr;
}

CC2ScriptEditor* CC2EditMain::getScriptEditorAt(int idx)
{
    if (idx < 0 || idx >= m_editorTabs->count())
        return nullptr;
    return qobject_cast<CC2ScriptEditor *>(m_editorTabs->widget(idx));
}

CC2EditorWidget* CC2EditMain::addEditor(cc2::Map* map, const QString& filename)
{
    QScrollArea* scroll = new QScrollArea(m_editorTabs);
    CC2EditorWidget* editor = new CC2EditorWidget(scroll);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidget(editor);
    // TODO: Set render flags
    editor->setTileset(m_currentTileset);
    editor->setMap(map);
    if (m_zoomFactor != 0.0)
        editor->setZoom(m_zoomFactor);

    if (filename.isEmpty()) {
        m_editorTabs->addTab(scroll, tr("Untitled Map"));
    } else {
        QFileInfo info(filename);
        editor->setFilename(info.canonicalFilePath());
        m_editorTabs->addTab(scroll, info.fileName());
    }
    resizeEvent(nullptr);

    connect(editor, &CC2EditorWidget::mouseInfo, statusBar(), &QStatusBar::showMessage);
    connect(editor, &CC2EditorWidget::canUndo, m_actions[ActionUndo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::canRedo, m_actions[ActionRedo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCut], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCopy], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionClear], &QAction::setEnabled);
    //connect(editor, &CC2EditorWidget::makeDirty, this, [map] { map->makeDirty(); });

    connect(this, &CC2EditMain::tilesetChanged, editor, &CC2EditorWidget::setTileset);

    m_editorTabs->setCurrentWidget(scroll);
    return editor;
}

CC2ScriptEditor* CC2EditMain::addScriptEditor(const QString& filename)
{
    auto editor = new CC2ScriptEditor(m_editorTabs);
    if (filename.isEmpty()) {
        m_editorTabs->addTab(editor, tr("Untitled Script"));
    } else {
        QFileInfo info(filename);
        editor->setFilename(info.canonicalFilePath());
        m_editorTabs->addTab(editor, info.fileName());
    }

    // TODO: Hook up signals

    m_editorTabs->setCurrentWidget(editor);
    return editor;
}

void CC2EditMain::closeAllTabs()
{
    while (m_editorTabs->count() != 0) {
        delete m_editorTabs->widget(0);
        m_editorTabs->removeTab(0);
    }

    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionSelect]->setEnabled(false);
    m_actions[ActionTest]->setEnabled(false);
}

void CC2EditMain::resizeEvent(QResizeEvent* event)
{
    if (event)
        QWidget::resizeEvent(event);

    if (m_zoomFactor == 0.0) {
        auto scroll = qobject_cast<QScrollArea*>(m_editorTabs->currentWidget());
        auto editor = getEditorAt(m_editorTabs->currentIndex());
        if (scroll && editor) {
            const auto& map = editor->map();
            QSize zmax = scroll->maximumViewportSize();
            double zx = (double)zmax.width() / (map->mapData().width() * m_currentTileset->size());
            double zy = (double)zmax.height() / (map->mapData().height() * m_currentTileset->size());
            editor->setZoom(std::min(zx, zy));
        }
    }
}

void CC2EditMain::onOpenAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Map..."),
                            m_dialogDir, tr("All supported files (*.c2m *.c2g);;"
                                            "CC2 Maps (*.c2m);;"
                                            "CC2 Game Scripts (*.c2g)"));
    if (!filename.isEmpty()) {
        loadFile(filename);
        QDir dir(filename);
        dir.cdUp();
        m_dialogDir = dir.absolutePath();
    }
}

void CC2EditMain::onCloseAction()
{
    if (m_editorTabs->count())
        onCloseTab(m_editorTabs->currentIndex());
}

void CC2EditMain::setZoomFactor(double zoom)
{
    m_zoomFactor = zoom;
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        auto editor = getEditorAt(i);
        if (editor)
            editor->setZoom(m_zoomFactor);
    }
}

void CC2EditMain::onZoomCust()
{
    bool ok;
    double zoom = QInputDialog::getDouble(this, tr("Set Custom Zoom"),
                        tr("Custom Zoom Percentage"), m_zoomFactor * 100.0,
                        2.5, 800.0, 2, &ok);
    if (ok)
        setZoomFactor(zoom / 100.0);
    if (m_zoomFactor == 1.0)
        m_actions[ActionZoom100]->setChecked(true);
    else if (m_zoomFactor == 0.5)
        m_actions[ActionZoom50]->setChecked(true);
    else if (m_zoomFactor == 0.25)
        m_actions[ActionZoom25]->setChecked(true);
    else if (m_zoomFactor == 0.125)
        m_actions[ActionZoom125]->setChecked(true);
}

void CC2EditMain::onZoomFit()
{
    m_zoomFactor = 0.0;
    resizeEvent(nullptr);
}

void CC2EditMain::onTilesetMenu(QAction* which)
{
    auto tileset = which->data().value<CC2ETileset*>();
    loadTileset(tileset);
}

void CC2EditMain::onTestChips2()
{
    auto editor = getEditorAt(m_editorTabs->currentIndex());
    if (!editor)
        return;

    if (m_subProc) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CCEdit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings("CCTools", "CC2Edit");
    QString chips2Exe = settings.value("Chips2Exe").toString();
    if (chips2Exe.isEmpty() || !QFile::exists(chips2Exe)) {
        QMessageBox::critical(this, tr("Could not find Chips2 executable"),
                tr("Could not find Chip's Challenge 2 executable.\n"
                   "Please configure Chips2 in the Test Setup dialog."));
        return;
    }
#ifndef Q_OS_WIN
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

    QDir chips2Dir(chips2Exe);
    chips2Dir.cdUp();
    m_testGameDir = chips2Dir.absolutePath();
    if (!chips2Dir.cd("data/games")) {
        QMessageBox::critical(this, tr("Could not find game data"),
                tr("Could not find game data relative to Chips2 executable."));
        return;
    }

    if (chips2Dir.exists("CC2Edit-playtest") && chips2Dir.cd("CC2Edit-playtest")) {
        chips2Dir.removeRecursively();
        chips2Dir.cdUp();
    }
    if (!chips2Dir.mkdir("CC2Edit-playtest")) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest directory in %1.  Do you have write access?")
                .arg(chips2Dir.absoluteFilePath("CC2Edit-playtest")));
        return;
    }
    chips2Dir.cd("CC2Edit-playtest");
    QFile testScript(chips2Dir.absoluteFilePath("playtest.c2g"));
    if (!testScript.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest script at %1.  Do you have write access?")
                .arg(chips2Dir.absoluteFilePath("playtest.c2g")));
        return;
    }
    testScript.write("game \"CC2Edit-playtest\"\n"
                     "0 flags =\n"
                     "0 score =\n"
                     "0 hispeed =\n"
                     "1 level =\n"
                     "map \"playtest.c2m\"\n");
    testScript.close();

    ccl::FileStream fs;
    if (!fs.open(chips2Dir.absoluteFilePath("playtest.c2m").toLocal8Bit().constData(), "wb")) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest map at %1.  Do you have write access?")
                .arg(chips2Dir.absoluteFilePath("playtest.c2m")));
        return;
    }
    editor->map()->write(&fs);
    fs.close();

    // Write a save and high score file, so we can jump directly into the map.
    cc2::CC2Save save;
    save.saveData().setLevel(1);
    save.saveData().setLine(2);    // After the "game" directive
    //save.saveData().setChecksum(what?);
    save.setFilename("playtest.c2g");
    save.setGamePath(QDir::toNativeSeparators(chips2Dir.absolutePath()).toStdString());

    cc2::CC2HighScore highScore;
    highScore.scores().emplace_back();
    auto& levelScore = highScore.scores().back();
    levelScore.setFilename("playtest.c2m");
    levelScore.setGameType("CC2Edit-playtest");
    levelScore.setTitle(editor->map()->title());
    levelScore.saveData().setLevel(1);
    levelScore.saveData().setLine(6);   // The line containing the "map" directive
    //levelScore.saveData().setChecksum(what?);

    QDir savesDir(m_testGameDir);
    if (!savesDir.cd("data/saves")) {
        QMessageBox::critical(this, tr("Could not find save data"),
                              tr("Could not find save data relative to Chips2 executable."));
        return;
    }

    if (savesDir.exists("CC2Edit-playtest") && savesDir.cd("CC2Edit-playtest")) {
        savesDir.removeRecursively();
        savesDir.cdUp();
    }
    if (!savesDir.mkdir("CC2Edit-playtest")) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest saves directory in %1.  Do you have write access?")
                .arg(savesDir.absoluteFilePath("CC2Edit-playtest")));
        return;
    }
    savesDir.cd("CC2Edit-playtest");

    if (!fs.open(savesDir.absoluteFilePath("save.c2s").toLocal8Bit().constData(), "wb")) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest save data at %1.  Do you have write access?")
                .arg(savesDir.absoluteFilePath("save.c2s")));
        return;
    }
    save.write(&fs);
    fs.close();

    if (!fs.open(savesDir.absoluteFilePath("save.c2h").toLocal8Bit().constData(), "wb")) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest highscore data at %1.  Do you have write access?")
                .arg(savesDir.absoluteFilePath("save.c2h")));
        return;
    }
    highScore.write(&fs);
    fs.close();

    // Indicate the currently active game
    chips2Dir.cdUp();
    if (chips2Dir.exists("save.c2l") && !chips2Dir.exists("save.c2l.CC2Edit-backup"))
        chips2Dir.rename("save.c2l", "save.c2l.CC2Edit-backup");
    QFile listFile(chips2Dir.absoluteFilePath("save.c2l"));
    if (!listFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create file at %1.  Do you have write access?")
                .arg(chips2Dir.absoluteFilePath("save.c2l")));
        return;
    }
    listFile.write("CC2Edit-playtest/save.c2s");
    listFile.close();

    QString cwd = QDir::currentPath();
    chips2Dir.setPath(m_testGameDir);
    if (chips2Dir.exists("steam_api.dll") && !chips2Dir.exists("steam_appid.txt")) {
        // This enables the game to work without being launched from Steam
        QFile appidFile(chips2Dir.absoluteFilePath("steam_appid.txt"));
        if (!appidFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error setting up playtest"),
                    tr("Could not write steam_appid.txt.  Do you have write access?"));
            return;
        }
        if (chips2Exe.contains(QLatin1String("chips2"), Qt::CaseInsensitive)) {
            // Chip's Challenge 2
            appidFile.write("348300");
        } else if (chips2Exe.contains(QLatin1String("chips1"), Qt::CaseInsensitive)) {
            // Chip's Challenge 1
            appidFile.write("346850");
        }
        appidFile.close();
    }

    QDir::setCurrent(chips2Dir.absolutePath());
    m_subProc = new QProcess(this);
    connect(m_subProc, SIGNAL(finished(int)), this, SLOT(onProcessFinished(int)));
    connect(m_subProc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onProcessError(QProcess::ProcessError)));
    m_subProc->start(chips2Exe, QStringList());
    QDir::setCurrent(cwd);
}

void CC2EditMain::onDockChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::RightDockWidgetArea)
        m_toolTabs->setTabPosition(QTabWidget::East);
    else
        m_toolTabs->setTabPosition(QTabWidget::West);
}

void CC2EditMain::onCloseTab(int index)
{
    const bool lastTab = m_editorTabs->count() == 1;
    m_editorTabs->widget(index)->deleteLater();

    if (lastTab) {
        m_actions[ActionSave]->setEnabled(false);
        m_actions[ActionSaveAs]->setEnabled(false);
        m_actions[ActionClose]->setEnabled(false);
        m_actions[ActionGenReport]->setEnabled(false);
        m_actions[ActionSelect]->setEnabled(false);
        m_actions[ActionTest]->setEnabled(false);
    }
}

void CC2EditMain::onTabChanged(int index)
{
    CC2EditorWidget* mapEditor = getEditorAt(index);
    if (!mapEditor) {
        m_title->setText(QString());
        m_author->setText(QString());
        m_lockText->setText(QString());
        m_editorVersion->setText(QString());
        m_mapSize->setText(QString());
        m_chipCounter->setText(QString());
        m_pointCounter->setText(QString());
        m_timeLimit->setValue(0);
        m_viewport->setCurrentIndex(0);
        m_blobPattern->setCurrentIndex(0);
        m_hideLogic->setChecked(false);
        m_cc1Boots->setChecked(false);
        m_readOnly->setChecked(false);
        m_clue->setPlainText(QString());
        m_note->setPlainText(QString());
        m_mapProperties->setEnabled(false);

        CC2ScriptEditor* scriptEditor = getScriptEditorAt(index);
        m_actions[ActionSelect]->setEnabled(scriptEditor == nullptr);
        return;
    }

    mapEditor->update();
    //mapEditor->updateUndoStatus();

    /*
    bool hasSelection = editor->selection() != QRect(-1, -1, -1, -1);
    m_actions[ActionCut]->setEnabled(hasSelection);
    m_actions[ActionCopy]->setEnabled(hasSelection);
    m_actions[ActionClear]->setEnabled(hasSelection);
    */
    m_actions[ActionSelect]->setEnabled(true);

    // Update the map properties page
    auto map = mapEditor->map();
    m_mapProperties->setEnabled(true);
    m_title->setText(QString::fromStdString(map->title()));
    m_author->setText(QString::fromStdString(map->author()));
    m_lockText->setText(QString::fromStdString(map->lock()));
    m_editorVersion->setText(QString::fromStdString(map->editorVersion()));
    m_mapSize->setText(tr("%1 x %2").arg(map->mapData().width())
                                    .arg(map->mapData().height()));
    m_chipCounter->setText(QString::number(map->mapData().countChips()));
    const auto points = map->mapData().countPoints();
    if (std::get<1>(points) != 1) {
        m_pointCounter->setText(tr("%1 (x%2)").arg(std::get<0>(points))
                                              .arg(std::get<1>(points)));
    } else {
        m_pointCounter->setText(QString::number(std::get<0>(points)));
    }
    m_timeLimit->setValue(map->option().timeLimit());
    m_viewport->setCurrentIndex(static_cast<int>(map->option().view()));
    m_blobPattern->setCurrentIndex(static_cast<int>(map->option().blobPattern()));
    m_hideLogic->setChecked(map->option().hideLogic());
    m_cc1Boots->setChecked(map->option().cc1Boots());
    m_readOnly->setChecked(map->readOnly());
    m_clue->setPlainText(QString::fromStdString(map->clue()));
    m_note->setPlainText(QString::fromStdString(map->note()));

    // Apply zoom
    resizeEvent(nullptr);
}

void CC2EditMain::setForeground(const cc2::Tile* tile)
{
    m_foreground = *tile;
    emit foregroundChanged(tile);
}

void CC2EditMain::setBackground(const cc2::Tile* tile)
{
    m_background = *tile;
    emit backgroundChanged(tile);
}

void CC2EditMain::onProcessError(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart) {
        QMessageBox::critical(this, tr("Error starting process"),
                tr("Error starting test process.  Please check your settings "
                   "and try again"), QMessageBox::Ok);
    }
    onProcessFinished(-1);
}

void CC2EditMain::onProcessFinished(int)
{
    // Clean up temporary files
    QDir chips2Dir(m_testGameDir);
    if (chips2Dir.exists("data/games/CC2Edit-playtest") && chips2Dir.cd("data/games/CC2Edit-playtest")) {
        chips2Dir.removeRecursively();
        chips2Dir.cd("../../..");
    }
    if (chips2Dir.exists("data/saves/CC2Edit-playtest") && chips2Dir.cd("data/saves/CC2Edit-playtest")) {
        chips2Dir.removeRecursively();
        chips2Dir.cd("../../..");
    }
    if (chips2Dir.exists("data/games") && chips2Dir.cd("data/games")) {
        if (chips2Dir.exists("save.c2l") && chips2Dir.exists("save.c2l.CC2Edit-backup")) {
            chips2Dir.remove("save.c2l");
            chips2Dir.rename("save.c2l.CC2Edit-backup", "save.c2l");
        }
        chips2Dir.cd("../..");
    }

    // Clean up subproc
    m_subProc->disconnect();
    m_subProc->deleteLater();
    m_subProc = nullptr;
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QIcon appicon(":/icons/boot-32.png");
    appicon.addFile(":/icons/boot-24.png");
    appicon.addFile(":/icons/boot-16.png");
    app.setWindowIcon(appicon);

    CC2EditMain win;
    win.show();

    QStringList qtArgs = app.arguments();
    if (qtArgs.size() > 1)
        win.loadFile(qtArgs.at(1));
    else
        win.createNewMap();

    return app.exec();
}
