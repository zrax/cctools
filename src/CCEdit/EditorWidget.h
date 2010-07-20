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

#ifndef _EDITORWIDGET_H
#define _EDITORWIDGET_H

#include <QFrame>
#include <../Tileset.h>
#include <../Levelset.h>

class EditorWidget : public QFrame {
    Q_OBJECT

public:
    EditorWidget(QWidget* parent = 0);

    void setTileset(CCETileset* tileset);
    CCETileset* tileset() const { return m_tileset; }

    virtual void paintEvent(QPaintEvent*);

private:
    CCETileset* m_tileset;
    ccl::LevelData* m_level;
};

#endif
