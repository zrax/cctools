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

#include <QtGlobal>
#include <algorithm>
#include <cstring>

cc2::MapOption::MapOption()
    : m_view(View9x9), m_blobPattern(BlobsDeterministic), m_timeLimit(),
      m_solutionValid(), m_hidden(), m_readOnly(), m_hideLogic(),
      m_cc1Boots()
{
    memset(m_solutionMD5, 0, sizeof(m_solutionMD5));
}

void cc2::MapOption::setSolutionMD5(const uint8_t* md5)
{
    memcpy(m_solutionMD5, md5, sizeof(m_solutionMD5));
}

#define KNOWN_OPTION_LENGTH 25L

void cc2::MapOption::read(ccl::Stream* stream, long size)
{
    // We treat all fields as optional with a zero-default
    size_t alloc_size = (size_t)std::max(KNOWN_OPTION_LENGTH, size);
    uint8_t* buffer = new uint8_t[alloc_size];
    memset(buffer, 0, alloc_size);
    if (stream->read(buffer, 1, size) != size) {
        delete[] buffer;
        throw ccl::IOException("Read past end of stream");
    }

    const uint8_t* bufp = buffer;
    memcpy(&m_timeLimit, bufp, sizeof(m_timeLimit));
    bufp += sizeof(m_timeLimit);
    m_timeLimit = SWAP16(m_timeLimit);

    m_view = (Viewport)(*bufp++);
    m_solutionValid = (*bufp++) != 0;
    m_hidden = (*bufp++) != 0;
    m_readOnly = (*bufp++) != 0;

    memcpy(&m_solutionMD5, bufp, sizeof(m_solutionMD5));
    bufp += sizeof(m_solutionMD5);

    m_hideLogic = (*bufp++) != 0;
    m_cc1Boots = (*bufp++) != 0;
    m_blobPattern = (BlobPattern)(*bufp++);

    Q_ASSERT((bufp - buffer) == KNOWN_OPTION_LENGTH);
}

long cc2::MapOption::write(ccl::Stream* stream) const
{
    throw std::runtime_error("Not yet implemented");
}


void cc2::Tile::read(ccl::Stream* stream)
{
    m_type = stream->read8();
    if (m_type == Modifier1 || m_type == Modifier2) {
        m_modifiers[0] = stream->read8();
        if (m_type == Modifier2)
            m_modifiers[1] = stream->read8();
        m_type = stream->read8();
    }

    switch (m_type) {
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
    case Player2:
    case TimidTeeth:
    case YellowTank:
    case MirrorPlayer:
    case MirrorPlayer2:
    case Rover:
    case FloorMimic:
    case Ghost:
        m_direction = stream->read8();
        break;
    case PanelCanopy:
        m_panelFlags = stream->read8();
        break;
    case DirBlock:
        m_direction = stream->read8();
        m_arrowMask = stream->read8();
        break;
    default:
        // No extra data
        break;
    }

    if (haveLower()) {
        m_lower = new Tile;
        m_lower->read(stream);
    }
}

void cc2::Tile::write(ccl::Stream* stream) const
{
    throw std::runtime_error("Not yet implemented");
}

bool cc2::Tile::haveLower() const
{
    switch (m_type) {
    case Player:
    case DirtBlock:
    case Walker:
    case Ship:
    case IceBlock:
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
    case Player2:
    case TimidTeeth:
    case HikingBoots:
    case Lightning:
    case YellowTank:
    case MirrorPlayer:
    case MirrorPlayer2:
    case BowlingBall:
    case Rover:
    case TimePenalty:
    case PanelCanopy:
    case RRSign:
    case Flag10:
    case Flag100:
    case Flag1000:
    case Disallow:
    case Flag2x:
    case DirBlock:
    case FloorMimic:
    case GreenBomb:
    case GreenChip:
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


void cc2::MapData::read(ccl::Stream* stream, long size)
{
    long start = stream->tell();

    m_width = stream->read8();
    m_height = stream->read8();
    m_map = new Tile[m_width * m_height];
    for (size_t i = 0; i < (size_t)(m_width * m_height); ++i)
        m_map[i].read(stream);

    if (start + size != stream->tell())
        throw ccl::FormatException("Failed to parse map data");
}

long cc2::MapData::write(ccl::Stream* stream) const
{
    throw std::runtime_error("Not yet implemented");
}


void cc2::ReplayData::read(ccl::Stream* stream, long size)
{
    // TODO
    stream->seek(size, SEEK_CUR);
}

long cc2::ReplayData::write(ccl::Stream* stream) const
{
    throw std::runtime_error("Not yet implemented");
}


cc2::Map::Map() : m_version("7"), m_readOnly()
{
    memset(m_key, 0, sizeof(m_key));
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
            m_title = stream->readString(size);
        } else if (memcmp(tag, "AUTH", 4) == 0) {
            m_author = stream->readString(size);
        } else if (memcmp(tag, "VERS", 4) == 0) {
            m_editorVersion = stream->readString(size);
        } else if (memcmp(tag, "CLUE", 4) == 0) {
            m_clue = stream->readString(size);
        } else if (memcmp(tag, "NOTE", 4) == 0) {
            m_note = stream->readString(size);
        } else if (memcmp(tag, "OPTN", 4) == 0) {
            m_option.read(stream, (long)size);
        } else if (memcmp(tag, "MAP ", 4) == 0) {
            m_mapData.read(stream, (long)size);
        } else if (memcmp(tag, "PACK", 4) == 0) {
            std::unique_ptr<ccl::Stream> ustream(stream->unpack(size));
            m_mapData.read(ustream.get(), ustream->size());
        } else if (memcmp(tag, "KEY ", 4) == 0) {
            if (size != sizeof(m_key))
                throw ccl::FormatException("Invalid KEY field size");
            if (stream->read(&m_key, 1, sizeof(m_key)) != sizeof(m_key))
                throw ccl::IOException("Read past end of file");
        } else if (memcmp(tag, "REPL", 4) == 0) {
            m_replay.read(stream, (long)size);
        } else if (memcmp(tag, "PRPL", 4) == 0) {
            std::unique_ptr<ccl::Stream> ustream(stream->unpack(size));
            m_replay.read(ustream.get(), ustream->size());
        } else if (memcmp(tag, "RDNY", 4) == 0) {
            m_readOnly = true;
            stream->seek(size, SEEK_CUR);
        } else if (memcmp(tag, "END ", 4) == 0) {
            stream->seek(size, SEEK_CUR);
            break;
        }
    }

    if (!have_magic)
        throw ccl::FormatException("Invalid c2m file format");
}
