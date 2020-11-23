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

set(CCHack_HEADERS
    CCHack.h
    HackSettings.h
    PageBitmap.h
    PageGeneral.h
    PageMenus.h
    PageMisc.h
    PageScores.h
    PageSounds.h
    PageStoryline.h
)

set(CCHack_SOURCES
    CCHack.cpp
    HackSettings.cpp
    PageBitmap.cpp
    PageGeneral.cpp
    PageMenus.cpp
    PageMisc.cpp
    PageScores.cpp
    PageSounds.cpp
    PageStoryline.cpp
)

if(WIN32)
    set(CCHack_SOURCES ${CCHack_SOURCES} CCHack.rc)
endif()

add_executable(CCHack WIN32 MACOSX_BUNDLE
               ${CCHack_HEADERS} ${CCHack_SOURCES} CCHack.qrc)
target_link_libraries(CCHack PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Xml
    libcc1
    CommonWidgets
)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCHack)
    set(MACOSX_BUNDLE_INFO_STRING "CCHack 3.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.96.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.96.0")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2020 Michael Hansen")
    set(MACOSX_BUNDLE_ICON_FILE "sock.icns")

    install(TARGETS CCHack
            BUNDLE DESTINATION .
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/sock.icns
            DESTINATION CCHack.app/Contents/Resources
    )
elseif(WIN32)
    install(TARGETS CCHack
            RUNTIME DESTINATION .
    )
else()
    install(TARGETS CCHack
            RUNTIME DESTINATION bin
    )
endif()
