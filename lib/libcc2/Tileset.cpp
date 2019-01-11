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
#include <QFileInfo>
#include "libcc1/Stream.h"

static quint8 read8(QFile& file)
{
    quint8 value;
    file.read((char*)&value, sizeof(quint8));
    return value;
}

static quint32 read32(QFile& file)
{
    quint32 value;
    file.read((char*)&value, sizeof(quint32));
    return SWAP32(value);
}

void CC2ETileset::load(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        throw ccl::IOException("Cannot open tileset file for reading");

    char magic[8];
    if (file.read(magic, 8) != 8 || memcmp(magic, "CCTILE02", 8) != 0) {
        file.close();
        throw ccl::IOException("Invalid Tileset format");
    }

    quint32 len;
    std::unique_ptr<char[]> utfbuffer;
    std::unique_ptr<uchar[]> pixbuffer;
    QPixmap tempmap;

    // Tileset name
    len = read32(file);
    utfbuffer.reset(new char[len]);
    file.read(utfbuffer.get(), len);
    m_name = QString::fromUtf8(utfbuffer.get(), len);

    // Description
    len = read32(file);
    utfbuffer.reset(new char[len]);
    file.read(utfbuffer.get(), len);
    m_description = QString::fromUtf8(utfbuffer.get(), len);

    // Tile size
    m_size = (int)read8(file);

    // Skip CC1 tilesets
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);

    // CC2 tiles
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    if (!tempmap.loadFromData(pixbuffer.get(), len, "PNG")) {
        file.close();
        throw ccl::IOException("Invalid or corrupt CC2 image");
    }
    for (int i = 0; i < cc2::NUM_GRAPHICS; ++i)
        m_gfx[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);

    file.close();
    m_filename = QFileInfo(filename).fileName();
}

