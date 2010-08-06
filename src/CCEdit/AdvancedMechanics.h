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

#ifndef _ADVANCEDMECHANICS_H
#define _ADVANCEDMECHANICS_H

#include <QDialog>
#include <QTreeWidget>
#include <QAction>
#include "../Levelset.h"

class AdvancedMechanicsDialog : public QDialog {
    Q_OBJECT

public:
    AdvancedMechanicsDialog(QWidget* parent = 0);
    void setFrom(ccl::LevelData* level);

private:
    enum ActionType {
        ActionAddTrap, ActionDelTrap, ActionTrapUp, ActionTrapDown,
        ActionAddClone, ActionDelClone, ActionCloneUp, ActionCloneDown,
        ActionAddMove, ActionDelMove, ActionMoveUp, ActionMoveDown,
        NUM_ACTIONS
    };

    QTreeWidget* m_trapList;
    QTreeWidget* m_cloneList;
    QTreeWidget* m_moveOrderList;
    QAction* m_actions[NUM_ACTIONS];

    std::vector<ccl::Trap> m_traps;
    std::vector<ccl::Clone> m_clones;
    std::vector<ccl::Point> m_moveOrder;
    ccl::LevelData* m_levelData;

    QTreeWidgetItem* addTrapItem(const ccl::Trap& trap);
    QTreeWidgetItem* addCloneItem(const ccl::Clone& clone);
    QTreeWidgetItem* addMoverItem(const ccl::Point& mover);

private slots:
    void onAccept();
    void onTrapSelect(QTreeWidgetItem*, QTreeWidgetItem*);
    void onCloneSelect(QTreeWidgetItem*, QTreeWidgetItem*);
    void onMoverSelect(QTreeWidgetItem*, QTreeWidgetItem*);
    void onTrapAdd();
    void onTrapDel();
    void onTrapUp();
    void onTrapDown();
    void onCloneAdd();
    void onCloneDel();
    void onCloneUp();
    void onCloneDown();
    void onMoverAdd();
    void onMoverDel();
    void onMoverUp();
    void onMoverDown();
};

#endif
