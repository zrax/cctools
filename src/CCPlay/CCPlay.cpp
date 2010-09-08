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
#include "PlaySettings.h"
#include "../Levelset.h"
#include "../DacFile.h"
#include "../IniFile.h"
#include "../ChipsHax.h"

static ccl::Levelset* load_levelset(QString filename, QWidget* self,
                                    int* dacLastLevel = 0)
{
    ccl::LevelsetType type = ccl::DetermineLevelsetType(filename.toUtf8().data());
    ccl::FileStream stream;
    ccl::Levelset* levelset;
    if (type == ccl::LevelsetCcl) {
        try {
            if (!stream.open(filename.toUtf8().data(), "rb")) {
                QMessageBox::critical(self, self->tr("Error Reading Levelset"),
                        self->tr("Error Opening levelset file %1").arg(filename));
                return 0;
            }
            levelset = new ccl::Levelset();
            levelset->read(&stream);
            stream.close();
        } catch (std::exception& e) {
            QMessageBox::critical(self, self->tr("Error Reading Levelset"),
                    self->tr("Error Reading levelset file: %1").arg(e.what()));
            delete levelset;
            return 0;
        }
        if (dacLastLevel != 0)
            *dacLastLevel = (levelset->levelCount() == 149) ? 144 : levelset->levelCount();
        return levelset;
    } else if (type == ccl::LevelsetDac) {
        FILE* dac = fopen(filename.toUtf8().data(), "rt");
        if (dac == 0) {
            QMessageBox::critical(self, self->tr("Error reading levelset"),
                                  self->tr("Could not open file: %1")
                                  .arg(filename));
            return 0;
        }
        ccl::DacFile dacInfo;
        try {
            dacInfo.read(dac);
            fclose(dac);
        } catch (ccl::Exception& e) {
            QMessageBox::critical(self, self->tr("Error reading levelset"),
                                  self->tr("Error loading levelset descriptor: %1")
                                  .arg(e.what()));
            fclose(dac);
            return 0;
        }

        QDir searchPath(filename);
        searchPath.cdUp();
        if (stream.open(searchPath.absoluteFilePath(dacInfo.m_filename.c_str()).toUtf8().data(), "rb")) {
            try {
                levelset = new ccl::Levelset();
                levelset->read(&stream);
                stream.close();
            } catch (std::exception& e) {
                QMessageBox::critical(self, self->tr("Error reading levelset"),
                                      self->tr("Error loading levelset: %1").arg(e.what()));
                stream.close();
                delete levelset;
                return 0;
            }
            if (dacLastLevel != 0)
                *dacLastLevel = (dacInfo.m_lastLevel == 0) ? levelset->levelCount() : dacInfo.m_lastLevel;
            return levelset;
        } else {
            QMessageBox::critical(self, self->tr("Error opening levelset"),
                                  self->tr("Error: could not open file %1")
                                  .arg(dacInfo.m_filename.c_str()));
            return 0;
        }
    } else {
        QMessageBox::critical(self, self->tr("Error reading levelset"),
                              self->tr("Cannot determine file type for %1").arg(filename));
        return 0;
    }
}

CCPlayMain::CCPlayMain(QWidget* parent)
          : QMainWindow(parent)
{
    setWindowTitle("CCPlay 2.0 ALPHA");
    QIcon appicon(":/icons/chip-48.png");
    appicon.addFile(":/icons/chip-32.png");
    appicon.addFile(":/icons/chip-24.png");
    appicon.addFile(":/icons/chip-16.png");
    setWindowIcon(appicon);

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
    m_actions[ActionEdit] = new QAction(tr("Custom Tool"), this);
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
    QWidget* alignPlayButton = new QWidget(toolbar);
    m_playButton = new QToolButton(alignPlayButton);
    m_playButton->setAutoRaise(true);
    m_playButton->setIconSize(QSize(32, 32));
    m_playButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_playButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_playButton->setMenu(new QMenu(m_playButton));
    m_playButton->menu()->addAction(m_actions[ActionPlayMSCC]);
    m_playButton->menu()->addAction(m_actions[ActionPlayTWorld]);
    QLayout* layPlayButton = new QGridLayout(alignPlayButton);
    layPlayButton->setContentsMargins(0, 0, 0, 0);
    layPlayButton->addWidget(m_playButton);
    toolbar->addWidget(alignPlayButton);
    QWidget* alignEditButton = new QWidget(toolbar);
    m_editButton = new QToolButton(alignEditButton);
    m_editButton->setAutoRaise(true);
    m_editButton->setIconSize(QSize(32, 32));
    m_editButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_editButton->setDefaultAction(m_actions[ActionEdit]);
    m_editButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_editButton->setMenu(new QMenu(m_editButton));
    QLayout* layEditButton = new QGridLayout(alignEditButton);
    layEditButton->setContentsMargins(0, 0, 0, 0);
    layEditButton->addWidget(m_editButton);
    toolbar->addWidget(alignEditButton);
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
    connect(m_editButton->menu(), SIGNAL(triggered(QAction*)), SLOT(onEditor(QAction*)));
    connect(m_actions[ActionSetup], SIGNAL(triggered()), SLOT(onSetup()));
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

    SettingsDialog::CheckEditors(settings);
    refreshTools();
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

void CCPlayMain::refreshTools()
{
    QSettings settings("CCTools", "CCPlay");

    if (settings.value("DefaultGame").toString() == "TWorld")
        m_playButton->setDefaultAction(m_actions[ActionPlayTWorld]);
    else
        m_playButton->setDefaultAction(m_actions[ActionPlayMSCC]);

    QStringList editors = settings.value("EditorNames").toStringList();
    QStringList editorIcons = settings.value("EditorIcons").toStringList();
    QStringList editorPaths = settings.value("EditorPaths").toStringList();
    m_editButton->menu()->clear();
    for (int i=0; i<editors.size(); ++i) {
        QAction* action = m_editButton->menu()->addAction(
                    SettingsDialog::IconForEditor(editorIcons[i]),
                    tr("Edit (%1)").arg(editors[i]));
        action->setStatusTip(tr("Edit with %1").arg(editors[i]));
        action->setData(editorPaths[i]);
    }
    if (editors.size() > 0) {
        m_actions[ActionEdit]->setEnabled(true);
        m_actions[ActionEdit]->setText(tr("Edit (%1)").arg(editors[0]));
        m_actions[ActionEdit]->setIcon(SettingsDialog::IconForEditor(editorIcons[0]));
        m_actions[ActionEdit]->setStatusTip(tr("Edit with %1 (F9)").arg(editors[0]));
    } else {
        m_actions[ActionEdit]->setEnabled(false);
        m_actions[ActionEdit]->setText(tr("Edit (N/A)"));
        QPixmap emptyIcon(32, 32);
        emptyIcon.fill(Qt::darkGray);
        m_actions[ActionEdit]->setIcon(QIcon(emptyIcon));
        m_actions[ActionEdit]->setStatusTip(tr("No editors configured"));
    }
}

void CCPlayMain::refreshScores()
{
    QString curFile = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();
    onPathChanged(m_levelsetPath->text());
    for (int i=0; i<m_levelsetList->topLevelItemCount(); ++i) {
        if (m_levelsetList->topLevelItem(i)->data(0, Qt::UserRole).toString() == curFile) {
            m_levelsetList->setCurrentItem(m_levelsetList->topLevelItem(i));
            break;
        }
    }
}

void CCPlayMain::onPlayMSCC()
{
    if (m_levelsetList->currentItem() == 0 || m_levelList->topLevelItemCount() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();

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

    // Save the levelset to temp file, and extract useful information
    ccl::FileStream stream;
    int dacLastLevel;
    ccl::Levelset* levelset = load_levelset(filename, this, &dacLastLevel);
    if (levelset == 0)
        return;
    if (!stream.open(tempDat.toUtf8().data(), "wb")) {
        QMessageBox::critical(this, tr("Error writing data file"),
                tr("Error opening CCRun.dat for writing"));
        stream.close();
        delete levelset;
        return;
    }
    try {
        levelset->write(&stream);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.what()));
        stream.close();
        QFile::remove(tempDat);
        delete levelset;
        return;
    }
    stream.close();

    // Make a CHIPS.EXE that we can use
    QFile::remove(tempExe);
    if (!QFile::copy(chipsExe, tempExe)) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error copying %1 to temp path").arg(chipsExe));
        QFile::remove(tempDat);
        delete levelset;
        return;
    }
    if (!stream.open(tempExe.toUtf8().data(), "r+b")) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error opening %1 for writing").arg(tempExe));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        delete levelset;
        return;
    }

    ccl::ChipsHax hax;
    hax.open(&stream);
    if (settings.value("UseCCPatch", true).toBool())
        hax.set_CCPatch(ccl::CCPatchPatched);
    if (settings.value("UseIgnorePasswords", false).toBool())
        hax.set_IgnorePasswords(true);
    if (settings.value("UseAlwaysFirstTry", false).toBool())
        hax.set_AlwaysFirstTry(true);
    if (levelset->type() == ccl::Levelset::TypePG || levelset->type() == ccl::Levelset::TypeLynxPG)
        hax.set_PGChips(ccl::CCPatchPatched);
    hax.set_LastLevel(levelset->levelCount());
    hax.set_FakeLastLevel(dacLastLevel);
    hax.set_DataFilename("CCRun.dat");
    hax.set_IniFilename("./CCRun.ini");
    hax.set_IniEntryName("CCPlay Runtime");
    stream.close();

    // Configure the INI file
    QString cwd = QDir::currentPath();
    QDir exePath = chipsExe;
    exePath.cdUp();

    QSqlQuery query;
    QString setName = filename.section(QChar('/'), -1);
    query.exec(QString("SELECT idx, high_level, cur_level FROM levelsets"
                       "  WHERE name='%1'").arg(setName));
    int setid = 0;
    int highLevel = 1, curLevel = 1;
    if (query.first()) {
        setid = query.value(0).toInt();
        highLevel = query.value(1).toInt();
        curLevel = query.value(2).toInt();
    }

    QString tempIni = exePath.absoluteFilePath("CCRun.ini");
    FILE* iniStream = fopen(tempIni.toUtf8().data(), "r+t");
    if (iniStream == 0)
        iniStream = fopen(tempIni.toUtf8().data(), "w+t");
    if (iniStream == 0) {
        QMessageBox::critical(this, tr("Error Creating CCRun.ini"),
                tr("Error: Could not open or create CCRun.ini file"));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        delete levelset;
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream);
        ini.setSection("CCPlay Runtime");
        ini.setInt("Current Level", curLevel);
        ini.setInt("Highest Level", highLevel);
        bool haveCurLevel = false;
        if (setid > 0) {
            for (int i=0; i<levelset->levelCount(); ++i) {
                query.exec(QString("SELECT my_time, my_score FROM scores"
                                   "  WHERE levelset=%1 AND level_num=%2")
                                   .arg(setid).arg(i + 1));
                if (!query.first())
                    continue;
                ini.setString(QString("Level%1").arg(i + 1).toUtf8().data(),
                              QString("%1,%2,%3").arg(levelset->level(i)->password().c_str())
                                                 .arg(query.value(0).toInt())
                                                 .arg(query.value(1).toInt())
                                                 .toUtf8().data());
                if (i + 1 == curLevel)
                    haveCurLevel = true;
            }
        }
        if (!haveCurLevel) {
            ini.setString(QString("Level%1").arg(curLevel).toUtf8().data(),
                          levelset->level(curLevel - 1)->password());
        }
        ini.write(iniStream);
        fclose(iniStream);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error writing CCRun.ini"),
                tr("Error writing INI file: %1").arg(e.what()));
        fclose(iniStream);
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        delete levelset;
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


    iniStream = fopen(tempIni.toUtf8().data(), "rt");
    if (iniStream == 0) {
        QMessageBox::critical(this, tr("Error Reading CCRun.ini"),
                tr("Error: Could not open CCRun.ini file for reading"));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        delete levelset;
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream);
        ini.setSection("CCPlay Runtime");
        curLevel = ini.getInt("Current Level");
        highLevel = ini.getInt("Highest Level");

        if (setid == 0) {
            query.exec(QString("INSERT INTO levelsets(name, cur_level, high_level)"
                               "  VALUES('%1', %2, %3)")
                       .arg(setName).arg(curLevel).arg(highLevel));
            setid = query.lastInsertId().toInt();
        }

        for (int i=0; i<levelset->levelCount(); ++i) {
            QString levelData = ini.getString(QString("Level%1").arg(i + 1).toUtf8().data()).c_str();
            if (levelData.isEmpty())
                continue;

            QStringList parts = levelData.split(QChar(','));
            if (parts.size() == 1) {
                // Just a password...  Ignore it and move along
                continue;
            } else if (parts.size() == 3) {
                query.exec(QString("SELECT my_time, my_score FROM scores"
                                   "  WHERE levelset=%1 AND level_num=%2")
                                   .arg(setid).arg(i + 1));
                if (!query.first()) {
                    query.exec(QString("INSERT INTO scores(levelset, level_num, my_time, my_score)"
                                       "  VALUES(%1, %2, %3, %4)")
                               .arg(setid).arg(i + 1).arg(parts[1].toInt()).arg(parts[2].toInt()));
                } else {
                    bool betterTime = parts[1].toInt() > query.value(0).toInt();
                    bool betterScore = parts[2].toInt() > query.value(1).toInt();
                    if (betterTime || betterScore) {
                        query.exec(QString("UPDATE scores SET my_time=%1, my_score=%2"
                                           "  WHERE levelset=%3 AND level_num=%4")
                                   .arg(parts[1].toInt()).arg(parts[2].toInt())
                                   .arg(setid).arg(i + 1));
                    }
                }
            } else {
                qDebug("Error parsing score: %s", levelData.toUtf8().data());
                continue;
            }
        }
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error reading CCRun.ini"),
                tr("Error reading INI file: %1").arg(e.what()));
        fclose(iniStream);
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        delete levelset;
        return;
    }

    // Clean up our mess
    QFile::remove(tempExe);
    QFile::remove(tempDat);
    QFile::remove(tempIni);
    delete levelset;

    refreshScores();
}

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

    QString setName = filename.section(QChar('/'), -1);
    QString cwd = QDir::currentPath();
    QDir exePath = tworldExe;
    exePath.cdUp();
    QString levelsetPath = QDir::toNativeSeparators(m_levelsetPath->text());
    QDir::setCurrent(exePath.absolutePath());
    QStringList twargs;
    twargs << "-D" << QDir::toNativeSeparators(levelsetPath)
           << "-S" << QDir::toNativeSeparators(QDir::homePath() + "/.cctools");
    if (settings.value("UseIgnorePasswords", false).toBool())
        twargs << "-p";
    twargs << QDir::toNativeSeparators(filename) << setName + ".tws";
    QProcess::execute(tworldExe, twargs);
    QDir::setCurrent(cwd);

    // Parse the TWS file and extract score data
    ccl::FileStream tws;
    QString twsName = QDir::toNativeSeparators(QDir::homePath() + "/.cctools/" + setName + ".tws");
    if (!tws.open(twsName.toUtf8().data(), "rb")) {
        QMessageBox::critical(this, tr("Error parsing score data"),
                tr("Error: Could not open %1 for reading").arg(twsName));
        tws.close();
        return;
    }

    if (tws.read32() != 0x999B3335) {
        QMessageBox::critical(this, tr("Error parsing score data"),
                tr("Error: TWS file is in an invalid or unrecognized format"));
        tws.close();
        return;
    }
    tws.read8();    // Ruleset
    tws.read8();    // Reserved
    tws.read8();    // Reserved
    size_t extraBytes = tws.read8();
    tws.seek(extraBytes, SEEK_CUR);

    ccl::Levelset* levelset = load_levelset(filename, this);
    if (levelset == 0) {
        tws.close();
        return;
    }

    QSqlQuery query;
    query.exec(QString("SELECT idx, high_level FROM levelsets"
                       "  WHERE name='%1").arg(setName));
    int setid = 0;
    int highLevel = 0;
    if (query.first()) {
        setid = query.value(0).toInt();
        highLevel = query.value(1).toInt();
    } else {
        query.exec(QString("INSERT INTO levelsets(name, cur_level, high_level)"
                           "  VALUES('%1', 1, 1)").arg(setName));
        setid = query.lastInsertId().toInt();
    }

    while (!tws.eof()) {
        size_t size = tws.read32();
        if (size == 0xFFFFFFFF)
            return;
        if (size == 0)
            continue;

        uint16_t levelNum = tws.read16();
        uint32_t levelPass = tws.read32();
        if (levelNum > highLevel)
            highLevel = levelNum;

        if (size == 6) {
            // No score info
            continue;
        }
        if (levelNum == 0 && levelPass == 0) {
            // Special levelset name field
            tws.seek(size - 6, SEEK_CUR);
            continue;
        }

        // Normal record
        tws.read8();    // Reserved
        tws.read8();    // Initial slide/step dir
        tws.read32();   // RNG seed
        unsigned int ticks = tws.read32();
        tws.seek(size - 16, SEEK_CUR);

        // Add score info
        int levelTimer = levelset->level(levelNum - 1)->timer();
        int timeScore = (levelTimer == 0) ? 0 : levelTimer - (ticks / 20);
        int bestScore = levelNum * 500 + timeScore * 10;
        query.exec(QString("SELECT my_time, my_score FROM scores"
                           "  WHERE levelset=%1 AND level_num=%2")
                   .arg(setid).arg(levelNum));
        if (query.first()) {
            int storedTime = query.value(0).toInt();
            int storedScore = query.value(1).toInt();
            if (storedTime < timeScore || storedScore < bestScore) {
                query.exec(QString("UPDATE scores"
                                   "  SET my_time=%1, my_score=%2"
                                   "  WHERE levelset=%3 AND level_num=%4")
                           .arg(timeScore).arg(bestScore)
                           .arg(setid).arg(levelNum));
            }
        } else {
            query.exec(QString("INSERT INTO scores(levelset, level_num, my_time, my_score)"
                               "  VALUES(%1, %2, %3, %4)")
                       .arg(setid).arg(levelNum)
                       .arg(timeScore).arg(bestScore));
        }
    }
    delete levelset;

    // TWorld does not store highest and last level separately, so store
    // the highest level into both fields
    query.exec(QString("UPDATE levelsets SET cur_level=%1, high_level=%2"
                       "  WHERE idx=%3").arg(highLevel).arg(highLevel).arg(setid));
    refreshScores();
}

void CCPlayMain::onEditDefault()
{
    if (m_editButton->menu()->isEmpty())
        return;
    onEditor(m_editButton->menu()->actions()[0]);
}

void CCPlayMain::onEditor(QAction* action)
{
    if (m_levelsetList->currentItem() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();
    int curLevel = 1;
    if (m_levelList->currentItem() != 0)
        curLevel = m_levelList->indexOfTopLevelItem(m_levelList->currentItem()) + 1;

    QStringList launch = action->data().toString().split('|');
    QStringList params = launch[1].split(' ', QString::SkipEmptyParts);
    for (int i=0; i<params.size(); ++i) {
        params[i].replace("%F", QDir::toNativeSeparators(filename))
                 .replace("%L", QString("%1").arg(curLevel));
    }
    QProcess::execute(launch[0], params);
}

void CCPlayMain::onSetup()
{
    SettingsDialog dlg;
    if (dlg.exec() == QDialog::Accepted)
        refreshTools();
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
    QStringList setList = levelsetDir.entryList(QStringList() << "*.dat" << "*.DAT" << "*.ccl" << "*.dac",
                                                QDir::Files, QDir::Name);
    foreach (QString set, setList) {
        QString filename = levelsetDir.absoluteFilePath(set);
        ccl::Levelset* levelset = load_levelset(filename, this);
        if (levelset == 0)
            continue;

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
        item->setText(1, QString("%1").arg(levelset->levelCount()));
        item->setText(2, highLevel == 0 ? "---" : QString("%1").arg(highLevel));
        item->setText(3, curLevel == 0 ? "---" : QString("%1").arg(curLevel));
        item->setText(4, totScore == 0 ? "---" : QString("%1").arg(totScore));
        item->setData(0, Qt::UserRole, levelsetDir.absoluteFilePath(set));
        delete levelset;
    }
}

void CCPlayMain::onLevelsetChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    m_levelList->clear();
    if (item == 0)
        return;

    QString filename = item->data(0, Qt::UserRole).toString();
    ccl::Levelset* levelset = load_levelset(filename, this);
    if (levelset == 0)
        return;

    QString fileid = filename.section(QChar('/'), -1);
    QSqlQuery query;
    query.exec(QString("SELECT idx, cur_level FROM levelsets WHERE name='%1'")
               .arg(fileid.replace("'", "''")));
    int setid = 0, curLevel = 0;
    if (query.first()) {
        setid = query.value(0).toInt();
        curLevel = query.value(1).toInt() - 1;
    }

    for (int i=0; i<levelset->levelCount(); ++i) {
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

        ccl::LevelData* level = levelset->level(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelList);
        item->setText(0, QString("%1").arg(level->levelNum()));
        item->setText(1, level->name().c_str());
        item->setText(2, level->chips() == 0 ? "---" : QString("%1").arg(level->chips()));
        item->setText(3, level->timer() == 0 ? "---" : QString("%1").arg(level->timer()));
        item->setText(4, myTime == 0 ? "---" : QString("%1").arg(myTime));
        item->setText(5, myScore == 0 ? "---" : QString("%1").arg(myScore));
    }
    delete levelset;

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
