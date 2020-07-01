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

#ifndef _CC2_TILEWIDGETS_H
#define _CC2_TILEWIDGETS_H

#include <QListWidget>
#include "libcc2/Tileset.h"

class LayerWidget : public QFrame {
    Q_OBJECT

public:
    LayerWidget(QWidget* parent = nullptr);

    void setTileset(CC2ETileset* tileset);
    CC2ETileset* tileset() const { return m_tileset; }

    void setUpper(const cc2::Tile* tile);
    void setLower(const cc2::Tile* tile);
    const cc2::Tile* upper() const { return &m_upper; }
    const cc2::Tile* lower() const { return &m_lower; }

    virtual void paintEvent(QPaintEvent*);

    QSize sizeHint() const override
    {
        if (!m_tileset)
            return QSize();
        int size = (m_tileset->size() * 3) / 2;
        return QSize(size, size);
    }

private:
    CC2ETileset* m_tileset;
    cc2::Tile m_upper, m_lower;
};

class TileListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit TileListWidget(QWidget* parent = nullptr) : QListWidget(parent) { }

    void setTiles(std::vector<cc2::Tile> tiles);
    cc2::Tile* tile(int index);

public slots:
    void setTileImages(CC2ETileset* tileset);

protected:
    void mousePressEvent(QMouseEvent*) override;

signals:
    void tileSelectedLeft(const cc2::Tile*);
    void tileSelectedRight(const cc2::Tile*);

private:
    std::vector<cc2::Tile> m_tiles;
};

#endif
