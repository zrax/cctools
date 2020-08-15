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

#ifndef _DACFILE_H
#define _DACFILE_H

#include <string>
#include <cstdio>

namespace ccl {

class Levelset;

struct DacFile {
    DacFile()
        : m_ruleset(), m_lastLevel(), m_usePasswords(true), m_fixLynx(false)
    { }

    void setFromLevelset(const Levelset& levelset);

    void read(FILE* stream);
    void write(FILE* stream) const;

    std::string m_filename;
    unsigned int m_ruleset;
    int m_lastLevel;        // use 0 to indicate default value
    bool m_usePasswords;    // default = y
    bool m_fixLynx;         // default = n
};

} /* {ccl} */

#endif
