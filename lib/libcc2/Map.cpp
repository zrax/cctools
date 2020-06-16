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
      m_arrowMask(copy.m_arrowMask), m_panelFlags(copy.m_panelFlags),
      m_modifier(copy.m_modifier), m_lower()
{
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);
}

cc2::Tile& cc2::Tile::operator=(const Tile& copy)
{
    m_type = copy.m_type;
    m_direction = copy.m_direction;
    m_arrowMask = copy.m_arrowMask;
    m_panelFlags = copy.m_panelFlags;
    m_modifier = copy.m_modifier;
    auto lower = checkLower();
    if (lower && copy.m_lower)
        lower->operator=(*copy.m_lower);

    return *this;
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
    if (m_type == PanelCanopy)
        m_panelFlags = stream->read8();
    if (m_type == DirBlock)
        m_arrowMask = stream->read8();

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
    if (m_type == PanelCanopy)
        stream->write8(m_panelFlags);
    if (m_type == DirBlock)
        stream->write8(m_arrowMask);

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
    case UNUSED_Barrier_E:
    case UNUSED_Barrier_S:
    case UNUSED_Barrier_SE:
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

bool cc2::Tile::haveDirection() const
{
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

bool cc2::Tile::needArrows() const
{
    switch (m_type) {
    case DirtBlock:
    case IceBlock:
    case DirBlock:
        return m_lower->type() == CC1Cloner;
    default:
        return haveDirection();
    }
}

bool cc2::Tile::supportsWires() const
{
    switch (m_type) {
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

cc2::Tile* cc2::Tile::checkLower()
{
    if (!haveLower())
        return nullptr;
    if (!m_lower)
        m_lower = new Tile;
    return m_lower;
}


void cc2::MapData::read(ccl::Stream* stream, size_t size)
{
    long start = stream->tell();

    delete[] m_map;
    m_width = stream->read8();
    m_height = stream->read8();
    m_map = new Tile[m_width * m_height];
    for (size_t i = 0; i < (size_t)(m_width * m_height); ++i)
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
    const int lowerChips = tile->haveLower() ? tileChips(tile->lower()) : 0;

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
    auto points = tile->haveLower()
                ? tilePoints(tile->lower()) : std::make_tuple(0, 1);

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
            std::unique_ptr<ccl::Stream> ustream(stream->unpack(size));
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
            std::unique_ptr<ccl::Stream> ustream(stream->unpack(size));
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
            CC2FieldStorage unknown;
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

    // TODO: Pack MAP data
    writeTagged(stream, "MAP ", [&] { m_mapData.write(stream); });
    writeTaggedBlock<sizeof(m_key)>(stream, "KEY ", m_key);

    // Ensure any unrecognized fields are preserved upon write
    for (const auto& unknown : m_unknown) {
        writeTagged(stream, unknown.tag, [&] {
            stream->write(&unknown.data[0], 1, unknown.data.size());
        });
    }

    // TODO: Pack REPL data
    if (!m_replay.empty()) {
        writeTagged(stream, "REPL", [&] {
            stream->write(&m_replay[0], 1, m_replay.size());
        });
    }

    if (m_readOnly)
        writeTaggedBlock<0>(stream, "RDNY");

    // End of tagged data
    writeTaggedBlock<0>(stream, "END ");
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
