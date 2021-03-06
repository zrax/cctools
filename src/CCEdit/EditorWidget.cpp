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
#include <queue>
#include "libcc1/GameLogic.h"
#include "CommonWidgets/CCTools.h"

static EditorWidget::DrawLayer select_layer(Qt::KeyboardModifiers keys)
{
    if ((keys & Qt::ShiftModifier) != 0)
        return EditorWidget::LayBottom;
    else if ((keys & Qt::ControlModifier) != 0)
        return EditorWidget::LayTop;
    return EditorWidget::LayAuto;
}

static void plot_box(EditorWidget* self, QPoint from, QPoint to, tile_t drawTile,
                     EditorWidget::DrawLayer layer)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = std::min(from.y(), to.y());
    int lowX = std::min(from.x(), to.x());
    int highY = std::max(from.y(), to.y());
    int highX = std::max(from.x(), to.x());

    for (int y = lowY; y <= highY; ++y)
        for (int x = lowX; x <= highX; ++x)
            self->putTile(drawTile, x, y, layer);
}

static void plot_rect(EditorWidget* self, QPoint from, QPoint to, tile_t drawTile,
                      EditorWidget::DrawLayer layer)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = std::min(from.y(), to.y());
    int lowX = std::min(from.x(), to.x());
    int highY = std::max(from.y(), to.y());
    int highX = std::max(from.x(), to.x());

    for (int x = lowX; x <= highX; ++x) {
        self->putTile(drawTile, x, lowY, layer);
        self->putTile(drawTile, x, highY, layer);
    }
    for (int y = lowY + 1; y <= highY - 1; ++y) {
        self->putTile(drawTile, lowX, y, layer);
        self->putTile(drawTile, highX, y, layer);
    }
}

static void plot_line(EditorWidget* self, QPoint from, QPoint to, tile_t drawTile,
                      EditorWidget::DrawLayer layer)
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
        self->putTile(drawTile, steep ? y : x, steep ? x : y, layer);
        err -= dY;
        if (err < 0) {
            y += ystep;
            err += dX;
        }
    }
}

static void plot_flood(EditorWidget* self, QPoint start, tile_t drawTile,
                       EditorWidget::DrawLayer layer)
{
    ccl::LevelMap& map = self->levelData()->map();
    tile_t replace_bg = map.getBG(start.x(), start.y());
    tile_t replace_fg = map.getFG(start.x(), start.y());

    std::queue<QPoint> floodQueue;
    floodQueue.push(start);
    self->putTile(drawTile, start.x(), start.y(), layer);
    if (map.getBG(start.x(), start.y()) == replace_bg
            && map.getFG(start.x(), start.y()) == replace_fg) {
        // No change was made.  Exit to avoid an infinite loop
        return;
    }

    while (!floodQueue.empty()) {
        QPoint pt = floodQueue.front();
        floodQueue.pop();

        if (pt.x() > 0 && map.getBG(pt.x() - 1, pt.y()) == replace_bg
                && map.getFG(pt.x() - 1, pt.y()) == replace_fg) {
            QPoint next(pt.x() - 1, pt.y());
            self->putTile(drawTile, next.x(), next.y(), layer);
            floodQueue.push(next);
        }
        if (pt.x() < 31 && map.getBG(pt.x() + 1, pt.y()) == replace_bg
                && map.getFG(pt.x() + 1, pt.y()) == replace_fg) {
            QPoint next(pt.x() + 1, pt.y());
            self->putTile(drawTile, next.x(), next.y(), layer);
            floodQueue.push(next);
        }
        if (pt.y() > 0 && map.getBG(pt.x(), pt.y() - 1) == replace_bg
                && map.getFG(pt.x(), pt.y() - 1) == replace_fg) {
            QPoint next(pt.x(), pt.y() - 1);
            self->putTile(drawTile, next.x(), next.y(), layer);
            floodQueue.push(next);
        }
        if (pt.y() < 31 && map.getBG(pt.x(), pt.y() + 1) == replace_bg
                && map.getFG(pt.x(), pt.y() + 1) == replace_fg) {
            QPoint next(pt.x(), pt.y() + 1);
            self->putTile(drawTile, next.x(), next.y(), layer);
            floodQueue.push(next);
        }
    }
}

enum ConnType { ConnNone, ConnTrap, ConnTrapRev, ConnClone, ConnCloneRev };

static bool test_start_connect(ccl::LevelData* level, QPoint from)
{
    if (level->traps().size() < MAX_TRAPS) {
        if ((level->map().getFG(from.x(), from.y()) == ccl::TileTrapButton)
            || (level->map().getBG(from.x(), from.y()) == ccl::TileTrapButton)
            || (level->map().getFG(from.x(), from.y()) == ccl::TileTrap)
            || (level->map().getBG(from.x(), from.y()) == ccl::TileTrap))
            return true;
    }
    if (level->clones().size() < MAX_CLONES) {
        if ((level->map().getFG(from.x(), from.y()) == ccl::TileCloneButton)
            || (level->map().getBG(from.x(), from.y()) == ccl::TileCloneButton)
            || (level->map().getFG(from.x(), from.y()) == ccl::TileCloner)
            || (level->map().getBG(from.x(), from.y()) == ccl::TileCloner))
            return true;
    }
    return false;
}

static ConnType test_connect(ccl::LevelData* level, QPoint from, QPoint to)
{
    if (level->traps().size() < MAX_TRAPS) {
        if (((level->map().getFG(from.x(), from.y()) == ccl::TileTrapButton)
                || (level->map().getBG(from.x(), from.y()) == ccl::TileTrapButton))
            && ((level->map().getFG(to.x(), to.y()) == ccl::TileTrap)
                || (level->map().getBG(to.x(), to.y()) == ccl::TileTrap)))
            return ConnTrap;
        else if (((level->map().getFG(to.x(), to.y()) == ccl::TileTrapButton)
                || (level->map().getBG(to.x(), to.y()) == ccl::TileTrapButton))
            && ((level->map().getFG(from.x(), from.y()) == ccl::TileTrap)
                || (level->map().getBG(from.x(), from.y()) == ccl::TileTrap)))
            return ConnTrapRev;
    }
    if (level->clones().size() < MAX_CLONES) {
        if (((level->map().getFG(from.x(), from.y()) == ccl::TileCloneButton)
                || (level->map().getBG(from.x(), from.y()) == ccl::TileCloneButton))
            && ((level->map().getFG(to.x(), to.y()) == ccl::TileCloner)
                || (level->map().getBG(to.x(), to.y()) == ccl::TileCloner)))
            return ConnClone;
        else if (((level->map().getFG(to.x(), to.y()) == ccl::TileCloneButton)
                || (level->map().getBG(to.x(), to.y()) == ccl::TileCloneButton))
            && ((level->map().getFG(from.x(), from.y()) == ccl::TileCloner)
                || (level->map().getBG(from.x(), from.y()) == ccl::TileCloner)))
            return ConnCloneRev;
    }
    return ConnNone;
}

static ccl::Direction calc_dir(const QPoint& from, const QPoint& to)
{
    int dX = to.x() - from.x();
    int dY = to.y() - from.y();
    if (dX < 0)
        return ccl::DirWest;
    else if (dX > 0)
        return ccl::DirEast;
    else if (dY < 0)
        return ccl::DirNorth;
    else if (dY > 0)
        return ccl::DirSouth;
    return ccl::DirInvalid;
}

static tile_t directionalize(tile_t tile, ccl::Direction dir)
{
    static tile_t _force_dir[4] = {
        ccl::TileForce_N, ccl::TileForce_W, ccl::TileForce_S,
        ccl::TileForce_E
    };

    if (dir == ccl::DirInvalid)
        return tile;
    else if (FORCE_TILE(tile))
        // Not in order
        return _force_dir[(int)dir-1];
    else if (tile >= ccl::TileBlock_N && tile <= ccl::TileBlock_E)
        // Not aligned to last 2 bits
        return ccl::TileBlock_N + (int)dir - 1;
    else if (MONSTER_TILE(tile) || (tile >= ccl::TilePlayer_N && tile <= ccl::TilePlayer_E)
             || (tile >= ccl::TilePlayerSwim_N && tile <= ccl::TilePlayerSwim_E))
        return (tile & ~3) + ((int)dir - 1);
    return tile;
}

static bool isValidPoint(const ccl::Point& point)
{
    return (point.X >= 0 && point.X < 32 && point.Y >= 0 && point.Y < 32);
}

static QPoint find_player(ccl::LevelData* levelData)
{
    for (int y = 31; y >= 0; --y) {
        for (int x = 31; x >= 0; --x) {
            if (levelData->map().getFG(x, y) >= ccl::TilePlayer_N
                && levelData->map().getFG(x, y) <= ccl::TilePlayer_E)
                return QPoint(x, y);
        }
    }
    return QPoint(0, 0);
}


EditorWidget::EditorWidget(QWidget* parent)
    : QWidget(parent), m_tileset(), m_levelData(), m_leftTile(), m_rightTile(),
      m_drawMode(DrawPencil), m_paintFlags(), m_cachedButton(Qt::NoButton),
      m_numbers(QStringLiteral(":/res/numbers.png")),
      m_errmk(QStringLiteral(":/res/err-mark.png")),
      m_lastDir(ccl::DirInvalid), m_zoomFactor(1.0)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);

    m_levelEditCache = new ccl::LevelData;
}

void EditorWidget::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    m_tileBuffer = QPixmap(32 * m_tileset->size(), 32 * m_tileset->size());
    resize(sizeHint());
    dirtyBuffer();
}

void EditorWidget::setLevelData(ccl::LevelData* level)
{
    level->ref();
    if (m_levelData)
        m_levelData->unref();
    m_levelData = level;

    m_origin = QPoint(-1, -1);
    m_selectRect = QRect(-1, -1, -1, -1);
    dirtyBuffer();

    emit hasSelection(false);
}

void EditorWidget::renderTileBuffer()
{
    QPainter tilePainter(&m_tileBuffer);
    if ((m_paintFlags & RevealLower) != 0) {
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                m_tileset->draw(tilePainter, x, y, m_levelData->map().getBG(x, y));
        tilePainter.setOpacity(0.15);
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                m_tileset->draw(tilePainter, x, y, m_levelData->map().getFG(x, y),
                                m_levelData->map().getBG(x, y));
        tilePainter.setOpacity(1.0);
    } else {
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                m_tileset->draw(tilePainter, x, y, m_levelData->map().getFG(x, y),
                                m_levelData->map().getBG(x, y));
    }

    if ((m_paintFlags & ShowErrors) != 0) {
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                if (m_levelData->map().getBG(x, y) != ccl::TileFloor
                    && !(m_levelData->map().getFG(x, y) >= ccl::TileBlock_N
                         && m_levelData->map().getFG(x, y) <= ccl::TileBlock_E)
                    && m_levelData->map().getFG(x, y) != ccl::TileBlock
                    && m_levelData->map().getFG(x, y) != ccl::TileIceBlock
                    && !(m_levelData->map().getFG(x, y) >= ccl::TilePlayer_N
                        && m_levelData->map().getFG(x, y) <= ccl::TilePlayer_E)
                    && !MONSTER_TILE(m_levelData->map().getFG(x, y)))
                    tilePainter.drawPixmap(x * m_tileset->size(), y * m_tileset->size(), m_errmk);
            }
        }
    }
}

void EditorWidget::paintEvent(QPaintEvent*)
{
    if (!m_tileset || !m_levelData)
        return;

    QPainter painter(this);
    renderTo(painter);
}

void EditorWidget::renderTo(QPainter& painter)
{
    if (m_cacheDirty) {
        renderTileBuffer();
        m_tileCache = m_tileBuffer.scaled(sizeHint());
        m_cacheDirty = false;
    }
    painter.drawPixmap(0, 0, m_tileCache);

    if (m_selectRect != QRect(-1, -1, -1, -1)) {
        QRect selectionArea = calcTileRect(m_selectRect);
        painter.fillRect(selectionArea, QBrush(QColor(95, 95, 191, 127)));
        painter.setPen(QColor(63, 63, 191));
        painter.drawRect(selectionArea);
    }

    if ((m_paintFlags & ShowMovement) != 0) {
        int num = 0;
        for (const ccl::Point& mover : m_levelData->moveList()) {
            if (!isValidPoint(mover))
                continue;
            painter.drawPixmap((mover.X + 1) * m_tileset->size() * m_zoomFactor - 16,
                               (mover.Y + 1) * m_tileset->size() * m_zoomFactor - 10,
                               m_numbers, 0, num++ * 10, 16, 10);
        }
    }

    if ((m_paintFlags & ShowMovePaths) != 0) {
        painter.setPen(QColor(0, 127, 255));
        for (ccl::Point from : m_levelData->moveList()) {
            if (!isValidPoint(from))
                continue;

            uint8_t looked[32*32];
            memset(looked, 0, sizeof(looked));
            tile_t tile = m_levelData->map().getFG(from.X, from.Y);
            ccl::MoveState move = ccl::CheckMove(m_levelData, tile, from.X, from.Y);

            do {
                looked[(from.Y*32)+from.X] |= 1 << (tile & 0x03);
                if ((move & ccl::MoveDirMask) < ccl::MoveBlocked) {
                    if ((move & ccl::MoveTrapped) != 0)
                        break;
                    ccl::Point to = ccl::AdvanceCreature(from, move);
                    painter.drawLine(calcPathCenter(from.X, from.Y),
                                     calcPathCenter(to.X, to.Y));
                    if ((move & ccl::MoveDeath) != 0)
                        break;
                    if ((move & ccl::MoveTeleport) != 0) {
                        //TODO
                        break;
                    }
                    from = to;
                    tile = ccl::TurnCreature(tile, move);
                } else {
                    break;
                }
                move = ccl::CheckMove(m_levelData, tile, from.X, from.Y);
            } while ((looked[(from.Y*32)+from.X] & (1 << (tile & 0x03))) == 0);
        }
    }

    if ((m_paintFlags & ShowPlayer) != 0) {
        painter.setPen(QColor(255, 127, 0));
        QPoint playerPos = find_player(m_levelData);
        painter.drawRect(calcTileRect(playerPos));
    }

    if ((m_paintFlags & ShowViewBox) != 0) {
        painter.setPen(QColor(0, 255, 127));
        QPoint topRight(m_current.x() - 4, m_current.y() - 4);
        if (topRight.x() < 0)
            topRight.setX(0);
        if (topRight.y() < 0)
            topRight.setY(0);
        if (topRight.x() > 23)
            topRight.setX(23);
        if (topRight.y() > 23)
            topRight.setY(23);
        painter.drawRect(calcTileRect(topRight.x(), topRight.y(), 9, 9));
    }

    if ((m_paintFlags & ShowButtons) != 0) {
        painter.setPen(QColor(255, 0, 0));
        for (const auto& trap_iter : m_levelData->traps()) {
            if (!isValidPoint(trap_iter.button) || !isValidPoint(trap_iter.trap))
                continue;
            painter.drawLine(calcTileCenter(trap_iter.button.X, trap_iter.button.Y),
                             calcTileCenter(trap_iter.trap.X, trap_iter.trap.Y));
        }
        for (const auto& clone_iter : m_levelData->clones()) {
            if (!isValidPoint(clone_iter.button) || !isValidPoint(clone_iter.clone))
                continue;
            painter.drawLine(calcTileCenter(clone_iter.button.X, clone_iter.button.Y),
                             calcTileCenter(clone_iter.clone.X, clone_iter.clone.Y));
        }
    }

    if (m_drawMode == DrawButtonConnect && m_origin != QPoint(-1, -1)
        && m_selectRect == QRect(-1, -1, -1, -1)) {
        if (test_connect(m_levelData, m_origin, m_current) != ConnNone)
            painter.setPen(QColor(255, 0, 0));
        else
            painter.setPen(QColor(127, 127, 127));
        painter.drawLine(calcTileCenter(m_origin), calcTileCenter(m_current));
    }

    // Highlight context-sensitive objects
    painter.setPen(QColor(255, 0, 0));
    for (const QPoint& hi : m_hilights)
        painter.drawRect(calcTileRect(hi));
}

/* This is only used to generate special reports, so no old draw/render state
 * data needs to be saved.
 */
QImage EditorWidget::renderReport()
{
    m_paintFlags = ShowAll;
    m_zoomFactor = 1.0;
    m_cacheDirty = true;
    m_current = find_player(m_levelData);

    QImage output(m_tileset->size() * 32, m_tileset->size() * 32, QImage::Format_RGB32);
    QPainter painter(&output);
    renderTo(painter);
    return output;
}

QImage EditorWidget::renderSelection()
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

void EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_levelData || !rect().contains(event->pos()))
        return;

    if (m_cachedButton != Qt::NoButton && (event->buttons() & m_cachedButton) == 0) {
        // We missed a mouseReleaseEvent (probably from a focus loss)
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, event->localPos(),
                                 event->windowPos(), event->screenPos(),
                                 m_cachedButton, event->buttons(), event->modifiers());
        mouseReleaseEvent(&releaseEvent);
    }

    int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    if (m_current == QPoint(posX, posY) && !m_cacheDirty)
        return;
    m_current = QPoint(posX, posY);

    if (event->modifiers() & Qt::ShiftModifier)
        setPaintFlag(RevealLower);
    else
        clearPaintFlag(RevealLower);

    if (m_cachedButton == Qt::MiddleButton && m_origin != QPoint(-1, -1)) {
        int lowX = std::min(m_origin.x(), m_current.x());
        int lowY = std::min(m_origin.y(), m_current.y());
        int highX = std::max(m_origin.x(), m_current.x());
        int highY = std::max(m_origin.y(), m_current.y());
        selectRegion(lowX, lowY, highX - lowX + 1, highY - lowY + 1);
        emit hasSelection(true);
    } else if ((m_cachedButton & (Qt::LeftButton | Qt::RightButton)) != 0) {
        tile_t curtile = (m_cachedButton == Qt::LeftButton)
                       ? m_leftTile : m_rightTile;
        if (m_drawMode == DrawPencil) {
            putTile(curtile, posX, posY, select_layer(event->modifiers()));
        } else if (m_drawMode >= DrawLine && m_drawMode <= DrawFill) {
            m_levelData->copyFrom(m_levelEditCache);
            // Draw current pending operation
            switch (m_drawMode) {
            case DrawLine:
                plot_line(this, m_origin, m_current, curtile, select_layer(event->modifiers()));
                break;
            case DrawRect:
                plot_rect(this, m_origin, m_current, curtile, select_layer(event->modifiers()));
                break;
            case DrawFill:
                plot_box(this, m_origin, m_current, curtile, select_layer(event->modifiers()));
                break;
            default:
                Q_ASSERT(false);
            }
            dirtyBuffer();
        } else if (m_drawMode == DrawPathMaker) {
            if (m_origin != m_current) {
                ccl::Direction dir = calc_dir(m_origin, m_current);
                tile_t dirtile = directionalize(curtile, dir);
                if (FORCE_TILE(curtile)) {
                    // Crossing a force floor should turn to ice
                    tile_t oldTile = (event->modifiers() & Qt::ShiftModifier) == 0
                                   ? m_levelData->map().getFG(posX, posY)
                                   : m_levelData->map().getBG(posX, posY);
                    if (((dirtile == ccl::TileForce_E || dirtile == ccl::TileForce_W)
                            && (oldTile == ccl::TileForce_N || oldTile == ccl::TileForce_S))
                        || ((dirtile == ccl::TileForce_N || dirtile == ccl::TileForce_S)
                            && (oldTile == ccl::TileForce_E || oldTile == ccl::TileForce_W)))
                        putTile(ccl::TileIce, posX, posY, select_layer(event->modifiers()));
                    else
                        putTile(dirtile, posX, posY, select_layer(event->modifiers()));
                    oldTile = (event->modifiers() & Qt::ShiftModifier) == 0
                            ? m_levelData->map().getFG(m_origin.x(), m_origin.y())
                            : m_levelData->map().getBG(m_origin.x(), m_origin.y());
                    if (oldTile != ccl::TileIce)
                        putTile(dirtile, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                } else if (curtile == ccl::TileIce && m_lastDir != ccl::DirInvalid) {
                    // Add curves to ice
                    putTile(dirtile, posX, posY, select_layer(event->modifiers()));
                    if ((m_lastDir == ccl::DirNorth && dir == ccl::DirEast)
                        || (m_lastDir == ccl::DirWest && dir == ccl::DirSouth))
                        putTile(ccl::TileIce_SE, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                    else if ((m_lastDir == ccl::DirNorth && dir == ccl::DirWest)
                             || (m_lastDir == ccl::DirEast && dir == ccl::DirSouth))
                        putTile(ccl::TileIce_SW, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                    else if ((m_lastDir == ccl::DirSouth && dir == ccl::DirEast)
                             || (m_lastDir == ccl::DirWest && dir == ccl::DirNorth))
                        putTile(ccl::TileIce_NE, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                    else if ((m_lastDir == ccl::DirSouth && dir == ccl::DirWest)
                             || (m_lastDir == ccl::DirEast && dir == ccl::DirNorth))
                        putTile(ccl::TileIce_NW, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                    else
                        putTile(ccl::TileIce, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                } else {
                    // Any other directional tile
                    putTile(dirtile, posX, posY, select_layer(event->modifiers()));
                    putTile(dirtile, m_origin.x(), m_origin.y(), select_layer(event->modifiers()));
                }
                m_origin = m_current;
                m_lastDir = dir;
            } else {
                putTile(curtile, posX, posY, select_layer(event->modifiers()));
                m_origin = m_current;
                m_lastDir = ccl::DirInvalid;
            }
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

    if (m_levelData->map().getBG(posX, posY) == 0) {
        emit mouseInfo(QStringLiteral("(%1, %2): %3").arg(posX).arg(posY)
                       .arg(CCETileset::TileName(m_levelData->map().getFG(posX, posY))));
    } else {
        emit mouseInfo(QStringLiteral("(%1, %2): %3 / %4").arg(posX).arg(posY)
                       .arg(CCETileset::TileName(m_levelData->map().getFG(posX, posY)))
                       .arg(CCETileset::TileName(m_levelData->map().getBG(posX, posY))));
    }

    QString tipText;

    m_hilights.clear();
    for (const auto& trap_iter : m_levelData->traps()) {
        if (trap_iter.button.X == posX && trap_iter.button.Y == posY) {
            if (isValidPoint(trap_iter.trap))
                m_hilights << QPoint(trap_iter.trap.X, trap_iter.trap.Y);
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Trap: (%1, %2)")
                       .arg(trap_iter.trap.X).arg(trap_iter.trap.Y);
        }
        if (trap_iter.trap.X == posX && trap_iter.trap.Y == posY) {
            if (isValidPoint(trap_iter.button))
                m_hilights << QPoint(trap_iter.button.X, trap_iter.button.Y);
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Button: (%1, %2)")
                       .arg(trap_iter.button.X).arg(trap_iter.button.Y);
        }
    }
    for (const auto& clone_iter : m_levelData->clones()) {
        if (clone_iter.button.X == posX && clone_iter.button.Y == posY) {
            if (isValidPoint(clone_iter.clone))
                m_hilights << QPoint(clone_iter.clone.X, clone_iter.clone.Y);
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Cloner: (%1, %2)")
                       .arg(clone_iter.clone.X).arg(clone_iter.clone.Y);
        }
        if (clone_iter.clone.X == posX && clone_iter.clone.Y == posY) {
            if (isValidPoint(clone_iter.button))
                m_hilights << QPoint(clone_iter.button.X, clone_iter.button.Y);
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Button: (%1, %2)")
                       .arg(clone_iter.button.X).arg(clone_iter.button.Y);
        }
    }

    if (tipText.isEmpty() && (m_levelData->map().getFG(posX, posY) == ccl::TileTrap
        || m_levelData->map().getFG(posX, posY) == ccl::TileTrapButton
        || m_levelData->map().getFG(posX, posY) == ccl::TileCloner
        || m_levelData->map().getFG(posX, posY) == ccl::TileCloneButton
        || m_levelData->map().getBG(posX, posY) == ccl::TileTrap
        || m_levelData->map().getBG(posX, posY) == ccl::TileTrapButton
        || m_levelData->map().getBG(posX, posY) == ccl::TileCloner
        || m_levelData->map().getBG(posX, posY) == ccl::TileCloneButton))
        tipText = tr("No connections");

    if (m_levelData->map().getFG(posX, posY) == ccl::TileTeleport
        || m_levelData->map().getBG(posX, posY) == ccl::TileTeleport) {
        // Scan for next teleport
        do {
            if (--posX < 0) {
                posX = 31;
                if (--posY < 0)
                    posY = 31;
            }
        } while (m_levelData->map().getFG(posX, posY) != ccl::TileTeleport
                 && m_levelData->map().getBG(posX, posY) != ccl::TileTeleport);
        m_hilights << QPoint(posX, posY);

        if (!tipText.isEmpty())
            tipText += QLatin1Char('\n');
        tipText += tr("Teleport to: (%1, %2)").arg(posX).arg(posY);
    }

    if (MONSTER_TILE(m_levelData->map().getFG(posX, posY))) {
        bool canMove = false;
        int moveIdx = 0;
        for (const auto& move_iter : m_levelData->moveList()) {
            ++moveIdx;
            if (move_iter.X == posX && move_iter.Y == posY) {
                if (!tipText.isEmpty())
                    tipText += QLatin1Char('\n');
                tipText += tr("Move order: %1").arg(moveIdx);
                canMove = true;
            }
        }
        if (!canMove) {
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Monster DOES NOT MOVE");
        }
    }

    if (m_levelData->map().getFG(posX, posY) == ccl::TileHint
            || m_levelData->map().getBG(posX, posY) == ccl::TileHint) {
        if (!tipText.isEmpty())
            tipText += QLatin1Char('\n');
        tipText += ccl::fromLatin1(m_levelData->hint());
    }

    setToolTip(tipText);
    update();
}

void EditorWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_levelData || !rect().contains(event->pos()))
        return;
    if (m_cachedButton != Qt::NoButton
        || (event->button() & (Qt::LeftButton | Qt ::MiddleButton | Qt::RightButton)) == 0)
        return;

    const int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    const int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    m_current = QPoint(-1, -1);
    m_cachedButton = event->button();
    m_levelEditCache->copyFrom(m_levelData);

    if ((m_cachedButton == Qt::LeftButton || m_cachedButton == Qt::RightButton)
            && m_drawMode >= DrawPencil && m_drawMode <= DrawPathMaker)
        emit editingStarted();

    if (m_drawMode != DrawSelect && event->button() != Qt::MiddleButton) {
        m_selectRect = QRect(-1, -1, -1, -1);
        emit hasSelection(false);
    }

    if (m_cachedButton == Qt::MiddleButton) {
        m_origin = QPoint(posX, posY);
    } else if (m_drawMode == DrawButtonConnect) {
        if (m_cachedButton == Qt::RightButton) {
            bool madeChange = false;
            emit editingStarted();
            std::list<ccl::Trap>::iterator trap_iter = m_levelData->traps().begin();
            while (trap_iter != m_levelData->traps().end()) {
                if ((trap_iter->button.X == posX && trap_iter->button.Y == posY)
                    || (trap_iter->trap.X == posX && trap_iter->trap.Y == posY)) {
                    trap_iter = m_levelData->traps().erase(trap_iter);
                    madeChange = true;
                } else {
                    ++trap_iter;
                }
            }

            std::list<ccl::Clone>::iterator clone_iter = m_levelData->clones().begin();
            while (clone_iter != m_levelData->clones().end()) {
                if ((clone_iter->button.X == posX && clone_iter->button.Y == posY)
                    || (clone_iter->clone.X == posX && clone_iter->clone.Y == posY)) {
                    clone_iter = m_levelData->clones().erase(clone_iter);
                    madeChange = true;
                } else {
                    ++clone_iter;
                }
            }
            if (madeChange)
                emit editingFinished();
            else
                emit editingCancelled();
            m_origin = QPoint(-1, -1);
        } else  if (m_origin == QPoint(-1, -1) && test_start_connect(m_levelData, QPoint(posX, posY))) {
            m_origin = QPoint(posX, posY);
        } else if (m_origin == m_current) {
            m_origin = QPoint(-1, -1);
        }
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

void EditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_levelData || !rect().contains(event->pos()))
        return;
    if (event->button() != m_cachedButton)
        return;

    bool resetOrigin = true;
    if (m_drawMode == DrawInspectTile) {
        emit tilePicked(m_origin.x(), m_origin.y());
    } else if (m_drawMode == DrawSelect || m_cachedButton == Qt::MiddleButton) {
        resetOrigin = false;
    } else if (m_drawMode == DrawFlood) {
        if (m_cachedButton == Qt::LeftButton)
            plot_flood(this, m_current, m_leftTile, select_layer(event->modifiers()));
        else if (m_cachedButton == Qt::RightButton)
            plot_flood(this, m_current, m_rightTile, select_layer(event->modifiers()));
    } else if (m_drawMode == DrawButtonConnect) {
        if (m_origin != m_current) {
            switch (test_connect(m_levelData, m_origin, m_current)) {
            case ConnTrap:
                emit editingStarted();
                m_levelData->trapConnect(m_origin.x(), m_origin.y(), m_current.x(), m_current.y());
                emit editingFinished();
                break;
            case ConnTrapRev:
                emit editingStarted();
                m_levelData->trapConnect(m_current.x(), m_current.y(), m_origin.x(), m_origin.y());
                emit editingFinished();
                break;
            case ConnClone:
                emit editingStarted();
                m_levelData->cloneConnect(m_origin.x(), m_origin.y(), m_current.x(), m_current.y());
                emit editingFinished();
                break;
            case ConnCloneRev:
                emit editingStarted();
                m_levelData->cloneConnect(m_current.x(), m_current.y(), m_origin.x(), m_origin.y());
                emit editingFinished();
                break;
            default:
                // Do nothing
                break;
            }
        } else {
            resetOrigin = false;
        }
    }

    if (resetOrigin)
        m_origin = QPoint(-1, -1);
    if ((m_cachedButton == Qt::LeftButton || m_cachedButton == Qt::RightButton)
            && m_drawMode >= DrawPencil && m_drawMode <= DrawPathMaker)
        emit editingFinished();

    update();
    m_cachedButton = Qt::NoButton;
    m_lastDir = ccl::DirInvalid;
}

void EditorWidget::setDrawMode(DrawMode mode)
{
    if (m_drawMode != mode) {
        m_drawMode = mode;
        m_origin = QPoint(-1, -1);
        m_selectRect = QRect(-1, -1, -1, -1);
        update();
        emit hasSelection(false);

        switch (m_drawMode) {
        case DrawButtonConnect:
            setCursor(QCursor(QPixmap(QStringLiteral(":/res/cur-button.png")), 4, 4));
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

void EditorWidget::putTile(tile_t tile, int x, int y, DrawLayer layer)
{
    const tile_t oldUpper = m_levelData->map().getFG(x, y);
    const tile_t oldLower = m_levelData->map().getBG(x, y);

    if (layer == LayTop) {
        if (oldUpper == tile)
            return;
        m_levelData->map().setFG(x, y, tile);
    } else if (layer == LayBottom) {
        if (oldLower == tile)
            return;
        m_levelData->map().setBG(x, y, tile);
    } else if (oldUpper == ccl::TileCloner && MOVING_TILE(tile)) {
        // Bury the cloner under a cloneable tile
        m_levelData->map().push(x, y, tile);
    } else if (tile == ccl::TileCloner && MOVING_TILE(oldUpper)) {
        m_levelData->map().setBG(x, y, tile);
    } else if (MOVING_TILE(oldUpper) && oldLower == ccl::TileCloner
               && MOVING_TILE(tile)) {
        // Only replace the top layer
        m_levelData->map().setFG(x, y, tile);
    } else if ((tile >= ccl::TileGlider_N && tile <= ccl::TileGlider_E)
               && oldUpper == ccl::TileWater) {
        m_levelData->map().push(x, y, tile);
    } else if ((oldUpper >= ccl::TileGlider_N && oldUpper <= ccl::TileGlider_E)
               && tile == ccl::TileWater) {
        m_levelData->map().setBG(x, y, tile);
    } else if ((tile >= ccl::TileFireball_N && tile <= ccl::TileFireball_E)
               && oldUpper == ccl::TileFire) {
        m_levelData->map().push(x, y, tile);
    } else if ((oldUpper >= ccl::TileFireball_N && oldUpper <= ccl::TileFireball_E)
               && tile == ccl::TileFire) {
        m_levelData->map().setBG(x, y, tile);
    } else if ((tile == ccl::TileBarrier_S && oldUpper == ccl::TileBarrier_E)
               || (tile == ccl::TileBarrier_E && oldUpper == ccl::TileBarrier_S)) {
        m_levelData->map().setFG(x, y, ccl::TileBarrier_SE);
    } else if ((tile == ccl::TileBarrier_S && oldLower == ccl::TileBarrier_E)
               || (tile == ccl::TileBarrier_E && oldLower == ccl::TileBarrier_S)) {
        m_levelData->map().setBG(x, y, ccl::TileBarrier_SE);
    } else if ((tile == ccl::TileBlock || tile == ccl::TileIceBlock)
               && (oldUpper == ccl::TileChip || oldUpper == ccl::TileBarrier_SE ||
                   (oldUpper >= ccl::TileBarrier_N && oldUpper <= ccl::TileBarrier_E) ||
                   oldUpper == ccl::TileExit || oldUpper == ccl::TileThief ||
                   (oldUpper >= ccl::TileKey_Blue && oldUpper <= ccl::TileForceBoots) ||
                   (oldUpper == ccl::TileFire && tile == ccl::TileBlock))) {
        m_levelData->map().push(x, y, tile);
    } else if ((oldUpper == ccl::TileBlock || oldUpper == ccl::TileIceBlock)
               && (tile == ccl::TileChip || tile == ccl::TileBarrier_SE ||
                   (tile >= ccl::TileBarrier_N && tile <= ccl::TileBarrier_E) ||
                   tile == ccl::TileExit || tile == ccl::TileThief ||
                   (tile >= ccl::TileKey_Blue && tile <= ccl::TileForceBoots) ||
                   (tile == ccl::TileFire && oldUpper == ccl::TileBlock))) {
        m_levelData->map().setBG(x, y, tile);
    } else if ((MASKED_TILE(tile) || tile == ccl::TileBlock || tile == ccl::TileIceBlock)
               && ((oldUpper >= ccl::TileBarrier_N && oldUpper <= ccl::TileBarrier_E) ||
                   oldUpper == ccl::TileBarrier_SE || oldUpper == ccl::TileCloneButton ||
                   oldUpper == ccl::TileTankButton || oldUpper == ccl::TileTrapButton ||
                   oldUpper == ccl::TileToggleButton || oldUpper == ccl::TileTrap ||
                   oldUpper == ccl::TileHint)) {
        m_levelData->map().push(x, y, tile);
    } else if ((MASKED_TILE(oldUpper) || oldUpper == ccl::TileBlock || oldUpper == ccl::TileIceBlock)
               && ((tile >= ccl::TileBarrier_N && tile <= ccl::TileBarrier_E) ||
                   tile == ccl::TileBarrier_SE || tile == ccl::TileCloneButton ||
                   tile == ccl::TileTankButton || tile == ccl::TileTrapButton ||
                   tile == ccl::TileToggleButton || tile == ccl::TileTrap ||
                   tile == ccl::TileHint)) {
        m_levelData->map().setBG(x, y, tile);
    } else if (MONSTER_TILE(tile)
               && (oldUpper == ccl::TileChip || oldUpper == ccl::TileIce ||
                  (oldUpper >= ccl::TileIce_SE && oldUpper <= ccl::TileIce_NE) ||
                  oldUpper == ccl::TileExit || oldUpper == ccl::TileThief ||
                  oldUpper == ccl::TileSocket || FORCE_TILE(oldUpper) ||
                  (oldUpper >= ccl::TileKey_Blue && oldUpper <= ccl::TileForceBoots))) {
        m_levelData->map().push(x, y, tile);
    } else if (MONSTER_TILE(oldUpper)
               && (tile == ccl::TileChip || tile == ccl::TileIce ||
                  (tile >= ccl::TileIce_SE && tile <= ccl::TileIce_NE) ||
                  tile == ccl::TileExit || tile == ccl::TileThief ||
                  tile == ccl::TileSocket || FORCE_TILE(tile) ||
                  (tile >= ccl::TileKey_Blue && tile <= ccl::TileForceBoots))) {
        m_levelData->map().setBG(x, y, tile);
    } else if (MASKED_TILE(tile) || tile == ccl::TileBlock || tile == ccl::TileIceBlock) {
        m_levelData->map().setFG(x, y, tile);
    } else if (tile == ccl::TileFloor) {
        m_levelData->map().pop(x, y);
    } else {
        m_levelData->map().setFG(x, y, tile);
        m_levelData->map().setBG(x, y, ccl::TileFloor);
    }

    // Clear or add monsters from replaced tiles into move list
    if ((MONSTER_TILE(oldUpper) && (m_levelData->map().getBG(x, y) == ccl::TileCloner))
        || !MONSTER_TILE(m_levelData->map().getFG(x, y))) {
        std::list<ccl::Point>::iterator move_iter = m_levelData->moveList().begin();
        while (move_iter != m_levelData->moveList().end()) {
            if (move_iter->X == x && move_iter->Y == y)
                move_iter = m_levelData->moveList().erase(move_iter);
            else
                ++move_iter;
        }
    } else if (MONSTER_TILE(m_levelData->map().getFG(x, y)) && !MONSTER_TILE(oldUpper)
               && m_levelData->map().getBG(x, y) != ccl::TileCloner) {
        if (m_levelData->moveList().size() < MAX_MOVERS)
            m_levelData->addMover(x, y);
    } else if (oldLower == ccl::TileCloner && m_levelData->map().getBG(x, y) != ccl::TileCloner
               &&  MONSTER_TILE(m_levelData->map().getFG(x, y))) {
        if (m_levelData->moveList().size() < MAX_MOVERS)
            m_levelData->addMover(x, y);
    }

    // Clear connections from replaced tiles
    std::list<ccl::Trap>::iterator trap_iter = m_levelData->traps().begin();
    while (trap_iter != m_levelData->traps().end()) {
        if (trap_iter->button.X == x && trap_iter->button.Y == y
                && m_levelData->map().getFG(x, y) != ccl::TileTrapButton
                && m_levelData->map().getBG(x, y) != ccl::TileTrapButton)
            trap_iter = m_levelData->traps().erase(trap_iter);
        else if (trap_iter->trap.X == x && trap_iter->trap.Y == y
                && m_levelData->map().getFG(x, y) != ccl::TileTrap
                && m_levelData->map().getBG(x, y) != ccl::TileTrap)
            trap_iter = m_levelData->traps().erase(trap_iter);
        else
            ++trap_iter;
    }

    std::list<ccl::Clone>::iterator clone_iter = m_levelData->clones().begin();
    while (clone_iter != m_levelData->clones().end()) {
        if (clone_iter->button.X == x && clone_iter->button.Y == y
                && m_levelData->map().getFG(x, y) != ccl::TileCloneButton
                && m_levelData->map().getBG(x, y) != ccl::TileCloneButton)
            clone_iter = m_levelData->clones().erase(clone_iter);
        else if (clone_iter->clone.X == x && clone_iter->clone.Y == y
                && m_levelData->map().getFG(x, y) != ccl::TileCloner
                && m_levelData->map().getBG(x, y) != ccl::TileCloner)
            clone_iter = m_levelData->clones().erase(clone_iter);
        else
            ++clone_iter;
    }

    dirtyBuffer();
}

void EditorWidget::setZoom(double factor)
{
    m_zoomFactor = factor;
    resize(sizeHint());
    dirtyBuffer();
}
