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

static void toggleTile(cc2::Tile* tile)
{
    if (tile->haveLower())
        toggleTile(tile->lower());

    switch (tile->type()) {
    case cc2::Tile::ToggleWall:
        tile->set(cc2::Tile::ToggleFloor);
        break;
    case cc2::Tile::ToggleFloor:
        tile->set(cc2::Tile::ToggleWall);
        break;
    case cc2::Tile::GreenBomb:
        tile->set(cc2::Tile::GreenChip);
        break;
    case cc2::Tile::GreenChip:
        tile->set(cc2::Tile::GreenBomb);
        break;
    default:
        break;
    }
}

void cc2::ToggleGreens(Map* map)
{
    MapData& mapData = map->mapData();
    for (int y = 0; y < mapData.height(); ++y) {
        for (int x = 0; x < mapData.width(); ++x) 
            toggleTile(mapData.tile(x, y));
    }
}
