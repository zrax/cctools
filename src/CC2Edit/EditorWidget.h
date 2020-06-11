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
#include "libcc2/Tileset.h"
#include "libcc2/Map.h"

class CC2EditorWidget : public QWidget {
    Q_OBJECT

public:
    enum DrawMode {
        DrawPencil, DrawLine, DrawFill, DrawSelect, DrawPathMaker,
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

    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    QSize sizeHint() const override
    {
        const int tilesetSize = m_tileset ? m_tileset->size() : 32;
        const int width = m_map ? m_map->mapData().width() : 16;
        const int height = m_map ? m_map->mapData().height() : 16;
        return QSize(width * tilesetSize * m_zoomFactor, height * tilesetSize * m_zoomFactor);
    }

    void renderTileBuffer();
    void dirtyBuffer() { m_cacheDirty = true; }
    double zoom() const { return m_zoomFactor; }

    void renderTo(QPainter& painter);

public slots:
    void setZoom(double factor);

private:
    CC2ETileset* m_tileset;
    cc2::Map* m_map;
    QList<QPoint> m_hilights;
    QPoint m_origin, m_current;

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
    {
        return calcTileRect(rect.left(), rect.top(), rect.width(), rect.height());
    }

    QPoint calcTileCenter(int x, int y)
    {
        return QPoint((int)((x * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor),
                      (int)((y * m_tileset->size() + (m_tileset->size() / 2)) * m_zoomFactor));
    }

    QSize renderSize() const
    {
        return QSize(m_map->mapData().width() * m_tileset->size() * m_zoomFactor,
                     m_map->mapData().height() * m_tileset->size() * m_zoomFactor);
    }

signals:
    void mouseInfo(const QString& text, int timeout = 0);
    void canUndo(bool);
    void canRedo(bool);
    void hasSelection(bool);
    void makeDirty();
};

#endif
