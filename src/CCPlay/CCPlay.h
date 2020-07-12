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

#ifndef _CCPLAY_H
#define _CCPLAY_H

#include <QMainWindow>
#include <QAction>
#include <QLineEdit>
#include <QTreeWidget>
#include <QToolButton>
#include <QSqlDatabase>

class CCPlayMain : public QMainWindow {
    Q_OBJECT

public:
    explicit CCPlayMain(QWidget* parent = nullptr);

    bool initDatabase();
    void setLevelsetPath(const QString& path) { m_levelsetPath->setText(path); }

protected:
    void closeEvent(QCloseEvent*) override;

private:
    enum {
        ActionPlayMSCC, ActionPlayTWorld, ActionPlayTWorld2,
        ActionTool, ActionSetup, ActionExit,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QToolButton* m_playButton;
    QToolButton* m_openToolButton;

    QLineEdit* m_levelsetPath;
    QTreeWidget* m_levelsetList;
    QTreeWidget* m_levelList;
    QSqlDatabase m_scoredb;

    void refreshTools();
    void refreshScores();

private slots:
    void onPlayMSCC();
    void onPlayTWorld(bool tworld2);
    void onToolDefault();
    void onTool(QAction* action);
    void onSetup();

    void onRefreshLevelsets();
    void onBrowseLevelsetPath();
    void onPathChanged(const QString&);
    void onLevelsetChanged(QTreeWidgetItem*, QTreeWidgetItem*);
};

#endif
