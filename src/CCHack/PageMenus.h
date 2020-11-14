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

#ifndef _PAGE_MENUS_H
#define _PAGE_MENUS_H

#include "HackSettings.h"

class QCheckBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;

namespace CCHack {

class PageMenus : public HackPage {
    Q_OBJECT

public:
    PageMenus(QWidget* parent);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private slots:
    void menuItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void updateMenuItem(QTreeWidgetItem* item);

private:
    QAction* m_addMenuAction;
    QAction* m_delMenuAction;

    QGroupBox* m_menuGroup;
    QTreeWidget* m_menuTree;
    QWidget* m_menuItemProps;
    QLineEdit* m_menuItemName;
    QLineEdit* m_menuItemAccelName;
    QSpinBox* m_menuItemId;
    QCheckBox* m_menuItemGrayed;
    QCheckBox* m_menuItemDisabled;
    QCheckBox* m_menuItemChecked;

    // Accelerators
    QGroupBox* m_accelGroup;
    QSpinBox* m_accelKeyCode;
    QCheckBox* m_accelVirtKey;
    QCheckBox* m_accelNoHighlight;
    QCheckBox* m_accelShift, * m_accelCtrl, * m_accelAlt;

    // For the cheat menu
    QCheckBox* m_cbIgnorePasswords;
    QLineEdit* m_ignorePasswords, * m_defIgnorePasswords;
};

}

#endif
