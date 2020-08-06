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

set(CC2Edit_HEADERS
    CC2Edit.h
    EditorWidget.h
    History.h
    ImportDialog.h
    ResizeDialog.h
    ScriptEditor.h
    ScriptTools.h
    TestSetup.h
    TileInspector.h
    TileWidgets.h
)

set(CC2Edit_SOURCES
    CC2Edit.cpp
    EditorWidget.cpp
    History.cpp
    ImportDialog.cpp
    ResizeDialog.cpp
    ScriptEditor.cpp
    ScriptTools.cpp
    TestSetup.cpp
    TileInspector.cpp
    TileWidgets.cpp
)

if(WIN32)
    set(CC2Edit_SOURCES ${CC2Edit_SOURCES} CC2Edit.rc)
endif()

add_executable(CC2Edit WIN32 MACOSX_BUNDLE
               ${CC2Edit_HEADERS} ${CC2Edit_SOURCES} CC2Edit.qrc)
target_link_libraries(CC2Edit PRIVATE
    Qt5::Core
    Qt5::Widgets
    libcc1
    libcc2
    CommonWidgets
)

target_include_directories(CC2Edit PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(CC2Edit PRIVATE qtextpad)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CC2Edit)
    set(MACOSX_BUNDLE_INFO_STRING "CC2Edit 3.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.95.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.95.0")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2020 Michael Hansen")
    set(MACOSX_BUNDLE_ICON_FILE "boot.icns")

    install(TARGETS CC2Edit
            BUNDLE DESTINATION .
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/boot.icns
            DESTINATION CC2Edit.app/Contents/Resources
    )
elseif(WIN32)
    install(TARGETS CC2Edit
            RUNTIME DESTINATION .
    )
else()
    install(TARGETS CC2Edit
            RUNTIME DESTINATION bin
    )
endif()
