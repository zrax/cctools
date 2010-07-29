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
#include "../Tileset.h"
#include "../Levelset.h"

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    enum DrawMode {
        DrawPencil, DrawLine, DrawFill, DrawSelect, DrawButtonConnect,
        DrawPathMaker,
    };

    enum PaintFlags {
        ShowPlayer = (1<<0),
        ShowMovement = (1<<1),
        ShowButtons = (1<<2),
        PaintLeftTemp = (1<<3),
        PaintRightTemp = (1<<4),
        PaintTempBury = (1<<5),
        PaintOverlayMask = PaintLeftTemp | PaintRightTemp,
    };

    EditorWidget(QWidget* parent = 0);

    void setTileset(CCETileset* tileset);
    CCETileset* tileset() const { return m_tileset; }

    void setLevelData(ccl::LevelData* level) { m_levelData = level; }
    ccl::LevelData* levelData() const { return m_levelData; }

    virtual void paintEvent(QPaintEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

    virtual QSize sizeHint() const
    {
        if (m_tileset == 0)
            return QSize();
        return QSize(m_tileset->size() * 32, m_tileset->size() * 32);
    }

    void setLeftTile(tile_t tile) { m_leftTile = tile; }
    void setRightTile(tile_t tile) { m_rightTile = tile; }
    DrawMode drawMode() const { return m_drawMode; }
    void setDrawMode(DrawMode mode) { m_drawMode = mode; }

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

public slots:
    void viewTile(QPainter& painter, int x, int y);
    void putTile(tile_t tile, int x, int y, bool bury);

private:
    CCETileset* m_tileset;
    ccl::LevelData* m_levelData;
    QList<QPoint> m_hilights;
    tile_t m_leftTile, m_rightTile;
    DrawMode m_drawMode;
    int m_paintFlags;
    QPoint m_origin, m_current;
    QPixmap m_numbers;

signals:
    void mouseInfo(QString text);
};

#endif
