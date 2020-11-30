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

#ifndef _CC2_EDITORWIDGET_H
#define _CC2_EDITORWIDGET_H

#include <QWidget>
#include "History.h"
#include "libcc2/Tileset.h"
#include "libcc2/Map.h"

class QPainter;
class QUndoStack;

class CC2EditorWidget : public QWidget {
    Q_OBJECT

public:
    enum DrawMode {
        DrawPencil, DrawLine, DrawFill, DrawFlood, DrawSelect, DrawPathMaker,
        DrawWires, DrawInspectTile, DrawInspectHint,
    };

    enum CombineMode {
        CombineSmart, CombineForce, Replace,
    };

    enum PaintFlags {
        ShowMovePaths = (1<<0),
        ShowViewBox = (1<<1),
        ShowErrors = (1<<2),
        ShowAll = ShowMovePaths | ShowViewBox | ShowErrors,
    };

    CC2EditorWidget(QWidget* parent = nullptr);

    ~CC2EditorWidget() override
    {
        if (m_map)
            m_map->unref();
        m_editCache->unref();
    }

    void setTileset(CC2ETileset* tileset);
    CC2ETileset* tileset() const { return m_tileset; }

    void setMap(cc2::Map* map);
    cc2::Map* map() const { return m_map; }
    void resizeMap(const QSize& size);

    void setFilename(const QString& filename) { m_filename = filename; }
    QString filename() const { return m_filename; }

    QSize sizeHint() const override
    {
        const int tilesetSize = m_tileset ? m_tileset->size() : 32;
        const int width = m_map ? m_map->mapData().width() : 16;
        const int height = m_map ? m_map->mapData().height() : 16;
        return QSize(width * tilesetSize * m_zoomFactor, height * tilesetSize * m_zoomFactor);
    }

    DrawMode drawMode() const { return m_drawMode; }
    void setDrawMode(DrawMode mode);

    QRect selection() const { return m_selectRect; }
    void selectRegion(int left, int top, int width, int height)
    {
        m_selectRect = QRect(left, top, width, height);
    }

    void setPaintFlag(int flag)
    {
        uint32_t newFlags = m_paintFlags | flag;
        if (newFlags != m_paintFlags) {
            m_paintFlags = newFlags;
            dirtyBuffer();
        }
    }

    void clearPaintFlag(int flag)
    {
        uint32_t newFlags = m_paintFlags & ~flag;
        if (newFlags != m_paintFlags) {
            m_paintFlags = newFlags;
            dirtyBuffer();
        }
    }

    void beginEdit(CC2EditHistory::Type type);
    void endEdit();
    void cancelEdit();

    bool canUndo() const { return m_undoStack->canUndo(); }
    bool canRedo() const { return m_undoStack->canRedo(); }
    bool isClean() const { return m_undoStack->isClean(); }
    void setClean();
    void resetClean();

    void renderTileBuffer();
    void dirtyBuffer()
    {
        m_cacheDirty = true;
        update();
    }

    double zoom() const { return m_zoomFactor; }

    void renderTo(QPainter& painter);
    QImage renderReport();
    QImage renderSelection();

signals:
    void mouseInfo(const QString& text, int timeout = 0);
    void canUndoChanged(bool);
    void canRedoChanged(bool);
    void cleanChanged(bool);
    void hasSelection(bool);
    void updateCounters();

    void tilePicked(int x, int y);
    void clueAdded(int x, int y);
    void clueDeleted(int x, int y);

public slots:
    void putTile(const cc2::Tile& tile, int x, int y, CombineMode mode);
    void setZoom(double factor);
    void undo();
    void redo();
    void setLeftTile(const cc2::Tile& tile) { m_leftTile = tile; }
    void setRightTile(const cc2::Tile& tile) { m_rightTile = tile; }

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    CC2ETileset* m_tileset;
    cc2::Map* m_map;
    cc2::Map* m_editCache;
    QString m_filename;
    QList<QPoint> m_hilights;
    cc2::Tile m_leftTile, m_rightTile;
    DrawMode m_drawMode;
    uint32_t m_paintFlags;
    Qt::MouseButton m_cachedButton;
    QPoint m_origin, m_current;
    cc2::Tile::Direction m_lastDir;
    cc2::Tile m_lastPathTile;
    QUndoStack* m_undoStack;
    MapUndoCommand* m_undoCommand;
    QRect m_selectRect;

    double m_zoomFactor;
    QPixmap m_tileBuffer;
    QPixmap m_tileCache;
    bool m_cacheDirty;

    QRect calcTileRect(int x, int y, int w = 1, int h = 1) const
    {
        // Size is calculated inclusively, so -2 is needed to get past
        // the border and adjust for the inclusive offset
        QPoint topleft((int)(x * m_tileset->size() * m_zoomFactor),
                       (int)(y * m_tileset->size() * m_zoomFactor));
        QPoint botright((int)((x + w) * m_tileset->size() * m_zoomFactor) - 2,
                        (int)((y + h) * m_tileset->size() * m_zoomFactor) - 2);
        return QRect(topleft, botright);
    }

    QRect calcTileRect(const QRect& rect) const
    {
        return calcTileRect(rect.left(), rect.top(), rect.width(), rect.height());
    }

    QPoint calcPathCenter(int x, int y) const
    {
        // Offset slightly to avoid drawing over logic wires
        return QPoint((int)((x * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor) + 2,
                      (int)((y * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor) + 2);
    }

    QSize renderSize() const
    {
        return QSize(m_map->mapData().width() * m_tileset->size() * m_zoomFactor,
                     m_map->mapData().height() * m_tileset->size() * m_zoomFactor);
    }

    void addWire(cc2::Tile& tile, cc2::Tile::Direction direction);
    void addWireTunnel(cc2::Tile& tile, cc2::Tile::Direction direction);
    void delWire(cc2::Tile& tile, cc2::Tile::Direction direction);

    void updateForUndoCommand(const QUndoCommand* command);
};

#endif
