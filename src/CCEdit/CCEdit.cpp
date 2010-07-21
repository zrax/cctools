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
#include <QLabel>
#include <QIntValidator>
#include <QGridLayout>
#include <QScrollArea>
#include <QToolBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <cstdio>

class QMinimalTextEdit : public QPlainTextEdit {
public:
    QMinimalTextEdit(QWidget* parent = 0)
    : QPlainTextEdit(parent) { }

    virtual QSize sizeHint() const
    { return QSize(minimumWidth(), minimumHeight()); }
};


CCEditMain::CCEditMain(QWidget* parent)
    : QMainWindow(parent), m_levelset(0)
{
    setWindowTitle("CCEdit 2.0");
    setDockOptions(QMainWindow::AnimatedDocks);

    // Control Toolbox
    QDockWidget* toolDock = new QDockWidget(this);
    toolDock->setObjectName("ToolDock");
    toolDock->setWindowTitle("Toolbox");
    QTabWidget* toolTabs = new QTabWidget(toolDock);
    toolTabs->setTabPosition(QTabWidget::West);
    toolDock->setWidget(toolTabs);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    QWidget* levelManWidget = new QWidget(toolDock);
    m_levelList = new QListWidget(levelManWidget);
    m_nameEdit = new QLineEdit(levelManWidget);
    QLabel* nameLabel = new QLabel(tr("&Name:"), levelManWidget);
    nameLabel->setBuddy(m_nameEdit);
    m_nameEdit->setMaxLength(31);
    m_passwordEdit = new QLineEdit(levelManWidget);
    QLabel* passLabel = new QLabel(tr("&Password:"), levelManWidget);
    passLabel->setBuddy(m_passwordEdit);
    m_passwordEdit->setMaxLength(9);
    m_passwordButton = new QToolButton(levelManWidget);
    m_passwordButton->setIcon(QIcon(":/res/view-refresh.png"));
    m_passwordButton->setStatusTip(tr("Generate new random level password"));
    m_chipEdit = new QLineEdit(levelManWidget);
    QLabel* chipLabel = new QLabel(tr("&Chips:"), levelManWidget);
    chipLabel->setBuddy(m_chipEdit);
    m_chipEdit->setValidator(new QIntValidator(0, 32767, m_chipEdit));
    m_chipsButton = new QToolButton(levelManWidget);
    m_chipsButton->setIcon(QIcon(":/res/view-refresh.png"));
    m_chipsButton->setStatusTip(tr("Count all chips in the selected level"));
    m_timeEdit = new QLineEdit(levelManWidget);
    QLabel* timeLabel = new QLabel(tr("&Time:"), levelManWidget);
    timeLabel->setBuddy(m_timeEdit);
    m_timeEdit->setValidator(new QIntValidator(0, 32767, m_timeEdit));
    m_hintEdit = new QMinimalTextEdit(levelManWidget);
    QLabel* hintLabel = new QLabel(tr("&Hint Text:"), levelManWidget);
    hintLabel->setBuddy(m_hintEdit);
    //m_hintEdit->setMaxLength(255);

    QGridLayout* levelManLayout = new QGridLayout(levelManWidget);
    levelManLayout->setContentsMargins(4, 4, 4, 4);
    levelManLayout->setVerticalSpacing(0);
    levelManLayout->setHorizontalSpacing(4);
    levelManLayout->addWidget(m_levelList, 0, 0, 1, 3);
    levelManLayout->addItem(new QSpacerItem(0, 8), 1, 0, 1, 3);
    levelManLayout->addWidget(nameLabel, 2, 0);
    levelManLayout->addWidget(m_nameEdit, 2, 1, 1, 2);
    levelManLayout->addWidget(passLabel, 3, 0);
    levelManLayout->addWidget(m_passwordEdit, 3, 1);
    levelManLayout->addWidget(m_passwordButton, 3, 2);
    levelManLayout->addWidget(chipLabel, 4, 0);
    levelManLayout->addWidget(m_chipEdit, 4, 1);
    levelManLayout->addWidget(m_chipsButton, 4, 2);
    levelManLayout->addWidget(timeLabel, 5, 0);
    levelManLayout->addWidget(m_timeEdit, 5, 1, 1, 2);
    levelManLayout->addItem(new QSpacerItem(0, 8), 6, 0, 1, 3);
    levelManLayout->addWidget(hintLabel, 7, 0, 1, 3);
    levelManLayout->addWidget(m_hintEdit, 8, 0, 1, 3);
    m_hintEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    toolDock->setFeatures(QDockWidget::DockWidgetMovable
                        | QDockWidget::DockWidgetClosable
                        | QDockWidget::DockWidgetFloatable);
    toolTabs->addTab(levelManWidget, tr("Level &Manager"));

    QToolBox* tileBox = new QToolBox(toolDock);
    m_tileLists[ListStandard] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListStandard], QList<tile_t>()
        << ccl::TileFloor << ccl::TileWall << ccl::TileChip << ccl::TileSocket
        << ccl::TileExit << ccl::TileHint << ccl::TileBarrier_N
        << ccl::TileBarrier_W << ccl::TileBarrier_S << ccl::TileBarrier_E
        << ccl::TileBarrier_SE << ccl::TileBlock << ccl::TileDirt
        << ccl::TileGravel << ccl::TilePlayer_S << ccl::TilePlayer_E
        << ccl::TilePlayer_W << ccl::TilePlayer_N);
    tileBox->addItem(m_tileLists[ListStandard], tr("Standard"));
    m_tileLists[ListObstacles] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListObstacles], QList<tile_t>()
        << ccl::TileWater << ccl::TileFire << ccl::TileBomb << ccl::TileForce_N
        << ccl::TileForce_W << ccl::TileForce_S << ccl::TileForce_E
        << ccl::TileForce_Rand << ccl::TileIce << ccl::TileIce_NW
        << ccl::TileIce_NE << ccl::TileIce_SE << ccl::TileIce_SW
        << ccl::TileTrap << ccl::TileTrapButton << ccl::TilePopUpWall
        << ccl::TileAppearingWall << ccl::TileInvisWall);
    tileBox->addItem(m_tileLists[ListObstacles], tr("Obstacles"));
    m_tileLists[ListDoors] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListDoors], QList<tile_t>()
        << ccl::TileDoor_Blue << ccl::TileDoor_Red << ccl::TileDoor_Green
        << ccl::TileDoor_Yellow << ccl::TileToggleFloor << ccl::TileToggleWall
        << ccl::TileToggleButton);
    tileBox->addItem(m_tileLists[ListDoors], tr("Doors"));
    m_tileLists[ListItems] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListItems], QList<tile_t>()
        << ccl::TileKey_Blue << ccl::TileKey_Red << ccl::TileKey_Green
        << ccl::TileKey_Yellow << ccl::TileFlippers << ccl::TileFireBoots
        << ccl::TileIceSkates << ccl::TileForceBoots);
    tileBox->addItem(m_tileLists[ListItems], tr("Items"));
    m_tileLists[ListMonsters] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListMonsters], QList<tile_t>()
        << ccl::TileBug_N << ccl::TileBug_E << ccl::TileBug_S << ccl::TileBug_W
        << ccl::TileFireball_N << ccl::TileFireball_E << ccl::TileFireball_S
        << ccl::TileFireball_W << ccl::TileBall_N << ccl::TileBall_E
        << ccl::TileBall_S << ccl::TileBall_W << ccl::TileTank_N
        << ccl::TileTank_E << ccl::TileTank_S << ccl::TileTank_W
        << ccl::TileTankButton << ccl::TileGlider_N << ccl::TileGlider_E
        << ccl::TileGlider_S << ccl::TileGlider_W << ccl::TileTeeth_N
        << ccl::TileTeeth_E << ccl::TileTeeth_S << ccl::TileTeeth_W
        << ccl::TileWalker_N << ccl::TileWalker_E << ccl::TileWalker_S
        << ccl::TileWalker_W << ccl::TileBlob_N << ccl::TileBlob_E
        << ccl::TileBlob_S << ccl::TileBlob_W << ccl::TileCrawler_N
        << ccl::TileCrawler_E << ccl::TileCrawler_S << ccl::TileCrawler_W);
    tileBox->addItem(m_tileLists[ListMonsters], tr("Monsters"));
    m_tileLists[ListMisc] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListMisc], QList<tile_t>()
        << ccl::TileThief << ccl::TileBlueWall << ccl::TileBlueFloor
        << ccl::TileTeleport << ccl::TileCloner << ccl::TileCloneButton
        << ccl::TileBlock_N << ccl::TileBlock_W << ccl::TileBlock_S
        << ccl::TileBlock_E);
    tileBox->addItem(m_tileLists[ListMisc], tr("Miscellaneous"));
    m_tileLists[ListSpecial] = new QListWidget(tileBox);
    m_tileset.addTiles(m_tileLists[ListSpecial], QList<tile_t>()
        << ccl::TilePlayerSplash << ccl::TilePlayerFire << ccl::TilePlayerBurnt
        << ccl::TilePlayerExit << ccl::TileExitAnim2 << ccl::TileExitAnim3
        << ccl::TilePlayerSwim_N << ccl::TilePlayerSwim_W
        << ccl::TilePlayerSwim_S << ccl::TilePlayerSwim_E << ccl::Tile_UNUSED_20
        << ccl::Tile_UNUSED_36 << ccl::Tile_UNUSED_37 << ccl::Tile_UNUSED_38);
    tileBox->addItem(m_tileLists[ListSpecial], tr("Special (Advanced)"));
    toolTabs->addTab(tileBox, tr("&Tiles"));

    // Main Editor
    QScrollArea* editorScroll = new QScrollArea(this);
    m_editor = new EditorWidget(editorScroll);
    editorScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editorScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editorScroll->setWidget(m_editor);
    setCentralWidget(editorScroll);

    // Actions
    m_actions[ActionNew] = new QAction(QIcon(":/res/document-new.png"), tr("&New Levelset..."), this);
    m_actions[ActionNew]->setStatusTip(tr("Create new Levelset"));
    m_actions[ActionNew]->setShortcut(Qt::Key_F2);
    m_actions[ActionOpen] = new QAction(QIcon(":/res/document-open.png"), tr("&Open Levelset..."), this);
    m_actions[ActionOpen]->setStatusTip(tr("Open a levelset from disk"));
    m_actions[ActionOpen]->setShortcut(QKeySequence::Open);
    m_actions[ActionSave] = new QAction(QIcon(":/res/document-save.png"), tr("&Save"), this);
    m_actions[ActionSave]->setStatusTip(tr("Save the current levelset to the same file"));
    m_actions[ActionSave]->setShortcut(QKeySequence::Save);
    m_actions[ActionSaveAs] = new QAction(QIcon(":/res/document-save-as.png"), tr("Save &As..."), this);
    m_actions[ActionSaveAs]->setStatusTip(tr("Save the current levelset to a new file or location"));
    m_actions[ActionSaveAs]->setShortcut(QKeySequence::SaveAs);
    m_actions[ActionExit] = new QAction(QIcon(":/res/application-exit.png"), tr("E&xit"), this);
    m_actions[ActionExit]->setStatusTip(tr("Close CCEdit"));
    m_actions[ActionExit]->setShortcut(QKeySequence::Quit);

    m_actions[ActionUndo] = new QAction(QIcon(":/res/edit-undo.png"), tr("&Undo"), this);
    m_actions[ActionUndo]->setStatusTip(tr("Undo the last edit"));
    m_actions[ActionUndo]->setShortcut(QKeySequence::Undo);
    m_actions[ActionUndo]->setEnabled(false);
    m_actions[ActionRedo] = new QAction(QIcon(":/res/edit-redo.png"), tr("&Redo"), this);
    m_actions[ActionRedo]->setStatusTip(tr("Redo the last edit"));
    m_actions[ActionRedo]->setShortcut(QKeySequence::Redo);
    m_actions[ActionRedo]->setEnabled(false);
    m_actions[ActionSelect] = new QAction(QIcon(":/res/edit-select.png"), tr("&Select"), this);
    m_actions[ActionSelect]->setStatusTip(tr("Enter selection mode"));
    m_actions[ActionSelect]->setShortcut(QKeySequence::SelectAll);
    m_actions[ActionSelect]->setCheckable(true);
    m_actions[ActionCut] = new QAction(QIcon(":/res/edit-cut.png"), tr("Cu&t"), this);
    m_actions[ActionCut]->setStatusTip(tr("Put the selection in the clipboard and clear it from the editor"));
    m_actions[ActionCut]->setShortcut(QKeySequence::Cut);
    m_actions[ActionCut]->setEnabled(false);
    m_actions[ActionCopy] = new QAction(QIcon(":/res/edit-copy.png"), tr("&Copy"), this);
    m_actions[ActionCopy]->setStatusTip(tr("Copy the current selection to the clipboard"));
    m_actions[ActionCopy]->setShortcut(QKeySequence::Copy);
    m_actions[ActionCopy]->setEnabled(false);
    m_actions[ActionPaste] = new QAction(QIcon(":/res/edit-paste.png"), tr("&Paste"), this);
    m_actions[ActionPaste]->setStatusTip(tr("Paste the clipboard contents into the levelset at the selection position"));
    m_actions[ActionPaste]->setShortcut(QKeySequence::Paste);
    m_actions[ActionPaste]->setEnabled(false);
    m_actions[ActionClear] = new QAction(QIcon(":/res/edit-delete.png"), tr("Clea&r"), this);
    m_actions[ActionClear]->setStatusTip(tr("Replace the selection contents with the current Background tile"));
    m_actions[ActionClear]->setShortcut(Qt::Key_Delete);
    m_actions[ActionClear]->setEnabled(false);
    m_actions[ActionFill] = new QAction(tr("&Fill"), this);
    m_actions[ActionFill]->setStatusTip(tr("Replace the selection contents with the current Foreground tile"));
    m_actions[ActionFill]->setShortcut(Qt::CTRL | Qt::Key_F);
    m_actions[ActionFill]->setEnabled(false);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNew]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionOpen]);
    fileMenu->addAction(m_actions[ActionSave]);
    fileMenu->addAction(m_actions[ActionSaveAs]);
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
    editMenu->addAction(m_actions[ActionFill]);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    m_tilesetMenu = viewMenu->addMenu(tr("&Tileset"));

    // Tool bars
    QToolBar* tbarFile = addToolBar(QString());
    tbarFile->addAction(m_actions[ActionNew]);
    tbarFile->addAction(m_actions[ActionOpen]);
    tbarFile->addAction(m_actions[ActionSave]);

    // Show status bar
    statusBar();

    connect(m_actions[ActionExit], SIGNAL(triggered()), this, SLOT(close()));
    connect(m_actions[ActionOpen], SIGNAL(triggered()), this, SLOT(onOpenAction()));
    connect(m_actions[ActionSelect], SIGNAL(toggled(bool)), this, SLOT(onSelectToggled(bool)));
    connect(m_levelList, SIGNAL(currentRowChanged(int)), this, SLOT(onSelectLevel(int)));
    connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onNameChanged(QString)));
    connect(m_passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordChanged(QString)));
    connect(m_passwordButton, SIGNAL(clicked()), this, SLOT(onPasswordGenAction()));
    connect(m_chipEdit, SIGNAL(textChanged(QString)), this, SLOT(onChipsChanged(QString)));
    connect(m_chipsButton, SIGNAL(clicked()), this, SLOT(onChipCountAction()));
    connect(m_timeEdit, SIGNAL(textChanged(QString)), this, SLOT(onTimerChanged(QString)));
    connect(m_editor, SIGNAL(mouseInfo(QString)), statusBar(), SLOT(showMessage(QString)));

    // Visual tweaks
    resize(800, 600);
    toolDock->resize(120, 0);
    loadTileset(":/TW32.tis");
}

void CCEditMain::loadLevelset(QString filename)
{
    if (m_levelset != 0 && !closeLevelset())
        return;

    FILE* set = fopen(filename.toUtf8(), "rb");
    if (set != 0) {
        m_levelset = new ccl::Levelset(0);
        try {
            m_levelset->read(set);
        } catch (ccl::Exception e) {
            QMessageBox::critical(this, tr("Error reading levelset"),
                                tr("Error loading levelset: %1").arg(e.what()));
            delete m_levelset;
            m_levelset = 0;
        }
        fclose(set);
    } else {
        QMessageBox::critical(this, tr("Error opening levelset"),
                              tr("Error: could not open file %1").arg(filename));
    }

    if (m_levelset != 0) {
        for (int i=0; i<m_levelset->levelCount(); ++i) {
            m_levelList->addItem(QString("%1 - %2")
                        .arg(m_levelset->level(i)->levelNum())
                        .arg(QString::fromAscii(m_levelset->level(i)->name().c_str())));
        }
        m_levelsetFilename = filename;
        if (m_levelset->levelCount() > 0)
            m_levelList->setCurrentRow(0);
    }
}

bool CCEditMain::closeLevelset()
{
    if (m_levelset == 0)
        return true;

    int reply =  QMessageBox::question(this, tr("Close levelset"),
                          tr("Save changes to %1 before closing?")
                          .arg(m_levelsetFilename),
                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return false;

    m_levelList->clear();
    onSelectLevel(-1);
    delete m_levelset;
    m_levelset = 0;
    return true;
}

void CCEditMain::loadTileset(QString filename)
{
    m_tileset.load(filename);
    m_editor->setTileset(&m_tileset);
    for (int i=0; i<NUM_TILE_LISTS; ++i)
        m_tileset.imageTiles(m_tileLists[i]);
}

void CCEditMain::onOpenAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Levelset..."),
                            QString(), "CC Levelsets (*.dat *.ccl)");
    if (!filename.isEmpty())
        loadLevelset(filename);
}

void CCEditMain::onSelectToggled(bool mode)
{
    m_actions[ActionCut]->setEnabled(mode);
    m_actions[ActionCopy]->setEnabled(mode);
    m_actions[ActionPaste]->setEnabled(mode);
    m_actions[ActionClear]->setEnabled(mode);
    m_actions[ActionFill]->setEnabled(mode);
}

void CCEditMain::onSelectLevel(int idx)
{
    if (m_levelset == 0 || idx < 0) {
        m_nameEdit->setText(QString());
        m_passwordEdit->setText(QString());
        m_chipEdit->setText(QString());
        m_timeEdit->setText(QString());
        m_hintEdit->setPlainText(QString());
        m_editor->setLevelData(0);
    } else {
        ccl::LevelData* level = m_levelset->level(idx);
        m_nameEdit->setText(QString::fromAscii(level->name().c_str()));
        m_passwordEdit->setText(QString::fromAscii(level->password().c_str()));
        m_chipEdit->setText(QString("%1").arg(level->chips()));
        m_timeEdit->setText(QString("%1").arg(level->timer()));
        m_hintEdit->setPlainText(QString::fromAscii(level->hint().c_str()));
        m_editor->setLevelData(level);
    }
    m_editor->update();
}

void CCEditMain::onPasswordGenAction()
{
    if (m_levelList->currentRow() < 0)
        return;
    m_passwordEdit->setText(QString::fromAscii(ccl::Levelset::RandomPassword().c_str()));
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
    m_chipEdit->setText(QString("%1").arg(chips));
}

void CCEditMain::onNameChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;

    ccl::LevelData* level = m_levelset->level(m_levelList->currentRow());
    level->setName(m_nameEdit->text().toAscii().data());
    m_levelList->currentItem()->setText(QString("%1 - %2")
                                        .arg(level->levelNum()).arg(value));
}

void CCEditMain::onPasswordChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;
    m_levelset->level(m_levelList->currentRow())->setPassword(m_passwordEdit->text().toAscii().data());
}

void CCEditMain::onChipsChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;
    m_levelset->level(m_levelList->currentRow())->setChips(m_chipEdit->text().toInt());
}

void CCEditMain::onTimerChanged(QString value)
{
    if (m_levelList->currentRow() < 0)
        return;
    m_levelset->level(m_levelList->currentRow())->setTimer(m_timeEdit->text().toInt());
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    CCEditMain mainWin;
    mainWin.show();
    if (argc > 1)
        mainWin.loadLevelset(argv[1]);
    return app.exec();
}
