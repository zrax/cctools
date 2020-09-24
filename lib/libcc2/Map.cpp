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

#include "Map.h"
#include "libcc1/Levelset.h"

#include <QtGlobal>
#include <algorithm>
#include <memory>
#include <cstring>

void cc2::MapOption::setReplayMD5(const uint8_t* md5)
{
    memcpy(m_replayMD5, md5, sizeof(m_replayMD5));
}

#define KNOWN_OPTION_LENGTH 25

void cc2::MapOption::read(ccl::Stream* stream, size_t size)
{
    // We treat all fields as optional with a zero-default
    size_t alloc_size = std::max((size_t)KNOWN_OPTION_LENGTH, size);
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[alloc_size]);
    memset(buffer.get(), 0, alloc_size);
    if (stream->read(buffer.get(), 1, size) != size)
        throw ccl::IOException("Read past end of stream");

    const uint8_t* bufp = buffer.get();
    memcpy(&m_timeLimit, bufp, sizeof(m_timeLimit));
    bufp += sizeof(m_timeLimit);
    m_timeLimit = SWAP16(m_timeLimit);

    m_view = (Viewport)(*bufp++);
    m_replayValid = (*bufp++) != 0;
    m_hidden = (*bufp++) != 0;
    m_readOnly = (*bufp++) != 0;

    memcpy(&m_replayMD5, bufp, sizeof(m_replayMD5));
    bufp += sizeof(m_replayMD5);

    m_hideLogic = (*bufp++) != 0;
    m_cc1Boots = (*bufp++) != 0;
    m_blobPattern = (BlobPattern)(*bufp++);

    Q_ASSERT((bufp - buffer.get()) == KNOWN_OPTION_LENGTH);
}

void cc2::MapOption::write(ccl::Stream* stream) const
{
    // Write the whole block of known options, and truncate it if
    // possible to a shorter size.
    uint8_t buffer[KNOWN_OPTION_LENGTH];

    uint8_t* bufp = buffer;
    uint16_t timeLimit = SWAP16(m_timeLimit);
    memcpy(bufp, &timeLimit, sizeof(timeLimit));
    bufp += sizeof(timeLimit);

    *bufp++ = (uint8_t)m_view;
    *bufp++ = m_replayValid ? 1 : 0;
    *bufp++ = m_hidden ? 1 : 0;
    *bufp++ = m_readOnly ? 1 : 0;

    memcpy(bufp, &m_replayMD5, sizeof(m_replayMD5));
    bufp += sizeof(m_replayMD5);

    *bufp++ = m_hideLogic ? 1 : 0;
    *bufp++ = m_cc1Boots ? 1 : 0;
    *bufp++ = (uint8_t)m_blobPattern;

    Q_ASSERT((bufp - buffer) == KNOWN_OPTION_LENGTH);

    size_t writeLen = KNOWN_OPTION_LENGTH;
    while (writeLen > 3 && buffer[writeLen - 1] == 0)
        --writeLen;

    // Never cut in the middle of the replay checksum
    if (writeLen > 6 && writeLen < 22)
        writeLen = 22;

    if (stream->write(buffer, 1, writeLen) != writeLen)
        throw ccl::IOException("Error writing to stream");
}


cc2::Tile::Tile(const Tile& copy)
    : m_type(copy.m_type), m_direction(copy.m_direction),
      m_tileFlags(copy.m_tileFlags), m_modifier(copy.m_modifier), m_lower()
{
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);
}

cc2::Tile& cc2::Tile::operator=(const Tile& copy)
{
    m_type = copy.m_type;
    m_direction = copy.m_direction;
    m_tileFlags = copy.m_tileFlags;
    m_modifier = copy.m_modifier;
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);

    return *this;
}

cc2::Tile::Tile(Tile&& move) noexcept
    : m_type(move.m_type), m_direction(move.m_direction),
      m_tileFlags(move.m_tileFlags), m_modifier(move.m_modifier),
      m_lower(move.m_lower)
{
    move.m_lower = nullptr;
}

cc2::Tile& cc2::Tile::operator=(Tile&& move) noexcept
{
    m_type = move.m_type;
    m_direction = move.m_direction;
    m_tileFlags = move.m_tileFlags;
    m_modifier = move.m_modifier;
    std::swap(m_lower, move.m_lower);
    return *this;
}

bool cc2::Tile::operator==(const Tile& other) const
{
    if (m_type != other.m_type || m_direction != other.m_direction
        || m_tileFlags != other.m_tileFlags || m_modifier != other.m_modifier)
        return false;

    const cc2::Tile* myLower = lower();
    if (myLower)
        return myLower->operator==(*other.lower());
    return true;
}

void cc2::Tile::read(ccl::Stream* stream)
{
    m_type = stream->read8();
    if (m_type >= Modifier8 && m_type <= Modifier32) {
        switch (m_type) {
        case Modifier8:
            m_modifier = stream->read8();
            break;
        case Modifier16:
            m_modifier = stream->read16();
            break;
        case Modifier32:
            m_modifier = stream->read32();
            break;
        default:
            // Should never get here
            Q_ASSERT(false);
        }
        m_type = stream->read8();
    }

    if (haveDirection())
        m_direction = stream->read8();
    if (m_type == PanelCanopy || m_type == DirBlock)
        m_tileFlags = stream->read8();

    auto nextLayer = checkLower();
    if (nextLayer)
        nextLayer->read(stream);
}

void cc2::Tile::write(ccl::Stream* stream) const
{
    if (m_modifier > 0xFFFF) {
        stream->write8(Modifier32);
        stream->write32(m_modifier);
    } else if (m_modifier > 0xFF) {
        stream->write8(Modifier16);
        stream->write16((uint16_t)m_modifier);
    } else if (m_modifier != 0) {
        stream->write8(Modifier8);
        stream->write8((uint8_t)m_modifier);
    }

    stream->write8(m_type);

    if (haveDirection())
        stream->write8(m_direction);
    if (m_type == PanelCanopy || m_type == DirBlock)
        stream->write8(m_tileFlags);

    if (haveLower()) {
        Q_ASSERT(m_lower);
        m_lower->write(stream);
    }
}

bool cc2::Tile::haveLower(int type)
{
    switch (type) {
    case Player:
    case DirtBlock:
    case Walker:
    case Ship:
    case IceBlock:
    case CC1_Barrier_E:
    case CC1_Barrier_S:
    case CC1_Barrier_SE:
    case BlueTank:
    case Key_Red:
    case Key_Blue:
    case Key_Yellow:
    case Key_Green:
    case Chip:
    case ExtraChip:
    case Ant:
    case Centipede:
    case Ball:
    case Blob:
    case AngryTeeth:
    case FireBox:
    case IceCleats:
    case MagnoShoes:
    case FireShoes:
    case Flippers:
    case RedBomb:
    case TimeBonus:
    case ToggleClock:
    case TimeBomb:
    case Helmet:
    case UNUSED_53:
    case Player2:
    case TimidTeeth:
    case UNUSED_Explosion:
    case HikingBoots:
    case UNUSED_5d:
    case Lightning:
    case YellowTank:
    case MirrorPlayer:
    case MirrorPlayer2:
    case BowlingBall:
    case Rover:
    case TimePenalty:
    case PanelCanopy:
    case RRSign:
    case UNUSED_79:
    case Flag10:
    case Flag100:
    case Flag1000:
    case Disallow:
    case Flag2x:
    case DirBlock:
    case FloorMimic:
    case GreenBomb:
    case GreenChip:
    case UNUSED_85:
    case UNUSED_86:
    case Ghost:
    case SteelFoil:
    case Eye:
    case Bribe:
    case SpeedShoes:
    case Hook:
        return true;
    default:
        return false;
    }
}

bool cc2::Tile::haveDirection(int type)
{
    switch (type) {
    case Player:
    case DirtBlock:
    case Walker:
    case Ship:
    case IceBlock:
    case BlueTank:
    case Ant:
    case Centipede:
    case Ball:
    case Blob:
    case AngryTeeth:
    case FireBox:
    case UNUSED_53:
    case Player2:
    case TimidTeeth:
    case UNUSED_Explosion:
    case UNUSED_5d:
    case YellowTank:
    case MirrorPlayer:
    case MirrorPlayer2:
    case Rover:
    case UNUSED_79:
    case DirBlock:
    case FloorMimic:
    case Ghost:
        return true;
    default:
        return false;
    }
}

bool cc2::Tile::supportsWires(int type)
{
    switch (type) {
    case Floor:
    case Teleport_Red:
    case Teleport_Blue:
    case Transformer:
    case SteelWall:
    case LogicButton:
    case RevLogicButton:
    case Switch_Off:
    case Switch_On:
        return true;
    default:
        return false;
    }
}

cc2::Tile::TileClass cc2::Tile::tileClass(int type)
{
    // Classification for combine rules (see EditorWidget.cpp)

    switch (type) {
    case Key_Red:
    case Key_Blue:
    case Key_Yellow:
    case Key_Green:
    case Chip:
    case ExtraChip:
    case IceCleats:
    case MagnoShoes:
    case FireShoes:
    case Flippers:
    case RedBomb:
    case TimeBonus:
    case ToggleClock:
    case TimeBomb:
    case Helmet:
    case HikingBoots:
    case Lightning:
    case BowlingBall:
    case TimePenalty:
    case RRSign:
    case Flag10:
    case Flag100:
    case Flag1000:
    case Flag2x:
    case GreenBomb:
    case GreenChip:
    case SteelFoil:
    case Eye:
    case Bribe:
    case SpeedShoes:
    case Hook:
        return ClassItem;
    case Walker:
    case Ship:
    case BlueTank:
    case Ant:
    case Centipede:
    case Ball:
    case Blob:
    case AngryTeeth:
    case FireBox:
    case TimidTeeth:
    case YellowTank:
    case MirrorPlayer:
    case MirrorPlayer2:
    case Rover:
    case FloorMimic:
    case Ghost:
        return ClassCreature;
    case Player:
    case Player2:
        return ClassPlayer;
    case DirtBlock:
    case IceBlock:
    case DirBlock:
        return ClassBlock;
    case Floor:
    case Ice:
    case Ice_NE:
    case Ice_SE:
    case Ice_SW:
    case Ice_NW:
    case Water:
    case Fire:
    case Force_N:
    case Force_E:
    case Force_S:
    case Force_W:
    case ToggleWall:
    case ToggleFloor:
    case Slime:
    case Gravel:
    case ToggleButton:
    case TankButton:
    case PopUpWall:
    case AppearingWall:
    case InvisWall:
    case BlueWall:
    case BlueFloor:
    case Dirt:
    case CloneButton:
    case TrapButton:
    case ToolThief:
    case Trap:
    case Clue:
    case Force_Rand:
    case AreaCtlButton:
    case RevolvDoor_SW:
    case RevolvDoor_NW:
    case RevolvDoor_NE:
    case RevolvDoor_SE:
    case Transformer:
    case TrainTracks:
    case MaleOnly:
    case FemaleOnly:
    case LogicGate:
    case LogicButton:
    case FlameJet_Off:
    case FlameJet_On:
    case FlameJetButton:
    case YellowTankCtrl:
    case StyledFloor:
    case AsciiGlyph:
    case LSwitchFloor:
    case LSwitchWall:
    case StayUpGWall:
    case PopDownGWall:
    case RevLogicButton:
    case Switch_Off:
    case Switch_On:
    case KeyThief:
    case Turtle:
        return ClassTerrain;
    case CC1_Barrier_S:
    case CC1_Barrier_E:
    case CC1_Barrier_SE:
    case PanelCanopy:
        return ClassPanelCanopy;
    case Wall:
    case Teleport_Red:
    case Teleport_Blue:
    case Teleport_Yellow:
    case Teleport_Green:
    case Exit:
    case Door_Red:
    case Door_Blue:
    case Door_Yellow:
    case Door_Green:
    case Socket:
    case CC1_Cloner:
    case Cloner:
    case SteelWall:
    case StyledWall:
    case Disallow:
        return ClassOther;
    default:
        return ClassInvalid;
    }
}

bool cc2::Tile::needArrows() const
{
    switch (m_type) {
    case DirtBlock:
    case IceBlock:
    case DirBlock:
        return (m_lower->type() == CC1_Cloner)
            || (m_lower->type() == Cloner);
    default:
        return haveDirection();
    }
}

static constexpr uint8_t rol4(uint8_t bits)
{
    const uint8_t rbits = ((bits << 1) & 0x0f) | ((bits & 0x08) >> 3);
    return (bits & 0xf0) | rbits;
}

static constexpr  uint8_t ror4(uint8_t bits)
{
    const uint8_t rbits = ((bits >> 1) & 0x07) | ((bits & 0x01) << 3);
    return (bits & 0xf0) | rbits;
}

void cc2::Tile::rotateLeft()
{
    if (haveDirection())
        m_direction = (m_direction + 3) % 4;

    switch (m_type) {
    case Floor:
        // Actually rotating wire tunnels
        {
            uint8_t wires = ror4(m_modifier) & 0x0f;
            wires |= (wires << 4);
            m_modifier = (m_modifier & ~0xff) | wires;
        }
        break;
    case PanelCanopy:
    case DirBlock:
        m_tileFlags = ror4(m_tileFlags);
        break;
    case StyledFloor:
    case StyledWall:
        m_modifier = (m_modifier + 3) % 4;
        break;
    case Cloner:
        m_modifier = ror4(m_modifier);
        break;
    case Ice_NE:
        setType(Ice_NW);
        break;
    case Ice_SE:
        setType(Ice_NE);
        break;
    case Ice_SW:
        setType(Ice_SE);
        break;
    case Ice_NW:
        setType(Ice_SW);
        break;
    case Force_N:
        setType(Force_W);
        break;
    case Force_E:
        setType(Force_N);
        break;
    case Force_S:
        setType(Force_E);
        break;
    case Force_W:
        setType(Force_S);
        break;
    case RevolvDoor_SW:
        setType(RevolvDoor_SE);
        break;
    case RevolvDoor_NW:
        setType(RevolvDoor_SW);
        break;
    case RevolvDoor_NE:
        setType(RevolvDoor_NW);
        break;
    case RevolvDoor_SE:
        setType(RevolvDoor_NE);
        break;
    case TrainTracks:
        {
            uint32_t track = ror4(m_modifier);
            if (track & TileModifier::Track_NS)
                track = (track & ~TileModifier::Track_NS) | TileModifier::Track_WE;
            else if (track & TileModifier::Track_WE)
                track = (track & ~TileModifier::Track_WE) | TileModifier::Track_NS;
            m_modifier = track;
        }
        break;
    case Switch_Off:
        setType(Switch_On);
        break;
    case Switch_On:
        setType(Switch_Off);
        break;
    case LogicGate:
        if (m_modifier >= TileModifier::CounterGate_0 && m_modifier <= TileModifier::CounterGate_9) {
            m_modifier = ((m_modifier - cc2::TileModifier::CounterGate_0) + 9) % 10
                         + cc2::TileModifier::CounterGate_0;
        } else {
            m_modifier = ((m_modifier + 3) % 4) | (m_modifier & ~0x03);
        }
        break;
    default:
        break;
    }
}

void cc2::Tile::rotateRight()
{
    if (haveDirection())
        m_direction = (m_direction + 1) % 4;

    switch (m_type) {
    case Floor:
        // Actually rotating wire tunnels
        {
            uint8_t wires = rol4(m_modifier) & 0x0f;
            wires |= (wires << 4);
            m_modifier = (m_modifier & ~0xff) | wires;
        }
        break;
    case PanelCanopy:
    case DirBlock:
        m_tileFlags = rol4(m_tileFlags);
        break;
    case StyledFloor:
    case StyledWall:
        m_modifier = (m_modifier + 1) % 4;
        break;
    case Cloner:
        m_modifier = rol4(m_modifier);
        break;
    case Ice_NE:
        setType(Ice_SE);
        break;
    case Ice_SE:
        setType(Ice_SW);
        break;
    case Ice_SW:
        setType(Ice_NW);
        break;
    case Ice_NW:
        setType(Ice_NE);
        break;
    case Force_N:
        setType(Force_E);
        break;
    case Force_E:
        setType(Force_S);
        break;
    case Force_S:
        setType(Force_W);
        break;
    case Force_W:
        setType(Force_N);
        break;
    case RevolvDoor_SW:
        setType(RevolvDoor_NW);
        break;
    case RevolvDoor_NW:
        setType(RevolvDoor_NE);
        break;
    case RevolvDoor_NE:
        setType(RevolvDoor_SE);
        break;
    case RevolvDoor_SE:
        setType(RevolvDoor_SW);
        break;
    case TrainTracks:
        {
            uint32_t track = rol4(m_modifier);
            if (track & TileModifier::Track_NS)
                track = (track & ~TileModifier::Track_NS) | TileModifier::Track_WE;
            else if (track & TileModifier::Track_WE)
                track = (track & ~TileModifier::Track_WE) | TileModifier::Track_NS;
            m_modifier = track;
        }
        break;
    case Switch_Off:
        setType(Switch_On);
        break;
    case Switch_On:
        setType(Switch_Off);
        break;
    case LogicGate:
        if (m_modifier >= TileModifier::CounterGate_0 && m_modifier <= TileModifier::CounterGate_9) {
            m_modifier = ((m_modifier - TileModifier::CounterGate_0) + 1) % 10
                         + TileModifier::CounterGate_0;
        } else {
            m_modifier = ((m_modifier + 1) % 4) | (m_modifier & ~0x03);
        }
        break;
    default:
        break;
    }
}

cc2::Tile* cc2::Tile::checkLower()
{
    if (!haveLower())
        return nullptr;
    if (!m_lower)
        m_lower = new Tile;
    return m_lower;
}


cc2::MapData::MapData(const MapData& other)
    : m_width(other.m_width), m_height(other.m_height), m_map()
{
    if (other.m_map) {
        const size_t mapSize = m_width * m_height;
        m_map = new Tile[mapSize];
        for (size_t i = 0; i < mapSize; ++i)
            m_map[i] = other.m_map[i];
    }
}

cc2::MapData& cc2::MapData::operator=(const MapData& other)
{
    delete[] m_map;
    m_width = other.m_width;
    m_height = other.m_height;
    if (other.m_map) {
        const size_t mapSize = m_width * m_height;
        m_map = new Tile[mapSize];
        for (size_t i = 0; i < mapSize; ++i)
            m_map[i] = other.m_map[i];
    } else {
        m_map = nullptr;
    }
    return *this;
}

void cc2::MapData::read(ccl::Stream* stream, size_t size)
{
    long start = stream->tell();

    delete[] m_map;
    m_width = stream->read8();
    m_height = stream->read8();
    const size_t mapSize = m_width * m_height;
    m_map = new Tile[mapSize];
    for (size_t i = 0; i < mapSize; ++i)
        m_map[i].read(stream);

    if (start + (long)size != stream->tell())
        throw ccl::FormatException("Failed to parse map data");
}

void cc2::MapData::write(ccl::Stream* stream) const
{
    stream->write8(m_width);
    stream->write8(m_height);
    for (size_t i = 0; i < (size_t)(m_width * m_height); ++i)
        m_map[i].write(stream);
}

void cc2::MapData::resize(uint8_t width, uint8_t height)
{
    if (width == 0 || height == 0) {
        delete[] m_map;
        m_map = nullptr;
        m_width = 0;
        m_height = 0;
        return;
    }

    Tile* newMap = new Tile[width * height];

    // Copy the old map if possible
    uint8_t copyWidth = std::min(m_width, width);
    uint8_t copyHeight = std::min(m_height, height);
    for (uint8_t y = 0; y < copyHeight; ++y) {
        for (uint8_t x = 0; x < copyWidth; ++x)
            newMap[(y * width) + x] = m_map[(y * m_width) + x];
    }

    delete[] m_map;
    m_map = newMap;
    m_width = width;
    m_height = height;
}

static int tileChips(const cc2::Tile* tile)
{
    const cc2::Tile* lower = tile->lower();
    const int lowerChips = lower ? tileChips(lower) : 0;

    switch (tile->type()) {
    case cc2::Tile::Chip:
    case cc2::Tile::GreenChip:
    case cc2::Tile::GreenBomb:
        return 1 + lowerChips;
    default:
        return lowerChips;
    }
}

int cc2::MapData::countChips() const
{
    int chips = 0;
    const Tile* mapEnd = m_map + (m_width * m_height);
    for (const Tile* tp = m_map; tp != mapEnd; ++tp)
        chips += tileChips(tp);
    return chips;
}

static std::tuple<int, int> tilePoints(const cc2::Tile* tile)
{
    const cc2::Tile* lower = tile->lower();
    auto points = lower ? tilePoints(lower) : std::make_tuple(0, 1);

    switch (tile->type()) {
    case cc2::Tile::Flag10:
        std::get<0>(points) += 10;
        break;
    case cc2::Tile::Flag100:
        std::get<0>(points) += 100;
        break;
    case cc2::Tile::Flag1000:
        std::get<0>(points) += 1000;
        break;
    case cc2::Tile::Flag2x:
        std::get<1>(points) *= 2;
        break;
    default:
        break;
    }

    return points;
}

std::tuple<int, int> cc2::MapData::countPoints() const
{
    // Raw points and multiplier
    auto points = std::make_tuple(0, 1);
    const Tile* mapEnd = m_map + (m_width * m_height);
    for (const Tile* tp = m_map; tp != mapEnd; ++tp) {
        auto p = tilePoints(tp);
        std::get<0>(points) += std::get<0>(p);
        std::get<1>(points) *= std::get<1>(p);
    }
    return points;
}

static bool _haveTile(const cc2::Tile* tile, cc2::Tile::Type type)
{
    if (tile->type() == type)
        return true;
    const cc2::Tile* lower = tile->lower();
    if (lower)
        return _haveTile(lower, type);
    return false;
}

bool cc2::MapData::haveTile(int x, int y, Tile::Type type) const
{
    return _haveTile(&tile(x, y), type);
}


void cc2::Map::copyFrom(const cc2::Map* map)
{
    m_version = map->m_version;
    m_lock = map->m_lock;
    m_title = map->m_title;
    m_author = map->m_author;
    m_editorVersion = map->m_editorVersion;
    m_clue = map->m_clue;
    m_note = map->m_note;
    m_option = map->m_option;
    m_mapData = map->m_mapData;
    memcpy(m_key, map->m_key, sizeof(m_key));
    m_replay = map->m_replay;
    m_readOnly = map->m_readOnly;
    m_unknown = map->m_unknown;
}

static cc2::Tile mapCC1Tile(tile_t type, int& chipsLeft)
{
    using namespace cc2;

    switch (type) {
    case ccl::TileFloor:
        return Tile(Tile::Floor);
    case ccl::TileWall:
        return Tile(Tile::Wall);
    case ccl::TileChip:
        if (--chipsLeft >= 0)
            return Tile(Tile::Chip);
        else
            return Tile(Tile::ExtraChip);
    case ccl::TileWater:
        return Tile(Tile::Water);
    case ccl::TileFire:
        return Tile(Tile::Fire);
    case ccl::TileInvisWall:
        return Tile(Tile::InvisWall);
    case ccl::TileBarrier_N:
        return Tile::panelTile(Tile::PanelNorth);
    case ccl::TileBarrier_W:
        return Tile::panelTile(Tile::PanelWest);
    case ccl::TileBarrier_S:
        return Tile::panelTile(Tile::PanelSouth);
    case ccl::TileBarrier_E:
        return Tile::panelTile(Tile::PanelEast);
    case ccl::TileBlock:
        return Tile(Tile::DirtBlock);
    case ccl::TileDirt:
        return Tile(Tile::Dirt);
    case ccl::TileIce:
        return Tile(Tile::Ice);
    case ccl::TileForce_S:
        return Tile(Tile::Force_S);
    case ccl::TileBlock_N:
        return Tile(Tile::DirtBlock, Tile::North, 0);
    case ccl::TileBlock_W:
        return Tile(Tile::DirtBlock, Tile::West, 0);
    case ccl::TileBlock_S:
        return Tile(Tile::DirtBlock, Tile::South, 0);
    case ccl::TileBlock_E:
        return Tile(Tile::DirtBlock, Tile::East, 0);
    case ccl::TileForce_N:
        return Tile(Tile::Force_N);
    case ccl::TileForce_E:
        return Tile(Tile::Force_E);
    case ccl::TileForce_W:
        return Tile(Tile::Force_W);
    case ccl::TileExit:
        return Tile(Tile::Exit);
    case ccl::TileDoor_Blue:
        return Tile(Tile::Door_Blue);
    case ccl::TileDoor_Red:
        return Tile(Tile::Door_Red);
    case ccl::TileDoor_Green:
        return Tile(Tile::Door_Green);
    case ccl::TileDoor_Yellow:
        return Tile(Tile::Door_Yellow);
    case ccl::TileIce_SE:
        return Tile(Tile::Ice_SE);
    case ccl::TileIce_SW:
        return Tile(Tile::Ice_SW);
    case ccl::TileIce_NW:
        return Tile(Tile::Ice_NW);
    case ccl::TileIce_NE:
        return Tile(Tile::Ice_NE);
    case ccl::TileBlueFloor:
        return Tile(Tile::BlueFloor);
    case ccl::TileBlueWall:
        return Tile(Tile::BlueWall);
    case ccl::TileThief:
        return Tile(Tile::ToolThief);
    case ccl::TileSocket:
        return Tile(Tile::Socket);
    case ccl::TileToggleButton:
        return Tile(Tile::ToggleButton);
    case ccl::TileCloneButton:
        return Tile(Tile::CloneButton);
    case ccl::TileToggleWall:
        return Tile(Tile::ToggleWall);
    case ccl::TileToggleFloor:
        return Tile(Tile::ToggleFloor);
    case ccl::TileTrapButton:
        return Tile(Tile::TrapButton);
    case ccl::TileTankButton:
        return Tile(Tile::TankButton);
    case ccl::TileTeleport:
        return Tile(Tile::Teleport_Blue);
    case ccl::TileBomb:
        return Tile(Tile::RedBomb);
    case ccl::TileTrap:
        return Tile(Tile::Trap);
    case ccl::TileAppearingWall:
        return Tile(Tile::AppearingWall);
    case ccl::TileGravel:
        return Tile(Tile::Gravel);
    case ccl::TilePopUpWall:
        return Tile(Tile::PopUpWall);
    case ccl::TileHint:
        return Tile(Tile::Clue);
    case ccl::TileBarrier_SE:
        return Tile::panelTile(Tile::PanelSouth | Tile::PanelEast);
    case ccl::TileCloner:
        // Needs to be the CC1 variant to use the direction of the
        // monster or block tile above...
        return Tile(Tile::CC1_Cloner);
    case ccl::TileForce_Rand:
        return Tile(Tile::Force_Rand);
    case ccl::TileIceBlock:
        return Tile(Tile::IceBlock);
    case ccl::TileBug_N:
        return Tile(Tile::Ant, Tile::North, 0);
    case ccl::TileBug_W:
        return Tile(Tile::Ant, Tile::West, 0);
    case ccl::TileBug_S:
        return Tile(Tile::Ant, Tile::South, 0);
    case ccl::TileBug_E:
        return Tile(Tile::Ant, Tile::East, 0);
    case ccl::TileFireball_N:
        return Tile(Tile::FireBox, Tile::North, 0);
    case ccl::TileFireball_W:
        return Tile(Tile::FireBox, Tile::West, 0);
    case ccl::TileFireball_S:
        return Tile(Tile::FireBox, Tile::South, 0);
    case ccl::TileFireball_E:
        return Tile(Tile::FireBox, Tile::East, 0);
    case ccl::TileBall_N:
        return Tile(Tile::Ball, Tile::North, 0);
    case ccl::TileBall_W:
        return Tile(Tile::Ball, Tile::West, 0);
    case ccl::TileBall_S:
        return Tile(Tile::Ball, Tile::South, 0);
    case ccl::TileBall_E:
        return Tile(Tile::Ball, Tile::East, 0);
    case ccl::TileTank_N:
        return Tile(Tile::BlueTank, Tile::North, 0);
    case ccl::TileTank_W:
        return Tile(Tile::BlueTank, Tile::West, 0);
    case ccl::TileTank_S:
        return Tile(Tile::BlueTank, Tile::South, 0);
    case ccl::TileTank_E:
        return Tile(Tile::BlueTank, Tile::East, 0);
    case ccl::TileGlider_N:
        return Tile(Tile::Ship, Tile::North, 0);
    case ccl::TileGlider_W:
        return Tile(Tile::Ship, Tile::West, 0);
    case ccl::TileGlider_S:
        return Tile(Tile::Ship, Tile::South, 0);
    case ccl::TileGlider_E:
        return Tile(Tile::Ship, Tile::East, 0);
    case ccl::TileTeeth_N:
        return Tile(Tile::AngryTeeth, Tile::North, 0);
    case ccl::TileTeeth_W:
        return Tile(Tile::AngryTeeth, Tile::West, 0);
    case ccl::TileTeeth_S:
        return Tile(Tile::AngryTeeth, Tile::South, 0);
    case ccl::TileTeeth_E:
        return Tile(Tile::AngryTeeth, Tile::East, 0);
    case ccl::TileWalker_N:
        return Tile(Tile::Walker, Tile::North, 0);
    case ccl::TileWalker_W:
        return Tile(Tile::Walker, Tile::West, 0);
    case ccl::TileWalker_S:
        return Tile(Tile::Walker, Tile::South, 0);
    case ccl::TileWalker_E:
        return Tile(Tile::Walker, Tile::East, 0);
    case ccl::TileBlob_N:
        return Tile(Tile::Blob, Tile::North, 0);
    case ccl::TileBlob_W:
        return Tile(Tile::Blob, Tile::West, 0);
    case ccl::TileBlob_S:
        return Tile(Tile::Blob, Tile::South, 0);
    case ccl::TileBlob_E:
        return Tile(Tile::Blob, Tile::East, 0);
    case ccl::TileCrawler_N:
        return Tile(Tile::Centipede, Tile::North, 0);
    case ccl::TileCrawler_W:
        return Tile(Tile::Centipede, Tile::West, 0);
    case ccl::TileCrawler_S:
        return Tile(Tile::Centipede, Tile::South, 0);
    case ccl::TileCrawler_E:
        return Tile(Tile::Centipede, Tile::East, 0);
    case ccl::TileKey_Blue:
        return Tile(Tile::Key_Blue);
    case ccl::TileKey_Red:
        return Tile(Tile::Key_Red);
    case ccl::TileKey_Green:
        return Tile(Tile::Key_Green);
    case ccl::TileKey_Yellow:
        return Tile(Tile::Key_Yellow);
    case ccl::TileFlippers:
        return Tile(Tile::Flippers);
    case ccl::TileFireBoots:
        return Tile(Tile::FireShoes);
    case ccl::TileIceSkates:
        return Tile(Tile::IceCleats);
    case ccl::TileForceBoots:
        return Tile(Tile::MagnoShoes);
    case ccl::TilePlayer_N:
        return Tile(Tile::Player, Tile::North, 0);
    case ccl::TilePlayer_W:
        return Tile(Tile::Player, Tile::West, 0);
    case ccl::TilePlayer_S:
        return Tile(Tile::Player, Tile::South, 0);
    case ccl::TilePlayer_E:
        return Tile(Tile::Player, Tile::East, 0);
    default:
        return Tile(Tile::Invalid);
    }
}

void cc2::Map::importFrom(const ccl::LevelData* level, bool autoResize)
{
    m_version = "7";
    m_lock = std::string();
    m_title = level->name();
    m_author = std::string();
    m_editorVersion = std::string();
    m_clue = level->hint();     // TODO: Apply word wrap
    m_note = "Imported by CCTools 3.0";
    m_option.setView(MapOption::View9x9);
    m_option.setBlobPattern(MapOption::BlobsDeterministic);
    m_option.setTimeLimit(level->timer());
    m_option.setHidden(false);
    m_option.setReadOnly(false);
    m_option.setHideLogic(false);
    m_option.setCc1Boots(true);
    discardReplay();

    // Note:  This does not preserve non-standard button connections, monster
    //   order, or invalid tile upper/lower combinations, since cc2 maps don't
    //   support those features.
    m_mapData.resize(32, 32);
    int chipsLeft = level->chips();
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            Tile& upper = m_mapData.tile(x, y);
            upper = mapCC1Tile(level->map().getFG(x, y), chipsLeft);
            Tile* lower = upper.lower();
            if (lower)
                *lower = mapCC1Tile(level->map().getBG(x, y), chipsLeft);
        }
    }

    if (autoResize) {
        const Tile blankTile;
        int blankRows = 0;
        for (int y = 0; y < m_mapData.height(); ++y, ++blankRows) {
            bool rowEmpty = true;
            for (int x = 0; rowEmpty && x < m_mapData.width(); ++x) {
                if (m_mapData.tile(x, y) != blankTile)
                    rowEmpty = false;
            }
            if (!rowEmpty)
                break;
        }
        if (blankRows) {
            // Move tiles up blankCount rows
            for (int y = 0; y < m_mapData.height() - blankRows; ++y) {
                for (int x = 0; x < m_mapData.width(); ++x)
                    m_mapData.tile(x, y) = m_mapData.tile(x, y + blankRows);
            }
            m_mapData.resize(m_mapData.width(), m_mapData.height() - blankRows);
        }

        int blankCols = 0;
        for (int x = 0; x < m_mapData.width(); ++x, ++blankCols) {
            bool colEmpty = true;
            for (int y = 0; colEmpty && y < m_mapData.height(); ++y) {
                if (m_mapData.tile(x, y) != blankTile)
                    colEmpty = false;
            }
            if (!colEmpty)
                break;
        }
        if (blankCols) {
            // Move tiles left blankCount columns
            for (int x = 0; x < m_mapData.width() - blankCols; ++x) {
                for (int y = 0; y < m_mapData.height(); ++y)
                    m_mapData.tile(x, y) = m_mapData.tile(x + blankCols, y);
            }
            m_mapData.resize(m_mapData.width() - blankCols, m_mapData.height());
        }

        blankRows = 0;
        blankCols = 0;
        for (int y = m_mapData.height() - 1; y > 0; --y, ++blankRows) {
            bool rowEmpty = true;
            for (int x = 0; rowEmpty && x < m_mapData.width(); ++x) {
                if (m_mapData.tile(x, y) != blankTile)
                    rowEmpty = false;
            }
            if (!rowEmpty)
                break;
        }
        for (int x = m_mapData.width() - 1; x > 0; --x, ++blankCols) {
            bool colEmpty = true;
            for (int y = 0; colEmpty && y < m_mapData.height(); ++y) {
                if (m_mapData.tile(x, y) != blankTile)
                    colEmpty = false;
            }
            if (!colEmpty)
                break;
        }
        // Ensure 10x10 minimum size even with blank area removal
        m_mapData.resize(std::max(10, m_mapData.width() - blankCols),
                         std::max(10, m_mapData.height() - blankRows));
    }
}

static std::string toGenericLF(const std::string& text)
{
    std::string lftext = text;

    size_t start = 0;
    for ( ;; ) {
        start = lftext.find("\r\n", start);
        if (start == std::string::npos)
            break;

        lftext.erase(start, 1);
        start += 1;
    }

    return lftext;
}

static std::string toWindowsCRLF(const std::string& text)
{
    std::string crlftext = text;

    size_t start = 0;
    for ( ;; ) {
        start = crlftext.find("\n", start);
        if (start == std::string::npos)
            break;

        crlftext.insert(start, 1, '\r');
        start += 2;
    }

    return crlftext;
}

void cc2::Map::read(ccl::Stream* stream)
{
    char tag[4];
    uint32_t size;
    bool have_magic = false;

    for ( ;; ) {
        if (stream->read(tag, 1, sizeof(tag)) != sizeof(tag))
            throw ccl::IOException("Read past end of stream");
        size = stream->read32();

        if (memcmp(tag, "CC2M", 4) == 0) {
            m_version = stream->readString(size);
            have_magic = true;
        } else if (memcmp(tag, "LOCK", 4) == 0) {
            m_lock = stream->readString(size);
        } else if (memcmp(tag, "TITL", 4) == 0) {
            m_title = toGenericLF(stream->readString(size));
        } else if (memcmp(tag, "AUTH", 4) == 0) {
            m_author = toGenericLF(stream->readString(size));
        } else if (memcmp(tag, "VERS", 4) == 0) {
            m_editorVersion = stream->readString(size);
        } else if (memcmp(tag, "CLUE", 4) == 0) {
            m_clue = toGenericLF(stream->readString(size));
        } else if (memcmp(tag, "NOTE", 4) == 0) {
            m_note = toGenericLF(stream->readString(size));
        } else if (memcmp(tag, "OPTN", 4) == 0) {
            m_option.read(stream, size);
        } else if (memcmp(tag, "MAP ", 4) == 0) {
            m_mapData.read(stream, size);
        } else if (memcmp(tag, "PACK", 4) == 0) {
            std::unique_ptr<ccl::Stream> ustream = stream->unpack(size);
            m_mapData.read(ustream.get(), ustream->size());
        } else if (memcmp(tag, "KEY ", 4) == 0) {
            if (size != sizeof(m_key))
                throw ccl::FormatException("Invalid KEY field size");
            if (stream->read(&m_key, 1, sizeof(m_key)) != sizeof(m_key))
                throw ccl::IOException("Read past end of file");
        } else if (memcmp(tag, "REPL", 4) == 0) {
            m_replay.resize(size);
            stream->read(&m_replay[0], 1, size);
        } else if (memcmp(tag, "PRPL", 4) == 0) {
            std::unique_ptr<ccl::Stream> ustream = stream->unpack(size);
            m_replay.resize(ustream->size());
            ustream->read(&m_replay[0], 1, ustream->size());
        } else if (memcmp(tag, "RDNY", 4) == 0) {
            m_readOnly = true;
            stream->seek(size, SEEK_CUR);
        } else if (memcmp(tag, "END ", 4) == 0) {
            stream->seek(size, SEEK_CUR);
            break;
        } else {
            fprintf(stderr, "Warning: Unrecognized field '%c%c%c%c' in map file.\n",
                    tag[0], tag[1], tag[2], tag[3]);
            m_unknown.emplace_back();
            CC2FieldStorage& unknown = m_unknown.back();
            memcpy(unknown.tag, tag, sizeof(unknown.tag));
            unknown.data.resize(size);
            stream->read(&unknown.data[0], 1, size);
        }
    }

    if (!have_magic)
        throw ccl::FormatException("Invalid c2m file format");
}

template <typename Writer>
void writeTagged(ccl::Stream* stream, const char* tag, const Writer& writer)
{
    if (stream->write(tag, 1, 4) != 4)
        throw ccl::IOException("Error writing to stream");

    stream->write32(0);
    long start = stream->tell();
    writer();
    long end = stream->tell();

    // Go back and write the size word
    stream->seek(start - 4, SEEK_SET);
    stream->write32((uint32_t)(end - start));
    stream->seek(end, SEEK_SET);
}

static void writeTaggedString(ccl::Stream* stream, const char* tag,
                              const std::string& str)
{
    if (stream->write(tag, 1, 4) != 4)
        throw ccl::IOException("Error writing to stream");

    stream->write32(str.size() + 1);
    if (stream->write(str.c_str(), 1, str.size()) != str.size())
        throw ccl::IOException("Error writing to stream");

    // Nul-terminator
    stream->write8(0);
}

template <size_t FixedSize>
void writeTaggedBlock(ccl::Stream* stream, const char* tag, const void* data = nullptr)
{
    if (stream->write(tag, 1, 4) != 4)
        throw ccl::IOException("Error writing to stream");

    stream->write32(FixedSize);
    if (FixedSize) {
        if (stream->write(data, 1, FixedSize) != FixedSize)
            throw ccl::IOException("Error writing to stream");
    }
}

void cc2::Map::write(ccl::Stream* stream) const
{
    // Always required
    writeTaggedString(stream, "CC2M", m_version);

    if (!m_lock.empty())
        writeTaggedString(stream, "LOCK", m_lock);
    if (!m_title.empty())
        writeTaggedString(stream, "TITL", toWindowsCRLF(m_title));
    if (!m_author.empty())
        writeTaggedString(stream, "AUTH", toWindowsCRLF(m_author));
    if (!m_editorVersion.empty())
        writeTaggedString(stream, "VERS", m_editorVersion);
    if (!m_clue.empty())
        writeTaggedString(stream, "CLUE", toWindowsCRLF(m_clue));
    if (!m_note.empty())
        writeTaggedString(stream, "NOTE", toWindowsCRLF(m_note));

    // To match the maps that ship with CC2, we always write at least
    // 3 bytes of the OPTN field
    writeTagged(stream, "OPTN", [&] { m_option.write(stream); });

    ccl::BufferStream unpackedMap;
    m_mapData.write(&unpackedMap);
    writeTagged(stream, "PACK", [&] { stream->pack(&unpackedMap); });
    writeTaggedBlock<sizeof(m_key)>(stream, "KEY ", m_key);

    // Ensure any unrecognized fields are preserved upon write
    for (const auto& unknown : m_unknown) {
        writeTagged(stream, unknown.tag, [&] {
            stream->write(&unknown.data[0], 1, unknown.data.size());
        });
    }

    if (!m_replay.empty()) {
        ccl::BufferStream unpackedReplay;
        unpackedReplay.write(&m_replay[0], 1, m_replay.size());
        writeTagged(stream, "PRPL", [&] { stream->pack(&unpackedReplay); });
    }

    if (m_readOnly)
        writeTaggedBlock<0>(stream, "RDNY");

    // End of tagged data
    writeTaggedBlock<0>(stream, "END ");
}

std::string cc2::Map::clueForTile(int x, int y)
{
    if (!m_mapData.haveTile(x, y, Tile::Clue))
        return std::string();

    size_t start = 0;
    for (int sy = 0; sy < m_mapData.height(); ++sy) {
        for (int sx = 0; sx < m_mapData.width(); ++sx) {
            if (!m_mapData.haveTile(sx, sy, Tile::Clue))
                continue;

            // Scan the NOTE for the next [CLUE] tag
            start = m_note.find("[CLUE]", start);
            if (start == std::string::npos)
                return m_clue;

            // Find the newline...  CC2 discards anything else on the same
            // line as the [CLUE] tag.
            start = m_note.find('\n', start);
            if (start == std::string::npos)
                return m_clue;
            ++start;

            if (sx == x && sy == y) {
                size_t next = m_note.find("[CLUE]", start);
                if (next == std::string::npos)
                    return m_clue;
                return m_note.substr(start, next - start);
            }
        }
    }

    Q_UNREACHABLE();
}

void cc2::Map::setClueForTile(int x, int y, const std::string& clue)
{
    if (!m_mapData.haveTile(x, y, Tile::Clue))
        return;

    // This function always assumes we want to directly update the tile-specific
    // clue, rather than the global one in m_clue.  If there aren't enough
    // clue fields in the NOTE, we will add tags as necessary.

    size_t start = 0;
    for (int sy = 0; sy < m_mapData.height(); ++sy) {
        for (int sx = 0; sx < m_mapData.width(); ++sx) {
            if (!m_mapData.haveTile(sx, sy, Tile::Clue))
                continue;

            // Scan the NOTE for the next [CLUE] tag
            start = m_note.find("[CLUE]", start);
            if (start == std::string::npos) {
                if (!m_note.empty() && m_note.back() != '\n')
                    m_note += "\n";
                m_note += "[CLUE]\n";
                start = m_note.size();
            } else {
                // Find the newline...  CC2 discards anything else on the same
                // line as the [CLUE] tag.
                start = m_note.find('\n', start);
                if (start == std::string::npos) {
                    m_note += "\n";
                    start = m_note.size();
                } else {
                    ++start;
                }
            }

            if (sx == x && sy == y) {
                size_t next = m_note.find("[CLUE]", start);
                if (next == std::string::npos) {
                    m_note += clue;
                    if (!m_note.empty() && m_note.back() != '\n')
                        m_note += "\n";
                    m_note += "[CLUE]\n";
                } else {
                    if (!clue.empty() && clue.back() != '\n')
                        m_note.replace(start, next - start, clue + "\n");
                    else
                        m_note.replace(start, next - start, clue);
                }
                return;
            }
        }
    }

    Q_UNREACHABLE();
}


#define SAVE_DATA_SIZE 100

void cc2::SaveData::read(ccl::Stream* stream, size_t size)
{
    // We treat all fields as optional with a zero-default
    size_t alloc_size = std::max((size_t)SAVE_DATA_SIZE, size);
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[alloc_size]);
    memset(buffer.get(), 0, alloc_size);
    if (stream->read(buffer.get(), 1, size) != size)
        throw ccl::IOException("Read past end of stream");

    const uint8_t* bufp = buffer.get();
    auto read32 = [&bufp](uint32_t& value) {
        memcpy(&value, bufp, sizeof(uint32_t));
        bufp += sizeof(uint32_t);
        value = SWAP32(value);
    };

    read32(m_line);
    read32(m_score);
    read32(m_scoreBonus);
    read32(m_timeLeft);
    read32(m_chipsLeft);
    read32(m_bonus);
    read32(m_level);
    read32(m_time);
    read32(m_tries);
    read32(m_gender);
    read32(m_enter);
    read32(m_exit);
    read32(m_finished);
    read32(m_result);
    read32(m_reg1);
    read32(m_reg2);
    read32(m_reg3);
    read32(m_reg4);
    read32(m_menu);
    read32(m_flags);
    read32(m_tools);
    read32(m_keys);
    read32(m_lastTime);
    read32(m_levelScore);
    read32(m_checksum);

    Q_ASSERT((bufp - buffer.get()) == SAVE_DATA_SIZE);
}

void cc2::SaveData::write(ccl::Stream* stream) const
{
    // Write the whole block of known options, and truncate it if
    // possible to a shorter size.
    uint8_t buffer[SAVE_DATA_SIZE];

    uint8_t* bufp = buffer;
    auto write32 = [&bufp](const uint32_t& value) {
        uint32_t wval = SWAP32(value);
        memcpy(bufp, &wval, sizeof(uint32_t));
        bufp += sizeof(uint32_t);
    };

    write32(m_line);
    write32(m_score);
    write32(m_scoreBonus);
    write32(m_timeLeft);
    write32(m_chipsLeft);
    write32(m_bonus);
    write32(m_level);
    write32(m_time);
    write32(m_tries);
    write32(m_gender);
    write32(m_enter);
    write32(m_exit);
    write32(m_finished);
    write32(m_result);
    write32(m_reg1);
    write32(m_reg2);
    write32(m_reg3);
    write32(m_reg4);
    write32(m_menu);
    write32(m_flags);
    write32(m_tools);
    write32(m_keys);
    write32(m_lastTime);
    write32(m_levelScore);
    write32(m_checksum);

    Q_ASSERT((bufp - buffer) == SAVE_DATA_SIZE);
    if (stream->write(buffer, 1, SAVE_DATA_SIZE) != SAVE_DATA_SIZE)
        throw ccl::IOException("Error writing to stream");
}


void cc2::CC2HighScore::read(ccl::Stream* stream)
{
    char tag[4];
    uint32_t size;
    bool have_magic = false;

    for ( ;; ) {
        if (stream->read(tag, 1, sizeof(tag)) != sizeof(tag))
            throw ccl::IOException("Read past end of stream");
        size = stream->read32();

        if (memcmp(tag, "CC2H", 4) == 0) {
            m_version = stream->readString(size);
            have_magic = true;
        } else if (memcmp(tag, "FILE", 4) == 0) {
            m_scores.emplace_back();
            m_scores.back().setFilename(stream->readString(size));
        } else if (memcmp(tag, "TYPE", 4) == 0) {
            if (m_scores.empty())
                throw ccl::IOException("Got a TYPE field with no FILE");
            m_scores.back().setGameType(stream->readString(size));
        } else if (memcmp(tag, "NAME", 4) == 0) {
            if (m_scores.empty())
                throw ccl::IOException("Got a NAME field with no FILE");
            m_scores.back().setTitle(stream->readString(size));
        } else if (memcmp(tag, "SAVE", 4) == 0) {
            if (m_scores.empty())
                throw ccl::IOException("Got a SAVE field with no FILE");
            m_scores.back().saveData().read(stream, size);
        } else if (memcmp(tag, "END ", 4) == 0) {
            stream->seek(size, SEEK_CUR);
            break;
        } else {
            fprintf(stderr, "Warning: Unrecognized field '%c%c%c%c' in c2h file.\n",
                    tag[0], tag[1], tag[2], tag[3]);
            CC2FieldStorage unknown;
            memcpy(unknown.tag, tag, sizeof(unknown.tag));
            unknown.data.resize(size);
            stream->read(&unknown.data[0], 1, size);
        }
    }

    if (!have_magic)
        throw ccl::FormatException("Invalid c2h file format");
}

void cc2::CC2HighScore::write(ccl::Stream* stream) const
{
    writeTaggedString(stream, "CC2H", m_version);

    for (const auto& score : m_scores) {
        writeTaggedString(stream, "FILE", score.filename());
        writeTaggedString(stream, "TYPE", score.gameType());
        writeTaggedString(stream, "NAME", score.title());
        writeTagged(stream, "SAVE", [&] { score.saveData().write(stream); });
    }

    // End of tagged data
    writeTaggedBlock<0>(stream, "END ");
}


void cc2::CC2Save::read(ccl::Stream* stream)
{
    char tag[4];
    uint32_t size;
    bool have_magic = false;

    for ( ;; ) {
        if (stream->read(tag, 1, sizeof(tag)) != sizeof(tag))
            throw ccl::IOException("Read past end of stream");
        size = stream->read32();

        if (memcmp(tag, "CC2S", 4) == 0) {
            m_version = stream->readString(size);
            have_magic = true;
        } else if (memcmp(tag, "FILE", 4) == 0) {
            m_filename = stream->readString(size);
        } else if (memcmp(tag, "PATH", 4) == 0) {
            m_gamePath = stream->readString(size);
        } else if (memcmp(tag, "GAME", 4) == 0) {
            m_save.read(stream, size);
        } else if (memcmp(tag, "END ", 4) == 0) {
            stream->seek(size, SEEK_CUR);
            break;
        } else {
            fprintf(stderr, "Warning: Unrecognized field '%c%c%c%c' in c2h file.\n",
                    tag[0], tag[1], tag[2], tag[3]);
            CC2FieldStorage unknown;
            memcpy(unknown.tag, tag, sizeof(unknown.tag));
            unknown.data.resize(size);
            stream->read(&unknown.data[0], 1, size);
        }
    }

    if (!have_magic)
        throw ccl::FormatException("Invalid c2h file format");
}

void cc2::CC2Save::write(ccl::Stream* stream) const
{
    writeTaggedString(stream, "CC2S", m_version);

    writeTaggedString(stream, "FILE", m_filename);
    writeTaggedString(stream, "PATH", m_gamePath);
    writeTagged(stream, "SAVE", [&] { m_save.write(stream); });

    // End of tagged data
    writeTaggedBlock<0>(stream, "END ");
}
