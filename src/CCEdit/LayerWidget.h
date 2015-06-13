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

#ifndef _LAYERWIDGET_H
#define _LAYERWIDGET_H

#include <QFrame>
#include "libcc1/Tileset.h"

class LayerWidget : public QFrame {
    Q_OBJECT

public:
    LayerWidget(QWidget* parent = 0);

    void setTileset(CCETileset* tileset);
    CCETileset* tileset() const { return m_tileset; }

    void setUpper(tile_t tile);
    void setLower(tile_t tile);
    tile_t upper() const { return m_upper; }
    tile_t lower() const { return m_lower; }

    virtual void paintEvent(QPaintEvent*);

    virtual QSize sizeHint() const
    {
        if (m_tileset == 0)
            return QSize();
        int size = (m_tileset->size() * 3) / 2;
        return QSize(size, size);
    }

private:
    CCETileset* m_tileset;
    tile_t m_upper, m_lower;
};

#endif
