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

#include <QMouseEvent>
#include <QPainter>

Q_DECLARE_METATYPE(cc2::Tile*)

LayerWidget::LayerWidget(QWidget* parent)
    : QFrame(parent), m_tileset()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void LayerWidget::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    updateGeometry();
    update();
}

void LayerWidget::setUpper(const cc2::Tile* tile)
{
    m_upper = *tile;
    update();
}

void LayerWidget::setLower(const cc2::Tile* tile)
{
    m_lower = *tile;
    update();
}

void LayerWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    if (!m_tileset)
        return;

    QPainter painter(this);
    const int halfway = m_tileset->size() / 2;
    m_tileset->drawAt(painter, halfway, halfway, &m_lower, false);
    m_tileset->drawAt(painter, 0, 0, &m_upper, false);
}


void TileListWidget::setTiles(std::vector<cc2::Tile> tiles)
{
    m_tiles = std::move(tiles);
    for (cc2::Tile& tile : m_tiles) {
        auto item = new QListWidgetItem(CC2ETileset::getName(&tile), this);
        item->setData(Qt::UserRole, QVariant::fromValue(&tile));
    }
}

cc2::Tile* TileListWidget::tile(int index)
{
    auto tileItem = item(index);
    if (tileItem)
        return tileItem->data(Qt::UserRole).value<cc2::Tile*>();
    return nullptr;
}

void TileListWidget::setTileImages(CC2ETileset* tileset)
{
    setIconSize(tileset->qsize());
    for (int i = 0; i < count(); ++i)
        item(i)->setIcon(tileset->getIcon(tile(i)));
}

void TileListWidget::mousePressEvent(QMouseEvent* event)
{
    QAbstractItemView::mousePressEvent(event);
    if (currentItem() == nullptr)
        return;

    if (event->button() == Qt::LeftButton)
        emit tileSelectedLeft(currentItem()->data(Qt::UserRole).value<cc2::Tile*>());
    else if (event->button() == Qt::RightButton)
        emit tileSelectedRight(currentItem()->data(Qt::UserRole).value<cc2::Tile*>());
    setCurrentItem(nullptr);
}
