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

void ccl::Stream::read_rle(tile_t* dest, size_t size)
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

std::string ccl::Stream::read_string(size_t length, bool password)
{
    char* buffer = new char[length];
    if (read(buffer, 1, length) != length) {
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

long ccl::Stream::write_rle(const tile_t* src, size_t size)
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
        long begin = tell();
        write16(dataLen);
        if (write(dest, 1, dataLen) != (size_t)dataLen)
            throw ccl::IOException("Error writing to stream");
        delete[] dest;
        return tell() - begin;
    } catch (...) {
        // Ensure dest is properly deleted
        delete[] dest;
        throw;
    }
}

void ccl::Stream::write_string(const std::string& value, bool password)
{
    size_t length = value.size();
    if (password) {
        char* buffer = new char[length];
        for (size_t i=0; i<length; ++i)
            buffer[i] = value[i] ^ 0x99;
        if (write(buffer, 1, length) != length) {
            delete[] buffer;
            throw ccl::IOException("Error writing to stream");
        }
        delete[] buffer;
    } else {
        if (write(value.c_str(), 1, length) != length)
            throw ccl::IOException("Error writing to stream");
    }

    // Null terminator
    write8(0);
}

size_t ccl::Stream::copyBytes(ccl::Stream* out, size_t size)
{
    std::unique_ptr<uint8_t[]> bytes(new uint8_t[size]);
    size_t nread = read(bytes.get(), 1, size);
    return out->write(bytes.get(), 1, nread);
}

ccl::Stream* ccl::Stream::unpack(long packedLength)
{
    std::unique_ptr<ccl::BufferStream> ustream(new ccl::BufferStream);
    uint16_t unpackedSize = read16();
    packedLength -= sizeof(unpackedSize);

    while (packedLength != 0) {
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

    if (unpackedSize != ustream->size())
        throw ccl::IOException("Packed data did not match expected length");

    ustream->seek(0, SEEK_SET);
    return ustream.release();
}

long ccl::Stream::pack(Stream* unpacked)
{
    throw std::runtime_error("Not yet implemented");
}


bool ccl::FileStream::open(const char* filename, const char* mode)
{
    m_file = fopen(filename, mode);
    return isOpen();
}

void ccl::FileStream::close()
{
    if (m_file != 0)
        fclose(m_file);
    m_file = 0;
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
        m_buffer = 0;
        m_size = 0;
        m_alloc = 0;
    }
    m_offs = 0;
}

size_t ccl::BufferStream::read(void* buffer, size_t size, size_t count)
{
    if (m_buffer == 0)
        return 0;

    size_t bytesCopied = 0;
    unsigned char* bufPtr = (unsigned char*)buffer;
    while (m_offs + size <= m_size && count > 0) {
        memcpy(bufPtr, m_buffer + m_offs, size);
        bufPtr += size;
        m_offs += size;
        bytesCopied += size;
        --count;
    }
    return bytesCopied;
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

    size_t bytesCopied = 0;
    unsigned char* bufPtr = (unsigned char*)buffer;
    while (count > 0) {
        memcpy(m_buffer + m_offs, bufPtr, size);
        bufPtr += size;
        m_offs += size;
        bytesCopied += size;
        --count;
    }
    if (m_offs > m_size)
        m_size = m_offs;
    return bytesCopied;
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
