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

#include <list>

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

    MapOption();

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

    void read(ccl::Stream* stream, long size);
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

class Tile {
public:
    enum Type {
        // These do NOT match CC1 tiles, so it needs its own enum.
        Invalid = 0,
        Floor, Wall, Ice, Ice_NE, Ice_SE, Ice_SW, Ice_NW, Water, Fire,
        Force_N, Force_E, Force_S, Force_W, ToggleWall, ToggleFloor,
        Teleport_Red, Teleport_Blue, Teleport_Yellow, Teleport_Green,
        Exit, Slime, Player, DirtBlock, Walker, Ship, IceBlock,
        UNUSED_Barrier_E, UNUSED_Barrier_S, UNUSED_Barrier_SE, Gravel,
        ToggleButton, TankButton, BlueTank, Door_Red, Door_Blue,
        Door_Yellow, Door_Green, Key_Red, Key_Blue, Key_Yellow, Key_Green,
        Chip, ExtraChip, Socket, PopUpWall, AppearingWall, InvisWall,
        BlueWall, BlueFloor, Dirt, Ant, Centipede, Ball, Blob,
        AngryTeeth, FireBox, CloneButton, TrapButton, IceCleats, MagnoShoes,
        FireShoes, Flippers, ToolThief, RedBomb, UNUSED_41, Trap,
        UNUSED_Cloner, Cloner, Clue, Force_Rand, AreaCtlButton,
        RevolvDoor_SW, RevolvDoor_NW, RevolvDoor_NE, RevolvDoor_SE,
        TimeBonus, ToggleClock, Transformer, TrainTracks, SteelWall,
        TimeBomb, Helmet, UNUSED_53, UNUSED_54, UNUSED_55, Player2,
        TimidTeeth, UNUSED_Explosion, HikingBoots, MaleOnly, FemaleOnly,
        LogicGate, UNUSED_5d, LogicSwitch, FlameJet_Off, FlameJet_On,
        FlameJetButton, Lightning, YellowTank, YellowTankCtrl,
        MirrorPlayer, MirrorPlayer2, UNUSED_67, BowlingBall, Rover,
        TimePenalty, CustomFloor, UNUSED_6c, PanelCanopy, UNUSED_6e, RRSign,
        CustomWall, AsciiGlyph, LSwitchWall, LSwitchFloor,
        UNUSED_74, UNUSED_75, Modifier8, Modifier16, Modifier32, UNUSED_79,
        Flag10, Flag100, Flag1000, StayUpGWall, PopDownGWall, Disallow,
        Flag2x, DirBlock, FloorMimic, GreenBomb, GreenChip,
        UNUSED_85, UNUSED_86, RevLogicButton, Switch_Off, Switch_On,
        KeyThief, Ghost, SteelFoil, Turtle, Eye, Bribe, SpeedShoes,
        UNUSED_91, Hook,
    };

    enum Direction { North, East, South, West };

    enum ArrowMask {
        ArrowNorth = 0x1,
        ArrowEast = 0x2,
        ArrowSouth = 0x4,
        ArrowWest = 0x8,
    };

    enum PanelFlags {
        PanelNorth = 0x1,
        PanelEast = 0x2,
        PanelSouth = 0x4,
        PanelWest = 0x8,
        Canopy = 0x10,
    };

    explicit Tile(int type = Floor)
        : m_type(type), m_direction(), m_arrowMask(), m_modifier(), m_lower()
    {
        checkLower();
    }

    ~Tile() { delete m_lower; }

    Tile(const Tile& copy);
    Tile& operator=(const Tile& copy);

    Type type() const { return (Type)m_type; }
    Direction direction() const { return (Direction)m_direction; }
    uint8_t arrowMask() const { return m_arrowMask; }
    uint8_t panelFlags() const { return m_panelFlags; }

    void set(int type, Direction dir = (Direction)0)
    {
        m_type = type;
        m_direction = dir;
        checkLower();
    }

    void setArrowMask(uint8_t mask) { m_arrowMask = mask; }
    void setPanelFlags(uint8_t flags) { m_panelFlags = flags; }

    uint32_t modifier() const { return m_modifier; }
    void setModifier(uint32_t modifier) { m_modifier = modifier; }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

    // This may return NULL if the tile should have a lower layer
    // but it hasn't yet been created.  Use checkLower() to ensure
    // a lower layer is created for tiles that need one.
    Tile* lower() { return haveLower() ? m_lower : 0; }
    const Tile* lower() const { return haveLower() ? m_lower : 0; }

    // This method will create the lower layer if necessary
    Tile* checkLower();

private:
    uint8_t m_type;
    uint8_t m_direction;
    uint8_t m_arrowMask;
    uint8_t m_panelFlags;

    // Attached modifier value, if any
    uint32_t m_modifier;

    // Lower-layer tile, if any (All tile data may recurse!)
    Tile* m_lower;

    bool haveLower() const;
    bool haveDirection() const;
};

class MapData {
public:
    MapData() : m_width(), m_height(), m_map() { }
    ~MapData() { delete[] m_map; }

    void read(ccl::Stream* stream, long size);
    void write(ccl::Stream* stream) const;

    uint8_t width() const { return m_width; }
    uint8_t height() const { return m_height; }

    void resize(uint8_t width, uint8_t height);

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

private:
    uint8_t m_width, m_height;
    Tile* m_map;
};

class ReplayInput {
public:
    enum ActionFlags {
        DropItem = 0x1,
        Down = 0x2,
        Left = 0x4,
        Right = 0x8,
        Up = 0x10,
        SwitchPlayer = 0x20,
        CycleInventory = 0x40,
        Player2Mask = 0x80,
    };

    explicit ReplayInput(int frames, uint8_t action)
        : m_frames(frames), m_action(action) { }

    int frames() const { return m_frames; }
    uint8_t action() const { return m_action; }

    void setFrames(int frames) { m_frames = frames; }
    void addFrames(int frames) { m_frames += frames; }
    void setAction(uint8_t action) { m_action = action; }

private:
    int m_frames;
    uint8_t m_action;
};

class ReplayData {
public:
    ReplayData() : m_flag(), m_initRandDir(), m_randSeed() { }

    uint8_t flag() const { return m_flag; }
    Tile::Direction initRandDir() const { return m_initRandDir; }
    uint8_t randSeed() const { return m_randSeed; }

    void setFlag(uint8_t flag) { m_flag = flag; }
    void setInitRandDir(Tile::Direction dir) { m_initRandDir = dir; }
    void setRandSeed(uint8_t seed) { m_randSeed = seed; }

    std::list<ReplayInput>& input() { return m_input; }
    const std::list<ReplayInput>& input() const { return m_input; }

    void read(ccl::Stream* stream, long size);
    void write(ccl::Stream* stream) const;

private:
    Tile::Direction m_initRandDir;
    uint8_t m_flag;
    uint8_t m_randSeed;
    std::list<ReplayInput> m_input;
};

class Map {
public:
    Map();

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

    void setVersion(const std::string& version) { m_version = version; }
    void setLock(const std::string& lock) { m_lock = lock; }
    void setTitle(const std::string& title) { m_title = title; }
    void setAuthor(const std::string& author) { m_author = author; }
    void setEditorVersion(const std::string& version) { m_editorVersion = version; }
    void setClue(const std::string& clue) { m_clue = clue; }
    void setNote(const std::string& note) { m_note = note; }
    void setReadOnly(bool ro) { m_readOnly = ro; }

    MapOption& option() { return m_option; }
    const MapOption& option() const { return m_option; }

    MapData& mapData() { return m_mapData; }
    const MapData& mapData() const { return m_mapData; }

    ReplayData& replay() { return m_replay; }
    const ReplayData& replay() const { return m_replay; }

private:
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
    ReplayData m_replay;
    bool m_readOnly;
};

}

#endif // _CC2MAP_H
