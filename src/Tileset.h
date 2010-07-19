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

#ifndef _TILESET_H
#define _TILESET_H

#include <QPixmap>

typedef unsigned char tile_t;

class CCETileset {
public:
    CCETileset() { }

    QString name() const { return m_name; }
    QString description() const { return m_description; }
    int size() const { return m_size; }

    void load(QString filename);
    void draw(QPainter painter, int x, int y, tile_t upper, tile_t lower = 0) const;

private:
    QString m_name;
    QString m_description;
    int m_size;

    QPixmap m_base;
    QPixmap m_overlay;
};

#endif
