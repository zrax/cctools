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

#include "LayerWidget.h"

#include <QPaintEvent>
#include <QPainter>

LayerWidget::LayerWidget(QWidget* parent)
           : QFrame(parent), m_tileset(0), m_upper(ccl::TileWall),
             m_lower(ccl::TileFloor)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void LayerWidget::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    update();
}

void LayerWidget::setUpper(tile_t tile)
{
    m_upper = tile;
    update();
}

void LayerWidget::setLower(tile_t tile)
{
    m_lower = tile;
    update();
}

void LayerWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    if (m_tileset == 0)
        return;

    QPainter painter(this);
    int halfway = m_tileset->size() / 2;
    m_tileset->drawAt(painter, halfway, halfway, m_lower);
    m_tileset->drawAt(painter, 0, 0, m_upper);
}
