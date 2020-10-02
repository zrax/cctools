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

#ifndef _LEVELSET_H
#define _LEVELSET_H

#include <string>
#include <list>
#include <vector>
#include <cstdio>
#include "Stream.h"

#define CCL_WIDTH   32
#define CCL_HEIGHT  32
#define MAX_TRAPS   25
#define MAX_CLONES  31
#define MAX_MOVERS  127

namespace ccl {

struct Point {
    int X, Y;

    bool operator==(const Point& other) const { return X == other.X && Y == other.Y; }
    bool operator!=(const Point& other) const { return X != other.X || Y != other.Y; }
};

struct Trap     { Point button, trap; };
struct Clone    { Point button, clone; };

class LevelMap {
public:
    LevelMap();
    LevelMap(const LevelMap& init) { operator=(init); }

    LevelMap& operator=(const LevelMap& source);
    void copyFrom(const LevelMap& source, int srcX = 0, int srcY = 0,
                  int destX = 0, int destY = 0,
                  int width = CCL_WIDTH, int height = CCL_HEIGHT);

    tile_t getFG(int x, int y) const { return m_fgTiles[(CCL_WIDTH*y) + x]; }
    tile_t getBG(int x, int y) const { return m_bgTiles[(CCL_WIDTH*y) + x]; }

    void setFG(int x, int y, tile_t tile) { m_fgTiles[(CCL_WIDTH*y) + x] = tile; }
    void setBG(int x, int y, tile_t tile) { m_bgTiles[(CCL_WIDTH*y) + x] = tile; }

    void push(int x, int y, tile_t tile);
    tile_t pop(int x, int y);

    long read(Stream* stream);
    long write(Stream* stream) const;

    ccl::Point findNext(int x, int y, tile_t tile) const;

private:
    tile_t m_fgTiles[CCL_WIDTH * CCL_HEIGHT];
    tile_t m_bgTiles[CCL_WIDTH * CCL_HEIGHT];
};


class LevelData {
public:
    enum FieldMarker {
        FieldName = 3, FieldTraps = 4, FieldClones = 5, FieldPassword = 6,
        FieldHint = 7, FieldMoveList = 10,
    };

public:
    LevelData() : m_refs(1), m_levelNum(), m_chips(), m_timer() { }
    LevelData(const LevelData&) = delete;
    LevelData& operator=(const LevelData&) = delete;

    void copyFrom(const LevelData* init);

    const ccl::LevelMap& map() const { return m_map; }
    ccl::LevelMap& map() { return m_map; }

    std::string name() const { return m_name; }
    std::string hint() const { return m_hint; }
    std::string password() const { return m_password; }
    unsigned short chips() const { return m_chips; }
    unsigned short timer() const { return m_timer; }
    const std::list<ccl::Trap>& traps() const { return m_traps; }
    const std::list<ccl::Clone>& clones() const { return m_clones; }
    const std::list<ccl::Point>& moveList() const { return m_moveList; }
    std::list<ccl::Trap>& traps() { return m_traps; }
    std::list<ccl::Clone>& clones() { return m_clones; }
    std::list<ccl::Point>& moveList() { return m_moveList; }

    std::list<ccl::Point> linkedTraps(int x, int y) const;
    std::list<ccl::Point> linkedTrapButtons(int x, int y) const;
    std::list<ccl::Point> linkedCloners(int x, int y) const;
    std::list<ccl::Point> linkedCloneButtons(int x, int y) const;
    bool checkMove(int x, int y) const;

    void setName(const std::string& name) { m_name = name; }
    void setHint(const std::string& hint) { m_hint = hint; }
    void setPassword(const std::string& pass) { m_password = pass; }
    void setLevelNum(int num) { m_levelNum = num; }
    void setChips(int chips) { m_chips = chips; }
    void setTimer(int timer) { m_timer = timer; }

    void trapConnect(int buttonX, int buttonY, int trapX, int trapY);
    void cloneConnect(int buttonX, int buttonY, int cloneX, int cloneY);
    void addMover(int moverX, int moverY);

    long read(Stream* stream, bool forClipboard = false);
    long write(Stream* stream, bool forClipboard = false) const;

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
    ~LevelData() = default;

    int m_refs;
    ccl::LevelMap m_map;
    std::string m_name;
    std::string m_hint;
    std::string m_password;
    int m_levelNum;
    int m_chips, m_timer;
    std::list<ccl::Trap> m_traps;
    std::list<ccl::Clone> m_clones;
    std::list<ccl::Point> m_moveList;
};


class Levelset {
public:
    enum LevelsetType {
        TypeMS     = 0x0002AAAC,
        TypeLynx   = 0x0102AAAC,
        TypePG     = 0x0003AAAC,
        TypeLynxPG = 0x0103AAAC,
    };

    static std::string RandomPassword();

public:
    Levelset(int levelCount = 1);
    Levelset(const Levelset& init);
    ~Levelset();

    unsigned int type() const { return m_magic; }
    void setType(unsigned int type) { m_magic = type; }

    ccl::LevelData* level(int num) const { return m_levels[(size_t)num]; }
    int levelCount() const { return (int)m_levels.size(); }
    ccl::LevelData* addLevel();
    void addLevel(ccl::LevelData* level);
    void insertLevel(int where, ccl::LevelData* level);
    ccl::LevelData* takeLevel(int num);

    void read(Stream* stream);
    void write(Stream* stream) const;

private:
    std::vector<ccl::LevelData*> m_levels;
    unsigned int m_magic;
};

enum LevelsetType { LevelsetError, LevelsetDac, LevelsetCcl };
LevelsetType DetermineLevelsetType(const char* filename);


class ClipboardData {
public:
    ClipboardData() : m_width(), m_height(), m_levelData(new LevelData) { }
    ClipboardData(int width, int height)
        : m_width(width), m_height(height), m_levelData(new LevelData) { }
    ~ClipboardData() { m_levelData->unref(); }

    int width() const { return m_width; }
    int height() const { return m_height; }

    ccl::LevelData* levelData() { return m_levelData; }
    const ccl::LevelData* levelData() const { return m_levelData; }

    void read(Stream* stream);
    void write(Stream* stream) const;

private:
    int m_width, m_height;
    ccl::LevelData* m_levelData;
};


enum Direction { DirInvalid, DirNorth, DirWest, DirSouth, DirEast };

#define MK_DIRTILE(name) \
    Tile##name##_N, Tile##name##_W, Tile##name##_S, Tile##name##_E

#define MK_COLORTILE(name) \
    Tile##name##_Blue, Tile##name##_Red, Tile##name##_Green, Tile##name##_Yellow

enum TileType {
    TileFloor, TileWall, TileChip, TileWater, TileFire, TileInvisWall,
    MK_DIRTILE(Barrier), TileBlock, TileDirt, TileIce, TileForce_S,
    MK_DIRTILE(Block), TileForce_N, TileForce_E, TileForce_W, TileExit,
    MK_COLORTILE(Door), TileIce_SE, TileIce_SW, TileIce_NW, TileIce_NE,
    TileBlueFloor, TileBlueWall, Tile_UNUSED_20, TileThief, TileSocket,
    TileToggleButton, TileCloneButton, TileToggleWall, TileToggleFloor,
    TileTrapButton, TileTankButton, TileTeleport, TileBomb, TileTrap,
    TileAppearingWall, TileGravel, TilePopUpWall, TileHint, TileBarrier_SE,
    TileCloner, TileForce_Rand, TilePlayerSplash, TilePlayerFire,
    TilePlayerBurnt, Tile_UNUSED_36, Tile_UNUSED_37, TileIceBlock,
    TilePlayerExit, TileExitAnim2, TileExitAnim3, MK_DIRTILE(PlayerSwim),
    MK_DIRTILE(Bug), MK_DIRTILE(Fireball), MK_DIRTILE(Ball), MK_DIRTILE(Tank),
    MK_DIRTILE(Glider), MK_DIRTILE(Teeth), MK_DIRTILE(Walker), MK_DIRTILE(Blob),
    MK_DIRTILE(Crawler), MK_COLORTILE(Key), TileFlippers,
    TileFireBoots, TileIceSkates, TileForceBoots, MK_DIRTILE(Player),
    NUM_TILE_TYPES,

    MONSTER_FIRST = TileBug_N,
    MONSTER_LAST = TileCrawler_E,
};

#define MOVING_TILE(tile)  (((tile) >= ccl::TileBlock_N && (tile) <= ccl::TileBlock_E) \
                            || ((tile) >= ccl::MONSTER_FIRST && (tile) <= ccl::MONSTER_LAST))
#define MONSTER_TILE(tile) ((tile) >= ccl::MONSTER_FIRST && (tile) <= ccl::MONSTER_LAST)
#define FORCE_TILE(tile)   ((tile) == ccl::TileForce_S || (tile) == ccl::TileForce_N \
                            || (tile) == ccl::TileForce_E || (tile) == ccl::TileForce_W)
#define MASKED_TILE(tile)  ((tile) >= ccl::MONSTER_FIRST && (tile) < ccl::NUM_TILE_TYPES)

} /* {ccl} */

#endif
