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

#ifndef _CC2_TILESET_H
#define _CC2_TILESET_H

#include <QObject>
#include <QPixmap>
#include <QIcon>
#include "Map.h"

namespace cc2 {

// For CC2, we need more graphics than we have tile types.  Therefore
// we keep a separate index into the graphics table for rendering, which
// is not necessarily in sync with the tile type enum.
enum GraphicIndex {
    G_InvalidBase, G_Floor, G_Wall, G_Ice, G_Ice_NE, G_Ice_SE,
    G_Ice_SW, G_Ice_NW, G_Water, G_Fire, G_Force_N, G_Force_E,
    G_Force_S, G_Force_W, G_ToggleWall, G_ToggleFloor,
    G_Teleport_Red, G_Teleport_Blue, G_Teleport_Yellow, G_Teleport_Green,
    G_Exit, G_Slime, G_Player_N, G_Player_E, G_Player_S, G_Player_W,
    G_DirtBlock, G_DirtBlock_Xray, G_Walker,
    G_Ship_N, G_Ship_E, G_Ship_S, G_Ship_W, G_IceBlock, G_IceBlock_Xray,
    G_Gravel, G_ToggleButton, G_TankButton,
    G_BlueTank_N, G_BlueTank_E, G_BlueTank_S, G_BlueTank_W,
    G_Door_Red, G_Door_Blue, G_Door_Yellow, G_Door_Green,
    G_Key_Red, G_Key_Blue, G_Key_Yellow, G_Key_Green, G_Chip, G_ExtraChip,
    G_Socket, G_PopUpWall, G_AppearingWall, G_InvisWall,
    G_BlueWall, G_BlueFloor, G_Dirt, G_Ant_N, G_Ant_E, G_Ant_S, G_Ant_W,
    G_Centipede_N, G_Centipede_E, G_Centipede_S, G_Centipede_W,
    G_Ball, G_Blob, G_AngryTeeth_S, G_AngryTeeth_E, G_AngryTeeth_W,
    G_FireBox, G_CloneButton, G_TrapButton, G_IceCleats, G_MagnoShoes,
    G_FireShoes, G_Flippers, G_ToolThief, G_RedBomb, G_Trap, G_Cloner,
    G_ClonerArrows, G_DirTileArrows, G_Clue, G_Force_Rand, G_AreaCtlButton,
    G_RevolvDoor_SW, G_RevolvDoor_NW, G_RevolvDoor_NE, G_RevolvDoor_SE,
    G_TimeBonus, G_ToggleClock, G_Transformer, G_Track_Switch,
    G_Track_NE, G_Track_SE, G_Track_SW, G_Track_NW, G_Track_WE, G_Track_NS,
    G_InactiveTRail_NE, G_InactiveTRail_SE, G_InactiveTRail_SW,
    G_InactiveTRail_NW, G_InactiveTRail_WE, G_InactiveTRail_NS,
    G_ActiveTRail_NE, G_ActiveTRail_SE, G_ActiveTRail_SW,
    G_ActiveTRail_NW, G_ActiveTRail_WE, G_ActiveTRail_NS,
    G_SteelWall, G_SteelWall_Wire2, G_SteelWall_Wire4, G_Floor_Wire2,
    G_Floor_Wire4, G_WireFill, G_TimeBomb, G_Helmet,
    G_Player2_N, G_Player2_E, G_Player2_S, G_Player2_W, G_TimidTeeth_S,
    G_TimidTeeth_E, G_TimidTeeth_W, G_HikingBoots, G_MaleOnly, G_FemaleOnly,
    G_Inverter_N, G_Inverter_E, G_Inverter_S, G_Inverter_W,
    G_AndGate_N, G_AndGate_E, G_AndGate_S, G_AndGate_W,
    G_OrGate_N, G_OrGate_E, G_OrGate_S, G_OrGate_W,
    G_XorGate_N, G_XorGate_E, G_XorGate_S, G_XorGate_W,
    G_LatchGateCW_N, G_LatchGateCW_E, G_LatchGateCW_S, G_LatchGateCW_W,
    G_NandGate_N, G_NandGate_E, G_NandGate_S, G_NandGate_W,
    G_LatchGateCCW_N, G_LatchGateCCW_E, G_LatchGateCCW_S, G_LatchGateCCW_W,
    G_CounterGate_0, G_CounterGate_1, G_CounterGate_2, G_CounterGate_3,
    G_CounterGate_4, G_CounterGate_5, G_CounterGate_6, G_CounterGate_7,
    G_CounterGate_8, G_CounterGate_9, G_LogicSwitch,
    G_FlameJet_Off, G_FlameJet_On, G_FlameJetButton, G_Lightning,
    G_YellowTank_N, G_YellowTank_E, G_YellowTank_S, G_YellowTank_W,
    G_YellowTankCtrl, G_MirrorPlayer_Underlay, G_BowlingBall,
    G_Rover_N, G_Rover_E, G_Rover_S, G_Rover_W, G_TimePenalty,
    G_CamoCFloor, G_PinkDotsCFloor, G_YellowBrickCFloor, G_BlueCFloor,
    G_CamoCWall, G_PinkDotsCWall, G_YellowBrickCWall, G_BlueCWall,
    G_Panel_N, G_Panel_E, G_Panel_S, G_Panel_W, G_Canopy, G_Canopy_Xray,
    G_RRSign, G_AsciiGlyphFrame, G_LSwitchFloor, G_LSwitchWall,
    G_Flag10, G_Flag100, G_Flag1000, G_StayUpGWall, G_PopDownGWall,
    G_Disallow, G_Flag2x, G_DirBlock, G_DirBlockArrows, G_FloorMimic,
    G_GreenBomb, G_GreenChip, G_RevLogicButton, G_Switch_Off, G_Switch_On,
    G_Switch_Base, G_KeyThief, G_Ghost_N, G_Ghost_E, G_Ghost_S, G_Ghost_W,
    G_SteelFoil, G_Turtle, G_Eye, G_Bribe, G_SpeedShoes, G_Hook,

    G_GlyphArrows = 0xef, G_GlyphAscii_Base, G_GlyphAscii_End = 0xff,

    NUM_GRAPHICS
};

}

class CC2ETileset : public QObject {
    Q_OBJECT

public:
    CC2ETileset(QObject* parent = 0)
        : QObject(parent), m_size()
    { }

    QString name() const { return m_name; }
    QString description() const { return m_description; }
    int size() const { return m_size; }
    QSize qsize() const { return QSize(m_size, m_size); }

    void load(const QString& filename);
    QString filename() const { return m_filename; }

    void drawAt(QPainter& painter, int x, int y, const cc2::Tile* tile) const;

    void draw(QPainter& painter, int x, int y, const cc2::Tile* tile) const
    {
        drawAt(painter, x * m_size, y * m_size, tile);
    }

    QIcon getIcon(cc2::Tile* tile) const;

private:
    QString m_name, m_filename;
    QString m_description;
    int m_size;

    QPixmap m_gfx[cc2::NUM_GRAPHICS];

    void drawArrow(QPainter& painter, int x, int y, cc2::Tile::Direction direction) const;
    void drawGlyph(QPainter& painter, int x, int y, uint32_t glyph) const;
};

#endif
