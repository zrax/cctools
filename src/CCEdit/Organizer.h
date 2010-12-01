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

#ifndef _ORGANIZER_H
#define _ORGANIZER_H

#include <QDialog>
#include <QListWidget>
#include "../Levelset.h"
#include "../Tileset.h"

class LevelListWidget : public QListWidget {
    Q_OBJECT

public:
    LevelListWidget(QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent*);

signals:
    void loadLevelImage(int level);
};


class OrganizerDialog : public QDialog {
    Q_OBJECT

public:
    OrganizerDialog(QWidget* parent = 0);

    void loadLevelset(ccl::Levelset* levelset);
    void setTileset(CCETileset* tileset) { m_tileset = tileset; }

private slots:
    void loadLevelImage(int level);

private:
    ccl::Levelset* m_levelset;
    CCETileset* m_tileset;
    LevelListWidget* m_levels;
};

#endif
