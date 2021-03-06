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

#ifndef _EDITORTABWIDGET_H
#define _EDITORTABWIDGET_H

#include <QTabWidget>
#include <QTabBar>

class EditorTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit EditorTabWidget(QWidget* parent = nullptr);

    void addFloatingTab(QWidget* widget, const QString& label);
    void promoteTab() { promoteTab(currentIndex()); }
    void promoteTab(int index);

    // Shadows QTabWidget::setTabText()
    void setTabText(int index, const QString& label);
};

#endif
