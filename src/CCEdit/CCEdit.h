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

#ifndef _CCEDIT_H
#define _CCEDIT_H

#include <QMainWindow>
#include <QAction>
#include <QListWidget>
#include <QTabWidget>
#include "../Levelset.h"

class CCEditMain : public QMainWindow {
    Q_OBJECT

public:
    CCEditMain(QWidget* parent = 0);

    void loadLevelset(QString filename);
    bool closeLevelset();

private:
    enum ActionType {
        ActionNew, ActionOpen, ActionSave, ActionSaveAs, ActionExit,
        NUM_ACTIONS
    };

    QAction* m_actions[NUM_ACTIONS];
    QListWidget* m_levelList;
    QTabWidget* m_editorTab;

    ccl::Levelset* m_levelset;
    QString m_levelsetFilename;

private slots:
    void onOpenAction();
};

#endif
