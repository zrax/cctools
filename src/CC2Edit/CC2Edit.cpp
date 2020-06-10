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

#include "CC2Edit.h"
#include "EditorWidget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ccl::FileStream fs;

    // Test for exploring format
    auto testMap = new cc2::Map;
    testMap->setAuthor("Michael Hansen");
    testMap->setTitle("TEST 01");
    testMap->option().setView(cc2::MapOption::View10x10);
    testMap->mapData().resize(10, 10);
    testMap->mapData().tile(0, 0)->set(cc2::Tile::Player, cc2::Tile::South);
    testMap->mapData().tile(0, 1)->set(cc2::Tile::UNUSED_Barrier_E);
    testMap->mapData().tile(1, 1)->set(cc2::Tile::UNUSED_Barrier_S);
    testMap->mapData().tile(2, 1)->set(cc2::Tile::UNUSED_Barrier_SE);
    testMap->mapData().tile(3, 1)->set(cc2::Tile::UNUSED_41);
    testMap->mapData().tile(4, 1)->set(cc2::Tile::UNUSED_Cloner);
    testMap->mapData().tile(5, 1)->set(cc2::Tile::UNUSED_53);
    testMap->mapData().tile(6, 1)->set(cc2::Tile::UNUSED_54);
    testMap->mapData().tile(7, 1)->set(cc2::Tile::UNUSED_55);
    testMap->mapData().tile(8, 1)->set(cc2::Tile::UNUSED_Explosion);
    testMap->mapData().tile(9, 1)->set(cc2::Tile::UNUSED_5d);
    testMap->mapData().tile(0, 3)->set(cc2::Tile::UNUSED_67);
    testMap->mapData().tile(1, 3)->set(cc2::Tile::UNUSED_6c);
    testMap->mapData().tile(2, 3)->set(cc2::Tile::UNUSED_6e);
    testMap->mapData().tile(3, 3)->set(cc2::Tile::UNUSED_74);
    testMap->mapData().tile(4, 3)->set(cc2::Tile::UNUSED_75);
    testMap->mapData().tile(5, 3)->set(cc2::Tile::UNUSED_79);
    testMap->mapData().tile(6, 3)->set(cc2::Tile::UNUSED_85);
    testMap->mapData().tile(7, 3)->set(cc2::Tile::UNUSED_86);
    testMap->mapData().tile(8, 3)->set(cc2::Tile::UNUSED_91);
    testMap->mapData().tile(0, 4)->set(cc2::Tile::Player2, cc2::Tile::South);

    testMap->mapData().tile(0, 5)->setModifier(cc2::TileModifier::WireSouth | cc2::TileModifier::WireEast);
    testMap->mapData().tile(1, 5)->set(cc2::Tile::SteelWall);
    testMap->mapData().tile(1, 5)->setModifier(cc2::TileModifier::WireSouth | cc2::TileModifier::WireEast);
    testMap->mapData().tile(2, 5)->set(cc2::Tile::PanelCanopy);
    testMap->mapData().tile(2, 5)->lower()->set(cc2::Tile::Chip);
    testMap->mapData().tile(2, 5)->setPanelFlags(cc2::Tile::Canopy | cc2::Tile::PanelEast | cc2::Tile::PanelSouth);

    if (fs.open("test\\ctest.c2m", "wb")) {
        testMap->write(&fs);
        fs.close();
    }

    auto ts = new CC2ETileset;
    ts->load("CC2.tis");

    CC2EditorWidget w;
    w.setTileset(ts);
    w.setMap(testMap);
    w.show();

    auto allTiles = new cc2::Map;
    allTiles->setAuthor("Michael Hansen");
    allTiles->setTitle("TEST 02");
    allTiles->mapData().resize(16, 20);
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 16; ++x) {
            int t = (y * 16) + x;
            if (t > 0 && t <= cc2::Tile::Hook
                    && !(t >= cc2::Tile::Modifier8 && t <= cc2::Tile::Modifier32)) {
                allTiles->mapData().tile(x, y)->set(t);
            }
            if (t >= 0xac && t <= 0xef) {
                allTiles->mapData().tile(x, y)->set(cc2::Tile::AsciiGlyph);
                allTiles->mapData().tile(x, y)->setModifier(t - 0x90);
            }
            if ((t >= 0xf0 && t <= 0x107) || (t >= 0x10e && t <= 0x117)
                    || (t >= 0x130 && t <= 0x133)) {
                allTiles->mapData().tile(x, y)->set(cc2::Tile::LogicGate);
                allTiles->mapData().tile(x, y)->setModifier(t - 0xf0);
            }
        }
    }

    if (fs.open("test\\ttest.c2m", "wb")) {
        allTiles->write(&fs);
        fs.close();
    }

    CC2EditorWidget wa;
    wa.setTileset(ts);
    wa.setMap(allTiles);
    wa.show();

    return app.exec();
}
