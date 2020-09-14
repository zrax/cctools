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
#include "Stream.h"

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

bool CCETileset::load(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        throw ccl::IOException("Cannot open tileset file for reading");

    char magic[8];
    if (file.read(magic, 8) != 8
            || (memcmp(magic, "CCTILE01", 8) != 0 && memcmp(magic, "CCTILE02", 8) != 0))
        return false;

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

    // Base tiles
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    if (!tempmap.loadFromData(pixbuffer.get(), len, "PNG"))
        throw ccl::IOException("Invalid or corrupt base image");
    for (int i=0; i<ccl::NUM_TILE_TYPES; ++i)
        m_base[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);

    // Overlay tiles
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    if (!tempmap.loadFromData(pixbuffer.get(), len, "PNG"))
        throw ccl::IOException("Invalid or corrupt overlay image");
    for (int i=0; i<ccl::NUM_TILE_TYPES; ++i)
        m_overlay[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);

    m_filename = QFileInfo(filename).fileName();
    return true;
}

void CCETileset::drawAt(QPainter& painter, int x, int y, tile_t upper, tile_t lower) const
{
    if (upper >= ccl::NUM_TILE_TYPES)
        upper = ccl::Tile_UNUSED_20;
    if (lower >= ccl::NUM_TILE_TYPES)
        lower = ccl::Tile_UNUSED_20;

    if (lower != 0) {
        painter.drawPixmap(x, y, m_base[lower]);
        painter.drawPixmap(x, y, m_overlay[upper]);
    } else {
        painter.drawPixmap(x, y, m_base[upper]);
    }
}

QPixmap CCETileset::getPixmap(tile_t tile) const
{
    return (tile < ccl::NUM_TILE_TYPES)
            ? m_base[tile]
            : m_base[ccl::Tile_UNUSED_20];
}

#define DIRTILENAME(prefix) \
    prefix " - North", prefix " - West", prefix " - South", prefix " - East"

#define COLORTILENAME(suffix) \
    "Blue " suffix, "Red " suffix, "Green " suffix, "Yellow " suffix

QString CCETileset::TileName(tile_t tile)
{
    static QString s_tileNames[] = {
        "Floor", "Wall", "Chip", "Water", "Fire", "Invisible Wall",
        DIRTILENAME("Barrier"), "Block", "Dirt", "Ice", "Force Floor - South",
        DIRTILENAME("Block"), "Force Floor - North", "Force Floor - East",
        "Force Floor - West", "Exit", COLORTILENAME("Door"),
        "Ice Turn - South/East", "Ice Turn - South/West",
        "Ice Turn - North/West", "Ice Turn - North/East", "Blue Block - Floor",
        "Blue Block - Wall", "UNUSED_20", "Thief", "Socket",
        "Door Toggle Button", "Clone Button", "Toggle Door - Closed",
        "Toggle Door - Open", "Trap Release Button", "Tank Button",
        "Teleporter", "Bomb", "Trap", "Appearing Wall", "Gravel", "Pop-Up Wall",
        "Hint", "Barrier - South/East", "Cloning Machine",
        "Force Floor - Random", "Splash Death", "Fire Death", "Burned Player",
        "UNUSED_36", "UNUSED_37", "Ice Block", "Player In Exit", "Exit Anim 2",
        "Exit Anim 3", DIRTILENAME("Player Swim"), DIRTILENAME("Beetle"),
        DIRTILENAME("Fireball"), DIRTILENAME("Bouncy Ball"),
        DIRTILENAME("Tank"), DIRTILENAME("Glider"), DIRTILENAME("Teeth"),
        DIRTILENAME("Walker"), DIRTILENAME("Blob"), DIRTILENAME("Crawler"),
        COLORTILENAME("Key"), "Flippers", "Fire Boots", "Ice Skates",
        "Force Suction Boots", DIRTILENAME("Player")
    };

    if (tile < ccl::NUM_TILE_TYPES)
        return s_tileNames[tile];
    else
        return QString("UNUSED_%1").arg(tile, 2, 16, QChar('0'));
}
