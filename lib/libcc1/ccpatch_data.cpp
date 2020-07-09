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
static_assert(sizeof(ccpatch_orig) == CCPATCH_SIZE, "CCPatch content size mismatch");

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
static_assert(sizeof(ccpatch_patch) == CCPATCH_SIZE, "CCPatch content size mismatch");
