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

#include "CCPlay.h"

#include <QApplication>
#include <QSettings>
#include <QSplitter>
#include <QToolButton>
#include <QToolBar>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <QDirModel>
#include <QCompleter>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QProcess>
#include "../Levelset.h"
#include "../DacFile.h"
#include "../IniFile.h"

CCPlayMain::CCPlayMain(QWidget* parent)
          : QMainWindow(parent)
{
    setWindowTitle("CCPlay 2.0 ALPHA");
    //QIcon appicon(":/icons/chip-48.png");
    //appicon.addFile(":/icons/chip-32.png");
    //appicon.addFile(":/icons/chip-24.png");
    //appicon.addFile(":/icons/chip-16.png");
    //setWindowIcon(appicon);

    QWidget* contents = new QWidget(this);
    m_levelsetPath = new QLineEdit(contents);
    QCompleter* dirCompleter = new QCompleter(this);
    dirCompleter->setModel(new QDirModel(QStringList(),
            QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot,
            QDir::Name, dirCompleter));
    m_levelsetPath->setCompleter(dirCompleter);
    QLabel* lblLevelsetPath = new QLabel(tr("Levelset &Path:"), contents);
    lblLevelsetPath->setBuddy(m_levelsetPath);
    QToolButton* btnOpenPath = new QToolButton(this);
    btnOpenPath->setAutoRaise(true);
    btnOpenPath->setIcon(QIcon(":/res/document-open-folder.png"));
    btnOpenPath->setStatusTip(tr("Browse for Levelset path"));

    QSplitter* splitLevelsetData = new QSplitter(Qt::Vertical, contents);
    m_levelsetList = new QTreeWidget(splitLevelsetData);
    m_levelsetList->setRootIsDecorated(false);
    m_levelsetList->setHeaderLabels(QStringList() << tr("Levelset")
            << tr("Levels") << tr("Highest") << tr("Last") << tr("My Score"));
    m_levelsetList->setColumnWidth(0, 160);
    m_levelsetList->setColumnWidth(1, m_levelsetList->fontMetrics().width(tr("Levels")) + 16);
    m_levelsetList->setColumnWidth(2, m_levelsetList->fontMetrics().width(tr("Highest")) + 16);
    m_levelsetList->setColumnWidth(3, m_levelsetList->fontMetrics().width(tr("Last")) + 16);
    m_levelsetList->setColumnWidth(4, m_levelsetList->fontMetrics().width(tr("My Score")) + 16);
    splitLevelsetData->addWidget(m_levelsetList);

    m_levelList = new QTreeWidget(splitLevelsetData);
    m_levelList->setRootIsDecorated(false);
    m_levelList->setHeaderLabels(QStringList() << "#" << tr("Name")
            << tr("Chips") << tr("Time") << tr("My Time") << tr("My Score"));
    m_levelList->setColumnWidth(0, m_levelList->fontMetrics().width("000") + 10);
    m_levelList->setColumnWidth(1, 160);
    m_levelList->setColumnWidth(2, m_levelList->fontMetrics().width(tr("Chips")) + 16);
    m_levelList->setColumnWidth(3, m_levelList->fontMetrics().width(tr("Time")) + 16);
    m_levelList->setColumnWidth(4, m_levelList->fontMetrics().width(tr("My Time")) + 16);
    m_levelList->setColumnWidth(5, m_levelList->fontMetrics().width(tr("My Score")) + 16);
    splitLevelsetData->addWidget(m_levelList);

    m_actions[ActionPlayMSCC] = new QAction(QIcon(":/res/play-chips.png"), tr("Play (MSCC)"), this);
    m_actions[ActionPlayMSCC]->setStatusTip(tr("Play level in CHIPS.EXE (F5)"));
    m_actions[ActionPlayMSCC]->setShortcut(Qt::Key_F5);
    m_actions[ActionPlayTWorld] = new QAction(QIcon(":/res/play-tworld.png"), tr("Play (TWorld)"), this);
    m_actions[ActionPlayTWorld]->setStatusTip(tr("Play level in Tile World (F6)"));
    m_actions[ActionPlayTWorld]->setShortcut(Qt::Key_F6);
    m_actions[ActionEdit] = new QAction(QIcon(":/res/edit-ccedit.png"), tr("Edit (CCEdit)"), this);
    m_actions[ActionEdit]->setStatusTip(tr("Edit level in CCEdit (F9)"));
    m_actions[ActionEdit]->setShortcut(Qt::Key_F9);
    m_actions[ActionSetup] = new QAction(QIcon(":/res/document-properties.png"), tr("Settings"), this);
    m_actions[ActionSetup]->setStatusTip(tr("Configure CCPlay settings"));
    m_actions[ActionSetup]->setShortcut(QKeySequence::Preferences);
    m_actions[ActionExit] = new QAction(QIcon(":/res/application-exit.png"), tr("Exit CCPlay"), this);
    m_actions[ActionExit]->setStatusTip(tr("Exit CCPlay"));
    m_actions[ActionExit]->setShortcut(QKeySequence::Quit);

    QToolBar* toolbar = new QToolBar(contents);
    toolbar->setIconSize(QSize(32, 32));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setOrientation(Qt::Vertical);
    QToolButton* tbPlay = new QToolButton(toolbar);
    tbPlay->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbPlay->setDefaultAction(m_actions[ActionPlayMSCC]);
    tbPlay->setPopupMode(QToolButton::MenuButtonPopup);
    tbPlay->setMenu(new QMenu(tbPlay));
    tbPlay->menu()->addAction(m_actions[ActionPlayMSCC]);
    tbPlay->menu()->addAction(m_actions[ActionPlayTWorld]);
    toolbar->addWidget(tbPlay);
    QToolButton* tbEdit = new QToolButton(toolbar);
    tbEdit->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    tbEdit->setDefaultAction(m_actions[ActionEdit]);
    tbEdit->setPopupMode(QToolButton::MenuButtonPopup);
    tbEdit->setMenu(new QMenu(tbEdit));
    toolbar->addWidget(tbEdit);
    toolbar->addSeparator();
    toolbar->addAction(m_actions[ActionSetup]);
    toolbar->addAction(m_actions[ActionExit]);

    // Form layout
    QGridLayout* layout = new QGridLayout(contents);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    layout->addWidget(lblLevelsetPath, 0, 0);
    layout->addWidget(m_levelsetPath, 0, 1);
    layout->addWidget(btnOpenPath, 0, 2);
    layout->addWidget(splitLevelsetData, 1, 0, 1, 3);
    layout->addWidget(toolbar, 0, 3, 2, 1);
    setCentralWidget(contents);
    statusBar();

    connect(m_actions[ActionPlayMSCC], SIGNAL(triggered()), SLOT(onPlayMSCC()));
    connect(m_actions[ActionPlayTWorld], SIGNAL(triggered()), SLOT(onPlayTWorld()));
    connect(m_actions[ActionEdit], SIGNAL(triggered()), SLOT(onEditDefault()));
    connect(m_actions[ActionExit], SIGNAL(triggered()), SLOT(close()));
    connect(btnOpenPath, SIGNAL(clicked()), SLOT(onBrowseLevelsetPath()));
    connect(m_levelsetPath, SIGNAL(textChanged(QString)), SLOT(onPathChanged(QString)));
    connect(m_levelsetList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(onLevelsetChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    QSettings settings("CCTools", "CCPlay");
    resize(settings.value("WindowSize", QSize(520, 400)).toSize());
    if (settings.value("WindowMaximized", false).toBool())
        showMaximized();
    if (settings.contains("WindowState"))
        restoreState(settings.value("WindowState").toByteArray());
    m_levelsetPath->setText(settings.value("LevelsetPath", QDir::currentPath()).toString());
}

bool CCPlayMain::initDatabase()
{
    QDir path;

    if (!m_scoredb.isDriverAvailable("QSQLITE")) {
        QMessageBox::critical(this, tr("SQLite Error"),
                tr("Error: Could not find Qt4 SQLite driver"));
        return false;
    }
    m_scoredb = QSqlDatabase::addDatabase("QSQLITE");

    path.setPath(QDir::homePath());
    if (!path.exists(".cctools") && !path.mkpath(".cctools")) {
        QMessageBox::critical(this, tr("Error creating data path"),
                tr("Error: Could not create CCTools data path"));
        return false;
    }
    if (!path.cd(".cctools")) {
        QMessageBox::critical(this, tr("Error setting data path"),
                tr("Error: Could not enter CCTools data path"));
        return false;
    }
    m_scoredb.setDatabaseName(path.absoluteFilePath("scoredb.db"));
    if (!m_scoredb.open()) {
        QMessageBox::critical(this, tr("SQLite Error"),
                tr("Error: Could not create or open score database"));
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS ccplay ("
               "  key TEXT NOT NULL,"
               "  value TEXT NOT NULL)");

    query.exec("SELECT value FROM ccplay WHERE key='version'");
    if (query.first()) {
        int version = query.value(0).toInt();
        if (version != 1) {
            QMessageBox::critical(this, tr("Database Error"),
                    tr("Error: Unrecognized database version!"));
            return false;
        }
    } else {
        query.exec("CREATE TABLE IF NOT EXISTS scores ("
                   "  levelset INTEGER NOT NULL,"
                   "  level_num INTEGER NOT NULL,"
                   "  my_time INTEGER NOT NULL,"
                   "  my_score INTEGER NOT NULL)");
        query.exec("CREATE TABLE IF NOT EXISTS levelsets ("
                   "  idx INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "  name TEXT NOT NULL,"
                   "  cur_level INTEGER NOT NULL,"
                   "  high_level INTEGER NOT NULL)");
        query.exec("INSERT INTO ccplay(key, value) VALUES('version', 1)");
    }

    onPathChanged(m_levelsetPath->text());
    return true;
}

void CCPlayMain::closeEvent(QCloseEvent*)
{
    QSettings settings("CCTools", "CCPlay");
    settings.setValue("WindowMaximized", (windowState() & Qt::WindowMaximized) != 0);
    showNormal();
    settings.setValue("WindowSize", size());
    settings.setValue("WindowState", saveState());
    settings.setValue("LevelsetPath", m_levelsetPath->text());
}

void CCPlayMain::onPlayMSCC()
{/*
    if (m_levelsetList->currentItem() == 0 || m_levelList->topLevelItemCount() == 0)
        return;

    QSettings settings("CCTools", "CCPlay");
    QString chipsExe = settings.value("ChipsExe").toString();
    if (chipsExe.isEmpty() || !QFile::exists(chipsExe)) {
        QMessageBox::critical(this, tr("Could not find CHIPS.EXE"),
                tr("Could not find Chip's Challenge executable.\n"
                   "Please configure MSCC in the CCPlay settings."));
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
                       "Please configure WINE in the CCPlay settings."));
            return;
        }
    }
#endif

    QString tempExe = QDir::tempPath() + "/CCRun.exe";
    QString tempDat = QDir::tempPath() + "/CCRun.dat";
    QFile::remove(tempExe);
    if (!QFile::copy(chipsExe, tempExe)) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error copying %1 to temp path").arg(chipsExe));
        return;
    }
    ccl::FileStream stream;
    if (!stream.open(tempExe.toUtf8().data(), "r+b")) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error opening %1 for writing").arg(tempExe));
        return;
    }

    // Make a CHIPS.EXE that we can use
    ccl::ChipsHax hax;
    hax.open(&stream);
    if (settings.value("ChipsPGPatch", false).toBool())
        hax.set_PGChips(ccl::CCPatchPatched);
    if (settings.value("ChipsCCPatch", true).toBool())
        hax.set_CCPatch(ccl::CCPatchPatched);
    hax.set_LastLevel(m_levelset->levelCount());
    if (m_useDac && m_dacInfo.m_lastLevel < m_levelset->levelCount())
        hax.set_FakeLastLevel(m_dacInfo.m_lastLevel);
    else
        hax.set_FakeLastLevel(m_levelset->levelCount());
    hax.set_IgnorePasswords(true);
    hax.set_DataFilename("CCRun.dat");
    hax.set_IniFilename("./CCRun.ini");
    hax.set_IniEntryName("CCPlay");
    stream.close();

    // Copy the levelset to the temp file
    unsigned int saveType = m_levelset->type();
    if (settings.value("ChipsPGPatch", false).toBool())
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

    QString tempIni = exePath.absoluteFilePath("CCRun.ini");
    FILE* iniStream = fopen(tempIni.toUtf8().data(), "r+t");
    if (iniStream == 0)
        iniStream = fopen(tempIni.toUtf8().data(), "w+t");
    if (iniStream == 0) {
        QMessageBox::critical(this, tr("Error Creating CCRun.ini"),
                tr("Error: Could not open or create CCRun.ini file"));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
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
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        return;
    }

    QDir::setCurrent(exePath.absolutePath());
#ifdef Q_OS_WIN32
    // Native execution
    QProcess::execute(tempExe);
#else
    // Try to use WINE
    QProcess::execute(winePath, QStringList() << tempExe);
#endif
    QDir::setCurrent(cwd);

    // Remove temp files
    QFile::remove(tempExe);
    QFile::remove(tempDat);
    QFile::remove(tempIni);
*/}

void CCPlayMain::onPlayTWorld()
{
    if (m_levelsetList->currentItem() == 0 || m_levelList->topLevelItemCount() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();

    QSettings settings("CCTools", "CCPlay");
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
                       "Please configure Tile World in the CCPlay settings."));
            return;
        }
#else
        QMessageBox::critical(this, tr("Could not find Tile World"),
                tr("Could not find Tile World executable.\n"
                   "Please configure Tile World in the CCPlay settings."));
        return;
#endif
    }

    QString cwd = QDir::currentPath();
    QDir exePath = tworldExe;
    exePath.cdUp();
    QString levelsetPath = QDir::toNativeSeparators(m_levelsetPath->text());
    QDir::setCurrent(exePath.absolutePath());
    QProcess::execute(tworldExe, QStringList() << "-D" << levelsetPath << filename);
    QDir::setCurrent(cwd);
}

void CCPlayMain::onEditDefault()
{
    if (m_levelsetList->currentItem() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();
    QProcess::execute("CCEdit", QStringList() << filename);
}

void CCPlayMain::onBrowseLevelsetPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Levelset Location"),
                                                     m_levelsetPath->text());
    if (!path.isEmpty())
        m_levelsetPath->setText(path);
}

void CCPlayMain::onPathChanged(QString path)
{
    QDir levelsetDir(path);
    if (!levelsetDir.exists() || !m_scoredb.isOpen())
        return;

    m_levelsetList->clear();
    QStringList setList = levelsetDir.entryList(QStringList() << "*.dat" << "*.ccl" << "*.dac",
                                                QDir::Files, QDir::Name);
    foreach (QString set, setList) {
        ccl::Levelset levelset;
        ccl::FileStream stream;
        QString filename = levelsetDir.absoluteFilePath(set);
        ccl::LevelsetType type = ccl::DetermineLevelsetType(filename.toUtf8().data());
        if (type == ccl::LevelsetCcl) {
            try {
                if (!stream.open(filename.toUtf8().data(), "rb"))
                    continue;
                levelset.read(&stream);
                stream.close();
            } catch (...) {
                continue;
            }
        } else if (type == ccl::LevelsetDac) {
            FILE* dac = fopen(filename.toUtf8().data(), "rt");
            if (dac == 0)
                continue;

            ccl::DacFile dacInfo;
            try {
                dacInfo.read(dac);
                fclose(dac);
            } catch (...) {
                fclose(dac);
                continue;
            }

            QDir searchPath(filename);
            searchPath.cdUp();
            try {
                if (!stream.open(searchPath.absoluteFilePath(dacInfo.m_filename.c_str()).toUtf8().data(), "rb"))
                    continue;
                levelset.read(&stream);
                stream.close();
            } catch (...) {
                continue;
            }
        }

        QString fileid = QDir(filename).absolutePath().section(QChar('/'), -1);
        QSqlQuery query(m_scoredb);
        query.exec(QString("SELECT idx, cur_level, high_level FROM levelsets WHERE name='%1'")
                  .arg(fileid.replace("'", "''")));
        int curLevel = 0, highLevel = 0, totScore = 0;
        if (query.first()) {
            int setid = query.value(0).toInt();
            curLevel = query.value(1).toInt();
            highLevel = query.value(2).toInt();

            query.exec(QString("SELECT SUM(my_score) FROM scores WHERE levelset=%1")
                       .arg(setid));
            if (query.first())
                totScore = query.value(0).toInt();
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelsetList);
        item->setText(0, set);
        item->setText(1, QString("%1").arg(levelset.levelCount()));
        item->setText(2, highLevel == 0 ? "---" : QString("%1").arg(highLevel));
        item->setText(3, curLevel == 0 ? "---" : QString("%1").arg(curLevel));
        item->setText(4, totScore == 0 ? "---" : QString("%1").arg(totScore));
        item->setData(0, Qt::UserRole, levelsetDir.absoluteFilePath(set));
    }
}

void CCPlayMain::onLevelsetChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    m_levelList->clear();
    if (item == 0)
        return;

    ccl::Levelset levelset;
    ccl::FileStream stream;
    QString filename = item->data(0, Qt::UserRole).toString();

    ccl::LevelsetType type = ccl::DetermineLevelsetType(filename.toUtf8().data());
    if (type == ccl::LevelsetCcl) {
        try {
            if (!stream.open(filename.toUtf8().data(), "rb")) {
                QMessageBox::critical(this, tr("Error Reading Levelset"),
                        tr("Error Opening levelset file %1").arg(filename));
                return;
            }
            levelset.read(&stream);
            stream.close();
        } catch (std::exception& e) {
            QMessageBox::critical(this, tr("Error Reading Levelset"),
                    tr("Error Reading levelset file: %1").arg(e.what()));
            return;
        }
    } else if (type == ccl::LevelsetDac) {
        FILE* dac = fopen(filename.toUtf8().data(), "rt");
        if (dac == 0) {
            QMessageBox::critical(this, tr("Error reading levelset"),
                                  tr("Could not open file: %1")
                                  .arg(filename));
            return;
        }
        ccl::DacFile dacInfo;
        try {
            dacInfo.read(dac);
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
        if (stream.open(searchPath.absoluteFilePath(dacInfo.m_filename.c_str()).toUtf8().data(), "rb")) {
            try {
                levelset.read(&stream);
                stream.close();
            } catch (std::exception& e) {
                QMessageBox::critical(this, tr("Error reading levelset"),
                                      tr("Error loading levelset: %1").arg(e.what()));
                stream.close();
                return;
            }
        } else {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                tr("Error: could not open file %1")
                                .arg(dacInfo.m_filename.c_str()));
            return;
        }
    } else {
        QMessageBox::critical(this, tr("Error reading levelset"),
                              tr("Cannot determine file type for %1").arg(filename));
        return;
    }

    QString fileid = QDir(filename).absolutePath().section(QChar('/'), -1);
    QSqlQuery query;
    query.exec(QString("SELECT idx, cur_level FROM levelsets WHERE name='%1'")
               .arg(fileid.replace("'", "''")));
    int setid = 0, curLevel = 0;
    if (query.first()) {
        setid = query.value(0).toInt();
        curLevel = query.value(1).toInt() - 1;
    }

    for (int i=0; i<levelset.levelCount(); ++i) {
        int myTime = 0, myScore = 0;
        if (setid > 0) {
            query.exec(QString("SELECT my_time, my_score FROM scores WHERE"
                               "  levelset=%1 AND level_num=%2")
                       .arg(setid).arg(i + 1));
            if (query.first()) {
                myTime = query.value(0).toInt();
                myScore = query.value(1).toInt();
            }
        }

        ccl::LevelData* level = levelset.level(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelList);
        item->setText(0, QString("%1").arg(level->levelNum()));
        item->setText(1, level->name().c_str());
        item->setText(2, level->chips() == 0 ? "---" : QString("%1").arg(level->chips()));
        item->setText(3, level->timer() == 0 ? "---" : QString("%1").arg(level->timer()));
        item->setText(4, myTime == 0 ? "---" : QString("%1").arg(myTime));
        item->setText(5, myScore == 0 ? "---" : QString("%1").arg(myScore));
    }

    if (curLevel < m_levelList->topLevelItemCount())
        m_levelList->setCurrentItem(m_levelList->topLevelItem(curLevel));
    else if (m_levelList->topLevelItemCount() > 0)
        m_levelList->setCurrentItem(m_levelList->topLevelItem(m_levelList->topLevelItemCount() - 1));
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CCPlayMain mainWin;
    mainWin.show();
    if (!mainWin.initDatabase())
        return 1;
    if (argc > 1)
        mainWin.setLevelsetPath(argv[1]);
    return app.exec();
}
