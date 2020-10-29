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

#include "DacFile.h"

#include <cstring>
#include <cstdlib>
#include <errno.h>
#include "Errors.h"
#include "Levelset.h"

#ifdef _WIN32
    #define strcasecmp  _stricmp
#endif

void ccl::DacFile::setFromLevelset(const ccl::Levelset& levelset)
{
    m_ruleset = levelset.type();
    m_lastLevel = (levelset.levelCount() == 149) ? 144 : 0;
    m_usePasswords = true;
    m_fixLynx = false;
}

void ccl::DacFile::read(FILE* stream)
{
    char buffer[1024];

    while (fgets(buffer, 1024, stream) != nullptr) {
        // strip newline chars
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\r' || buffer[len-1] == '\n'))
            --len;
        buffer[len] = 0;

        // Split line into key = value pairs
        char* key = buffer;
        char* value = strchr(buffer, '=');
        if (!value) {
            // Verify line is empty, throw error otherwise
            for (char* ch = buffer; *ch != 0; ++ch) {
                if (!isspace(*ch))
                    throw ccl::FormatError(ccl::RuntimeError::tr("Invalid DAC file format"));
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

        // Now we can finally make use of key and value!
        if (strcasecmp(key, "file") == 0) {
            m_filename = value;
        } else if (strcasecmp(key, "usepasswords") == 0) {
            if (strcasecmp(value, "y") == 0)
                m_usePasswords = true;
            else if (strcasecmp(value, "n") == 0)
                m_usePasswords = false;
            else {
                throw ccl::FormatError(ccl::RuntimeError::tr(
                        "Invalid parameter, expected 'y' or 'n'"));
            }
        } else if (strcasecmp(key, "ruleset") == 0) {
            if (strcasecmp(value, "ms") == 0)
                m_ruleset = ccl::Levelset::TypeMS;
            else if (strcasecmp(value, "lynx") == 0)
                m_ruleset = ccl::Levelset::TypeLynx;
            else {
                throw ccl::FormatError(ccl::RuntimeError::tr(
                        "Invalid parameter, expected 'ms' or 'lynx'"));
            }
        } else if (strcasecmp(key, "lastlevel") == 0) {
            errno = 0;
            m_lastLevel = (int)strtol(value, nullptr, 10);
            if (m_lastLevel == 0 && errno != 0) {
                throw ccl::FormatError(ccl::RuntimeError::tr(
                        "Invalid parameter, expected integer constant"));
            }
        } else if (strcasecmp(key, "fixlynx") == 0) {
            if (strcasecmp(value, "y") == 0)
                m_fixLynx = true;
            else if (strcasecmp(value, "n") == 0)
                m_fixLynx = false;
            else {
                throw ccl::FormatError(ccl::RuntimeError::tr(
                        "Invalid parameter, expected 'y' or 'n'"));
            }
        } else {
            throw ccl::FormatError(ccl::RuntimeError::tr(
                    "Unexpected/unsupported DAC parameter"));
        }
    }
}

void ccl::DacFile::write(FILE* stream) const
{
    fprintf(stream, "file=%s\n", m_filename.c_str());
    if (!m_usePasswords)
        fprintf(stream, "usepasswords=n\n");
    if (m_ruleset == ccl::Levelset::TypeMS)
        fprintf(stream, "ruleset=ms\n");
    else if (m_ruleset == ccl::Levelset::TypeLynx)
        fprintf(stream, "ruleset=lynx\n");
    else
        throw ccl::FormatError(ccl::RuntimeError::tr("Invalid ruleset value"));
    if (m_lastLevel != 0)
        fprintf(stream, "lastlevel=%d\n", m_lastLevel);
    if (m_fixLynx)
        fprintf(stream, "fixlynx=y\n");
}
