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

    ~LevelMap()
    {
        delete[] m_fgTiles;
        delete[] m_bgTiles;
    }

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

    void setName(const std::string& name) { m_name = name; }
    void setHint(const std::string& hint) { m_hint = hint; }
    void setPassword(const std::string& pass) { m_password = pass; }
    void setLevelNum(int num) { m_levelNum = num; }
    void setChips(int chips) { m_chips = chips; }
    void setTimer(int timer) { m_timer = timer; }

    void trapConnect(int buttonX, int buttonY, int trapX, int trapY);
    void cloneConnect(int buttonX, int buttonY, int cloneX, int cloneY);
    void addMover(int moverX, int moverY);
    void clearLogic(int x, int y);

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
    ~Levelset();

    unsigned int type() const { return m_magic; }
    void setType(unsigned int type) { m_magic = type; }

    ccl::LevelData* level(int num) const { return m_levels[(size_t)num]; }
    int levelCount() const { return (int)m_levels.size(); }
    ccl::LevelData* addLevel();
    void delLevel(int num);

    void read(FILE* stream);
    void write(FILE* stream);

private:
    std::vector<ccl::LevelData*> m_levels;
    unsigned int m_magic;
};

} /* {ccl} */

#endif
