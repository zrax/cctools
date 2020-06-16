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

#define GET_N1(x, y, level) ((y > 0) ? (level)->map().getFG(x, y - 1) : (tile_t)TileWall)
#define GET_N2(x, y, level) ((y > 0) ? (level)->map().getBG(x, y - 1) : (tile_t)TileWall)
#define GET_W1(x, y, level) ((x > 0) ? (level)->map().getFG(x - 1, y) : (tile_t)TileWall)
#define GET_W2(x, y, level) ((x > 0) ? (level)->map().getBG(x - 1, y) : (tile_t)TileWall)
#define GET_S1(x, y, level) ((y < 31) ? (level)->map().getFG(x, y + 1) : (tile_t)TileWall)
#define GET_S2(x, y, level) ((y < 31) ? (level)->map().getBG(x, y + 1) : (tile_t)TileWall)
#define GET_E1(x, y, level) ((x < 31) ? (level)->map().getFG(x + 1, y) : (tile_t)TileWall)
#define GET_E2(x, y, level) ((x < 31) ? (level)->map().getBG(x + 1, y) : (tile_t)TileWall)

void ccl::GetPreferredDirections(tile_t tile, ccl::Direction dirs[])
{
    tile_t north = tile & 0xFC;
    switch (north) {
    case TileBug_N:         // L,F,R,B
        dirs[0] = (ccl::Direction)(((tile + 1) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[2] = (ccl::Direction)(((tile - 1) & 0x03) + ccl::DirNorth);
        dirs[3] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        break;
    case TileFireball_N:    // F,R,L,B
        dirs[0] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile - 1) & 0x03) + ccl::DirNorth);
        dirs[2] = (ccl::Direction)(((tile + 1) & 0x03) + ccl::DirNorth);
        dirs[3] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        break;
    case TileBall_N:        // F,B
    case TileTank_N:        // F,B  -- Treat tank as going both directions
        dirs[0] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        dirs[2] = ccl::DirInvalid;
        dirs[3] = ccl::DirInvalid;
        break;
    case TileGlider_N:      // F,L,R,B
        dirs[0] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile + 1) & 0x03) + ccl::DirNorth);
        dirs[2] = (ccl::Direction)(((tile - 1) & 0x03) + ccl::DirNorth);
        dirs[3] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        break;
    case TileCrawler_N:     // R,F,L,B
        dirs[0] = (ccl::Direction)(((tile - 1) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[2] = (ccl::Direction)(((tile + 1) & 0x03) + ccl::DirNorth);
        dirs[3] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        break;
    }
}

ccl::MoveState ccl::CheckMove(ccl::LevelData* level, tile_t tile, int x, int y)
{
    ccl::Direction dirs[4];
    ccl::GetPreferredDirections(tile, dirs);
    tile_t north = tile & 0xFC;
    tile_t base = level->map().getFG(x, y);
    if (MASKED_TILE(base))
        base = level->map().getBG(x, y);
    int state = 0;

    switch (base) {
    case TileForce_N:
        return (ccl::MoveState)(state | MoveNorth);
    case TileForce_W:
        return (ccl::MoveState)(state | MoveWest);
    case TileForce_S:
        return (ccl::MoveState)(state | MoveSouth);
    case TileForce_E:
        return (ccl::MoveState)(state | MoveEast);
    case TileForce_Rand:
        // TODO: Blocked isn't really accurate here...
        return (ccl::MoveState)(state | MoveBlocked);
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
        dirs[0] = (ccl::Direction)(((tile    ) & 0x03) + ccl::DirNorth);
        dirs[1] = (ccl::Direction)(((tile + 2) & 0x03) + ccl::DirNorth);
        dirs[2] = ccl::DirInvalid;
        dirs[3] = ccl::DirInvalid;
        break;
    default:
        break;
    }

    for (int i=0; i<4; ++i) {
        ccl::Direction dir = dirs[i];
        if (dir == DirInvalid)
            return (ccl::MoveState)(state | MoveBlocked);

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
        case TileIce_SE:
            if (dir == DirNorth)
                return (ccl::MoveState)(state | MoveEast);
            if (dir == DirWest)
                return (ccl::MoveState)(state | MoveSouth);
            break;
        case TileIce_SW:
            if (dir == DirNorth)
                return (ccl::MoveState)(state | MoveWest);
            if (dir == DirEast)
                return (ccl::MoveState)(state | MoveSouth);
            break;
        case TileIce_NW:
            if (dir == DirSouth)
                return (ccl::MoveState)(state | MoveWest);
            if (dir == DirEast)
                return (ccl::MoveState)(state | MoveNorth);
            break;
        case TileIce_NE:
            if (dir == DirSouth)
                return (ccl::MoveState)(state | MoveEast);
            if (dir == DirWest)
                return (ccl::MoveState)(state | MoveNorth);
            break;
        }

        tile_t peek = (dir == DirNorth) ? GET_N1(x, y, level)
                    : (dir == DirWest)  ? GET_W1(x, y, level)
                    : (dir == DirSouth) ? GET_S1(x, y, level)
                    :                     GET_E1(x, y, level);
        if (MASKED_TILE(peek)) {
            peek = (dir == DirNorth) ? GET_N2(x, y, level)
                 : (dir == DirWest)  ? GET_W2(x, y, level)
                 : (dir == DirSouth) ? GET_S2(x, y, level)
                 :                     GET_E2(x, y, level);
        }
        if (peek == TileWall || peek == TileChip || peek == TileToggleWall
            || peek == TileInvisWall || peek == TileBlock || peek == TileDirt
            || (peek >= TileBlock_N && peek <= TileBlock_E)
            || (peek >= TileExit && peek <= TileDoor_Yellow)
            || (peek >= TileBlueFloor && peek <= TileSocket)
            || (peek >= TileAppearingWall && peek <= TilePopUpWall)
            || (peek >= TileCloner && peek <= TileExitAnim3))
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
        return (ccl::MoveState)(state | (dirs[i] - DirNorth));
    }
    return (ccl::MoveState)(state | MoveBlocked);
}

tile_t ccl::TurnCreature(tile_t tile, MoveState state)
{
    if ((state & MoveDirMask) >= MoveBlocked)
        return tile;

    return (tile & 0xFC) | (state & MoveDirMask);
}

ccl::Point ccl::AdvanceCreature(const ccl::Point& pos, MoveState state)
{
    if ((state & MoveDirMask) >= MoveBlocked)
        return pos;

    ccl::Point result;
    switch (state & MoveDirMask) {
    case ccl::MoveNorth:
        result.X = pos.X;
        result.Y = pos.Y - 1;
        break;
    case ccl::MoveWest:
        result.X = pos.X - 1;
        result.Y = pos.Y;
        break;
    case ccl::MoveSouth:
        result.X = pos.X;
        result.Y = pos.Y + 1;
        break;
    case ccl::MoveEast:
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
