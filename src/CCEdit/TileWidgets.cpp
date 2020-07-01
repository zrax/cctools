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

#include "TileWidgets.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>

void TileListWidget::addTiles(const QVector<tile_t>& tiles)
{
    for (tile_t tile : tiles) {
        QListWidgetItem* item = new QListWidgetItem(CCETileset::TileName(tile), this);
        item->setData(Qt::UserRole, (int)tile);
    }
}

void TileListWidget::setTileImages(CCETileset* tileset)
{
    setIconSize(tileset->qsize());
    for (int i = 0; i < count(); ++i)
        item(i)->setIcon(tileset->getIcon(item(i)->data(Qt::UserRole).toInt()));
}

void TileListWidget::mousePressEvent(QMouseEvent* event)
{
    QAbstractItemView::mousePressEvent(event);
    if (currentItem() == 0)
        return;

    if (event->button() == Qt::LeftButton)
        emit itemSelectedLeft((tile_t)currentItem()->data(Qt::UserRole).toUInt());
    else if (event->button() == Qt::RightButton)
        emit itemSelectedRight((tile_t)currentItem()->data(Qt::UserRole).toUInt());
    setCurrentItem(0);
}


BigTileWidget::BigTileWidget(QWidget* parent)
    : QWidget(parent), m_tileset()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void BigTileWidget::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    update();
}

void BigTileWidget::paintEvent(QPaintEvent*)
{
    if (m_tileset == 0)
        return;

    QPainter painter(this);
    for (int y=0; y<16; ++y) {
        for (int x=0; x<7; ++x) {
            m_tileset->drawAt(painter, x * m_tileset->size(), y * m_tileset->size(),
                              (tile_t)((x * 16) + y));
        }
    }
}

void BigTileWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_tileset || event->x() >= (m_tileset->size() * 7)
            || event->y() >= (m_tileset->size() * 16)) {
        return;
    }

    tile_t tileid = ((event->x() / m_tileset->size()) * 16)
                  + (event->y() / m_tileset->size());
    if (event->button() == Qt::LeftButton)
        emit itemSelectedLeft(tileid);
    else if (event->button() == Qt::RightButton)
        emit itemSelectedRight(tileid);
}

void BigTileWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if (!m_tileset || event->x() >= (m_tileset->size() * 7)
            || event->y() >= (m_tileset->size() * 16)) {
        setToolTip(QString());
        return;
    }

    tile_t tileid = ((event->x() / m_tileset->size()) * 16)
                  + (event->y() / m_tileset->size());
    setToolTip(CCETileset::TileName(tileid));
}
