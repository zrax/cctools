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

static const uint8_t fullsec_orig_1[] = {
    //cseg02:25AB
    0xAD,                           // jmp      cseg02:265A
};
static const uint8_t fullsec_patch_1[] = {
    //cseg02:25AB
    0x81,                           // jmp      cseg02:262E
};
static_assert(sizeof(fullsec_orig_1) == sizeof(fullsec_patch_1),
              "Fullsec patch size mismatch");

static const uint8_t fullsec_orig_2[] = {
    //cseg02:2661
    0x0B, 0xC0,                     // or       ax, ax
    0x7C, 0x03,                     // jl       cseg02:2668
    0xE9, 0xC1, 0x00,               // jmp      cseg02:2729
};
static const uint8_t fullsec_patch_2[] = {
    //cseg02:2661
    0x80, 0x36, 0xF7, 0x0A, 0x20,   // xor      byte ptr [dseg10:0AF7], 0x20
    0xEB, 0xE7,                     // jmp      cseg02:264F
};
static_assert(sizeof(fullsec_orig_2) == sizeof(fullsec_patch_2),
              "Fullsec patch size mismatch");

static const uint8_t fullsec_orig_3[] = {
    //cseg04:0964
    0xFC,                           // lea      ax, [bp - 4]
    0x16,                           // push     ss
    0x50,                           // push     ax
    0x6A, 0x02,                     // push     2
    0x9A, 0x88, 0x09, 0x00, 0x00,   // lcall    KERNEL:_LREAD
    0x3D, 0x02, 0x00,               // cmp      ax, 2
    0x73, 0x05,                     // jnb      cseg04:0978

    //cseg04:0973
    0x33, 0xC0,                     // xor      ax, ax
    0xEB, 0x35,                     // jmp      cseg04:09AC
    0x90,                           // nop

    //cseg04:0978
    0x81, 0x7E, 0xFC, 0xAC, 0xAA,   // cmp      word ptr [bp - 4], 0xAAAC
    0x75, 0xF4,                     // jne      cseg04:0973
    0x56,                           // push     si
    0x8D, 0x46, 0xFC,               // lea      ax, [bp - 4]
    0x16,                           // push     ss
    0x50,                           // push     ax
    0x6A, 0x02,                     // push     2
};
static const uint8_t fullsec_patch_3[] = {
    //cseg04:0964
    0xFA,                           // lea      ax, [bp - 6]
    0x16,                           // push     ss
    0x50,                           // push     ax
    0x6A, 0x04,                     // push     4
    0x9A, 0x88, 0x09, 0x00, 0x00,   // lcall    KERNEL:_LREAD
    0x3D, 0x04, 0x00,               // cmp      ax, 4
    0x73, 0x04,                     // jnb      cseg04:0977
    0x33, 0xC0,                     // xor      ax, ax
    0xEB, 0x35,                     // jmp      cseg04:09AC

    //cseg04:0977
    0x31, 0xC0,                     // xor      ax, ax
    0xF6, 0x06, 0xF7, 0x0A, 0x20,   // test     byte ptr [dseg10:0AF7], 0x20
    0x75, 0x02,                     // jne      cseg04:0982
    0xB0, 0x0A,                     // mov      al, 0xA

    //cseg04:0982
    0xA3, 0x4E, 0x06,               // mov      word ptr [dseg10:064E], ax
    0xEB, 0x10,                     // jmp      cseg04:0997
};
static_assert(sizeof(fullsec_orig_3) == sizeof(fullsec_patch_3),
              "Fullsec patch size mismatch");

static const uint8_t fullsec_orig_4[] = {
    //cseg03:075D
    0x8B, 0x1E, 0x80, 0x16,         // mov      bx, word ptr [dseg10:1680]
};
static const uint8_t fullsec_patch_4[] = {
    //cseg03:075D
    0xE9, 0xA2, 0x00,               // jmp      cseg03:0802
    0x90,                           // nop
};
static_assert(sizeof(fullsec_orig_4) == sizeof(fullsec_patch_4),
              "Fullsec patch size mismatch");

static const uint8_t fullsec_orig_5[] = {
    //cseg03:07E6
    0xD1, 0xE0,                     // shl      ax, 1
    0x93,                           // xchg     ax, bx
    0x2E, 0xFF, 0xA7, 0xEE, 0x07,   // jmp      word ptr cs:[bx + cseg03:07EE]

    //cseg03:07EE
    0x36, 0x08,                     // dw       offset cseg03:0836
    0x36, 0x08,                     // dw       offset cseg03:0836
    0x36, 0x08,                     // dw       offset cseg03:0836
    0x36, 0x08,                     // dw       offset cseg03:0836
    0xCE, 0x08,                     // dw       offset cseg03:08CE
    0xCE, 0x08,                     // dw       offset cseg03:08CE
    0xCE, 0x08,                     // dw       offset cseg03:08CE
    0xCE, 0x08,                     // dw       offset cseg03:08CE
    0x56, 0x09,                     // dw       offset cseg03:0956
    0x56, 0x09,                     // dw       offset cseg03:0956
    0x56, 0x09,                     // dw       offset cseg03:0956
    0x56, 0x09,                     // dw       offset cseg03:0956
    0xC6, 0x09,                     // dw       offset cseg03:09C6
    0xC6, 0x09,                     // dw       offset cseg03:09C6
    0xC6, 0x09,                     // dw       offset cseg03:09C6
    0xC6, 0x09,                     // dw       offset cseg03:09C6
    0xA0, 0x0A,                     // dw       offset cseg03:0AA0
    0xA0, 0x0A,                     // dw       offset cseg03:0AA0
    0xA0, 0x0A,                     // dw       offset cseg03:0AA0
    0xA0, 0x0A,                     // dw       offset cseg03:0AA0
    0x7C, 0x0B,                     // dw       offset cseg03:0B7C
    0x7C, 0x0B,                     // dw       offset cseg03:0B7C
    0x7C, 0x0B,                     // dw       offset cseg03:0B7C
    0x7C, 0x0B,                     // dw       offset cseg03:0B7C
    0x9E, 0x0D,                     // dw       offset cseg03:0D9E
    0x9E, 0x0D,                     // dw       offset cseg03:0D9E
    0x9E, 0x0D,                     // dw       offset cseg03:0D9E
};
static const uint8_t fullsec_patch_5[] = {
    //cseg03:07E6
    0x24, 0xFC,                     // and      al, 0xFC
    0xD1, 0xE8,                     // shr      ax, 1
    0x93,                           // xchg     ax, bx
    0x2E, 0xFF, 0xA7, 0xF0, 0x07,   // jmp      word ptr cs:[bx + cseg03:07F0]

    //cseg03:07F0
    0x36, 0x08,                     // dw       offset cseg03:0836
    0xCE, 0x08,                     // dw       offset cseg03:08CE
    0x56, 0x09,                     // dw       offset cseg03:0956
    0xC6, 0x09,                     // dw       offset cseg03:09C6
    0xA0, 0x0A,                     // dw       offset cseg03:0AA0
    0x7C, 0x0B,                     // dw       offset cseg03:0B7C
    0x9E, 0x0D,                     // dw       offset cseg03:0D9E
    0xD2, 0x0E,                     // dw       offset cseg03:0ED2
    0x14, 0x10,                     // dw       offset cseg03:1014

    //cseg03:0802
    0x8B, 0x46, 0x22,               // mov      ax, word ptr [bp + 0x22]
    0xA3, 0x4E, 0x06,               // mov      word prt [dseg10:064E], ax
    0x8B, 0x1E, 0x80, 0x16,         // mov      bx, word ptr [dseg10:1680]
    0xE9, 0x52, 0xFF,               // jmp      cseg03:0761

    //cseg03:080F
    0x8B, 0x1E, 0x4E, 0x06,         // mov      bx, word ptr [dseg10:064E]
    0x3B, 0x5E, 0x20,               // cmp      bx, word ptr [bp + 0x20]
    0x74, 0x05,                     // je       cseg03:081D
    0xC7, 0x46, 0x20, 0x00, 0x00,   // mov      word ptr [bp + 0x20], 0

    //cseg03:081D
    0x8B, 0x1E, 0x80, 0x16,         // mov      bx, word ptr [dseg10:1680]
    0xE9, 0xD2, 0x0B,               // jmp      cseg03:13F6
};
static_assert(sizeof(fullsec_orig_5) == sizeof(fullsec_patch_5),
              "Fullsec patch size mismatch");

static const uint8_t fullsec_orig_6[] = {
    //cseg03:13F2
    0x8B, 0x1E, 0x80, 0x16,         // mov      bx, word ptr [dseg10:1680]
};
static const uint8_t fullsec_patch_6[] = {
    //cseg03:13F2
    0xE9, 0x1A, 0xF4,               // jmp      cseg03:080F
    0x90,                           // nop
};
static_assert(sizeof(fullsec_orig_6) == sizeof(fullsec_patch_6),
              "Fullsec patch size mismatch");


struct FSPatchChunk {
    uint32_t        address;
    size_t          size;
    const uint8_t*  orig;
    const uint8_t*  patch;
};

#define FS_PATCH_N(a, n)  \
    { a, sizeof(fullsec_orig_##n), fullsec_orig_##n, fullsec_patch_##n }

static const FSPatchChunk fullsec_patch[] = {
    FS_PATCH_N(0x3BAB, 1),
    FS_PATCH_N(0x3C61, 2),
    FS_PATCH_N(0x9764, 3),
    FS_PATCH_N(0x695D, 4),
    FS_PATCH_N(0x69E6, 5),
    FS_PATCH_N(0x75F2, 6),
};

#undef FS_PATCH_N
