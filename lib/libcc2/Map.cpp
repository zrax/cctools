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
      m_replayValid(), m_hidden(), m_readOnly(), m_hideLogic(),
      m_cc1Boots()
{
    memset(m_replayMD5, 0, sizeof(m_replayMD5));
}

void cc2::MapOption::setReplayMD5(const uint8_t* md5)
{
    memcpy(m_replayMD5, md5, sizeof(m_replayMD5));
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
    m_replayValid = (*bufp++) != 0;
    m_hidden = (*bufp++) != 0;
    m_readOnly = (*bufp++) != 0;

    memcpy(&m_replayMD5, bufp, sizeof(m_replayMD5));
    bufp += sizeof(m_replayMD5);

    m_hideLogic = (*bufp++) != 0;
    m_cc1Boots = (*bufp++) != 0;
    m_blobPattern = (BlobPattern)(*bufp++);

    Q_ASSERT((bufp - buffer) == KNOWN_OPTION_LENGTH);
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
      m_arrowMask(copy.m_arrowMask)
{
    memcpy(m_modifiers, copy.m_modifiers, sizeof(m_modifiers));
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);
}

cc2::Tile& cc2::Tile::operator=(const Tile& copy)
{
    m_type = copy.m_type;
    m_direction = copy.m_direction;
    m_arrowMask = copy.m_arrowMask;
    memcpy(m_modifiers, copy.m_modifiers, sizeof(m_modifiers));
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);

    return *this;
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

    auto nextLayer = checkLower();
    if (nextLayer)
        nextLayer->read(stream);
}

void cc2::Tile::write(ccl::Stream* stream) const
{
    if (m_modifiers[1] != 0) {
        stream->write8(Modifier2);
        stream->write8(m_modifiers[0]);
        stream->write8(m_modifiers[1]);
    } else if (m_modifiers[0] != 0) {
        stream->write8(Modifier1);
        stream->write8(m_modifiers[0]);
    }

    stream->write8(m_type);

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
        stream->write8(m_direction);
        break;
    case PanelCanopy:
        stream->write8(m_panelFlags);
        break;
    case DirBlock:
        stream->write8(m_direction);
        stream->write8(m_arrowMask);
        break;
    default:
        // No extra data
        break;
    }

    if (haveLower()) {
        Q_ASSERT(m_lower);
        m_lower->write(stream);
    }
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

cc2::Tile* cc2::Tile::checkLower()
{
    if (!haveLower())
        return 0;
    if (!m_lower)
        m_lower = new Tile;
    return m_lower;
}


void cc2::MapData::read(ccl::Stream* stream, long size)
{
    long start = stream->tell();

    delete[] m_map;
    m_width = stream->read8();
    m_height = stream->read8();
    m_map = new Tile[m_width * m_height];
    for (size_t i = 0; i < (size_t)(m_width * m_height); ++i)
        m_map[i].read(stream);

    if (start + size != stream->tell())
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
        m_map = 0;
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


void cc2::ReplayData::read(ccl::Stream* stream, long size)
{
    long start = stream->tell();

    m_flag = stream->read8();
    m_initRandDir = (Tile::Direction)stream->read8();
    m_randSeed = stream->read8();

    m_input.clear();
    while (stream->tell() < start + size) {
        uint8_t frames = stream->read8();
        uint8_t action = stream->read8();
        if (frames == 0xff)
            break;

        // Merge split action groups for easier processing
        if (!m_input.empty() && m_input.back().action() == action)
            m_input.back().addFrames(frames);
        else
            m_input.emplace_back(frames, action);
    }

    if (start + size != stream->tell())
        throw ccl::FormatException("Failed to parse replay data");
}

void cc2::ReplayData::write(ccl::Stream* stream) const
{
    stream->write8(m_flag);
    stream->write8((uint8_t)m_initRandDir);
    stream->write8(m_randSeed);

    for (const auto& input : m_input) {
        int frames = input.frames();
        while (frames > 0) {
            if (frames > 0xfc) {
                stream->write8(0xfc);
                frames -= 0xfc;
            } else {
                stream->write8((uint8_t)frames);
                frames = 0;
            }
            stream->write8(input.action());
        }
    }

    // Input list terminator
    stream->write8(0xff);
    stream->write8(0);
}


cc2::Map::Map() : m_version("7"), m_readOnly()
{
    memset(m_key, 0, sizeof(m_key));
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

template <typename Writer>
void writeTagged(ccl::Stream* stream, const char* tag, Writer& writer)
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
void writeTaggedBlock(ccl::Stream* stream, const char* tag, const void* data = 0)
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
    writeTagged(stream, "OPTN", [&]() { m_option.write(stream); });

    // TODO: Pack MAP data
    writeTagged(stream, "MAP ", [&]() { m_mapData.write(stream); });
    writeTaggedBlock<sizeof(m_key)>(stream, "KEY ", m_key);

    // TODO: Pack REPL data
    writeTagged(stream, "REPL", [&]() { m_replay.write(stream); });

    if (m_readOnly)
        writeTaggedBlock<0>(stream, "RDNY");

    // End of tagged data
    writeTaggedBlock<0>(stream, "END ");
}
