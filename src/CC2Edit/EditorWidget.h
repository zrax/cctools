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
        DrawPencil, DrawLine, DrawFill, DrawSelect, DrawPathMaker,
        DrawInspectTile,
    };

    enum PaintFlags {
        ShowPlayer = (1<<0),
        ShowMovement = (1<<1),
        ShowButtons = (1<<2),
        ShowMovePaths = (1<<3),
        ShowViewBox = (1<<4),
        ShowErrors = (1<<5),
    };

    CC2EditorWidget(QWidget* parent = nullptr);

    ~CC2EditorWidget() override
    {
        if (m_map)
            m_map->unref();
    }

    void setTileset(CC2ETileset* tileset);
    CC2ETileset* tileset() const { return m_tileset; }

    void setMap(cc2::Map* map);
    cc2::Map* map() const { return m_map; }
    bool isOrphaned() const { return m_map->refs() == 1; }

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

    void setPaintFlag(int flag)
    {
        m_paintFlags |= flag;
        update();
    }

    void clearPaintFlag(int flag)
    {
        m_paintFlags &= ~flag;
        update();
    }

    void beginEdit(CC2EditHistory::Type type);
    void endEdit();
    void cancelEdit();

    void renderTileBuffer();
    void dirtyBuffer() { m_cacheDirty = true; }
    double zoom() const { return m_zoomFactor; }

    void renderTo(QPainter& painter);

signals:
    void mouseInfo(const QString& text, int timeout = 0);
    void canUndo(bool);
    void canRedo(bool);
    void cleanChanged(bool);
    void hasSelection(bool);

    void inspectTile(cc2::Tile* tile);

public slots:
    void setZoom(double factor);
    void undo();
    void redo();

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    CC2ETileset* m_tileset;
    cc2::Map* m_map;
    QString m_filename;
    QList<QPoint> m_hilights;
    DrawMode m_drawMode;
    uint32_t m_paintFlags;
    Qt::MouseButton m_cachedButton;
    QPoint m_origin, m_current;
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

    QPoint calcTileCenter(int x, int y) const
    {
        return QPoint((int)((x * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor),
                      (int)((y * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor));
    }

    QSize renderSize() const
    {
        return QSize(m_map->mapData().width() * m_tileset->size() * m_zoomFactor,
                     m_map->mapData().height() * m_tileset->size() * m_zoomFactor);
    }
};

#endif
