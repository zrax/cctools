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

#ifndef _CCL_STREAM_H
#define _CCL_STREAM_H

#include <string>
#include <cstdio>
#include "Errors.h"

#ifdef _MSC_VER
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
#else
    #include <stdint.h>
#endif

typedef unsigned char tile_t;

namespace ccl {

class Stream {
public:
    Stream() { }
    virtual ~Stream() { }

    virtual size_t read(void* buffer, size_t size, size_t count) = 0;
    virtual size_t write(const void* buffer, size_t size, size_t count) = 0;
    virtual long tell() = 0;
    virtual long size() = 0;
    virtual void seek(long offset, int whence) = 0;
    virtual bool eof() = 0;

    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    void read_rle(tile_t* dest, size_t size);
    std::string read_string(size_t length, bool password = false);

    void write8(uint8_t value);
    void write16(uint16_t value);
    void write32(uint32_t value);
    long write_rle(const tile_t* src, size_t size);
    void write_string(const std::string& value, bool password = false);
};

class FileStream : public Stream {
public:
    FileStream() : m_file(0) { }
    virtual ~FileStream() { close(); }

    bool open(const char* filename, const char* mode);
    bool isOpen() const { return m_file != 0; }
    void close();

    virtual size_t read(void* buffer, size_t size, size_t count)
    { return fread(buffer, size, count, m_file); }

    virtual size_t write(const void* buffer, size_t size, size_t count)
    { return fwrite(buffer, size, count, m_file); }

    virtual long tell() { return ftell(m_file); }
    virtual long size();
    virtual void seek(long offset, int whence) { fseek(m_file, offset, whence); }
    virtual bool eof();

private:
    FILE* m_file;
};

class BufferStream : public Stream {
public:
    BufferStream() : m_size(0), m_offs(0), m_alloc(0), m_buffer(0) { }
    virtual ~BufferStream() { delete[] m_buffer; }

    void setFrom(const void* buffer, size_t size);
    unsigned char* buffer() const { return m_buffer; }

    virtual size_t read(void* buffer, size_t size, size_t count);
    virtual size_t write(const void* buffer, size_t size, size_t count);
    virtual long tell() { return (long)m_offs; }
    virtual long size() { return (long)m_size; }
    virtual void seek(long offset, int whence);
    virtual bool eof() { return (m_offs >= m_size); }

private:
    size_t m_size, m_offs, m_alloc;
    unsigned char* m_buffer;
};

}

#endif
