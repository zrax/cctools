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

#include "HackSettings.h"
#include "libcc1/ChipsHax.h"

void HackSettings::setKnownDefaults()
{
    set_title("Chip's Challenge");
    set_iniFile("entpack.ini");
    set_iniEntry("Chip's Challenge");
    set_datFile("CHIPS.DAT");
    set_alwaysFirstTry(false);
    set_ccPatch(false);
    set_pgChips(false);
    set_fakeLastLevel(144);
    set_realLastLevel(149);
}

void HackSettings::clearAll()
{
    clear_title();
    clear_iniFile();
    clear_iniEntry();
    clear_datFile();
    clear_alwaysFirstTry();
    clear_ccPatch();
    clear_pgChips();
    clear_fakeLastLevel();
    clear_realLastLevel();
}

bool HackSettings::loadFromExe(const char* filename)
{
    ccl::ChipsHax hax;
    ccl::FileStream exeStream;
    if (!exeStream.open(filename, "rb"))
        return false;

    hax.open(&exeStream);

    // General settings
    set_title(hax.get_WindowTitle());
    set_iniFile(hax.get_IniFilename());
    set_iniEntry(hax.get_IniEntryName());
    set_datFile(hax.get_DataFilename());
    set_alwaysFirstTry(hax.get_AlwaysFirstTry());
    set_ccPatch(hax.get_CCPatch() == ccl::CCPatchPatched);
    set_pgChips(hax.get_PGChips() == ccl::CCPatchPatched);
    set_fakeLastLevel(hax.get_FakeLastLevel());
    set_realLastLevel(hax.get_LastLevel());

    exeStream.close();
    return true;
}

bool HackSettings::loadFromPatch(const char* filename)
{
    //TODO
    return false;
}

bool HackSettings::writeToExe(const char* filename)
{
    ccl::ChipsHax hax;
    ccl::FileStream exeStream;
    if (!exeStream.open(filename, "r+b"))
        return false;

    hax.open(&exeStream);

    // Do these first as a way to check if the EXE is sane
    if (have_ccPatch()) {
        if (hax.get_CCPatch() == ccl::CCPatchOther)
            return false;
        hax.set_CCPatch(get_ccPatch() ? ccl::CCPatchPatched : ccl::CCPatchOriginal);
    }
    if (have_pgChips()) {
        if (hax.get_PGChips() == ccl::CCPatchOther)
            return false;
        hax.set_PGChips(get_pgChips() ? ccl::CCPatchPatched : ccl::CCPatchOriginal);
    }

    // General settings
    if (have_title()) {
        hax.set_WindowTitle(get_title());
        hax.set_DialogTitle(get_title());
    }
    if (have_iniFile())
        hax.set_IniFilename(get_iniFile());
    if (have_iniEntry())
        hax.set_IniEntryName(get_iniEntry());
    if (have_datFile())
        hax.set_DataFilename(get_datFile());
    if (have_alwaysFirstTry())
        hax.set_AlwaysFirstTry(get_alwaysFirstTry());
    if (have_fakeLastLevel())
        hax.set_FakeLastLevel(get_fakeLastLevel());
    if (have_realLastLevel())
        hax.set_LastLevel(get_realLastLevel());

    exeStream.close();
    return true;
}

bool HackSettings::writeToPatch(const char* filename)
{
    //TODO
    return false;
}
