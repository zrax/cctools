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

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CCTools_SOURCE_DIR}/lib)

set(CCPlay_HEADERS
    CCPlay.h
    PlaySettings.h
)

set(CCPlay_SOURCES
    CCPlay.cpp
    PlaySettings.cpp
)

if(WIN32)
    configure_file(CCPlay.rc.in CCPlay.rc @ONLY)
    list(APPEND CCPlay_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/CCPlay.rc")
endif()

add_executable(CCPlay WIN32 MACOSX_BUNDLE
               ${CCPlay_HEADERS} ${CCPlay_SOURCES} CCPlay.qrc)
target_link_libraries(CCPlay PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    libcc1
    CommonWidgets
)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCPlay)
    set(MACOSX_BUNDLE_INFO_STRING "CCPlay 3.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CCTOOLS_VER_STRING}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CCTOOLS_VER_STRING}")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2020 Michael Hansen")
    set(MACOSX_BUNDLE_ICON_FILE "chip.icns")

    install(TARGETS CCPlay
            BUNDLE DESTINATION .
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/chip.icns
            DESTINATION CCPlay.app/Contents/Resources
    )
elseif(WIN32)
    install(TARGETS CCPlay
            RUNTIME DESTINATION .
    )
else()
    install(TARGETS CCPlay
            RUNTIME DESTINATION bin
    )
endif()
