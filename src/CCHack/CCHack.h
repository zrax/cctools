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
#include "HackSettings.h"

class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;

class CCHackMain : public QMainWindow {
    Q_OBJECT

public:
    CCHackMain(QWidget* parent = nullptr);

    void loadFile(const QString& filename);

private slots:
    void onChangePage(QTreeWidgetItem* page, QTreeWidgetItem*);

private:
    QStackedWidget* m_container;

    HackSettings m_defaults, m_settings;

    enum PageType {
        PageNothing, PageGeneral, PageSound,
        PageMenus, PageStory, PageEndLevel, PageEndGame, PageMisc,
        PageVGATS, PageEGATS, PageMonoTS, PageBackground, PageEndGfx, PageDigits,
        Pages_COUNT
    };
    HackPage* m_pages[Pages_COUNT];
};

#endif
