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

#include "CC2MD5.h"
#include "libcc1/Stream.h"

#include <cstring>

CC2MD5::CC2MD5(InitialState state)
    : m_work()
{
    switch (state) {
    case StandardMD5:
        m_context[0] = 0x67452301;
        m_context[1] = 0xefcdab89;
        m_context[2] = 0x98badcfe;
        m_context[3] = 0x10325476;
        m_bytes = 0;
        break;
    case CC2Lock:
        m_context[0] = 0xf8611bf2;
        m_context[1] = 0xee782ca4;
        m_context[2] = 0xf30869b2;
        m_context[3] = 0xb048f18f;
        m_bytes = 0x200;
        break;
    default:
        // Missing a case
        Q_ASSERT(false);
        break;
    }
}

/* The rest of the code in this file is based loosely on Colin Plumb's
   implementation, which is in the public domain. */

static void byteSwap(uint32_t* buffer, size_t words)
{
    for (size_t i = 0; i < words; ++i)
        buffer[i] = SWAP32(buffer[i]);
}

void CC2MD5::update(const void* data, size_t size)
{
    uint32_t tBytes = m_bytes & 0x3f;
    m_bytes += size;

    tBytes = 64 - tBytes;
    if (tBytes > size) {
        memcpy(reinterpret_cast<uint8_t*>(m_work) + 64 - tBytes, data, size);
        return;
    }

    // Deal with the first chunk
    memcpy(reinterpret_cast<uint8_t*>(m_work) + 64 - tBytes, data, tBytes);
    byteSwap(m_work, 16);
    md5Transform();
    data = reinterpret_cast<const uint8_t*>(data) + tBytes;
    size -= tBytes;

    // Process the rest of the data in 64-byte chunks
    while (size >= 64) {
        memcpy(m_work, data, 64);
        byteSwap(m_work, 16);
        md5Transform();
        data = reinterpret_cast<const uint8_t*>(data) + 64;
        size -= 64;
    }

    // Save off any remaining bytes for the next update
    memcpy(m_work, data, size);
}

QByteArray CC2MD5::finish()
{
    int count = m_bytes & 0x3f;
    auto p = reinterpret_cast<uint8_t*>(m_work) + count;

    // Set the first padding bit -- there is always room.
    *p++ = 0x80;

    count = 56 - 1 - count;
    if (count < 0) {
        // We need an extra block
        memset(p, 0, count + 8);
        byteSwap(m_work, 16);
        md5Transform();
        p = reinterpret_cast<uint8_t*>(m_work);
        count = 56;
    }
    memset(p, 0, count);
    byteSwap(m_work, 14);

    // Append length in bits, and finalize the last block
    m_work[14] = static_cast<uint32_t>(m_bytes << 3);
    m_work[15] = static_cast<uint32_t>(m_bytes >> 29);
    md5Transform();

    byteSwap(m_context, 4);
    QByteArray digest;
    digest.resize(16);
    memcpy(digest.data(), m_context, 16);
    return digest;
}

#define F1(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) ((x) ^ (y) ^ (z))
#define F4(x, y, z) ((y) ^ ((x) | ~(z)))

#define MD5STEP(fn, w, x, y, z, in, s) \
    (w += fn((x), (y), (z)) + (in), w = ((w) << (s) | (w) >> (32 - (s))) + (x))

void CC2MD5::md5Transform()
{
    uint32_t a = m_context[0];
    uint32_t b = m_context[1];
    uint32_t c = m_context[2];
    uint32_t d = m_context[3];

    MD5STEP(F1, a, b, c, d, m_work[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, m_work[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, m_work[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, m_work[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, m_work[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, m_work[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, m_work[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, m_work[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, m_work[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, m_work[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, m_work[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, m_work[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, m_work[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, m_work[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, m_work[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, m_work[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, m_work[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, m_work[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, m_work[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, m_work[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, m_work[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, m_work[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, m_work[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, m_work[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, m_work[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, m_work[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, m_work[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, m_work[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, m_work[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, m_work[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, m_work[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, m_work[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, m_work[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, m_work[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, m_work[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, m_work[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, m_work[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, m_work[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, m_work[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, m_work[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, m_work[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, m_work[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, m_work[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, m_work[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, m_work[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, m_work[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, m_work[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, m_work[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, m_work[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, m_work[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, m_work[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, m_work[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, m_work[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, m_work[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, m_work[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, m_work[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, m_work[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, m_work[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, m_work[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, m_work[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, m_work[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, m_work[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, m_work[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, m_work[9] + 0xeb86d391, 21);

    m_context[0] += a;
    m_context[1] += b;
    m_context[2] += c;
    m_context[3] += d;
}
