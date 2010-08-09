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
#include <QLineEdit>
#include <QTreeWidget>

class CCPlayMain : public QMainWindow {
    Q_OBJECT

public:
    CCPlayMain(QWidget* parent = 0);

    void setLevelsetPath(QString path) { m_levelsetPath->setText(path); }

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    QLineEdit* m_levelsetPath;
    QTreeWidget* m_levelsetList;
    QTreeWidget* m_levelList;

private slots:
    void onBrowseLevelsetPath();
    void onPathChanged(QString);
    void onLevelsetChanged(QTreeWidgetItem*, QTreeWidgetItem*);
};

#endif
