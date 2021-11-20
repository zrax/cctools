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
#include "CommonWidgets/CCTools.h"
#include "libcc2/GameLogic.h"

#include <QUndoStack>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <queue>

static CC2EditorWidget::CombineMode select_cmode(Qt::KeyboardModifiers keys)
{
    if ((keys & Qt::ShiftModifier) != 0)
        return CC2EditorWidget::CombineForce;
    else if ((keys & Qt::ControlModifier) != 0)
        return CC2EditorWidget::Replace;
    return CC2EditorWidget::CombineSmart;
}

static void plot_box(CC2EditorWidget* self, QPoint from, QPoint to,
                     const cc2::Tile& drawTile, CC2EditorWidget::CombineMode mode)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = std::min(from.y(), to.y());
    int lowX = std::min(from.x(), to.x());
    int highY = std::max(from.y(), to.y());
    int highX = std::max(from.x(), to.x());

    for (int y = lowY; y <= highY; ++y)
        for (int x = lowX; x <= highX; ++x)
            self->putTile(drawTile, x, y, mode);
}

static void plot_rect(CC2EditorWidget* self, QPoint from, QPoint to,
                      const cc2::Tile& drawTile, CC2EditorWidget::CombineMode mode)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = std::min(from.y(), to.y());
    int lowX = std::min(from.x(), to.x());
    int highY = std::max(from.y(), to.y());
    int highX = std::max(from.x(), to.x());

    for (int x = lowX; x <= highX; ++x) {
        self->putTile(drawTile, x, lowY, mode);
        self->putTile(drawTile, x, highY, mode);
    }
    for (int y = lowY + 1; y <= highY - 1; ++y) {
        self->putTile(drawTile, lowX, y, mode);
        self->putTile(drawTile, highX, y, mode);
    }
}

static void plot_line(CC2EditorWidget* self, QPoint from, QPoint to,
                      const cc2::Tile& drawTile, CC2EditorWidget::CombineMode mode)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = from.y();
    int lowX = from.x();
    int highY = to.y();
    int highX = to.x();
    bool steep = abs(highY - lowY) > abs(highX - lowX);
    if (steep) {
        std::swap(lowX, lowY);
        std::swap(highX, highY);
    }
    if (lowX > highX) {
        std::swap(lowX, highX);
        std::swap(lowY, highY);
    }

    int dX = highX - lowX;
    int dY = abs(highY - lowY);
    int err = dX / 2;
    int ystep = (lowY < highY) ? 1 : -1;
    int y = lowY;
    for (int x = lowX; x <= highX; ++x) {
        self->putTile(drawTile, steep ? y : x, steep ? x : y, mode);
        err -= dY;
        if (err < 0) {
            y += ystep;
            err += dX;
        }
    }
}

static void plot_flood(CC2EditorWidget* self, QPoint start,
                       const cc2::Tile& drawTile, CC2EditorWidget::CombineMode mode)
{
    cc2::MapData& map = self->map()->mapData();
    const cc2::Tile replaceTile = map.tile(start.x(), start.y());

    std::queue<QPoint> floodQueue;
    floodQueue.push(start);
    self->putTile(drawTile, start.x(), start.y(), mode);
    if (map.tile(start.x(), start.y()) == replaceTile) {
        // No change was made.  Exit to avoid an infinite loop
        return;
    }

    while (!floodQueue.empty()) {
        QPoint pt = floodQueue.front();
        floodQueue.pop();

        if (pt.x() > 0 && map.tile(pt.x() - 1, pt.y()) == replaceTile) {
            QPoint next(pt.x() - 1, pt.y());
            self->putTile(drawTile, next.x(), next.y(), mode);
            floodQueue.push(next);
        }
        if (pt.x() < map.width() - 1 && map.tile(pt.x() + 1, pt.y()) == replaceTile) {
            QPoint next(pt.x() + 1, pt.y());
            self->putTile(drawTile, next.x(), next.y(), mode);
            floodQueue.push(next);
        }
        if (pt.y() > 0 && map.tile(pt.x(), pt.y() - 1) == replaceTile) {
            QPoint next(pt.x(), pt.y() - 1);
            self->putTile(drawTile, next.x(), next.y(), mode);
            floodQueue.push(next);
        }
        if (pt.y() < map.height() - 1 && map.tile(pt.x(), pt.y() + 1) == replaceTile) {
            QPoint next(pt.x(), pt.y() + 1);
            self->putTile(drawTile, next.x(), next.y(), mode);
            floodQueue.push(next);
        }
    }
}

static cc2::Tile::Direction calc_dir(const QPoint& from, const QPoint& to)
{
    int dX = to.x() - from.x();
    int dY = to.y() - from.y();
    if (dX < 0)
        return cc2::Tile::West;
    else if (dX > 0)
        return cc2::Tile::East;
    else if (dY < 0)
        return cc2::Tile::North;
    else if (dY > 0)
        return cc2::Tile::South;
    return cc2::Tile::InvalidDir;
}

static cc2::Tile::Direction rot_180(cc2::Tile::Direction dir)
{
    if (dir == cc2::Tile::InvalidDir)
        return dir;
    return cc2::Tile::Direction((int(dir) + 2) % 4);
}

static cc2::Tile directionalize(const cc2::Tile& tile, cc2::Tile::Direction dir)
{
    if (dir == cc2::Tile::InvalidDir) {
        return tile;
    } else if (tile.type() >= cc2::Tile::Force_N && tile.type() <= cc2::Tile::Force_W) {
        return cc2::Tile(cc2::Tile::Force_N + int(dir));
    } else if (tile.type() == cc2::Tile::TrainTracks
               && (tile.modifier() & cc2::TileModifier::TrackDir_MASK) != 0) {
        // Assume we want to exit straight.  The end can be cleaned up by
        // the user if necessary...
        cc2::Tile dirTile(tile);
        if (dir == cc2::Tile::North || dir == cc2::Tile::South)
            dirTile.setModifier(cc2::TileModifier::Track_NS);
        else if (dir == cc2::Tile::West || dir == cc2::Tile::East)
            dirTile.setModifier(cc2::TileModifier::Track_WE);
        return dirTile;
    } else if (tile.haveDirection()) {
        cc2::Tile dirTile(tile);
        dirTile.setDirection(dir);
        return dirTile;
    }
    return tile;
}


CC2EditorWidget::CC2EditorWidget(QWidget* parent)
    : QWidget(parent), m_tileset(), m_map(), m_drawMode(DrawPencil),
      m_paintFlags(), m_cachedButton(Qt::NoButton), m_lastDir(cc2::Tile::InvalidDir),
      m_undoCommand(), m_zoomFactor(1.0)
{
    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &CC2EditorWidget::canUndoChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &CC2EditorWidget::canRedoChanged);
    connect(m_undoStack, &QUndoStack::cleanChanged, this, &CC2EditorWidget::cleanChanged);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);

    m_selectRect = QRect(-1, -1, -1, -1);
    m_editCache = new cc2::Map;
}

void CC2EditorWidget::setTileset(CC2ETileset* tileset)
{
    m_tileset = tileset;
    const QSize size = mapSize();
    m_tileBuffer = QPixmap(size.width() * m_tileset->size(), size.height() * m_tileset->size());
    resize(sizeHint());
    dirtyBuffer();
}

void CC2EditorWidget::setMap(cc2::Map* map)
{
    map->ref();
    if (m_map)
        m_map->unref();
    m_map = map;

    m_tileBuffer = QPixmap(m_map->mapData().width() * m_tileset->size(),
                           m_map->mapData().height() * m_tileset->size());
    resize(sizeHint());

    m_undoStack->clear();
    dirtyBuffer();

    m_selectRect = QRect(-1, -1, -1, -1);
    emit hasSelection(false);
}

void CC2EditorWidget::resizeMap(const QSize& newSize)
{
    beginEdit(CC2EditHistory::EditResizeMap);
    m_map->mapData().resize(newSize.width(), newSize.height());
    endEdit();

    m_tileBuffer = QPixmap(newSize.width() * m_tileset->size(),
                           newSize.height() * m_tileset->size());
    resize(sizeHint());
}

void CC2EditorWidget::setDrawMode(DrawMode mode)
{
    if (m_drawMode != mode) {
        m_drawMode = mode;
        m_origin = QPoint(-1, -1);
        m_selectRect = QRect(-1, -1, -1, -1);
        update();
        emit hasSelection(false);

        switch (m_drawMode) {
        case DrawWires:
            setCursor(QCursor(QPixmap(QStringLiteral(":/res/cur-wire.png")), 4, 4));
            break;
        case DrawInspectHint:
            setCursor(QCursor(QPixmap(QStringLiteral(":/res/cur-hint.png")), 4, 4));
            break;
        case DrawInspectTile:
            setCursor(QCursor(QPixmap(QStringLiteral(":/res/cur-inspect.png")), 4, 4));
            break;
        default:
            setCursor(Qt::ArrowCursor);
            break;
        }
    }
}

void CC2EditorWidget::beginEdit(CC2EditHistory::Type type)
{
    if (m_undoCommand)
        m_undoCommand->enter();
    else
        m_undoCommand = new MapUndoCommand(type, m_map);
}

void CC2EditorWidget::endEdit()
{
    if (m_undoCommand->leave(m_map)) {
        const int editType = m_undoCommand->id();
        m_undoStack->push(m_undoCommand);
        if (editType == CC2EditHistory::EditMap)
            emit updateCounters();
        m_undoCommand = nullptr;
    }
    dirtyBuffer();
}

void CC2EditorWidget::cancelEdit()
{
    if (m_undoCommand->leave(nullptr)) {
        delete m_undoCommand;
        m_undoCommand = nullptr;
    }
}

void CC2EditorWidget::setClean()
{
    m_undoStack->setClean();
}

void CC2EditorWidget::resetClean()
{
    m_undoStack->resetClean();
}

void CC2EditorWidget::renderTileBuffer()
{
    if (!m_map)
        return;

    QPainter tilePainter(&m_tileBuffer);
    const cc2::MapData& mapData = m_map->mapData();
    for (int y = 0; y < mapData.height(); ++y) {
        for (int x = 0; x < mapData.width(); ++x)
            m_tileset->draw(tilePainter, x, y, &mapData.tile(x, y), true);
    }
}

void CC2EditorWidget::paintEvent(QPaintEvent*)
{
    if (!m_tileset || !m_map)
        return;

    QPainter painter(this);
    renderTo(painter);
}

static const cc2::Tile* findCreature(const cc2::Tile* tile)
{
    do {
        if (tile->isCreature())
            return tile;
        tile = tile->lower();
    } while (tile);

    return nullptr;
}

void CC2EditorWidget::renderTo(QPainter& painter)
{
    if (m_cacheDirty) {
        renderTileBuffer();
        m_tileCache = m_tileBuffer.scaled(renderSize());
        m_cacheDirty = false;
    }
    painter.drawPixmap(0, 0, m_tileCache);

    if (m_selectRect != QRect(-1, -1, -1, -1)) {
        QRect selectionArea = calcTileRect(m_selectRect);
        painter.fillRect(selectionArea, QBrush(QColor(95, 95, 191, 127)));
        painter.setPen(QColor(63, 63, 191));
        painter.drawRect(selectionArea);
    }

    const cc2::MapData& mapData = m_map->mapData();
    if ((m_paintFlags & ShowMovePaths) != 0) {
        painter.setPen(QColor(0, 127, 255));
        std::vector<uint8_t> looked;
        looked.resize(mapData.width() * mapData.height());

        for (int y = 0; y < mapData.height(); ++y) {
            for (int x = 0; x < mapData.width(); ++x) {
                const cc2::Tile* tile = &mapData.tile(x, y);
                while (tile && (tile = findCreature(tile)) != nullptr) {
                    std::fill(looked.begin(), looked.end(), 0);
                    cc2::MoveState move = cc2::CheckMove(mapData, tile, x, y);

                    cc2::Tile tmpCre(*tile);
                    QPoint from(x, y);
                    do {
                        looked[(from.y() * mapData.width()) + from.x()] |= 1 << (int)tmpCre.direction();
                        if ((move & cc2::MoveDirMask) < cc2::MoveBlocked) {
                            if ((move & cc2::MoveTrapped) != 0)
                                break;

                            QPoint to = cc2::AdvanceCreature(from, move);
                            painter.drawLine(calcPathCenter(from.x(), from.y()),
                                             calcPathCenter(to.x(), to.y()));
                            if ((move & cc2::MoveDeath) != 0)
                                break;
                            if ((move & cc2::MoveTeleport) != 0) {
                                //TODO
                                break;
                            }
                            from = to;
                            cc2::TurnCreature(&tmpCre, move);
                        } else {
                            break;
                        }
                        move = cc2::CheckMove(mapData, &tmpCre, from.x(), from.y());
                    } while ((looked[(from.y() * mapData.width()) + from.x()]
                               & (1 << (int)tmpCre.direction())) == 0);

                    tile = tile->lower();
                }
            }
        }
    }

    if ((m_paintFlags & ShowViewBox) != 0) {
        painter.setPen(QColor(0, 255, 127));
        QRect tileRect;
        if (m_map->option().view() == cc2::MapOption::View9x9) {
            tileRect = calcTileRect(m_current.x() - 4, m_current.y() - 4, 9, 9);
        } else {
            tileRect = calcTileRect(m_current.x() - 4, m_current.y() - 4, 10, 10);
            tileRect.translate(-((m_tileset->size() / 2) * m_zoomFactor),
                               -((m_tileset->size() / 2) * m_zoomFactor));
        }
        if (tileRect.left() < 0)
            tileRect.moveLeft(0);
        if (tileRect.top() < 0)
            tileRect.moveTop(0);
        if (tileRect.right() > m_tileCache.width() - 2)
            tileRect.moveRight(m_tileCache.width() - 2);
        if (tileRect.bottom() > m_tileCache.height() - 2)
            tileRect.moveBottom(m_tileCache.height() - 2);
        painter.drawRect(tileRect);
    }

    // Highlight context-sensitive objects
    painter.setPen(QColor(255, 0, 0));
    for (const QPoint& hi : m_hilights)
        painter.drawRect(calcTileRect(hi.x(), hi.y()));
}

static QPoint findPlayer(const cc2::MapData& mapData)
{
    for (int y = 0; y < mapData.height(); ++y) {
        for (int x = 0; x < mapData.width(); ++x) {
            if (mapData.tile(x, y).haveTile({cc2::Tile::Player, cc2::Tile::Player2}))
                return {x, y};
        }
    }
    return {0, 0};
}

/* This is only used to generate special reports, so no old draw/render state
 * data needs to be saved.
 */
QImage CC2EditorWidget::renderReport()
{
    m_paintFlags = ShowAll;
    m_zoomFactor = 1.0;
    m_cacheDirty = true;
    m_current = findPlayer(m_map->mapData());

    QImage output(m_tileset->size() * m_map->mapData().width(),
                  m_tileset->size() * m_map->mapData().height(),
                  QImage::Format_RGB32);
    QPainter painter(&output);
    renderTo(painter);
    return output;
}

QImage CC2EditorWidget::renderSelection()
{
    if (m_selectRect == QRect(-1, -1, -1, -1))
        return QImage();

    QImage output(m_tileset->size() * m_selectRect.width(),
                  m_tileset->size() * m_selectRect.height(),
                  QImage::Format_RGB32);
    QPainter painter(&output);
    painter.drawPixmap(0, 0, m_tileBuffer,
                       m_selectRect.x() * m_tileset->size(),
                       m_selectRect.y() * m_tileset->size(),
                       m_selectRect.width() * m_tileset->size(),
                       m_selectRect.height() * m_tileset->size());
    return output;
}

static QPoint scanForR(cc2::Tile::Type type, int x, int y, const cc2::MapData& map)
{
    int sx = x, sy = y;
    do {
        if (--sx < 0) {
            sx = map.width() - 1;
            if (--sy < 0)
                sy = map.height() - 1;
        }
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return QPoint(-1, -1);
        }
    } while (!map.tile(sx, sy).haveTile(type));

    return QPoint(sx, sy);
}

static QPoint scanForF(cc2::Tile::Type type, int x, int y, const cc2::MapData& map)
{
    int sx = x, sy = y;
    do {
        if (++sx >= map.width()) {
            sx = 0;
            if (++sy >= map.height())
                sy = 0;
        }
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return QPoint(-1, -1);
        }
    } while (!map.tile(sx, sy).haveTile(type));

    return QPoint(sx, sy);
}

static QList<QPoint> scanForAll(cc2::Tile::Type type, const cc2::MapData& map)
{
    QList<QPoint> matches;
    for (int sy = 0; sy < map.height(); ++sy) {
        for (int sx = 0; sx < map.width(); ++sx) {
            if (map.tile(sx, sy).haveTile(type))
                matches << QPoint(sx, sy);
        }
    }
    return matches;
}

static QPoint scanForControl(const std::vector<cc2::Tile::Type>& controlTypes,
                             int x, int y, const cc2::MapData& map)
{
    int sx = x, sy = y;
    for ( ;; ) {
        if (++sx >= map.width()) {
            sx = 0;
            if (++sy >= map.height())
                sy = 0;
        }
        if (map.tile(sx, sy).haveTile(controlTypes))
            return QPoint(sx, sy);
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return QPoint(-1, -1);
        }
    }

    Q_UNREACHABLE();
}

static QList<QPoint> scanForButtons(cc2::Tile::Type buttonType,
                                    const std::vector<cc2::Tile::Type>& controlTypes,
                                    int x, int y, const cc2::MapData& map)
{
    QList<QPoint> matches;

    int sx = x, sy = y;
    for ( ;; ) {
        if (--sx < 0) {
            sx = map.width() - 1;
            if (--sy < 0)
                sy = map.height() - 1;
        }
        if (map.tile(sx, sy).haveTile(buttonType))
            matches << QPoint(sx, sy);
        if (map.tile(sx, sy).haveTile(controlTypes))
            return matches;
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return matches;
        }
    }

    Q_UNREACHABLE();
}

static QList<QPoint> areaCtlSearch(int x, int y, const cc2::MapData& map)
{
    QList<QPoint> matches;

    const int xmin = std::max(x - 2, 0);
    const int xmax = std::min(x + 2, map.width() - 1);
    const int ymin = std::max(y - 2, 0);
    const int ymax = std::min(y + 2, map.height() - 1);
    for (int sy = ymin; sy <= ymax; ++sy) {
        for (int sx = xmin; sx <= xmax; ++sx) {
            if (map.tile(sx, sy).haveTile({
                    cc2::Tile::Force_N, cc2::Tile::Force_E,
                    cc2::Tile::Force_S, cc2::Tile::Force_W,
                    cc2::Tile::ToggleWall, cc2::Tile::ToggleFloor,
                    cc2::Tile::CC1_Cloner, cc2::Tile::Cloner,
                    cc2::Tile::RevolvDoor_SW, cc2::Tile::RevolvDoor_NW,
                    cc2::Tile::RevolvDoor_NE, cc2::Tile::RevolvDoor_SE,
                    cc2::Tile::FlameJet_Off, cc2::Tile::FlameJet_On,
                    cc2::Tile::LSwitchFloor, cc2::Tile::LSwitchWall})) {
                matches << QPoint(sx, sy);
            } else {
                // Only match track tiles if the track has a switch
                const cc2::Tile* trackTile = &map.tile(sx, sy);
                while (trackTile) {
                    if (trackTile->type() == cc2::Tile::TrainTracks
                            && (trackTile->modifier() & cc2::TileModifier::TrackSwitch) != 0) {
                        matches << QPoint(sx, sy);
                        break;
                    }
                    trackTile = trackTile->lower();
                }
            }
        }
    }

    return matches;
}

static QPoint diamondClosest(const std::vector<cc2::Tile::Type>& controlTypes,
                             int x, int y, const cc2::MapData& map)
{
    int sx = x + 1, sy = y;
    int dx = -1, dy = -1;
    int scannedTiles = 1;
    const int allTiles = map.width() * map.height();

    // TODO:  The actual game will give up in certain conditions, but I don't
    // yet know what those conditions are.  This version will keep looking
    // until the entire board has been searched.
    while (scannedTiles < allTiles) {
        if (sx >= 0 && sy >= 0 && sx < map.width() && sy < map.height()) {
            if (map.tile(sx, sy).haveTile(controlTypes))
                return QPoint(sx, sy);
            ++scannedTiles;
        }

        // TODO: This could probably be optimized to eliminate the need for
        //   the range check above...
        sx += dx;
        sy += dy;
        if (sx == x)
            dy = -dy;
        if (sy == y) {
            dx = -dx;
            if (sx > x)
                sx += 1;
        }
    }

    return QPoint(-1, -1);
}

void CC2EditorWidget::addWire(cc2::Tile& tile, cc2::Tile::Direction direction)
{
    cc2::Tile& baseTile = tile.bottom();
    if (!baseTile.supportsWires())
        return;

    switch (direction) {
    case cc2::Tile::North:
        baseTile.setModifier((baseTile.modifier()
                             & ~cc2::TileModifier::WireTunnelNorth)
                             | cc2::TileModifier::WireNorth);
        break;
    case cc2::Tile::East:
        baseTile.setModifier((baseTile.modifier()
                             & ~cc2::TileModifier::WireTunnelEast)
                             | cc2::TileModifier::WireEast);
        break;
    case cc2::Tile::South:
        baseTile.setModifier((baseTile.modifier()
                             & ~cc2::TileModifier::WireTunnelSouth)
                             | cc2::TileModifier::WireSouth);
        break;
    case cc2::Tile::West:
        baseTile.setModifier((baseTile.modifier()
                             & ~cc2::TileModifier::WireTunnelWest)
                             | cc2::TileModifier::WireWest);
        break;
    default:
        // No change on invalid direction
        return;
    }

    dirtyBuffer();
}

void CC2EditorWidget::addWireTunnel(cc2::Tile& tile, cc2::Tile::Direction direction)
{
    cc2::Tile& baseTile = tile.bottom();
    if (!baseTile.supportsWires())
        return;

    switch (direction) {
    case cc2::Tile::North:
        baseTile.setModifier((baseTile.modifier()
                             | cc2::TileModifier::WireTunnelNorth)
                             | cc2::TileModifier::WireNorth);
        break;
    case cc2::Tile::East:
        baseTile.setModifier((baseTile.modifier()
                             | cc2::TileModifier::WireTunnelEast)
                             | cc2::TileModifier::WireEast);
        break;
    case cc2::Tile::South:
        baseTile.setModifier((baseTile.modifier()
                             | cc2::TileModifier::WireTunnelSouth)
                             | cc2::TileModifier::WireSouth);
        break;
    case cc2::Tile::West:
        baseTile.setModifier((baseTile.modifier()
                             | cc2::TileModifier::WireTunnelWest)
                             | cc2::TileModifier::WireWest);
        break;
    default:
        // No change on invalid direction
        return;
    }

    dirtyBuffer();
}

void CC2EditorWidget::delWire(cc2::Tile& tile, cc2::Tile::Direction direction)
{
    cc2::Tile& baseTile = tile.bottom();
    if (!baseTile.supportsWires())
        return;

    switch (direction) {
    case cc2::Tile::North:
        baseTile.setModifier(baseTile.modifier()
                             & ~(cc2::TileModifier::WireTunnelNorth
                                | cc2::TileModifier::WireNorth));
        break;
    case cc2::Tile::East:
        baseTile.setModifier(baseTile.modifier()
                             & ~(cc2::TileModifier::WireTunnelEast
                                | cc2::TileModifier::WireEast));
        break;
    case cc2::Tile::South:
        baseTile.setModifier(baseTile.modifier()
                             & ~(cc2::TileModifier::WireTunnelSouth
                                | cc2::TileModifier::WireSouth));
        break;
    case cc2::Tile::West:
        baseTile.setModifier(baseTile.modifier()
                             & ~(cc2::TileModifier::WireTunnelWest
                                | cc2::TileModifier::WireWest));
        break;
    default:
        // No change on invalid direction
        return;
    }

    dirtyBuffer();
}

void CC2EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;

    if (m_cachedButton != Qt::NoButton && (event->buttons() & m_cachedButton) == 0) {
        // We missed a mouseReleaseEvent (probably from a focus loss)
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(),
                                 event->windowPos(), event->screenPos(),
                                 m_cachedButton, event->buttons(), event->modifiers());
        mouseReleaseEvent(&releaseEvent);
    }

    const int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    const int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    if (m_current == QPoint(posX, posY) && !m_cacheDirty)
        return;
    m_current = QPoint(posX, posY);

    const cc2::MapData& map = m_map->mapData();
    if (m_cachedButton == Qt::MiddleButton && m_origin != QPoint(-1, -1)) {
        int lowX = std::min(m_origin.x(), m_current.x());
        int lowY = std::min(m_origin.y(), m_current.y());
        int highX = std::max(m_origin.x(), m_current.x());
        int highY = std::max(m_origin.y(), m_current.y());
        selectRegion(lowX, lowY, highX - lowX + 1, highY - lowY + 1);
        emit hasSelection(true);
    } else if ((m_cachedButton & (Qt::LeftButton | Qt::RightButton)) != 0) {
        const cc2::Tile& curTile = (m_cachedButton == Qt::LeftButton)
                                 ? m_leftTile : m_rightTile;
        if (m_drawMode == DrawPencil) {
            putTile(curTile, posX, posY, select_cmode(event->modifiers()));
        } else if (m_drawMode >= DrawLine && m_drawMode <= DrawFill) {
            m_map->copyFrom(m_editCache);
            // Draw current pending operation
            switch (m_drawMode) {
            case DrawLine:
                plot_line(this, m_origin, m_current, curTile, select_cmode(event->modifiers()));
                break;
            case DrawRect:
                plot_rect(this, m_origin, m_current, curTile, select_cmode(event->modifiers()));
                break;
            case DrawFill:
                plot_box(this, m_origin, m_current, curTile, select_cmode(event->modifiers()));
                break;
            default:
                Q_ASSERT(false);
            }
            dirtyBuffer();
        } else if (m_drawMode == DrawPathMaker) {
            cc2::Tile oldTile = map.tile(posX, posY);
            if (m_origin != m_current) {
                cc2::Tile::Direction dir = calc_dir(m_origin, m_current);
                cc2::Tile dirTile = directionalize(curTile, dir);
                if (curTile.type() >= cc2::Tile::Force_N && curTile.type() <= cc2::Tile::Force_W) {
                    // Crossing a force floor should turn to ice
                    const cc2::Tile& oldFloor = oldTile.bottom();
                    if (((dirTile.type() == cc2::Tile::Force_E || dirTile.type() == cc2::Tile::Force_W)
                            && (oldFloor.type() == cc2::Tile::Force_N || oldFloor.type() == cc2::Tile::Force_S))
                        || ((dirTile.type() == cc2::Tile::Force_N || dirTile.type() == cc2::Tile::Force_S)
                            && (oldFloor.type() == cc2::Tile::Force_E || oldFloor.type() == cc2::Tile::Force_W)))
                        putTile(cc2::Tile(cc2::Tile::Ice), posX, posY, select_cmode(event->modifiers()));
                    else
                        putTile(dirTile, posX, posY, select_cmode(event->modifiers()));
                    const cc2::Tile& originTile = map.tile(m_origin.x(), m_origin.y()).bottom();
                    if (originTile.type() != cc2::Tile::Ice)
                        putTile(dirTile, m_origin.x(), m_origin.y(), select_cmode(event->modifiers()));
                } else if (curTile.type() == cc2::Tile::Ice && m_lastDir != cc2::Tile::InvalidDir) {
                    // Add curves to ice
                    putTile(dirTile, posX, posY, select_cmode(event->modifiers()));
                    if ((m_lastDir == cc2::Tile::North && dir == cc2::Tile::East)
                            || (m_lastDir == cc2::Tile::West && dir == cc2::Tile::South))
                        dirTile.setType(cc2::Tile::Ice_SE);
                    else if ((m_lastDir == cc2::Tile::North && dir == cc2::Tile::West)
                            || (m_lastDir == cc2::Tile::East && dir == cc2::Tile::South))
                        dirTile.setType(cc2::Tile::Ice_SW);
                    else if ((m_lastDir == cc2::Tile::South && dir == cc2::Tile::East)
                            || (m_lastDir == cc2::Tile::West && dir == cc2::Tile::North))
                        dirTile.setType(cc2::Tile::Ice_NE);
                    else if ((m_lastDir == cc2::Tile::South && dir == cc2::Tile::West)
                            || (m_lastDir == cc2::Tile::East && dir == cc2::Tile::North))
                        dirTile.setType(cc2::Tile::Ice_NW);
                    else
                        dirTile.setType(cc2::Tile::Ice);
                    putTile(dirTile, m_origin.x(), m_origin.y(), select_cmode(event->modifiers()));
                } else if (curTile.type() == cc2::Tile::TrainTracks
                           && (curTile.modifier() & cc2::TileModifier::TrackDir_MASK) != 0
                           /*&& m_lastDir != cc2::Tile::InvalidDir*/) {
                    // Curve train tracks, and cross them if necessary
                    putTile(dirTile, posX, posY, select_cmode(event->modifiers()));
                    const cc2::Tile& originTile = map.tile(m_origin.x(), m_origin.y()).bottom();
                    if (originTile.type() == cc2::Tile::TrainTracks
                            && (originTile.modifier() & cc2::TileModifier::TrackDir_MASK) != 0) {
                        m_map->mapData().tile(m_origin.x(), m_origin.y()) = m_lastPathTile;
                        if ((m_lastDir == cc2::Tile::North && dir == cc2::Tile::East)
                                || (m_lastDir == cc2::Tile::West && dir == cc2::Tile::South))
                            dirTile.setModifier(cc2::TileModifier::Track_SE);
                        else if ((m_lastDir == cc2::Tile::North && dir == cc2::Tile::West)
                                || (m_lastDir == cc2::Tile::East && dir == cc2::Tile::South))
                            dirTile.setModifier(cc2::TileModifier::Track_SW);
                        else if ((m_lastDir == cc2::Tile::South && dir == cc2::Tile::East)
                                || (m_lastDir == cc2::Tile::West && dir == cc2::Tile::North))
                            dirTile.setModifier(cc2::TileModifier::Track_NE);
                        else if ((m_lastDir == cc2::Tile::South && dir == cc2::Tile::West)
                                || (m_lastDir == cc2::Tile::East && dir == cc2::Tile::North))
                            dirTile.setModifier(cc2::TileModifier::Track_NW);
                        putTile(dirTile, m_origin.x(), m_origin.y(), select_cmode(event->modifiers()));
                    }
                } else {
                    // Any other directional tile
                    putTile(dirTile, posX, posY, select_cmode(event->modifiers()));
                    putTile(dirTile, m_origin.x(), m_origin.y(), select_cmode(event->modifiers()));
                }
                m_origin = m_current;
                m_lastDir = dir;
            } else {
                putTile(curTile, posX, posY, select_cmode(event->modifiers()));
                m_origin = m_current;
                m_lastDir = cc2::Tile::InvalidDir;
            }
            m_lastPathTile = oldTile;
        } else if (m_drawMode == DrawWires && m_origin != m_current) {
            cc2::Tile::Direction dir = calc_dir(m_origin, m_current);
            if (m_cachedButton == Qt::RightButton) {
                delWire(m_map->mapData().tile(m_origin.x(), m_origin.y()), dir);
                delWire(m_map->mapData().tile(posX, posY), rot_180(dir));
            } else if (event->modifiers() & Qt::ShiftModifier) {
                addWireTunnel(m_map->mapData().tile(m_origin.x(), m_origin.y()), dir);
            } else {
                addWire(m_map->mapData().tile(m_origin.x(), m_origin.y()), dir);
                addWire(m_map->mapData().tile(posX, posY), rot_180(dir));
            }
            m_origin = m_current;
        } else if (m_drawMode == DrawSelect && m_origin != QPoint(-1, -1)) {
            int lowX = std::min(m_origin.x(), m_current.x());
            int lowY = std::min(m_origin.y(), m_current.y());
            int highX = std::max(m_origin.x(), m_current.x());
            int highY = std::max(m_origin.y(), m_current.y());
            selectRegion(lowX, lowY, highX - lowX + 1, highY - lowY + 1);
            emit hasSelection(true);

            //TODO:  Allow dragging of floating selection without losing data
        }
    }

    const cc2::Tile* tile = &map.tile(posX, posY);
    const cc2::Tile* baseTile = &tile->bottom();
    QString info = QStringLiteral("(%1, %2): %3").arg(posX).arg(posY).arg(CC2ETileset::getName(tile));
    while ((tile = tile->lower()) != nullptr && tile->type() != cc2::Tile::Floor)
        info += tr(" / %1").arg(CC2ETileset::getName(tile));
    emit mouseInfo(info);

    QString tipText;
    m_hilights.clear();
    switch (baseTile->type()) {
    case cc2::Tile::Teleport_Blue:
        {
            // TODO: Handle wires
            QPoint nextTeleport = scanForR(cc2::Tile::Teleport_Blue, posX, posY, map);
            if (nextTeleport != QPoint(-1, -1)) {
                m_hilights << nextTeleport;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
            }
        }
        break;
    case cc2::Tile::Teleport_Red:
        {
            QPoint nextTeleport = scanForF(cc2::Tile::Teleport_Red, posX, posY, map);
            if (nextTeleport != QPoint(-1, -1)) {
                m_hilights << nextTeleport;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
            }
        }
        break;
    case cc2::Tile::Teleport_Green:
        {
            QList<QPoint> teleports = scanForAll(cc2::Tile::Teleport_Green, map);
            for (const QPoint& teleport : teleports) {
                // Exclude the teleport under the cursor from the highlight list
                if (teleport.x() == posX && teleport.y() == posY)
                    continue;
                m_hilights << teleport;
            }
        }
        break;
    case cc2::Tile::Teleport_Yellow:
        {
            QPoint nextTeleport = scanForR(cc2::Tile::Teleport_Yellow, posX, posY, map);
            if (nextTeleport != QPoint(-1, -1)) {
                m_hilights << nextTeleport;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
            }
        }
        break;
    case cc2::Tile::CloneButton:
        {
            QPoint cloner = scanForControl({cc2::Tile::Cloner, cc2::Tile::CC1_Cloner},
                                           posX, posY, map);
            if (cloner != QPoint(-1, -1)) {
                m_hilights << cloner;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Cloner: (%1, %2)").arg(cloner.x()).arg(cloner.y());
            }
        }
        break;
    case cc2::Tile::Cloner:
    case cc2::Tile::CC1_Cloner:
        {
            QList<QPoint> buttons = scanForButtons(cc2::Tile::CloneButton,
                                           {cc2::Tile::Cloner, cc2::Tile::CC1_Cloner},
                                           posX, posY, map);
            for (const QPoint& button : buttons) {
                m_hilights << button;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Button: (%1, %2)").arg(button.x()).arg(button.y());
            }
        }
        break;
    case cc2::Tile::TrapButton:
        {
            QPoint trap = scanForControl({cc2::Tile::Trap, cc2::Tile::Trap_Open},
                                         posX, posY, map);
            if (trap != QPoint(-1, -1)) {
                m_hilights << trap;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Trap: (%1, %2)").arg(trap.x()).arg(trap.y());
            }
        }
        break;
    case cc2::Tile::Trap:
    case cc2::Tile::Trap_Open:
        {
            QList<QPoint> buttons = scanForButtons(cc2::Tile::TrapButton,
                                                   {cc2::Tile::Trap, cc2::Tile::Trap_Open},
                                                   posX, posY, map);
            for (const QPoint& button : buttons) {
                m_hilights << button;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Button: (%1, %2)").arg(button.x()).arg(button.y());
            }
        }
        break;
    case cc2::Tile::FlameJetButton:
        {
            QPoint jet = diamondClosest({cc2::Tile::FlameJet_Off, cc2::Tile::FlameJet_On},
                                        posX, posY, map);
            if (jet != QPoint(-1, -1)) {
                m_hilights << jet;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Flame Jet: (%1, %2)").arg(jet.x()).arg(jet.y());
            }
        }
        break;
    case cc2::Tile::AreaCtlButton:
        {
            QList<QPoint> controlTiles = areaCtlSearch(posX, posY, map);
            for (const QPoint& ctile : controlTiles) {
                m_hilights << ctile;
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("(%1, %2)").arg(ctile.x()).arg(ctile.y());
            }
        }
        break;
    default:
        break;
    }

    std::string clue = m_map->clueForTile(posX, posY);
    if (!clue.empty()) {
        if (!tipText.isEmpty())
            tipText += QLatin1Char('\n');
        tipText += tr("Clue:\n") + ccl::fromLatin1(clue).trimmed();
    }

    setToolTip(tipText);
    update();
}

void CC2EditorWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;
    if (m_cachedButton != Qt::NoButton
            || (event->button() & (Qt::LeftButton | Qt ::MiddleButton | Qt::RightButton)) == 0)
        return;

    const int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    const int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    m_current = QPoint(-1, -1);
    m_cachedButton = event->button();
    m_editCache->copyFrom(m_map);

    if ((m_cachedButton == Qt::LeftButton || m_cachedButton == Qt::RightButton)
            && m_drawMode >= DrawPencil && m_drawMode <= DrawWires)
        beginEdit(CC2EditHistory::EditMap);

    if (m_drawMode != DrawSelect && event->button() != Qt::MiddleButton) {
        m_selectRect = QRect(-1, -1, -1, -1);
        emit hasSelection(false);
    }

    if (m_cachedButton == Qt::MiddleButton) {
        m_origin = QPoint(posX, posY);
    } else if (m_drawMode == DrawSelect) {
        if (m_cachedButton == Qt::LeftButton) {
            m_origin = QPoint(posX, posY);
        } else if (m_cachedButton == Qt::RightButton) {
            m_origin = QPoint(-1, -1);
            m_selectRect = QRect(-1, -1, -1, -1);
            emit hasSelection(false);
        }
    } else {
        m_origin = QPoint(posX, posY);
    }

    mouseMoveEvent(event);
}

void CC2EditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;
    if (event->button() != m_cachedButton)
        return;

    bool resetOrigin = true;
    if (m_drawMode == DrawInspectTile || m_drawMode == DrawInspectHint) {
        emit tilePicked(m_origin.x(), m_origin.y());
    } else if (m_drawMode == DrawSelect || m_cachedButton == Qt::MiddleButton) {
        resetOrigin = false;
    } else if (m_drawMode == DrawFlood) {
        if (m_cachedButton == Qt::LeftButton)
            plot_flood(this, m_current, m_leftTile, select_cmode(event->modifiers()));
        else if (m_cachedButton == Qt::RightButton)
            plot_flood(this, m_current, m_rightTile, select_cmode(event->modifiers()));
    }

    if (resetOrigin)
        m_origin = QPoint(-1, -1);
    if ((m_cachedButton == Qt::LeftButton || m_cachedButton == Qt::RightButton)
            && m_drawMode >= DrawPencil && m_drawMode <= DrawWires)
        endEdit();

    update();
    m_cachedButton = Qt::NoButton;
    m_lastDir = cc2::Tile::InvalidDir;
}

static uint32_t trackToActive(uint32_t trackModifier)
{
    if ((trackModifier & cc2::TileModifier::Track_NE) != 0)
        return cc2::TileModifier::ActiveTrack_NE;
    if ((trackModifier & cc2::TileModifier::Track_SE) != 0)
        return cc2::TileModifier::ActiveTrack_SE;
    if ((trackModifier & cc2::TileModifier::Track_SW) != 0)
        return cc2::TileModifier::ActiveTrack_SW;
    if ((trackModifier & cc2::TileModifier::Track_NW) != 0)
        return cc2::TileModifier::ActiveTrack_NW;
    if ((trackModifier & cc2::TileModifier::Track_WE) != 0)
        return cc2::TileModifier::ActiveTrack_WE;
    if ((trackModifier & cc2::TileModifier::Track_NS) != 0)
        return cc2::TileModifier::ActiveTrack_NS;
    return 0;
}

static cc2::Tile::Direction cloneDirection(uint32_t modifier)
{
    if ((modifier & cc2::TileModifier::CloneNorth) != 0)
        return cc2::Tile::North;
    if ((modifier & cc2::TileModifier::CloneEast) != 0)
        return cc2::Tile::East;
    if ((modifier & cc2::TileModifier::CloneSouth) != 0)
        return cc2::Tile::South;
    if ((modifier & cc2::TileModifier::CloneWest) != 0)
        return cc2::Tile::West;
    return cc2::Tile::InvalidDir;
}

static uint32_t cloneModifier(cc2::Tile::Direction dir)
{
    switch (dir) {
    case cc2::Tile::North:
        return cc2::TileModifier::CloneNorth;
    case cc2::Tile::East:
        return cc2::TileModifier::CloneEast;
    case cc2::Tile::South:
        return cc2::TileModifier::CloneSouth;
    case cc2::Tile::West:
        return cc2::TileModifier::CloneWest;
    default:
        return 0;
    }
}

enum ReplaceMode { REPLACE_NONE, REPLACE_LAYER, REPLACE_TYPE };
static bool matchTiles(const cc2::Tile& first, const cc2::Tile& second, ReplaceMode mode)
{
    switch (mode) {
    case REPLACE_LAYER:
        return first.layer() == second.layer();
    case REPLACE_TYPE:
        return first.type() == second.type();
    default:
        return false;
    }
}

static void pushTile(cc2::Tile& destTile, cc2::Tile tile, ReplaceMode mode)
{
    *tile.lower() = destTile;
    destTile = std::move(tile);

    // Remove duplicates (if you REALLY want to stack duplicates, use the
    // tile inspector tool...  Otherwise, this just gets messy)
    cc2::Tile* tp = destTile.lower();
    while (tp) {
        if (matchTiles(*tp, destTile, mode)) {
            cc2::Tile* lower = tp->lower();
            if (lower)
                *tp = *tp->lower();
            else
                *tp = cc2::Tile();
        } else {
            tp = tp->lower();
        }
    }
}

static void popTile(cc2::Tile& destTile)
{
    // Pop the top *visible* layer, which might not be the actual top of
    // the tile list...
    cc2::Tile* topLayer = destTile.topVisible();
    cc2::Tile* lower = topLayer->lower();
    if (lower)
        *topLayer = *lower;
    else
        *topLayer = cc2::Tile();
}

void CC2EditorWidget::putTile(const cc2::Tile& tile, int x, int y, CombineMode mode)
{
    cc2::Tile& curTile = m_map->mapData().tile(x, y);
    cc2::Tile& baseTile = curTile.bottom();
    // WARNING: Modifying curTile's layers can invalidate the baseTile reference!
    uint32_t baseWires = (baseTile.supportsWires() ? baseTile.modifier() : 0)
                         & cc2::TileModifier::WireMask;

    const bool clueTile = (baseTile.type() == cc2::Tile::Clue);

    if (mode == Replace) {
        curTile = tile;
    } else if (mode == CombineForce) {
        if (tile.type() == baseTile.type()) {
            // Combine flags on matching tiles
            baseTile.setModifier(baseTile.modifier() | tile.modifier());
            baseTile.setTileFlags(baseTile.tileFlags() | tile.tileFlags());
        } else if (tile.haveLower()) {
            pushTile(curTile, tile, REPLACE_TYPE);
        } else {
            baseTile = tile;
            if (baseTile.supportsWires())
                baseTile.setModifier(baseWires | tile.modifier());
        }
    } else if (tile.type() == cc2::Tile::Floor && tile.modifier() == 0) {
        // Floor with no wires pops the top layer
        popTile(curTile);
    } else if ((tile.type() == cc2::Tile::Floor && tile.modifier() != 0
                    && baseTile.type() == cc2::Tile::Floor)
                || (tile.type() == cc2::Tile::TrainTracks
                    && baseTile.type() == cc2::Tile::TrainTracks)) {
        // Combine wire tunnels and tracks
        baseTile.setModifier(baseTile.modifier() | tile.modifier());
    } else if (tile.type() == cc2::Tile::PanelCanopy) {
        // Combine panel/canopy flags
        cc2::Tile* curPanelCanopy = &curTile;
        while (curPanelCanopy && !curPanelCanopy->isPanelCanopy())
            curPanelCanopy = curPanelCanopy->lower();
        if (curPanelCanopy) {
            if (curPanelCanopy->type() == cc2::Tile::PanelCanopy) {
                curPanelCanopy->setTileFlags(curPanelCanopy->tileFlags() | tile.tileFlags());
            } else {
                // Upgrade CC1 panels to the CC2 equivalent
                uint8_t panelFlags = tile.tileFlags();
                if (curPanelCanopy->type() == cc2::Tile::CC1_Barrier_S
                    || curPanelCanopy->type() == cc2::Tile::CC1_Barrier_SE)
                    panelFlags |= cc2::Tile::PanelSouth;
                if (curPanelCanopy->type() == cc2::Tile::CC1_Barrier_E
                    || curPanelCanopy->type() == cc2::Tile::CC1_Barrier_SE)
                    panelFlags |= cc2::Tile::PanelEast;
                *curPanelCanopy = cc2::Tile::panelTile(panelFlags);
            }
        } else {
            pushTile(curTile, tile, REPLACE_LAYER);
        }
    } else if (tile.type() == cc2::Tile::Cloner || tile.type() == cc2::Tile::CC1_Cloner) {
        if (curTile.isCreature()) {
            // Adjust the cloner to match the creature's direction
            *curTile.lower() = tile;
            curTile.lower()->setModifier(cloneModifier(curTile.direction()));
        } else if (curTile.isBlock()) {
            // Adjust the block to match the cloner's direction
            *curTile.lower() = tile;
            const cc2::Tile::Direction dir = cloneDirection(tile.modifier());
            if (dir != cc2::Tile::InvalidDir)
                curTile.setDirection(dir);
        } else {
            // Something unclonable was here before...
            curTile = tile;
        }
    } else if (baseTile.type() == cc2::Tile::Cloner || baseTile.type() == cc2::Tile::CC1_Cloner) {
        if (tile.isCreature()) {
            // Remove anything already on the cloner, and adjust the
            // cloner to match the creature's direction
            curTile = baseTile;
            curTile.setModifier(cloneModifier(tile.direction()));
            pushTile(curTile, tile, REPLACE_NONE);
        } else if (tile.isBlock()) {
            // Remove anything already on the cloner, and adjust the
            // block to match the cloner's direction
            const cc2::Tile::Direction dir = cloneDirection(baseTile.modifier());
            curTile = baseTile;
            cc2::Tile dirBlock(tile);
            if (dir != cc2::Tile::InvalidDir)
                dirBlock.setDirection(dir);
            pushTile(curTile, std::move(dirBlock), REPLACE_NONE);
        } else {
            // Something unclonable is being placed...
            curTile = tile;
        }
    } else if (tile.haveLower()) {
        pushTile(curTile, tile, REPLACE_LAYER);
    } else {
        // For everything else: just replace the entire tile
        curTile = tile;
        if (curTile.supportsWires())
            curTile.setModifier(baseWires | tile.modifier());
    }

    if (tile.type() == cc2::Tile::TrainTracks) {
        // Set the active track, if necessary
        cc2::Tile& trackTile = curTile.bottom();
        Q_ASSERT(trackTile.type() == cc2::Tile::TrainTracks);
        if (trackTile.modifier() & cc2::TileModifier::TrackSwitch) {
            const uint32_t activeBase = trackTile.modifier() & ~cc2::TileModifier::ActiveTrack_MASK;
            if ((tile.modifier() & cc2::TileModifier::TrackDir_MASK) != 0) {
                // Use the track in the current drawing tile
                trackTile.setModifier(activeBase | trackToActive(tile.modifier()));
            } else {
                // Find the first valid track and make it active
                trackTile.setModifier(activeBase | trackToActive(trackTile.modifier()));
            }
        }
    }

    if (curTile.bottom().type() == cc2::Tile::TrainTracks) {
        // Ensure the "entered" direction is set for train tracks with mobs
        cc2::Tile& trackTile = curTile.bottom();
        const uint32_t baseMod = trackTile.modifier() & ~cc2::TileModifier::TrackEntered_MASK;
        cc2::Tile* mobTile = &curTile;
        while (mobTile != nullptr) {
            if (mobTile->haveDirection()) {
                trackTile.setModifier(baseMod | mobTile->direction() << cc2::TileModifier::TrackEntered_SHIFT);
                break;
            }
            mobTile = mobTile->lower();
        }
    }

    if (clueTile && curTile.bottom().type() != cc2::Tile::Clue)
        emit clueDeleted(x, y);
    else if (!clueTile && curTile.bottom().type() == cc2::Tile::Clue)
        emit clueAdded(x, y);

    dirtyBuffer();
}

void CC2EditorWidget::setZoom(double factor)
{
    m_zoomFactor = factor;
    resize(sizeHint());
    dirtyBuffer();
}

void CC2EditorWidget::undo()
{
    m_undoStack->undo();
    updateForUndoCommand(m_undoStack->command(m_undoStack->index()));
}

void CC2EditorWidget::redo()
{
    auto command = m_undoStack->command(m_undoStack->index());
    m_undoStack->redo();
    updateForUndoCommand(command);
}

void CC2EditorWidget::updateForUndoCommand(const QUndoCommand* command)
{
    auto mapCommand = dynamic_cast<const MapUndoCommand*>(command);
    if (mapCommand) {
        if (mapCommand->id() == CC2EditHistory::EditResizeMap) {
            m_tileBuffer = QPixmap(m_map->mapData().width() * m_tileset->size(),
                                   m_map->mapData().height() * m_tileset->size());
            resize(sizeHint());
        }

        dirtyBuffer();
    }
}
