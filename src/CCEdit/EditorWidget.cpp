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
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

EditorWidget::EditorWidget(QWidget* parent)
            : QWidget(parent), m_tileset(0), m_levelData(0), m_leftTile(0),
              m_rightTile(0), m_drawMode(DrawPencil), m_paintFlags(0)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

void EditorWidget::setTileset(CCETileset* tileset)
{
    m_tileset = tileset;
    resize(sizeHint());
    update();
}

void EditorWidget::paintEvent(QPaintEvent* event)
{
    if (m_tileset == 0)
        return;

    QPainter painter(this);

    if (m_levelData == 0) {
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                m_tileset->draw(painter, x, y, ccl::TileFloor);
    } else {
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                m_tileset->draw(painter, x, y, m_levelData->map().getFG(x, y),
                                m_levelData->map().getBG(x, y));

        // Draw current buffer
        if (m_drawMode == DrawLine) {
            // TODO
        } else if (m_drawMode == DrawFill) {
            int lowY = std::min(m_origin.y(), m_current.y());
            int lowX = std::min(m_origin.x(), m_current.x());
            int highY = std::max(m_origin.y(), m_current.y());
            int highX = std::max(m_origin.x(), m_current.x());

            if ((m_paintFlags & PaintLeftTemp) != 0) {
                for (int y = lowY; y <= highY; ++y) {
                    for (int x = lowX; x <= highX; ++x) {
                        if ((m_paintFlags & PaintTempBury) != 0)
                            m_tileset->draw(painter, x, y, m_levelData->map().getFG(x, y),
                                            m_leftTile);
                        else
                            m_tileset->draw(painter, x, y, m_leftTile,
                                            m_levelData->map().getBG(x, y));
                    }
                }
            } else if ((m_paintFlags & PaintRightTemp) != 0) {
                for (int y = lowY; y <= highY; ++y) {
                    for (int x = lowX; x <= highX; ++x) {
                        if ((m_paintFlags & PaintTempBury) != 0)
                            m_tileset->draw(painter, x, y, m_levelData->map().getFG(x, y),
                                            m_rightTile);
                        else
                            m_tileset->draw(painter, x, y, m_rightTile,
                                            m_levelData->map().getBG(x, y));
                    }
                }
            }
        }

        if ((m_paintFlags & ShowMovement) != 0) {
            painter.setPen(QColor(0, 0, 255));
            std::list<ccl::Point>::const_iterator move_iter;
            for (move_iter = m_levelData->moveList().begin(); move_iter != m_levelData->moveList().end(); ++move_iter)
                painter.drawRect(move_iter->X * m_tileset->size(), move_iter->Y * m_tileset->size(),
                                 m_tileset->size() - 1, m_tileset->size() - 1);
        }

        if ((m_paintFlags & ShowPlayer) != 0) {
            bool playerFound = false;
            painter.setPen(QColor(0, 255, 0));
            for (int y = 31; !playerFound && y >= 0; --y) {
                for (int x = 31; !playerFound && x >= 0; --x) {
                    if (m_levelData->map().getFG(x, y) >= ccl::TilePlayer_N
                        && m_levelData->map().getFG(x, y) <= ccl::TilePlayer_E) {
                        painter.drawRect(x * m_tileset->size(), y * m_tileset->size(),
                                        m_tileset->size() - 1, m_tileset->size() - 1);
                        playerFound = true;
                    }
                }
            }
            if (!playerFound) {
                painter.setPen(QColor(255, 127, 0));
                painter.drawRect(1 * m_tileset->size(), 1 * m_tileset->size(),
                                m_tileset->size() - 1, m_tileset->size() - 1);
            }
        }

        // Hilight context-sensitive objects
        painter.setPen(QColor(255, 0, 0));
        foreach (QPoint hi, m_hilights)
            painter.drawRect(hi.x() * m_tileset->size(), hi.y() * m_tileset->size(),
                             m_tileset->size() - 1, m_tileset->size() - 1);
    }
}

void EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_tileset == 0 || m_levelData == 0)
        return;

    int posX = event->x() / m_tileset->size();
    int posY = event->y() / m_tileset->size();

    m_paintFlags &= ~(PaintLeftTemp | PaintRightTemp | PaintTempBury);
    if (m_drawMode == DrawPencil) {
        if ((event->buttons() & Qt::LeftButton) != 0)
            putTile(m_leftTile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
        else if ((event->buttons() & Qt::RightButton) != 0)
            putTile(m_rightTile, posX, posY, (event->modifiers() & Qt::ShiftModifier) != 0);
    } else if (m_drawMode == DrawLine || m_drawMode == DrawFill) {
        m_current = QPoint(posX, posY);
        if ((event->buttons() & Qt::LeftButton) != 0)
            m_paintFlags |= PaintLeftTemp;
        else if ((event->buttons() & Qt::RightButton) != 0)
            m_paintFlags |= PaintRightTemp;
        if ((event->modifiers() & Qt::ShiftModifier) != 0)
            m_paintFlags |= PaintTempBury;
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

    if ((m_paintFlags & ShowButtons) != 0) {
        std::list<ccl::Trap>::const_iterator trap_iter;
        for (trap_iter = m_levelData->traps().begin(); trap_iter != m_levelData->traps().end(); ++trap_iter) {
            if (trap_iter->button.X == posX && trap_iter->button.Y == posY)
                m_hilights << QPoint(trap_iter->trap.X, trap_iter->trap.Y);
            if (trap_iter->trap.X == posX && trap_iter->trap.Y == posY)
                m_hilights << QPoint(trap_iter->button.X, trap_iter->button.Y);
        }
        std::list<ccl::Clone>::const_iterator clone_iter;
        for (clone_iter = m_levelData->clones().begin(); clone_iter != m_levelData->clones().end(); ++clone_iter) {
            if (clone_iter->button.X == posX && clone_iter->button.Y == posY)
                m_hilights << QPoint(clone_iter->clone.X, clone_iter->clone.Y);
            if (clone_iter->clone.X == posX && clone_iter->clone.Y == posY)
                m_hilights << QPoint(clone_iter->button.X, clone_iter->button.Y);
        }
    }
    if (((m_paintFlags & ShowTeleport) != 0)
        && (m_levelData->map().getFG(posX, posY) == ccl::TileTeleport
            || m_levelData->map().getBG(posX, posY) == ccl::TileTeleport)) {
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
    int posX = event->x() / m_tileset->size();
    int posY = event->y() / m_tileset->size();
    m_origin = QPoint(posX, posY);

    mouseMoveEvent(event);
}

void EditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_tileset == 0 || m_levelData == 0)
        return;

    if (m_drawMode == DrawLine) {
        // TODO: Finish line
    } else if (m_drawMode == DrawFill) {
        int lowY = std::min(m_origin.y(), m_current.y());
        int lowX = std::min(m_origin.x(), m_current.x());
        int highY = std::max(m_origin.y(), m_current.y());
        int highX = std::max(m_origin.x(), m_current.x());

        if (event->button() == Qt::LeftButton) {
            for (int y = lowY; y <= highY; ++y)
                for (int x = lowX; x <= highX; ++x)
                    putTile(m_leftTile, x, y, (event->modifiers() & Qt::ShiftModifier) != 0);
        } else if (event->button() == Qt::RightButton) {
            for (int y = lowY; y <= highY; ++y)
                for (int x = lowX; x <= highX; ++x)
                    putTile(m_rightTile, x, y, (event->modifiers() & Qt::ShiftModifier) != 0);
        }
        update();
    }

    //TODO: Emit save point
}

void EditorWidget::putTile(tile_t tile, int x, int y, bool bury)
{
    if (bury) {
        // No fancy smart stuff when sending to bottom layer
        m_levelData->map().setBG(x, y, tile);
        return;
    }

    tile_t oldUpper = m_levelData->map().getFG(x, y);
    tile_t oldLower = m_levelData->map().getBG(x, y);

    if (oldUpper == ccl::TileCloner
        && ((tile >= ccl::MONSTER_FIRST && tile <= ccl::MONSTER_LAST)
            || (tile >= ccl::TileBlock_N && tile <= ccl::TileBlock_E))) {
        // Bury the cloner under a cloneable tile
        m_levelData->map().push(x, y, tile);
    } else if (oldLower == ccl::TileCloner
        && ((oldUpper >= ccl::MONSTER_FIRST && oldUpper <= ccl::MONSTER_LAST)
            || (oldUpper >= ccl::TileBlock_N && oldUpper <= ccl::TileBlock_E))
        && ((tile < ccl::MONSTER_FIRST || tile > ccl::MONSTER_LAST)
            && (tile < ccl::TileBlock_N || tile > ccl::TileBlock_E))) {
        // Clear both the cloner and the cloneable before replacing it
        // with something non-cloneable
        m_levelData->map().setFG(x, y, tile);
        m_levelData->map().setBG(x, y, ccl::TileFloor);
    } else {
        m_levelData->map().setFG(x, y, tile);
    }

    // Clear or add monsters from replaced tiles into move list
    if ((oldUpper >= ccl::MONSTER_FIRST && oldUpper <= ccl::MONSTER_LAST)
        && (tile < ccl::MONSTER_FIRST || tile > ccl::MONSTER_LAST)) {
        std::list<ccl::Point>::iterator move_iter = m_levelData->moveList().begin();
        while (move_iter != m_levelData->moveList().end()) {
            if (move_iter->X == x && move_iter->Y == y)
                move_iter = m_levelData->moveList().erase(move_iter);
            else
                ++move_iter;
        }
    } else if ((tile >= ccl::MONSTER_FIRST && tile <= ccl::MONSTER_LAST)
               && (oldUpper < ccl::MONSTER_FIRST || oldUpper > ccl::MONSTER_LAST)) {
        if (m_levelData->moveList().size() < 127) {
            ccl::Point mover;
            mover.X = x;
            mover.Y = y;
            m_levelData->moveList().push_back(mover);
        }
    }
}
