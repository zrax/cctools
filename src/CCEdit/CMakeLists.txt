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

set(CCEdit_HEADERS
    CCEdit.h
    EditorWidget.h
    LayerWidget.h
    TileWidgets.h
    LevelsetProps.h
    History.h
    AdvancedMechanics.h
    TestSetup.h
    ErrorCheck.h
    Organizer.h
    TileInspector.h
)

set(CCEdit_SOURCES
    CCEdit.cpp
    EditorWidget.cpp
    LayerWidget.cpp
    TileWidgets.cpp
    LevelsetProps.cpp
    History.cpp
    AdvancedMechanics.cpp
    TestSetup.cpp
    ErrorCheck.cpp
    Organizer.cpp
    TileInspector.cpp
)

if(WIN32)
    set(CCEdit_SOURCES ${CCEdit_SOURCES} CCEdit.rc)
endif()

add_executable(CCEdit WIN32 MACOSX_BUNDLE
               ${CCEdit_HEADERS} ${CCEdit_SOURCES} CCEdit.qrc)
target_link_libraries(CCEdit PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Xml
    libcc1
    CommonWidgets
)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCEdit)
    set(MACOSX_BUNDLE_INFO_STRING "CCEdit 3.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.97.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.97.0")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2020 Michael Hansen")
    set(MACOSX_BUNDLE_ICON_FILE "boot.icns")

    install(TARGETS CCEdit
            BUNDLE DESTINATION .
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/boot.icns
            DESTINATION CCEdit.app/Contents/Resources
    )
elseif(WIN32)
    install(TARGETS CCEdit
            RUNTIME DESTINATION .
    )
else()
    install(TARGETS CCEdit
            RUNTIME DESTINATION bin
    )
endif()
