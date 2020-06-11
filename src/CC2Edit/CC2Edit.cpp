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

#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QAction>
#include <QDesktopWidget>
#include <QDockWidget>
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
#include <QFileDialog>

#define CC2EDIT_TITLE "CC2Edit 1.0"

Q_DECLARE_METATYPE(CC2ETileset*)

CC2EditMain::CC2EditMain(QWidget* parent)
    : QMainWindow(parent), m_currentTileset()
{
    setWindowTitle(CC2EDIT_TITLE);
    QIcon appicon(":/icons/boot-32.png");
    appicon.addFile(":/icons/boot-24.png");
    appicon.addFile(":/icons/boot-16.png");
    setWindowIcon(appicon);
    setDockOptions(QMainWindow::AnimatedDocks);

    // Actions
    m_actions[ActionNew] = new QAction(QIcon(":/res/document-new.png"), tr("&New Map..."), this);
    m_actions[ActionNew]->setStatusTip(tr("Create new map"));
    m_actions[ActionNew]->setShortcut(Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(QIcon(":/res/document-open.png"), tr("&Open Map..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a map file from disk"));
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

    m_actions[ActionAbout] = new QAction(QIcon(":/res/help-about.png"), tr("&About CC2Edit"), this);
    m_actions[ActionAbout]->setStatusTip(tr("Show information about CC2Edit"));

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
    int row = 0;
    mapPropsLayout->setContentsMargins(4, 4, 4, 4);
    mapPropsLayout->setVerticalSpacing(4);
    mapPropsLayout->setHorizontalSpacing(4);
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

    // Editor area
    m_editorTabs = new QTabWidget(this);
    m_editorTabs->setMovable(true);
    m_editorTabs->setTabsClosable(true);
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
    tbarTools->addAction(m_actions[ActionToggleWalls]);

    // Show status bar
    statusBar();

    connect(m_actions[ActionNew], &QAction::triggered, this, &CC2EditMain::createNewMap);
    connect(m_actions[ActionOpen], &QAction::triggered, this, &CC2EditMain::onOpenAction);
    connect(m_actions[ActionClose], &QAction::triggered, this, &CC2EditMain::onCloseAction);

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
}

void CC2EditMain::createNewMap()
{
    auto map = new cc2::Map;
    map->mapData().resize(32, 32);
    addEditor(map, tr("Untitled"));
    map->unref();

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
}

void CC2EditMain::loadMap(const QString& filename)
{
    ccl::FileStream fs;
    if (!fs.open(filename.toLocal8Bit().constData(), "rb")) {
        QMessageBox::critical(this, tr("Error loading map"),
                tr("Could not open %1 for reading.").arg(filename));
        return;
    }

    auto map = new cc2::Map;
    QFileInfo info(filename);
    try {
        map->read(&fs);
        addEditor(map, info.fileName());
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, tr("Error loading map"), ex.what());
    }
    map->unref();

    m_actions[ActionSave]->setEnabled(true);
    m_actions[ActionSaveAs]->setEnabled(true);
    m_actions[ActionClose]->setEnabled(true);
    m_actions[ActionGenReport]->setEnabled(true);
}

void CC2EditMain::registerTileset(const QString& filename)
{
    auto tileset = new CC2ETileset(this);
    tileset->load(filename);
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
    // TODO
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
    m_editorTabs->addTab(scroll, filename);
    resizeEvent(nullptr);

    connect(editor, &CC2EditorWidget::mouseInfo, statusBar(), &QStatusBar::showMessage);
    connect(editor, &CC2EditorWidget::canUndo, m_actions[ActionUndo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::canRedo, m_actions[ActionRedo], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCut], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionCopy], &QAction::setEnabled);
    connect(editor, &CC2EditorWidget::hasSelection, m_actions[ActionClear], &QAction::setEnabled);
    //connect(editor, &CC2EditorWidget::makeDirty, this, [map] { map->makeDirty(); });

    m_editorTabs->setCurrentWidget(scroll);
    return editor;
}

void CC2EditMain::closeAllTabs()
{
    while (m_editorTabs->count() != 0) {
        delete m_editorTabs->widget(0);
        m_editorTabs->removeTab(0);
    }
}

void CC2EditMain::onOpenAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Map..."),
                            m_dialogDir, "CC2 Maps (*.c2m)");
    if (!filename.isEmpty()) {
        loadMap(filename);
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

void CC2EditMain::onCloseTab(int index)
{
    const bool lastTab = m_editorTabs->count() == 1;
    m_editorTabs->widget(index)->deleteLater();

    if (lastTab) {
        m_actions[ActionSave]->setEnabled(false);
        m_actions[ActionSaveAs]->setEnabled(false);
        m_actions[ActionClose]->setEnabled(false);
        m_actions[ActionGenReport]->setEnabled(false);
    }
}

void CC2EditMain::onTabChanged(int index)
{
    CC2EditorWidget* editor = getEditorAt(index);
    if (!editor) {
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
        return;
    }

    editor->update();
    //editor->updateUndoStatus();

    /*
    bool hasSelection = editor->selection() != QRect(-1, -1, -1, -1);
    m_actions[ActionCut]->setEnabled(hasSelection);
    m_actions[ActionCopy]->setEnabled(hasSelection);
    m_actions[ActionClear]->setEnabled(hasSelection);
    */

    // Update the map properties page
    auto map = editor->map();
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
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    CC2EditMain win;
    win.show();

    QStringList qtArgs = app.arguments();
    if (qtArgs.size() > 1)
        win.loadMap(qtArgs.at(1));
    else
        win.createNewMap();

    return app.exec();
}
