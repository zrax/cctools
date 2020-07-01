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

class BigTileWidget : public QWidget {
    Q_OBJECT

public:
    explicit BigTileWidget(QWidget* parent = nullptr);

    void setTileset(CC2ETileset* tileset);
    CC2ETileset* tileset() const { return m_tileset; }

    enum ViewType { ViewTiles, ViewGlyphs };
    void setView(ViewType type);

    QSize sizeHint() const override
    {
        int tsetSize = m_tileset ? m_tileset->size() : 32;
        return QSize(tsetSize * 9, tsetSize * 15);
    }

signals:
    void tileSelectedLeft(const cc2::Tile*);
    void tileSelectedRight(const cc2::Tile*);

public slots:
    void rotateLeft();
    void rotateRight();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    CC2ETileset* m_tileset;
    std::vector<cc2::Tile> m_tiles;
    std::vector<cc2::Tile> m_glyphs;
    ViewType m_view;

    std::vector<cc2::Tile>& tileList()
    {
        if (m_view == ViewGlyphs)
            return m_glyphs;
        return m_tiles;
    }
};

#endif
