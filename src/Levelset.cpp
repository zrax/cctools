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

#include <cstring>
#include <cstdlib>

#ifdef WIN32
#define snprintf _sprintf_p
#endif

#ifdef BYTES_BIG_ENDIAN
#define SWAP16(x)   (((x) << 8) & 0xFF00) | (((x) >> 8) & 0x00FF)
#define SWAP32(x)   (((x) << 24) & 0xFF000000) | (((x) << 8) & 0x00FF0000) | \
                    (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00)
#else
#define SWAP16(x)   (x)
#define SWAP32(x)   (x)
#endif

static unsigned char read8(FILE* stream)
{
    unsigned char val;
    if (fread(&val, sizeof(val), 1, stream) == 0)
        throw ccl::IOException("Read past end of stream");
    return val;
}

static unsigned short read16(FILE* stream)
{
    unsigned short val;
    if (fread(&val, sizeof(val), 1, stream) == 0)
        throw ccl::IOException("Read past end of stream");
    return SWAP16(val);
}

static unsigned int read32(FILE* stream)
{
    unsigned int val;
    if (fread(&val, sizeof(val), 1, stream) == 0)
        throw ccl::IOException("Read past end of stream");
    return SWAP32(val);
}

static void read_rle(tile_t* dest, size_t size, FILE* stream)
{
    int dataLen = (int)read16(stream);
    tile_t* cur = dest;
    while (dataLen > 0 && cur < (dest + size)) {
        tile_t tile = (tile_t)read8(stream);
        if (tile == 0xFF) {
            unsigned char count = read8(stream);
            tile = (tile_t)read8(stream);
            if ((cur + count) > (dest + size))
                throw ccl::IOException("RLE buffer overflow");
            if ((dataLen - 3) < 0)
                throw ccl::IOException("RLE buffer underflow");
            memset(cur, tile, count);
            cur += count;
            dataLen -= 3;
        } else {
            *cur++ = tile;
            --dataLen;
        }
    }

    if (dataLen != 0)
        throw ccl::IOException("RLE buffer overflow");
    if (cur != (dest + size))
        throw ccl::IOException("RLE buffer underflow");
}

static std::string read_string(FILE* stream, size_t length, bool password)
{
    char* buffer = new char[length];
    if (fread(buffer, 1, length, stream) != length) {
        delete[] buffer;
        throw ccl::IOException("Read past end of stream");
    }
    buffer[length - 1] = 0;
    if (password) {
        for (size_t i=0; i<(length-1); ++i)
            buffer[i] ^= 0x99;
    }
    std::string str = buffer;
    delete[] buffer;
    return str;
}

static void write8(FILE* stream, unsigned char value)
{
    if (fwrite(&value, sizeof(value), 1, stream) == 0)
        throw ccl::IOException("Error writing to stream");
}

static void write16(FILE* stream, unsigned short value)
{
    unsigned short wval = SWAP16(value);
    if (fwrite(&wval, sizeof(wval), 1, stream) == 0)
        throw ccl::IOException("Error writing to stream");
}

static void write32(FILE* stream, unsigned int value)
{
    unsigned int wval = SWAP32(value);
    if (fwrite(&wval, sizeof(wval), 1, stream) == 0)
        throw ccl::IOException("Error writing to stream");
}

static long write_rle(const tile_t* src, size_t size, FILE* stream)
{
    const tile_t* cur = src;
    tile_t* dest = new tile_t[size];  // Output will never be larger than input
    int dataLen = 0;
    while (cur < (src + size)) {
        int count = 1;
        tile_t tile = *cur++;
        while ((cur < (src + size)) && (count < 255) && (*cur == tile)) {
            ++cur;
            ++count;
        }
        if (count > 3) {
            dest[dataLen++] = (tile_t)0xFF;
            dest[dataLen++] = (tile_t)count;
            dest[dataLen++] = tile;
        } else {
            for (int i=0; i<count; ++i)
                dest[dataLen++] = tile;
        }
    }

    try {
        long begin = ftell(stream);
        write16(stream, dataLen);
        if (fwrite(dest, 1, dataLen, stream) != (size_t)dataLen)
            throw ccl::IOException("Error writing to stream");
        delete[] dest;
        return ftell(stream) - begin;
    } catch (...) {
        // Ensure dest is properly deleted
        delete[] dest;
        throw;
    }
}

static void write_string(FILE* stream, const std::string& str, bool password)
{
    size_t length = str.size();
    if (password) {
        char* buffer = new char[length];
        for (size_t i=0; i<length; ++i)
            buffer[i] = str[i] ^ 0x99;
        if (fwrite(buffer, 1, length, stream) != length) {
            delete[] buffer;
            throw ccl::IOException("Error writing to stream");
        }
        delete[] buffer;
    } else {
        if (fwrite(str.c_str(), 1, length, stream) != length)
            throw ccl::IOException("Error writing to stream");
    }

    // Null terminator
    write8(stream, 0);
}


ccl::LevelMap::LevelMap(int height, int width)
    : m_height(height), m_width(width)
{
    m_fgTiles = new tile_t[width*height];
    m_bgTiles = new tile_t[width*height];
    memset(m_fgTiles, 0, width * height * sizeof(tile_t));
    memset(m_bgTiles, 0, width * height * sizeof(tile_t));
}

ccl::LevelMap& ccl::LevelMap::operator=(const ccl::LevelMap& source)
{
    delete[] m_fgTiles;
    delete[] m_bgTiles;

    m_height = source.m_height;
    m_width = source.m_width;
    m_fgTiles = new tile_t[m_width*m_height];
    m_bgTiles = new tile_t[m_width*m_height];
    memcpy(m_fgTiles, source.m_fgTiles, m_width * m_height * sizeof(tile_t));
    memcpy(m_bgTiles, source.m_bgTiles, m_width * m_height * sizeof(tile_t));
    return *this;
}

void ccl::LevelMap::copyFrom(const ccl::LevelMap& source, int destX, int destY)
{
    int endX = std::max(destX + source.m_width, m_width);
    int endY = std::max(destY + source.m_height, m_height);
    for (int x = destX; x < endX; ++x) {
        for (int y = destY; y < endY; ++y) {
            m_fgTiles[(y*m_width) + x] = source.m_fgTiles[(y*source.m_width) + x];
            m_bgTiles[(y*m_width) + x] = source.m_bgTiles[(y*source.m_width) + x];
        }
    }
}

void ccl::LevelMap::push(int x, int y, tile_t tile)
{
    m_bgTiles[(y*m_width) + x] = m_fgTiles[(y*m_width) + x];
    m_fgTiles[(y*m_width) + x] = tile;
}

tile_t ccl::LevelMap::pop(int x, int y)
{
    tile_t tile = m_fgTiles[(y*m_width) + x];
    m_fgTiles[(y*m_width) + x] = m_bgTiles[(y*m_width) + x];
    m_bgTiles[(y*m_width) + x] = 0;
    return tile;
}

long ccl::LevelMap::read(FILE* stream)
{
    long begin = ftell(stream);
    read_rle(m_fgTiles, m_width * m_height, stream);
    read_rle(m_bgTiles, m_width * m_height, stream);
    return ftell(stream) - begin;
}

long ccl::LevelMap::write(FILE* stream)
{
    long outsize = 0;
    outsize += write_rle(m_fgTiles, m_width * m_height, stream);
    outsize += write_rle(m_bgTiles, m_width * m_height, stream);
    return outsize;
}


ccl::LevelData::LevelData()
    : m_levelNum(0), m_chips(0), m_timer(0)
{ }

void ccl::LevelData::trapConnect(int buttonX, int buttonY, int trapX, int trapY)
{
    ccl::Trap item;
    item.button.X = buttonX;
    item.button.Y = buttonY;
    item.trap.X = trapX;
    item.trap.Y = trapY;
    m_traps.push_back(item);
}

void ccl::LevelData::cloneConnect(int buttonX, int buttonY, int cloneX, int cloneY)
{
    ccl::Clone item;
    item.button.X = buttonX;
    item.button.Y = buttonY;
    item.clone.X = cloneX;
    item.clone.Y = cloneY;
    m_clones.push_back(item);
}

void ccl::LevelData::addMover(int moverX, int moverY)
{
    ccl::Point item;
    item.X = moverX;
    item.Y = moverY;
    m_moveList.push_back(item);
}

long ccl::LevelData::read(FILE* stream)
{
    long levelBegin = ftell(stream);
    long dataSize = (long)read16(stream);

    m_levelNum = read16(stream);
    m_timer = read16(stream);
    m_chips = read16(stream);
    dataSize -= 3 * sizeof(unsigned short);

    if (read16(stream) != 1)
        throw ccl::IOException("Invalid map data field");
    dataSize -= m_map.read(stream) + sizeof(unsigned short);

    dataSize -= sizeof(unsigned short);
    if ((long)read16(stream) != dataSize)
        throw ccl::IOException("Corrupt map data");

    while (dataSize > 0) {
        unsigned char field = read8(stream);
        unsigned char size = read8(stream);
        dataSize -= size + 2 * sizeof(unsigned char);
        if (dataSize < 0)
            throw ccl::IOException("Invalid or corrupt level data");

        switch (field) {
        case FieldName:
            m_name = read_string(stream, size, false);
            break;
        case FieldHint:
            m_hint = read_string(stream, size, false);
            break;
        case FieldPassword:
            m_password = read_string(stream, size, true);
            break;
        case FieldTraps:
            if ((size % 10) != 0)
                throw ccl::IOException("Invalid trap field size");
            for (size_t i=0; i<size; i += 10) {
                ccl::Trap trap;
                trap.button.X = read16(stream);
                trap.button.Y = read16(stream);
                trap.trap.X = read16(stream);
                trap.trap.Y = read16(stream);
                read16(stream); // Unknown or useless value
                m_traps.push_back(trap);
            }
            break;
        case FieldClones:
            if ((size % 8) != 0)
                throw ccl::IOException("Invalid clone field size");
            for (size_t i=0; i<size; i += 8) {
                ccl::Clone clone;
                clone.button.X = read16(stream);
                clone.button.Y = read16(stream);
                clone.clone.X = read16(stream);
                clone.clone.Y = read16(stream);
                m_clones.push_back(clone);
            }
            break;
        case FieldMoveList:
            if ((size % 2) != 0)
                throw ccl::IOException("Invalid move list field size");
            for (size_t i=0; i<size; i += 2) {
                ccl::Point mover;
                mover.X = read8(stream);
                mover.Y = read8(stream);
                m_moveList.push_back(mover);
            }
            break;
        default:
            throw ccl::IOException("Invalid / unrecognized field type");
        }
    }

    if (dataSize != 0)
        throw ccl::IOException("Invalid level checksum");
    return ftell(stream) - levelBegin;
}

long ccl::LevelData::write(FILE* stream)
{
    long levelBegin = ftell(stream);
    write16(stream, 0); // This will be updated at the end

    write16(stream, m_levelNum);
    write16(stream, m_timer);
    write16(stream, m_chips);

    // Map data
    write16(stream, 1);
    m_map.write(stream);

    long fieldBegin = ftell(stream);
    write16(stream, 0); // This will be updated at the end

    if (!m_name.empty()) {
        write8(stream, (unsigned char)FieldName);
        write8(stream, (unsigned char)(m_name.size() + 1));
        write_string(stream, m_name, false);
    }
    if (!m_hint.empty()) {
        write8(stream, (unsigned char)FieldHint);
        write8(stream, (unsigned char)(m_hint.size() + 1));
        write_string(stream, m_hint, false);
    }
    if (!m_password.empty()) {
        write8(stream, (unsigned char)FieldPassword);
        write8(stream, (unsigned char)(m_password.size() + 1));
        write_string(stream, m_password, true);
    }
    if (m_traps.size() > 0) {
        write8(stream, (unsigned char)FieldTraps);
        write8(stream, m_traps.size() * 10);
        std::list<ccl::Trap>::iterator it;
        for (it = m_traps.begin(); it != m_traps.end(); ++it) {
            write16(stream, it->button.X);
            write16(stream, it->button.Y);
            write16(stream, it->trap.X);
            write16(stream, it->trap.Y);
            write16(stream, 0);
        }
    }
    if (m_clones.size() > 0) {
        write8(stream, (unsigned char)FieldClones);
        write8(stream, m_clones.size() * 8);
        std::list<ccl::Clone>::iterator it;
        for (it = m_clones.begin(); it != m_clones.end(); ++it) {
            write16(stream, it->button.X);
            write16(stream, it->button.Y);
            write16(stream, it->clone.X);
            write16(stream, it->clone.Y);
        }
    }
    if (m_moveList.size() > 0) {
        write8(stream, (unsigned char)FieldMoveList);
        write8(stream, m_moveList.size() * 2);
        std::list<ccl::Point>::iterator it;
        for (it = m_moveList.begin(); it != m_moveList.end(); ++it) {
            write8(stream, it->X);
            write8(stream, it->Y);
        }
    }

    // Update checksums
    long levelEnd = ftell(stream);
    fseek(stream, levelBegin, SEEK_SET);
    write16(stream, levelEnd - levelBegin - sizeof(unsigned short));
    fseek(stream, fieldBegin, SEEK_SET);
    write16(stream, levelEnd - fieldBegin - sizeof(unsigned short));
    fseek(stream, levelEnd, SEEK_SET);
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
        //m_levels[i]->setLevelNum(i + 1);
        m_levels[i]->setName(nameBuf);
        m_levels[i]->setPassword(RandomPassword());
    }
}

ccl::Levelset::~Levelset()
{
    for (size_t i=0; i<m_levels.size(); ++i)
        delete m_levels[i];
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

    //level->setLevelNum((int)m_levels.size());
    snprintf(nameBuf, 32, "Level %d", (int)m_levels.size());
    level->setName(nameBuf);
    level->setPassword(RandomPassword());
    return level;
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

void ccl::Levelset::read(FILE* stream)
{
    for (size_t i=0; i<m_levels.size(); ++i)
        delete m_levels[i];
    m_levels.resize(0);

    m_magic = read32(stream);
    if (m_magic != TypeMS && m_magic != TypeLynx)
        throw ccl::IOException("Invalid levelset header");

    size_t numLevels = (size_t)read16(stream);
    m_levels.resize(numLevels);
    for (size_t i=0; i<numLevels; ++i) {
        m_levels[i] = new ccl::LevelData();
        m_levels[i]->read(stream);
    }
}

void ccl::Levelset::write(FILE* stream)
{
    write32(stream, m_magic);
    write16(stream, m_levels.size());

    std::vector<ccl::LevelData*>::iterator it;
    int levelNum = 0;
    for (it = m_levels.begin(); it != m_levels.end(); ++it) {
        // Re-set level number in case levels were re-ordered
        (*it)->setLevelNum(++levelNum);
        (*it)->write(stream);
    }
}
