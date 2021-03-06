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
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QProcess>
#include <QStandardPaths>
#include "PlaySettings.h"
#include "libcc1/Levelset.h"
#include "libcc1/DacFile.h"
#include "libcc1/IniFile.h"
#include "libcc1/ChipsHax.h"
#include "libcc1/CCMetaData.h"
#include "CommonWidgets/CCTools.h"
#include "CommonWidgets/PathCompleter.h"

std::unique_ptr<ccl::Levelset>
CCPlayMain::loadLevelset(const QString& filename, int* dacLastLevel)
{
    ccl::LevelsetType type = ccl::DetermineLevelsetType(filename);
    ccl::FileStream stream;
    std::unique_ptr<ccl::Levelset> levelset;
    if (type == ccl::LevelsetCcl) {
        try {
            if (!stream.open(filename, ccl::FileStream::Read)) {
                QMessageBox::critical(this, tr("Error Reading Levelset"),
                        tr("Error Opening levelset file %1").arg(filename));
                return {};
            }
            levelset = std::make_unique<ccl::Levelset>();
            levelset->read(&stream);
        } catch (const ccl::RuntimeError& e) {
            qDebug("Error trying to load %s: %s", qPrintable(filename),
                   qPrintable(e.message()));
            QMessageBox::critical(this, tr("Error Reading Levelset"),
                    tr("Error Reading levelset file %1: %2")
                    .arg(filename).arg(e.message()));
            return {};
        }
        if (dacLastLevel)
            *dacLastLevel = (levelset->levelCount() == 149) ? 144 : levelset->levelCount();
        return levelset;
    } else if (type == ccl::LevelsetDac) {
        ccl::unique_FILE dac = ccl::FileStream::Fopen(filename, ccl::FileStream::ReadText);
        if (!dac) {
            QMessageBox::critical(this, tr("Error reading levelset"),
                                  tr("Could not open file: %1")
                                  .arg(filename));
            return {};
        }
        ccl::DacFile dacInfo;
        try {
            dacInfo.read(dac.get());
        } catch (const ccl::FormatError&) {
            // Tried to read an invalid levelset file (probably not really
            // a levelset -- e.g. "unins000.dat")
            qDebug("Format error trying to load %s", qPrintable(filename));
            return {};
        } catch (const ccl::RuntimeError& e) {
            qDebug("Error trying to load %s: %s", qPrintable(filename),
                   qPrintable(e.message()));
            QMessageBox::critical(this, tr("Error reading levelset"),
                                  tr("Error loading levelset descriptor for %1: %2")
                                  .arg(filename).arg(e.message()));
            return {};
        }

        QDir searchPath(filename);
        searchPath.cdUp();
        if (stream.open(searchPath.absoluteFilePath(dacInfo.m_filename), ccl::FileStream::Read)) {
            try {
                levelset = std::make_unique<ccl::Levelset>();
                levelset->read(&stream);
            } catch (const ccl::RuntimeError& e) {
                qDebug("Error trying to load %s: %s", qPrintable(filename),
                       qPrintable(e.message()));
                QMessageBox::critical(this, tr("Error reading levelset"),
                                      tr("Error loading levelset: %1").arg(e.message()));
                return {};
            }
            if (dacLastLevel)
                *dacLastLevel = (dacInfo.m_lastLevel == 0) ? levelset->levelCount() : dacInfo.m_lastLevel;
            return levelset;
        } else {
            QMessageBox::critical(this, tr("Error opening levelset"),
                                  tr("Error: could not open file %1")
                                  .arg(dacInfo.m_filename));
            return {};
        }
    } else {
        QMessageBox::critical(this, tr("Error reading levelset"),
                              tr("Cannot determine file type for %1").arg(filename));
        return {};
    }
}

CCPlayMain::CCPlayMain(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("CCPlay " CCTOOLS_VERSION));

    auto contents = new QWidget(this);
    m_levelsetPath = new QLineEdit(contents);
    m_levelsetPath->setCompleter(new DirCompleter(this));
    auto lblLevelsetPath = new QLabel(tr("Levelset &Path:"), contents);
    lblLevelsetPath->setBuddy(m_levelsetPath);
    auto btnRefresh = new QToolButton(this);
    btnRefresh->setAutoRaise(true);
    btnRefresh->setIcon(ICON("view-refresh"));
    btnRefresh->setStatusTip(tr("Refresh Levelsets"));
    auto btnOpenPath = new QToolButton(this);
    btnOpenPath->setAutoRaise(true);
    btnOpenPath->setIcon(ICON("document-open-folder"));
    btnOpenPath->setStatusTip(tr("Browse for Levelset path"));

    auto splitLevelsetData = new QSplitter(Qt::Vertical, contents);
    m_levelsetList = new QTreeWidget(splitLevelsetData);
    m_levelsetList->setRootIsDecorated(false);
    m_levelsetList->setHeaderLabels(QStringList{
        tr("Levelset"), tr("Levels"), tr("Highest"), tr("Last"), tr("My Score")
    });
    m_levelsetList->setColumnWidth(0, 160);
    m_levelsetList->setColumnWidth(1, m_levelsetList->fontMetrics().boundingRect(tr("Levels")).width() + 16);
    m_levelsetList->setColumnWidth(2, m_levelsetList->fontMetrics().boundingRect(tr("Highest")).width() + 16);
    m_levelsetList->setColumnWidth(3, m_levelsetList->fontMetrics().boundingRect(tr("Last")).width() + 16);
    m_levelsetList->setColumnWidth(4, m_levelsetList->fontMetrics().boundingRect(tr("My Score")).width() + 16);
    splitLevelsetData->addWidget(m_levelsetList);

    m_levelList = new QTreeWidget(splitLevelsetData);
    m_levelList->setRootIsDecorated(false);
    m_levelList->setHeaderLabels(QStringList{
        QStringLiteral("#"), tr("Name"), tr("Author"), tr("Time"),
        tr("My Time"), tr("My Score")
    });
    m_levelList->setColumnWidth(0, m_levelList->fontMetrics().boundingRect(QStringLiteral("000")).width() + 10);
    m_levelList->setColumnWidth(1, 160);
    m_levelList->setColumnWidth(2, 80);
    m_levelList->setColumnWidth(3, m_levelList->fontMetrics().boundingRect(tr("Time")).width() + 16);
    m_levelList->setColumnWidth(4, m_levelList->fontMetrics().boundingRect(tr("My Time")).width() + 16);
    m_levelList->setColumnWidth(5, m_levelList->fontMetrics().boundingRect(tr("My Score")).width() + 16);
    splitLevelsetData->addWidget(m_levelList);

    m_actions[ActionPlayMSCC] = new QAction(ICON("play-chips"), tr("Play (MSCC)"), this);
    m_actions[ActionPlayMSCC]->setStatusTip(tr("Play level in CHIPS.EXE (F5)"));
    m_actions[ActionPlayMSCC]->setShortcut(Qt::Key_F5);
    m_actions[ActionPlayTWorld] = new QAction(ICON("play-tworld"), tr("Play (TWorld)"), this);
    m_actions[ActionPlayTWorld]->setStatusTip(tr("Play level in Tile World (F6)"));
    m_actions[ActionPlayTWorld]->setShortcut(Qt::Key_F6);
    m_actions[ActionTool] = new QAction(tr("Custom Tool"), this);
    m_actions[ActionTool]->setShortcut(Qt::Key_F9);
    m_actions[ActionSetup] = new QAction(ICON("document-properties"), tr("Settings"), this);
    m_actions[ActionSetup]->setStatusTip(tr("Configure CCPlay settings"));
    m_actions[ActionSetup]->setShortcut(QKeySequence::Preferences);
    m_actions[ActionExit] = new QAction(ICON("application-exit"), tr("Exit CCPlay"), this);
    m_actions[ActionExit]->setStatusTip(tr("Exit CCPlay"));
    m_actions[ActionExit]->setShortcut(QKeySequence::Quit);

    auto toolbar = new QToolBar(contents);
    toolbar->setIconSize(QSize(32, 32));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setOrientation(Qt::Vertical);
    auto alignPlayButton = new QWidget(toolbar);
    m_playButton = new QToolButton(alignPlayButton);
    m_playButton->setAutoRaise(true);
    m_playButton->setIconSize(QSize(32, 32));
    m_playButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_playButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_playButton->setMenu(new QMenu(m_playButton));
    m_playButton->menu()->addAction(m_actions[ActionPlayMSCC]);
    m_playButton->menu()->addAction(m_actions[ActionPlayTWorld]);
    auto layPlayButton = new QGridLayout(alignPlayButton);
    layPlayButton->setContentsMargins(0, 0, 0, 0);
    layPlayButton->addWidget(m_playButton);
    toolbar->addWidget(alignPlayButton);
    auto alignOpenToolButton = new QWidget(toolbar);
    m_openToolButton = new QToolButton(alignOpenToolButton);
    m_openToolButton->setAutoRaise(true);
    m_openToolButton->setIconSize(QSize(32, 32));
    m_openToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_openToolButton->setDefaultAction(m_actions[ActionTool]);
    m_openToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_openToolButton->setMenu(new QMenu(m_openToolButton));
    QLayout* layOpenToolButton = new QGridLayout(alignOpenToolButton);
    layOpenToolButton->setContentsMargins(0, 0, 0, 0);
    layOpenToolButton->addWidget(m_openToolButton);
    toolbar->addWidget(alignOpenToolButton);
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
    layout->addWidget(btnRefresh, 0, 2);
    layout->addWidget(btnOpenPath, 0, 3);
    layout->addWidget(splitLevelsetData, 1, 0, 1, 4);
    layout->addWidget(toolbar, 0, 4, 2, 1);
    setCentralWidget(contents);
    statusBar();

    connect(m_actions[ActionPlayMSCC], &QAction::triggered, this, &CCPlayMain::onPlayMSCC);
    connect(m_actions[ActionPlayTWorld], &QAction::triggered, this, &CCPlayMain::onPlayTWorld);
    connect(m_actions[ActionTool], &QAction::triggered, this, &CCPlayMain::onToolDefault);
    connect(m_openToolButton->menu(), &QMenu::triggered, this, &CCPlayMain::onTool);
    connect(m_actions[ActionSetup], &QAction::triggered, this, &CCPlayMain::onSetup);
    connect(m_actions[ActionExit], &QAction::triggered, this, &CCPlayMain::close);
    connect(btnRefresh, &QToolButton::clicked, this, &CCPlayMain::onRefreshLevelsets);
    connect(btnOpenPath, &QToolButton::clicked, this, &CCPlayMain::onBrowseLevelsetPath);
    connect(m_levelsetPath, &QLineEdit::textChanged, this, &CCPlayMain::onPathChanged);
    connect(m_levelsetList, &QTreeWidget::currentItemChanged,
            this, &CCPlayMain::onLevelsetChanged);

    QSettings settings;
    resize(settings.value(QStringLiteral("WindowSize"), QSize(520, 400)).toSize());
    if (settings.value(QStringLiteral("WindowMaximized"), false).toBool())
        showMaximized();
    if (settings.contains(QStringLiteral("WindowState")))
        restoreState(settings.value(QStringLiteral("WindowState")).toByteArray());
    m_levelsetPath->setText(settings.value(QStringLiteral("LevelsetPath"),
                                           QDir::currentPath()).toString());

    SettingsDialog::CheckTools(settings);
    refreshTools();
}

bool CCPlayMain::initDatabase()
{
    QDir path;

    if (!m_scoredb.isDriverAvailable(QStringLiteral("QSQLITE"))) {
        QMessageBox::critical(this, tr("SQLite Error"),
                tr("Error: Could not find Qt SQLite driver"));
        return false;
    }
    m_scoredb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));

    path.setPath(QDir::homePath());
    const QString cctoolsDir = QStringLiteral(".cctools");
    if (!path.exists(cctoolsDir) && !path.mkpath(cctoolsDir)) {
        QMessageBox::critical(this, tr("Error creating data path"),
                tr("Error: Could not create CCTools data path"));
        return false;
    }
    if (!path.cd(cctoolsDir)) {
        QMessageBox::critical(this, tr("Error setting data path"),
                tr("Error: Could not enter CCTools data path"));
        return false;
    }
    m_scoredb.setDatabaseName(path.absoluteFilePath(QStringLiteral("scoredb.db")));
    if (!m_scoredb.open()) {
        QMessageBox::critical(this, tr("SQLite Error"),
                tr("Error: Could not create or open score database"));
        return false;
    }

    QSqlQuery query;
    query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS ccplay ("
            "  key TEXT NOT NULL,"
            "  value TEXT NOT NULL)"));

    query.exec(QStringLiteral("SELECT value FROM ccplay WHERE key='version'"));
    if (query.first()) {
        int version = query.value(0).toInt();
        if (version != 1) {
            QMessageBox::critical(this, tr("Database Error"),
                    tr("Error: Unrecognized database version!"));
            return false;
        }
    } else {
        query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS scores ("
                "  levelset INTEGER NOT NULL,"
                "  level_num INTEGER NOT NULL,"
                "  my_time INTEGER NOT NULL,"
                "  my_score INTEGER NOT NULL)"));
        query.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS levelsets ("
                "  idx INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  name TEXT NOT NULL,"
                "  cur_level INTEGER NOT NULL,"
                "  high_level INTEGER NOT NULL)"));
        query.exec(QStringLiteral(
                "INSERT INTO ccplay(key, value) VALUES('version', 1)"));
    }

    onPathChanged(m_levelsetPath->text());
    return true;
}

void CCPlayMain::closeEvent(QCloseEvent*)
{
    QSettings settings;
    settings.setValue(QStringLiteral("WindowMaximized"),
                      (windowState() & Qt::WindowMaximized) != 0);
    showNormal();
    settings.setValue(QStringLiteral("WindowSize"), size());
    settings.setValue(QStringLiteral("WindowState"), saveState());
    settings.setValue(QStringLiteral("LevelsetPath"), m_levelsetPath->text());
}

void CCPlayMain::refreshTools()
{
    QSettings settings;

    const QString defaultGame = settings.value(QStringLiteral("DefaultGame")).toString();
    if (defaultGame == QLatin1String("TWorld2") || defaultGame == QLatin1String("TWorld"))
        m_playButton->setDefaultAction(m_actions[ActionPlayTWorld]);
    else
        m_playButton->setDefaultAction(m_actions[ActionPlayMSCC]);

    QStringList tools = settings.value(QStringLiteral("EditorNames")).toStringList();
    QStringList toolIcons = settings.value(QStringLiteral("EditorIcons")).toStringList();
    QStringList toolPaths = settings.value(QStringLiteral("EditorPaths")).toStringList();
    m_openToolButton->menu()->clear();
    for (int i = 0; i < tools.size(); ++i) {
        QAction* action = m_openToolButton->menu()->addAction(
                    SettingsDialog::IconForTool(toolIcons[i]),
                    tr("Open (%1)").arg(tools[i]));
        action->setStatusTip(tr("Open with %1").arg(tools[i]));
        action->setData(toolPaths[i]);
    }
    if (tools.size() > 0) {
        m_actions[ActionTool]->setEnabled(true);
        m_actions[ActionTool]->setText(tr("Open (%1)").arg(tools[0]));
        m_actions[ActionTool]->setIcon(SettingsDialog::IconForTool(toolIcons[0]));
        m_actions[ActionTool]->setStatusTip(tr("Open with %1 (F9)").arg(tools[0]));
    } else {
        m_actions[ActionTool]->setEnabled(false);
        m_actions[ActionTool]->setText(tr("Open (N/A)"));
        QPixmap emptyIcon(32, 32);
        emptyIcon.fill(Qt::darkGray);
        m_actions[ActionTool]->setIcon(QIcon(emptyIcon));
        m_actions[ActionTool]->setStatusTip(tr("No tools configured"));
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
    if (!m_levelsetList->currentItem() || m_levelList->topLevelItemCount() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();

    QSettings settings;
    QString chipsExe = settings.value(QStringLiteral("ChipsExe")).toString();
    if (chipsExe.isEmpty() || !QFile::exists(chipsExe)) {
        QMessageBox::critical(this, tr("Could not find CHIPS.EXE"),
                tr("Could not find Chip's Challenge executable.\n"
                   "Please configure MSCC in the CCPlay settings."));
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
                       "Please configure WINE in the CCPlay settings."));
            return;
        }
    }
#endif

    QString tempExe = QDir::tempPath() + QStringLiteral("/CCRun.exe");
    QString tempDat = QDir::tempPath() + QStringLiteral("/CCRun.dat");

    // Save the levelset to temp file, and extract useful information
    ccl::FileStream stream;
    int dacLastLevel;
    auto levelset = loadLevelset(filename, &dacLastLevel);
    if (!levelset)
        return;
    if (!stream.open(tempDat, ccl::FileStream::Write)) {
        QMessageBox::critical(this, tr("Error writing data file"),
                tr("Error opening CCRun.dat for writing"));
        return;
    }
    try {
        levelset->write(&stream);
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error Creating Test Data File"),
                tr("Error writing data file: %1").arg(e.message()));
        stream.close();
        QFile::remove(tempDat);
        return;
    }
    stream.close();

    // Make a CHIPS.EXE that we can use
    QFile::remove(tempExe);
    if (!QFile::copy(chipsExe, tempExe)) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error copying %1 to temp path").arg(chipsExe));
        QFile::remove(tempDat);
        return;
    }
    if (!stream.open(tempExe, ccl::FileStream::ReadWrite)) {
        QMessageBox::critical(this, tr("Error creating temp EXE"),
                tr("Error opening %1 for writing").arg(tempExe));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        return;
    }

    ccl::ChipsHax hax;
    hax.open(&stream);
    if (settings.value(QStringLiteral("UseCCPatch"), true).toBool())
        hax.set_CCPatch(ccl::CCPatchPatched);
    if (settings.value(QStringLiteral("UseIgnorePasswords"), false).toBool())
        hax.set_IgnorePasswords(true);
    if (settings.value(QStringLiteral("UseAlwaysFirstTry"), false).toBool())
        hax.set_AlwaysFirstTry(true);
    if (levelset->type() == ccl::Levelset::TypePG || levelset->type() == ccl::Levelset::TypeLynxPG)
        hax.set_PGChips(ccl::CCPatchPatched);
    hax.set_LastLevel(levelset->levelCount());
    hax.set_FakeLastLevel(dacLastLevel);
    hax.set_DataFilename("CCRun.dat");
    hax.set_IniFilename(".\\CCRun.ini");
    hax.set_IniEntryName("CCPlay Runtime");
    stream.close();

    // Configure the INI file
    QString cwd = QDir::currentPath();
    QDir exePath = QFileInfo(chipsExe).absoluteDir();

    QSqlQuery query;
    QString setName = filename.section(QLatin1Char('/'), -1);
    query.exec(QStringLiteral("SELECT idx, high_level, cur_level FROM levelsets"
                              "  WHERE name='%1'").arg(setName));
    int setid = 0;
    int highLevel = 1, curLevel = 1;
    if (query.first()) {
        setid = query.value(0).toInt();
        highLevel = query.value(1).toInt();
        curLevel = query.value(2).toInt();
    }

    QString tempIni = exePath.absoluteFilePath(QStringLiteral("CCRun.ini"));
    ccl::unique_FILE iniStream = ccl::FileStream::Fopen(tempIni, ccl::FileStream::ReadWriteText);
    if (!iniStream)
        iniStream = ccl::FileStream::Fopen(tempIni, ccl::FileStream::RWCreateText);
    if (!iniStream) {
        QMessageBox::critical(this, tr("Error Creating CCRun.ini"),
                tr("Error: Could not open or create CCRun.ini file"));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream.get());
        ini.setSection("CCPlay Runtime");
        ini.setInt("Current Level", curLevel);
        ini.setInt("Highest Level", highLevel);
        bool haveCurLevel = false;
        if (setid > 0) {
            for (int i=0; i<levelset->levelCount(); ++i) {
                query.exec(QStringLiteral("SELECT my_time, my_score FROM scores"
                                          "  WHERE levelset=%1 AND level_num=%2")
                                          .arg(setid).arg(i + 1));
                if (!query.first())
                    continue;
                ini.setString(ccl::toLatin1(QStringLiteral("Level%1").arg(i + 1)),
                              ccl::toLatin1(QStringLiteral("%1,%2,%3")
                                                    .arg(ccl::fromLatin1(levelset->level(i)->password()))
                                                    .arg(query.value(0).toInt())
                                                    .arg(query.value(1).toInt())));
                if (i + 1 == curLevel)
                    haveCurLevel = true;
            }
        }
        if (!haveCurLevel) {
            ini.setString(ccl::toLatin1(QStringLiteral("Level%1").arg(curLevel)),
                          levelset->level(curLevel - 1)->password());
        }
        ini.write(iniStream.get());
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error writing CCRun.ini"),
                tr("Error writing INI file: %1").arg(e.message()));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        return;
    }
    iniStream.reset();      // Force a close of the file...

    QDir::setCurrent(exePath.absolutePath());
    if (!winePath.isEmpty()) {
        // Launch with Wine (Unix) or WineVDM (Windows)
        QProcess::execute(winePath, QStringList{ tempExe });
    } else {
        // Native execution
        QProcess::execute(tempExe, QStringList());
    }
    QDir::setCurrent(cwd);


    iniStream = ccl::FileStream::Fopen(tempIni, ccl::FileStream::ReadText);
    if (!iniStream) {
        QMessageBox::critical(this, tr("Error Reading CCRun.ini"),
                tr("Error: Could not open CCRun.ini file for reading"));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        return;
    }
    try {
        ccl::IniFile ini;
        ini.read(iniStream.get());
        ini.setSection("CCPlay Runtime");
        curLevel = ini.getInt("Current Level");
        highLevel = ini.getInt("Highest Level");

        if (setid == 0) {
            query.exec(QStringLiteral("INSERT INTO levelsets(name, cur_level, high_level)"
                                      "  VALUES('%1', %2, %3)")
                       .arg(setName).arg(curLevel).arg(highLevel));
            setid = query.lastInsertId().toInt();
        } else {
            query.exec(QStringLiteral("UPDATE levelsets SET cur_level=%1, high_level=%2"
                                      "  WHERE name='%3'")
                       .arg(curLevel).arg(highLevel).arg(setName));
        }

        for (int i=0; i<levelset->levelCount(); ++i) {
            QString levelData = ccl::fromLatin1(ini.getString(ccl::toLatin1(QStringLiteral("Level%1").arg(i + 1))));
            if (levelData.isEmpty())
                continue;

            QStringList parts = levelData.split(QLatin1Char(','));
            if (parts.size() == 1) {
                // Just a password...  Ignore it and move along
                continue;
            } else if (parts.size() == 3) {
                query.exec(QStringLiteral("SELECT my_time, my_score FROM scores"
                                          "  WHERE levelset=%1 AND level_num=%2")
                                          .arg(setid).arg(i + 1));
                if (!query.first()) {
                    query.exec(QStringLiteral("INSERT INTO scores(levelset, level_num, my_time, my_score)"
                                              "  VALUES(%1, %2, %3, %4)")
                               .arg(setid).arg(i + 1).arg(parts[1].toInt()).arg(parts[2].toInt()));
                } else {
                    bool betterTime = parts[1].toInt() > query.value(0).toInt();
                    bool betterScore = parts[2].toInt() > query.value(1).toInt();
                    if (betterTime || betterScore) {
                        query.exec(QStringLiteral("UPDATE scores SET my_time=%1, my_score=%2"
                                                  "  WHERE levelset=%3 AND level_num=%4")
                                   .arg(parts[1].toInt()).arg(parts[2].toInt())
                                   .arg(setid).arg(i + 1));
                    }
                }
            } else {
                qDebug("Error parsing score: %s", qPrintable(levelData));
                continue;
            }
        }
    } catch (const ccl::RuntimeError& e) {
        QMessageBox::critical(this, tr("Error reading CCRun.ini"),
                tr("Error reading INI file: %1").arg(e.message()));
        QFile::remove(tempExe);
        QFile::remove(tempDat);
        QFile::remove(tempIni);
        return;
    }

    // Clean up our mess
    QFile::remove(tempExe);
    QFile::remove(tempDat);
    QFile::remove(tempIni);

    refreshScores();
}

void CCPlayMain::onPlayTWorld()
{
    if (!m_levelsetList->currentItem() || m_levelList->topLevelItemCount() == 0)
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();

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
                       "Please configure Tile World in the CCPlay settings."));
            return;
        }
    }

    QString setName = filename.section(QLatin1Char('/'), -1);
    QString cwd = QDir::currentPath();
    QDir exePath = tworldExe;
    exePath.cdUp();
    QString levelsetPath = QDir::toNativeSeparators(m_levelsetPath->text());
    QDir::setCurrent(exePath.absolutePath());
    QStringList twargs {
        QStringLiteral("-D"), QDir::toNativeSeparators(levelsetPath),
        QStringLiteral("-S"), QDir::toNativeSeparators(QDir::homePath() + QStringLiteral("/.cctools"))
    };
    if (settings.value(QStringLiteral("UseIgnorePasswords"), false).toBool())
        twargs << QStringLiteral("-p");
    twargs << QDir::toNativeSeparators(filename) << setName + QStringLiteral(".tws");
    QProcess::execute(tworldExe, twargs);
    QDir::setCurrent(cwd);

    // Parse the TWS file and extract score data
    ccl::FileStream tws;
    QString twsName = QDir::toNativeSeparators(QDir::homePath() + QStringLiteral("/.cctools/")
                                               + setName + QStringLiteral(".tws"));
    if (!tws.open(twsName, ccl::FileStream::Read)) {
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

    auto levelset = loadLevelset(filename);
    if (!levelset) {
        tws.close();
        return;
    }

    QSqlQuery query;
    query.exec(QStringLiteral("SELECT idx, high_level FROM levelsets"
                              "  WHERE name='%1").arg(setName));
    int setid = 0;
    int highLevel = 0;
    if (query.first()) {
        setid = query.value(0).toInt();
        highLevel = query.value(1).toInt();
    } else {
        query.exec(QStringLiteral("INSERT INTO levelsets(name, cur_level, high_level)"
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
        query.exec(QStringLiteral("SELECT my_time, my_score FROM scores"
                                  "  WHERE levelset=%1 AND level_num=%2")
                   .arg(setid).arg(levelNum));
        if (query.first()) {
            int storedTime = query.value(0).toInt();
            int storedScore = query.value(1).toInt();
            if (storedTime < timeScore || storedScore < bestScore) {
                query.exec(QStringLiteral("UPDATE scores"
                                          "  SET my_time=%1, my_score=%2"
                                          "  WHERE levelset=%3 AND level_num=%4")
                           .arg(timeScore).arg(bestScore)
                           .arg(setid).arg(levelNum));
            }
        } else {
            query.exec(QStringLiteral("INSERT INTO scores(levelset, level_num, my_time, my_score)"
                                      "  VALUES(%1, %2, %3, %4)")
                       .arg(setid).arg(levelNum)
                       .arg(timeScore).arg(bestScore));
        }
    }

    // TWorld does not store highest and last level separately, so store
    // the highest level into both fields
    query.exec(QStringLiteral("UPDATE levelsets SET cur_level=%1, high_level=%2"
                              "  WHERE idx=%3").arg(highLevel).arg(highLevel).arg(setid));
    refreshScores();
}

void CCPlayMain::onToolDefault()
{
    if (m_openToolButton->menu()->isEmpty())
        return;
    onTool(m_openToolButton->menu()->actions()[0]);
}

#ifdef Q_OS_WIN
#define JAVA_EXE QStringLiteral("javaw")
#else
#define JAVA_EXE QStringLiteral("java")
#endif

void CCPlayMain::onTool(QAction* action)
{
    if (!m_levelsetList->currentItem())
        return;

    QString filename = m_levelsetList->currentItem()->data(0, Qt::UserRole).toString();
    int curLevel = 1;
    if (m_levelList->currentItem())
        curLevel = m_levelList->indexOfTopLevelItem(m_levelList->currentItem()) + 1;

    QStringList launch = action->data().toString().split(QLatin1Char('|'));
    QStringList params = launch[1].split(QRegularExpression(QStringLiteral("\\s+")),
                                         QT_SKIP_EMPTY_PARTS);
    for (int i = 0; i < params.size(); ++i) {
        params[i].replace(QLatin1String("%F"), QDir::toNativeSeparators(filename))
                 .replace(QLatin1String("%L"), QString::number(curLevel));
    }
    if (launch[0].endsWith(QLatin1String(".jar"), Qt::CaseInsensitive)) {
        const QString javaExecutable = QStandardPaths::findExecutable(JAVA_EXE);
        if (javaExecutable.isEmpty() || !QFile::exists(javaExecutable)) {
            QMessageBox::critical(this, tr("Could not find Java"),
                    tr("This tool requires Java, but the '%1' executable could not be found. "
                       "Please ensure java is installed and is in your PATH.").arg(JAVA_EXE));
            return;
        }
        params = QStringList{ QStringLiteral("-jar"), launch[0] } + params;
        QProcess::execute(javaExecutable, params);
    } else {
        QProcess::execute(launch[0], params);
    }
}

void CCPlayMain::onSetup()
{
    SettingsDialog dlg;
    if (dlg.exec() == QDialog::Accepted)
        refreshTools();
}

void CCPlayMain::onRefreshLevelsets()
{
    onPathChanged(m_levelsetPath->text());
}

void CCPlayMain::onBrowseLevelsetPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select Levelset Location"),
                                                     m_levelsetPath->text());
    if (!path.isEmpty())
        m_levelsetPath->setText(path);
}

void CCPlayMain::onPathChanged(const QString& path)
{
    QDir levelsetDir(path);
    if (!levelsetDir.exists() || !m_scoredb.isOpen())
        return;

    m_levelsetList->clear();
#ifdef Q_OS_WIN
    const QStringList setExts = QStringList{
        QStringLiteral("*.dat"), QStringLiteral("*.ccl"), QStringLiteral("*.dac"),
    };
#else
    const QStringList setExts = QStringList{
        QStringLiteral("*.dat"), QStringLiteral("*.DAT"), QStringLiteral("*.Dat"),
        QStringLiteral("*.ccl"), QStringLiteral("*.CCL"), QStringLiteral("*.Ccl"),
        QStringLiteral("*.dac"), QStringLiteral("*.DAC"), QStringLiteral("*.Dac"),
    };
#endif
    QStringList setList = levelsetDir.entryList(setExts, QDir::Files,
                                                QDir::Name | QDir::IgnoreCase);
    for (const QString& set : setList) {
        QString filename = levelsetDir.absoluteFilePath(set);
        auto levelset = loadLevelset(filename);
        if (!levelset)
            continue;

        QString fileid = QDir(filename).absolutePath().section(QLatin1Char('/'), -1);
        QSqlQuery query(m_scoredb);
        query.exec(QStringLiteral("SELECT idx, cur_level, high_level FROM levelsets WHERE name='%1'")
                  .arg(fileid.replace(QLatin1String("'"), QStringLiteral("''"))));
        int curLevel = 0, highLevel = 0, totScore = 0;
        if (query.first()) {
            int setid = query.value(0).toInt();
            curLevel = query.value(1).toInt();
            highLevel = query.value(2).toInt();

            query.exec(QStringLiteral("SELECT SUM(my_score) FROM scores WHERE levelset=%1")
                       .arg(setid));
            if (query.first())
                totScore = query.value(0).toInt();
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelsetList);
        item->setText(0, set);
        item->setText(1, QString::number(levelset->levelCount()));
        item->setText(2, highLevel == 0 ? QStringLiteral("---") : QString::number(highLevel));
        item->setText(3, curLevel == 0 ? QStringLiteral("---") : QString::number(curLevel));
        item->setText(4, totScore == 0 ? QStringLiteral("---") : QString::number(totScore));
        item->setData(0, Qt::UserRole, levelsetDir.absoluteFilePath(set));
    }
}

void CCPlayMain::onLevelsetChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    m_levelList->clear();
    if (!item)
        return;

    QString filename = item->data(0, Qt::UserRole).toString();
    auto levelset = loadLevelset(filename);
    if (!levelset)
        return;

    CCX::Levelset ccx;
    bool haveCCX = false;
    QString ccxName = filename.left(filename.lastIndexOf(QLatin1Char('.'))) + QStringLiteral(".ccx");
    if (ccx.readFile(ccxName, levelset->levelCount()))
        haveCCX = true;

    QString fileid = filename.section(QLatin1Char('/'), -1);
    QSqlQuery query;
    query.exec(QStringLiteral("SELECT idx, cur_level FROM levelsets WHERE name='%1'")
               .arg(fileid.replace(QLatin1String("'"), QStringLiteral("''"))));
    int setid = 0, curLevel = 0;
    if (query.first()) {
        setid = query.value(0).toInt();
        curLevel = query.value(1).toInt() - 1;
    }

    for (int i=0; i<levelset->levelCount(); ++i) {
        int myTime = 0, myScore = 0;
        if (setid > 0) {
            query.exec(QStringLiteral("SELECT my_time, my_score FROM scores WHERE"
                                      "  levelset=%1 AND level_num=%2")
                       .arg(setid).arg(i + 1));
            if (query.first()) {
                myTime = query.value(0).toInt();
                myScore = query.value(1).toInt();
            }
        }

        ccl::LevelData* level = levelset->level(i);
        auto item = new QTreeWidgetItem(m_levelList);
        item->setText(0, QString::number(i + 1));
        item->setText(1, ccl::fromLatin1(level->name()));
        item->setText(2, haveCCX ? ccx.m_levels[i].m_author : QString());
        item->setText(3, level->timer() == 0 ? QStringLiteral("---") : QString::number(level->timer()));
        item->setText(4, myTime == 0 ? QStringLiteral("---") : QString::number(myTime));
        item->setText(5, myScore == 0 ? QStringLiteral("---") : QString::number(myScore));
    }

    if (curLevel < m_levelList->topLevelItemCount())
        m_levelList->setCurrentItem(m_levelList->topLevelItem(curLevel));
    else if (m_levelList->topLevelItemCount() > 0)
        m_levelList->setCurrentItem(m_levelList->topLevelItem(m_levelList->topLevelItemCount() - 1));
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("CCTools"));
    QApplication::setApplicationName(QStringLiteral("CCPlay"));

    QIcon appicon(QStringLiteral(":/icons/chip-48.png"));
    appicon.addFile(QStringLiteral(":/icons/chip-32.png"));
    appicon.addFile(QStringLiteral(":/icons/chip-24.png"));
    appicon.addFile(QStringLiteral(":/icons/chip-16.png"));
    QApplication::setWindowIcon(appicon);

    CCPlayMain mainWin;
    mainWin.show();
    if (!mainWin.initDatabase())
        return 1;

    QStringList qtArgs = QApplication::arguments();
    if (qtArgs.size() > 1)
        mainWin.setLevelsetPath(qtArgs[1]);

    return QApplication::exec();
}
