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

#ifndef _CC2MAP_H
#define _CC2MAP_H

#include "libcc1/Stream.h"

#include <vector>
#include <tuple>
#include <stdexcept>

namespace ccl { class LevelData; }

namespace cc2 {

class MapOption {
public:
    enum Viewport {
        View10x10,
        View9x9,
        ViewSplit,
    };

    enum BlobPattern {
        BlobsDeterministic,
        Blobs4Pattern,
        BlobsExtraRandom,
    };

    MapOption()
        : m_view(View10x10), m_blobPattern(BlobsDeterministic), m_timeLimit(),
          m_replayMD5(), m_replayValid(), m_hidden(), m_readOnly(),
          m_hideLogic(), m_cc1Boots() { }

    MapOption(const MapOption&) = default;
    MapOption& operator=(const MapOption&) = default;

    Viewport view() const { return m_view; }
    BlobPattern blobPattern() const { return m_blobPattern; }
    uint16_t timeLimit() const { return m_timeLimit; }
    const uint8_t* replayMD5() const { return m_replayMD5; }
    bool replayValid() const { return m_replayValid; }
    bool hidden() const { return m_hidden; }
    bool readOnly() const { return m_readOnly; }
    bool hideLogic() const { return m_hideLogic; }
    bool cc1Boots() const { return m_cc1Boots; }

    void setView(Viewport view) { m_view = view; }
    void setBlobPattern(BlobPattern pattern) { m_blobPattern = pattern; }
    void setTimeLimit(uint16_t limit) { m_timeLimit = limit; }
    void setReplayMD5(const uint8_t* md5);
    void setReplayValid(bool valid) { m_replayValid = valid; }
    void setHidden(bool hidden) { m_hidden = hidden; }
    void setReadOnly(bool ro) { m_readOnly = ro; }
    void setHideLogic(bool hide) { m_hideLogic = hide; }
    void setCc1Boots(bool cc1Boots) { m_cc1Boots = cc1Boots; }

    void read(ccl::Stream* stream, size_t size);
    void write(ccl::Stream* stream) const;

private:
    Viewport m_view;
    BlobPattern m_blobPattern;
    uint16_t m_timeLimit;
    uint8_t m_replayMD5[16];
    bool m_replayValid;
    bool m_hidden;
    bool m_readOnly;
    bool m_hideLogic;
    bool m_cc1Boots;
};

/* Only used as a namespace for modifier values */
class TileModifier {
public:
    enum WireMask {
        WireNorth = 0x1,
        WireEast = 0x2,
        WireSouth = 0x4,
        WireWest = 0x8,
        WireMask = 0xf,
        WireTunnelNorth = 0x10,
        WireTunnelEast = 0x20,
        WireTunnelSouth = 0x40,
        WireTunnelWest = 0x80,
        WireTunnelMask = 0xf0,
    };

    enum CloneDirection {
        // Any combination of arrows will be shown, but only
        // one will be used for the clone direction
        CloneNorth = 0x1,
        CloneEast = 0x2,
        CloneSouth = 0x4,
        CloneWest = 0x8,
        CloneAllDirs = 0xf,
    };

    enum ColorTheme {
        // For CustomFloor / CustomWall tiles
        CamoTheme,
        PinkDotsTheme,
        YellowBrickTheme,
        BlueTheme,
    };

    enum LogicGateType {
        Inverter_N, Inverter_E, Inverter_S, Inverter_W,
        AndGate_N, AndGate_E, AndGate_S, AndGate_W,
        OrGate_N, OrGate_E, OrGate_S, OrGate_W,
        XorGate_N, XorGate_E, XorGate_S, XorGate_W,
        LatchGateCW_N, LatchGateCW_E, LatchGateCW_S, LatchGateCW_W,
        NandGate_N, NandGate_E, NandGate_S, NandGate_W,
        CounterGate_0 = 30, CounterGate_1, CounterGate_2, CounterGate_3,
        CounterGate_4, CounterGate_5, CounterGate_6, CounterGate_7,
        CounterGate_8, CounterGate_9,
        LatchGateCCW_N = 0x40, LatchGateCCW_E, LatchGateCCW_S, LatchGateCCW_W,
    };

    enum TrainTracks {
        Track_NE = 0x1,
        Track_SE = 0x2,
        Track_SW = 0x4,
        Track_NW = 0x8,
        Track_WE = 0x10,
        Track_NS = 0x20,
        TrackDir_MASK = 0x3f,
        TrackSwitch = 0x40,

        ActiveTrack_MASK = 0x700,
        ActiveTrack_NE = 0x000,
        ActiveTrack_SE = 0x100,
        ActiveTrack_SW = 0x200,
        ActiveTrack_NW = 0x300,
        ActiveTrack_WE = 0x400,
        ActiveTrack_NS = 0x500,
    };

    enum AsciiGlyphType {
        GlyphUp = 0x1c,
        GlyphRight = 0x1d,
        GlyphDown = 0x1e,
        GlyphLeft = 0x1f,
        /* Range from ' ' to '_' can be represented with the equivalent
           ASCII character, rather than duplicating them in the enum */
        GlyphASCII_MIN = ' ',
        GlyphASCII_MAX = '_',
        GlyphMIN = GlyphUp,
        GlyphMAX = GlyphASCII_MAX,
    };
};

class Tile {
public:
    enum Type {
        // These do NOT match CC1 tiles, so it needs its own enum.
        Invalid = 0,
        Floor, Wall, Ice, Ice_NE, Ice_SE, Ice_SW, Ice_NW, Water, Fire,
        Force_N, Force_E, Force_S, Force_W, ToggleWall, ToggleFloor,
        Teleport_Red, Teleport_Blue, Teleport_Yellow, Teleport_Green,
        Exit, Slime, Player, DirtBlock, Walker, Ship, IceBlock,
        CC1_Barrier_S, CC1_Barrier_E, CC1_Barrier_SE, Gravel,
        ToggleButton, TankButton, BlueTank, Door_Red, Door_Blue,
        Door_Yellow, Door_Green, Key_Red, Key_Blue, Key_Yellow, Key_Green,
        Chip, ExtraChip, Socket, PopUpWall, AppearingWall, InvisWall,
        BlueWall, BlueFloor, Dirt, Ant, Centipede, Ball, Blob,
        AngryTeeth, FireBox, CloneButton, TrapButton, IceCleats, MagnoShoes,
        FireShoes, Flippers, ToolThief, RedBomb, UNUSED_41, Trap,
        CC1_Cloner, Cloner, Clue, Force_Rand, AreaCtlButton,
        RevolvDoor_SW, RevolvDoor_NW, RevolvDoor_NE, RevolvDoor_SE,
        TimeBonus, ToggleClock, Transformer, TrainTracks, SteelWall,
        TimeBomb, Helmet, UNUSED_53, UNUSED_54, UNUSED_55, Player2,
        TimidTeeth, UNUSED_Explosion, HikingBoots, MaleOnly, FemaleOnly,
        LogicGate, UNUSED_5d, LogicButton, FlameJet_Off, FlameJet_On,
        FlameJetButton, Lightning, YellowTank, YellowTankCtrl,
        MirrorPlayer, MirrorPlayer2, UNUSED_67, BowlingBall, Rover,
        TimePenalty, StyledFloor, UNUSED_6c, PanelCanopy, UNUSED_6e, RRSign,
        StyledWall, AsciiGlyph, LSwitchFloor, LSwitchWall,
        UNUSED_74, UNUSED_75, Modifier8, Modifier16, Modifier32, UNUSED_79,
        Flag10, Flag100, Flag1000, StayUpGWall, PopDownGWall, Disallow,
        Flag2x, DirBlock, FloorMimic, GreenBomb, GreenChip,
        UNUSED_85, UNUSED_86, RevLogicButton, Switch_Off, Switch_On,
        KeyThief, Ghost, SteelFoil, Turtle, Eye, Bribe, SpeedShoes,
        UNUSED_91, Hook,
        NUM_TILE_TYPES,
    };

    enum Direction { InvalidDir = -1, North, East, South, West };

    enum ArrowMask {
        ArrowNorth = 0x1,
        ArrowEast = 0x2,
        ArrowSouth = 0x4,
        ArrowWest = 0x8,
        AllArrows = 0xf,
    };

    enum PanelFlags {
        PanelNorth = 0x1,
        PanelEast = 0x2,
        PanelSouth = 0x4,
        PanelWest = 0x8,
        Canopy = 0x10,
    };

    Tile() : m_type(Floor), m_direction(), m_tileFlags(), m_modifier(), m_lower() { }

    explicit Tile(int type, uint32_t modifier = 0)
        : m_type(type), m_direction(), m_tileFlags(), m_modifier(modifier),
          m_lower()
    {
        checkLower();
    }

    Tile(int type, Direction dir, uint32_t modifier)
        : m_type(type), m_direction(dir), m_tileFlags(), m_modifier(modifier),
          m_lower()
    {
        checkLower();
    }

    static Tile panelTile(uint8_t panelFlags)
    {
        Tile panel(PanelCanopy);
        panel.setTileFlags(panelFlags);
        return panel;
    }

    static Tile dirBlockTile(uint8_t arrowMask)
    {
        Tile panel(DirBlock);
        panel.setTileFlags(arrowMask);
        return panel;
    }

    ~Tile() { delete m_lower; }

    Tile(const Tile& copy);
    Tile& operator=(const Tile& copy);

    Tile(Tile&& move) noexcept;
    Tile& operator=(Tile&& move) noexcept;

    bool operator==(const Tile& other) const;
    bool operator!=(const Tile& other) const { return !operator==(other); }

    Type type() const { return (Type)m_type; }
    void setType(int type)
    {
        m_type = type;
        checkLower();
    }

    Direction direction() const { return (Direction)m_direction; }
    void setDirection(Direction dir) { m_direction = dir; }

    uint8_t tileFlags() const { return m_tileFlags; }
    void setTileFlags(uint8_t mask) { m_tileFlags = mask; }

    uint32_t modifier() const { return m_modifier; }
    void setModifier(uint32_t modifier) { m_modifier = modifier; }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    Tile* lower() { return haveLower() ? m_lower : nullptr; }
    const Tile* lower() const { return haveLower() ? m_lower : nullptr; }

    Tile& bottom()
    {
        Tile* tp = this;
        while (tp->haveLower())
            tp = tp->m_lower;
        return *tp;
    }

    const Tile& bottom() const
    {
        const Tile* tp = this;
        while (tp->haveLower())
            tp = tp->m_lower;
        return *tp;
    }

    static bool haveLower(int type);
    static bool haveDirection(int type);
    static bool supportsWires(int type);
    static bool isCreature(int type);
    static bool isBlock(int type);
    static bool isPanelCanopy(int type);

    bool haveLower() const { return haveLower(m_type); }
    bool haveDirection() const { return haveDirection(m_type); }
    bool supportsWires() const { return supportsWires(m_type); }
    bool isCreature() const { return isCreature(m_type); }
    bool isBlock() const { return isBlock(m_type); }
    bool isPanelCanopy() const { return isPanelCanopy(m_type); }
    bool needArrows() const;

    bool needXray() const
    {
        return (m_type > Floor) || (m_type == Floor && m_modifier != 0);
    }

    void rotateLeft();
    void rotateRight();

private:
    uint8_t m_type;
    uint8_t m_direction;
    uint8_t m_tileFlags;

    // Attached modifier value, if any
    uint32_t m_modifier;

    // Lower-layer tile, if any (All tile data may recurse!)
    Tile* m_lower;

    // This will create the lower layer if necessary
    Tile* checkLower();
};

class MapData {
public:
    MapData() : m_width(), m_height(), m_map() { }
    ~MapData() { delete[] m_map; }

    MapData(const MapData& other);
    MapData& operator=(const MapData& other);

    void read(ccl::Stream* stream, size_t size);
    void write(ccl::Stream* stream) const;

    uint8_t width() const { return m_width; }
    uint8_t height() const { return m_height; }

    void resize(uint8_t width, uint8_t height);

    int countChips() const;
    std::tuple<int, int> countPoints() const;

    Tile& tile(int x, int y)
    {
        if (!m_map || x >= m_width || y >= m_height)
            throw std::out_of_range("Map index out of bounds");
        return m_map[(y * m_width) + x];
    }

    const Tile& tile(int x, int y) const
    {
        if (!m_map || x >= m_width || y >= m_height)
            throw std::out_of_range("Map index out of bounds");
        return m_map[(y * m_width) + x];
    }

    bool haveTile(int x, int y, Tile::Type type) const;

private:
    uint8_t m_width, m_height;
    Tile* m_map;
};

struct CC2FieldStorage
{
    char tag[4];
    std::vector<uint8_t> data;
};

class Map {
public:
    Map() : m_refs(1), m_version("7"), m_key(), m_readOnly() { }
    ~Map() = default;

    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    void copyFrom(const cc2::Map* map);
    void importFrom(const ccl::LevelData* level);

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    std::string version() const { return m_version; }
    std::string lock() const { return m_lock; }
    std::string title() const { return m_title; }
    std::string author() const { return m_author; }
    std::string editorVersion() const { return m_editorVersion; }
    std::string clue() const { return m_clue; }
    std::string note() const { return m_note; }
    bool readOnly() const { return m_readOnly; }

    void setVersion(std::string version) { m_version = std::move(version); }
    void setLock(std::string lock) { m_lock = std::move(lock); }
    void setTitle(std::string title) { m_title = std::move(title); }
    void setAuthor(std::string author) { m_author = std::move(author); }
    void setEditorVersion(std::string version) { m_editorVersion = std::move(version); }
    void setClue(std::string clue) { m_clue = std::move(clue); }
    void setNote(std::string note) { m_note = std::move(note); }
    void setReadOnly(bool ro) { m_readOnly = ro; }

    MapOption& option() { return m_option; }
    const MapOption& option() const { return m_option; }

    MapData& mapData() { return m_mapData; }
    const MapData& mapData() const { return m_mapData; }

    std::vector<uint8_t>& replay() { return m_replay; }
    const std::vector<uint8_t>& replay() const { return m_replay; }

    void discardReplay()
    {
        m_replay.clear();
        m_option.setReplayValid(false);
        static const uint8_t zero_md5[16] = { 0 };
        m_option.setReplayMD5(zero_md5);
    }

    std::string clueForTile(int x, int y);
    void setClueForTile(int x, int y, const std::string& clue);

    void ref()
    {
        ++m_refs;
    }

    void unref()
    {
        if (--m_refs == 0)
            delete this;
    }

    int refs() const { return m_refs; }

private:
    int m_refs;

    std::string m_version;
    std::string m_lock;
    std::string m_title;
    std::string m_author;
    std::string m_editorVersion;
    std::string m_clue;
    std::string m_note;
    MapOption m_option;
    MapData m_mapData;
    uint8_t m_key[16];
    std::vector<uint8_t> m_replay;
    bool m_readOnly;

    std::vector<CC2FieldStorage> m_unknown;
};

class SaveData {
public:
    SaveData()
        : m_line(), m_score(), m_scoreBonus(), m_timeLeft(), m_chipsLeft(),
          m_bonus(), m_level(), m_time(), m_tries(), m_gender(), m_enter(),
          m_exit(), m_finished(), m_result(), m_reg1(), m_reg2(), m_reg3(), m_reg4(),
          m_menu(), m_flags(), m_tools(), m_keys(), m_lastTime(), m_levelScore(),
          m_checksum() { }

    void setLine(uint32_t line) { m_line = line; }
    void setLevel(uint32_t level) { m_level = level; }
    void setChecksum(uint32_t checksum) { m_checksum = checksum; }

    void read(ccl::Stream* stream, size_t size);
    void write(ccl::Stream* stream) const;

private:
    // From http://www.pvv.org/~spaans/CC2fileformat.txt
    uint32_t m_line;        // 0 where in the script it is
    uint32_t m_score;       //+ 1 total score for all levels ( this should probably be global)
    uint32_t m_scoreBonus;  //+ 2 flag bonus
    uint32_t m_timeLeft;    //- 3 sixtyth of a seconds left
    uint32_t m_chipsLeft;   //+ 4 chips left (assuming exited and counter was non zero)
    uint32_t m_bonus;       //+ 5 time bonus
    uint32_t m_level;       // 6 level number
    uint32_t m_time;        // 7 initial time in seconds
    uint32_t m_tries;       //- 8 how many try for Yowser!, ect
    uint32_t m_gender;      // 9 exit gender
    uint32_t m_enter;       // 10 which entry point was used (advanced scripting)
    uint32_t m_exit;        // 11 which exit was used
    uint32_t m_finished;    //+ 12 one (1) for levels that were finished
    uint32_t m_result;      // 13 advanced scripting
    uint32_t m_reg1;        // 14 advanced scripting
    uint32_t m_reg2;        // 15 advanced scripting
    uint32_t m_reg3;        // 16 advanced scripting
    uint32_t m_reg4;        // 17 advanced scripting
    uint32_t m_menu;        // 18 menu processing
    uint32_t m_flags;       // 19 internal
    uint32_t m_tools;       //+ 20 tools carried on exit
    uint32_t m_keys;        //+ 21 keys carried on exit
    uint32_t m_lastTime;    //+ 22 seconds left
    uint32_t m_levelScore;  //+ 23 final score
    uint32_t m_checksum;    // 24 level checksum
};

class CC2HighScore {
public:
    class ScoreData {
    public:
        ScoreData() = default;

        std::string filename() const { return m_filename; }
        std::string gameType() const { return m_gameType; }
        std::string title() const { return m_title; }

        void setFilename(std::string filename) { m_filename = std::move(filename); }
        void setGameType(std::string type) { m_gameType = std::move(type); }
        void setTitle(std::string title) { m_title = std::move(title); }

        SaveData& saveData() { return m_save; }
        const SaveData& saveData() const { return m_save; }

    private:
        std::string m_filename;
        std::string m_gameType;
        std::string m_title;
        SaveData m_save;
    };

    CC2HighScore() : m_version("7") { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    std::string version() const { return m_version; }
    void setVersion(std::string version) { m_version = std::move(version); }

    std::vector<ScoreData>& scores() { return m_scores; }
    const std::vector<ScoreData>& scores() const { return m_scores; }

private:
    std::string m_version;
    std::vector<ScoreData> m_scores;

    std::vector<CC2FieldStorage> m_unknown;
};

class CC2Save {
public:
    CC2Save() : m_version("7") { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    std::string version() const { return m_version; }
    std::string filename() const { return m_filename; }
    std::string gamePath() const { return m_gamePath; }

    void setVersion(std::string version) { m_version = std::move(version); }
    void setFilename(std::string filename) { m_filename = std::move(filename); }
    void setGamePath(std::string path) { m_gamePath = std::move(path); }

    SaveData& saveData() { return m_save; }
    const SaveData& saveData() const { return m_save; }

private:
    std::string m_version;
    std::string m_filename;
    std::string m_gamePath;
    SaveData m_save;

    std::vector<CC2FieldStorage> m_unknown;
};

}

#endif // _CC2MAP_H
