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

#include "Win16Rsrc.h"
#include <cstring>

void Win16::Resource::read(ccl::Stream* stream)
{
    m_offset = stream->read16();
    m_size = stream->read16();
    m_flags = stream->read16();
    m_resourceID = stream->read16();
    m_reserved = stream->read32();
}

void Win16::Resource::update(ccl::Stream* stream)
{
    stream->write16(m_offset);
    stream->write16(m_size);
    stream->seek(8, SEEK_CUR);
}

std::string Win16::Resource::name() const
{
    if (m_resourceID & 0x8000) {
        char numbuf[16];
        snprintf(numbuf, 16, "%u", (uint32_t)m_resourceID);
        return numbuf;
    } else {
        return m_name;
    }
}


void Win16::ResourceGroup::read(ccl::Stream* stream)
{
    m_typeID = stream->read16();
    if (m_typeID == 0)
        return;
    m_count = stream->read16();
    m_reserved = stream->read32();

    m_resources.resize(m_count);
    std::vector<Resource>::iterator iter;
    for (iter = m_resources.begin(); iter != m_resources.end(); ++iter)
        iter->read(stream);
}

void Win16::ResourceGroup::update(ccl::Stream* stream)
{
    stream->seek(8, SEEK_CUR);
    std::vector<Resource>::iterator iter;
    for (iter = m_resources.begin(); iter != m_resources.end(); ++iter)
        iter->update(stream);
}


bool Win16::ResourceDirectory::read(ccl::Stream* stream)
{
    char magic[2];

    stream->seek(0, SEEK_SET);
    stream->read(magic, 1, 2);
    if (memcmp(magic, "MZ", 2) != 0)
        return false;

    // Find the Win16 header
    stream->seek(60, SEEK_SET);
    uint32_t ofNewHeader = stream->read32();

    stream->seek(ofNewHeader, SEEK_SET);
    stream->read(magic, 1, 2);
    if (memcmp(magic, "NE", 2) != 0)
        return false;

    stream->seek(ofNewHeader + 36, SEEK_SET);
    m_dirOffset = stream->read16() + ofNewHeader;

    // Read the resource tree
    stream->seek(m_dirOffset, SEEK_SET);
    m_resAlign = stream->read16();
    for ( ;; ) {
        ResourceGroup group;
        group.read(stream);
        if (group.typeID() == 0)
            break;
        m_groups.push_back(group);
    }

    // Fetch names for the resources
    std::list<ResourceGroup>::iterator giter;
    for (giter = m_groups.begin(); giter != m_groups.end(); ++giter) {
        std::vector<Resource>::iterator rciter;
        for (rciter = giter->resources().begin(); rciter != giter->resources().end(); ++rciter) {
            if (!(rciter->resourceID() & 0x8000)) {
                stream->seek(m_dirOffset + rciter->resourceID(), SEEK_SET);
                uint8_t length = stream->read8();
                char* buffer = new char[length + 1];
                stream->read(buffer, 1, length);
                buffer[length] = 0;
                rciter->setName(buffer);
                delete[] buffer;
            }
        }
    }
    return true;
}

void Win16::ResourceDirectory::update(ccl::Stream* stream)
{
    // Update the resource tree only with necessary info
    stream->seek(m_dirOffset + 2, SEEK_SET);
    std::list<ResourceGroup>::iterator iter;
    for (iter = m_groups.begin(); iter != m_groups.end(); ++iter)
        iter->update(stream);
}

Win16::RcBlob* Win16::ResourceDirectory::loadResource(Resource* res, ccl::Stream* stream)
{
    RcBlob* blob = new RcBlob();
    blob->m_size = res->size() << m_resAlign;
    blob->m_data = new uint8_t[blob->m_size];
    stream->seek(res->offset() << m_resAlign, SEEK_SET);
    stream->read(blob->m_data, 1, blob->m_size);
    return blob;
}

bool Win16::ResourceDirectory::updateResource(Resource* res, ccl::Stream* stream, RcBlob* blob)
{
    std::list<RcBlob*> savedBlobs;

    std::list<ResourceGroup>::iterator giter;
    std::vector<Resource>::iterator rciter;

    // Save backup blobs if the size has changed
    uint32_t alignSize = blob->m_size >> m_resAlign;
    if (blob->m_size % (1 << m_resAlign))
        ++alignSize;
    if (alignSize != res->size()) {
        for (giter = m_groups.begin(); giter != m_groups.end(); ++giter) {
            for (rciter = giter->resources().begin(); rciter != giter->resources().end(); ++rciter) {
                RcBlob* blob = loadResource(&(*rciter), stream);
                savedBlobs.push_back(blob);

                // Update the offsets of blobs after the updated one
                if (rciter->offset() > res->offset())
                    rciter->setOffset(rciter->offset() + (alignSize - res->size()));
            }
        }
    }

    // Now go update all the blobs
    res->setSize(alignSize);
    for (giter = m_groups.begin(); giter != m_groups.end(); ++giter) {
        for (rciter = giter->resources().begin(); rciter != giter->resources().end(); ++rciter) {
            // NOTE: We rely on the fact that the savedBlobs should stay in the
            //       same order during writing as during reading...
            RcBlob* nextBlob = savedBlobs.front();
            savedBlobs.pop_front();
            if (&(*rciter) == res) {
                stream->seek(rciter->offset() << m_resAlign, SEEK_SET);
                stream->write(blob->m_data, 1, blob->m_size);
                if (blob->m_size % (1 << m_resAlign)) {
                    // Pad with zeroes
                    size_t padLength = (1 << m_resAlign) - (blob->m_size % (1 << m_resAlign));
                    uint8_t* zero = new uint8_t[padLength];
                    memset(zero, 0, padLength);
                    stream->write(zero, 1, padLength);
                    delete[] zero;
                }
            } else {
                stream->seek(rciter->offset() << m_resAlign, SEEK_SET);
                stream->write(nextBlob->m_data, 1, nextBlob->m_size);
            }
            delete nextBlob;
        }
    }

    // And finally update the resource table
    update(stream);
    return true;
}
