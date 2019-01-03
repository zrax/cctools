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


cc2::Map::Map() : m_version("7"), m_readOnly()
{
    memset(m_key, 0, sizeof(m_key));
}

static std::string readZString(ccl::Stream* stream, long size)
{
    // Strings stored in the .c2m file include the nul-terminator, but we
    // don't want to rely on that...
    char* buffer = new char[size];
    stream->read(buffer, 1, size);
    std::string str(buffer, size - 1);
    delete[] buffer;
    return str;
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
            m_version = readZString(stream, (long)size);
            have_magic = true;
        } else if (memcmp(tag, "LOCK", 4) == 0) {
            m_lock = readZString(stream, (long)size);
        } else if (memcmp(tag, "TITL", 4) == 0) {
            m_title = readZString(stream, (long)size);
        } else if (memcmp(tag, "AUTH", 4) == 0) {
            m_author = readZString(stream, (long)size);
        } else if (memcmp(tag, "VERS", 4) == 0) {
            m_editorVersion = readZString(stream, (long)size);
        } else if (memcmp(tag, "CLUE", 4) == 0) {
            m_clue = readZString(stream, (long)size);
        } else if (memcmp(tag, "NOTE", 4) == 0) {
            m_note = readZString(stream, (long)size);
        } else if (memcmp(tag, "OPTN", 4) == 0) {
            m_option.read(stream, (long)size);
        } else if (memcmp(tag, "MAP ", 4) == 0) {
            m_mapData.read(stream, (long)size, false);
        } else if (memcmp(tag, "PACK", 4) == 0) {
            m_mapData.read(stream, (long)size, true);
        } else if (memcmp(tag, "KEY ", 4) == 0) {
            if (size != sizeof(m_key))
                throw ccl::FormatException("Invalid KEY field size");
            if (stream->read(&m_key, 1, sizeof(m_key)) != sizeof(m_key))
                throw ccl::IOException("Read past end of file");
        } else if (memcmp(tag, "REPL", 4) == 0) {
            m_replay.read(stream, (long)size, false);
        } else if (memcmp(tag, "PRPL", 4) == 0) {
            m_replay.read(stream, (long)size, true);
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
