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

#ifndef _TILEWIDGETS_H
#define _TILEWIDGETS_H

#include <QListWidget>
#include "libcc1/Tileset.h"

class TileListWidget : public QListWidget {
    Q_OBJECT

public:
    TileListWidget(QWidget* parent = 0) : QListWidget(parent) { }
    void addTiles(const QList<tile_t>& tiles);

protected:
    virtual void mousePressEvent(QMouseEvent*);

signals:
    void itemSelectedLeft(tile_t);
    void itemSelectedRight(tile_t);
};


class BigTileWiget : public QWidget {
    Q_OBJECT

public:
    BigTileWiget(QWidget* parent = 0);
    void setTileset(CCETileset* tileset);

    virtual QSize sizeHint() const
    {
        int tsetSize = (m_tileset != 0) ? m_tileset->size() : 32;
        return QSize(tsetSize * 7, tsetSize * 16);
    }

private:
    CCETileset* m_tileset;

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);

signals:
    void itemSelectedLeft(tile_t);
    void itemSelectedRight(tile_t);
};

#endif
