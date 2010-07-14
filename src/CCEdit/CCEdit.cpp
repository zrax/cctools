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
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <cstdio>

CCEditMain::CCEditMain(QWidget* parent)
    : QMainWindow(parent), m_levelset(0)
{
    setWindowTitle("CCEdit 3.0");
    setDockOptions(QMainWindow::AnimatedDocks);

    // Control Toolbox
    QDockWidget* toolDock = new QDockWidget(this);
    toolDock->setObjectName("ToolDock");
    QTabWidget* toolTabs = new QTabWidget(toolDock);
    toolTabs->setTabPosition(QTabWidget::West);
    toolDock->setWidget(toolTabs);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    m_levelList = new QListWidget(toolDock);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    toolDock->setFeatures(QDockWidget::DockWidgetMovable
                        | QDockWidget::DockWidgetClosable
                        | QDockWidget::DockWidgetFloatable);
    toolTabs->addTab(m_levelList, tr("Level Manager"));

    // Editor tab area
    m_editorTab = new QTabWidget(this);
    m_editorTab->setTabsClosable(true);
    m_editorTab->setMovable(true);
    setCentralWidget(m_editorTab);

    // Actions
    m_actions[ActionNew] = new QAction(QIcon(":/res/document-new.png"), tr("&New Levelset..."), this);
    m_actions[ActionOpen] = new QAction(QIcon(":/res/document-open.png"), tr("&Open Levelset..."), this);
    m_actions[ActionSave] = new QAction(QIcon(":/res/document-save.png"), tr("&Save"), this);
    m_actions[ActionSaveAs] = new QAction(QIcon(":/res/document-save-as.png"), tr("Save &As..."), this);
    m_actions[ActionExit] = new QAction(tr("E&xit"), this);

    // Main Menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_actions[ActionNew]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionOpen]);
    fileMenu->addAction(m_actions[ActionSave]);
    fileMenu->addAction(m_actions[ActionSaveAs]);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actions[ActionExit]);

    connect(m_actions[ActionExit], SIGNAL(triggered()), this, SLOT(close()));
    connect(m_actions[ActionOpen], SIGNAL(triggered()), this, SLOT(onOpenAction()));

    // Visual tweaks
    resize(640, 480);
    toolDock->resize(120, 0);
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
                        .arg(m_levelset->level(i)->name().c_str()));
        }
        m_levelsetFilename = filename;
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
    delete m_levelset;
    m_levelset = 0;
    return true;
}

void CCEditMain::onOpenAction()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Levelset..."),
                            QString(), "CC Levelsets (*.dat *.ccl)");
    if (!filename.isEmpty())
        loadLevelset(filename);
}


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    CCEditMain mainWin;
    mainWin.show();
    if (argc > 1)
        mainWin.loadLevelset(argv[1]);
    return app.exec();
}
