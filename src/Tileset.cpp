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
#include <cstdio>
#include "Errors.h"

#ifdef BYTES_BIG_ENDIAN
#define SWAP32(x)   (((x) << 24) & 0xFF000000) | (((x) << 8) & 0x00FF0000) | \
                    (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00)
#else
#define SWAP32(x)   (x)
#endif

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
    QPixmap tempmap;

    // Tileset name
    len = read32(file);
    utfbuffer = new char[len+1];
    file.read(utfbuffer, len);
    utfbuffer[len] = 0;
    m_name = QString::fromUtf8(utfbuffer);
    delete[] utfbuffer;

    // Description
    len = read32(file);
    utfbuffer = new char[len+1];
    file.read(utfbuffer, len);
    utfbuffer[len] = 0;
    m_description = QString::fromUtf8(utfbuffer);
    delete[] utfbuffer;

    // Tile size
    m_size = (int)read8(file);

    // Base tiles
    len = read32(file);
    pixbuffer = new uchar[len];
    file.read((char*)pixbuffer, len);
    if (!tempmap.loadFromData(pixbuffer, len, "PNG")) {
        delete[] pixbuffer;
        file.close();
        throw ccl::IOException("Invalid or corrupt base image");
    }
    for (int i=0; i<ccl::NUM_TILE_TYPES; ++i)
        m_base[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);
    delete[] pixbuffer;

    // Overlay tiles
    len = read32(file);
    pixbuffer = new uchar[len];
    file.read((char*)pixbuffer, len);
    if (!tempmap.loadFromData(pixbuffer, len, "PNG")) {
        delete[] pixbuffer;
        file.close();
        throw ccl::IOException("Invalid or corrupt overlay image");
    }
    for (int i=0; i<ccl::NUM_TILE_TYPES; ++i)
        m_overlay[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);
    delete[] pixbuffer;

    file.close();
}

void CCETileset::drawAt(QPainter& painter, int x, int y, tile_t upper, tile_t lower) const
{
    if (lower != 0) {
        painter.drawPixmap(x, y, m_base[lower]);
        painter.drawPixmap(x, y, m_overlay[upper]);
    } else {
        painter.drawPixmap(x, y, m_base[upper]);
    }
}

void CCETileset::addTiles(QListWidget* list, QList<tile_t> tiles) const
{
    foreach (tile_t tile, tiles) {
        QListWidgetItem* item = new QListWidgetItem(TileName(tile), list);
        item->setData(Qt::UserRole, (int)tile);
    }
}

void CCETileset::imageTiles(QListWidget* list) const
{
    list->setIconSize(QSize(m_size, m_size));
    for (int i=0; i<list->count(); ++i)
        list->item(i)->setIcon(getIcon(list->item(i)->data(Qt::UserRole).toInt()));
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
        "UNUSED_36", "UNUSED_37", "UNUSED_38", "Player In Exit", "Exit Anim 2",
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
