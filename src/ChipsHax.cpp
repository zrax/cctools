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


#define CCPATCH_SIZE    82
#define CCPATCH_ADDR    0x7C93

static uint8_t ccpatch_orig[] = {
    //cseg03:1A93
    0x8A, 0xD8,                 // mov      bl, al
    0x2A, 0xFF,                 // sub      bh, bh
    0x8B, 0xC3,                 // mov      ax, bx
    0xD1, 0xE3,                 // shl      bx, 1
    0x03, 0xD8,                 // add      bx, ax
    0xD1, 0xE3,                 // shl      bx, 1
    0x8D, 0x7E, 0xF6,           // lea      di, [bp-local_A]
    0x8D, 0xB7, 0x6C, 0x06,     // lea      si, [bx+066Ch]
    0x8C, 0xD0,                 // mov      ax, ss
    0x8E, 0xC0,                 // mov      es, ax
    0xA5,                       // movsw
    0xA5,                       // movsw
    0xA5,                       // movsw
    0x8A, 0x46, 0xF7,           // mov      al, byte ptr [bp-local_9]
    0x2A, 0xE4,                 // sub      ah, ah
    0x8B, 0x5E, 0x0E,           // mov      bx, [bp+arg_8]
    0x89, 0x07,                 // mov      [bx], ax
    0x80, 0x7E, 0xF6, 0x01,     // cmp      byte ptr [bp-local_A], 1
    0x75, 0x07,                 // jnz      short cseg03:1AC4
    0xB8, 0x01, 0x00,           // mov      ax, 1
    0xE9, 0xD8, 0x01,           // jmp      cseg03:1C9B
    0x90,                       // nop

    //cseg03:1AC4
    0x80, 0x7E, 0xF6, 0x02,     // cmp      byte ptr [bp-local_A], 2
    0x74, 0x03,                 // jz       short cseg03:1ACD
    0xE9, 0xC7, 0x01,           // jmp      cseg03:1C94

    //cseg03:1ACD
    0x80, 0x7E, 0xFD, 0x40,     // cmp      [bp-local_3], 40h   ; TILE_BUG_N
    0x72, 0x09,                 // jb       short cseg03:1ADC
    0x80, 0x7E, 0xFD, 0x6F,     // cmp      [bp-local_3], 6Fh   ; TILE_PLAYER_E
    0x77, 0x03,                 // ja       short cseg03:1ADC
    0xE9, 0x98, 0x01,           // jmp      cseg03:1C74

    //cseg03:1ADC
    0x80, 0x7E, 0xFD, 0x0A,     // cmp      [bp-local_3], 0Ah   ; TILE_BLOCK
    0x75, 0x03,                 // jnz      short cseg03:1AE5
    0xE9, 0x8F, 0x01,           // jmp      cseg03:1C74

    //cseg03:1AE5
};

static uint8_t ccpatch_patch[] = {
    //cseg03:1A93
    0xB3, 0x06,                 // mov      bl, 6
    0xF6, 0xE3,                 // mul      bl
    0x89, 0xC3,                 // mov      bx, ax
    0x8D, 0x7E, 0xF6,           // lea      di, [bp-local_A]
    0x8D, 0xB7, 0x6C, 0x06,     // lea      si, [bx+66Ch]
    0x8C, 0xD0,                 // mov      ax, ss
    0x8E, 0xC0,                 // mov      es, ax
    0xA5,                       // movsw
    0xA5,                       // movsw
    0xA5,                       // movsw
    0x8A, 0x46, 0xF7,           // mov      al, [bp-local_9]
    0x28, 0xE4,                 // sub      ah, ah
    0x8B, 0x5E, 0x0E,           // mov      bx, [bp+arg_8]
    0x89, 0x07,                 // mov      [bx], ax
    0x80, 0x7E, 0xF6, 0x01,     // cmp      [bp-local_A], 1
    0x75, 0x08,                 // jnz      short cseg03:1ABF

    //cseg03:1AB7
    0xB8, 0x01, 0x00,           // mov      ax, 1
    0xE9, 0xDE, 0x01,           // jmp      cseg03:1C9B

    //cseg03:1ABD
    0xEB, 0xF8,                 // jmp      short cseg03:1AB7

    //cseg03:1ABF
    0x80, 0x7E, 0xF6, 0x02,     // cmp      [bp-local_A], 2
    0x74, 0x03,                 // jz       short cseg03:1AC8
    0xE9, 0xCC, 0x01,           // jmp      cseg03:1C94

    //cseg03:1AC8
    0x80, 0x7E, 0xFD, 0x40,     // cmp      [bp-local_3], 40h   ; TILE_BUG_N
    0x72, 0x0F,                 // jb       short cseg03:1ADD
    0x80, 0x7E, 0xFD, 0x6F,     // cmp      [bp-local_3], 6Fh   ; TILE_PLAYER_E
    0x77, 0x09,                 // jb       short cseg03:1ADD

    //cseg03:1AD4
    0x83, 0x7E, 0x12, 0x00,     // cmp      [bp+arg_C], 0
    0x74, 0xDD,                 // jz       short cseg03:1AB7
    0xE9, 0x97, 0x01,           // jmp      cseg03:1C74

    //cseg03:1ADD
    0x80, 0x7E, 0xFD, 0x0A,     // cmp      [bp-local_3], 0Ah   ; TILE_BLOCK
    0x75, 0x02,                 // jnz      short cseg03:1AE5
    0xEB, 0xEF,                 // jmp      short cseg03:1AD4

    //cseg03:1AE5
};

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
        throw ccl::Exception("Invalid patch state parameter");
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


#include "pgchips_data.cpp"

void ccl::ChipsHax::set_PGChips(CCPatchState state)
{
    static const uint8_t* data;
    switch (state) {
    case CCPatchOriginal:
        data = pg_original;
        break;
    case CCPatchPatched:
        data = pg_patch;
        break;
    default:
        throw ccl::Exception("Invalid patch state parameter");
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
    static const uint8_t* data = pg_original;
    m_stream->seek(0, SEEK_SET);
    for ( ;; ) {
        uint8_t b = *data++;
        if (b == 0xFF) {
            b = *data++;
            if (b == 0) {
                if (m_stream->read8() != 0xFF)
                    break;
            } else if (b == 0xFF) {
                uint16_t s = (data[0] << 8) | data[1];
                data += 2;
                if (s == 0xFFFF) {
                    uint32_t l = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
                    if (l == 0xFFFFFFFF && m_stream->tell() == m_stream->size())
                        return ccl::CCPatchOriginal;
                    m_stream->seek(l, SEEK_CUR);
                } else {
                    m_stream->seek(s, SEEK_CUR);
                }
            } else {
                m_stream->seek(b, SEEK_CUR);
            }
        } else {
            if (m_stream->read8() != b)
                break;
        }
    }
    fprintf(stderr, "Found invalid bytes at %08X\n", m_stream->tell());

    data = pg_patch;
    m_stream->seek(0, SEEK_SET);
    for ( ;; ) {
        uint8_t b = *data++;
        if (b == 0xFF) {
            b = *data++;
            if (b == 0) {
                if (m_stream->read8() != 0xFF)
                    break;
            } else if (b == 0xFF) {
                uint16_t s = (data[0] << 8) | data[1];
                data += 2;
                if (s == 0xFFFF) {
                    uint32_t l = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
                    if (l == 0xFFFFFFFF && m_stream->tell() == m_stream->size())
                        return ccl::CCPatchPatched;
                    m_stream->seek(l, SEEK_CUR);
                } else {
                    m_stream->seek(s, SEEK_CUR);
                }
            } else {
                m_stream->seek(b, SEEK_CUR);
            }
        } else {
            if (m_stream->read8() != b)
                break;
        }
    }
    fprintf(stderr, "Found invalid bytes at %08X\n", m_stream->tell());

    return ccl::CCPatchOther;
}
