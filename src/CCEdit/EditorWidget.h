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

#ifndef _EDITORWIDGET_H
#define _EDITORWIDGET_H

#include <QWidget>
#include <QPainter>
#include "libcc1/Tileset.h"
#include "libcc1/Levelset.h"

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    enum DrawMode {
        DrawPencil, DrawLine, DrawFill, DrawFlood, DrawSelect,
        DrawButtonConnect, DrawInspectTile, DrawPathMaker,
    };

    enum DrawLayer { LayTop, LayBottom, LayAuto };

    enum PaintFlags {
        ShowPlayer = (1<<0),
        ShowMovement = (1<<1),
        ShowButtons = (1<<2),
        ShowMovePaths = (1<<3),
        ShowViewBox = (1<<4),
        ShowErrors = (1<<5),
        ShowAll = ShowPlayer | ShowMovement | ShowButtons | ShowMovePaths |
                  ShowViewBox | ShowErrors,

        // Renders the upper layer very faintly over the lower layer.
        RevealLower = (1<<11),
    };

    EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget() override
    {
        if (m_levelData)
            m_levelData->unref();
        m_levelEditCache->unref();
    }

    void setTileset(CCETileset* tileset);
    CCETileset* tileset() const { return m_tileset; }

    void setLevelData(ccl::LevelData* level);
    ccl::LevelData* levelData() const { return m_levelData; }

    QSize sizeHint() const override
    {
        if (!m_tileset)
            return QSize();
        return QSize(32 * m_tileset->size() * m_zoomFactor,
                     32 * m_tileset->size() * m_zoomFactor);
    }

    void setLeftTile(tile_t tile) { m_leftTile = tile; }
    void setRightTile(tile_t tile) { m_rightTile = tile; }
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

    void renderTileBuffer();
    void dirtyBuffer()
    {
        m_cacheDirty = true;
        update();
    }

    double zoom() const { return m_zoomFactor; }

    void renderTo(QPainter& painter);
    QPixmap renderReport();

public slots:
    void putTile(tile_t tile, int x, int y, DrawLayer layer);
    void setZoom(double factor);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    CCETileset* m_tileset;
    ccl::LevelData* m_levelData;
    ccl::LevelData* m_levelEditCache;
    QList<QPoint> m_hilights;
    tile_t m_leftTile, m_rightTile;
    DrawMode m_drawMode;
    uint32_t m_paintFlags;
    Qt::MouseButton m_cachedButton;
    QPixmap m_numbers, m_errmk;
    QPoint m_origin, m_current;
    ccl::Direction m_lastDir;
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

    QRect calcTileRect(const QPoint& point) const
    {
        return calcTileRect(point.x(), point.y());
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

    QPoint calcTileCenter(const QPoint& point) const
    {
        return calcTileCenter(point.x(), point.y());
    }

signals:
    void mouseInfo(const QString& text, int timeout = 0);
    void hasSelection(bool);
    void editingStarted();
    void editingFinished();
    void editingCancelled();
    void tilePicked(int x, int y);
};

#endif
