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

#ifndef _INIFILE_H
#define _INIFILE_H

#include <map>
#include <string>
#include <cstdio>

namespace ccl {

class IniFile {
public:
    IniFile() { }

    void read(FILE* stream);
    void write(FILE* stream);

    void setSection(const std::string& section)
    { m_activeSection = section; }

    int getInt(const std::string& name, int defaultValue = 0);
    std::string getString(const std::string& name, const std::string& defaultValue = "");

    void setInt(const std::string& name, int value);
    void setString(const std::string& name, const std::string& value)
    { m_sections[m_activeSection][name] = value; }

private:
    typedef std::map<std::string, std::string> Section;
    std::map<std::string, Section> m_sections;
    std::string m_activeSection;
};

}

#endif