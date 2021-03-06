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
#include "CommonWidgets/CCTools.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QMessageBox>

void HackSettings::setKnownDefaults()
{
    set_title("Chip's Challenge");
    set_iniFile("entpack.ini");
    set_iniEntry("Chip's Challenge");
    set_datFile("CHIPS.DAT");
    set_alwaysFirstTry(false);
    set_ccPatch(false);
    set_fullSec(false);
    set_pgChips(false);
    set_fakeLastLevel(144);
    set_realLastLevel(149);

    set_toolSound("blip2.wav");
    set_doorSound("door.wav");
    set_deathSound("bummer.wav");
    set_levelCompleteSound("ditty1.wav");
    set_socketSound("chimes.wav");
    set_wallSound("oof3.wav");
    set_thiefSound("strike.wav");
    set_soundOnSound("chimes.wav");
    set_chipSound("click3.wav");
    set_buttonSound("pop2.wav");
    set_waterSound("water2.wav");
    set_bombSound("hit3.wav");
    set_teleportSound("teleport.wav");
    set_timerTickSound("click1.wav");
    set_timesUpSound("bell.wav");
    set_midi_1("chip01.mid");
    set_midi_2("chip02.mid");
    set_midi_3("canyon.mid");

    set_progressMsg_1("(Level 50)\n\nPicking up chips is what the challenge is "
                      "all about. But on the ice, Chip gets chapped and feels "
                      "like a chump instead of a champ.");
    set_progressMsg_2("(Level 60)\n\nChip hits the ice and decides to chill out. "
                      "Then he runs into a fake wall and turns the maze into a "
                      "thrash-a-thon!");
    set_progressMsg_3("(Level 70)\n\nChip is halfway through the world's hardest "
                      "puzzle. If he succeeds, maybe the kids will stop calling "
                      "him computer breath!");
    set_progressMsg_4("(Level 80)\n\nChip used to spend his time programming "
                      "computer games and making models. But that was just "
                      "practice for this brain-buster!");
    set_progressMsg_5("(Level 90)\n\n'I can do it! I know I can!' Chip thinks as "
                      "the going gets tougher. Besides, Melinda the Mental "
                      "Marvel waits at the end!");
    set_progressMsg_6("(Level 100)\n\nBesides being an angel on earth, Melinda "
                      "is the top scorer in the Challenge--and the president of "
                      "the Bit Busters.");
    set_progressMsg_7("(Level 110)\n\nChip can't wait to join the Bit Busters! "
                      "The club's already figured out the school's password and "
                      "accessed everyone's grades!");
    set_progressMsg_8("(Level 120)\n\nIf Chip's grades aren't as good as "
                      "Melinda's, maybe she'll come over to his house and help "
                      "him study!");
    set_progressMsg_9("(Level 130)\n\n'I've made it this far,' Chip thinks. "
                      "'Totally fair, with my mega-brain.' Then he starts the "
                      "next maze. 'Totally unfair!' he yelps.");
    set_progressMsg_10("(Level 140)\n\nGroov-u-loids! Chip makes it almost to "
                       "the end. He's stoked!");
    set_endgameMsg_1("Great Job, Chip!\nYou did it!  You finished the challenge!");
    set_endgameMsg_2("Melinda herself offers Chip membership in the exclusive "
                     "Bit Busters computer club, and gives him access to the "
                     "club's computer system.  Chip is in heaven!");

    set_firstTryMsg("Yowser! First Try!");
    set_thirdTryMsg("Go Bit Buster!");
    set_fifthTryMsg("Finished! Good Work!");
    set_finalTryMsg("At last! You did it!");
    set_estTimeRecordMsg("You have established a time record for this level!");
    set_beatTimeRecordMsg("You beat the previous time record by %d second%s!");
    set_increasedScoreMsg("You increased your score on this level by %li point%s!");
    set_endgameScoreMsg("You completed %d levels, and your total score for the "
                        "challenge is %li points.\n\n"
                        "You can still improve your score, by completing levels "
                        "that you skipped, and getting better times on each "
                        "level.  When you replay a level, if your new score is "
                        "better than your old, your score will be adjusted by "
                        "the difference.  Select Best Times from the Game menu "
                        "to see your scores for each level.");

    set_fireDeathMsg("Ooops! Don't step in the fire without fire boots!");
    set_waterDeathMsg("Ooops! Chip can't swim without flippers!");
    set_bombDeathMsg("Ooops! Don't touch the bombs!");
    set_blockDeathMsg("Ooops! Watch out for moving blocks!");
    set_creatureDeathMsg("Ooops! Look out for creatures!");
    set_timeLimitMsg("Ooops! Out of time!");
    set_newGameConfirmMsg("Starting a new game will begin you back at level 1, "
                          "reset your score to zero, and forget the passwords to "
                          "any levels you have visited.\n"
                          "Is this what you want?");
    set_skipLevelMsg("You seem to be having trouble with this level.\n"
                     "Would you like to skip to the next level?");
    set_notEnoughTimersMsg("Not enough system timers are available.");
    set_notEnoughMemoryMsg("There is not enough memory to load Chip's Challenge.");
    set_corruptDataFileMsg("Corrupt or inaccessible CHIPS.DAT file.");

    clear_vgaTileset();
    clear_egaTileset();
    clear_monoTileset();
    clear_background();
    clear_digits();
    clear_infoBox();
    clear_chipEnd();

    clear_chipsMenu();
    clear_chipsMenuAccel();
    set_ignorePasswords("&Ignore Passwords");
}

void HackSettings::clearAll()
{
    clear_title();
    clear_iniFile();
    clear_iniEntry();
    clear_datFile();
    clear_alwaysFirstTry();
    clear_ccPatch();
    clear_fullSec();
    clear_pgChips();
    clear_fakeLastLevel();
    clear_realLastLevel();

    clear_toolSound();
    clear_doorSound();
    clear_deathSound();
    clear_levelCompleteSound();
    clear_socketSound();
    clear_wallSound();
    clear_thiefSound();
    clear_soundOnSound();
    clear_chipSound();
    clear_buttonSound();
    clear_waterSound();
    clear_bombSound();
    clear_teleportSound();
    clear_timerTickSound();
    clear_timesUpSound();
    clear_midi_1();
    clear_midi_2();
    clear_midi_3();

    clear_progressMsg_1();
    clear_progressMsg_2();
    clear_progressMsg_3();
    clear_progressMsg_4();
    clear_progressMsg_5();
    clear_progressMsg_6();
    clear_progressMsg_7();
    clear_progressMsg_8();
    clear_progressMsg_9();
    clear_progressMsg_10();
    clear_endgameMsg_1();
    clear_endgameMsg_2();

    clear_firstTryMsg();
    clear_thirdTryMsg();
    clear_fifthTryMsg();
    clear_finalTryMsg();
    clear_estTimeRecordMsg();
    clear_beatTimeRecordMsg();
    clear_increasedScoreMsg();
    clear_endgameScoreMsg();

    clear_fireDeathMsg();
    clear_waterDeathMsg();
    clear_bombDeathMsg();
    clear_blockDeathMsg();
    clear_creatureDeathMsg();
    clear_timeLimitMsg();
    clear_newGameConfirmMsg();
    clear_skipLevelMsg();
    clear_notEnoughTimersMsg();
    clear_notEnoughMemoryMsg();
    clear_corruptDataFileMsg();

    clear_vgaTileset();
    clear_egaTileset();
    clear_monoTileset();
    clear_background();
    clear_digits();
    clear_infoBox();
    clear_chipEnd();

    clear_chipsMenu();
    clear_chipsMenuAccel();
    clear_ignorePasswords();
}

bool HackSettings::loadFromExe(const QString& filename)
{
    ccl::ChipsHax hax;
    ccl::FileStream exeStream;
    if (!exeStream.open(filename, ccl::FileStream::Read))
        return false;

    hax.open(&exeStream);
    ccl::CCPatchState state = hax.get_CCPatch();
    ccl::CCPatchState fs_state = hax.get_FullSec();
    ccl::CCPatchState pg_state = hax.get_PGChips();
    if (state == ccl::CCPatchOther || fs_state == ccl::CCPatchOther
            || pg_state == ccl::CCPatchOther)
        throw ccl::FormatError(ccl::RuntimeError::tr("Unrecognized EXE format"));

    // General settings
    set_title(hax.get_WindowTitle());
    set_iniFile(hax.get_IniFilename());
    set_iniEntry(hax.get_IniEntryName());
    set_datFile(hax.get_DataFilename());
    set_alwaysFirstTry(hax.get_AlwaysFirstTry());
    set_ccPatch(state == ccl::CCPatchPatched);
    set_fullSec(fs_state == ccl::CCPatchPatched);
    set_pgChips(pg_state == ccl::CCPatchPatched);
    set_fakeLastLevel(hax.get_FakeLastLevel());
    set_realLastLevel(hax.get_LastLevel());

    // Sounds and MIDI
    set_toolSound(hax.get_ToolSound());
    set_doorSound(hax.get_DoorSound());
    set_deathSound(hax.get_DeathSound());
    set_levelCompleteSound(hax.get_LevelCompleteSound());
    set_socketSound(hax.get_SocketSound());
    set_wallSound(hax.get_WallSound());
    set_thiefSound(hax.get_ThiefSound());
    set_soundOnSound(hax.get_SoundOnSound());
    set_chipSound(hax.get_ChipSound());
    set_buttonSound(hax.get_ButtonSound());
    set_waterSound(hax.get_WaterSound());
    set_bombSound(hax.get_BombSound());
    set_teleportSound(hax.get_TeleportSound());
    set_timerTickSound(hax.get_TimerTickSound());
    set_timesUpSound(hax.get_TimesUpSound());
    set_midi_1(hax.get_Midi_1());
    set_midi_2(hax.get_Midi_2());
    set_midi_3(hax.get_Midi_3());

    set_progressMsg_1(hax.get_ProgressMsg1());
    set_progressMsg_2(hax.get_ProgressMsg2());
    set_progressMsg_3(hax.get_ProgressMsg3());
    set_progressMsg_4(hax.get_ProgressMsg4());
    set_progressMsg_5(hax.get_ProgressMsg5());
    set_progressMsg_6(hax.get_ProgressMsg6());
    set_progressMsg_7(hax.get_ProgressMsg7());
    set_progressMsg_8(hax.get_ProgressMsg8());
    set_progressMsg_9(hax.get_ProgressMsg9());
    set_progressMsg_10(hax.get_ProgressMsg10());
    set_endgameMsg_1(hax.get_EndgameMsg1());
    set_endgameMsg_2(hax.get_EndgameMsg2());

    set_firstTryMsg(hax.get_FirstTryMsg());
    set_thirdTryMsg(hax.get_ThirdTryMsg());
    set_fifthTryMsg(hax.get_FifthTryMsg());
    set_finalTryMsg(hax.get_FinalTryMsg());
    set_estTimeRecordMsg(hax.get_EstTimeRecordMsg());
    set_beatTimeRecordMsg(hax.get_BeatTimeRecordMsg());
    set_increasedScoreMsg(hax.get_IncreasedScoreMsg());
    set_endgameScoreMsg(hax.get_EndgameScoreMsg());

    set_fireDeathMsg(hax.get_FireDeathMsg());
    set_waterDeathMsg(hax.get_WaterDeathMsg());
    set_bombDeathMsg(hax.get_BombDeathMsg());
    set_blockDeathMsg(hax.get_BlockDeathMsg());
    set_creatureDeathMsg(hax.get_CreatureDeathMsg());
    set_timeLimitMsg(hax.get_TimeLimitMsg());
    set_newGameConfirmMsg(hax.get_NewGameConfirm());
    set_skipLevelMsg(hax.get_SkipLevelMsg());
    set_notEnoughTimersMsg(hax.get_NotEnoughTimers());
    set_notEnoughMemoryMsg(hax.get_NotEnoughMemory());
    set_corruptDataFileMsg(hax.get_CorruptFileMsg());

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

    // Menus
    blob = rcDir.loadResource(Win16::RT_Menu, "CHIPSMENU", &exeStream);
    if (!blob.isNull())
        set_chipsMenu(QByteArray((const char*)blob.m_data, blob.m_size));
    blob = rcDir.loadResource(Win16::RT_Accelerator, "CHIPSMENU", &exeStream);
    if (!blob.isNull())
        set_chipsMenuAccel(QByteArray((const char*)blob.m_data, blob.m_size));
    set_ignorePasswords(hax.get_IgnorePwdMenu());

    exeStream.close();
    return true;
}

// These match the keys used in CCHack 1.2a whenever possible, for compatibility
// NOTE: An empty group is implied to be in the "General" category
static const QString ccp_Title = QStringLiteral("PrgTitle");
static const QString ccp_IniFile = QStringLiteral("EntPack");
static const QString ccp_IniEntry = QStringLiteral("EntPackEntry");
static const QString ccp_DatFile = QStringLiteral("ChipsDAT");
static const QString ccp_AlwaysFirstTry = QStringLiteral("Code Patches/AlwaysFirstTry");
static const QString ccp_CCPatch = QStringLiteral("Code Patches/CCPatch");
static const QString ccp_FullSec = QStringLiteral("Code Patches/FullSec");
static const QString ccp_PGChips = QStringLiteral("Code Patches/PGChips");
static const QString ccp_FakeLastLevel = QStringLiteral("End Game/FLevNum");
static const QString ccp_RealLastLevel = QStringLiteral("End Game/LevNum");
static const QString ccp_ToolSound = QStringLiteral("Default Sounds/Tool");
static const QString ccp_DoorSound = QStringLiteral("Default Sounds/Door");
static const QString ccp_DeathSound = QStringLiteral("Default Sounds/Death");
static const QString ccp_LevelCompleteSound = QStringLiteral("Default Sounds/LevComplete");
static const QString ccp_SocketSound = QStringLiteral("Default Sounds/Socket");
static const QString ccp_WallSound = QStringLiteral("Default Sounds/HitWall");
static const QString ccp_ThiefSound = QStringLiteral("Default Sounds/Thief");
static const QString ccp_SoundOnSound = QStringLiteral("Default Sounds/Sound");
static const QString ccp_ChipSound = QStringLiteral("Default Sounds/Chip");
static const QString ccp_ButtonSound = QStringLiteral("Default Sounds/Button");
static const QString ccp_WaterSound = QStringLiteral("Default Sounds/Water");
static const QString ccp_BombSound = QStringLiteral("Default Sounds/Bomb");
static const QString ccp_TeleportSound = QStringLiteral("Default Sounds/Teleporter");
static const QString ccp_TimerTickSound = QStringLiteral("Default Sounds/Timer");
static const QString ccp_TimesUpSound = QStringLiteral("Default Sounds/Bell");
static const QString ccp_Midi_1 = QStringLiteral("Default Sounds/Midi1");
static const QString ccp_Midi_2 = QStringLiteral("Default Sounds/Midi2");
static const QString ccp_Midi_3 = QStringLiteral("Default Sounds/Midi3");
static const QString ccp_PartText_1 = QStringLiteral("Part Texts/Part1");
static const QString ccp_PartText_2 = QStringLiteral("Part Texts/Part2");
static const QString ccp_PartText_3 = QStringLiteral("Part Texts/Part3");
static const QString ccp_PartText_4 = QStringLiteral("Part Texts/Part4");
static const QString ccp_PartText_5 = QStringLiteral("Part Texts/Part5");
static const QString ccp_PartText_6 = QStringLiteral("Part Texts/Part6");
static const QString ccp_PartText_7 = QStringLiteral("Part Texts/Part7");
static const QString ccp_PartText_8 = QStringLiteral("Part Texts/Part8");
static const QString ccp_PartText_9 = QStringLiteral("Part Texts/Part9");
static const QString ccp_PartText_10 = QStringLiteral("Part Texts/Part10");
static const QString ccp_EndGameMsg_1 = QStringLiteral("End Game/Msg1");
static const QString ccp_EndGameMsg_2 = QStringLiteral("End Game/Msg2");
static const QString ccp_FirstTryMsg = QStringLiteral("End Level/FirstTry");
static const QString ccp_ThirdTryMsg = QStringLiteral("End Level/GoBitBuster");
static const QString ccp_FifthTryMsg = QStringLiteral("End Level/Finished");
static const QString ccp_FinalTryMsg = QStringLiteral("End Level/AtLast");
static const QString ccp_EstTimeRecordMsg = QStringLiteral("End Level/TimeRecord");
static const QString ccp_BeatTimeRecordMsg = QStringLiteral("End Level/BeatTR");
static const QString ccp_IncreasedScoreMsg = QStringLiteral("End Level/IncScore");
static const QString ccp_EndgameScoreMsg = QStringLiteral("End Game/MsgScore");
static const QString ccp_FireDeathMsg = QStringLiteral("Miscellaneous/Death1");
static const QString ccp_WaterDeathMsg = QStringLiteral("Miscellaneous/Death2");
static const QString ccp_BombDeathMsg = QStringLiteral("Miscellaneous/Death3");
static const QString ccp_BlockDeathMsg = QStringLiteral("Miscellaneous/Death4");
static const QString ccp_CreatureDeathMsg = QStringLiteral("Miscellaneous/Death5");
static const QString ccp_TimeLimitMsg = QStringLiteral("Miscellaneous/Death6");
static const QString ccp_NewGameConfirmMsg = QStringLiteral("Miscellaneous/NewGame");
static const QString ccp_SkipLevelMsg = QStringLiteral("Miscellaneous/SkipLev");
static const QString ccp_NotEnoughTimersMsg = QStringLiteral("Miscellaneous/NoTimers");
static const QString ccp_NotEnoughMemoryMsg = QStringLiteral("Miscellaneous/NoMemory");
static const QString ccp_CorruptDataFileMsg = QStringLiteral("Miscellaneous/CorruptDAT");
static const QString ccp_VgaTileset = QStringLiteral("Graphics/OBJ32_4");
static const QString ccp_EgaTileset = QStringLiteral("Graphics/OBJ32_4E");
static const QString ccp_MonoTileset = QStringLiteral("Graphics/OBJ32_1");
static const QString ccp_Background = QStringLiteral("Graphics/BACKGROUND");
static const QString ccp_Digits = QStringLiteral("Graphics/RC200");
static const QString ccp_InfoBox = QStringLiteral("Graphics/INFOWND");
static const QString ccp_ChipEnd = QStringLiteral("Graphics/CHIPEND");
static const QString ccp_ChipsMenu = QStringLiteral("Menus/CHIPSMENU");
static const QString ccp_ChipsMenuAccel = QStringLiteral("Menus/CHIPSMENU_Accel");
static const QString ccp_CheatMenu = QStringLiteral("Menus/Cheat");

static bool validString(const QSettings& settings, const QString& key)
{
    if (!settings.contains(key))
        return false;
    return !settings.value(key).toString().isEmpty()
        || !settings.value(key).toStringList().isEmpty();
}

static std::string loadString(const QSettings& settings, const QString& key)
{
    QString strValue = settings.value(key).toString();
    if (strValue.isEmpty())
        strValue = settings.value(key).toStringList().join(QStringLiteral(", "));
    return ccl::toLatin1(strValue);
}

static bool validInt(const QSettings& settings, const QString& key)
{
    if (!settings.contains(key))
        return false;
    bool ok;
    (void)settings.value(key).toInt(&ok);
    return ok;
}

static bool validByteArray(const QSettings& settings, const QString& key)
{
    if (!settings.contains(key))
        return false;
    return settings.value(key).type() == QVariant::ByteArray;
}

static bool validFilename(const QSettings& settings, const QString& key, const QDir& baseDir)
{
    if (!settings.contains(key))
        return false;
    const QString filename = settings.value(key).toString();
    if (filename.isEmpty())
        return false;
    return baseDir.exists(filename);
}

static QByteArray loadRCBitmap(const QString& filename, const QDir& baseDir)
{
    const QString filePath = baseDir.filePath(filename);
    QFile bmp(filePath);
    if (!bmp.open(QIODevice::ReadOnly))
        throw ccl::IOError(ccl::RuntimeError::tr("Could not open bitmap file for reading"));

    // Skip over the BITMAPFILEHEADER, since that is not stored in the EXE resource
    bmp.seek(BITMAPFILEHEADER_SIZE);
    return bmp.readAll();
}

bool HackSettings::loadFromPatch(const QString& filename)
{
    if (!QFile::exists(filename))
        return false;

    QSettings patch(filename, QSettings::IniFormat);

    // General Settings
    if (validString(patch, ccp_Title))
        set_title(loadString(patch, ccp_Title));
    if (validString(patch, ccp_IniFile))
        set_iniFile(loadString(patch, ccp_IniFile));
    if (validString(patch, ccp_IniEntry))
        set_iniEntry(loadString(patch, ccp_IniEntry));
    if (validString(patch, ccp_DatFile))
        set_datFile(loadString(patch, ccp_DatFile));
    if (patch.contains(ccp_AlwaysFirstTry))
        set_alwaysFirstTry(patch.value(ccp_AlwaysFirstTry).toBool());
    if (patch.contains(ccp_CCPatch))
        set_ccPatch(patch.value(ccp_CCPatch).toBool());
    if (patch.contains(ccp_FullSec))
        set_fullSec(patch.value(ccp_FullSec).toBool());
    if (patch.contains(ccp_PGChips))
        set_pgChips(patch.value(ccp_PGChips).toBool());
    if (validInt(patch, ccp_FakeLastLevel))
        set_fakeLastLevel(patch.value(ccp_FakeLastLevel).toInt());
    if (validInt(patch, ccp_RealLastLevel))
        set_realLastLevel(patch.value(ccp_RealLastLevel).toInt());

    // Sounds and MIDI
    if (validString(patch, ccp_ToolSound))
        set_toolSound(loadString(patch, ccp_ToolSound));
    if (validString(patch, ccp_DoorSound))
        set_doorSound(loadString(patch, ccp_DoorSound));
    if (validString(patch, ccp_DeathSound))
        set_deathSound(loadString(patch, ccp_DeathSound));
    if (validString(patch, ccp_LevelCompleteSound))
        set_levelCompleteSound(loadString(patch, ccp_LevelCompleteSound));
    if (validString(patch, ccp_SocketSound))
        set_socketSound(loadString(patch, ccp_SocketSound));
    if (validString(patch, ccp_WallSound))
        set_wallSound(loadString(patch, ccp_WallSound));
    if (validString(patch, ccp_ThiefSound))
        set_thiefSound(loadString(patch, ccp_ThiefSound));
    if (validString(patch, ccp_SoundOnSound))
        set_soundOnSound(loadString(patch, ccp_SoundOnSound));
    if (validString(patch, ccp_ChipSound))
        set_chipSound(loadString(patch, ccp_ChipSound));
    if (validString(patch, ccp_ButtonSound))
        set_buttonSound(loadString(patch, ccp_ButtonSound));
    if (validString(patch, ccp_WaterSound))
        set_waterSound(loadString(patch, ccp_WaterSound));
    if (validString(patch, ccp_BombSound))
        set_bombSound(loadString(patch, ccp_BombSound));
    if (validString(patch, ccp_TeleportSound))
        set_teleportSound(loadString(patch, ccp_TeleportSound));
    if (validString(patch, ccp_TimerTickSound))
        set_timerTickSound(loadString(patch, ccp_TimerTickSound));
    if (validString(patch, ccp_TimesUpSound))
        set_timesUpSound(loadString(patch, ccp_TimesUpSound));
    if (validString(patch, ccp_Midi_1))
        set_midi_1(loadString(patch, ccp_Midi_1));
    if (validString(patch, ccp_Midi_2))
        set_midi_2(loadString(patch, ccp_Midi_2));
    if (validString(patch, ccp_Midi_3))
        set_midi_3(loadString(patch, ccp_Midi_3));

    // Storyline Texts
    if (validString(patch, ccp_PartText_1))
        set_progressMsg_1(loadString(patch, ccp_PartText_1));
    if (validString(patch, ccp_PartText_2))
        set_progressMsg_2(loadString(patch, ccp_PartText_2));
    if (validString(patch, ccp_PartText_3))
        set_progressMsg_3(loadString(patch, ccp_PartText_3));
    if (validString(patch, ccp_PartText_4))
        set_progressMsg_4(loadString(patch, ccp_PartText_4));
    if (validString(patch, ccp_PartText_5))
        set_progressMsg_5(loadString(patch, ccp_PartText_5));
    if (validString(patch, ccp_PartText_6))
        set_progressMsg_6(loadString(patch, ccp_PartText_6));
    if (validString(patch, ccp_PartText_7))
        set_progressMsg_7(loadString(patch, ccp_PartText_7));
    if (validString(patch, ccp_PartText_8))
        set_progressMsg_8(loadString(patch, ccp_PartText_8));
    if (validString(patch, ccp_PartText_9))
        set_progressMsg_9(loadString(patch, ccp_PartText_9));
    if (validString(patch, ccp_PartText_10))
        set_progressMsg_10(loadString(patch, ccp_PartText_10));
    if (validString(patch, ccp_EndGameMsg_1))
        set_endgameMsg_1(loadString(patch, ccp_EndGameMsg_1));
    if (validString(patch, ccp_EndGameMsg_2))
        set_endgameMsg_2(loadString(patch, ccp_EndGameMsg_2));

    // Scores and Records
    if (validString(patch, ccp_FirstTryMsg))
        set_firstTryMsg(loadString(patch, ccp_FirstTryMsg));
    if (validString(patch, ccp_ThirdTryMsg))
        set_thirdTryMsg(loadString(patch, ccp_ThirdTryMsg));
    if (validString(patch, ccp_FifthTryMsg))
        set_fifthTryMsg(loadString(patch, ccp_FifthTryMsg));
    if (validString(patch, ccp_FinalTryMsg))
        set_finalTryMsg(loadString(patch, ccp_FinalTryMsg));
    if (validString(patch, ccp_EstTimeRecordMsg))
        set_estTimeRecordMsg(loadString(patch, ccp_EstTimeRecordMsg));
    if (validString(patch, ccp_BeatTimeRecordMsg))
        set_beatTimeRecordMsg(loadString(patch, ccp_BeatTimeRecordMsg));
    if (validString(patch, ccp_IncreasedScoreMsg))
        set_increasedScoreMsg(loadString(patch, ccp_IncreasedScoreMsg));
    if (validString(patch, ccp_EndgameScoreMsg))
        set_endgameScoreMsg(loadString(patch, ccp_EndgameScoreMsg));

    // Misc strings
    if (validString(patch, ccp_FireDeathMsg))
        set_fireDeathMsg(loadString(patch, ccp_FireDeathMsg));
    if (validString(patch, ccp_WaterDeathMsg))
        set_waterDeathMsg(loadString(patch, ccp_WaterDeathMsg));
    if (validString(patch, ccp_BombDeathMsg))
        set_bombDeathMsg(loadString(patch, ccp_BombDeathMsg));
    if (validString(patch, ccp_BlockDeathMsg))
        set_blockDeathMsg(loadString(patch, ccp_BlockDeathMsg));
    if (validString(patch, ccp_CreatureDeathMsg))
        set_creatureDeathMsg(loadString(patch, ccp_CreatureDeathMsg));
    if (validString(patch, ccp_TimeLimitMsg))
        set_timeLimitMsg(loadString(patch, ccp_TimeLimitMsg));
    if (validString(patch, ccp_NewGameConfirmMsg))
        set_newGameConfirmMsg(loadString(patch, ccp_NewGameConfirmMsg));
    if (validString(patch, ccp_SkipLevelMsg))
        set_skipLevelMsg(loadString(patch, ccp_SkipLevelMsg));
    if (validString(patch, ccp_NotEnoughTimersMsg))
        set_notEnoughTimersMsg(loadString(patch, ccp_NotEnoughTimersMsg));
    if (validString(patch, ccp_NotEnoughMemoryMsg))
        set_notEnoughMemoryMsg(loadString(patch, ccp_NotEnoughMemoryMsg));
    if (validString(patch, ccp_CorruptDataFileMsg))
        set_corruptDataFileMsg(loadString(patch, ccp_CorruptDataFileMsg));

    // Graphics
    const QDir baseDir = QFileInfo(filename).dir();
    if (validByteArray(patch, ccp_VgaTileset))
        set_vgaTileset(patch.value(ccp_VgaTileset).toByteArray());
    else if (validFilename(patch, ccp_VgaTileset, baseDir))
        set_vgaTileset(loadRCBitmap(patch.value(ccp_VgaTileset).toString(), baseDir));
    if (validByteArray(patch, ccp_EgaTileset))
        set_egaTileset(patch.value(ccp_EgaTileset).toByteArray());
    else if (validFilename(patch, ccp_EgaTileset, baseDir))
        set_egaTileset(loadRCBitmap(patch.value(ccp_EgaTileset).toString(), baseDir));
    if (validByteArray(patch, ccp_MonoTileset))
        set_monoTileset(patch.value(ccp_MonoTileset).toByteArray());
    else if (validFilename(patch, ccp_MonoTileset, baseDir))
        set_monoTileset(loadRCBitmap(patch.value(ccp_MonoTileset).toString(), baseDir));
    if (validByteArray(patch, ccp_Background))
        set_background(patch.value(ccp_Background).toByteArray());
    else if (validFilename(patch, ccp_Background, baseDir))
        set_background(loadRCBitmap(patch.value(ccp_Background).toString(), baseDir));
    if (validByteArray(patch, ccp_Digits))
        set_digits(patch.value(ccp_Digits).toByteArray());
    else if (validFilename(patch, ccp_Digits, baseDir))
        set_digits(loadRCBitmap(patch.value(ccp_Digits).toString(), baseDir));
    if (validByteArray(patch, ccp_InfoBox))
        set_infoBox(patch.value(ccp_InfoBox).toByteArray());
    else if (validFilename(patch, ccp_InfoBox, baseDir))
        set_infoBox(loadRCBitmap(patch.value(ccp_InfoBox).toString(), baseDir));
    if (validByteArray(patch, ccp_ChipEnd))
        set_chipEnd(patch.value(ccp_ChipEnd).toByteArray());
    else if (validFilename(patch, ccp_ChipEnd, baseDir))
        set_chipEnd(loadRCBitmap(patch.value(ccp_ChipEnd).toString(), baseDir));

    // Menus
    if (validByteArray(patch, ccp_ChipsMenu))
        set_chipsMenu(patch.value(ccp_ChipsMenu).toByteArray());
    if (validByteArray(patch, ccp_ChipsMenuAccel))
        set_chipsMenuAccel(patch.value(ccp_ChipsMenuAccel).toByteArray());
    if (validString(patch, ccp_CheatMenu))
        set_ignorePasswords(loadString(patch, ccp_CheatMenu));

    return true;
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
    if (!exeStream.open(filename, ccl::FileStream::ReadWrite))
        return false;

    hax.open(&exeStream);
    if (hax.get_CCPatch() == ccl::CCPatchOther || hax.get_FullSec() == ccl::CCPatchOther
            || hax.get_PGChips() == ccl::CCPatchOther)
        throw ccl::FormatError(ccl::RuntimeError::tr("Unrecognized EXE format"));

    if (have_pgChips()) {
        // We do this first (before writing graphics) so the graphics patch
        // has a chance of applying cleanly.
        ccl::CCPatchState pg_state = hax.validate_PGChips();
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

    if (have_fullSec())
        hax.set_FullSec(get_fullSec() ? ccl::CCPatchPatched : ccl::CCPatchOriginal);

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

    // Sounds and MIDI
    if (have_toolSound())
        hax.set_ToolSound(get_toolSound());
    if (have_doorSound())
        hax.set_DoorSound(get_doorSound());
    if (have_deathSound())
        hax.set_DeathSound(get_deathSound());
    if (have_levelCompleteSound())
        hax.set_LevelCompleteSound(get_levelCompleteSound());
    if (have_socketSound())
        hax.set_SocketSound(get_socketSound());
    if (have_wallSound())
        hax.set_WallSound(get_wallSound());
    if (have_thiefSound())
        hax.set_ThiefSound(get_thiefSound());
    if (have_soundOnSound())
        hax.set_SoundOnSound(get_soundOnSound());
    if (have_chipSound())
        hax.set_ChipSound(get_chipSound());
    if (have_buttonSound())
        hax.set_ButtonSound(get_buttonSound());
    if (have_waterSound())
        hax.set_WaterSound(get_waterSound());
    if (have_bombSound())
        hax.set_BombSound(get_bombSound());
    if (have_teleportSound())
        hax.set_TeleportSound(get_teleportSound());
    if (have_timerTickSound())
        hax.set_TimerTickSound(get_timerTickSound());
    if (have_timesUpSound())
        hax.set_TimesUpSound(get_timesUpSound());
    if (have_midi_1())
        hax.set_Midi_1(get_midi_1());
    if (have_midi_2())
        hax.set_Midi_2(get_midi_2());
    if (have_midi_3())
        hax.set_Midi_3(get_midi_3());

    // Storyline Texts
    if (have_progressMsg_1())
        hax.set_ProgressMsg1(get_progressMsg_1());
    if (have_progressMsg_2())
        hax.set_ProgressMsg2(get_progressMsg_2());
    if (have_progressMsg_3())
        hax.set_ProgressMsg3(get_progressMsg_3());
    if (have_progressMsg_4())
        hax.set_ProgressMsg4(get_progressMsg_4());
    if (have_progressMsg_5())
        hax.set_ProgressMsg5(get_progressMsg_5());
    if (have_progressMsg_6())
        hax.set_ProgressMsg6(get_progressMsg_6());
    if (have_progressMsg_7())
        hax.set_ProgressMsg7(get_progressMsg_7());
    if (have_progressMsg_8())
        hax.set_ProgressMsg8(get_progressMsg_8());
    if (have_progressMsg_9())
        hax.set_ProgressMsg9(get_progressMsg_9());
    if (have_progressMsg_10())
        hax.set_ProgressMsg10(get_progressMsg_10());
    if (have_endgameMsg_1())
        hax.set_EndgameMsg1(get_endgameMsg_1());
    if (have_endgameMsg_2())
        hax.set_EndgameMsg2(get_endgameMsg_2());

    // Scores and Records
    if (have_firstTryMsg())
        hax.set_FirstTryMsg(get_firstTryMsg());
    if (have_thirdTryMsg())
        hax.set_ThirdTryMsg(get_thirdTryMsg());
    if (have_fifthTryMsg())
        hax.set_FifthTryMsg(get_fifthTryMsg());
    if (have_finalTryMsg())
        hax.set_FinalTryMsg(get_finalTryMsg());
    if (have_estTimeRecordMsg())
        hax.set_EstTimeRecordMsg(get_estTimeRecordMsg());
    if (have_beatTimeRecordMsg())
        hax.set_BeatTimeRecordMsg(get_beatTimeRecordMsg());
    if (have_increasedScoreMsg())
        hax.set_IncreasedScoreMsg(get_increasedScoreMsg());
    if (have_endgameScoreMsg())
        hax.set_EndgameScoreMsg(get_endgameScoreMsg());

    // Misc strings
    if (have_fireDeathMsg())
        hax.set_FireDeathMsg(get_fireDeathMsg());
    if (have_waterDeathMsg())
        hax.set_WaterDeathMsg(get_waterDeathMsg());
    if (have_bombDeathMsg())
        hax.set_BombDeathMsg(get_bombDeathMsg());
    if (have_blockDeathMsg())
        hax.set_BlockDeathMsg(get_blockDeathMsg());
    if (have_creatureDeathMsg())
        hax.set_CreatureDeathMsg(get_creatureDeathMsg());
    if (have_timeLimitMsg())
        hax.set_TimeLimitMsg(get_timeLimitMsg());
    if (have_newGameConfirmMsg())
        hax.set_NewGameConfirm(get_newGameConfirmMsg());
    if (have_skipLevelMsg())
        hax.set_SkipLevelMsg(get_skipLevelMsg());
    if (have_notEnoughTimersMsg())
        hax.set_NotEnoughTimers(get_notEnoughTimersMsg());
    if (have_notEnoughMemoryMsg())
        hax.set_NotEnoughMemory(get_notEnoughMemoryMsg());
    if (have_corruptDataFileMsg())
        hax.set_CorruptFileMsg(get_corruptDataFileMsg());

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

    // Menus
    if (have_chipsMenu()) {
        Win16::RcBlob blob = toBlob(get_chipsMenu());
        rcDir.updateResource(Win16::RT_Menu, "CHIPSMENU", &exeStream, blob);
    }
    if (have_chipsMenuAccel()) {
        Win16::RcBlob blob = toBlob(get_chipsMenuAccel());
        rcDir.updateResource(Win16::RT_Accelerator, "CHIPSMENU", &exeStream, blob);
    }
    if (have_ignorePasswords())
        hax.set_IgnorePwdMenu(get_ignorePasswords());

    exeStream.close();
    return true;
}

bool HackSettings::writeToPatch(const QString& filename) const
{
    QSettings patch(filename, QSettings::IniFormat);
    if (!patch.isWritable())
        return false;

    // Don't leave previous settings in the file
    patch.clear();

    // General Settings
    if (have_title())
        patch.setValue(ccp_Title, ccl::fromLatin1(get_title()));
    if (have_iniFile())
        patch.setValue(ccp_IniFile, ccl::fromLatin1(get_iniFile()));
    if (have_iniEntry())
        patch.setValue(ccp_IniEntry, ccl::fromLatin1(get_iniEntry()));
    if (have_datFile())
        patch.setValue(ccp_DatFile, ccl::fromLatin1(get_datFile()));
    if (have_alwaysFirstTry())
        patch.setValue(ccp_AlwaysFirstTry, get_alwaysFirstTry());
    if (have_ccPatch())
        patch.setValue(ccp_CCPatch, get_ccPatch());
    if (have_fullSec())
        patch.setValue(ccp_FullSec, get_fullSec());
    if (have_pgChips())
        patch.setValue(ccp_PGChips, get_pgChips());
    if (have_fakeLastLevel())
        patch.setValue(ccp_FakeLastLevel, get_fakeLastLevel());
    if (have_realLastLevel())
        patch.setValue(ccp_RealLastLevel, get_realLastLevel());

    // Sounds and MIDI
    if (have_toolSound())
        patch.setValue(ccp_ToolSound, ccl::fromLatin1(get_toolSound()));
    if (have_doorSound())
        patch.setValue(ccp_DoorSound, ccl::fromLatin1(get_doorSound()));
    if (have_deathSound())
        patch.setValue(ccp_DeathSound, ccl::fromLatin1(get_deathSound()));
    if (have_levelCompleteSound())
        patch.setValue(ccp_LevelCompleteSound, ccl::fromLatin1(get_levelCompleteSound()));
    if (have_socketSound())
        patch.setValue(ccp_SocketSound, ccl::fromLatin1(get_socketSound()));
    if (have_wallSound())
        patch.setValue(ccp_WallSound, ccl::fromLatin1(get_wallSound()));
    if (have_thiefSound())
        patch.setValue(ccp_ThiefSound, ccl::fromLatin1(get_thiefSound()));
    if (have_soundOnSound())
        patch.setValue(ccp_SoundOnSound, ccl::fromLatin1(get_soundOnSound()));
    if (have_chipSound())
        patch.setValue(ccp_ChipSound, ccl::fromLatin1(get_chipSound()));
    if (have_buttonSound())
        patch.setValue(ccp_ButtonSound, ccl::fromLatin1(get_buttonSound()));
    if (have_waterSound())
        patch.setValue(ccp_WaterSound, ccl::fromLatin1(get_waterSound()));
    if (have_bombSound())
        patch.setValue(ccp_BombSound, ccl::fromLatin1(get_bombSound()));
    if (have_teleportSound())
        patch.setValue(ccp_TeleportSound, ccl::fromLatin1(get_teleportSound()));
    if (have_timerTickSound())
        patch.setValue(ccp_TimerTickSound, ccl::fromLatin1(get_timerTickSound()));
    if (have_timesUpSound())
        patch.setValue(ccp_TimesUpSound, ccl::fromLatin1(get_timesUpSound()));
    if (have_midi_1())
        patch.setValue(ccp_Midi_1, ccl::fromLatin1(get_midi_1()));
    if (have_midi_2())
        patch.setValue(ccp_Midi_2, ccl::fromLatin1(get_midi_2()));
    if (have_midi_3())
        patch.setValue(ccp_Midi_3, ccl::fromLatin1(get_midi_3()));

    // Storyline Texts
    if (have_progressMsg_1())
        patch.setValue(ccp_PartText_1, ccl::fromLatin1(get_progressMsg_1()));
    if (have_progressMsg_2())
        patch.setValue(ccp_PartText_2, ccl::fromLatin1(get_progressMsg_2()));
    if (have_progressMsg_3())
        patch.setValue(ccp_PartText_3, ccl::fromLatin1(get_progressMsg_3()));
    if (have_progressMsg_4())
        patch.setValue(ccp_PartText_4, ccl::fromLatin1(get_progressMsg_4()));
    if (have_progressMsg_5())
        patch.setValue(ccp_PartText_5, ccl::fromLatin1(get_progressMsg_5()));
    if (have_progressMsg_6())
        patch.setValue(ccp_PartText_6, ccl::fromLatin1(get_progressMsg_6()));
    if (have_progressMsg_7())
        patch.setValue(ccp_PartText_7, ccl::fromLatin1(get_progressMsg_7()));
    if (have_progressMsg_8())
        patch.setValue(ccp_PartText_8, ccl::fromLatin1(get_progressMsg_8()));
    if (have_progressMsg_9())
        patch.setValue(ccp_PartText_9, ccl::fromLatin1(get_progressMsg_9()));
    if (have_progressMsg_10())
        patch.setValue(ccp_PartText_10, ccl::fromLatin1(get_progressMsg_10()));
    if (have_endgameMsg_1())
        patch.setValue(ccp_EndGameMsg_1, ccl::fromLatin1(get_endgameMsg_1()));
    if (have_endgameMsg_2())
        patch.setValue(ccp_EndGameMsg_2, ccl::fromLatin1(get_endgameMsg_2()));

    // Scores and Records
    if (have_firstTryMsg())
        patch.setValue(ccp_FirstTryMsg, ccl::fromLatin1(get_firstTryMsg()));
    if (have_thirdTryMsg())
        patch.setValue(ccp_ThirdTryMsg, ccl::fromLatin1(get_thirdTryMsg()));
    if (have_fifthTryMsg())
        patch.setValue(ccp_FifthTryMsg, ccl::fromLatin1(get_fifthTryMsg()));
    if (have_finalTryMsg())
        patch.setValue(ccp_FinalTryMsg, ccl::fromLatin1(get_finalTryMsg()));
    if (have_estTimeRecordMsg())
        patch.setValue(ccp_EstTimeRecordMsg, ccl::fromLatin1(get_estTimeRecordMsg()));
    if (have_beatTimeRecordMsg())
        patch.setValue(ccp_BeatTimeRecordMsg, ccl::fromLatin1(get_beatTimeRecordMsg()));
    if (have_increasedScoreMsg())
        patch.setValue(ccp_IncreasedScoreMsg, ccl::fromLatin1(get_increasedScoreMsg()));
    if (have_endgameScoreMsg())
        patch.setValue(ccp_EndgameScoreMsg, ccl::fromLatin1(get_endgameScoreMsg()));

    // Misc strings
    if (have_fireDeathMsg())
        patch.setValue(ccp_FireDeathMsg, ccl::fromLatin1(get_fireDeathMsg()));
    if (have_waterDeathMsg())
        patch.setValue(ccp_WaterDeathMsg, ccl::fromLatin1(get_waterDeathMsg()));
    if (have_bombDeathMsg())
        patch.setValue(ccp_BombDeathMsg, ccl::fromLatin1(get_bombDeathMsg()));
    if (have_blockDeathMsg())
        patch.setValue(ccp_BlockDeathMsg, ccl::fromLatin1(get_blockDeathMsg()));
    if (have_creatureDeathMsg())
        patch.setValue(ccp_CreatureDeathMsg, ccl::fromLatin1(get_creatureDeathMsg()));
    if (have_timeLimitMsg())
        patch.setValue(ccp_TimeLimitMsg, ccl::fromLatin1(get_timeLimitMsg()));
    if (have_newGameConfirmMsg())
        patch.setValue(ccp_NewGameConfirmMsg, ccl::fromLatin1(get_newGameConfirmMsg()));
    if (have_skipLevelMsg())
        patch.setValue(ccp_SkipLevelMsg, ccl::fromLatin1(get_skipLevelMsg()));
    if (have_notEnoughTimersMsg())
        patch.setValue(ccp_NotEnoughTimersMsg, ccl::fromLatin1(get_notEnoughTimersMsg()));
    if (have_notEnoughMemoryMsg())
        patch.setValue(ccp_NotEnoughMemoryMsg, ccl::fromLatin1(get_notEnoughMemoryMsg()));
    if (have_corruptDataFileMsg())
        patch.setValue(ccp_CorruptDataFileMsg, ccl::fromLatin1(get_corruptDataFileMsg()));

    // Graphics -- we store them directly in the .ccp file for CCHack 3.0
    if (have_vgaTileset())
        patch.setValue(ccp_VgaTileset, get_vgaTileset());
    if (have_egaTileset())
        patch.setValue(ccp_EgaTileset, get_egaTileset());
    if (have_monoTileset())
        patch.setValue(ccp_MonoTileset, get_monoTileset());
    if (have_background())
        patch.setValue(ccp_Background, get_background());
    if (have_digits())
        patch.setValue(ccp_Digits, get_digits());
    if (have_infoBox())
        patch.setValue(ccp_InfoBox, get_infoBox());
    if (have_chipEnd())
        patch.setValue(ccp_ChipEnd, get_chipEnd());

    // Menus (MENU resource stored as a "compiled" blob)
    if (have_chipsMenu())
        patch.setValue(ccp_ChipsMenu, get_chipsMenu());
    if (have_chipsMenuAccel())
        patch.setValue(ccp_ChipsMenuAccel, get_chipsMenuAccel());
    if (have_ignorePasswords())
        patch.setValue(ccp_CheatMenu, ccl::fromLatin1(get_ignorePasswords()));

    return true;
}
