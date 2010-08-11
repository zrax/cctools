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

#ifndef _CHIPSHAX_H
#define _CHIPSHAX_H

#include <list>
#include <string>
#include <cstring>
#include "Stream.h"

#define HACK_BOOL16(name, addr) \
    void set_##name(bool b) \
    { \
        uint16_t value = b ? 1 : 0; \
        m_stream->seek(addr, SEEK_SET); \
        m_stream->write(&value, sizeof(uint16_t), 1); \
    } \
    bool get_##name() \
    { \
        uint16_t value; \
        m_stream->seek(addr, SEEK_SET); \
        m_stream->read(&value, sizeof(uint16_t), 1); \
        return (value != 0); \
    }

#define HACK_STRING(name, addr, maxLength) \
    void set_##name(const std::string& str) \
    { \
        char value[maxLength+1]; \
        strncpy(value, str.c_str(), maxLength); \
        value[maxLength] = 0; \
        m_stream->seek(addr, SEEK_SET); \
        m_stream->write(value, 1, maxLength+1); \
    } \
    std::string get_##name() \
    { \
        char value[maxLength+1]; \
        m_stream->seek(addr, SEEK_SET); \
        m_stream->read(value, 1, maxLength); \
        value[maxLength] = 0; \
        return value; \
    }

namespace ccl {

enum CCPatchState { CCPatchOriginal, CCPatchPatched, CCPatchOther };

class ChipsHax {
public:
    ChipsHax() : m_stream(0) { }
    void open(ccl::Stream* stream) { m_stream = stream; }

    /* Enter all available hacks here */
    HACK_BOOL16(IgnorePasswords,    0x482E      )   // No need for the menu cheat
    HACK_STRING(DialogTitle,        0x4868,   17)   // Used for message boxes
    HACK_STRING(FireDeathMsg,       0x4976,   49)
    HACK_STRING(WaterDeathMsg,      0x49A8,   41)
    HACK_STRING(BombDeathMsg,       0x49D2,   29)
    HACK_STRING(BlockDeathMsg,      0x49F0,   35)
    HACK_STRING(CreatureDeathMsg,   0x4A14,   31)
    HACK_STRING(TimeLimitMsg,       0x4A34,   19)
    HACK_STRING(IniFilename,        0x4A68,   11)
    HACK_STRING(IniEntryName,       0x4A74,   17)
    HACK_STRING(DataFilename,       0x4AD4,    9)
    HACK_STRING(WindowTitle,        0x4D16,   16)   // Displayed in main window
    HACK_STRING(DefaultDeath,       0x4D7C,    6)
    HACK_STRING(CorruptFileMsg,     0x5166,   39)
    HACK_STRING(FirstTryMsg,        0x5334,   18)   // Yowser! First Try!
    HACK_STRING(ThirdTryMsg,        0x5347,   14)   // Go Bit Buster!
    HACK_STRING(FifthTryMsg,        0x5356,   20)   // Finished! Good work!
    HACK_STRING(FinalTryMsg,        0x536B,   20)   // At last! You did it!
    HACK_STRING(EndgameMsg1,        0x546E,   57)
    HACK_STRING(EndgameMsg2,        0x54A8,  155)
    HACK_STRING(ProgressMsg1,       0x56D6,  129)
    HACK_STRING(ProgressMsg2,       0x5758,  114)
    HACK_STRING(ProgressMsg3,       0x57CB,  121)
    HACK_STRING(ProgressMsg4,       0x5845,  123)
    HACK_STRING(ProgressMsg5,       0x58C1,  120)
    HACK_STRING(ProgressMsg6,       0x593A,  114)
    HACK_STRING(ProgressMsg7,       0x59AD,  125)
    HACK_STRING(ProgressMsg8,       0x5A2B,  101)
    HACK_STRING(ProgressMsg9,       0x5A91,  131)
    HACK_STRING(ProgressMsg10,      0x5B15,   60)

    void set_LastLevel(int level);
    void set_FakeLastLevel(int level);
    int get_LastLevel();
    int get_FakeLastLevel();

    void set_AlwaysFirstTry(bool on);
    bool get_AlwaysFirstTry();

    // CCExplore's patch for a crash when chip enters a square occupied
    // by two masked objects (e.g. monsters and keys)
    void set_CCPatch(CCPatchState state);
    CCPatchState get_CCPatch();

private:
    ccl::Stream* m_stream;
};

}

#endif
