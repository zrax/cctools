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

#include "IniFile.h"

#include <cstring>
#include <cstdlib>
#include "Errors.h"

void ccl::IniFile::read(FILE* stream)
{
    char buffer[1024];

    m_sections.clear();
    while (fgets(buffer, 1024, stream) != 0) {
        // strip newline chars
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\r' || buffer[len-1] == '\n'))
            --len;
        buffer[len] = 0;

        // Look for section header
        if (buffer[0] == '[') {
            char* endp = strrchr(buffer, ']');
            if (endp == 0)
                throw ccl::FormatException("Invalid INI file format");
            *endp = 0;    // Ensure null termination
            setSection(buffer + 1);
            continue;
        }

        // Split line into key = value pairs
        char* key = buffer;
        char* value = strchr(buffer, '=');
        if (value == 0) {
            // Verify line is empty, throw error otherwise
            for (char* ch = buffer; *ch != 0; ++ch) {
                if (!isspace(*ch))
                    throw ccl::FormatException("Invalid INI file format");
            }
            continue;
        }

        // Strip spaces around = character
        len = value - key;
        while (len > 0 && isspace(key[len-1]))
            --len;
        key[len] = 0;
        do {
            ++value;
        } while (isspace(*value));

        // Store the key and value pair
        setString(key, value);
    }
}

void ccl::IniFile::write(FILE* stream)
{
    // In case the file is already open
    ftruncate(fileno(stream), 0);
    fseek(stream, 0, SEEK_SET);

    // Write sectionless part first
    std::map<std::string, Section>::iterator section_iter = m_sections.find("");
    Section::iterator value_iter;
    if (section_iter != m_sections.end()) {
        for (value_iter = section_iter->second.begin();
             value_iter != section_iter->second.end(); ++value_iter)
            fprintf(stream, "%s=%s\n", value_iter->first.c_str(), value_iter->second.c_str());
    }

    // All other sections
    for (section_iter = m_sections.begin(); section_iter != m_sections.end(); ++section_iter) {
        if (section_iter->first == "")
            continue;

        fprintf(stream, "[%s]\n", section_iter->first.c_str());
        for (value_iter = section_iter->second.begin();
             value_iter != section_iter->second.end(); ++value_iter)
            fprintf(stream, "%s=%s\n", value_iter->first.c_str(), value_iter->second.c_str());
    }
}

int ccl::IniFile::getInt(const std::string& name, int defaultValue)
{
    std::map<std::string, Section>::iterator section_iter = m_sections.find(m_activeSection);
    if (section_iter == m_sections.end())
        return defaultValue;

    Section::iterator value_iter = section_iter->second.find(name);
    if (value_iter == section_iter->second.end())
        return defaultValue;

    return (int)strtol(value_iter->second.c_str(), 0, 0);
}

std::string ccl::IniFile::getString(const std::string& name, const std::string& defaultValue)
{
    std::map<std::string, Section>::iterator section_iter = m_sections.find(m_activeSection);
    if (section_iter == m_sections.end())
        return defaultValue;

    Section::iterator value_iter = section_iter->second.find(name);
    if (value_iter == section_iter->second.end())
        return defaultValue;

    return value_iter->second;
}

void ccl::IniFile::setInt(const std::string& name, int value)
{
    char buffer[16];
    snprintf(buffer, 16, "%d", value);
    setString(name, buffer);
}
