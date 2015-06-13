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

#ifndef _GAMELOGIC_H
#define _GAMELOGIC_H

#include "Levelset.h"

#define TILE_DIR(tile)          ((tile & 0x03) + ccl::DirNorth)
#define TURN_TILE(tile, dir)    ((tile & 0xFC) | (dir - 1))
#define TURN_CW(tile)           ((tile & 0xFC) | ((tile - 1) & 0x03))
#define TURN_CCW(tile)          ((tile & 0xFC) | ((tile + 1) & 0x03))

namespace ccl {

enum MoveState {
    MoveOk1,        // OK to move (first choice)
    MoveOk2,        // OK to move (second choice)
    MoveOk3,        // OK to move (third choice)
    MoveOk4,        // OK to move (final choice)
    MoveBlocked,    // All 4 directions are blocked
    MoveDirMask = 0x0F,

    // Extra data
    MoveTrapped  = (1<<4),  // Monster is on unreleased trap
    MoveDeath    = (1<<5),  // OK to move, but results in death
    MoveTeleport = (1<<6),  // Entered teleporter, move to exit teleport
};

void GetPreferredDirections(tile_t tile, ccl::Direction dirs[]);
MoveState CheckMove(LevelData* level, tile_t tile, int x, int y);

tile_t TurnCreature(tile_t tile, MoveState state);
ccl::Point AdvanceCreature(tile_t tile, const ccl::Point& pos, MoveState state);

void ToggleDoors(LevelData* level);

}

#endif
