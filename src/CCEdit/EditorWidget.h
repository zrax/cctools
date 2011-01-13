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
#include "History.h"
#include "../Tileset.h"
#include "../Levelset.h"

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    enum DrawMode {
        DrawPencil, DrawLine, DrawFill, DrawSelect, DrawButtonConnect,
        DrawPathMaker,
    };

    enum DrawLayer { LayTop, LayBottom, LayAuto };

    enum PaintFlags {
        ShowPlayer = (1<<0),
        ShowMovement = (1<<1),
        ShowButtons = (1<<2),
        ShowMovePaths = (1<<3),
        ShowViewBox = (1<<4),
        ShowAll = ShowPlayer | ShowMovement | ShowButtons | ShowMovePaths |
                  ShowViewBox,
        PaintLeftTemp = (1<<5),
        PaintRightTemp = (1<<6),
        PaintTempBury = (1<<7),
        PaintOverlayMask = PaintLeftTemp | PaintRightTemp,
    };

    EditorWidget(QWidget* parent = 0);
    virtual ~EditorWidget()
    {
        m_levelData->unref();
    }

    void setTileset(CCETileset* tileset);
    CCETileset* tileset() const { return m_tileset; }

    void setLevelData(ccl::LevelData* level);
    ccl::LevelData* levelData() const { return m_levelData; }
    bool isOrphaned() const { return m_levelData->refs() == 1; }

    virtual void paintEvent(QPaintEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

    virtual QSize sizeHint() const
    {
        if (m_tileset == 0)
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
    { m_selectRect = QRect(left, top, width, height); }

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

    void beginEdit(CCEHistoryNode::Type type)
    { m_history.beginEdit(type, m_levelData); }

    void endEdit()
    {
         m_history.endEdit(m_levelData);
         updateUndoStatus();
         dirtyBuffer();
    }

    void cancelEdit() { m_history.cancelEdit(); }

    void updateUndoStatus()
    {
         emit canUndo(m_history.canUndo());
         emit canRedo(m_history.canRedo());
    }

    void renderTileBuffer();
    void dirtyBuffer() { m_cacheDirty = true; }
    double zoom() const { return m_zoomFactor; }

    void renderTo(QPainter& painter);
    QPixmap renderReport();

public slots:
    void viewTile(QPainter& painter, int x, int y);
    void putTile(tile_t tile, int x, int y, DrawLayer layer);
    void undo();
    void redo();
    void setZoom(double factor);

private:
    CCETileset* m_tileset;
    ccl::LevelData* m_levelData;
    QList<QPoint> m_hilights;
    tile_t m_leftTile, m_rightTile;
    DrawMode m_drawMode;
    int m_paintFlags;
    Qt::MouseButton m_cachedButton;
    QPixmap m_numbers;
    QPoint m_origin, m_current;
    ccl::Direction m_lastDir;
    CCEHistory m_history;
    QRect m_selectRect;

    double m_zoomFactor;
    QPixmap m_tileBuffer;
    QPixmap m_tileCache;
    bool m_cacheDirty;

    QRect calcTileRect(int x, int y, int w = 1, int h = 1)
    {
        // Size is calculated inclusively, so -2 is needed to get past
        // the border and adjust for the inclusive offset
        QPoint topleft((int)(x * m_tileset->size() * m_zoomFactor),
                       (int)(y * m_tileset->size() * m_zoomFactor));
        QPoint botright((int)((x + w) * m_tileset->size() * m_zoomFactor) - 2,
                        (int)((y + h) * m_tileset->size() * m_zoomFactor) - 2);
        return QRect(topleft, botright);
    }

    QRect calcTileRect(QRect rect)
    { return calcTileRect(rect.left(), rect.top(), rect.width(), rect.height()); }

    QPoint calcTileCenter(int x, int y)
    {
        return QPoint((int)((x * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor),
                      (int)((y * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor));
    }

signals:
    void mouseInfo(QString text);
    void canUndo(bool);
    void canRedo(bool);
    void hasSelection(bool);
    void makeDirty();
};

#endif
