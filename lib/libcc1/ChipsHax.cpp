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

#include "ChipsHax.h"

#include <vector>

void ccl::ChipsHax::set_LastLevel(int level)
{
    uint16_t levelValue = level;

    m_stream->seek(0x91C0, SEEK_SET);       // Test for skipping help
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
    m_stream->seek(0xBA14, SEEK_SET);       // Test for endgame dialog
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
    m_stream->seek(0xBB1C, SEEK_SET);       // Test for game state
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
}

void ccl::ChipsHax::set_FakeLastLevel(int level)
{
    uint16_t levelValue = level;

    m_stream->seek(0x91B9, SEEK_SET);       // Test for skipping help
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
    m_stream->seek(0xBB14, SEEK_SET);       // Test for game state
    m_stream->write(&levelValue, sizeof(uint16_t), 1);

    // Checks for skipping to first level beyond fake last
    ++levelValue;
    m_stream->seek(0x9F85, SEEK_SET);
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
    m_stream->seek(0xA6D9, SEEK_SET);
    m_stream->write(&levelValue, sizeof(uint16_t), 1);
}

int ccl::ChipsHax::get_LastLevel()
{
    uint16_t levelValue;
    m_stream->seek(0xBB1C, SEEK_SET);
    m_stream->read(&levelValue, sizeof(uint16_t), 1);
    return (int)levelValue;
}

int ccl::ChipsHax::get_FakeLastLevel()
{
    uint16_t levelValue;
    m_stream->seek(0xBB14, SEEK_SET);
    m_stream->read(&levelValue, sizeof(uint16_t), 1);
    return (int)levelValue;
}


static uint8_t firstTry_enabled[]  = { 0xB8, 0x00, 0x00, 0x90 };  // mov  ax, 0
static uint8_t firstTry_disabled[] = { 0x8B, 0x87, 0x30, 0x0A };  // mov  ax, [bx+0A30h]

void ccl::ChipsHax::set_AlwaysFirstTry(bool on)
{
    m_stream->seek(0xAA57, SEEK_SET);
    m_stream->write(on ? firstTry_enabled : firstTry_disabled, 1, 4);
}

bool ccl::ChipsHax::get_AlwaysFirstTry()
{
    uint8_t buffer[4];
    m_stream->seek(0xAA57, SEEK_SET);
    m_stream->read(buffer, 1, 4);
    return (memcmp(buffer, firstTry_enabled, 4) == 0);
}


#include "ccpatch_data.cpp"

void ccl::ChipsHax::set_CCPatch(CCPatchState state)
{
    m_stream->seek(CCPATCH_ADDR, SEEK_SET);
    switch (state) {
    case CCPatchOriginal:
        m_stream->write(ccpatch_orig, 1, CCPATCH_SIZE);
        break;
    case CCPatchPatched:
        m_stream->write(ccpatch_patch, 1, CCPATCH_SIZE);
        break;
    default:
        throw ccl::RuntimeError(ccl::RuntimeError::tr("Invalid patch state parameter"));
    }
}

ccl::CCPatchState ccl::ChipsHax::get_CCPatch()
{
    uint8_t buffer[CCPATCH_SIZE];
    m_stream->seek(CCPATCH_ADDR, SEEK_SET);
    m_stream->read(buffer, 1, CCPATCH_SIZE);

    if (memcmp(buffer, ccpatch_orig, CCPATCH_SIZE) == 0)
        return CCPatchOriginal;
    if (memcmp(buffer, ccpatch_patch, CCPATCH_SIZE) == 0)
        return CCPatchPatched;
    return CCPatchOther;
}


#include "fullsec_data.cpp"

void ccl::ChipsHax::set_FullSec(CCPatchState state)
{
    for (const FSPatchChunk& chunk : fullsec_patch) {
        m_stream->seek(chunk.address, SEEK_SET);
        switch (state) {
        case CCPatchOriginal:
            m_stream->write(chunk.orig, 1, chunk.size);
            break;
        case CCPatchPatched:
            m_stream->write(chunk.patch, 1, chunk.size);
            break;
        default:
            throw ccl::RuntimeError(ccl::RuntimeError::tr("Invalid patch state parameter"));
        }
    }
}

ccl::CCPatchState ccl::ChipsHax::get_FullSec()
{
    std::vector<uint8_t> buffer;
    int state = -1;
    for (const FSPatchChunk& chunk : fullsec_patch) {
        m_stream->seek(chunk.address, SEEK_SET);
        buffer.resize(chunk.size);
        m_stream->read(&buffer[0], 1, chunk.size);

        int chunkState = CCPatchOther;
        if (memcmp(&buffer[0], chunk.orig, chunk.size) == 0)
            chunkState = CCPatchOriginal;
        else if (memcmp(&buffer[0], chunk.patch, chunk.size) == 0)
            chunkState = CCPatchPatched;
        else
            return CCPatchOther;

        if (state < 0)
            state = chunkState;
        else if (state != chunkState)
            return CCPatchOther;    // Mixed patch states
    }
    return static_cast<CCPatchState>(state);
}


#include "pgchips_data.cpp"

void ccl::ChipsHax::set_PGChips(CCPatchState state)
{
    const uint8_t* data;
    switch (state) {
    case CCPatchOriginal:
        data = pg_original;
        break;
    case CCPatchPatched:
        data = pg_patch;
        break;
    default:
        throw ccl::RuntimeError(ccl::RuntimeError::tr("Invalid patch state parameter"));
    }

    m_stream->seek(0, SEEK_SET);
    for ( ;; ) {
        uint8_t b = *data++;
        if (b == 0xFF) {
            b = *data++;
            if (b == 0) {
                m_stream->write8(0xFF);
            } else if (b == 0xFF) {
                uint16_t s = (data[0] << 8) | data[1];
                data += 2;
                if (s == 0xFFFF) {
                    uint32_t l = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
                    data += 4;
                    if (l == 0xFFFFFFFF)
                        break;
                    m_stream->seek(l, SEEK_CUR);
                } else {
                    m_stream->seek(s, SEEK_CUR);
                }
            } else {
                m_stream->seek(b, SEEK_CUR);
            }
        } else {
            m_stream->write8(b);
        }
    }
}

ccl::CCPatchState ccl::ChipsHax::get_PGChips()
{
    uint8_t buffer[PGCHECK_SIZE];
    m_stream->seek(PGCHECK_ADDR, SEEK_SET);
    m_stream->read(buffer, 1, PGCHECK_SIZE);

    if (memcmp(buffer, pgcheck_orig, PGCHECK_SIZE) == 0)
        return CCPatchOriginal;
    if (memcmp(buffer, pgcheck_patch, PGCHECK_SIZE) == 0)
        return CCPatchPatched;
    return CCPatchOther;
}

static bool check_patch(const uint8_t* data, ccl::Stream* stream)
{
    stream->seek(0, SEEK_SET);
    for ( ;; ) {
        uint8_t b = *data++;
        if (b == 0xFF) {
            b = *data++;
            if (b == 0) {
                if (stream->read8() != 0xFF)
                    return false;
            } else if (b == 0xFF) {
                uint16_t s = (data[0] << 8) | data[1];
                data += 2;
                if (s == 0xFFFF) {
                    uint32_t l = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
                    data += 4;
                    if (l == 0xFFFFFFFF)
                        return true;
                    stream->seek(l, SEEK_CUR);
                } else {
                    stream->seek(s, SEEK_CUR);
                }
            } else {
                stream->seek(b, SEEK_CUR);
            }
        } else if (stream->read8() != b) {
            return false;
        }
    }

    /* Unreachable */
    std::abort();
}

ccl::CCPatchState ccl::ChipsHax::validate_PGChips()
{
    static const struct {
        const uint8_t* data;
        CCPatchState state;
    } try_patch[] = {
        { pg_original, CCPatchOriginal },
        { pg_patch, CCPatchPatched },
    };

    for (const auto& patch : try_patch) {
        if (check_patch(patch.data, m_stream))
            return patch.state;
    }
    return CCPatchOther;
}
