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

#include "EditorWidget.h"

#include <QPaintEvent>
#include <QMouseEvent>


CC2EditorWidget::CC2EditorWidget(QWidget* parent)
    : QWidget(parent), m_tileset(), m_map(), m_zoomFactor(1.0)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void CC2EditorWidget::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    dirtyBuffer();
    resize(sizeHint());
    update();
}

void CC2EditorWidget::setMap(cc2::Map* map)
{
    map->ref();
    if (m_map)
        m_map->unref();
    m_map = map;

    m_tileBuffer = QPixmap(m_map->mapData().width() * m_tileset->size(),
                           m_map->mapData().height() * m_tileset->size());

    dirtyBuffer();
    update();

    emit canUndo(false);
    emit canRedo(false);
    emit hasSelection(false);
}

void CC2EditorWidget::renderTileBuffer()
{
    if (!m_map)
        return;

    QPainter tilePainter(&m_tileBuffer);
    const cc2::MapData& mapData = m_map->mapData();
    for (int y = 0; y < mapData.height(); ++y) {
        for (int x = 0; x < mapData.width(); ++x)
            m_tileset->draw(tilePainter, x, y, mapData.tile(x, y));
    }

    dirtyBuffer();
}

void CC2EditorWidget::paintEvent(QPaintEvent*)
{
    if (!m_tileset || !m_map)
        return;

    QPainter painter(this);
    renderTo(painter);
}

void CC2EditorWidget::renderTo(QPainter& painter)
{
    if (m_cacheDirty) {
        renderTileBuffer();
        m_tileCache = m_tileBuffer.scaled(renderSize());
        m_cacheDirty = false;
    }
    painter.drawPixmap(0, 0, m_tileCache);

    // Highlight context-sensitive objects
    painter.setPen(QColor(255, 0, 0));
    foreach (QPoint hi, m_hilights)
        painter.drawRect(calcTileRect(hi.x(), hi.y()));
}

void CC2EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;

    int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    if (m_current == QPoint(posX, posY) && !m_cacheDirty)
        return;
    m_current = QPoint(posX, posY);

    cc2::Tile* tile = m_map->mapData().tile(posX, posY);
    QString info = QString("(%1, %2): %3").arg(posX).arg(posY).arg(CC2ETileset::getName(tile));
    while (tile->haveLower() && tile->lower()->type() != cc2::Tile::Floor) {
        tile = tile->lower();
        info += tr(" / %1").arg(CC2ETileset::getName(tile));
    }
    emit mouseInfo(info);

    update();
}

void CC2EditorWidget::mousePressEvent(QMouseEvent* event)
{
    mouseMoveEvent(event);
}

void CC2EditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    update();
}

void CC2EditorWidget::setZoom(double factor)
{
    m_zoomFactor = factor;
    dirtyBuffer();
    resize(sizeHint());
    update();
}
