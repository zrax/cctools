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
#include <QLabel>
#include <QGridLayout>
#include <QDirModel>
#include <QCompleter>
#include <QMessageBox>
#include <QFileDialog>
#include "../Levelset.h"

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

    // Form layout
    QGridLayout* layout = new QGridLayout(contents);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    layout->addWidget(lblLevelsetPath, 0, 0);
    layout->addWidget(m_levelsetPath, 0, 1);
    layout->addWidget(btnOpenPath, 0, 2);
    layout->addWidget(splitLevelsetData, 1, 0, 1, 3);
    setCentralWidget(contents);
    statusBar();

    connect(btnOpenPath, SIGNAL(clicked()), SLOT(onBrowseLevelsetPath()));
    connect(m_levelsetPath, SIGNAL(textChanged(QString)), SLOT(onPathChanged(QString)));
    connect(m_levelsetList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(onLevelsetChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    QSettings settings("CCTools", "CCPlay");
    resize(settings.value("WindowSize", QSize(480, 400)).toSize());
    if (settings.value("WindowMaximized", false).toBool())
        showMaximized();
    if (settings.contains("WindowState"))
        restoreState(settings.value("WindowState").toByteArray());
    m_levelsetPath->setText(settings.value("LevelsetPath", QDir::currentPath()).toString());
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
    if (!levelsetDir.exists())
        return;

    m_levelsetList->clear();
    QStringList setList = levelsetDir.entryList(QStringList() << "*.dat" << "*.ccl" << "*.dac",
                                                QDir::Files, QDir::Name);
    foreach (QString set, setList) {
        ccl::Levelset levelset;
        ccl::FileStream stream;
        try {
            if (!stream.open(levelsetDir.absoluteFilePath(set).toUtf8().data(), "rb"))
                continue;
            levelset.read(&stream);
            stream.close();
        } catch (...) {
            continue;
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelsetList);
        item->setText(0, set);
        item->setText(1, QString("%1").arg(levelset.levelCount()));
        //item->setText(2, QString("%1").arg(149));
        item->setText(2, "---");
        //item->setText(3, QString("%1").arg(149));
        item->setText(3, "---");
        //item->setText(4, QString("%1").arg(6000000));
        item->setText(4, "---");
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

    for (int i=0; i<levelset.levelCount(); ++i) {
        ccl::LevelData* level = levelset.level(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_levelList);
        item->setText(0, QString("%1").arg(level->levelNum()));
        item->setText(1, level->name().c_str());
        item->setText(2, level->chips() == 0 ? "---" : QString("%1").arg(level->chips()));
        item->setText(3, level->timer() == 0 ? "---" : QString("%1").arg(level->timer()));
        //item->setText(3, QString("%1").arg(149));
        item->setText(4, "---");
        //item->setText(4, QString("%1").arg(6000000));
        item->setText(5, "---");
    }
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CCPlayMain mainWin;
    mainWin.show();
    if (argc > 1)
        mainWin.setLevelsetPath(argv[1]);
    return app.exec();
}
