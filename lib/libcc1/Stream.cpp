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

#include "Stream.h"

#include <cstring>
#include <vector>
#include <algorithm>
#include <stdexcept>

uint8_t ccl::Stream::read8()
{
    uint8_t val;
    if (read(&val, sizeof(val), 1) == 0)
        throw ccl::IOException("Read past end of stream");
    return val;
}

uint16_t ccl::Stream::read16()
{
    uint16_t val;
    if (read(&val, sizeof(val), 1) == 0)
        throw ccl::IOException("Read past end of stream");
    return SWAP16(val);
}

uint32_t ccl::Stream::read32()
{
    uint32_t val;
    if (read(&val, sizeof(val), 1) == 0)
        throw ccl::IOException("Read past end of stream");
    return SWAP32(val);
}

void ccl::Stream::readRLE(tile_t* dest, size_t size)
{
    int dataLen = (int)read16();
    tile_t* cur = dest;
    while (dataLen > 0 && cur < (dest + size)) {
        tile_t tile = (tile_t)read8();
        if (tile == 0xFF) {
            unsigned char count = read8();
            tile = (tile_t)read8();
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

std::string ccl::Stream::readString(size_t length, bool password)
{
    std::unique_ptr<char[]> buffer(new char[length]);
    if (read(buffer.get(), 1, length) != length)
        throw ccl::IOException("Read past end of stream");

    if (password) {
        for (size_t i=0; i<(length-1); ++i)
            buffer[i] ^= 0x99;
    }

    // Strings stored in levelset files include the nul-terminator, but we
    // don't want to rely on that...
    return std::string(buffer.get(), length - 1);
}

std::string ccl::Stream::readZString()
{
    std::string buffer;
    buffer.reserve(32);

    for ( ;; ) {
        char ch = (char)read8();
        if (!ch)
            break;
        buffer.push_back(ch);
    }
    return buffer;
}

void ccl::Stream::write8(uint8_t value)
{
    if (write(&value, sizeof(value), 1) == 0)
        throw ccl::IOException("Error writing to stream");
}

void ccl::Stream::write16(uint16_t value)
{
    uint16_t wval = SWAP16(value);
    if (write(&wval, sizeof(wval), 1) == 0)
        throw ccl::IOException("Error writing to stream");
}

void ccl::Stream::write32(uint32_t value)
{
    uint32_t wval = SWAP32(value);
    if (write(&wval, sizeof(wval), 1) == 0)
        throw ccl::IOException("Error writing to stream");
}

static uint16_t rleLength(const tile_t* src, size_t size)
{
    // Pre-compute the length of RLE-encoded data, so we don't have to
    // perform extra allocations, seeks, etc. during the writing process
    const tile_t* cur = src;
    int dataLen = 0;
    while (cur < (src + size)) {
        int count = 1;
        tile_t tile = *cur++;
        while ((cur < (src + size)) && (count < 255) && (*cur == tile)) {
            ++cur;
            ++count;
        }
        if (count > 3)
            dataLen += 3;
        else
            dataLen += count;
    }
    return (uint16_t)dataLen;
}

long ccl::Stream::writeRLE(const tile_t* src, size_t size)
{
    long begin = tell();

    write16(rleLength(src, size));
    const tile_t* cur = src;
    while (cur < (src + size)) {
        int count = 1;
        tile_t tile = *cur++;
        while ((cur < (src + size)) && (count < 255) && (*cur == tile)) {
            ++cur;
            ++count;
        }
        if (count > 3) {
            write8(0xFF);
            write8((uint8_t)count);
            write8((uint8_t)tile);
        } else {
            for (int i=0; i<count; ++i)
                write8((uint8_t)tile);
        }
    }

    return tell() - begin;
}

void ccl::Stream::writeString(const std::string& value, bool password)
{
    if (password) {
        for (char ch : value)
            write8((uint8_t)(ch ^ 0x99));
        // Null terminator
        write8(0);
    } else {
        writeZString(value);
    }
}

void ccl::Stream::writeZString(const std::string& value)
{
    if (write(value.c_str(), sizeof(char), value.size()) != value.size())
        throw ccl::IOException("Error writing to stream");

    // Null terminator
    write8(0);
}

size_t ccl::Stream::copyBytes(ccl::Stream* out, size_t size)
{
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[size]);
    size_t nread = read(bytes.get(), 1, size);
    return out->write(bytes.get(), 1, nread);
}

std::unique_ptr<ccl::Stream> ccl::Stream::unpack(long packedLength)
{
    std::unique_ptr<ccl::BufferStream> ustream(new ccl::BufferStream);
    uint16_t unpackedSize = read16();
    packedLength -= sizeof(unpackedSize);

    while (packedLength > 0) {
        uint8_t control = read8();
        packedLength -= 1;
        if (control >= 0x80) {
            // Copy block
            uint8_t offset = read8();
            packedLength -= 1;

            if (offset == 0 || offset > ustream->tell())
                throw ccl::IOException("Pack offset invalid");

            // Need to copy only one byte at a time, to ensure that bytes
            // written to the output can be looped correctly
            control -= 0x80;
            while (control--) {
                uint8_t copy = ustream->buffer()[ustream->tell() - offset];
                ustream->write8(copy);
            }
        } else {
            if (copyBytes(ustream.get(), control) != control)
                throw ccl::IOException("Read past end of stream");
            packedLength -= control;
        }
    }

    if (unpackedSize != (ustream->size() & 0xffff))
        throw ccl::IOException("Packed data did not match expected length");

    ustream->seek(0, SEEK_SET);
    return ustream;
}

static long match_length(const uint8_t* m1, const uint8_t *m2, long max)
{
    long length = 0;
    while (length < max && m1[length] == m2[length])
        ++length;
    return length;
}

long ccl::Stream::pack(Stream* unpacked)
{
    unpacked->seek(0, SEEK_SET);

    std::vector<uint8_t> bytes;
    const size_t unpackedSize = unpacked->size();
    bytes.resize(unpackedSize);
    if (unpacked->read(&bytes[0], 1, unpackedSize) != unpackedSize)
        throw std::runtime_error("Failed reading unpacked data");

    // Write the unpacked size checksum
    write16(static_cast<uint16_t>(unpackedSize & 0xffff));

    // LZSS-like compression
    long pos = 0;
    long packedSize = sizeof(uint16_t);
    std::vector<uint8_t> accum;

    auto flush_accum = [&] {
        while (!accum.empty()) {
            size_t take_bytes = std::min(accum.size(), size_t(0x7f));
            write8(static_cast<uint8_t>(take_bytes));
            write(&accum[0], 1, take_bytes);
            accum.erase(accum.begin(), accum.begin() + take_bytes);
            packedSize += 1 + take_bytes;
        }
    };

    while (static_cast<size_t>(pos) < bytes.size()) {
        long longest_match_len = 0;
        long longest_match_seek = 0;
        long mSeek = std::max(0L, pos - 0xffL);
        while (mSeek < pos) {
            long mLen = std::min(match_length(&bytes[pos], &bytes[mSeek], bytes.size() - pos),
                                 0x7fL);
            if (mLen > longest_match_len) {
                longest_match_len = mLen;
                longest_match_seek = pos - mSeek;
            }
            ++mSeek;
        }
        if (longest_match_len > 3) {
            flush_accum();

            // Encode this as a back-reference
            write8(static_cast<uint8_t>(0x80 + longest_match_len));
            write8(static_cast<uint8_t>(longest_match_seek));
            packedSize += 2;
            pos += longest_match_len;
        } else {
            accum.push_back(bytes[pos]);
            pos += 1;
        }
    }

    // Flush any leftover unencoded bytes
    flush_accum();

    return packedSize;
}


bool ccl::FileStream::open(const char* filename, const char* mode)
{
    m_file = fopen(filename, mode);
    return isOpen();
}

void ccl::FileStream::close()
{
    if (m_file)
        fclose(m_file);
    m_file = nullptr;
}

long ccl::FileStream::size()
{
    long current = tell();
    seek(0, SEEK_END);
    long size = tell();
    seek(current, SEEK_SET);
    return size;
}

bool ccl::FileStream::eof()
{
    int ch = fgetc(m_file);
    if (ch == EOF)
        return true;
    ungetc(ch, m_file);
    return false;
}


void ccl::BufferStream::setFrom(const void* buffer, size_t size)
{
    delete[] m_buffer;
    if (size > 0) {
        m_buffer = new unsigned char[size];
        memcpy(m_buffer, buffer, size);
        m_size = size;
        m_alloc = size;
    } else {
        m_buffer = nullptr;
        m_size = 0;
        m_alloc = 0;
    }
    m_offs = 0;
}

size_t ccl::BufferStream::read(void* buffer, size_t size, size_t count)
{
    if (m_buffer == 0)
        return 0;

    size_t numCopied = 0;
    unsigned char* bufPtr = (unsigned char*)buffer;
    while (m_offs + size <= m_size && count > 0) {
        memcpy(bufPtr, m_buffer + m_offs, size);
        bufPtr += size;
        m_offs += size;
        ++numCopied;
        --count;
    }
    return numCopied;
}

size_t ccl::BufferStream::write(const void* buffer, size_t size, size_t count)
{
    if (m_offs + (size * count) > m_alloc) {
        size_t bigger = (m_alloc == 0) ? 4096 : m_alloc * 2;
        while (m_offs + (size * count) > bigger)
            bigger *= 2;
        unsigned char* largeBuf = new unsigned char[bigger];
        if (m_buffer != 0)
            memcpy(largeBuf, m_buffer, m_size);
        delete[] m_buffer;
        m_buffer = largeBuf;
        m_alloc = bigger;
    }

    size_t numCopied = 0;
    unsigned char* bufPtr = (unsigned char*)buffer;
    while (count > 0) {
        memcpy(m_buffer + m_offs, bufPtr, size);
        bufPtr += size;
        m_offs += size;
        ++numCopied;
        --count;
    }
    if (m_offs > m_size)
        m_size = m_offs;
    return numCopied;
}

void ccl::BufferStream::seek(long offset, int whence)
{
    if (whence == SEEK_SET)
        m_offs = offset;
    else if (whence == SEEK_CUR)
        m_offs += offset;
    else if (whence == SEEK_END)
        m_offs = m_size - offset;
    else
        throw ccl::Exception("Invalid whence parameter");

    if ((long)m_offs < 0)
        m_offs = 0;
    if (m_offs > m_size)
        m_offs = m_size;
}
