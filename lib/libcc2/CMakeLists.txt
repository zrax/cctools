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

include_directories(${CCTools_SOURCE_DIR}/lib)

set(libcc2_HEADERS
    Map.h
    Tileset.h
)

set(libcc2_SOURCES
    Map.cpp
    Tileset.cpp
)

add_library(libcc2 STATIC ${libcc2_HEADERS} ${libcc2_SOURCES})
target_link_libraries(libcc2 Qt5::Core libcc1)
