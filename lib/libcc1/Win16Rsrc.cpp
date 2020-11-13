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
#include <queue>

#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

void Win16::Resource::read(ccl::Stream* stream)
{
    m_offset = stream->read16();
    m_size = stream->read16();
    m_flags = stream->read16();
    m_resourceID = stream->read16();
    m_reserved = stream->read32();
}

void Win16::Resource::update(ccl::Stream* stream) const
{
    stream->write16(m_offset);
    stream->write16(m_size);
    stream->seek(8, SEEK_CUR);
}

std::string Win16::Resource::name() const
{
    if (m_resourceID & 0x8000) {
        char numbuf[16];
        snprintf(numbuf, 16, "%u", (uint32_t)(m_resourceID & ~0x8000));
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
    for (Resource& resource : m_resources)
        resource.read(stream);
}

void Win16::ResourceGroup::update(ccl::Stream* stream) const
{
    stream->seek(8, SEEK_CUR);
    for (const Resource& resource : m_resources)
        resource.update(stream);
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
    for (ResourceGroup& group : m_groups) {
        for (Resource& resource : group.resources()) {
            if (!(resource.resourceID() & 0x8000)) {
                stream->seek(m_dirOffset + resource.resourceID(), SEEK_SET);
                uint8_t length = stream->read8();
                std::string buffer;
                buffer.resize(length);
                stream->read(&buffer[0], 1, length);
                resource.setName(std::move(buffer));
            }
        }
    }
    return true;
}

void Win16::ResourceDirectory::update(ccl::Stream* stream) const
{
    // Update the resource tree only with necessary info
    stream->seek(m_dirOffset + 2, SEEK_SET);
    for (const ResourceGroup& group : m_groups)
        group.update(stream);
}

Win16::RcBlob Win16::ResourceDirectory::loadResource(const Resource* res, ccl::Stream* stream)
{
    RcBlob blob;
    blob.m_size = res->size() << m_resAlign;
    blob.m_data = new uint8_t[blob.m_size];
    stream->seek(res->offset() << m_resAlign, SEEK_SET);
    stream->read(blob.m_data, 1, blob.m_size);
    return blob;
}

bool Win16::ResourceDirectory::updateResource(Resource* res, ccl::Stream* stream, const RcBlob& blob)
{
    std::queue<RcBlob> savedBlobs;

    // Save backup blobs if the size has changed
    uint32_t alignSize = blob.m_size >> m_resAlign;
    if (blob.m_size % (1 << m_resAlign))
        ++alignSize;

    for (ResourceGroup& group : m_groups) {
        for (Resource& resource : group.resources()) {
            savedBlobs.push(loadResource(&resource, stream));

            // Update the offsets of blobs after the updated one
            if (resource.offset() > res->offset())
                resource.setOffset(resource.offset() + (alignSize - res->size()));
        }
    }

    // Now go update all the blobs
    res->setSize(alignSize);
    for (const ResourceGroup& group : m_groups) {
        for (const Resource& resource : group.resources()) {
            // NOTE: We rely on the fact that the savedBlobs should stay in the
            //       same order during writing as during reading...
            RcBlob nextBlob = std::move(savedBlobs.front());
            savedBlobs.pop();
            if (&resource == res) {
                stream->seek(resource.offset() << m_resAlign, SEEK_SET);
                stream->write(blob.m_data, 1, blob.m_size);
                if (blob.m_size % (1 << m_resAlign)) {
                    // Pad with zeroes
                    size_t padLength = (1 << m_resAlign) - (blob.m_size % (1 << m_resAlign));
                    std::vector<uint8_t> zero(padLength, 0);
                    stream->write(&zero[0], 1, padLength);
                }
            } else {
                stream->seek(resource.offset() << m_resAlign, SEEK_SET);
                stream->write(nextBlob.m_data, 1, nextBlob.m_size);
            }
        }
    }

    // And finally update the resource table
    update(stream);
    return true;
}


void Win16::RcMenuItem::read(ccl::Stream* stream)
{
    m_flags = stream->read16();
    if (m_flags & MF_POPUP) {
        m_name = stream->readZString();

        do {
            m_children.emplace_back();
            m_children.back().read(stream);
        } while (!(m_children.back().m_flags & _LAST_CHILD));
    } else {
        m_id = stream->read16();
        m_name = stream->readZString();
    }
}

void Win16::RcMenuItem::write(ccl::Stream* stream)
{
    if (m_children.empty())
        m_flags &= ~MF_POPUP;
    else
        m_flags |= MF_POPUP;

    stream->write16(m_flags);
    if (m_flags & MF_POPUP) {
        stream->writeZString(m_name);
        for (RcMenuItem& item : m_children)
            item.m_flags &= ~_LAST_CHILD;
        m_children.back().m_flags |= _LAST_CHILD;
        for (RcMenuItem& item : m_children)
            item.write(stream);
    } else {
        stream->write16(m_id);
        stream->writeZString(m_name);
    }
}


void Win16::MenuResource::read(ccl::Stream* stream)
{
    m_version = stream->read16();
    m_offset = stream->read16();

    do {
        m_menus.emplace_back();
        m_menus.back().read(stream);
    } while (!(m_menus.back().m_flags & RcMenuItem::_LAST_CHILD));
}

void Win16::MenuResource::write(ccl::Stream* stream)
{
    stream->write16(m_version);
    stream->write16(m_offset);

    if (m_menus.empty())
        throw ccl::RuntimeError(ccl::RuntimeError::tr("MenuResource cannot be empty!"));

    for (RcMenuItem& item : m_menus)
        item.m_flags &= ~RcMenuItem::_LAST_CHILD;
    m_menus.back().m_flags |= RcMenuItem::_LAST_CHILD;
    for (RcMenuItem& item : m_menus)
        item.write(stream);
}


void Win16::Accelerator::read(ccl::Stream* stream)
{
    m_flags = stream->read8();
    m_event = stream->read16();
    m_id = stream->read16();
}

void Win16::Accelerator::write(ccl::Stream *stream) const
{
    stream->write8(m_flags);
    stream->write16(m_event);
    stream->write16(m_id);
}


void Win16::AccelResource::read(ccl::Stream *stream)
{
    int order = 0;
    do {
        m_accelerators.emplace_back();
        m_accelerators.back().read(stream);
        m_accelerators.back().setOrder(++order);
    } while (!(m_accelerators.back().m_flags & Accelerator::_LAST_CHILD));
}

void Win16::AccelResource::write(ccl::Stream *stream)
{
    if (m_accelerators.empty())
        throw ccl::RuntimeError(ccl::RuntimeError::tr("AccelResource cannot be empty!"));

    for (Accelerator& item : m_accelerators)
        item.m_flags &= ~Accelerator::_LAST_CHILD;
    m_accelerators.back().m_flags |= Accelerator::_LAST_CHILD;
    for (Accelerator& item : m_accelerators)
        item.write(stream);
}
