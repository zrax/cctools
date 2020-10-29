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
        throw ccl::IOError(ccl::RuntimeError::tr("Cannot open tileset file for reading"));

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
        throw ccl::IOError(ccl::RuntimeError::tr("Invalid or corrupt base image"));
    for (int i=0; i<ccl::NUM_TILE_TYPES; ++i)
        m_base[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);

    // Overlay tiles
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    if (!tempmap.loadFromData(pixbuffer.get(), len, "PNG"))
        throw ccl::IOError(ccl::RuntimeError::tr("Invalid or corrupt overlay image"));
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

QString CCETileset::TileName(tile_t tile)
{
    switch (tile) {
    case ccl::TileFloor:
        return tr("Floor");
    case ccl::TileWall:
        return tr("Wall");
    case ccl::TileChip:
        return tr("Chip");
    case ccl::TileWater:
        return tr("Water");
    case ccl::TileFire:
        return tr("Fire");
    case ccl::TileInvisWall:
        return tr("Invisible Wall");
    case ccl::TileBarrier_N:
        return tr("Barrier - North");
    case ccl::TileBarrier_W:
        return tr("Barrier - West");
    case ccl::TileBarrier_S:
        return tr("Barrier - South");
    case ccl::TileBarrier_E:
        return tr("Barrier - East");
    case ccl::TileBlock:
        return tr("Block");
    case ccl::TileDirt:
        return tr("Dirt");
    case ccl::TileIce:
        return tr("Ice");
    case ccl::TileForce_S:
        return tr("Force Floor - South");
    case ccl::TileBlock_N:
        return tr("Block - North");
    case ccl::TileBlock_W:
        return tr("Block - West");
    case ccl::TileBlock_S:
        return tr("Block - South");
    case ccl::TileBlock_E:
        return tr("Block - East");
    case ccl::TileForce_N:
        return tr("Force Floor - North");
    case ccl::TileForce_E:
        return tr("Force Floor - East");
    case ccl::TileForce_W:
        return tr("Force Floor - West");
    case ccl::TileExit:
        return tr("Exit");
    case ccl::TileDoor_Blue:
        return tr("Blue Door");
    case ccl::TileDoor_Red:
        return tr("Red Door");
    case ccl::TileDoor_Green:
        return tr("Green Door");
    case ccl::TileDoor_Yellow:
        return tr("Yellow Door");
    case ccl::TileIce_SE:
        return tr("Ice Turn - South/East");
    case ccl::TileIce_SW:
        return tr("Ice Turn - South/West");
    case ccl::TileIce_NW:
        return tr("Ice Turn - North/West");
    case ccl::TileIce_NE:
        return tr("Ice Turn - North/East");
    case ccl::TileBlueFloor:
        return tr("Blue Block - Floor");
    case ccl::TileBlueWall:
        return tr("Blue Block - Wall");
    case ccl::TileThief:
        return tr("Thief");
    case ccl::TileSocket:
        return tr("Socket");
    case ccl::TileToggleButton:
        return tr("Door Toggle Button");
    case ccl::TileCloneButton:
        return tr("Clone Button");
    case ccl::TileToggleWall:
        return tr("Toggle Door - Closed");
    case ccl::TileToggleFloor:
        return tr("Toggle Door - Open");
    case ccl::TileTrapButton:
        return tr("Trap Release Button");
    case ccl::TileTankButton:
        return tr("Tank Button");
    case ccl::TileTeleport:
        return tr("Teleporter");
    case ccl::TileBomb:
        return tr("Bomb");
    case ccl::TileTrap:
        return tr("Trap");
    case ccl::TileAppearingWall:
        return tr("Appearing Wall");
    case ccl::TileGravel:
        return tr("Gravel");
    case ccl::TilePopUpWall:
        return tr("Pop-Up Wall");
    case ccl::TileHint:
        return tr("Hint");
    case ccl::TileBarrier_SE:
        return tr("Barrier - South/East");
    case ccl::TileCloner:
        return tr("Cloning Machine");
    case ccl::TileForce_Rand:
        return tr("Force Floor - Random");
    case ccl::TilePlayerSplash:
        return tr("Splash Death");
    case ccl::TilePlayerFire:
        return tr("Fire Death");
    case ccl::TilePlayerBurnt:
        return tr("Burned Player");
    case ccl::TileIceBlock:
        return tr("Ice Block");
    case ccl::TilePlayerExit:
        return tr("Player In Exit");
    case ccl::TileExitAnim2:
        return tr("Exit Anim 2");
    case ccl::TileExitAnim3:
        return tr("Exit Anim 3");
    case ccl::TilePlayerSwim_N:
        return tr("Player Swim - North");
    case ccl::TilePlayerSwim_W:
        return tr("Player Swim - West");
    case ccl::TilePlayerSwim_S:
        return tr("Player Swim - South");
    case ccl::TilePlayerSwim_E:
        return tr("Player Swim - East");
    case ccl::TileBug_N:
        return tr("Beetle - North");
    case ccl::TileBug_W:
        return tr("Beetle - West");
    case ccl::TileBug_S:
        return tr("Beetle - South");
    case ccl::TileBug_E:
        return tr("Beetle - East");
    case ccl::TileFireball_N:
        return tr("Fireball - North");
    case ccl::TileFireball_W:
        return tr("Fireball - West");
    case ccl::TileFireball_S:
        return tr("Fireball - South");
    case ccl::TileFireball_E:
        return tr("Fireball - East");
    case ccl::TileBall_N:
        return tr("Bouncy Ball - North");
    case ccl::TileBall_W:
        return tr("Bouncy Ball - West");
    case ccl::TileBall_S:
        return tr("Bouncy Ball - South");
    case ccl::TileBall_E:
        return tr("Bouncy Ball - East");
    case ccl::TileTank_N:
        return tr("Tank - North");
    case ccl::TileTank_W:
        return tr("Tank - West");
    case ccl::TileTank_S:
        return tr("Tank - South");
    case ccl::TileTank_E:
        return tr("Tank - East");
    case ccl::TileGlider_N:
        return tr("Glider - North");
    case ccl::TileGlider_W:
        return tr("Glider - West");
    case ccl::TileGlider_S:
        return tr("Glider - South");
    case ccl::TileGlider_E:
        return tr("Glider - East");
    case ccl::TileTeeth_N:
        return tr("Teeth - North");
    case ccl::TileTeeth_W:
        return tr("Teeth - West");
    case ccl::TileTeeth_S:
        return tr("Teeth - South");
    case ccl::TileTeeth_E:
        return tr("Teeth - East");
    case ccl::TileWalker_N:
        return tr("Walker - North");
    case ccl::TileWalker_W:
        return tr("Walker - West");
    case ccl::TileWalker_S:
        return tr("Walker - South");
    case ccl::TileWalker_E:
        return tr("Walker - East");
    case ccl::TileBlob_N:
        return tr("Blob - North");
    case ccl::TileBlob_W:
        return tr("Blob - West");
    case ccl::TileBlob_S:
        return tr("Blob - South");
    case ccl::TileBlob_E:
        return tr("Blob - East");
    case ccl::TileCrawler_N:
        return tr("Crawler - North");
    case ccl::TileCrawler_W:
        return tr("Crawler - West");
    case ccl::TileCrawler_S:
        return tr("Crawler - South");
    case ccl::TileCrawler_E:
        return tr("Crawler - East");
    case ccl::TileKey_Blue:
        return tr("Blue Key");
    case ccl::TileKey_Red:
        return tr("Red Key");
    case ccl::TileKey_Green:
        return tr("Green Key");
    case ccl::TileKey_Yellow:
        return tr("Yellow Key");
    case ccl::TileFlippers:
        return tr("Flippers");
    case ccl::TileFireBoots:
        return tr("Fire Boots");
    case ccl::TileIceSkates:
        return tr("Ice Skates");
    case ccl::TileForceBoots:
        return tr("Force Suction Boots");
    case ccl::TilePlayer_N:
        return tr("Player - North");
    case ccl::TilePlayer_W:
        return tr("Player - West");
    case ccl::TilePlayer_S:
        return tr("Player - South");
    case ccl::TilePlayer_E:
        return tr("Player - East");
    default:
        return tr("UNUSED_%1").arg(tile, 2, 16, QLatin1Char('0'));
    }
}
