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
#include <QWidget>
#include "libcc1/Stream.h"

#define MAKE_SETTING(type, name) \
    private: \
        bool m_##name##_set; \
        type m_##name##_value; \
    public: \
        bool have_##name() const { return m_##name##_set; } \
        type get_##name() const { return m_##name##_value; } \
        void set_##name(type value) { m_##name##_value = value; m_##name##_set = true; } \
        void clear_##name() { m_##name##_value = type(); m_##name##_set = false; }

#define MAKE_SETTING_STR(name) \
    private: \
        bool m_##name##_set; \
        std::string m_##name##_value; \
    public: \
        bool have_##name() const { return m_##name##_set; } \
        const std::string& get_##name() const { return m_##name##_value; } \
        void set_##name(std::string value) \
            { m_##name##_value = std::move(value); m_##name##_set = true; } \
        void clear_##name() { m_##name##_value.clear(); m_##name##_set = false; }

class HackSettings {
public:
    HackSettings() { clearAll(); }

    void setKnownDefaults();
    void clearAll();

    bool loadFromExe(const char* filename);
    bool loadFromPatch(const char* filename);
    bool writeToExe(const char* filename) const;
    bool writeToPatch(const char* filename) const;

    // General settings
    MAKE_SETTING_STR(title);
    MAKE_SETTING_STR(iniFile);
    MAKE_SETTING_STR(iniEntry);
    MAKE_SETTING_STR(datFile);
    MAKE_SETTING(bool, alwaysFirstTry);
    MAKE_SETTING(bool, ccPatch);
    MAKE_SETTING(bool, pgChips);
    MAKE_SETTING(int, fakeLastLevel);
    MAKE_SETTING(int, realLastLevel);
};

class HackPage : public QWidget {
public:
    HackPage(QWidget* parent = nullptr) : QWidget(parent) { }
    virtual void setValues(HackSettings* settings) = 0;
    virtual void setDefaults(HackSettings* settings) = 0;
};

#endif
