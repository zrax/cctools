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

bool CC2ETileset::load(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        throw ccl::IOException("Cannot open tileset file for reading");

    char magic[8];
    if (file.read(magic, 8) != 8 || memcmp(magic, "CCTILE02", 8) != 0)
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

    // Skip CC1 tilesets
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    len = read32(file);
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);

    // CC2 tiles
    len = read32(file);
    if (!len) {
        // Can't use this tileset, as it contains no CC2 tile image
        return false;
    }
    pixbuffer.reset(new uchar[len]);
    file.read((char*)pixbuffer.get(), len);
    if (!tempmap.loadFromData(pixbuffer.get(), len, "PNG"))
        throw ccl::IOException("Invalid or corrupt CC2 image");
    for (int i = 0; i < cc2::NUM_GRAPHICS; ++i)
        m_gfx[i] = tempmap.copy((i / 16) * m_size, (i % 16) * m_size, m_size, m_size);

    m_filename = QFileInfo(filename).fileName();
    return true;
}

void CC2ETileset::drawAt(QPainter& painter, int x, int y, const cc2::Tile* tile,
                         bool allLayers) const
{
    // Recurse up from the bottom-most layer
    const cc2::Tile* lower = tile->lower();
    if (lower) {
        if (allLayers)
            drawAt(painter, x, y, lower, true);
        else
            painter.drawPixmap(x, y, m_gfx[cc2::G_Floor]);
    }

    // Draw the base tile
    switch (tile->type()) {
    case cc2::Tile::Floor:
        if (tile->modifier() != 0) {
            drawWires(painter, x, y, tile->modifier(), cc2::G_Floor);
            if ((tile->modifier() & cc2::TileModifier::WireMask) == cc2::TileModifier::WireMask)
                painter.drawPixmap(x, y, m_gfx[cc2::G_Floor_Wire2]);
            else
                painter.drawPixmap(x, y, m_gfx[cc2::G_Floor_Wire4]);

            // Tunnels are only relevant to floor tiles, and should be drawn
            // on top of the overlay mask.
            if (tile->modifier() & cc2::TileModifier::WireTunnelNorth)
                painter.drawPixmap(x, y, m_gfx[cc2::G_WireTunnels],
                                   0, 0, m_size, m_size / 4);
            if (tile->modifier() & cc2::TileModifier::WireTunnelEast)
                painter.drawPixmap(x + (3 * m_size) / 4, y, m_gfx[cc2::G_WireTunnels],
                                   (3 * m_size) / 4, 0, m_size / 4, m_size);
            if (tile->modifier() & cc2::TileModifier::WireTunnelSouth)
                painter.drawPixmap(x, y + (3 * m_size) / 4, m_gfx[cc2::G_WireTunnels],
                                   0, (3 * m_size) / 4, m_size, m_size / 4);
            if (tile->modifier() & cc2::TileModifier::WireTunnelWest)
                painter.drawPixmap(x, y, m_gfx[cc2::G_WireTunnels],
                                   0, 0, m_size / 4, m_size);
        } else {
            painter.drawPixmap(x, y, m_gfx[cc2::G_Floor]);
        }
        break;
    case cc2::Tile::Wall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Wall]);
        break;
    case cc2::Tile::Ice:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ice]);
        break;
    case cc2::Tile::Ice_NE:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ice_NE]);
        break;
    case cc2::Tile::Ice_SE:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ice_SE]);
        break;
    case cc2::Tile::Ice_SW:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ice_SW]);
        break;
    case cc2::Tile::Ice_NW:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ice_NW]);
        break;
    case cc2::Tile::Water:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Water]);
        break;
    case cc2::Tile::Fire:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Fire]);
        break;
    case cc2::Tile::Force_N:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Force_N]);
        break;
    case cc2::Tile::Force_E:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Force_E]);
        break;
    case cc2::Tile::Force_S:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Force_S]);
        break;
    case cc2::Tile::Force_W:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Force_W]);
        break;
    case cc2::Tile::ToggleWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleWall]);
        break;
    case cc2::Tile::ToggleFloor:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleFloor]);
        break;
    case cc2::Tile::Teleport_Red:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Floor);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Teleport_Red]);
        break;
    case cc2::Tile::Teleport_Blue:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Floor);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Teleport_Blue]);
        break;
    case cc2::Tile::Teleport_Yellow:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Teleport_Yellow]);
        break;
    case cc2::Tile::Teleport_Green:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Teleport_Green]);
        break;
    case cc2::Tile::Exit:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Exit]);
        break;
    case cc2::Tile::Slime:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Slime]);
        break;
    case cc2::Tile::MirrorPlayer:
        painter.drawPixmap(x, y, m_gfx[cc2::G_MirrorPlayer_Underlay]);
        /* fall through */
    case cc2::Tile::Player:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player_S]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::DirtBlock:
        if (allLayers && tile->lower()->needXray())
            painter.drawPixmap(x, y, m_gfx[cc2::G_DirtBlock_Xray]);
        else
            painter.drawPixmap(x, y, m_gfx[cc2::G_DirtBlock]);
        if (tile->needArrows())
            drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::Walker:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Walker]);
        drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::Ship:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ship_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ship_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ship_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ship_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ship_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::IceBlock:
        if (allLayers && tile->lower()->needXray())
            painter.drawPixmap(x, y, m_gfx[cc2::G_IceBlock_Xray]);
        else
            painter.drawPixmap(x, y, m_gfx[cc2::G_IceBlock]);
        if (tile->needArrows())
            drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::CC1_Barrier_S:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_S]);
        break;
    case cc2::Tile::CC1_Barrier_E:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_E]);
        break;
    case cc2::Tile::CC1_Barrier_SE:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_S]);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_E]);
        break;
    case cc2::Tile::Gravel:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Gravel]);
        break;
    case cc2::Tile::ToggleButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleButton]);
        break;
    case cc2::Tile::TankButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_TankButton]);
        break;
    case cc2::Tile::BlueTank:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueTank_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueTank_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueTank_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueTank_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueTank_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::Door_Red:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Door_Red]);
        break;
    case cc2::Tile::Door_Blue:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Door_Blue]);
        break;
    case cc2::Tile::Door_Yellow:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Door_Yellow]);
        break;
    case cc2::Tile::Door_Green:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Door_Green]);
        break;
    case cc2::Tile::Key_Red:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Key_Red]);
        break;
    case cc2::Tile::Key_Blue:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Key_Blue]);
        break;
    case cc2::Tile::Key_Yellow:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Key_Yellow]);
        break;
    case cc2::Tile::Key_Green:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Key_Green]);
        break;
    case cc2::Tile::Chip:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Chip]);
        break;
    case cc2::Tile::ExtraChip:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ExtraChip]);
        break;
    case cc2::Tile::Socket:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Socket]);
        break;
    case cc2::Tile::PopUpWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_PopUpWall]);
        break;
    case cc2::Tile::AppearingWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_AppearingWall]);
        break;
    case cc2::Tile::InvisWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_InvisWall]);
        break;
    case cc2::Tile::BlueWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_BlueWall]);
        break;
    case cc2::Tile::BlueFloor:
        painter.drawPixmap(x, y, m_gfx[cc2::G_BlueFloor]);
        break;
    case cc2::Tile::Dirt:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Dirt]);
        break;
    case cc2::Tile::Ant:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ant_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ant_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ant_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ant_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ant_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::Centipede:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Centipede_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Centipede_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Centipede_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Centipede_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Centipede_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::Ball:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Ball]);
        drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::Blob:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Blob]);
        drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::AngryTeeth:
        switch (tile->direction()) {
        case cc2::Tile::North:
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AngryTeeth_S]);
            drawArrow(painter, x, y, tile->direction());
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AngryTeeth_E]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AngryTeeth_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AngryTeeth_S]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::FireBox:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FireBox]);
        drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::CloneButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_CloneButton]);
        break;
    case cc2::Tile::TrapButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_TrapButton]);
        break;
    case cc2::Tile::IceCleats:
        painter.drawPixmap(x, y, m_gfx[cc2::G_IceCleats]);
        break;
    case cc2::Tile::MagnoShoes:
        painter.drawPixmap(x, y, m_gfx[cc2::G_MagnoShoes]);
        break;
    case cc2::Tile::FireShoes:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FireShoes]);
        break;
    case cc2::Tile::Flippers:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Flippers]);
        break;
    case cc2::Tile::ToolThief:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ToolThief]);
        break;
    case cc2::Tile::RedBomb:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RedBomb]);
        break;
    //case cc2::Tile::UNUSED_41:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_Trap]);
    //    break;
    case cc2::Tile::Trap:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Trap]);
        break;
    case cc2::Tile::CC1_Cloner:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Cloner]);
        break;
    case cc2::Tile::Cloner:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Cloner]);
        if (tile->modifier() & cc2::TileModifier::CloneNorth)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ClonerArrows],
                               0, 0, m_size, m_size / 4);
        if (tile->modifier() & cc2::TileModifier::CloneEast)
            painter.drawPixmap(x + (3 * m_size) / 4, y, m_gfx[cc2::G_ClonerArrows],
                               (3 * m_size) / 4, 0, m_size / 4, m_size);
        if (tile->modifier() & cc2::TileModifier::CloneSouth)
            painter.drawPixmap(x, y + (3 * m_size) / 4, m_gfx[cc2::G_ClonerArrows],
                               0, (3 * m_size) / 4, m_size, m_size / 4);
        if (tile->modifier() & cc2::TileModifier::CloneWest)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ClonerArrows],
                               0, 0, m_size / 4, m_size);
        break;
    case cc2::Tile::Clue:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Clue]);
        break;
    case cc2::Tile::Force_Rand:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Force_Rand]);
        break;
    case cc2::Tile::AreaCtlButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_AreaCtlButton]);
        break;
    case cc2::Tile::RevolvDoor_SW:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RevolvDoor_SW]);
        break;
    case cc2::Tile::RevolvDoor_NW:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RevolvDoor_NW]);
        break;
    case cc2::Tile::RevolvDoor_NE:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RevolvDoor_NE]);
        break;
    case cc2::Tile::RevolvDoor_SE:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RevolvDoor_SE]);
        break;
    case cc2::Tile::TimeBonus:
        painter.drawPixmap(x, y, m_gfx[cc2::G_TimeBonus]);
        break;
    case cc2::Tile::ToggleClock:
        painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleClock]);
        break;
    case cc2::Tile::Transformer:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Transformer]);
        break;
    case cc2::Tile::TrainTracks:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Gravel]);
        drawTracks(painter, x, y, tile->modifier());
        break;
    case cc2::Tile::SteelWall:
        if (tile->modifier() != 0) {
            drawWires(painter, x, y, tile->modifier(), cc2::G_SteelWall);
            if ((tile->modifier() & cc2::TileModifier::WireMask) == cc2::TileModifier::WireMask)
                painter.drawPixmap(x, y, m_gfx[cc2::G_SteelWall_Wire2]);
            else
                painter.drawPixmap(x, y, m_gfx[cc2::G_SteelWall_Wire4]);
        } else {
            painter.drawPixmap(x, y, m_gfx[cc2::G_SteelWall]);
        }
        break;
    case cc2::Tile::TimeBomb:
        painter.drawPixmap(x, y, m_gfx[cc2::G_TimeBomb]);
        break;
    case cc2::Tile::Helmet:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Helmet]);
        break;
    //case cc2::Tile::UNUSED_53:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_PopDownGWall]);
    //    drawArrow(painter, x, y, tile->direction());
    //    break;
    //case cc2::Tile::UNUSED_54:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleFloor]);
    //    break;
    //case cc2::Tile::UNUSED_55:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_ToggleFloor]);
    //    break;
    case cc2::Tile::MirrorPlayer2:
        painter.drawPixmap(x, y, m_gfx[cc2::G_MirrorPlayer_Underlay]);
        /* fall through */
    case cc2::Tile::Player2:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player2_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player2_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player2_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player2_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Player2_S]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::TimidTeeth:
        switch (tile->direction()) {
        case cc2::Tile::North:
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_TimidTeeth_S]);
            drawArrow(painter, x, y, tile->direction());
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_TimidTeeth_E]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_TimidTeeth_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_TimidTeeth_S]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    //case cc2::Tile::UNUSED_Explosion:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_??]);
    //    break;
    case cc2::Tile::HikingBoots:
        painter.drawPixmap(x, y, m_gfx[cc2::G_HikingBoots]);
        break;
    case cc2::Tile::MaleOnly:
        painter.drawPixmap(x, y, m_gfx[cc2::G_MaleOnly]);
        break;
    case cc2::Tile::FemaleOnly:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FemaleOnly]);
        break;
    case cc2::Tile::LogicGate:
        painter.drawPixmap(x, y, m_gfx[cc2::G_WireFill]);
        switch (tile->modifier()) {
        case cc2::TileModifier::Inverter_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Inverter_N]);
            break;
        case cc2::TileModifier::Inverter_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Inverter_E]);
            break;
        case cc2::TileModifier::Inverter_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Inverter_S]);
            break;
        case cc2::TileModifier::Inverter_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Inverter_W]);
            break;
        case cc2::TileModifier::AndGate_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AndGate_N]);
            break;
        case cc2::TileModifier::AndGate_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AndGate_E]);
            break;
        case cc2::TileModifier::AndGate_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AndGate_S]);
            break;
        case cc2::TileModifier::AndGate_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_AndGate_W]);
            break;
        case cc2::TileModifier::OrGate_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_OrGate_N]);
            break;
        case cc2::TileModifier::OrGate_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_OrGate_E]);
            break;
        case cc2::TileModifier::OrGate_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_OrGate_S]);
            break;
        case cc2::TileModifier::OrGate_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_OrGate_W]);
            break;
        case cc2::TileModifier::XorGate_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_XorGate_N]);
            break;
        case cc2::TileModifier::XorGate_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_XorGate_E]);
            break;
        case cc2::TileModifier::XorGate_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_XorGate_S]);
            break;
        case cc2::TileModifier::XorGate_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_XorGate_W]);
            break;
        case cc2::TileModifier::LatchGateCW_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCW_N]);
            break;
        case cc2::TileModifier::LatchGateCW_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCW_E]);
            break;
        case cc2::TileModifier::LatchGateCW_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCW_S]);
            break;
        case cc2::TileModifier::LatchGateCW_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCW_W]);
            break;
        case cc2::TileModifier::NandGate_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_NandGate_N]);
            break;
        case cc2::TileModifier::NandGate_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_NandGate_E]);
            break;
        case cc2::TileModifier::NandGate_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_NandGate_S]);
            break;
        case cc2::TileModifier::NandGate_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_NandGate_W]);
            break;
        case cc2::TileModifier::CounterGate_0:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_0]);
            break;
        case cc2::TileModifier::CounterGate_1:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_1]);
            break;
        case cc2::TileModifier::CounterGate_2:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_2]);
            break;
        case cc2::TileModifier::CounterGate_3:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_3]);
            break;
        case cc2::TileModifier::CounterGate_4:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_4]);
            break;
        case cc2::TileModifier::CounterGate_5:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_5]);
            break;
        case cc2::TileModifier::CounterGate_6:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_6]);
            break;
        case cc2::TileModifier::CounterGate_7:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_7]);
            break;
        case cc2::TileModifier::CounterGate_8:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_8]);
            break;
        case cc2::TileModifier::CounterGate_9:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CounterGate_9]);
            break;
        case cc2::TileModifier::LatchGateCCW_N:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCCW_N]);
            break;
        case cc2::TileModifier::LatchGateCCW_E:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCCW_E]);
            break;
        case cc2::TileModifier::LatchGateCCW_S:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCCW_S]);
            break;
        case cc2::TileModifier::LatchGateCCW_W:
            painter.drawPixmap(x, y, m_gfx[cc2::G_LatchGateCCW_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Inverter_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    //case cc2::Tile::UNUSED_5d:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_Player]);
    //    break;
    case cc2::Tile::LogicButton:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Floor);
        painter.drawPixmap(x, y, m_gfx[cc2::G_LogicSwitch]);
        break;
    case cc2::Tile::FlameJet_Off:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FlameJet_Off]);
        break;
    case cc2::Tile::FlameJet_On:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FlameJet_On]);
        break;
    case cc2::Tile::FlameJetButton:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FlameJetButton]);
        break;
    case cc2::Tile::Lightning:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Lightning]);
        break;
    case cc2::Tile::YellowTank:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTank_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTank_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTank_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTank_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTank_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::YellowTankCtrl:
        painter.drawPixmap(x, y, m_gfx[cc2::G_YellowTankCtrl]);
        break;
    //case cc2::Tile::UNUSED_67:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_BowlingBall]);
    //    break;
    case cc2::Tile::BowlingBall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_BowlingBall]);
        break;
    case cc2::Tile::Rover:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Rover_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Rover_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Rover_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Rover_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Rover_N]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::TimePenalty:
        painter.drawPixmap(x, y, m_gfx[cc2::G_TimePenalty]);
        break;
    case cc2::Tile::StyledFloor:
        switch (tile->modifier()) {
        case cc2::TileModifier::CamoTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CamoCFloor]);
            break;
        case cc2::TileModifier::PinkDotsTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_PinkDotsCFloor]);
            break;
        case cc2::TileModifier::YellowBrickTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowBrickCFloor]);
            break;
        case cc2::TileModifier::BlueTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueCFloor]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CamoCFloor]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    //case cc2::Tile::UNUSED_6c:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_??]);
    //    break;
    case cc2::Tile::PanelCanopy:
        if (tile->tileFlags() & cc2::Tile::Canopy) {
            if (allLayers && lower->needXray())
                painter.drawPixmap(x, y, m_gfx[cc2::G_Canopy_Xray]);
            else
                painter.drawPixmap(x, y, m_gfx[cc2::G_Canopy]);
        }
        if (tile->tileFlags() & cc2::Tile::PanelNorth)
            painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_N]);
        if (tile->tileFlags() & cc2::Tile::PanelEast)
            painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_E]);
        if (tile->tileFlags() & cc2::Tile::PanelSouth)
            painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_S]);
        if (tile->tileFlags() & cc2::Tile::PanelWest)
            painter.drawPixmap(x, y, m_gfx[cc2::G_Panel_W]);
        if (tile->tileFlags() == 0) {
            painter.drawPixmap(x, y, m_gfx[cc2::G_Canopy_Xray]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
        }
        break;
    //case cc2::Tile::UNUSED_6e:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_RRSign]);
    //    break;
    case cc2::Tile::RRSign:
        painter.drawPixmap(x, y, m_gfx[cc2::G_RRSign]);
        break;
    case cc2::Tile::StyledWall:
        switch (tile->modifier()) {
        case cc2::TileModifier::CamoTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CamoCWall]);
            break;
        case cc2::TileModifier::PinkDotsTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_PinkDotsCWall]);
            break;
        case cc2::TileModifier::YellowBrickTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_YellowBrickCWall]);
            break;
        case cc2::TileModifier::BlueTheme:
            painter.drawPixmap(x, y, m_gfx[cc2::G_BlueCWall]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_CamoCWall]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::AsciiGlyph:
        painter.drawPixmap(x, y, m_gfx[cc2::G_AsciiGlyphFrame]);
        drawGlyph(painter, x, y, tile->modifier());
        break;
    case cc2::Tile::LSwitchFloor:
        painter.drawPixmap(x, y, m_gfx[cc2::G_LSwitchFloor]);
        break;
    case cc2::Tile::LSwitchWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_LSwitchWall]);
        break;
    //case cc2::Tile::UNUSED_74:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_??]);
    //    break;
    //case cc2::Tile::UNUSED_75:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_??]);
    //    break;
    //case cc2::Tile::UNUSED_79:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_??]);
    //    drawArrow(painter, x, y, tile->direction());
    //    break;
    case cc2::Tile::Flag10:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Flag10]);
        break;
    case cc2::Tile::Flag100:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Flag100]);
        break;
    case cc2::Tile::Flag1000:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Flag1000]);
        break;
    case cc2::Tile::StayUpGWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_StayUpGWall]);
        break;
    case cc2::Tile::PopDownGWall:
        painter.drawPixmap(x, y, m_gfx[cc2::G_PopDownGWall]);
        break;
    case cc2::Tile::Disallow:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Disallow]);
        break;
    case cc2::Tile::Flag2x:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Flag2x]);
        break;
    case cc2::Tile::DirBlock:
        painter.drawPixmap(x, y, m_gfx[cc2::G_DirBlock]);
        if (tile->tileFlags() & cc2::Tile::ArrowNorth)
            painter.drawPixmap(x, y, m_gfx[cc2::G_DirBlockArrows],
                               0, 0, m_size, m_size / 4);
        if (tile->tileFlags() & cc2::Tile::ArrowEast)
            painter.drawPixmap(x + (3 * m_size) / 4, y, m_gfx[cc2::G_DirBlockArrows],
                               (3 * m_size) / 4, 0, m_size / 4, m_size);
        if (tile->tileFlags() & cc2::Tile::ArrowSouth)
            painter.drawPixmap(x, y + (3 * m_size) / 4, m_gfx[cc2::G_DirBlockArrows],
                               0, (3 * m_size) / 4, m_size, m_size / 4);
        if (tile->tileFlags() & cc2::Tile::ArrowWest)
            painter.drawPixmap(x, y, m_gfx[cc2::G_DirBlockArrows],
                               0, 0, m_size / 4, m_size);
        if (tile->needArrows())
            drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::FloorMimic:
        painter.drawPixmap(x, y, m_gfx[cc2::G_FloorMimic]);
        drawArrow(painter, x, y, tile->direction());
        break;
    case cc2::Tile::GreenBomb:
        painter.drawPixmap(x, y, m_gfx[cc2::G_GreenBomb]);
        break;
    case cc2::Tile::GreenChip:
        painter.drawPixmap(x, y, m_gfx[cc2::G_GreenChip]);
        break;
    //case cc2::Tile::UNUSED_85:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_GreenBomb]);
    //    break;
    //case cc2::Tile::UNUSED_86:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_GreenChip]);
    //    break;
    case cc2::Tile::RevLogicButton:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Floor);
        painter.drawPixmap(x, y, m_gfx[cc2::G_RevLogicButton]);
        break;
    case cc2::Tile::Switch_Off:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Switch_Base);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Switch_Off]);
        break;
    case cc2::Tile::Switch_On:
        drawWires(painter, x, y, tile->modifier(), cc2::G_Switch_Base);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Switch_On]);
        break;
    case cc2::Tile::KeyThief:
        painter.drawPixmap(x, y, m_gfx[cc2::G_KeyThief]);
        break;
    case cc2::Tile::Ghost:
        switch (tile->direction()) {
        case cc2::Tile::North:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ghost_N]);
            break;
        case cc2::Tile::East:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ghost_E]);
            break;
        case cc2::Tile::South:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ghost_S]);
            break;
        case cc2::Tile::West:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ghost_W]);
            break;
        default:
            painter.drawPixmap(x, y, m_gfx[cc2::G_Ghost_S]);
            painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
            break;
        }
        break;
    case cc2::Tile::SteelFoil:
        painter.drawPixmap(x, y, m_gfx[cc2::G_SteelFoil]);
        break;
    case cc2::Tile::Turtle:
        // TODO: Maybe we don't need a masked version of the turtle...
        painter.drawPixmap(x, y, m_gfx[cc2::G_Water]);
        painter.drawPixmap(x, y, m_gfx[cc2::G_Turtle]);
        break;
    case cc2::Tile::Eye:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Eye]);
        break;
    case cc2::Tile::Bribe:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Bribe]);
        break;
    case cc2::Tile::SpeedShoes:
        painter.drawPixmap(x, y, m_gfx[cc2::G_SpeedShoes]);
        break;
    //case cc2::Tile::UNUSED_91:
    //    painter.drawPixmap(x, y, m_gfx[cc2::G_Canopy]);
    //    break;
    case cc2::Tile::Hook:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Hook]);
        break;
    default:
        painter.drawPixmap(x, y, m_gfx[cc2::G_Floor]);
        painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
        if (tile->haveDirection())
            drawArrow(painter, x, y, tile->direction());
        break;
    }
}

void CC2ETileset::drawArrow(QPainter& painter, int x, int y,
                            cc2::Tile::Direction direction) const
{
    switch (direction) {
    case cc2::Tile::North:
        painter.drawPixmap(x + (m_size / 4), y, m_gfx[cc2::G_GlyphArrows],
                           0, 0, m_size / 2, m_size / 2);
        break;
    case cc2::Tile::East:
        painter.drawPixmap(x + (m_size / 2), y + (m_size / 4), m_gfx[cc2::G_GlyphArrows],
                           m_size / 2, 0, m_size / 2, m_size / 2);
        break;
    case cc2::Tile::South:
        painter.drawPixmap(x + (m_size / 4), y + (m_size / 2), m_gfx[cc2::G_GlyphArrows],
                           0, m_size / 2, m_size / 2, m_size / 2);
        break;
    case cc2::Tile::West:
        painter.drawPixmap(x, y + (m_size / 4), m_gfx[cc2::G_GlyphArrows],
                           m_size / 2, m_size / 2, m_size / 2, m_size / 2);
        break;
    default:
        break;
    }
}

void CC2ETileset::drawGlyph(QPainter& painter, int x, int y, uint32_t glyph) const
{
    if (glyph < cc2::TileModifier::GlyphMIN || glyph > cc2::TileModifier::GlyphMAX) {
        painter.drawPixmap(x, y, m_gfx[cc2::G_InvalidBase]);
        return;
    }

    const size_t id = cc2::G_GlyphArrows + ((glyph - cc2::TileModifier::GlyphMIN) / 4);
    const int sx = (glyph % 2) * (m_size / 2);
    const int sy = ((glyph / 2) % 2) * (m_size / 2);
    painter.drawPixmap(x + (m_size / 4), y + (m_size / 4), m_gfx[id],
                       sx, sy, m_size / 2, m_size / 2);
}

void CC2ETileset::drawTracks(QPainter& painter, int x, int y, uint32_t tracks) const
{
    // Draw track base first
    if (tracks & cc2::TileModifier::Track_NE)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_NE]);
    if (tracks & cc2::TileModifier::Track_SE)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_SE]);
    if (tracks & cc2::TileModifier::Track_SW)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_SW]);
    if (tracks & cc2::TileModifier::Track_NW)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_NW]);
    if (tracks & cc2::TileModifier::Track_NS)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_NS]);
    if (tracks & cc2::TileModifier::Track_WE)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_WE]);

    // Draw any applicable rails.  Active rails must be drawn after
    // inactive rails, for cleanest appearance (and to match CC2)
    const bool haveSwitch = (tracks & cc2::TileModifier::TrackSwitch) != 0;
    const uint32_t activeTrack = tracks & cc2::TileModifier::ActiveTrack_MASK;
    if (haveSwitch) {
        if ((tracks & cc2::TileModifier::Track_NE) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_NE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_NE]);
        if ((tracks & cc2::TileModifier::Track_SE) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_SE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_SE]);
        if ((tracks & cc2::TileModifier::Track_SW) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_SW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_SW]);
        if ((tracks & cc2::TileModifier::Track_NW) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_NW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_NW]);
        if ((tracks & cc2::TileModifier::Track_NS) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_NS)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_NS]);
        if ((tracks & cc2::TileModifier::Track_WE) != 0
                && activeTrack != cc2::TileModifier::ActiveTrack_WE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_InactiveTRail_WE]);

        if ((tracks & cc2::TileModifier::Track_NE) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_NE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NE]);
        if ((tracks & cc2::TileModifier::Track_SE) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_SE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_SE]);
        if ((tracks & cc2::TileModifier::Track_SW) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_SW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_SW]);
        if ((tracks & cc2::TileModifier::Track_NW) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_NW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NW]);
        if ((tracks & cc2::TileModifier::Track_NS) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_NS)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NS]);
        if ((tracks & cc2::TileModifier::Track_WE) != 0
                && activeTrack == cc2::TileModifier::ActiveTrack_WE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_WE]);
    } else {
        if (tracks & cc2::TileModifier::Track_NE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NE]);
        if (tracks & cc2::TileModifier::Track_SE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_SE]);
        if (tracks & cc2::TileModifier::Track_SW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_SW]);
        if (tracks & cc2::TileModifier::Track_NW)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NW]);
        if (tracks & cc2::TileModifier::Track_NS)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_NS]);
        if (tracks & cc2::TileModifier::Track_WE)
            painter.drawPixmap(x, y, m_gfx[cc2::G_ActiveTRail_WE]);
    }

    // Always draw the switch last
    if (haveSwitch)
        painter.drawPixmap(x, y, m_gfx[cc2::G_Track_Switch]);
}

void CC2ETileset::drawWires(QPainter& painter, int x, int y, uint32_t wireMask,
                            cc2::GraphicIndex base) const
{
    // TODO: This assumes wires are always 2 pixels wide and aligned to
    // the center of the tileset...
    const int mid = m_size / 2;
    painter.drawPixmap(x, y, m_gfx[base]);
    if (wireMask & cc2::TileModifier::WireNorth)
        painter.drawPixmap(x + mid - 1, y, m_gfx[cc2::G_WireFill],
                           mid - 1, 0, 2, mid + 1);
    if (wireMask & cc2::TileModifier::WireEast)
        painter.drawPixmap(x + mid - 1, y + mid - 1, m_gfx[cc2::G_WireFill],
                           mid - 1, mid - 1, mid + 1, 2);
    if (wireMask & cc2::TileModifier::WireSouth)
        painter.drawPixmap(x + mid - 1, y + mid - 1, m_gfx[cc2::G_WireFill],
                           mid - 1, mid - 1, 2, mid + 1);
    if (wireMask & cc2::TileModifier::WireWest)
        painter.drawPixmap(x, y + mid - 1, m_gfx[cc2::G_WireFill],
                           0, mid - 1, mid + 1, 2);
}

QIcon CC2ETileset::getIcon(const cc2::Tile* tile) const
{
    QPixmap ico(m_size, m_size);
    if (tile) {
        QPainter painter(&ico);
        draw(painter, 0, 0, tile, false);
        painter.end();
    }
    return QIcon(ico);
}

QString CC2ETileset::baseName(cc2::Tile::Type type)
{
    switch (type) {
    case cc2::Tile::Floor:
        return tr("Floor");
    case cc2::Tile::Wall:
        return tr("Wall");
    case cc2::Tile::Ice:
        return tr("Ice");
    case cc2::Tile::Ice_NE:
        return tr("Ice Turn - North/East");
    case cc2::Tile::Ice_SE:
        return tr("Ice Turn - South/East");
    case cc2::Tile::Ice_SW:
        return tr("Ice Turn - South/West");
    case cc2::Tile::Ice_NW:
        return tr("Ice Turn - North/West");
    case cc2::Tile::Water:
        return tr("Water");
    case cc2::Tile::Fire:
        return tr("Fire");
    case cc2::Tile::Force_N:
        return tr("Force Floor - North");
    case cc2::Tile::Force_E:
        return tr("Force Floor - East");
    case cc2::Tile::Force_S:
        return tr("Force Floor - South");
    case cc2::Tile::Force_W:
        return tr("Force Floor - West");
    case cc2::Tile::ToggleWall:
        return tr("Toggle Door - Closed");
    case cc2::Tile::ToggleFloor:
        return tr("Toggle Door - Open");
    case cc2::Tile::Teleport_Red:
        return tr("Red Teleport");
    case cc2::Tile::Teleport_Blue:
        return tr("Blue Teleport");
    case cc2::Tile::Teleport_Yellow:
        return tr("Yellow Teleport");
    case cc2::Tile::Teleport_Green:
        return tr("Green Teleport");
    case cc2::Tile::Exit:
        return tr("Exit");
    case cc2::Tile::Slime:
        return tr("Slime");
    case cc2::Tile::Player:
        return tr("Chip");
    case cc2::Tile::DirtBlock:
        return tr("Dirt Block");
    case cc2::Tile::Walker:
        return tr("Walker");
    case cc2::Tile::Ship:
        return tr("Ship");
    case cc2::Tile::IceBlock:
        return tr("Ice Block");
    case cc2::Tile::CC1_Barrier_S:
        return tr("CC1 Panel - South");
    case cc2::Tile::CC1_Barrier_E:
        return tr("CC1 Panel - East");
    case cc2::Tile::CC1_Barrier_SE:
        return tr("CC1 Panel - South/East");
    case cc2::Tile::Gravel:
        return tr("Gravel");
    case cc2::Tile::ToggleButton:
        return tr("Toggle Door Button");
    case cc2::Tile::TankButton:
        return tr("Blue Tank Button");
    case cc2::Tile::BlueTank:
        return tr("Blue Tank");
    case cc2::Tile::Door_Red:
        return tr("Red Door");
    case cc2::Tile::Door_Blue:
        return tr("Blue Door");
    case cc2::Tile::Door_Yellow:
        return tr("Yellow Door");
    case cc2::Tile::Door_Green:
        return tr("Green Door");
    case cc2::Tile::Key_Red:
        return tr("Red Key");
    case cc2::Tile::Key_Blue:
        return tr("Blue Key");
    case cc2::Tile::Key_Yellow:
        return tr("Yellow Key");
    case cc2::Tile::Key_Green:
        return tr("Green Key");
    case cc2::Tile::Chip:
        return tr("IC Chip");
    case cc2::Tile::ExtraChip:
        return tr("Extra IC Chip");
    case cc2::Tile::Socket:
        return tr("Socket");
    case cc2::Tile::PopUpWall:
        return tr("Pop-Up Wall");
    case cc2::Tile::AppearingWall:
        return tr("Appearing Wall");
    case cc2::Tile::InvisWall:
        return tr("Invisible Wall");
    case cc2::Tile::BlueWall:
        return tr("Blue Block - Wall");
    case cc2::Tile::BlueFloor:
        return tr("Blue Block - Floor");
    case cc2::Tile::Dirt:
        return tr("Dirt");
    case cc2::Tile::Ant:
        return tr("Ant");
    case cc2::Tile::Centipede:
        return tr("Centipede");
    case cc2::Tile::Ball:
        return tr("Bouncy Ball");
    case cc2::Tile::Blob:
        return tr("Blob");
    case cc2::Tile::AngryTeeth:
        return tr("Angry Teeth");
    case cc2::Tile::FireBox:
        return tr("Fire Box");
    case cc2::Tile::CloneButton:
        return tr("Clone Button");
    case cc2::Tile::TrapButton:
        return tr("Trap Button");
    case cc2::Tile::IceCleats:
        return tr("Ice Cleats");
    case cc2::Tile::MagnoShoes:
        return tr("Magno Shoes");
    case cc2::Tile::FireShoes:
        return tr("Fire Boots");
    case cc2::Tile::Flippers:
        return tr("Flippers");
    case cc2::Tile::ToolThief:
        return tr("Tool Thief");
    case cc2::Tile::RedBomb:
        return tr("Red Bomb");
    case cc2::Tile::Trap:
        return tr("Trap");
    case cc2::Tile::CC1_Cloner:
        return tr("CC1 Cloning Machine");
    case cc2::Tile::Cloner:
        return tr("Cloning Machine");
    case cc2::Tile::Clue:
        return tr("Clue");
    case cc2::Tile::Force_Rand:
        return tr("Force Floor - Random");
    case cc2::Tile::AreaCtlButton:
        return tr("Area Control Button");
    case cc2::Tile::RevolvDoor_SW:
        return tr("Revolving Door - South/West");
    case cc2::Tile::RevolvDoor_NW:
        return tr("Revolving Door - North/West");
    case cc2::Tile::RevolvDoor_NE:
        return tr("Revolving Door - North/East");
    case cc2::Tile::RevolvDoor_SE:
        return tr("Revolving Door - South/East");
    case cc2::Tile::TimeBonus:
        return tr("Time Bonus");
    case cc2::Tile::ToggleClock:
        return tr("Toggle Clock");
    case cc2::Tile::Transformer:
        return tr("Transformer");
    case cc2::Tile::TrainTracks:
        return tr("Train Track");
    case cc2::Tile::SteelWall:
        return tr("Steel Wall");
    case cc2::Tile::TimeBomb:
        return tr("Time Bomb");
    case cc2::Tile::Helmet:
        return tr("Helmet");
    case cc2::Tile::Player2:
        return tr("Melinda");
    case cc2::Tile::TimidTeeth:
        return tr("Timid Teeth");
    case cc2::Tile::HikingBoots:
        return tr("Hiking Boots");
    case cc2::Tile::MaleOnly:
        return tr("Male Only");
    case cc2::Tile::FemaleOnly:
        return tr("Female Only");
    case cc2::Tile::LogicGate:
        return tr("Logic Gate");
    case cc2::Tile::LogicButton:
        return tr("Logic Button");
    case cc2::Tile::FlameJet_Off:
        return tr("Flame Jet - Off");
    case cc2::Tile::FlameJet_On:
        return tr("Flame Jet - On");
    case cc2::Tile::FlameJetButton:
        return tr("Flame Jet Button");
    case cc2::Tile::Lightning:
        return tr("Lightning");
    case cc2::Tile::YellowTank:
        return tr("Yellow Tank");
    case cc2::Tile::YellowTankCtrl:
        return tr("Yellow Tank Control");
    case cc2::Tile::MirrorPlayer:
        return tr("Mirror Chip");
    case cc2::Tile::MirrorPlayer2:
        return tr("Mirror Melinda");
    case cc2::Tile::BowlingBall:
        return tr("Bowling Ball");
    case cc2::Tile::Rover:
        return tr("Rover");
    case cc2::Tile::TimePenalty:
        return tr("Time Penalty");
    case cc2::Tile::StyledFloor:
        return tr("Styled Floor");
    case cc2::Tile::PanelCanopy:
        return tr("Panel / Canopy");
    case cc2::Tile::RRSign:
        return tr("Railroad Sign");
    case cc2::Tile::StyledWall:
        return tr("Styled Wall");
    case cc2::Tile::AsciiGlyph:
        return tr("Glyph");
    case cc2::Tile::LSwitchFloor:
        return tr("Logic Door - Open");
    case cc2::Tile::LSwitchWall:
        return tr("Logic Door - Closed");
    case cc2::Tile::Flag10:
        return tr("10 Point Flag");
    case cc2::Tile::Flag100:
        return tr("100 Point Flag");
    case cc2::Tile::Flag1000:
        return tr("1000 Point Flag");
    case cc2::Tile::StayUpGWall:
        return tr("Stay Up Wall");
    case cc2::Tile::PopDownGWall:
        return tr("Pop Down Wall");
    case cc2::Tile::Disallow:
        return tr("Not Allowed");
    case cc2::Tile::Flag2x:
        return tr("2x Point Flag");
    case cc2::Tile::DirBlock:
        return tr("Directional Block");
    case cc2::Tile::FloorMimic:
        return tr("Floor Mimic");
    case cc2::Tile::GreenBomb:
        return tr("Toggle Bomb");
    case cc2::Tile::GreenChip:
        return tr("Toggle IC Chip");
    case cc2::Tile::RevLogicButton:
        return tr("Reverse Logic Button");
    case cc2::Tile::Switch_Off:
        return tr("Switch - Off");
    case cc2::Tile::Switch_On:
        return tr("Switch - On");
    case cc2::Tile::KeyThief:
        return tr("Key Thief");
    case cc2::Tile::Ghost:
        return tr("Ghost");
    case cc2::Tile::SteelFoil:
        return tr("Steel Foil");
    case cc2::Tile::Turtle:
        return tr("Turtle");
    case cc2::Tile::Eye:
        return tr("Secret Eye");
    case cc2::Tile::Bribe:
        return tr("Bribe");
    case cc2::Tile::SpeedShoes:
        return tr("Speed Shoes");
    case cc2::Tile::Hook:
        return tr("Hook");
    default:
        return tr("Invalid (%1)").arg(type);
    }
}

QString CC2ETileset::getName(const cc2::Tile* tile)
{
    QString name = baseName(tile->type());

    switch (tile->type()) {
    case cc2::Tile::Floor:
        if (tile->modifier() & cc2::TileModifier::WireTunnelMask) {
            name = tr("Wire Tunnel");
            QStringList directions;
            if (tile->modifier() & cc2::TileModifier::WireTunnelNorth)
                directions << "North";
            if (tile->modifier() & cc2::TileModifier::WireTunnelSouth)
                directions << "South";
            if (tile->modifier() & cc2::TileModifier::WireTunnelEast)
                directions << "East";
            if (tile->modifier() & cc2::TileModifier::WireTunnelWest)
                directions << "West";
            if (!directions.isEmpty())
                name += QStringLiteral(" - ") + directions.join(QLatin1Char('/'));
        }
        break;
    case cc2::Tile::Cloner:
        if (tile->modifier() == cc2::TileModifier::CloneAllDirs) {
            name += tr(" - Any");
        } else {
            QStringList directions;
            if (tile->modifier() & cc2::TileModifier::CloneNorth)
                directions << "North";
            if (tile->modifier() & cc2::TileModifier::CloneSouth)
                directions << "South";
            if (tile->modifier() & cc2::TileModifier::CloneEast)
                directions << "East";
            if (tile->modifier() & cc2::TileModifier::CloneWest)
                directions << "West";
            if (!directions.isEmpty())
                name += QStringLiteral(" - ") + directions.join(QLatin1Char('/'));
        }
        break;
    case cc2::Tile::TrainTracks:
        {
            QStringList directions;
            if (tile->modifier() & cc2::TileModifier::Track_NE)
                directions << "NE";
            if (tile->modifier() & cc2::TileModifier::Track_SE)
                directions << "SE";
            if (tile->modifier() & cc2::TileModifier::Track_SW)
                directions << "SW";
            if (tile->modifier() & cc2::TileModifier::Track_NW)
                directions << "NW";
            if (tile->modifier() & cc2::TileModifier::Track_NS)
                directions << "NS";
            if (tile->modifier() & cc2::TileModifier::Track_WE)
                directions << "EW";
            if (tile->modifier() & cc2::TileModifier::TrackSwitch)
                directions << "Switch";
            if (!directions.isEmpty())
                name += QStringLiteral(" - ") + directions.join(QLatin1Char('/'));
        }
        // TODO: List active direction
        break;
    case cc2::Tile::LogicGate:
        switch (tile->modifier()) {
        case cc2::TileModifier::Inverter_N:
            name = tr("Inverter - North");
            break;
        case cc2::TileModifier::Inverter_E:
            name = tr("Inverter - East");
            break;
        case cc2::TileModifier::Inverter_S:
            name = tr("Inverter - South");
            break;
        case cc2::TileModifier::Inverter_W:
            name = tr("Inverter - West");
            break;
        case cc2::TileModifier::AndGate_N:
            name = tr("And Gate - North");
            break;
        case cc2::TileModifier::AndGate_E:
            name = tr("And Gate - East");
            break;
        case cc2::TileModifier::AndGate_S:
            name = tr("And Gate - South");
            break;
        case cc2::TileModifier::AndGate_W:
            name = tr("And Gate - West");
            break;
        case cc2::TileModifier::OrGate_N:
            name = tr("Or Gate - North");
            break;
        case cc2::TileModifier::OrGate_E:
            name = tr("Or Gate - East");
            break;
        case cc2::TileModifier::OrGate_S:
            name = tr("Or Gate - South");
            break;
        case cc2::TileModifier::OrGate_W:
            name = tr("Or Gate - West");
            break;
        case cc2::TileModifier::XorGate_N:
            name = tr("Xor Gate - North");
            break;
        case cc2::TileModifier::XorGate_E:
            name = tr("Xor Gate - East");
            break;
        case cc2::TileModifier::XorGate_S:
            name = tr("Xor Gate - South");
            break;
        case cc2::TileModifier::XorGate_W:
            name = tr("Xor Gate - West");
            break;
        case cc2::TileModifier::LatchGateCW_N:
            name = tr("Latch Gate CW - North");
            break;
        case cc2::TileModifier::LatchGateCW_E:
            name = tr("Latch Gate CW - East");
            break;
        case cc2::TileModifier::LatchGateCW_S:
            name = tr("Latch Gate CW - South");
            break;
        case cc2::TileModifier::LatchGateCW_W:
            name = tr("Latch Gate CW - West");
            break;
        case cc2::TileModifier::NandGate_N:
            name = tr("Nand Gate - North");
            break;
        case cc2::TileModifier::NandGate_E:
            name = tr("Nand Gate - East");
            break;
        case cc2::TileModifier::NandGate_S:
            name = tr("Nand Gate - South");
            break;
        case cc2::TileModifier::NandGate_W:
            name = tr("Nand Gate - West");
            break;
        case cc2::TileModifier::CounterGate_0:
            name = tr("Counter Gate - 0");
            break;
        case cc2::TileModifier::CounterGate_1:
            name = tr("Counter Gate - 1");
            break;
        case cc2::TileModifier::CounterGate_2:
            name = tr("Counter Gate - 2");
            break;
        case cc2::TileModifier::CounterGate_3:
            name = tr("Counter Gate - 3");
            break;
        case cc2::TileModifier::CounterGate_4:
            name = tr("Counter Gate - 4");
            break;
        case cc2::TileModifier::CounterGate_5:
            name = tr("Counter Gate - 5");
            break;
        case cc2::TileModifier::CounterGate_6:
            name = tr("Counter Gate - 6");
            break;
        case cc2::TileModifier::CounterGate_7:
            name = tr("Counter Gate - 7");
            break;
        case cc2::TileModifier::CounterGate_8:
            name = tr("Counter Gate - 8");
            break;
        case cc2::TileModifier::CounterGate_9:
            name = tr("Counter Gate - 9");
            break;
        case cc2::TileModifier::LatchGateCCW_N:
            name = tr("Latch Gate CCW - North");
            break;
        case cc2::TileModifier::LatchGateCCW_E:
            name = tr("Latch Gate CCW - East");
            break;
        case cc2::TileModifier::LatchGateCCW_S:
            name = tr("Latch Gate CCW - South");
            break;
        case cc2::TileModifier::LatchGateCCW_W:
            name = tr("Latch Gate CCW - West");
            break;
        default:
            name = tr("Invalid Logic Gate (0x%1)").arg(tile->modifier(), 0, 16);
            break;
        }
        break;
    case cc2::Tile::StyledFloor:
        switch (tile->modifier()) {
        case cc2::TileModifier::CamoTheme:
            name += tr(" - Camo");
            break;
        case cc2::TileModifier::PinkDotsTheme:
            name += tr(" - Pink");
            break;
        case cc2::TileModifier::YellowBrickTheme:
            name += tr(" - Yellow Brick");
            break;
        case cc2::TileModifier::BlueTheme:
            name += tr(" - Blue");
            break;
        default:
            name += tr(" - Invalid (0x%1)").arg(tile->modifier(), 0, 16);
            break;
        }
        break;
    case cc2::Tile::PanelCanopy:
        {
            if (tile->tileFlags() & cc2::Tile::Canopy)
                name = tr("Canopy");
            else
                name = QString();

            QStringList directions;
            if (tile->tileFlags() & cc2::Tile::PanelNorth)
                directions << tr("North");
            if (tile->tileFlags() & cc2::Tile::PanelSouth)
                directions << tr("South");
            if (tile->tileFlags() & cc2::Tile::PanelEast)
                directions << tr("East");
            if (tile->tileFlags() & cc2::Tile::PanelWest)
                directions << tr("West");
            if (!directions.isEmpty()) {
                if (!name.isEmpty())
                    name += QStringLiteral(" / ");
                name += tr("Panel - ") + directions.join(QLatin1Char('/'));
            }
            if (tile->tileFlags() == 0)
                name = tr("Panel/Canopy - Invalid");
        }
        break;
    case cc2::Tile::StyledWall:
        switch (tile->modifier()) {
        case cc2::TileModifier::CamoTheme:
            name += tr(" - Camo");
            break;
        case cc2::TileModifier::PinkDotsTheme:
            name += tr(" - Pink");
            break;
        case cc2::TileModifier::YellowBrickTheme:
            name += tr(" - Yellow Brick");
            break;
        case cc2::TileModifier::BlueTheme:
            name += tr(" - Blue");
            break;
        default:
            name += tr(" - Invalid (0x%1)").arg(tile->modifier(), 0, 16);
            break;
        }
        break;
    case cc2::Tile::AsciiGlyph:
        if (tile->modifier() == cc2::TileModifier::GlyphUp)
            name += tr(" - Up");
        else if (tile->modifier() == cc2::TileModifier::GlyphRight)
            name += tr(" - Right");
        else if (tile->modifier() == cc2::TileModifier::GlyphDown)
            name += tr(" - Down");
        else if (tile->modifier() == cc2::TileModifier::GlyphLeft)
            name += tr(" - Left");
        else if (tile->modifier() == ' ')
            name += tr(" - Space");
        else if (tile->modifier() >= cc2::TileModifier::GlyphASCII_MIN
                 && tile->modifier() <= cc2::TileModifier::GlyphASCII_MAX)
            name += tr(" - %1").arg(QLatin1Char(tile->modifier()));
        else
            name += tr(" - Invalid (0x%1)").arg(tile->modifier(), 0, 16);
        break;
    case cc2::Tile::DirBlock:
        if (tile->tileFlags() == cc2::Tile::AllArrows) {
            name += tr(" - Any");
        } else {
            QStringList arrows;
            if (tile->tileFlags() & cc2::Tile::ArrowNorth)
                arrows << "North";
            if (tile->tileFlags() & cc2::Tile::ArrowSouth)
                arrows << "South";
            if (tile->tileFlags() & cc2::Tile::ArrowEast)
                arrows << "East";
            if (tile->tileFlags() & cc2::Tile::ArrowWest)
                arrows << "West";
            if (!arrows.isEmpty())
                name += QStringLiteral(" - ") + arrows.join(QLatin1Char('/'));
        }
        break;
    default:
        break;
    }

    if (tile->needArrows()) {
        switch (tile->direction()) {
        case cc2::Tile::North:
            name += tr(" - North");
            break;
        case cc2::Tile::East:
            name += tr(" - East");
            break;
        case cc2::Tile::South:
            name += tr(" - South");
            break;
        case cc2::Tile::West:
            name += tr(" - West");
            break;
        default:
            name += tr(" - Invalid Direction");
            break;
        }
    }

    return name;
}
