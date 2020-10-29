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

#include "Levelset.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>

#ifdef _MSC_VER
    #define snprintf _sprintf_p
#endif

ccl::LevelMap::LevelMap()
{
    memset(m_fgTiles, 0, CCL_WIDTH * CCL_HEIGHT * sizeof(tile_t));
    memset(m_bgTiles, 0, CCL_WIDTH * CCL_HEIGHT * sizeof(tile_t));
}

ccl::LevelMap& ccl::LevelMap::operator=(const ccl::LevelMap& source)
{
    memcpy(m_fgTiles, source.m_fgTiles, CCL_WIDTH * CCL_HEIGHT * sizeof(tile_t));
    memcpy(m_bgTiles, source.m_bgTiles, CCL_WIDTH * CCL_HEIGHT * sizeof(tile_t));
    return *this;
}

void ccl::LevelMap::copyFrom(const ccl::LevelMap& source, int srcX, int srcY,
                             int destX, int destY, int width, int height)
{
    if (srcX < 0 || srcX >= CCL_WIDTH || srcY < 0 || srcY >= CCL_HEIGHT)
        return;
    if (destX < 0 || destX >= CCL_WIDTH || destY < 0 || destY >= CCL_HEIGHT)
        return;

    width = std::min(width, CCL_WIDTH - destX);
    height = std::min(height, CCL_HEIGHT - destY);

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            m_fgTiles[((y+destY) * CCL_WIDTH) + (x+destX)]
                = source.m_fgTiles[((y+srcY) * CCL_WIDTH) + (x+srcX)];
            m_bgTiles[((y+destY) * CCL_WIDTH) + (x+destX)]
                = source.m_bgTiles[((y+srcY) * CCL_WIDTH) + (x+srcX)];
        }
    }
}

void ccl::LevelMap::push(int x, int y, tile_t tile)
{
    m_bgTiles[(y*CCL_WIDTH) + x] = m_fgTiles[(y*CCL_WIDTH) + x];
    m_fgTiles[(y*CCL_WIDTH) + x] = tile;
}

tile_t ccl::LevelMap::pop(int x, int y)
{
    tile_t tile = m_fgTiles[(y*CCL_WIDTH) + x];
    m_fgTiles[(y*CCL_WIDTH) + x] = m_bgTiles[(y*CCL_WIDTH) + x];
    m_bgTiles[(y*CCL_WIDTH) + x] = 0;
    return tile;
}

long ccl::LevelMap::read(ccl::Stream* stream)
{
    long begin = stream->tell();
    stream->readRLE(m_fgTiles, CCL_WIDTH * CCL_HEIGHT);
    stream->readRLE(m_bgTiles, CCL_WIDTH * CCL_HEIGHT);
    return stream->tell() - begin;
}

long ccl::LevelMap::write(ccl::Stream* stream) const
{
    long outsize = 0;
    outsize += stream->writeRLE(m_fgTiles, CCL_WIDTH * CCL_HEIGHT);
    outsize += stream->writeRLE(m_bgTiles, CCL_WIDTH * CCL_HEIGHT);
    return outsize;
}

ccl::Point ccl::LevelMap::findNext(int x, int y, tile_t tile) const
{
    ccl::Point result;
    result.X = x;
    result.Y = y;
    for ( ;; ) {
        if (++result.X >= 32) {
            if (++result.Y >= 32)
                result.Y = 0;
            result.X = 0;
        }
        if (getFG(result.X, result.Y) == tile || getBG(result.X, result.Y) == tile)
            return result;
        if (result.X == x && result.Y == y) {
            result.X = -1;
            result.Y = -1;
            return result;
        }
    }
}


void ccl::LevelData::copyFrom(const ccl::LevelData* init)
{
    m_map = init->m_map;
    m_name = init->m_name;
    m_hint = init->m_hint;
    m_password = init->m_password;
    m_levelNum = init->m_levelNum;
    m_chips = init->m_chips;
    m_timer = init->m_timer;
    m_traps = init->m_traps;
    m_clones = init->m_clones;
    m_moveList = init->m_moveList;
}

std::list<ccl::Point> ccl::LevelData::linkedTraps(int x, int y) const
{
    std::list<ccl::Point> result;
    std::list<ccl::Trap>::const_iterator iter = m_traps.begin();
    while (iter != m_traps.end()) {
        if (iter->button.X == x && iter->button.Y == y)
            result.push_back(iter->trap);
        ++iter;
    }
    return result;
}

std::list<ccl::Point> ccl::LevelData::linkedTrapButtons(int x, int y) const
{
    std::list<ccl::Point> result;
    std::list<ccl::Trap>::const_iterator iter = m_traps.begin();
    while (iter != m_traps.end()) {
        if (iter->trap.X == x && iter->trap.Y == y)
            result.push_back(iter->button);
        ++iter;
    }
    return result;
}

std::list<ccl::Point> ccl::LevelData::linkedCloners(int x, int y) const
{
    std::list<ccl::Point> result;
    std::list<ccl::Clone>::const_iterator iter = m_clones.begin();
    while (iter != m_clones.end()) {
        if (iter->button.X == x && iter->button.Y == y)
            result.push_back(iter->clone);
        ++iter;
    }
    return result;
}

std::list<ccl::Point> ccl::LevelData::linkedCloneButtons(int x, int y) const
{
    std::list<ccl::Point> result;
    std::list<ccl::Clone>::const_iterator iter = m_clones.begin();
    while (iter != m_clones.end()) {
        if (iter->clone.X == x && iter->clone.Y == y)
            result.push_back(iter->button);
        ++iter;
    }
    return result;
}

bool ccl::LevelData::checkMove(int x, int y) const
{
    std::list<ccl::Point>::const_iterator iter = m_moveList.begin();
    while (iter != m_moveList.end()) {
        if (iter->X == x && iter->Y == y)
            return true;
        ++iter;
    }
    return false;
}

void ccl::LevelData::trapConnect(int buttonX, int buttonY, int trapX, int trapY)
{
    std::list<ccl::Trap>::iterator iter = m_traps.begin();
    while (iter != m_traps.end()) {
        if (iter->button.X == buttonX && iter->button.Y == buttonY
            && iter->trap.X == trapX && iter->trap.Y == trapY)
            return;
        else
            ++iter;
    }

    ccl::Trap item;
    item.button.X = buttonX;
    item.button.Y = buttonY;
    item.trap.X = trapX;
    item.trap.Y = trapY;
    m_traps.push_back(item);
}

void ccl::LevelData::cloneConnect(int buttonX, int buttonY, int cloneX, int cloneY)
{
    std::list<ccl::Clone>::iterator iter = m_clones.begin();
    while (iter != m_clones.end()) {
        if (iter->button.X == buttonX && iter->button.Y == buttonY
            && iter->clone.X == cloneX && iter->clone.Y == cloneY)
            return;
        else
            ++iter;
    }

    ccl::Clone item;
    item.button.X = buttonX;
    item.button.Y = buttonY;
    item.clone.X = cloneX;
    item.clone.Y = cloneY;
    m_clones.push_back(item);
}

void ccl::LevelData::addMover(int moverX, int moverY)
{
    std::list<ccl::Point>::iterator iter = m_moveList.begin();
    while (iter != m_moveList.end()) {
        if (iter->X == moverX && iter->Y == moverY)
            return;
        else
            ++iter;
    }

    ccl::Point item;
    item.X = moverX;
    item.Y = moverY;
    m_moveList.push_back(item);
}

long ccl::LevelData::read(ccl::Stream* stream, bool forClipboard)
{
    long levelBegin = stream->tell();
    long dataSize = forClipboard ? 0 : (long)stream->read16();

    m_levelNum = stream->read16();
    m_timer = stream->read16();
    m_chips = stream->read16();
    dataSize -= 3 * sizeof(unsigned short);

    if (stream->read16() != 1)
        throw ccl::IOError(ccl::RuntimeError::tr("Invalid map data field"));
    dataSize -= m_map.read(stream) + sizeof(unsigned short);

    dataSize -= sizeof(unsigned short);
    if (forClipboard) {
        dataSize = (long)stream->read16();
    } else {
        long fieldSize = (long)stream->read16();
        if (fieldSize != dataSize) {
            fprintf(stderr, "Warning: Ignoring invalid field data size: %ld (expected %ld)\n",
                    fieldSize, dataSize);
        }
    }

    while (dataSize > 0) {
        unsigned char field = stream->read8();
        unsigned char size = stream->read8();
        dataSize -= size + 2 * sizeof(unsigned char);
        if (dataSize < 0)
            throw ccl::IOError(ccl::RuntimeError::tr("Invalid or corrupt level data"));

        switch (field) {
        case FieldName:
            m_name = stream->readString(size);
            break;
        case FieldHint:
            m_hint = stream->readString(size);
            break;
        case FieldPassword:
            m_password = stream->readString(size, true);
            break;
        case FieldTraps:
            if ((size % 10) != 0)
                throw ccl::IOError(ccl::RuntimeError::tr("Invalid trap field size"));
            for (size_t i=0; i<size; i += 10) {
                ccl::Trap trap;
                trap.button.X = stream->read16();
                trap.button.Y = stream->read16();
                trap.trap.X = stream->read16();
                trap.trap.Y = stream->read16();
                stream->read16(); // Internal trap state, unused by CCTools
                m_traps.push_back(trap);
            }
            break;
        case FieldClones:
            if ((size % 8) != 0)
                throw ccl::IOError(ccl::RuntimeError::tr("Invalid clone field size"));
            for (size_t i=0; i<size; i += 8) {
                ccl::Clone clone;
                clone.button.X = stream->read16();
                clone.button.Y = stream->read16();
                clone.clone.X = stream->read16();
                clone.clone.Y = stream->read16();
                m_clones.push_back(clone);
            }
            break;
        case FieldMoveList:
            if ((size % 2) != 0)
                throw ccl::IOError(ccl::RuntimeError::tr("Invalid move list field size"));
            for (size_t i=0; i<size; i += 2) {
                ccl::Point mover;
                mover.X = stream->read8();
                mover.Y = stream->read8();
                m_moveList.push_back(mover);
            }
            break;
        default:
            throw ccl::IOError(ccl::RuntimeError::tr("Invalid / unrecognized field type"));
        }
    }

    if (dataSize != 0)
        throw ccl::IOError(forClipboard ? ccl::RuntimeError::tr("Corrupt level data")
                    : ccl::RuntimeError::tr("Invalid level checksum"));
    return stream->tell() - levelBegin;
}

long ccl::LevelData::write(ccl::Stream* stream, bool forClipboard) const
{
    long levelBegin = stream->tell();
    if (!forClipboard)
        stream->write16(0); // This will be updated at the end

    stream->write16(m_levelNum);
    stream->write16(m_timer);
    stream->write16(m_chips);

    // Map data
    stream->write16(1);
    m_map.write(stream);

    long fieldBegin = stream->tell();
    stream->write16(0); // This will be updated at the end

    if (!m_name.empty()) {
        stream->write8((uint8_t)FieldName);
        stream->write8((uint8_t)(m_name.size() + 1));
        stream->writeString(m_name);
    }
    if (!m_hint.empty()) {
        stream->write8((uint8_t)FieldHint);
        stream->write8((uint8_t)(m_hint.size() + 1));
        stream->writeString(m_hint);
    }
    if (!m_password.empty()) {
        stream->write8((uint8_t)FieldPassword);
        stream->write8((uint8_t)(m_password.size() + 1));
        stream->writeString(m_password, true);
    }
    if (m_traps.size() > 0) {
        stream->write8((uint8_t)FieldTraps);
        stream->write8((uint8_t)(m_traps.size() * 10));
        std::list<ccl::Trap>::const_iterator it;
        for (it = m_traps.begin(); it != m_traps.end(); ++it) {
            stream->write16(it->button.X);
            stream->write16(it->button.Y);
            stream->write16(it->trap.X);
            stream->write16(it->trap.Y);
            stream->write16(0);
        }
    }
    if (m_clones.size() > 0) {
        stream->write8((uint8_t)FieldClones);
        stream->write8((uint8_t)(m_clones.size() * 8));
        std::list<ccl::Clone>::const_iterator it;
        for (it = m_clones.begin(); it != m_clones.end(); ++it) {
            stream->write16(it->button.X);
            stream->write16(it->button.Y);
            stream->write16(it->clone.X);
            stream->write16(it->clone.Y);
        }
    }
    if (m_moveList.size() > 0) {
        stream->write8((uint8_t)FieldMoveList);
        stream->write8((uint8_t)(m_moveList.size() * 2));
        std::list<ccl::Point>::const_iterator it;
        for (it = m_moveList.begin(); it != m_moveList.end(); ++it) {
            stream->write8(it->X);
            stream->write8(it->Y);
        }
    }

    // Update checksums
    long levelEnd = stream->tell();
    if (!forClipboard) {
        stream->seek(levelBegin, SEEK_SET);
        stream->write16((uint16_t)(levelEnd - levelBegin - sizeof(unsigned short)));
    }
    stream->seek(fieldBegin, SEEK_SET);
    stream->write16((uint16_t)(levelEnd - fieldBegin - sizeof(unsigned short)));
    stream->seek(levelEnd, SEEK_SET);
    return levelEnd - levelBegin;
}


ccl::Levelset::Levelset(int levelCount)
    : m_magic(TypeMS)
{
    m_levels.resize(levelCount);
    char nameBuf[32];
    for (int i=0; i<levelCount; ++i) {
        snprintf(nameBuf, 32, "Level %d", i + 1);
        m_levels[i] = new ccl::LevelData();
        m_levels[i]->setName(nameBuf);
        m_levels[i]->setPassword(RandomPassword());
    }
}

ccl::Levelset::Levelset(const ccl::Levelset& init)
    : m_magic(init.m_magic)
{
    m_levels.resize(init.m_levels.size());
    for (size_t i=0; i<m_levels.size(); ++i) {
        m_levels[i] = new ccl::LevelData;
        m_levels[i]->copyFrom(init.m_levels[i]);
    }
}

ccl::Levelset::~Levelset()
{
    for (size_t i=0; i<m_levels.size(); ++i)
        m_levels[i]->unref();
}

std::string ccl::Levelset::RandomPassword()
{
    char buf[5];
    for (size_t i=0; i<4; i++)
        buf[i] = (char)((rand() % 26) + 'A');
    buf[4] = 0;
    return buf;
}

ccl::LevelData* ccl::Levelset::addLevel()
{
    char nameBuf[32];
    ccl::LevelData* level = new ccl::LevelData();
    m_levels.push_back(level);

    snprintf(nameBuf, 32, "Level %d", (int)m_levels.size());
    level->setName(nameBuf);
    level->setPassword(RandomPassword());
    return level;
}

void ccl::Levelset::addLevel(ccl::LevelData* level)
{
    m_levels.push_back(level);
}

void ccl::Levelset::insertLevel(int where, ccl::LevelData* level)
{
    m_levels.insert(m_levels.begin() + where, level);
}

ccl::LevelData* ccl::Levelset::takeLevel(int num)
{
    ccl::LevelData* level = m_levels[num];
    m_levels.erase(m_levels.begin() + num);
    return level;
}

void ccl::Levelset::read(ccl::Stream* stream)
{
    for (ccl::LevelData* level : m_levels)
        level->unref();
    m_levels.resize(0);

    m_magic = stream->read32();
    if (m_magic != TypeMS && m_magic != TypeLynx && m_magic != TypePG
        && m_magic != TypeLynxPG)
        throw ccl::IOError(ccl::RuntimeError::tr("Invalid levelset header"));

    uint16_t numLevels = stream->read16();
    m_levels.reserve(numLevels);
    for (uint16_t i = 0; i < numLevels; ++i) {
        m_levels.push_back(new ccl::LevelData);
        m_levels.back()->read(stream);
    }
}

void ccl::Levelset::write(ccl::Stream* stream) const
{
    stream->write32(m_magic);
    stream->write16((uint16_t)m_levels.size());

    int levelNum = 0;
    for (ccl::LevelData* level : m_levels) {
        // Re-set level number in case levels were re-ordered
        level->setLevelNum(++levelNum);
        level->write(stream);
    }
}


ccl::LevelsetType ccl::DetermineLevelsetType(const char* filename)
{
    ccl::FileStream stream;
    if (!stream.open(filename, "rb"))
        return LevelsetError;

    uint32_t magic;
    size_t count = stream.read(&magic, sizeof(uint32_t), 1);
    stream.close();
    if (count == 0)
        return LevelsetError;
    if (magic == Levelset::TypeLynx || magic == Levelset::TypeMS
        || magic == Levelset::TypePG || magic == Levelset::TypeLynxPG)
        return LevelsetCcl;
    return LevelsetDac;
}


void ccl::ClipboardData::read(Stream* stream)
{
    m_width = stream->read32();
    m_height = stream->read32();
    (void)stream->read16();
    (void)stream->read32();
    m_levelData->read(stream, true);
}

void ccl::ClipboardData::write(Stream* stream) const
{
    // We only support writing to the beginning of seekable streams for now
    if (stream->tell() != 0L) {
        throw ccl::RuntimeError(ccl::RuntimeError::tr(
                "Cannot write clipboard data to the middle of a stream"));
    }

    stream->write32(m_width);
    stream->write32(m_height);
    stream->write16(0);    // Revisit after writing data
    stream->write32(0);
    m_levelData->write(stream, true);
    stream->seek(8, SEEK_SET);
    stream->write16(stream->size() - 14); // Size of data buffer
}
