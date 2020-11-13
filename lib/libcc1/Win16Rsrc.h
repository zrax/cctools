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

#ifndef _WIN16RSRC_H
#define _WIN16RSRC_H

#include <vector>
#include <list>
#include "Stream.h"

namespace Win16 {

enum ResourceTypes {
    RT_None, RT_Cursor, RT_Bitmap, RT_Icon, RT_Menu, RT_Dialog, RT_String,
    RT_FontDir, RT_Font, RT_Accelerator, RT_RCData, RT_MessageTable,
    RT_GroupCursor = RT_Cursor + 11, RT_GroupIcon = RT_Icon + 11,
    RT_Version = 16, RT_DlgInclude,
};

struct RcBlob {
    RcBlob() noexcept : m_size(), m_data() { }
    ~RcBlob() { delete[] m_data; }

    RcBlob(const RcBlob&) = delete;
    RcBlob& operator=(const RcBlob&) = delete;

    RcBlob(RcBlob&& init) noexcept : m_size(init.m_size), m_data(init.m_data)
    {
        init.m_data = nullptr;
    }

    RcBlob& operator=(RcBlob&& init) noexcept
    {
        m_size = init.m_size;
        std::swap(m_data, init.m_data);
        return *this;
    }

    bool isNull() const { return m_data == nullptr; }

    uint32_t m_size;
    uint8_t* m_data;
};

class Resource {
public:
    Resource()
        : m_offset(), m_size(), m_flags(), m_resourceID(), m_reserved() { }

    Resource(const Resource& copy) = default;
    Resource& operator=(const Resource& copy) = default;

    void read(ccl::Stream* stream);
    void update(ccl::Stream* stream) const;
    void setName(std::string name) { m_name = std::move(name); }

    // These are 32-bit to make shifting easier
    uint32_t offset() const { return (uint32_t)m_offset; }
    uint32_t size() const { return (uint32_t)m_size; }

    void setOffset(uint16_t offset) { m_offset = offset; }
    void setSize(uint16_t size) { m_size = size; }

    uint16_t flags() const { return m_flags; }
    uint16_t resourceID() const { return m_resourceID; }
    std::string name() const;

private:
    uint16_t m_offset;
    uint16_t m_size;
    uint16_t m_flags;
    uint16_t m_resourceID;
    uint32_t m_reserved;
    std::string m_name;
};

class ResourceGroup {
public:
    ResourceGroup() : m_typeID(), m_count(), m_reserved() { }

    ResourceGroup(const ResourceGroup& copy) = default;
    ResourceGroup& operator=(const ResourceGroup& copy) = default;

    void read(ccl::Stream* stream);
    void update(ccl::Stream* stream) const;

    uint16_t typeID() const { return m_typeID; }
    uint16_t count() const { return m_count; }

    std::vector<Resource>& resources() { return m_resources; }
    const std::vector<Resource>& resources() const { return m_resources; }

private:
    uint16_t m_typeID;
    uint16_t m_count;
    uint32_t m_reserved;

    std::vector<Resource> m_resources;
};

class ResourceDirectory {
public:
    ResourceDirectory() : m_dirOffset(), m_resAlign() { }

    bool read(ccl::Stream* stream);
    void update(ccl::Stream* stream) const;

    Resource* findResource(uint16_t type, const std::string& name)
    {
        for (ResourceGroup& group : m_groups) {
            if ((group.typeID() & ~0x8000) != type)
                continue;

            for (Resource& resource : group.resources()) {
                if (resource.name() == name)
                    return &resource;
            }
        }
        return nullptr;
    }

    Resource* findResource(uint16_t type, uint16_t resourceID)
    {
        for (ResourceGroup& group : m_groups) {
            if ((group.typeID() & ~0x8000) != type)
                continue;

            for (Resource& resource : group.resources()) {
                if ((resource.resourceID() & ~0x8000) == resourceID)
                    return &resource;
            }
        }
        return nullptr;
    }

    RcBlob loadResource(uint16_t type, const std::string& name, ccl::Stream* stream)
    {
        Resource* res = findResource(type, name);
        return res ? loadResource(res, stream) : RcBlob();
    }

    RcBlob loadResource(uint16_t type, uint16_t resourceID, ccl::Stream* stream)
    {
        Resource* res = findResource(type, resourceID);
        return res ? loadResource(res, stream) : RcBlob();
    }

    bool updateResource(uint16_t type, const std::string& name,
                        ccl::Stream* stream, const RcBlob& blob)
    {
        Resource* res = findResource(type, name);
        if (res)
            return updateResource(res, stream, blob);
        return false;
    }

    bool updateResource(uint16_t type, uint16_t resourceID,
                        ccl::Stream* stream, const RcBlob& blob)
    {
        Resource* res = findResource(type, resourceID);
        if (res)
            return updateResource(res, stream, blob);
        return false;
    }

private:
    uint32_t m_dirOffset;
    uint16_t m_resAlign;
    std::list<ResourceGroup> m_groups;

    RcBlob loadResource(const Resource* res, ccl::Stream* stream);
    bool updateResource(Resource* res, ccl::Stream* stream, const RcBlob& blob);
};

class RcMenuItem {
public:
    enum {
        MF_GRAYED = 0x01,
        MF_DISABLED = 0x02,
        MF_CHECKED = 0x08,
        MF_POPUP = 0x10,
        MF_MENUBARBREAK = 0x20,
        MF_MENUBREAK = 0x40,
        _LAST_CHILD = 0x80,
    };

    RcMenuItem() : m_flags(), m_id() { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream);

    uint16_t flags() const { return m_flags; }
    uint16_t id() const { return m_id; }
    const std::string& name() const { return m_name; }

    void setFlags(uint16_t flags) { m_flags = flags; }
    void setId(uint16_t id) { m_id = id; }
    void setName(std::string name) { m_name = std::move(name); }

    std::vector<RcMenuItem>& children() { return m_children; }
    const std::vector<RcMenuItem>& children() const { return m_children; }

private:
    friend class MenuResource;

    uint16_t m_flags;
    uint16_t m_id;
    std::string m_name;
    std::vector<RcMenuItem> m_children;
};

class MenuResource {
public:
    MenuResource() : m_version(), m_offset() { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream);

    uint16_t version() const { return m_version; }
    uint16_t offset() const { return m_offset; }

    void setVersion(uint16_t version) { m_version = version; }
    void setOffset(uint16_t offset) { m_offset = offset; }

    std::vector<RcMenuItem>& menus() { return m_menus; }
    const std::vector<RcMenuItem>& menus() const { return m_menus; }

private:
    uint16_t m_version;
    uint16_t m_offset;
    std::vector<RcMenuItem> m_menus;
};

class Accelerator {
public:
    enum {
        FVIRTKEY = 0x01,
        FNOINVERT = 0x02,
        FSHIFT = 0x04,
        FCONTROL = 0x08,
        FALT = 0x10,
        _LAST_CHILD = 0x80,
    };

    Accelerator() : m_flags(), m_event(), m_id(), m_order(-1) { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    uint8_t flags() const { return m_flags; }
    uint16_t event() const { return m_event; }
    uint16_t id() const { return m_id; }

    void setFlags(uint8_t flags) { m_flags = flags; }
    void setEvent(uint16_t event) { m_event = event; }
    void setId(uint16_t id) { m_id = id; }

    int order() const { return m_order; }
    void setOrder(int order) { m_order = order; }

private:
    friend class AccelResource;

    uint8_t m_flags;
    uint16_t m_event, m_id;

    // Only used for preserving table order in CCHack
    int m_order;
};

class AccelResource {
public:
    AccelResource() { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream);

    std::vector<Accelerator>& accelerators() { return m_accelerators; }
    const std::vector<Accelerator>& accelerators() const { return m_accelerators; }

private:
    std::vector<Accelerator> m_accelerators;
};

}

#endif
