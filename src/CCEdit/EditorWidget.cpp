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
#include "../GameLogic.h"

enum PlotMethod { PlotPreview, PlotDraw };

static void plot_box(EditorWidget* self, QPoint from, QPoint to, PlotMethod method,
                     QPainter* previewPainter, tile_t drawTile = 0, bool drawBury = false)
{
    if (from == QPoint(-1, -1))
        return;

    int lowY = std::min(from.y(), to.y());
    int lowX = std::min(from.x(), to.x());
    int highY = std::max(from.y(), to.y());
    int highX = std::max(from.x(), to.x());

    if (method == PlotPreview) {
        for (int y = lowY; y <= highY; ++y)
            for (int x = lowX; x <= highX; ++x)
                self->viewTile(*previewPainter, x, y);
    } else if (method == PlotDraw) {
        for (int y = lowY; y <= highY; ++y)
            for (int x = lowX; x <= highX; ++x)
                self->putTile(drawTile, x, y, drawBury);
    }
}

static void plot_line(EditorWidget* self, QPoint from, QPoint to, PlotMethod method,
                      QPainter* previewPainter, tile_t drawTile = 0, bool drawBury = false)
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
        if (method == PlotPreview)
            self->viewTile(*previewPainter, steep ? y : x, steep ? x : y);
        else if (method == PlotDraw)
            self->putTile(drawTile, steep ? y : x, steep ? x : y, drawBury);
        err -= dY;
        if (err < 0) {
            y += ystep;
            err += dX;
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

static ccl::Direction calc_dir(QPoint from, QPoint to)
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


EditorWidget::EditorWidget(QWidget* parent)
            : QWidget(parent), m_tileset(0), m_levelData(0), m_leftTile(0),
              m_rightTile(0), m_drawMode(DrawPencil), m_paintFlags(0),
              m_cachedButton(Qt::NoButton),  m_numbers(":/res/numbers.png"),
              m_lastDir(ccl::DirInvalid), m_zoomFactor(1.0)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void EditorWidget::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    m_tileBuffer = QPixmap(32 * m_tileset->size(), 32 * m_tileset->size());
    dirtyBuffer();
    resize(sizeHint());
    update();
}

void EditorWidget::setLevelData(ccl::LevelData* level)
{
    level->ref();
    m_levelData->unref();
    m_levelData = level;

    m_origin = QPoint(-1, -1);
    m_selectRect = QRect(-1, -1, -1, -1);
    dirtyBuffer();
    update();

    m_history.clear();
    emit canUndo(false);
    emit canRedo(false);
    emit hasSelection(false);
}

void EditorWidget::renderTileBuffer()
{
    QPainter tilePainter(&m_tileBuffer);
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            m_tileset->draw(tilePainter, x, y, m_levelData->map().getFG(x, y),
                            m_levelData->map().getBG(x, y));

    // Draw current buffer
    if (m_drawMode == DrawLine && (m_paintFlags & PaintOverlayMask) != 0)
        plot_line(this, m_origin, m_current, PlotPreview, &tilePainter);
    else if (m_drawMode == DrawFill && (m_paintFlags & PaintOverlayMask) != 0)
        plot_box(this, m_origin, m_current, PlotPreview, &tilePainter);

    dirtyBuffer();
}

void EditorWidget::paintEvent(QPaintEvent*)
{
    if (m_tileset == 0 || m_levelData == 0)
        return;

    QPainter painter(this);
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
        std::list<ccl::Point>::const_iterator move_iter;
        int num = 0;
        for (move_iter = m_levelData->moveList().begin();
             move_iter != m_levelData->moveList().end(); ++move_iter) {
            if (!isValidPoint(*move_iter))
                continue;
            painter.drawPixmap((move_iter->X + 1) * m_tileset->size() * m_zoomFactor - 16,
                               (move_iter->Y + 1) * m_tileset->size() * m_zoomFactor - 10,
                               m_numbers, 0, num++ * 10, 16, 10);
        }
    }

    if ((m_paintFlags & ShowMovePaths) != 0) {
        painter.setPen(QColor(0, 0, 255));
        std::list<ccl::Point>::const_iterator move_iter;
        for (move_iter = m_levelData->moveList().begin();
             move_iter != m_levelData->moveList().end(); ++move_iter) {
            ccl::Point from = *move_iter;
            if (!isValidPoint(from))
                continue;

            uint8_t looked[32*32];
            memset(looked, 0, sizeof(looked));
            tile_t tile = m_levelData->map().getFG(from.X, from.Y);
            tile_t tileN = tile & 0xFC;
            ccl::MoveState move = ccl::CheckMove(m_levelData, tile, from.X, from.Y);

            if (tileN == ccl::TileTeeth_N || tileN == ccl::TileWalker_N
                || tileN == ccl::TileBlob_N)
                continue;

            do {
                looked[(from.Y*32)+from.X] |= 1 << (tile & 0x03);
                if ((move & ccl::MoveDirMask) < ccl::MoveBlocked) {
                    if ((move & ccl::MoveTrapped) != 0)
                        break;
                    ccl::Point to = ccl::AdvanceCreature(tile, from, move);
                    painter.drawLine(calcTileCenter(from.X, from.Y),
                                     calcTileCenter(to.X, to.Y));
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
        bool playerFound = false;
        painter.setPen(QColor(255, 127, 0));
        for (int y = 31; !playerFound && y >= 0; --y) {
            for (int x = 31; !playerFound && x >= 0; --x) {
                if (m_levelData->map().getFG(x, y) >= ccl::TilePlayer_N
                    && m_levelData->map().getFG(x, y) <= ccl::TilePlayer_E) {
                    painter.drawRect(calcTileRect(x, y));
                    playerFound = true;
                }
            }
        }
        if (!playerFound)
            painter.drawRect(calcTileRect(0, 0));
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
        std::list<ccl::Trap>::const_iterator trap_iter;
        for (trap_iter = m_levelData->traps().begin(); trap_iter != m_levelData->traps().end(); ++trap_iter) {
            if (!isValidPoint(trap_iter->button) || !isValidPoint(trap_iter->trap))
                continue;
            painter.drawLine(calcTileCenter(trap_iter->button.X, trap_iter->button.Y),
                             calcTileCenter(trap_iter->trap.X, trap_iter->trap.Y));
        }
        std::list<ccl::Clone>::const_iterator clone_iter;
        for (clone_iter = m_levelData->clones().begin(); clone_iter != m_levelData->clones().end(); ++clone_iter) {
            if (!isValidPoint(clone_iter->button) || !isValidPoint(clone_iter->clone))
                continue;
            painter.drawLine(calcTileCenter(clone_iter->button.X, clone_iter->button.Y),
                             calcTileCenter(clone_iter->clone.X, clone_iter->clone.Y));
        }
    }

    if (m_drawMode == DrawButtonConnect && m_origin != QPoint(-1, -1)
        && m_selectRect == QRect(-1, -1, -1, -1)) {
        if (test_connect(m_levelData, m_origin, m_current) != ConnNone)
            painter.setPen(QColor(255, 0, 0));
        else
            painter.setPen(QColor(127, 127, 127));
        painter.drawLine(calcTileCenter(m_origin.x(), m_origin.y()),
                         calcTileCenter(m_current.x(), m_current.y()));
    }

    // Hilight context-sensitive objects
    painter.setPen(QColor(255, 0, 0));
    foreach (QPoint hi, m_hilights)
        painter.drawRect(calcTileRect(hi.x(), hi.y()));
}

void EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_tileset == 0 || m_levelData == 0 || !rect().contains(event->pos()))
        return;

    int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    if (m_current == QPoint(posX, posY) && !m_cacheDirty)
        return;
    m_current = QPoint(posX, posY);

    m_paintFlags &= ~(PaintLeftTemp | PaintRightTemp | PaintTempBury);
    if (m_cachedButton == Qt::MidButton && m_origin != QPoint(-1, -1)) {
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
            putTile(curtile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
        } else if (m_drawMode == DrawLine || m_drawMode == DrawFill) {
            if (m_cachedButton == Qt::LeftButton)
                m_paintFlags |= PaintLeftTemp;
            else if (m_cachedButton == Qt::RightButton)
                m_paintFlags |= PaintRightTemp;
            if ((event->modifiers() & Qt::ShiftModifier) != 0)
                m_paintFlags |= PaintTempBury;
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
                        putTile(ccl::TileIce, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
                    else
                        putTile(dirtile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
                    oldTile = (event->modifiers() & Qt::ShiftModifier) == 0
                            ? m_levelData->map().getFG(m_origin.x(), m_origin.y())
                            : m_levelData->map().getBG(m_origin.x(), m_origin.y());
                    if (oldTile != ccl::TileIce)
                        putTile(dirtile, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                } else if (curtile == ccl::TileIce && m_lastDir != ccl::DirInvalid) {
                    // Add curves to ice
                    putTile(dirtile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
                    if ((m_lastDir == ccl::DirNorth && dir == ccl::DirEast)
                        || (m_lastDir == ccl::DirWest && dir == ccl::DirSouth))
                        putTile(ccl::TileIce_SE, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                    else if ((m_lastDir == ccl::DirNorth && dir == ccl::DirWest)
                             || (m_lastDir == ccl::DirEast && dir == ccl::DirSouth))
                        putTile(ccl::TileIce_SW, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                    else if ((m_lastDir == ccl::DirSouth && dir == ccl::DirEast)
                             || (m_lastDir == ccl::DirWest && dir == ccl::DirNorth))
                        putTile(ccl::TileIce_NE, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                    else if ((m_lastDir == ccl::DirSouth && dir == ccl::DirWest)
                             || (m_lastDir == ccl::DirEast && dir == ccl::DirNorth))
                        putTile(ccl::TileIce_NW, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                    else
                        putTile(ccl::TileIce, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                } else {
                    // Any other directional tile
                    putTile(dirtile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
                    putTile(dirtile, m_origin.x(), m_origin.y(), (event->modifiers() & Qt::ShiftModifier) != 0);
                }
                m_origin = m_current;
                m_lastDir = dir;
            } else {
                putTile(curtile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
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
        emit mouseInfo(QString("(%1, %2): %3").arg(posX).arg(posY)
                       .arg(CCETileset::TileName(m_levelData->map().getFG(posX, posY))));
    } else {
        emit mouseInfo(QString("(%1, %2): %3 / %4").arg(posX).arg(posY)
                       .arg(CCETileset::TileName(m_levelData->map().getFG(posX, posY)))
                       .arg(CCETileset::TileName(m_levelData->map().getBG(posX, posY))));
    }

    m_hilights.clear();
    std::list<ccl::Trap>::const_iterator trap_iter;
    for (trap_iter = m_levelData->traps().begin(); trap_iter != m_levelData->traps().end(); ++trap_iter) {
        if (trap_iter->button.X == posX && trap_iter->button.Y == posY
            && isValidPoint(trap_iter->trap))
            m_hilights << QPoint(trap_iter->trap.X, trap_iter->trap.Y);
        if (trap_iter->trap.X == posX && trap_iter->trap.Y == posY
            && isValidPoint(trap_iter->button))
            m_hilights << QPoint(trap_iter->button.X, trap_iter->button.Y);
    }
    std::list<ccl::Clone>::const_iterator clone_iter;
    for (clone_iter = m_levelData->clones().begin(); clone_iter != m_levelData->clones().end(); ++clone_iter) {
        if (clone_iter->button.X == posX && clone_iter->button.Y == posY
            && isValidPoint(clone_iter->clone))
            m_hilights << QPoint(clone_iter->clone.X, clone_iter->clone.Y);
        if (clone_iter->clone.X == posX && clone_iter->clone.Y == posY
            && isValidPoint(clone_iter->button))
            m_hilights << QPoint(clone_iter->button.X, clone_iter->button.Y);
    }

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
    }

    update();
}

void EditorWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_cachedButton != Qt::NoButton
        || (event->button() & (Qt::LeftButton | Qt ::MidButton | Qt::RightButton)) == 0)
        return;
    if (m_tileset == 0 || m_levelData == 0)
        return;

    int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    m_current = QPoint(-1, -1);
    m_cachedButton = event->button();

    if ((m_cachedButton == Qt::LeftButton || m_cachedButton == Qt::RightButton)
        && (m_drawMode == DrawPencil || m_drawMode == DrawLine || m_drawMode == DrawFill
            || m_drawMode == DrawPathMaker))
        beginEdit(CCEHistoryNode::HistDraw);

    if (m_drawMode != DrawSelect && event->button() != Qt::MidButton) {
        m_selectRect = QRect(-1, -1, -1, -1);
        emit hasSelection(false);
    }

    if (m_cachedButton == Qt::MidButton) {
        m_origin = QPoint(posX, posY);
    } else if (m_drawMode == DrawButtonConnect) {
        if (m_cachedButton == Qt::RightButton) {
            bool madeChange = false;
            beginEdit(CCEHistoryNode::HistDisconnect);
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
                endEdit();
            else
                cancelEdit();
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
    if (event->button() != m_cachedButton)
        return;
    if (m_tileset == 0 || m_levelData == 0)
        return;

    bool resetOrigin = true;
    if (m_drawMode == DrawSelect || m_cachedButton == Qt::MidButton) {
        resetOrigin = false;
    } else if (m_drawMode == DrawLine) {
        if (m_cachedButton == Qt::LeftButton)
            plot_line(this, m_origin, m_current, PlotDraw, 0, m_leftTile,
                      (event->modifiers() & Qt::ShiftModifier) != 0);
        else if (m_cachedButton == Qt::RightButton)
            plot_line(this, m_origin, m_current, PlotDraw, 0, m_rightTile,
                      (event->modifiers() & Qt::ShiftModifier) != 0);
    } else if (m_drawMode == DrawFill) {
        if (m_cachedButton == Qt::LeftButton)
            plot_box(this, m_origin, m_current, PlotDraw, 0, m_leftTile,
                     (event->modifiers() & Qt::ShiftModifier) != 0);
        else if (m_cachedButton == Qt::RightButton)
            plot_box(this, m_origin, m_current, PlotDraw, 0, m_rightTile,
                     (event->modifiers() & Qt::ShiftModifier) != 0);
    } else if (m_drawMode == DrawButtonConnect) {
        if (m_origin != m_current) {
            switch (test_connect(m_levelData, m_origin, m_current)) {
            case ConnTrap:
                beginEdit(CCEHistoryNode::HistConnect);
                m_levelData->trapConnect(m_origin.x(), m_origin.y(), m_current.x(), m_current.y());
                endEdit();
                break;
            case ConnTrapRev:
                beginEdit(CCEHistoryNode::HistConnect);
                m_levelData->trapConnect(m_current.x(), m_current.y(), m_origin.x(), m_origin.y());
                endEdit();
                break;
            case ConnClone:
                beginEdit(CCEHistoryNode::HistConnect);
                m_levelData->cloneConnect(m_origin.x(), m_origin.y(), m_current.x(), m_current.y());
                endEdit();
                break;
            case ConnCloneRev:
                beginEdit(CCEHistoryNode::HistConnect);
                m_levelData->cloneConnect(m_current.x(), m_current.y(), m_origin.x(), m_origin.y());
                endEdit();
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
        && (m_drawMode == DrawPencil || m_drawMode == DrawLine || m_drawMode == DrawFill
            || m_drawMode == DrawPathMaker))
        endEdit();

    update();
    m_cachedButton = Qt::NoButton;
}

void EditorWidget::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;
    m_origin = QPoint(-1, -1);
    m_selectRect = QRect(-1, -1, -1, -1);
    update();
    emit hasSelection(false);
}

void EditorWidget::viewTile(QPainter& painter, int x, int y)
{
    if ((m_paintFlags & PaintLeftTemp) != 0) {
        if ((m_paintFlags & PaintTempBury) != 0)
            m_tileset->draw(painter, x, y, m_levelData->map().getFG(x, y),
                            m_leftTile);
        else
            m_tileset->draw(painter, x, y, m_leftTile,
                            m_levelData->map().getBG(x, y));
    } else if ((m_paintFlags & PaintRightTemp) != 0) {
        if ((m_paintFlags & PaintTempBury) != 0)
            m_tileset->draw(painter, x, y, m_levelData->map().getFG(x, y),
                            m_rightTile);
        else
            m_tileset->draw(painter, x, y, m_rightTile,
                            m_levelData->map().getBG(x, y));
    }
    dirtyBuffer();
}

void EditorWidget::putTile(tile_t tile, int x, int y, bool bury)
{
    tile_t oldUpper = m_levelData->map().getFG(x, y);
    tile_t oldLower = m_levelData->map().getBG(x, y);

    if ((bury && oldLower == tile) || (!bury && oldUpper == tile))
        return;

    if (bury) {
        m_levelData->map().setBG(x, y, tile);
    } else if (oldUpper == ccl::TileCloner && MOVING_TILE(tile)) {
        // Bury the cloner under a cloneable tile
        m_levelData->map().push(x, y, tile);
    } else if (tile == ccl::TileCloner && MOVING_TILE(oldUpper)) {
        m_levelData->map().setBG(x, y, tile);
    } else if (oldLower == ccl::TileCloner && MOVING_TILE(oldUpper)
               && !MOVING_TILE(tile)) {
        // Clear both the cloner and the cloneable before replacing it
        // with something non-cloneable
        m_levelData->map().setFG(x, y, tile);
        m_levelData->map().setBG(x, y, ccl::TileFloor);
    } else {
        m_levelData->map().setFG(x, y, tile);
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

void EditorWidget::undo()
{
    ccl::LevelData* data = m_history.undo();
    m_levelData->map().copyFrom(data->map());
    m_levelData->traps() = data->traps();
    m_levelData->clones() = data->clones();
    m_levelData->moveList() = data->moveList();
    dirtyBuffer();
    update();
    updateUndoStatus();
}

void EditorWidget::redo()
{
    ccl::LevelData* data = m_history.redo();
    m_levelData->map().copyFrom(data->map());
    m_levelData->traps() = data->traps();
    m_levelData->clones() = data->clones();
    m_levelData->moveList() = data->moveList();
    dirtyBuffer();
    update();
    updateUndoStatus();
}

void EditorWidget::setZoom(double factor)
{
    m_zoomFactor = factor;
    dirtyBuffer();
    resize(sizeHint());
    update();
}
