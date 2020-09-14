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

#ifndef _TILESET_H
#define _TILESET_H

#include <QObject>
#include <QPixmap>
#include <QIcon>
#include "Levelset.h"

typedef unsigned char tile_t;

class CCETileset : public QObject {
    Q_OBJECT

public:
    explicit CCETileset(QObject* parent = nullptr)
        : QObject(parent), m_size()
    { }

    QString name() const { return m_name; }
    QString description() const { return m_description; }
    int size() const { return m_size; }
    QSize qsize() const { return QSize(m_size, m_size); }

    bool load(const QString& filename);
    QString filename() const { return m_filename; }

    void drawAt(QPainter& painter, int x, int y, tile_t upper, tile_t lower = 0) const;

    void draw(QPainter& painter, int x, int y, tile_t upper, tile_t lower = 0) const
    {
        drawAt(painter, x * m_size, y * m_size,  upper, lower);
    }

    QPixmap getPixmap(tile_t tile) const;
    QIcon getIcon(tile_t tile) const { return QIcon(getPixmap(tile)); }
    static QString TileName(tile_t tile);

private:
    QString m_name, m_filename;
    QString m_description;
    int m_size;

    QPixmap m_base[ccl::NUM_TILE_TYPES];
    QPixmap m_overlay[ccl::NUM_TILE_TYPES];
};

#endif
