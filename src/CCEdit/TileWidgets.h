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
    explicit TileListWidget(QWidget* parent = nullptr) : QListWidget(parent) { }
    void addTiles(const QVector<tile_t>& tiles);

public slots:
    void setTileImages(CCETileset* tileset);

protected:
    void mousePressEvent(QMouseEvent*) override;

signals:
    void itemSelectedLeft(tile_t);
    void itemSelectedRight(tile_t);
};


class BigTileWiget : public QWidget {
    Q_OBJECT

public:
    explicit BigTileWiget(QWidget* parent = nullptr);
    void setTileset(CCETileset* tileset);

    QSize sizeHint() const override
    {
        int tsetSize = (m_tileset != 0) ? m_tileset->size() : 32;
        return QSize(tsetSize * 7, tsetSize * 16);
    }

private:
    CCETileset* m_tileset;

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

signals:
    void itemSelectedLeft(tile_t);
    void itemSelectedRight(tile_t);
};

#endif
