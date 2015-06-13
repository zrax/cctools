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

#ifndef _HACK_SETTINGS_H
#define _HACK_SETTINGS_H

#include <string>
#include <QObject>
#include "libcc1/Stream.h"

#define MAKE_SETTING(type, name) \
    private: \
        bool m_##name##_set; \
        type m_##name##_value; \
    public: \
        bool isset_##name() const { return m_##name##_set; } \
        type get_##name() const { return m_##name##_value; } \
        void set_##name(type value, bool isset = true) \
        { m_##name##_value = value; m_##name##_set = isset; }

class HackSettings {
public:
    HackSettings() { clearAll(); }

    void setKnownDefaults();
    void clearAll();

    bool loadFromExe(const char* filename);
    bool loadFromPatch(const char* filename);
    bool writeToExe(const char* filename);
    bool writeToPatch(const char* filename);

    // General settings
    MAKE_SETTING(std::string, title);
    MAKE_SETTING(std::string, iniFile);
    MAKE_SETTING(std::string, iniEntry);
    MAKE_SETTING(std::string, datFile);
    MAKE_SETTING(bool,        alwaysFirstTry);
    MAKE_SETTING(bool,        ccPatch);
    MAKE_SETTING(bool,        pgChips);
    MAKE_SETTING(int,         fakeLastLevel);
    MAKE_SETTING(int,         realLastLevel);
};

class HackPage : public QObject {
public:
    HackPage(QObject* parent = 0) : QObject(parent) { }
    virtual ~HackPage() { }
    virtual void setValues(HackSettings* settings) = 0;
    virtual void setDefaults(HackSettings* settings) = 0;
};

#endif
