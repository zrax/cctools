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

#include "Tileset.h"

#include <QPainter>
#include <QFile>
#include "Errors.h"
#include <cstdio>

void CCETileset::load(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        throw ccl::IOException("Cannot open tileset file for reading");

    char magic[8];
    if (file.read(magic, 8) != 8 || memcmp(magic, "CCTILE01", 8) != 0) {
        file.close();
        throw ccl::IOException("Invalid Tileset format");
    }

    quint32 len;
    char* utfbuffer;
    uchar* pixbuffer;

    // Tileset name
    file.read((char*)&len, sizeof(quint32));
    utfbuffer = new char[len+1];
    file.read(utfbuffer, len);
    utfbuffer[len] = 0;
    m_name = QString::fromUtf8(utfbuffer);
    delete[] utfbuffer;

    // Description
    file.read((char*)&len, sizeof(quint32));
    utfbuffer = new char[len+1];
    file.read(utfbuffer, len);
    utfbuffer[len] = 0;
    m_description = QString::fromUtf8(utfbuffer);
    delete[] utfbuffer;

    // Tile size
    quint8 tsize;
    file.read((char*)&tsize, sizeof(quint8));
    m_size = tsize;

    // Base tiles
    file.read((char*)&len, sizeof(quint32));
    pixbuffer = new uchar[len];
    file.read((char*)pixbuffer, len);
    if (!m_base.loadFromData(pixbuffer, len, "PNG")) {
        delete[] pixbuffer;
        file.close();
        throw ccl::IOException("Invalid or corrupt base image");
    }
    delete[] pixbuffer;

    // Overlay tiles
    file.read((char*)&len, sizeof(quint32));
    pixbuffer = new uchar[len];
    file.read((char*)pixbuffer, len);
    if (!m_overlay.loadFromData(pixbuffer, len, "PNG")) {
        delete[] pixbuffer;
        file.close();
        throw ccl::IOException("Invalid or corrupt overlay image");
    }
    delete[] pixbuffer;

    file.close();
}

void CCETileset::draw(QPainter painter, int x, int y, tile_t upper, tile_t lower) const
{
    int destX = x * m_size;
    int destY = y * m_size;
    if (lower != 0) {
        painter.drawPixmap(QPoint(destX, destY), m_base,
                           QRect((lower / 32) * m_size, (lower % 32) * m_size,
                                 m_size, m_size));
        painter.drawPixmap(QPoint(destX, destY), m_overlay,
                           QRect((upper / 32) * m_size, (upper % 32) * m_size,
                                 m_size, m_size));
    } else {
        painter.drawPixmap(QPoint(destX, destY), m_base,
                           QRect((upper / 32) * m_size, (upper % 32) * m_size,
                                 m_size, m_size));
    }
}
