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

#include <QUndoStack>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

CC2EditorWidget::CC2EditorWidget(QWidget* parent)
    : QWidget(parent), m_tileset(), m_map(), m_drawMode(DrawPencil),
      m_paintFlags(), m_cachedButton(Qt::NoButton), m_undoCommand(),
      m_zoomFactor(1.0)
{
    m_undoStack = new QUndoStack(this);
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, &CC2EditorWidget::canUndoChanged);
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, &CC2EditorWidget::canRedoChanged);
    connect(m_undoStack, &QUndoStack::cleanChanged, this, &CC2EditorWidget::cleanChanged);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);

    m_selectRect = QRect(-1, -1, -1, -1);
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
    resize(sizeHint());

    m_undoStack->clear();
    dirtyBuffer();
    update();

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
    m_drawMode = mode;
    m_origin = QPoint(-1, -1);
    m_selectRect = QRect(-1, -1, -1, -1);
    update();
    emit hasSelection(false);
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
        m_undoStack->push(m_undoCommand);
        m_undoCommand = nullptr;
    }
    dirtyBuffer();
    update();
}

void CC2EditorWidget::cancelEdit()
{
    if (m_undoCommand->leave(nullptr)) {
        delete m_undoCommand;
        m_undoCommand = nullptr;
    }
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

void CC2EditorWidget::renderTo(QPainter& painter)
{
    if (m_cacheDirty) {
        renderTileBuffer();
        m_tileCache = m_tileBuffer.scaled(renderSize());
        m_cacheDirty = false;
    }
    painter.drawPixmap(0, 0, m_tileCache);

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
    foreach (QPoint hi, m_hilights)
        painter.drawRect(calcTileRect(hi.x(), hi.y()));
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
    } while (!map.haveTile(sx, sy, type));

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
    } while (!map.haveTile(sx, sy, type));

    return QPoint(sx, sy);
}

static QList<QPoint> scanForAll(cc2::Tile::Type type, const cc2::MapData& map)
{
    QList<QPoint> matches;
    for (int sy = 0; sy < map.height(); ++sy) {
        for (int sx = 0; sx < map.width(); ++sx) {
            if (map.haveTile(sx, sy, type))
                matches << QPoint(sx, sy);
        }
    }
    return matches;
}

static QPoint scanForControl(const QVector<cc2::Tile::Type>& controlTypes,
                             int x, int y, const cc2::MapData& map)
{
    int sx = x, sy = y;
    for ( ;; ) {
        if (++sx >= map.width()) {
            sx = 0;
            if (++sy >= map.height())
                sy = 0;
        }
        for (cc2::Tile::Type type : controlTypes) {
            if (map.haveTile(sx, sy, type))
                return QPoint(sx, sy);
        }
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return QPoint(-1, -1);
        }
    }

    Q_UNREACHABLE();
}

static QList<QPoint> scanForButtons(cc2::Tile::Type buttonType,
                                    const QVector<cc2::Tile::Type>& controlTypes,
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
        if (map.haveTile(sx, sy, buttonType))
            matches << QPoint(sx, sy);
        for (cc2::Tile::Type type : controlTypes) {
            if (map.haveTile(sx, sy, type))
                return matches;
        }
        if (sx == x && sy == y) {
            // We've wrapped around to our starting location
            return matches;
        }
    }

    Q_UNREACHABLE();
}

static QPoint diamondClosest(const QVector<cc2::Tile::Type>& controlTypes,
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
            for (cc2::Tile::Type type : controlTypes) {
                if (map.haveTile(sx, sy, type))
                    return QPoint(sx, sy);
            }
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

void CC2EditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;

    const int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    const int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    if (m_current == QPoint(posX, posY) && !m_cacheDirty)
        return;
    m_current = QPoint(posX, posY);

    const cc2::MapData& map = m_map->mapData();
    const cc2::Tile* tile = &map.tile(posX, posY);
    QString info = QString("(%1, %2): %3").arg(posX).arg(posY).arg(CC2ETileset::getName(tile));
    while (tile->haveLower() && tile->lower()->type() != cc2::Tile::Floor) {
        tile = tile->lower();
        info += tr(" / %1").arg(CC2ETileset::getName(tile));
    }
    emit mouseInfo(info);

    QString tipText;
    m_hilights.clear();
    if (map.haveTile(posX, posY, cc2::Tile::Teleport_Blue)) {
        // TODO: Handle wires
        QPoint nextTeleport = scanForR(cc2::Tile::Teleport_Blue, posX, posY, map);
        if (nextTeleport != QPoint(-1, -1)) {
            m_hilights << nextTeleport;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::Teleport_Red)) {
        QPoint nextTeleport = scanForF(cc2::Tile::Teleport_Red, posX, posY, map);
        if (nextTeleport != QPoint(-1, -1)) {
            m_hilights << nextTeleport;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::Teleport_Green)) {
        QList<QPoint> teleports = scanForAll(cc2::Tile::Teleport_Green, map);
        for (const QPoint& teleport : teleports) {
            // Exclude the teleport under the cursor from the highlight list
            if (teleport.x() == posX && teleport.y() == posY)
                continue;
            m_hilights << teleport;
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::Teleport_Yellow)) {
        QPoint nextTeleport = scanForR(cc2::Tile::Teleport_Yellow, posX, posY, map);
        if (nextTeleport != QPoint(-1, -1)) {
            m_hilights << nextTeleport;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Teleport to: (%1, %2)").arg(nextTeleport.x()).arg(nextTeleport.y());
        }
    }

    if (map.haveTile(posX, posY, cc2::Tile::CloneButton)) {
        QPoint cloner = scanForControl({cc2::Tile::Cloner, cc2::Tile::CC1_Cloner},
                                       posX, posY, map);
        if (cloner != QPoint(-1, -1)) {
            m_hilights << cloner;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Cloner: (%1, %2)").arg(cloner.x()).arg(cloner.y());
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::Cloner) || map.haveTile(posX, posY, cc2::Tile::CC1_Cloner)) {
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
    if (map.haveTile(posX, posY, cc2::Tile::TrapButton)) {
        QPoint trap = scanForControl({cc2::Tile::Trap}, posX, posY, map);
        if (trap != QPoint(-1, -1)) {
            m_hilights << trap;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Trap: (%1, %2)").arg(trap.x()).arg(trap.y());
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::Trap)) {
        QList<QPoint> buttons = scanForButtons(cc2::Tile::TrapButton, {cc2::Tile::Trap},
                                               posX, posY, map);
        for (const QPoint& button : buttons) {
            m_hilights << button;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Button: (%1, %2)").arg(button.x()).arg(button.y());
        }
    }
    if (map.haveTile(posX, posY, cc2::Tile::FlameJetButton)) {
        QPoint jet = diamondClosest({cc2::Tile::FlameJet_Off, cc2::Tile::FlameJet_On},
                                    posX, posY, map);
        if (jet != QPoint(-1, -1)) {
            m_hilights << jet;
            if (!tipText.isEmpty())
                tipText += QLatin1Char('\n');
            tipText += tr("Flame Jet: (%1, %2)").arg(jet.x()).arg(jet.y());
        }
    }
    std::string clue = m_map->clueForTile(posX, posY);
    if (!clue.empty()) {
        if (!tipText.isEmpty())
            tipText += QLatin1Char('\n');
        tipText += tr("Clue:\n") + QString::fromLatin1(clue.c_str()).trimmed();
    }

    setToolTip(tipText);
    update();
}

void CC2EditorWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;
    if (m_cachedButton != Qt::NoButton
            || (event->button() & (Qt::LeftButton | Qt ::MidButton | Qt::RightButton)) == 0)
        return;

    const int posX = event->x() / (m_tileset->size() * m_zoomFactor);
    const int posY = event->y() / (m_tileset->size() * m_zoomFactor);
    m_current = QPoint(-1, -1);
    m_cachedButton = event->button();
    m_origin = QPoint(posX, posY);

    mouseMoveEvent(event);
}

void CC2EditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_tileset || !m_map || !rect().contains(event->pos()))
        return;
    if (event->button() != m_cachedButton)
        return;

    if (m_drawMode == DrawInspectTile || m_drawMode == DrawInspectHint)
        emit tilePicked(m_origin.x(), m_origin.y());

    update();
    m_cachedButton = Qt::NoButton;
}

void CC2EditorWidget::setZoom(double factor)
{
    m_zoomFactor = factor;
    dirtyBuffer();
    resize(sizeHint());
    update();
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
        update();
    }
}
