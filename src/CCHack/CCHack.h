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

#ifndef _CCHACK_H
#define _CCHACK_H

#include <QMainWindow>
#include <QTreeWidget>
#include "HackSettings.h"

class CCHackMain : public QMainWindow {
    Q_OBJECT

public:
    CCHackMain(QWidget* parent = 0);

    void loadFile(const QString& filename);

private slots:
    void onChangePage(QTreeWidgetItem* page, QTreeWidgetItem*);

private:
    QWidget* m_container;
    HackPage* m_page;

    HackSettings m_defaults, m_settings;
};

#endif
