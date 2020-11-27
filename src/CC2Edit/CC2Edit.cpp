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
#include "TileWidgets.h"
#include "TileInspector.h"
#include "ScriptTools.h"
#include "TestSetup.h"
#include "ImportDialog.h"
#include "ResizeDialog.h"
#include "HintEdit.h"
#include "libcc1/Levelset.h"
#include "libcc2/GameLogic.h"
#include "CommonWidgets/CCTools.h"
#include "CommonWidgets/EditorTabWidget.h"

#include <QApplication>
#include <QDesktopServices>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QClipboard>
#include <QMimeData>
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
#include <QProgressDialog>
#include <QTextBlock>
#include <QElapsedTimer>

Q_DECLARE_METATYPE(CC2ETileset*)

enum TileListId {
    ListStandard, ListObstacles, ListDoors, ListItems, ListMonsters,
    ListLogic, ListGlyphs, ListMisc, ListAdvanced, NUM_TILE_LISTS
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
    : QMainWindow(parent), m_currentTileset(),  m_savedDrawMode(ActionDrawPencil),
      m_currentDrawMode(CC2EditorWidget::DrawPencil), m_subProc()
{
    setWindowTitle(QStringLiteral("CC2Edit " CCTOOLS_VERSION));

    // Actions
    m_actions[ActionNewMap] = new QAction(ICON("document-new"), tr("&New Map..."), this);
    m_actions[ActionNewMap]->setStatusTip(tr("Create new map"));
    m_actions[ActionNewMap]->setShortcut(Qt::Key_F2);
    m_actions[ActionNewScript] = new QAction(ICON("document-new"), tr("N&ew Script..."), this);
    m_actions[ActionNewScript]->setStatusTip(tr("Create new game script"));
    m_actions[ActionNewScript]->setShortcut(Qt::SHIFT | Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(ICON("document-open"), tr("&Open..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a game file from disk"));
    m_actions[ActionOpen]->setShortcut(Qt::CTRL | Qt::Key_O);
    m_actions[ActionImportCC1] = new QAction(ICON("document-open"), tr("&Import CC1 Map..."), this);
    m_actions[ActionImportCC1]->setStatusTip(tr("Import a map from a CC1 levelset"));
    m_actions[ActionImportCC1]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_O);
    m_actions[ActionSave] = new QAction(ICON("document-save"), tr("&Save"), this);
    m_actions[ActionSave]->setStatusTip(tr("Save the current document to the same file"));
    m_actions[ActionSave]->setShortcut(Qt::CTRL | Qt::Key_S);
    m_actions[ActionSave]->setEnabled(false);
    m_actions[ActionSaveAs] = new QAction(ICON("document-save-as"), tr("Save &As..."), this);
    m_actions[ActionSaveAs]->setStatusTip(tr("Save the current document to a new file or location"));
    m_actions[ActionSaveAs]->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    m_actions[ActionSaveAs]->setEnabled(false);
    m_actions[ActionClose] = new QAction(tr("&Close Game"), this);
    m_actions[ActionClose]->setStatusTip(tr("Close the currently open game"));
    m_actions[ActionClose]->setShortcut(Qt::CTRL | Qt::Key_W);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport] = new QAction(tr("Generate &Report"), this);
    m_actions[ActionGenReport]->setStatusTip(tr("Generate an HTML report of the current game"));
    m_actions[ActionGenReport]->setEnabled(false);
    m_actions[ActionExit] = new QAction(ICON("application-exit"), tr("E&xit"), this);
    m_actions[ActionExit]->setStatusTip(tr("Close CC2Edit"));

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
    m_actions[ActionDrawWire] = new QAction(ICON("draw-wire"), tr("Draw &Wires"), this);
    m_actions[ActionDrawWire]->setStatusTip(tr("Draw logic wires"));
    m_actions[ActionDrawWire]->setShortcut(Qt::CTRL | Qt::Key_T);
    m_actions[ActionDrawWire]->setCheckable(true);
    m_actions[ActionInspectHints] = new QAction(ICON("draw-hints"), tr("Edit &Hints"), this);
    m_actions[ActionInspectHints]->setStatusTip(tr("Directly edit hint tiles"));
    m_actions[ActionInspectHints]->setShortcut(Qt::CTRL | Qt::Key_H);
    m_actions[ActionInspectHints]->setCheckable(true);
    m_actions[ActionInspectTiles] = new QAction(ICON("draw-inspect"), tr("&Inspect Tiles"), this);
    m_actions[ActionInspectTiles]->setStatusTip(tr("Inspect tiles and make advanced modifications"));
    m_actions[ActionInspectTiles]->setShortcut(Qt::CTRL | Qt::Key_I);
    m_actions[ActionInspectTiles]->setCheckable(true);
    m_actions[ActionToggleGreens] = new QAction(ICON("cctools-gbutton"), tr("&Toggle "), this);
    m_actions[ActionToggleGreens]->setStatusTip(tr("Toggle all toggle doors and chips in the current level"));
    m_actions[ActionToggleGreens]->setShortcut(Qt::CTRL | Qt::Key_G);
    m_actions[ActionToggleGreens]->setEnabled(false);
    m_drawModeGroup = new QActionGroup(this);
    m_drawModeGroup->addAction(m_actions[ActionDrawPencil]);
    m_drawModeGroup->addAction(m_actions[ActionDrawLine]);
    m_drawModeGroup->addAction(m_actions[ActionDrawFill]);
    m_drawModeGroup->addAction(m_actions[ActionDrawFlood]);
    m_drawModeGroup->addAction(m_actions[ActionPathMaker]);
    m_actions[ActionDrawPencil]->setChecked(true);
    m_modalToolGroup = new QActionGroup(this);
    m_modalToolGroup->setExclusive(false);
    m_modalToolGroup->addAction(m_actions[ActionSelect]);
    m_modalToolGroup->addAction(m_actions[ActionDrawWire]);
    m_modalToolGroup->addAction(m_actions[ActionInspectHints]);
    m_modalToolGroup->addAction(m_actions[ActionInspectTiles]);
    m_drawModeGroup->setEnabled(false);
    m_modalToolGroup->setEnabled(false);

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

    m_actions[ActionAbout] = new QAction(ICON("help-about"), tr("&About CC2Edit"), this);
    m_actions[ActionAbout]->setStatusTip(tr("Show information about CC2Edit"));
    m_actions[ActionScriptHelp] = new QAction(tr("&C2G Scripting Reference"), this);
    m_actions[ActionScriptHelp]->setStatusTip(tr("Open the C2G scripting reference wiki (external)"));

    m_actions[ActionReloadScript] = new QAction(ICON("view-refresh"),
                                                tr("&Reload Game Script"), this);
    m_actions[ActionReloadScript]->setStatusTip(tr("Re-load the current game script from disk"));
    m_actions[ActionEditScript] = new QAction(ICON("document-properties"),
                                              tr("&Edit Game Script"), this);
    m_actions[ActionEditScript]->setStatusTip(tr("Open the current game script for editing"));

    // Control Toolbox
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);

    m_gameProperties = new QWidget(this);
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

    connect(m_gameMapList, &QListWidget::currentRowChanged, this, [this](int row) {
        loadEditorForItem(m_gameMapList->item(row));
    });
    connect(m_gameMapList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        loadEditorForItem(item);
        m_editorTabs->promoteTab();
    });

    auto gamePropsLayout = new QGridLayout(m_gameProperties);
    gamePropsLayout->setContentsMargins(4, 4, 4, 4);
    gamePropsLayout->setVerticalSpacing(4);
    gamePropsLayout->setHorizontalSpacing(4);
    gamePropsLayout->addWidget(tbarGameScript, 0, 0);
    gamePropsLayout->addWidget(m_gameMapList, 1, 0);
    m_gameProperties->setEnabled(false);

    m_gamePropsDock = new QDockWidget(this);
    m_gamePropsDock->setObjectName(QStringLiteral("GameDock"));
    m_gamePropsDock->setWindowTitle(tr("Game"));
    m_gamePropsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_gamePropsDock->setWidget(m_gameProperties);
    addDockWidget(Qt::LeftDockWidgetArea, m_gamePropsDock);

    m_mapProperties = new QWidget(this);
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
    m_viewport->addItem(tr("10 x 10"), (int)cc2::MapOption::View10x10);
    m_viewport->addItem(tr("9 x 9"), (int)cc2::MapOption::View9x9);
    m_viewport->addItem(tr("Split"), (int)cc2::MapOption::ViewSplit);
    auto viewLabel = new QLabel(tr("Vie&w:"), m_mapProperties);
    viewLabel->setBuddy(m_viewport);
    m_blobPattern = new QComboBox(m_mapProperties);
    m_blobPattern->addItem(tr("Deterministic"), (int)cc2::MapOption::BlobsDeterministic);
    m_blobPattern->addItem(tr("4 Patterns"), (int)cc2::MapOption::Blobs4Pattern);
    m_blobPattern->addItem(tr("Extra Random"), (int)cc2::MapOption::BlobsExtraRandom);
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
    m_mapProperties->setEnabled(false);

    m_mapPropsDock = new QDockWidget(this);
    m_mapPropsDock->setObjectName(QStringLiteral("MapPropsDock"));
    m_mapPropsDock->setWindowTitle(tr("Map Properties"));
    m_mapPropsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_mapPropsDock->setWidget(m_mapProperties);
    tabifyDockWidget(m_gamePropsDock, m_mapPropsDock);

    connect(m_title, &QLineEdit::textChanged, this, &CC2EditMain::onTitleChanged);
    connect(m_author, &QLineEdit::textChanged, this, &CC2EditMain::onAuthorChanged);
    connect(m_lockText, &QLineEdit::textChanged, this, &CC2EditMain::onLockChanged);
    connect(m_editorVersion, &QLineEdit::textChanged, this, &CC2EditMain::onEditorVersionChanged);
    connect(m_timeLimit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CC2EditMain::onTimeLimitChanged);
    connect(m_viewport, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CC2EditMain::onViewportChanged);
    connect(m_blobPattern, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CC2EditMain::onBlobPatternChanged);
    connect(m_hideLogic, &QCheckBox::toggled, this, &CC2EditMain::onHideLogicChanged);
    connect(m_cc1Boots, &QCheckBox::toggled, this, &CC2EditMain::onCC1BootsChanged);
    connect(m_readOnly, &QCheckBox::toggled, this, &CC2EditMain::onReadOnlyChanged);
    connect(m_clue, &QPlainTextEdit::textChanged, this, &CC2EditMain::onClueChanged);
    connect(m_note, &QPlainTextEdit::textChanged, this, &CC2EditMain::onNoteChanged);
    connect(resizeButton, &QPushButton::clicked, this, &CC2EditMain::onResizeMap);

    auto sortedTiles = new QWidget(this);
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
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireNorth | cc2::TileModifier::WireTunnelNorth),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireEast | cc2::TileModifier::WireTunnelEast),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireSouth | cc2::TileModifier::WireTunnelSouth),
        cc2::Tile(cc2::Tile::Floor, cc2::TileModifier::WireWest | cc2::TileModifier::WireTunnelWest),
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
        cc2::Tile::dirBlockTile(0),
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
    tileLists[ListAdvanced] = new TileListWidget(tileBox);
    tileLists[ListAdvanced]->setTiles({
        cc2::Tile(cc2::Tile::Trap_Open),
        cc2::Tile(cc2::Tile::CC1_Cloner),
        cc2::Tile(cc2::Tile::CC1_Barrier_S),
        cc2::Tile(cc2::Tile::CC1_Barrier_E),
        cc2::Tile(cc2::Tile::CC1_Barrier_SE),
        DIR_TILE_LIST(cc2::Tile::UNUSED_Explosion),
        cc2::Tile(cc2::Tile::UNUSED_53),
        cc2::Tile(cc2::Tile::UNUSED_54),
        cc2::Tile(cc2::Tile::UNUSED_55),
        cc2::Tile(cc2::Tile::UNUSED_5d),
        cc2::Tile(cc2::Tile::UNUSED_67),
        cc2::Tile(cc2::Tile::UNUSED_6c),
        cc2::Tile(cc2::Tile::UNUSED_6e),
        cc2::Tile(cc2::Tile::UNUSED_74),
        cc2::Tile(cc2::Tile::UNUSED_75),
        cc2::Tile(cc2::Tile::UNUSED_79),
        cc2::Tile(cc2::Tile::UNUSED_85),
        cc2::Tile(cc2::Tile::UNUSED_86),
        cc2::Tile(cc2::Tile::UNUSED_91),
    });
    tileBox->addItem(tileLists[ListAdvanced], tr("Advanced"));

    for (auto listWidget : tileLists) {
        connect(listWidget, &TileListWidget::tileSelectedLeft, this, &CC2EditMain::setLeftTile);
        connect(listWidget, &TileListWidget::tileSelectedRight, this, &CC2EditMain::setRightTile);
        connect(this, &CC2EditMain::tilesetChanged, listWidget, &TileListWidget::setTileImages);
    }

    auto layerWidget = new LayerWidget(sortedTiles);
    auto leftLabel = new QLabel(tr("Left Button: "), sortedTiles);
    auto leftTileLabel = new QLabel(sortedTiles);
    auto rightLabel = new QLabel(tr("Right Button: "), sortedTiles);
    auto rightTileLabel = new QLabel(sortedTiles);
    leftTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    rightTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(this, &CC2EditMain::tilesetChanged, layerWidget, &LayerWidget::setTileset);
    connect(this, &CC2EditMain::leftTileChanged, layerWidget, &LayerWidget::setUpper);
    connect(this, &CC2EditMain::rightTileChanged, layerWidget, &LayerWidget::setLower);
    connect(this, &CC2EditMain::leftTileChanged, this, [leftTileLabel](const cc2::Tile& tile) {
        leftTileLabel->setText(CC2ETileset::getName(&tile));
    });
    connect(this, &CC2EditMain::rightTileChanged, this, [rightTileLabel](const cc2::Tile& tile) {
        rightTileLabel->setText(CC2ETileset::getName(&tile));
    });

    auto tileLayout = new QGridLayout(sortedTiles);
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
    sortedTilesDock->setWidget(sortedTiles);
    tabifyDockWidget(m_gamePropsDock, sortedTilesDock);

    auto allTileWidget = new QWidget(this);
    auto allTileTbar = new QToolBar(allTileWidget);
    auto allTileScroll = new QScrollArea(allTileWidget);
    auto allTiles = new BigTileWidget(allTileScroll);
    allTileScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    allTileScroll->setWidget(allTiles);

    allTileTbar->setIconSize(QSize(32, 32));
    auto glyphAction = allTileTbar->addAction(ICON("tile-glyph-lg"), tr("Show Glyph Tiles"));
    glyphAction->setCheckable(true);
    allTileTbar->addSeparator();
    auto rolAction = allTileTbar->addAction(ICON("object-rotate-left-lg"), tr("Rotate Left"));
    rolAction->setShortcut(Qt::Key_Comma);
    auto rorAction = allTileTbar->addAction(ICON("object-rotate-right-lg"), tr("Rotate Right"));
    rorAction->setShortcut(Qt::Key_Period);

    connect(rolAction, &QAction::triggered, this, [this, allTiles] {
        allTiles->rotateLeft();
        m_leftTile.rotateLeft();
        setLeftTile(m_leftTile);
        m_rightTile.rotateLeft();
        setRightTile(m_rightTile);
    });
    connect(rorAction, &QAction::triggered, this, [this, allTiles] {
        allTiles->rotateRight();
        m_leftTile.rotateRight();
        setLeftTile(m_leftTile);
        m_rightTile.rotateRight();
        setRightTile(m_rightTile);
    });
    connect(glyphAction, &QAction::toggled, this, [allTiles](bool checked) {
        allTiles->setView(checked ? BigTileWidget::ViewGlyphs
                                  : BigTileWidget::ViewTiles);
    });

    layerWidget = new LayerWidget(allTileWidget);
    leftLabel = new QLabel(tr("Left Button: "), allTileWidget);
    rightLabel = new QLabel(tr("Right Button: "), allTileWidget);
    leftTileLabel = new QLabel(allTileWidget);
    rightTileLabel = new QLabel(allTileWidget);
    leftTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    rightTileLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(this, &CC2EditMain::tilesetChanged, allTiles, &BigTileWidget::setTileset);
    connect(allTiles, &BigTileWidget::tileSelectedLeft, this, &CC2EditMain::setLeftTile);
    connect(allTiles, &BigTileWidget::tileSelectedRight, this, &CC2EditMain::setRightTile);
    connect(this, &CC2EditMain::tilesetChanged, layerWidget, &LayerWidget::setTileset);
    connect(this, &CC2EditMain::leftTileChanged, layerWidget, &LayerWidget::setUpper);
    connect(this, &CC2EditMain::rightTileChanged, layerWidget, &LayerWidget::setLower);
    connect(this, &CC2EditMain::leftTileChanged, this, [leftTileLabel](const cc2::Tile& tile) {
        leftTileLabel->setText(CC2ETileset::getName(&tile));
    });
    connect(this, &CC2EditMain::rightTileChanged, this, [rightTileLabel](const cc2::Tile& tile) {
        rightTileLabel->setText(CC2ETileset::getName(&tile));
    });

    auto allTileLayout = new QGridLayout(allTileWidget);
    allTileLayout->setContentsMargins(4, 4, 4, 4);
    allTileLayout->setVerticalSpacing(4);
    allTileLayout->addWidget(allTileTbar, 0, 0, 1, 3);
    allTileLayout->addWidget(allTileScroll, 1, 0, 1, 3);
    allTileLayout->addWidget(leftLabel, 2, 0);
    allTileLayout->addWidget(leftTileLabel, 2, 1);
    allTileLayout->addWidget(rightLabel, 3, 0);
    allTileLayout->addWidget(rightTileLabel, 3, 1);
    allTileLayout->addWidget(layerWidget, 2, 2, 2, 1);

    auto allTilesDock = new QDockWidget(this);
    allTilesDock->setObjectName(QStringLiteral("AllTilesDock"));
    allTilesDock->setWindowTitle(tr("All Tiles"));
    allTilesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    allTilesDock->setWidget(allTileWidget);
    tabifyDockWidget(m_gamePropsDock, allTilesDock);

    // Editor area
    m_editorTabs = new EditorTabWidget(this);
    setCentralWidget(m_editorTabs);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNewMap]);
    fileMenu->addAction(m_actions[ActionNewScript]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionOpen]);
    fileMenu->addAction(m_actions[ActionImportCC1]);
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
    toolsMenu->addAction(m_actions[ActionDrawFlood]);
    toolsMenu->addAction(m_actions[ActionPathMaker]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionDrawWire]);
    toolsMenu->addAction(m_actions[ActionInspectHints]);
    toolsMenu->addAction(m_actions[ActionInspectTiles]);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_actions[ActionToggleGreens]);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_actions[ActionViewViewport]);
    viewMenu->addAction(m_actions[ActionViewMonsterPaths]);
    viewMenu->addSeparator();
    QMenu* dockMenu = viewMenu->addMenu(tr("&Toolbox"));
    dockMenu->addAction(m_gamePropsDock->toggleViewAction());
    dockMenu->addAction(m_mapPropsDock->toggleViewAction());
    dockMenu->addAction(sortedTilesDock->toggleViewAction());
    dockMenu->addAction(allTilesDock->toggleViewAction());
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
    helpMenu->addSeparator();
    helpMenu->addAction(m_actions[ActionScriptHelp]);

    // Tool bars
    QToolBar* tbarMain = addToolBar(QString());
    tbarMain->setObjectName(QStringLiteral("ToolbarMain"));
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
    tbarTools->setObjectName(QStringLiteral("ToolbarTools"));
    tbarTools->setWindowTitle(tr("Tools"));
    tbarTools->addAction(m_actions[ActionDrawPencil]);
    tbarTools->addAction(m_actions[ActionDrawLine]);
    tbarTools->addAction(m_actions[ActionDrawFill]);
    tbarTools->addAction(m_actions[ActionDrawFlood]);
    tbarTools->addAction(m_actions[ActionPathMaker]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionDrawWire]);
    tbarTools->addAction(m_actions[ActionInspectHints]);
    tbarTools->addAction(m_actions[ActionInspectTiles]);
    tbarTools->addSeparator();
    tbarTools->addAction(m_actions[ActionToggleGreens]);

    // Status bar
    m_positionLabel = new QLabel(this);
    statusBar()->addWidget(m_positionLabel, 1);

    connect(m_actions[ActionNewMap], &QAction::triggered, this, &CC2EditMain::createNewMap);
    connect(m_actions[ActionNewScript], &QAction::triggered, this, &CC2EditMain::createNewScript);
    connect(m_actions[ActionOpen], &QAction::triggered, this, &CC2EditMain::onOpenAction);
    connect(m_actions[ActionImportCC1], &QAction::triggered, this, &CC2EditMain::onImportCC1Action);
    connect(m_actions[ActionSave], &QAction::triggered, this, &CC2EditMain::onSaveAction);
    connect(m_actions[ActionSaveAs], &QAction::triggered, this, &CC2EditMain::onSaveAsAction);
    connect(m_actions[ActionClose], &QAction::triggered, this, &CC2EditMain::closeScript);
    connect(m_actions[ActionGenReport], &QAction::triggered, this, &CC2EditMain::onReportAction);
    connect(m_actions[ActionExit], &QAction::triggered, this, &CC2EditMain::close);

    connect(m_actions[ActionSelect], &QAction::toggled, this, &CC2EditMain::onSelectToggled);
    connect(m_actions[ActionCut], &QAction::triggered, this, &CC2EditMain::onCutAction);
    connect(m_actions[ActionCopy], &QAction::triggered, this, &CC2EditMain::onCopyAction);
    connect(m_actions[ActionPaste], &QAction::triggered, this, &CC2EditMain::onPasteAction);
    connect(m_actions[ActionClear], &QAction::triggered, this, &CC2EditMain::onClearAction);
    connect(m_actions[ActionUndo], &QAction::triggered, this, &CC2EditMain::onUndoAction);
    connect(m_actions[ActionRedo], &QAction::triggered, this, &CC2EditMain::onRedoAction);

    connect(m_actions[ActionDrawPencil], &QAction::toggled, this, &CC2EditMain::onDrawPencilAction);
    connect(m_actions[ActionDrawLine], &QAction::toggled, this, &CC2EditMain::onDrawLineAction);
    connect(m_actions[ActionDrawFill], &QAction::toggled, this, &CC2EditMain::onDrawFillAction);
    connect(m_actions[ActionDrawFlood], &QAction::toggled, this, &CC2EditMain::onDrawFloodAction);
    connect(m_actions[ActionPathMaker], &QAction::toggled, this, &CC2EditMain::onPathMakerAction);
    connect(m_actions[ActionDrawWire], &QAction::toggled, this, &CC2EditMain::onDrawWireAction);
    connect(m_actions[ActionInspectHints], &QAction::toggled, this, &CC2EditMain::onInspectHints);
    connect(m_actions[ActionInspectTiles], &QAction::toggled, this, &CC2EditMain::onInspectTiles);
    connect(m_actions[ActionToggleGreens], &QAction::triggered, this, &CC2EditMain::onToggleGreensAction);

    connect(m_actions[ActionViewViewport], &QAction::toggled, this, &CC2EditMain::onViewViewportToggled);
    connect(m_actions[ActionViewMonsterPaths], &QAction::toggled, this, &CC2EditMain::onViewMonsterPathsToggled);

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
        AboutDialog about(QStringLiteral("CC2Edit"),
                          QPixmap(QStringLiteral(":/icons/boot-32.png")), this);
        about.exec();
    });

    connect(m_actions[ActionScriptHelp], &QAction::triggered, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://wiki.bitbusters.club/C2G")));
    });

    connect(m_actions[ActionReloadScript], &QAction::triggered, this, [this] {
        if (!m_currentGameScript.isEmpty())
            loadScript(m_currentGameScript);
    });
    connect(m_actions[ActionEditScript], &QAction::triggered, this, [this] {
        if (!m_currentGameScript.isEmpty())
            editScript(m_currentGameScript);
    });

    connect(m_editorTabs, &QTabWidget::tabCloseRequested, this, &CC2EditMain::onTabClosed);
    connect(m_editorTabs, &QTabWidget::currentChanged, this, &CC2EditMain::onTabChanged);

    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &CC2EditMain::onClipboardDataChanged);

    // Load window settings and defaults
    QSettings settings;
    resize(settings.value(QStringLiteral("WindowSize"), QSize(1024, 768)).toSize());
    if (settings.value(QStringLiteral("WindowMaximized"), false).toBool())
        showMaximized();
    if (settings.contains(QStringLiteral("WindowState")))
        restoreState(settings.value(QStringLiteral("WindowState")).toByteArray());
    m_zoomFactor = settings.value(QStringLiteral("ZoomFactor"), 1.0).toDouble();
    m_actions[ActionViewViewport]->setChecked(
            settings.value(QStringLiteral("ViewViewport"), true).toBool());
    m_actions[ActionViewMonsterPaths]->setChecked(
            settings.value(QStringLiteral("ViewMonsterPaths"), false).toBool());

    // Make sure the toolbox docks are visible
    QDockWidget* docks[] = {m_gamePropsDock, m_mapPropsDock, sortedTilesDock, allTilesDock};
    for (QDockWidget* dock : docks) {
        if (dock->isFloating()) {
            QPoint dockPos = dock->pos();
            QDesktopWidget* desktop = QApplication::desktop();
            if ((dockPos.x() + dock->width() - 10) < desktop->contentsRect().left())
                dockPos.setX(desktop->contentsRect().left());
            if (dockPos.x() + 10 > desktop->contentsRect().right())
                dockPos.setX(desktop->contentsRect().right() - dock->width());
            if (dockPos.y() < desktop->contentsRect().top())
                dockPos.setY(desktop->contentsRect().top());
            if (dockPos.y() + 10 > desktop->contentsRect().bottom())
                dockPos.setY(desktop->contentsRect().bottom() - dock->height());
            dock->move(dockPos);
            dock->show();
        }
    }
    m_gamePropsDock->raise();

    findTilesets();
    if (m_tilesetGroup->actions().size() == 0) {
        QMessageBox::critical(this, tr("Error loading tilesets"),
              tr("Error: No tilesets found.  Please check your CCTools installation"),
              QMessageBox::Ok);
        exit(1);
    } else {
        QString tilesetFilename = settings.value(QStringLiteral("TilesetName"),
                                                 QStringLiteral("CC2.tis")).toString();
        bool foundTset = false;
        for (int i = 0; i < m_tilesetGroup->actions().size(); ++i) {
            auto tileset = m_tilesetGroup->actions()[i]->data().value<CC2ETileset*>();
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
    setLeftTile(defTile);
    defTile.setType(cc2::Tile::Floor);
    setRightTile(defTile);

    setGameName(QString());
    onClipboardDataChanged();
}

void CC2EditMain::createNewMap()
{
    auto map = new cc2::Map;
    map->mapData().resize(32, 32);
    addEditor(map, QString(), false);
    map->unref();

    m_mapPropsDock->raise();
}

void CC2EditMain::createNewScript()
{
    auto editor = addScriptEditor(QString());
    editor->setPlainText(QStringLiteral(
                "game \"My CC2 Game\"\n"
                "0 flags =\n"
                "0 score =\n"
                "0 hispeed =\n"
                "1 level =\n"));

    // Select the game name for easy editing.
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, 6);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    editor->setTextCursor(cursor);
    editor->setFocus(Qt::OtherFocusReason);
}

void CC2EditMain::loadFile(const QString& filename)
{
    QFileInfo info(filename);
    if (info.suffix().compare(QLatin1String("c2g"), Qt::CaseInsensitive) == 0) {
        if (loadScript(filename))
            m_gamePropsDock->raise();
    } else if (info.suffix().compare(QLatin1String("c2m"), Qt::CaseInsensitive) == 0) {
        if (loadMap(filename, false))
            m_mapPropsDock->raise();
    } else {
        QMessageBox::critical(this, tr("Invalid filename"),
                              tr("Unsupported file type for %1").arg(filename));
    }
}

bool CC2EditMain::loadMap(const QString& filename, bool floatTab)
{
    QFileInfo info(filename);
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2EditorWidget* editor = getEditorAt(i);
        if (editor && editor->filename() == info.canonicalFilePath()) {
            m_editorTabs->setCurrentIndex(i);
            return true;
        }
    }

    ccl::FileStream fs;
    if (!fs.open(filename, ccl::FileStream::Read)) {
        QMessageBox::critical(this, tr("Error loading map"),
                tr("Could not open %1 for reading.").arg(filename));
        return false;
    }

    auto map = new cc2::Map;
    bool success = true;
    try {
        map->read(&fs);
        addEditor(map, filename, floatTab);
    } catch (const ccl::RuntimeError& ex) {
        QMessageBox::critical(this, tr("Error loading map"), ex.message());
        success = false;
    }
    map->unref();
    return success;
}

bool CC2EditMain::loadScript(const QString& filename)
{
    m_gameMapList->clear();
    setGameName(tr("(Unknown)"));

    ScriptMapLoader mapLoader;
    connect(&mapLoader, &ScriptMapLoader::gameName, this, &CC2EditMain::setGameName);
    connect(&mapLoader, &ScriptMapLoader::mapAdded, this,
            [this](int levelNum, const QString &filename) {
        cc2::Map map;
        ccl::FileStream fs;
        if (fs.open(filename, ccl::FileStream::Read)) {
            try {
                map.read(&fs);
            } catch (const ccl::RuntimeError& err) {
                QMessageBox::critical(this, tr("Error processing map"),
                                      tr("Failed to load map data for %1: %2")
                                      .arg(filename).arg(err.message()));
            }
        }
        QString title = !map.title().empty()
                            ? ccl::fromLatin1(map.title())
                            : QFileInfo(filename).fileName();

        QString name = tr("%1 - %2").arg(levelNum).arg(title);
        auto item = new QListWidgetItem(name, m_gameMapList);
        item->setData(Qt::UserRole, filename);
    });
    if (mapLoader.loadScript(filename)) {
        m_currentGameScript = filename;
        m_gameProperties->setEnabled(true);
        m_actions[ActionClose]->setEnabled(true);
        m_actions[ActionGenReport]->setEnabled(true);
        return true;
    } else {
        closeScript();
        return false;
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
    editor->setFocus(Qt::OtherFocusReason);
}

void CC2EditMain::closeScript()
{
    m_gameMapList->clear();
    m_currentGameScript = QString();
    setGameName(QString());
    m_gameProperties->setEnabled(false);
    m_actions[ActionClose]->setEnabled(false);
    m_actions[ActionGenReport]->setEnabled(false);
}

bool CC2EditMain::saveTab(int index)
{
    CC2EditorWidget* mapEditor = getEditorAt(index);
    CC2ScriptEditor* scriptEditor = getScriptEditorAt(index);

    QString filename = mapEditor ? mapEditor->filename()
                     : scriptEditor ? scriptEditor->filename()
                     : QString();
    if (filename.isEmpty())
        return saveTabAs(index);

    bool result = false;
    if (mapEditor) {
        result = saveMap(mapEditor->map(), filename);
        if (result)
            mapEditor->setClean();
    }
    if (scriptEditor) {
        result = saveScript(scriptEditor->toPlainText(), filename);
        if (result)
            scriptEditor->setClean();
    }

    if (result)
        m_editorTabs->promoteTab(index);
    return result;
}

bool CC2EditMain::saveTabAs(int index)
{
    CC2EditorWidget* mapEditor = getEditorAt(index);
    CC2ScriptEditor* scriptEditor = getScriptEditorAt(index);
    if (!mapEditor && !scriptEditor)
        return false;

    QSettings settings;
    QString filename = mapEditor ? mapEditor->filename()
                     : scriptEditor ? scriptEditor->filename()
                     : QString();
    if (filename.isEmpty())
        filename = settings.value(QStringLiteral("DialogDir")).toString();

    bool result = false;
    if (mapEditor) {
        filename = QFileDialog::getSaveFileName(this, tr("Save Map..."),
                                                filename, tr("CC2 Maps (*.c2m)"));
        if (!filename.isEmpty())
            result = saveMap(mapEditor->map(), filename);
        if (result) {
            mapEditor->setFilename(filename);
            mapEditor->setClean();
        }
    }
    if (scriptEditor) {
        filename = QFileDialog::getSaveFileName(this, tr("Save Game Script..."),
                                                filename, tr("CC2 Game Scripts (*.c2g)"));
        if (!filename.isEmpty())
            result = saveScript(scriptEditor->toPlainText(), filename);
        if (result) {
            scriptEditor->setFilename(filename);
            scriptEditor->setClean();
        }
    }

    if (result) {
        QFileInfo info(filename);
        m_editorTabs->setTabText(index, info.fileName());
        m_editorTabs->promoteTab(index);
        settings.setValue(QStringLiteral("DialogDir"), info.dir().absolutePath());
    }
    return result;
}

bool CC2EditMain::saveMap(cc2::Map* map, const QString& filename)
{
    ccl::FileStream fs;
    if (!fs.open(filename, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Could not open %1 for writing.").arg(filename));
        return false;
    }
    try {
        map->write(&fs);
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Failed to write map to %1: %2")
                        .arg(filename).arg(err.message()));
        return false;
    }
    fs.close();
    return true;
}

bool CC2EditMain::saveScript(const QString& script, const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Could not open %1 for writing.").arg(filename));
        return false;
    }
    const QByteArray scriptBytes = script.toLatin1();
    if (file.write(scriptBytes) != scriptBytes.size()) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Failed to write script to %1.").arg(filename));
        return false;
    }
    file.close();

    // Reload the open game script if we're saving over it.
    if (QFileInfo(m_currentGameScript) == QFileInfo(filename))
        loadScript(filename);

    return true;
}

void CC2EditMain::registerTileset(const QString& filename)
{
    auto tileset = new CC2ETileset(this);
    bool valid = false;
    try {
        valid = tileset->load(filename);
    } catch (const ccl::IOError& err) {
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

void CC2EditMain::loadEditorForItem(QListWidgetItem* item)
{
    if (!item)
        return;

    QString filename = item->data(Qt::UserRole).toString();
    if (!filename.isEmpty())
        loadMap(filename, true);
}

void CC2EditMain::findTilesets()
{
    m_tilesetMenu->clear();

    QDir path;
    QStringList tilesets;
    const QStringList tilesetGlob { QStringLiteral("*.tis") };
#if defined(Q_OS_WIN)
    // Search app directory
    path.setPath(QCoreApplication::applicationDirPath());
    for (const QString& file : path.entryList(tilesetGlob, QDir::Files | QDir::Readable, QDir::Name))
        tilesets << path.absoluteFilePath(file);
#else
    // Search install path
    path.setPath(QCoreApplication::applicationDirPath());
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

CC2EditorWidget* CC2EditMain::currentEditor()
{
    return getEditorAt(m_editorTabs->currentIndex());
}

CC2ScriptEditor* CC2EditMain::getScriptEditorAt(int idx)
{
    if (idx < 0 || idx >= m_editorTabs->count())
        return nullptr;
    return qobject_cast<CC2ScriptEditor *>(m_editorTabs->widget(idx));
}

CC2ScriptEditor* CC2EditMain::currentScriptEditor()
{
    return getScriptEditorAt(m_editorTabs->currentIndex());
}

CC2EditorWidget* CC2EditMain::addEditor(cc2::Map* map, const QString& filename, bool floatTab)
{
    auto scroll = new QScrollArea(m_editorTabs);
    auto editor = new CC2EditorWidget(scroll);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidget(editor);
    if (m_actions[ActionViewViewport]->isChecked())
        editor->setPaintFlag(CC2EditorWidget::ShowViewBox);
    if (m_actions[ActionViewMonsterPaths]->isChecked())
        editor->setPaintFlag(CC2EditorWidget::ShowMovePaths);
    //if (m_actions[ActionViewErrors]->isChecked())
    //    editor->setPaintFlag(CC2EditorWidget::ShowErrors);
    editor->setTileset(m_currentTileset);
    editor->setMap(map);
    editor->setLeftTile(m_leftTile);
    editor->setRightTile(m_rightTile);
    if (m_zoomFactor != 0.0)
        editor->setZoom(m_zoomFactor);

    if (filename.isEmpty()) {
        m_editorTabs->addTab(scroll, tr("Untitled Map"));
    } else {
        QFileInfo info(filename);
        editor->setFilename(info.canonicalFilePath());
        if (floatTab)
            m_editorTabs->addFloatingTab(scroll, info.fileName());
        else
            m_editorTabs->addTab(scroll, info.fileName());
    }
    resizeEvent(nullptr);

    connect(editor, &CC2EditorWidget::mouseInfo, statusBar(), &QStatusBar::showMessage);
    connect(editor, &CC2EditorWidget::canUndoChanged, m_actions[ActionUndo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::canRedoChanged, m_actions[ActionRedo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCut], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCopy], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionClear], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::tilePicked, this, &CC2EditMain::onTilePicked);

    connect(editor, &CC2EditorWidget::cleanChanged, this, [this, scroll](bool clean) {
        const int index = m_editorTabs->indexOf(scroll);
        if (index < 0)
            return;

        const QString tabText = m_editorTabs->tabText(index);
        if (clean && tabText.endsWith(QStringLiteral(" *")))
            m_editorTabs->setTabText(index, tabText.left(tabText.size() - 2));
        else if (!clean && !tabText.endsWith(QStringLiteral(" *")))
            m_editorTabs->setTabText(index, tabText + QStringLiteral(" *"));

        m_editorTabs->promoteTab();
    });

    connect(editor, &CC2EditorWidget::updateCounters, this, [this, editor] {
        updateCounters(editor->map()->mapData());
    });

    connect(this, &CC2EditMain::tilesetChanged, editor, &CC2EditorWidget::setTileset);
    connect(this, &CC2EditMain::leftTileChanged, editor, &CC2EditorWidget::setLeftTile);
    connect(this, &CC2EditMain::rightTileChanged, editor, &CC2EditorWidget::setRightTile);

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

    connect(editor, &QPlainTextEdit::copyAvailable, m_actions[ActionCut], &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::copyAvailable, m_actions[ActionCopy], &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::copyAvailable, m_actions[ActionClear], &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::undoAvailable, m_actions[ActionUndo], &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::redoAvailable, m_actions[ActionRedo], &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::modificationChanged, this, [this, editor](bool dirty) {
        const int index = m_editorTabs->indexOf(editor);
        if (index < 0)
            return;

        const QString tabText = m_editorTabs->tabText(index);
        if (dirty && !tabText.endsWith(QStringLiteral(" *")))
            m_editorTabs->setTabText(index, tabText + QStringLiteral(" *"));
        else if (!dirty && tabText.endsWith(QStringLiteral(" *")))
            m_editorTabs->setTabText(index, tabText.left(tabText.size() - 2));
    });
    connect(editor, &SyntaxTextEdit::cursorPositionChanged, this, [this, editor] {
        const QTextCursor cursor = editor->textCursor();
        const int column = editor->textColumn(cursor.block().text(), cursor.positionInBlock());
        QString positionText = tr("Line %1, Col %2")
                                    .arg(cursor.blockNumber() + 1)
                                    .arg(column + 1);
        m_positionLabel->setText(positionText);
    });

    m_editorTabs->setCurrentWidget(editor);
    return editor;
}

void CC2EditMain::closeAllTabs()
{
    while (m_editorTabs->count() != 0) {
        m_editorTabs->widget(0)->deleteLater();
        m_editorTabs->removeTab(0);
    }
}

void CC2EditMain::closeEvent(QCloseEvent* event)
{
    QList<int> modifiedTabs;
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2EditorWidget* editor = getEditorAt(i);
        CC2ScriptEditor* scriptEditor = getScriptEditorAt(i);
        if ((editor && !editor->isClean()) || (scriptEditor && scriptEditor->isModified()))
            modifiedTabs.push_back(i);
    }
    if (!modifiedTabs.empty()) {
        int response = QMessageBox::question(this, tr("Close all"),
                              tr("%n open file(s) have unsaved changes."
                                 "  Would you like to save before closing?", "",
                                 modifiedTabs.size()),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (response == QMessageBox::Cancel) {
            event->setAccepted(false);
            return;
        }
        if (response == QMessageBox::Yes) {
            for (int tabIndex : modifiedTabs) {
                if (!saveTab(tabIndex)) {
                    event->setAccepted(false);
                    return;
                }
            }
        }
    }
    closeAllTabs();

    if (m_subProc) {
        // Don't handle events after we're exiting.
        // Note that temp file cleanup will not take place if this happens!
        m_subProc->disconnect();
    }

    QSettings settings;
    settings.setValue(QStringLiteral("WindowMaximized"),
                      (windowState() & Qt::WindowMaximized) != 0);
    showNormal();
    settings.setValue(QStringLiteral("WindowSize"), size());
    settings.setValue(QStringLiteral("WindowState"), saveState());
    settings.setValue(QStringLiteral("ZoomFactor"), m_zoomFactor);
    settings.setValue(QStringLiteral("ViewViewport"),
                      m_actions[ActionViewViewport]->isChecked());
    settings.setValue(QStringLiteral("ViewMonsterPaths"),
                      m_actions[ActionViewMonsterPaths]->isChecked());
    settings.setValue(QStringLiteral("TilesetName"), m_currentTileset->filename());
}

void CC2EditMain::resizeEvent(QResizeEvent* event)
{
    if (event)
        QWidget::resizeEvent(event);

    if (m_zoomFactor == 0.0) {
        auto scroll = qobject_cast<QScrollArea*>(m_editorTabs->currentWidget());
        auto editor = currentEditor();
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
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Map..."),
                            settings.value(QStringLiteral("DialogDir")).toString(),
                            tr("All supported files (*.c2m *.c2g);;"
                               "CC2 Maps (*.c2m);;"
                               "CC2 Game Scripts (*.c2g)"));
    if (!filename.isEmpty()) {
        loadFile(filename);
        settings.setValue(QStringLiteral("DialogDir"),
                          QFileInfo(filename).dir().absolutePath());
    }
}

void CC2EditMain::onImportCC1Action()
{
    QSettings settings;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Map..."),
                            settings.value(QStringLiteral("Import/DialogDir")).toString(),
                            tr("CC1 Levelsets (*.ccl *.dat)"));
    if (!filename.isEmpty()) {
        ImportDialog dialog(this);
        dialog.loadLevelset(filename);
        if (dialog.exec() != QDialog::Accepted)
            return;

        int levelNum = 0;
        cc2::Map* map = dialog.importMap(&levelNum);
        if (!map)
            return;

        QFileInfo info(filename);
        QString tempName = QStringLiteral("%1-%2.c2m").arg(info.baseName())
                                .arg(levelNum + 1, 3, 10, QLatin1Char('0'));
        CC2EditorWidget* editor = addEditor(map, tempName, false);
        editor->resetClean();
        m_mapPropsDock->raise();

        settings.setValue(QStringLiteral("Import/DialogDir"),
                          QFileInfo(filename).dir().absolutePath());
    }
}

void CC2EditMain::onSaveAction()
{
    saveTab(m_editorTabs->currentIndex());
}

void CC2EditMain::onSaveAsAction()
{
    saveTabAs(m_editorTabs->currentIndex());
}

void CC2EditMain::onReportAction()
{
    if (m_currentGameScript.isEmpty())
        return;

    QSettings settings;
    QString reportPath = m_currentGameScript.left(m_currentGameScript.lastIndexOf(QLatin1Char('.')))
                            + QStringLiteral(".html");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Report..."),
                                                    reportPath, tr("HTML Source (*.html)"));
    if (filename.isEmpty())
        return;

    QProgressDialog proDlg(this);
    proDlg.setMaximum(m_gameMapList->count() + 1);
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
    const QString filebase = QFileInfo(m_currentGameScript).fileName();
    report.write("<html>\n<head><title>Levelset Report for ");
    report.write(filebase.toUtf8().constData());
    report.write("</title></head>\n\n");
    report.write("<body>\n<h1 align=\"center\">Levelset Report for ");
    report.write(filebase.toUtf8().constData());
    report.write("</h1>\n");

    const QString dirbase = QFileInfo(filename).baseName() + QStringLiteral("_mapimg");
    const QString imgdir = QFileInfo(filename).path() + QDir::separator() + dirbase;
    QDir dir;
    dir.mkdir(imgdir);
    if (!dir.cd(imgdir)) {
        QMessageBox::critical(this, tr("Error creating report"),
                              tr("Error creating report: Could not create level image folder"));
        return;
    }

    for (int i = 0; i < m_gameMapList->count(); ++i) {
        const QString mapFile = m_gameMapList->item(i)->data(Qt::UserRole).toString();

        ccl::FileStream fs;
        if (!fs.open(mapFile, ccl::FileStream::Read)) {
            QMessageBox::critical(this, tr("Error loading map"),
                                  tr("Could not open %1 for reading.").arg(mapFile));
            return;
        }

        auto map = new cc2::Map;
        try {
            map->read(&fs);
        } catch (const ccl::RuntimeError& ex) {
            QMessageBox::critical(this, tr("Error loading map"), ex.message());
            map->unref();
            return;
        }

        report.write("<hr />\n<h2>Level ");
        report.write(QString::number(i + 1).toUtf8().constData());
        report.write("</h2>\n<pre>\n");
        if (!map->title().empty()) {
            report.write("<b>Title:</b>    ");
            report.write(map->title().c_str());
            report.write("\n");
        }
        if (!map->author().empty()) {
            report.write("<b>Author:</b>   ");
            report.write(map->author().c_str());
            report.write("\n");
        }
        if (!map->lock().empty()) {
            report.write("<b>Lock:</b>     ");
            report.write(map->lock().c_str());
            report.write("\n");
        }
        if (!map->editorVersion().empty()) {
            report.write("<b>Version:</b>  ");
            report.write(map->editorVersion().c_str());
            report.write("\n");
        }
        report.write("\n<b>Map Size:</b> ");
        report.write(tr("%1 x %2").arg(map->mapData().width())
                                  .arg(map->mapData().height()).toUtf8().constData());
        report.write("\n<b>Chips:</b>    ");
        const auto chips = map->mapData().countChips();
        if (std::get<0>(chips) != std::get<1>(chips)) {
            report.write(tr("%1 (of %2)").arg(std::get<0>(chips))
                                         .arg(std::get<1>(chips)).toUtf8().constData());
        } else {
            report.write(QString::number(std::get<0>(chips)).toUtf8().constData());
        }
        report.write("\n<b>Points:</b>   ");
        const auto points = map->mapData().countPoints();
        if (std::get<1>(points) != 1) {
            report.write(tr("%1 (x%2)").arg(std::get<0>(points))
                                       .arg(std::get<1>(points)).toUtf8().constData());
        } else {
            report.write(QString::number(std::get<0>(points)).toUtf8().constData());
        }
        report.write("\n<b>Time:</b>     ");
        report.write(QString::number(map->option().timeLimit()).toUtf8().constData());
        report.write("\n<b>View:</b>     ");
        switch (map->option().view()) {
        case cc2::MapOption::View10x10:
            report.write("10x10");
            break;
        case cc2::MapOption::View9x9:
            report.write("9x9");
            break;
        case cc2::MapOption::ViewSplit:
            report.write("Split");
            break;
        default:
            report.write(tr("Unknown (%d)").arg(static_cast<int>(map->option().view()))
                         .toUtf8().constData());
            break;
        }
        report.write("\n<b>Blobs:</b>    ");
        switch (map->option().blobPattern()) {
        case cc2::MapOption::BlobsDeterministic:
            report.write("Deterministic");
            break;
        case cc2::MapOption::Blobs4Pattern:
            report.write("4 Patterns");
            break;
        case cc2::MapOption::BlobsExtraRandom:
            report.write("Extra Random");
            break;
        default:
            report.write(tr("Unknown (%d)").arg(static_cast<int>(map->option().blobPattern()))
                         .toUtf8().constData());
            break;
        }
        report.write("\n<b>Options:</b>\n");
        if (map->option().hideLogic())
            report.write("\tHide Logic\n");
        if (map->option().cc1Boots())
            report.write("\tCC1 Boots\n");
        if (map->option().readOnly())
            report.write("\tRead Only\n");
        if (!map->clue().empty()) {
            report.write("\n<b>Clue:</b>\n");
            report.write(map->clue().c_str());
            report.write("\n");
        }
        if (!map->note().empty()) {
            report.write("\n<b>Notes:</b>\n");
            report.write(map->note().c_str());
            report.write("\n");
        }
        report.write("\n</pre>\n<img src=\"");
        report.write(QStringLiteral("%1/map%2.png").arg(dirbase).arg(i + 1).toUtf8().constData());
        report.write("\" />\n");
        report.write("<p style=\"page-break-after: always;\">&nbsp;</p>\n");

        // Write the level image
        CC2EditorWidget reportDummy;
        reportDummy.setVisible(false);
        reportDummy.setTileset(m_currentTileset);
        reportDummy.setMap(map);
        QImage levelImage = reportDummy.renderReport();
        levelImage.save(QStringLiteral("%1/map%2.png").arg(imgdir).arg(i + 1), "PNG");

        map->unref();

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

void CC2EditMain::onSelectToggled(bool mode)
{
    if (!mode && m_currentDrawMode == CC2EditorWidget::DrawSelect) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = CC2EditorWidget::DrawSelect;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionSelect]);

        CC2EditorWidget* editor = currentEditor();
        if (editor)
            editor->setDrawMode(m_currentDrawMode);
    }
}

static const QString s_chipeditFormat = QStringLiteral("CHIPEDIT MAPSECT");
static const QString s_cc2eFormat = QStringLiteral("CC2Edit MapData");

//TODO: Need to figure out the mapping from CCCreator's tile format to CC2's
//static const QString s_cccreFormat = QStringLiteral("CCS Map");

static bool haveMapClipboardData()
{
    const QMimeData* cbData = QApplication::clipboard()->mimeData();
    return cbData->hasFormat(s_chipeditFormat) || cbData->hasFormat(s_cc2eFormat)
        /*|| cbData->hasFormat(s_cccreFormat)*/;
}

void CC2EditMain::onClipboardDataChanged()
{
    CC2EditorWidget* mapEditor = currentEditor();
    CC2ScriptEditor* scriptEditor = currentScriptEditor();

    if (mapEditor)
        m_actions[ActionPaste]->setEnabled(haveMapClipboardData());
    else if (scriptEditor)
        m_actions[ActionPaste]->setEnabled(scriptEditor->canPaste());
}

void CC2EditMain::onCutAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        onCopyAction();
        onClearAction();
    } else if (scriptEditor) {
        scriptEditor->cut();
    }
}

void CC2EditMain::onCopyAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        if (mapEditor->selection() == QRect(-1, -1, -1, -1))
            return;

        cc2::ClipboardMap cbMap;
        const QRect selection = mapEditor->selection();
        cbMap.mapData().resize(selection.width(), selection.height());
        cbMap.mapData().copyFrom(mapEditor->map()->mapData(),
                                 selection.x(), selection.y(), 0, 0,
                                 selection.width(), selection.height());

        try {
            ccl::BufferStream cbStream;
            cbMap.write(&cbStream);
            QByteArray buffer((const char*)cbStream.buffer(), cbStream.size());

            auto copyData = new QMimeData;
            copyData->setData(s_cc2eFormat, buffer);
            //TODO: set CC1 map data
            copyData->setImageData(mapEditor->renderSelection());
            QApplication::clipboard()->setMimeData(copyData);
        } catch (const ccl::RuntimeError& err) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Error saving clipboard data: %1").arg(err.message()),
                    QMessageBox::Ok);
        }
    } else if (scriptEditor) {
        scriptEditor->copy();
    }
}

void CC2EditMain::onPasteAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        const QMimeData* cbData = QApplication::clipboard()->mimeData();
        cc2::ClipboardMap cbMap;
        if (cbData->hasFormat(s_cc2eFormat)) {
            QByteArray buffer = cbData->data(s_cc2eFormat);
            ccl::BufferStream cbStream;
            cbStream.setFrom(buffer.constData(), buffer.size());

            try {
                cbMap.read(&cbStream);
            } catch (const ccl::RuntimeError& err) {
                QMessageBox::critical(this, tr("Error"),
                        tr("Error parsing clipboard data: %1").arg(err.message()),
                        QMessageBox::Ok);
                return;
            }
        } else if (cbData->hasFormat(s_chipeditFormat)) {
            QByteArray buffer = cbData->data(s_chipeditFormat);
            ccl::BufferStream cbStream;
            cbStream.setFrom(buffer.constData(), buffer.size());

            ccl::ClipboardData cc1Map;
            try {
                cc1Map.read(&cbStream);
            } catch (const ccl::RuntimeError& err) {
                QMessageBox::critical(this, tr("Error"),
                        tr("Error parsing clipboard data: %1").arg(err.message()),
                        QMessageBox::Ok);
                return;
            }

            cbMap.mapData().importFrom(cc1Map.levelData(), false);
            cbMap.mapData().resize(cc1Map.width(), cc1Map.height());
        } else {
            // No recognized clipboard formats
            return;
        }

        int destX, destY;
        if (mapEditor->selection() == QRect(-1, -1, -1, -1)) {
            destX = 0;
            destY = 0;
        } else {
            destX = mapEditor->selection().left();
            destY = mapEditor->selection().top();
        }

        int width = std::min((int)cbMap.mapData().width(),
                             mapEditor->map()->mapData().width() - destX);
        int height = std::min((int)cbMap.mapData().height(),
                              mapEditor->map()->mapData().height() - destY);

        mapEditor->beginEdit(CC2EditHistory::EditMap);
        mapEditor->selectRegion(destX, destY, width, height);
        mapEditor->map()->mapData().copyFrom(cbMap.mapData(), 0, 0, destX, destY, width, height);
        mapEditor->endEdit();
    } else if (scriptEditor) {
        scriptEditor->paste();
    }
}

void CC2EditMain::onClearAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        mapEditor->beginEdit(CC2EditHistory::EditMap);
        for (int y = mapEditor->selection().top(); y <= mapEditor->selection().bottom(); ++y) {
            for (int x = mapEditor->selection().left(); x <= mapEditor->selection().right(); ++x)
                mapEditor->putTile(cc2::Tile(), x, y, CC2EditorWidget::Replace);
        }
        mapEditor->endEdit();
    } else if (scriptEditor) {
        scriptEditor->deleteSelection();
    }
}

void CC2EditMain::onUndoAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        mapEditor->undo();
        updateMapProperties(mapEditor->map());
    } else if (scriptEditor) {
        scriptEditor->undo();
    }
}

void CC2EditMain::onRedoAction()
{
    auto mapEditor = currentEditor();
    auto scriptEditor = currentScriptEditor();

    if (mapEditor) {
        mapEditor->redo();
        updateMapProperties(mapEditor->map());
    } else if (scriptEditor) {
        scriptEditor->redo();
    }
}

void CC2EditMain::onDrawPencilAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawPencil;
    m_currentDrawMode = CC2EditorWidget::DrawPencil;
    uncheckAll(m_modalToolGroup);

    CC2EditorWidget* editor = currentEditor();
    if (editor)
        editor->setDrawMode(m_currentDrawMode);
}

void CC2EditMain::onDrawLineAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawLine;
    m_currentDrawMode = CC2EditorWidget::DrawLine;
    uncheckAll(m_modalToolGroup);

    CC2EditorWidget* editor = currentEditor();
    if (editor)
        editor->setDrawMode(m_currentDrawMode);
}

void CC2EditMain::onDrawFillAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawFill;
    m_currentDrawMode = CC2EditorWidget::DrawFill;
    uncheckAll(m_modalToolGroup);

    CC2EditorWidget* editor = currentEditor();
    if (editor)
        editor->setDrawMode(m_currentDrawMode);
}

void CC2EditMain::onDrawFloodAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionDrawFlood;
    m_currentDrawMode = CC2EditorWidget::DrawFlood;
    uncheckAll(m_modalToolGroup);

    CC2EditorWidget* editor = currentEditor();
    if (editor)
        editor->setDrawMode(m_currentDrawMode);
}

void CC2EditMain::onPathMakerAction(bool checked)
{
    if (!checked)
        return;

    m_savedDrawMode = ActionPathMaker;
    m_currentDrawMode = CC2EditorWidget::DrawPathMaker;
    uncheckAll(m_modalToolGroup);

    CC2EditorWidget* editor = currentEditor();
    if (editor)
        editor->setDrawMode(m_currentDrawMode);
}

void CC2EditMain::onDrawWireAction(bool mode)
{
    if (!mode && m_currentDrawMode == CC2EditorWidget::DrawWires) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = CC2EditorWidget::DrawWires;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionDrawWire]);

        CC2EditorWidget* editor = currentEditor();
        if (editor)
            editor->setDrawMode(m_currentDrawMode);
    }
}

void CC2EditMain::onInspectHints(bool mode)
{
    if (!mode && m_currentDrawMode == CC2EditorWidget::DrawInspectHint) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = CC2EditorWidget::DrawInspectHint;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionInspectHints]);

        CC2EditorWidget* editor = currentEditor();
        if (editor)
            editor->setDrawMode(m_currentDrawMode);
    }
}

void CC2EditMain::onInspectTiles(bool mode)
{
    if (!mode && m_currentDrawMode == CC2EditorWidget::DrawInspectTile) {
        m_actions[m_savedDrawMode]->setChecked(true);
    } else if (mode) {
        m_currentDrawMode = CC2EditorWidget::DrawInspectTile;
        uncheckAll(m_drawModeGroup);
        uncheckAll(m_modalToolGroup, m_actions[ActionInspectTiles]);

        CC2EditorWidget* editor = currentEditor();
        if (editor)
            editor->setDrawMode(m_currentDrawMode);
    }
}

void CC2EditMain::onToggleGreensAction()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    editor->beginEdit(CC2EditHistory::EditMap);
    cc2::ToggleGreens(editor->map());
    editor->endEdit();
}

void CC2EditMain::onViewViewportToggled(bool view)
{
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2EditorWidget* editor = getEditorAt(i);
        if (editor) {
            if (view)
                editor->setPaintFlag(CC2EditorWidget::ShowViewBox);
            else
                editor->clearPaintFlag(CC2EditorWidget::ShowViewBox);
        }
    }
}

void CC2EditMain::onViewMonsterPathsToggled(bool view)
{
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        CC2EditorWidget* editor = getEditorAt(i);
        if (editor) {
            if (view)
                editor->setPaintFlag(CC2EditorWidget::ShowMovePaths);
            else
                editor->clearPaintFlag(CC2EditorWidget::ShowMovePaths);
        }
    }
}

void CC2EditMain::onTilePicked(int x, int y)
{
    CC2EditorWidget* editor = currentEditor();
    Q_ASSERT(editor);

    if (m_currentDrawMode == CC2EditorWidget::DrawInspectTile) {
        TileInspector inspector(this);
        inspector.setTileset(m_currentTileset);
        cc2::Tile& tile = editor->map()->mapData().tile(x, y);
        inspector.loadTile(tile);
        if (inspector.exec() == QDialog::Accepted) {
            editor->beginEdit(CC2EditHistory::EditMap);
            tile = inspector.tile();
            editor->endEdit();
        }
    } else if (m_currentDrawMode == CC2EditorWidget::DrawInspectHint) {
        if (editor->map()->mapData().tile(x, y).bottom().type() != cc2::Tile::Clue)
            return;

        std::string clue = editor->map()->clueForTile(x, y);
        HintEditDialog dialog(x, y, this);
        dialog.setText(ccl::fromLatin1(clue));
        if (dialog.exec() == QDialog::Accepted) {
            editor->beginEdit(CC2EditHistory::EditMap);
            const std::string clueText = ccl::toLatin1(dialog.text());
            editor->map()->setClueForTile(x, y, clueText);
            editor->endEdit();
            updateMapProperties(editor->map());
        }
    }
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
    auto editor = currentEditor();
    if (!editor)
        return;

    if (m_subProc) {
        QMessageBox::critical(this, tr("Process already running"),
                tr("A CC2Edit test process is already running.  Please close the "
                   "running process before trying to start a new one"),
                QMessageBox::Ok);
        return;
    }

    QSettings settings;
    QString chips2Exe = settings.value(QStringLiteral("Chips2Exe")).toString();
    if (chips2Exe.isEmpty() || !QFile::exists(chips2Exe)) {
        QMessageBox::critical(this, tr("Could not find Chips2 executable"),
                tr("Could not find Chip's Challenge 2 executable.\n"
                   "Please configure Chips2 in the Test Setup dialog."));
        return;
    }
#ifndef Q_OS_WIN
    QString winePath = settings.value(QStringLiteral("WineExe")).toString();
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

    QDir chips2Dir(chips2Exe);
    chips2Dir.cdUp();
    m_testGameDir = chips2Dir.absolutePath();
    if (!chips2Dir.cd(QStringLiteral("data/games"))) {
        QMessageBox::critical(this, tr("Could not find game data"),
                tr("Could not find game data relative to Chips2 executable."));
        return;
    }

    static const QString playtestDirName = QStringLiteral("CC2Edit-playtest");
    if (chips2Dir.exists(playtestDirName) && chips2Dir.cd(playtestDirName)) {
        chips2Dir.removeRecursively();
        chips2Dir.cdUp();
    }
    if (!chips2Dir.mkdir(playtestDirName)) {
        QMessageBox::critical(this, tr("Error setting up playtest"),
                tr("Could not create playtest directory in %1.  Do you have write access?")
                .arg(chips2Dir.absoluteFilePath(playtestDirName)));
        return;
    }
    chips2Dir.cd(playtestDirName);
    QString testScript = QStringLiteral(
        "game \"CC2Edit-playtest\"\n"
        "0 flags =\n"
        "0 score =\n"
        "0 hispeed =\n"
        "1 level =\n"
        "map \"playtest.c2m\"\n");
    if (!saveScript(testScript, chips2Dir.absoluteFilePath(QStringLiteral("playtest.c2g"))))
        return;
    if (!saveMap(editor->map(), chips2Dir.absoluteFilePath(QStringLiteral("playtest.c2m"))))
        return;

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
    if (!savesDir.mkpath(QStringLiteral("data/saves")) || !savesDir.cd(QStringLiteral("data/saves"))) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Could not find or create save data directory relative to Chips2 executable."));
        return;
    }

    if (savesDir.exists(playtestDirName) && savesDir.cd(playtestDirName)) {
        savesDir.removeRecursively();
        savesDir.cdUp();
    }
    if (!savesDir.mkdir(playtestDirName)) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Could not create playtest saves directory at %1.")
                        .arg(savesDir.absoluteFilePath(playtestDirName)));
        return;
    }
    savesDir.cd(playtestDirName);

    ccl::FileStream fs;
    const QString saveFilename = savesDir.absoluteFilePath(QStringLiteral("save.c2s"));
    if (!fs.open(saveFilename, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Could not open %1 for writing.").arg(saveFilename));
        return;
    }
    try {
        save.write(&fs);
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Failed to write save data to %1: %2")
                        .arg(saveFilename).arg(err.message()));
        return;
    }
    fs.close();

    const QString scoreFilename = savesDir.absoluteFilePath(QStringLiteral("save.c2h"));
    if (!fs.open(scoreFilename, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error"),
                tr("Could not open %1 for writing.").arg(scoreFilename));
        return;
    }
    try {
        highScore.write(&fs);
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error"),
                        tr("Failed to write high score data to %1: %2")
                        .arg(scoreFilename).arg(err.message()));
        return;
    }
    fs.close();

    // Indicate the currently active game
    chips2Dir.cdUp();
    if (chips2Dir.exists(QStringLiteral("save.c2l"))
            && !chips2Dir.exists(QStringLiteral("save.c2l.CC2Edit-backup"))) {
        chips2Dir.rename(QStringLiteral("save.c2l"),
                         QStringLiteral("save.c2l.CC2Edit-backup"));
    }
    QFile listFile(chips2Dir.absoluteFilePath(QStringLiteral("save.c2l")));
    if (!listFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"),
                tr("Could not open %1 for writing.").arg(listFile.fileName()));
        return;
    }
    listFile.write("CC2Edit-playtest/save.c2s");
    listFile.close();

    QString cwd = QDir::currentPath();
    chips2Dir.setPath(m_testGameDir);
    if (chips2Dir.exists(QStringLiteral("steam_api.dll"))
            && !chips2Dir.exists(QStringLiteral("steam_appid.txt"))) {
        // This enables the game to work without being launched from Steam
        QFile appidFile(chips2Dir.absoluteFilePath(QStringLiteral("steam_appid.txt")));
        if (!appidFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Could not open steam_appid.txt for writing."));
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
    connect(m_subProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CC2EditMain::onProcessFinished);
    connect(m_subProc, &QProcess::errorOccurred, this, &CC2EditMain::onProcessError);
    m_subProc->start(chips2Exe, QStringList());
    QDir::setCurrent(cwd);
}

void CC2EditMain::onTabClosed(int index)
{
    CC2EditorWidget* mapEditor = getEditorAt(index);
    CC2ScriptEditor* scriptEditor = getScriptEditorAt(index);

    QString filename;
    int response = QMessageBox::NoButton;
    if (mapEditor && !mapEditor->isClean()) {
        filename = mapEditor->filename();
        response = QMessageBox::question(this, tr("Save Map"),
                            tr("Save changes to %1 before closing?")
                            .arg(filename.isEmpty() ? tr("new map") : QFileInfo(filename).fileName()),
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    } else if (scriptEditor && scriptEditor->isModified()) {
        filename = scriptEditor->filename();
        response = QMessageBox::question(this, tr("Save Script"),
                            tr("Save changes to %1 before closing?")
                            .arg(filename.isEmpty() ? tr("new script") : QFileInfo(filename).fileName()),
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    }

    if (response == QMessageBox::Cancel)
        return;
    if (response == QMessageBox::Yes) {
        if (!saveTab(index))
            return;
    }

    m_editorTabs->widget(index)->deleteLater();
}

void CC2EditMain::onTabChanged(int index)
{
    CC2EditorWidget* mapEditor = getEditorAt(index);
    CC2ScriptEditor* scriptEditor = getScriptEditorAt(index);

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
    }

    m_actions[ActionSave]->setEnabled(scriptEditor || mapEditor);
    m_actions[ActionSaveAs]->setEnabled(scriptEditor || mapEditor);
    m_actions[ActionUndo]->setEnabled(false);
    m_actions[ActionRedo]->setEnabled(false);
    m_actions[ActionCut]->setEnabled(false);
    m_actions[ActionCopy]->setEnabled(false);
    m_actions[ActionPaste]->setEnabled(false);
    m_actions[ActionClear]->setEnabled(false);
    m_actions[ActionToggleGreens]->setEnabled(!!mapEditor);
    m_actions[ActionTest]->setEnabled(!!mapEditor);
    m_drawModeGroup->setEnabled(!!mapEditor);
    m_modalToolGroup->setEnabled(!!mapEditor);

    m_positionLabel->setVisible(!!scriptEditor);

    if (scriptEditor) {
        m_actions[ActionUndo]->setEnabled(scriptEditor->canUndo());
        m_actions[ActionRedo]->setEnabled(scriptEditor->canRedo());

        QTextCursor cursor = scriptEditor->textCursor();
        m_actions[ActionCut]->setEnabled(cursor.hasSelection());
        m_actions[ActionCopy]->setEnabled(cursor.hasSelection());
        m_actions[ActionPaste]->setEnabled(scriptEditor->canPaste());
        m_actions[ActionClear]->setEnabled(cursor.hasSelection());
    } else if (mapEditor) {
        m_actions[ActionUndo]->setEnabled(mapEditor->canUndo());
        m_actions[ActionRedo]->setEnabled(mapEditor->canRedo());
        mapEditor->update();
        mapEditor->setDrawMode(m_currentDrawMode);

        const bool hasSelection = mapEditor->selection() != QRect(-1, -1, -1, -1);
        m_actions[ActionCut]->setEnabled(hasSelection);
        m_actions[ActionCopy]->setEnabled(hasSelection);
        m_actions[ActionPaste]->setEnabled(haveMapClipboardData());
        m_actions[ActionClear]->setEnabled(hasSelection);

        // Update the map properties page
        auto map = mapEditor->map();
        updateMapProperties(map);
        m_mapProperties->setEnabled(true);

        // Apply zoom
        resizeEvent(nullptr);
    }
}

void CC2EditMain::updateCounters(const cc2::MapData& mapData)
{
    const auto chips = mapData.countChips();
    if (std::get<0>(chips) != std::get<1>(chips))
        m_chipCounter->setText(tr("%1 (of %2)").arg(std::get<0>(chips)).arg(std::get<1>(chips)));
    else
        m_chipCounter->setText(QString::number(std::get<0>(chips)));
    const auto points = mapData.countPoints();
    if (std::get<1>(points) != 1) {
        m_pointCounter->setText(tr("%1 (x%2)").arg(std::get<0>(points))
                                        .arg(std::get<1>(points)));
    } else {
        m_pointCounter->setText(QString::number(std::get<0>(points)));
    }
}

void CC2EditMain::updateMapProperties(cc2::Map* map)
{
    m_title->setText(ccl::fromLatin1(map->title()));
    m_author->setText(ccl::fromLatin1(map->author()));
    m_lockText->setText(ccl::fromLatin1(map->lock()));
    m_editorVersion->setText(ccl::fromLatin1(map->editorVersion()));
    m_mapSize->setText(tr("%1 x %2").arg(map->mapData().width())
                                    .arg(map->mapData().height()));
    updateCounters(map->mapData());
    m_timeLimit->setValue(map->option().timeLimit());
    m_viewport->setCurrentIndex(static_cast<int>(map->option().view()));
    m_blobPattern->setCurrentIndex(static_cast<int>(map->option().blobPattern()));
    m_hideLogic->setChecked(map->option().hideLogic());
    m_cc1Boots->setChecked(map->option().cc1Boots());
    m_readOnly->setChecked(map->readOnly());
    m_clue->setPlainText(ccl::fromLatin1(map->clue()));
    m_note->setPlainText(ccl::fromLatin1(map->note()));
}

void CC2EditMain::setLeftTile(const cc2::Tile& tile)
{
    // Return to normal drawing mode if necessary
    m_actions[m_savedDrawMode]->setChecked(true);

    m_leftTile = tile;
    emit leftTileChanged(tile);
}

void CC2EditMain::setRightTile(const cc2::Tile& tile)
{
    // Return to normal drawing mode if necessary
    m_actions[m_savedDrawMode]->setChecked(true);

    m_rightTile = tile;
    emit rightTileChanged(tile);
}

void CC2EditMain::onTitleChanged(const QString& value)
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    if (map->title() != ccl::toLatin1(value)) {
        editor->beginEdit(CC2EditHistory::EditTitle);
        map->setTitle(ccl::toLatin1(value));
        editor->endEdit();
    }
}

void CC2EditMain::onAuthorChanged(const QString& value)
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    if (map->author() != ccl::toLatin1(value)) {
        editor->beginEdit(CC2EditHistory::EditAuthor);
        map->setAuthor(ccl::toLatin1(value));
        editor->endEdit();
    }
}

void CC2EditMain::onLockChanged(const QString& value)
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    if (map->lock() != ccl::toLatin1(value)) {
        editor->beginEdit(CC2EditHistory::EditLock);
        map->setLock(ccl::toLatin1(value));
        editor->endEdit();
    }
}

void CC2EditMain::onEditorVersionChanged(const QString& value)
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    if (map->editorVersion() != ccl::toLatin1(value)) {
        editor->beginEdit(CC2EditHistory::EditVersion);
        map->setEditorVersion(ccl::toLatin1(value));
        editor->endEdit();
    }
}

void CC2EditMain::onTimeLimitChanged(int value)
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    if (map->option().timeLimit() != value) {
        editor->beginEdit(CC2EditHistory::EditTime);
        map->option().setTimeLimit(value);
        editor->endEdit();
    }
}

void CC2EditMain::onViewportChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    auto value = static_cast<cc2::MapOption::Viewport>(m_viewport->currentData().toInt());
    if (map->option().view() != value) {
        editor->beginEdit(CC2EditHistory::EditOptions);
        map->option().setView(value);
        editor->endEdit();
    }
}

void CC2EditMain::onBlobPatternChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    auto value = static_cast<cc2::MapOption::BlobPattern>(m_blobPattern->currentData().toInt());
    if (map->option().blobPattern() != value) {
        editor->beginEdit(CC2EditHistory::EditOptions);
        map->option().setBlobPattern(value);
        editor->endEdit();
    }
}

void CC2EditMain::onHideLogicChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    bool value = m_hideLogic->isChecked();
    if (map->option().hideLogic() != value) {
        editor->beginEdit(CC2EditHistory::EditOptions);
        map->option().setHideLogic(value);
        editor->endEdit();
    }
}

void CC2EditMain::onCC1BootsChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    bool value = m_cc1Boots->isChecked();
    if (map->option().cc1Boots() != value) {
        editor->beginEdit(CC2EditHistory::EditOptions);
        map->option().setCc1Boots(value);
        editor->endEdit();
    }
}

void CC2EditMain::onReadOnlyChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    bool value = m_readOnly->isChecked();
    if (map->readOnly() != value) {
        editor->beginEdit(CC2EditHistory::EditOptions);
        map->setReadOnly(value);
        editor->endEdit();
    }
}

void CC2EditMain::onClueChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    const std::string value = ccl::toLatin1(m_clue->toPlainText());
    if (map->clue() != value) {
        editor->beginEdit(CC2EditHistory::EditClue);
        map->setClue(value);
        editor->endEdit();
    }
}

void CC2EditMain::onNoteChanged()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::Map* map = editor->map();
    const std::string value = ccl::toLatin1(m_note->toPlainText());
    if (map->note() != value) {
        editor->beginEdit(CC2EditHistory::EditNotes);
        map->setNote(value);
        editor->endEdit();
    }
}

void CC2EditMain::onResizeMap()
{
    CC2EditorWidget* editor = currentEditor();
    if (!editor)
        return;

    cc2::MapData& mapData = editor->map()->mapData();
    ResizeDialog dialog(QSize(mapData.width(), mapData.height()), this);
    if (dialog.exec() == QDialog::Accepted) {
        editor->resizeMap(dialog.requestedSize());
        updateMapProperties(editor->map());
    }
}

void CC2EditMain::onProcessError(QProcess::ProcessError err)
{
    if (err == QProcess::FailedToStart) {
        QMessageBox::critical(this, tr("Error starting process"),
                tr("Error starting test process.  Please check your settings "
                   "and try again"), QMessageBox::Ok);
    }
    onProcessFinished(-1, QProcess::NormalExit);
}

void CC2EditMain::onProcessFinished(int, QProcess::ExitStatus)
{
    // Clean up temporary files
    QDir chips2Dir(m_testGameDir);
    if (chips2Dir.exists(QStringLiteral("data/games/CC2Edit-playtest"))
            && chips2Dir.cd(QStringLiteral("data/games/CC2Edit-playtest"))) {
        chips2Dir.removeRecursively();
        chips2Dir.cd(QStringLiteral("../../.."));
    }
    if (chips2Dir.exists(QStringLiteral("data/saves/CC2Edit-playtest"))
            && chips2Dir.cd(QStringLiteral("data/saves/CC2Edit-playtest"))) {
        chips2Dir.removeRecursively();
        chips2Dir.cd(QStringLiteral("../../.."));
    }
    if (chips2Dir.exists(QStringLiteral("data/games"))
            && chips2Dir.cd(QStringLiteral("data/games"))) {
        if (chips2Dir.exists(QStringLiteral("save.c2l"))
                && chips2Dir.exists(QStringLiteral("save.c2l.CC2Edit-backup"))) {
            chips2Dir.remove(QStringLiteral("save.c2l"));
            chips2Dir.rename(QStringLiteral("save.c2l.CC2Edit-backup"),
                             QStringLiteral("save.c2l"));
        }
        chips2Dir.cd(QStringLiteral("../.."));
    }

    // Clean up subproc
    m_subProc->disconnect();
    m_subProc->deleteLater();
    m_subProc = nullptr;
}

void CC2EditMain::setGameName(const QString& name)
{
    if (name.isEmpty()) {
        if (m_currentGameScript.isEmpty())
            m_gameName->setText(tr("(No game script)"));
        else
            m_gameName->setText(tr("(Empty)"));
    } else {
        m_gameName->setText(name);
    }
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("CCTools"));
    QApplication::setApplicationName(QStringLiteral("CC2Edit"));

    QIcon appicon(QStringLiteral(":/icons/boot-32.png"));
    appicon.addFile(QStringLiteral(":/icons/boot-24.png"));
    appicon.addFile(QStringLiteral(":/icons/boot-16.png"));
    QApplication::setWindowIcon(appicon);

    CC2EditMain win;
    win.show();

    QStringList qtArgs = QApplication::arguments();
    if (qtArgs.size() > 1)
        win.loadFile(qtArgs.at(1));

    return QApplication::exec();
}
