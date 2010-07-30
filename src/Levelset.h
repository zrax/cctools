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
#include "Errors.h"

typedef unsigned char tile_t;

namespace ccl {

struct Point    { int X, Y; };
struct Trap     { Point button, trap; };
struct Clone    { Point button, clone; };

class LevelMap {
public:
    LevelMap(int height = 32, int width = 32);

    LevelMap(const LevelMap& init)
        : m_fgTiles(0), m_bgTiles(0)
    {
        operator=(init);
    }

    ~LevelMap()
    {
        delete[] m_fgTiles;
        delete[] m_bgTiles;
    }

    LevelMap& operator=(const LevelMap& source);
    void copyFrom(const LevelMap& source, int destX = 0, int destY = 0);

    tile_t getFG(int x, int y) const
    { return m_fgTiles[(m_width*y) + x]; }

    tile_t getBG(int x, int y) const
    { return m_bgTiles[(m_width*y) + x]; }

    void setFG(int x, int y, tile_t tile)
    { m_fgTiles[(m_width*y) + x] = tile; }

    void setBG(int x, int y, tile_t tile)
    { m_bgTiles[(m_width*y) + x] = tile; }

    void push(int x, int y, tile_t tile);
    tile_t pop(int x, int y);

    long read(FILE* stream);
    long write(FILE* stream);

private:
    tile_t* m_fgTiles;
    tile_t* m_bgTiles;
    int m_height, m_width;
};


class LevelData {
public:
    enum FieldMarker {
        FieldName = 3, FieldTraps = 4, FieldClones = 5, FieldPassword = 6,
        FieldHint = 7, FieldMoveList = 10,
    };

public:
    LevelData();
    LevelData(const LevelData& init);

    const ccl::LevelMap& map() const { return m_map; }
    ccl::LevelMap& map() { return m_map; }

    std::string name() const { return m_name; }
    std::string hint() const { return m_hint; }
    std::string password() const { return m_password; }
    unsigned short levelNum() const { return m_levelNum; }
    unsigned short chips() const { return m_chips; }
    unsigned short timer() const { return m_timer; }
    const std::list<ccl::Trap>& traps() const { return m_traps; }
    const std::list<ccl::Clone>& clones() const { return m_clones; }
    const std::list<ccl::Point>& moveList() const { return m_moveList; }
    std::list<ccl::Trap>& traps() { return m_traps; }
    std::list<ccl::Clone>& clones() { return m_clones; }
    std::list<ccl::Point>& moveList() { return m_moveList; }

    void setName(const std::string& name) { m_name = name; }
    void setHint(const std::string& hint) { m_hint = hint; }
    void setPassword(const std::string& pass) { m_password = pass; }
    void setLevelNum(int num) { m_levelNum = num; }
    void setChips(int chips) { m_chips = chips; }
    void setTimer(int timer) { m_timer = timer; }

    void trapConnect(int buttonX, int buttonY, int trapX, int trapY);
    void cloneConnect(int buttonX, int buttonY, int cloneX, int cloneY);
    void addMover(int moverX, int moverY);

    long read(FILE* stream);
    long write(FILE* stream);

private:
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
        TypeMS   = 0x0002AAAC,
        TypeLynx = 0x0102AAAC,
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
    void insertLevel(int where, ccl::LevelData* level);
    ccl::LevelData* takeLevel(int num);

    void read(FILE* stream);
    void write(FILE* stream);

private:
    std::vector<ccl::LevelData*> m_levels;
    unsigned int m_magic;
};

enum Direction { DirInvalid, DirNorth, DirWest, DirSouth, DirEast };

#define DIRTILE(name) \
    Tile##name##_N, Tile##name##_W, Tile##name##_S, Tile##name##_E

#define COLORTILE(name) \
    Tile##name##_Blue, Tile##name##_Red, Tile##name##_Green, Tile##name##_Yellow

enum TileType {
    TileFloor, TileWall, TileChip, TileWater, TileFire, TileInvisWall,
    DIRTILE(Barrier), TileBlock, TileDirt, TileIce, TileForce_S, DIRTILE(Block),
    TileForce_N, TileForce_E, TileForce_W, TileExit, COLORTILE(Door),
    TileIce_SE, TileIce_SW, TileIce_NW, TileIce_NE, TileBlueFloor, TileBlueWall,
    Tile_UNUSED_20, TileThief, TileSocket, TileToggleButton, TileCloneButton,
    TileToggleWall, TileToggleFloor, TileTrapButton, TileTankButton,
    TileTeleport, TileBomb, TileTrap, TileAppearingWall, TileGravel,
    TilePopUpWall, TileHint, TileBarrier_SE, TileCloner, TileForce_Rand,
    TilePlayerSplash, TilePlayerFire, TilePlayerBurnt, Tile_UNUSED_36,
    Tile_UNUSED_37, Tile_UNUSED_38, TilePlayerExit, TileExitAnim2, TileExitAnim3,
    DIRTILE(PlayerSwim), DIRTILE(Bug), DIRTILE(Fireball), DIRTILE(Ball),
    DIRTILE(Tank), DIRTILE(Glider), DIRTILE(Teeth), DIRTILE(Walker),
    DIRTILE(Blob), DIRTILE(Crawler), COLORTILE(Key), TileFlippers,
    TileFireBoots, TileIceSkates, TileForceBoots, DIRTILE(Player),
    NUM_TILE_TYPES,

    MONSTER_FIRST = TileBug_N,
    MONSTER_LAST = TileCrawler_E,
};

#define MOVING_TILE(tile)  (((tile) >= ccl::TileBlock_N && (tile) <= ccl::TileBlock_E) \
                            || ((tile) >= ccl::MONSTER_FIRST && (tile) <= ccl::MONSTER_LAST))
#define MONSTER_TILE(tile) ((tile) >= ccl::MONSTER_FIRST && (tile) <= ccl::MONSTER_LAST)
#define FORCE_TILE(tile)   ((tile) == ccl::TileForce_S || (tile) == ccl::TileForce_N \
                            || (tile) == ccl::TileForce_E || (tile) == ccl::TileForce_W)

} /* {ccl} */

#endif
