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

#include "GameLogic.h"

#define TILE_DIR(tile)  (ccl::Direction)(((tile) & 0x03) + ccl::DirNorth)

void ccl::GetPreferredDirections(tile_t tile, Direction dirs[])
{
    tile_t north = tile & 0xFC;
    switch (north) {
    case TileBug_N:         // L,F,R,B
        dirs[0] = TILE_DIR(tile + 1);
        dirs[1] = TILE_DIR(tile    );
        dirs[2] = TILE_DIR(tile + 3);
        dirs[3] = TILE_DIR(tile + 2);
        break;
    case TileFireball_N:    // F,R,L,B
        dirs[0] = TILE_DIR(tile    );
        dirs[1] = TILE_DIR(tile + 3);
        dirs[2] = TILE_DIR(tile + 1);
        dirs[3] = TILE_DIR(tile + 2);
        break;
    case TileBall_N:        // F,B
    case TileTank_N:        // F,B  -- Treat tank as going both directions
        dirs[0] = TILE_DIR(tile    );
        dirs[1] = TILE_DIR(tile + 2);
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileGlider_N:      // F,L,R,B
        dirs[0] = TILE_DIR(tile    );
        dirs[1] = TILE_DIR(tile + 1);
        dirs[2] = TILE_DIR(tile + 3);
        dirs[3] = TILE_DIR(tile + 2);
        break;
    case TileCrawler_N:     // R,F,L,B
        dirs[0] = TILE_DIR(tile + 3);
        dirs[1] = TILE_DIR(tile    );
        dirs[2] = TILE_DIR(tile + 1);
        dirs[3] = TILE_DIR(tile + 2);
        break;
    default:
        dirs[0] = DirInvalid;
        dirs[1] = DirInvalid;
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    }
}

static tile_t peekTile(const ccl::LevelData* level, int x, int y,
                       ccl::Direction dir)
{
    tile_t tile = ccl::TileWall;

    switch (dir) {
    case ccl::DirNorth:
        tile = (y > 0) ? level->map().getFG(x, y - 1) : (tile_t)ccl::TileWall;
        if (MASKED_TILE(tile))
            tile = level->map().getBG(x, y - 1);
        break;
    case ccl::DirWest:
        tile = (x > 0) ? level->map().getFG(x - 1, y) : (tile_t)ccl::TileWall;
        if (MASKED_TILE(tile))
            tile = level->map().getBG(x - 1, y);
        break;
    case ccl::DirSouth:
        tile = (y < 31) ? level->map().getFG(x, y + 1) : (tile_t)ccl::TileWall;
        if (MASKED_TILE(tile))
            tile = level->map().getBG(x, y + 1);
        break;
    case ccl::DirEast:
        tile = (x < 31) ? level->map().getFG(x + 1, y) : (tile_t)ccl::TileWall;
        if (MASKED_TILE(tile))
            tile = level->map().getBG(x + 1, y);
        break;
    default:
        break;
    }

    return tile;
}

ccl::MoveState ccl::CheckMove(const LevelData* level, tile_t tile, int x, int y)
{
    Direction dirs[4];
    GetPreferredDirections(tile, dirs);
    if (dirs[0] == DirInvalid)
        return MoveBlocked;

    tile_t north = tile & 0xFC;
    tile_t base = level->map().getFG(x, y);
    if (MASKED_TILE(base))
        base = level->map().getBG(x, y);
    int state = 0;

    switch (base) {
    case TileForce_N:
        dirs[0] = DirNorth;
        dirs[1] = DirInvalid;
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileForce_W:
        dirs[0] = DirWest;
        dirs[1] = DirInvalid;
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileForce_S:
        dirs[0] = DirSouth;
        dirs[1] = DirInvalid;
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileForce_E:
        dirs[0] = DirEast;
        dirs[1] = DirInvalid;
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileForce_Rand:
        // TODO: Blocked isn't really accurate here...
        return MoveBlocked;
    case TileTrap:
        state |= MoveTrapped;
        for (const auto& trap_iter : level->traps()) {
            if (trap_iter.trap.X == x && trap_iter.trap.Y == y
                && level->map().getBG(trap_iter.button.X, trap_iter.button.Y) == TileTrapButton) {
                tile_t trigger = level->map().getFG(trap_iter.button.X, trap_iter.button.Y);
                if (trigger == TileBlock || MOVING_TILE(trigger)
                    || (trigger >= TilePlayer_N && trigger <= TilePlayer_E))
                    state &= ~MoveTrapped;
                break;
            }
        }
        break;
    case TileIce:
        // Preferred directions are only straight and back when on ice.
        dirs[0] = TILE_DIR(tile    );
        dirs[1] = TILE_DIR(tile + 2);
        dirs[2] = DirInvalid;
        dirs[3] = DirInvalid;
        break;
    case TileIce_SE:
        if (TILE_DIR(tile) == DirNorth) {
            dirs[0] = DirEast;
            dirs[1] = DirSouth;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        } else if (TILE_DIR(tile) == DirWest) {
            dirs[0] = DirSouth;
            dirs[1] = DirEast;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        }
        break;
    case TileIce_SW:
        if (TILE_DIR(tile) == DirNorth) {
            dirs[0] = DirWest;
            dirs[1] = DirSouth;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        } else if (TILE_DIR(tile) == DirEast) {
            dirs[0] = DirSouth;
            dirs[1] = DirWest;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        }
        break;
    case TileIce_NW:
        if (TILE_DIR(tile) == DirSouth) {
            dirs[0] = DirWest;
            dirs[1] = DirNorth;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        } else if (TILE_DIR(tile) == DirEast) {
            dirs[0] = DirNorth;
            dirs[1] = DirWest;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        }
        break;
    case TileIce_NE:
        if (TILE_DIR(tile) == DirSouth) {
            dirs[0] = DirEast;
            dirs[1] = DirNorth;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        } else if (TILE_DIR(tile) == DirWest) {
            dirs[0] = DirNorth;
            dirs[1] = DirEast;
            dirs[2] = DirInvalid;
            dirs[3] = DirInvalid;
        }
        break;
    default:
        break;
    }

    for (Direction dir : dirs) {
        if (dir == DirInvalid)
            return (MoveState)(state | MoveBlocked);

        switch (base) {
        case TileBarrier_N:
            if (dir == DirNorth)
                continue;
            break;
        case TileBarrier_W:
            if (dir == DirWest)
                continue;
            break;
        case TileBarrier_S:
            if (dir == DirSouth)
                continue;
            break;
        case TileBarrier_E:
            if (dir == DirEast)
                continue;
            break;
        case TileBarrier_SE:
            if (dir == DirSouth || dir == DirEast)
                continue;
            break;
        default:
            break;
        }

        const tile_t peek = peekTile(level, x, y, dir);
        if (peek == TileWall || peek == TileChip || peek == TileToggleWall
            || peek == TileInvisWall || peek == TileBlock || peek == TileDirt
            || (peek >= TileBlock_N && peek <= TileBlock_E)
            || (peek >= TileExit && peek <= TileDoor_Yellow)
            || (peek >= TileBlueFloor && peek <= TileSocket)
            || (peek >= TileAppearingWall && peek <= TilePopUpWall)
            || (peek >= TileCloner && peek <= TileExitAnim3)
            || (peek >= NUM_TILE_TYPES))
            continue;
        if ((peek == TileBarrier_S || peek == TileBarrier_SE || peek == TileIce_NE
            || peek == TileIce_NW) && dir == DirNorth)
            continue;
        if ((peek == TileBarrier_E || peek == TileBarrier_SE || peek == TileIce_NW
            || peek == TileIce_SW) && dir == DirWest)
            continue;
        if ((peek == TileBarrier_N || peek == TileIce_SE || peek == TileIce_SW)
            && dir == DirSouth)
            continue;
        if ((peek == TileBarrier_W || peek == TileIce_SE || peek == TileIce_NE)
            && dir == DirEast)
            continue;

        if (peek == TileWater) {
            if (north != TileGlider_N)
                state |= MoveDeath;
        }
        if (peek == TileFire) {
            if (north == TileBug_N || north == TileWalker_N)
                continue;
            if (north != TileFireball_N)
                state |= MoveDeath;
        }
        if (peek == TileBomb)
            state |= MoveDeath;
        if (peek == TileTeleport)
            state |= MoveTeleport;
        return (MoveState)(state | (dir - DirNorth));
    }
    return (MoveState)(state | MoveBlocked);
}

tile_t ccl::TurnCreature(tile_t tile, MoveState state)
{
    if ((state & MoveDirMask) >= MoveBlocked)
        return tile;

    return (tile & 0xFC) | (state & MoveDirMask);
}

ccl::Point ccl::AdvanceCreature(const Point& pos, MoveState state)
{
    if ((state & MoveDirMask) >= MoveBlocked)
        return pos;

    Point result;
    switch (state & MoveDirMask) {
    case MoveNorth:
        result.X = pos.X;
        result.Y = pos.Y - 1;
        break;
    case MoveWest:
        result.X = pos.X - 1;
        result.Y = pos.Y;
        break;
    case MoveSouth:
        result.X = pos.X;
        result.Y = pos.Y + 1;
        break;
    case MoveEast:
        result.X = pos.X + 1;
        result.Y = pos.Y;
        break;
    default:
        result = pos;
        break;
    }

    return result;
}

void ccl::ToggleDoors(ccl::LevelData* level)
{
    for (int y=0; y<32; ++y) {
        for (int x=0; x<32; ++x) {
            if (level->map().getBG(x, y) == ccl::TileToggleFloor)
                level->map().setBG(x, y, ccl::TileToggleWall);
            else if (level->map().getBG(x, y) == ccl::TileToggleWall)
                level->map().setBG(x, y, ccl::TileToggleFloor);
            if (level->map().getFG(x, y) == ccl::TileToggleFloor)
                level->map().setFG(x, y, ccl::TileToggleWall);
            else if (level->map().getFG(x, y) == ccl::TileToggleWall)
                level->map().setFG(x, y, ccl::TileToggleFloor);
        }
    }
}
