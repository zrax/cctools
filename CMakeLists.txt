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

cmake_minimum_required(VERSION 3.1)
project(CCTools)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(TestBigEndian)
test_big_endian(BYTES_BIG_ENDIAN)
if(${BYTES_BIG_ENDIAN})
    add_definitions(-DBYTES_BIG_ENDIAN)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
    set(CMAKE_COMPILER_IS_CLANG 1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "-Wall -Wextra ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
elseif(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# 5.9 needed for certain features of QUndoStack
find_package(Qt5 5.9 REQUIRED COMPONENTS Core Gui Xml Widgets Sql)

set(CCTools_Tilesets
    ${CMAKE_CURRENT_SOURCE_DIR}/share/cctools/CC2.tis
    ${CMAKE_CURRENT_SOURCE_DIR}/share/cctools/TW32.tis
    ${CMAKE_CURRENT_SOURCE_DIR}/share/cctools/WEP.tis
)

set(QTEXTPAD_WIDGET_ONLY ON)
add_subdirectory(qtextpad)

add_subdirectory(lib/libcc1)
add_subdirectory(lib/libcc2)
add_subdirectory(lib/CommonWidgets)
add_subdirectory(src/CCEdit)
add_subdirectory(src/CCPlay)
add_subdirectory(src/CCHack)
add_subdirectory(src/CC2Edit)

if(WIN32)
    install(FILES ${CCTools_Tilesets}
            DESTINATION .
    )
else()
    install(FILES ${CCTools_Tilesets}
            DESTINATION share/cctools
    )
endif()
