# This file is part of CCTools.
#
# CCTools is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CCTools is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CCTools.  If not, see <http://www.gnu.org/licenses/>.

set(libcc1_HEADERS
    Errors.h
    Stream.h
    Levelset.h
    DacFile.h
    IniFile.h
    ChipsHax.h
    GameLogic.h
    CCMetaData.h
    Tileset.h
    QtHelpers.h
)

set(libcc1_SOURCES
    Stream.cpp
    Levelset.cpp
    DacFile.cpp
    IniFile.cpp
    ChipsHax.cpp
    GameLogic.cpp
    CCMetaData.cpp
    Tileset.cpp
)

add_library(libcc1 STATIC ${libcc1_HEADERS} ${libcc1_SOURCES})
target_link_libraries(libcc1 Qt5::Core Qt5::Gui Qt5::Xml)
