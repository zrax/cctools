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
#include "libcc1/Win16Rsrc.h"

#include <QFile>
#include <QMessageBox>

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

    clear_vgaTileset();
    clear_egaTileset();
    clear_monoTileset();
    clear_background();
    clear_digits();
    clear_infoBox();
    clear_chipEnd();
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

    clear_vgaTileset();
    clear_egaTileset();
    clear_monoTileset();
    clear_background();
    clear_digits();
    clear_infoBox();
    clear_chipEnd();
}

bool HackSettings::loadFromExe(const QString& filename)
{
    ccl::ChipsHax hax;
    ccl::FileStream exeStream;
    if (!exeStream.open(filename.toLocal8Bit().constData(), "rb"))
        return false;

    hax.open(&exeStream);
    ccl::CCPatchState state = hax.get_CCPatch();
    ccl::CCPatchState pg_state = hax.get_PGChips();
    if (state == ccl::CCPatchOther || pg_state == ccl::CCPatchOther)
        throw std::runtime_error("Unrecognized EXE format");

    // General settings
    set_title(hax.get_WindowTitle());
    set_iniFile(hax.get_IniFilename());
    set_iniEntry(hax.get_IniEntryName());
    set_datFile(hax.get_DataFilename());
    set_alwaysFirstTry(hax.get_AlwaysFirstTry());
    set_ccPatch(state == ccl::CCPatchPatched);
    set_pgChips(pg_state == ccl::CCPatchPatched);
    set_fakeLastLevel(hax.get_FakeLastLevel());
    set_realLastLevel(hax.get_LastLevel());

    Win16::ResourceDirectory rcDir;
    Win16::RcBlob blob;
    rcDir.read(&exeStream);
    blob = rcDir.loadResource(Win16::RT_Bitmap, "OBJ32_4", &exeStream);
    if (!blob.isNull())
        set_vgaTileset(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, "OBJ32_4E", &exeStream);
    if (!blob.isNull())
        set_egaTileset(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, "OBJ32_1", &exeStream);
    if (!blob.isNull())
        set_monoTileset(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, "BACKGROUND", &exeStream);
    if (!blob.isNull())
        set_background(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, 200, &exeStream);
    if (!blob.isNull())
        set_digits(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, "INFOWND", &exeStream);
    if (!blob.isNull())
        set_infoBox(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Bitmap, "CHIPEND", &exeStream);
    if (!blob.isNull())
        set_chipEnd(QByteArray((const char*)blob.m_data, blob.m_size));

    exeStream.close();
    return true;
}

bool HackSettings::loadFromPatch(const QString& filename)
{
    //TODO
    return false;
}

static Win16::RcBlob toBlob(const QByteArray& ba)
{
    Win16::RcBlob blob;
    blob.m_size = ba.size();
    blob.m_data = new uint8_t[blob.m_size];
    memcpy(blob.m_data, ba.constData(), blob.m_size);
    return blob;
}

bool HackSettings::writeToExe(const QString& filename) const
{
    ccl::ChipsHax hax;
    ccl::FileStream exeStream;
    if (!exeStream.open(filename.toLocal8Bit().constData(), "r+b"))
        return false;

    hax.open(&exeStream);
    ccl::CCPatchState state = hax.get_CCPatch();
    ccl::CCPatchState pg_state = hax.get_PGChips();
    if (state == ccl::CCPatchOther || pg_state == ccl::CCPatchOther)
        throw std::runtime_error("Unrecognized EXE format");

    if (have_pgChips()) {
        // We do this first (before writing graphics) so the graphics patch
        // has a chance of applying cleanly.
        pg_state = hax.validate_PGChips();
        if (pg_state == ccl::CCPatchOther) {
            QMessageBox::warning(nullptr, QObject::tr("Cannot apply patch"),
                    QObject::tr("Cannot apply PGChips Patch -- the executable doesn't "
                                "match the expected format.  Perhaps a custom tileset "
                                "graphic has already been written?"));
        } else {
            hax.set_PGChips(get_pgChips() ? ccl::CCPatchPatched : ccl::CCPatchOriginal);
        }
    }

    if (have_ccPatch())
        hax.set_CCPatch(get_ccPatch() ? ccl::CCPatchPatched : ccl::CCPatchOriginal);

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

    Win16::ResourceDirectory rcDir;
    rcDir.read(&exeStream);
    if (have_vgaTileset()) {
        Win16::RcBlob blob = toBlob(get_vgaTileset());
        rcDir.updateResource(Win16::RT_Bitmap, "OBJ32_4", &exeStream, blob);
    }
    if (have_egaTileset()) {
        Win16::RcBlob blob = toBlob(get_egaTileset());
        rcDir.updateResource(Win16::RT_Bitmap, "OBJ32_4E", &exeStream, blob);
    }
    if (have_monoTileset()) {
        Win16::RcBlob blob = toBlob(get_monoTileset());
        rcDir.updateResource(Win16::RT_Bitmap, "OBJ32_1", &exeStream, blob);
    }
    if (have_background()) {
        Win16::RcBlob blob = toBlob(get_background());
        rcDir.updateResource(Win16::RT_Bitmap, "BACKGROUND", &exeStream, blob);
    }
    if (have_digits()) {
        Win16::RcBlob blob = toBlob(get_digits());
        rcDir.updateResource(Win16::RT_Bitmap, 200, &exeStream, blob);
    }
    if (have_infoBox()) {
        Win16::RcBlob blob = toBlob(get_infoBox());
        rcDir.updateResource(Win16::RT_Bitmap, "INFOWND", &exeStream, blob);
    }
    if (have_chipEnd()) {
        Win16::RcBlob blob = toBlob(get_chipEnd());
        rcDir.updateResource(Win16::RT_Bitmap, "CHIPEND", &exeStream, blob);
    }

    exeStream.close();
    return true;
}

bool HackSettings::writeToPatch(const QString& filename) const
{
    //TODO
    return false;
}
