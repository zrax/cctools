find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCEdit_SOURCES
    ../Stream.cpp
    ../Levelset.cpp
    ../Tileset.cpp
    ../DacFile.cpp
    CCEdit.cpp
    EditorWidget.cpp
    LayerWidget.cpp
    LevelsetProps.cpp
    History.cpp
)

set(CCEdit_MOCS
    ../Tileset.h
    CCEdit.h
    EditorWidget.h
    LayerWidget.h
    LevelsetProps.h
)

set(CCEdit_RCCS
    CCEdit.qrc
)

qt4_wrap_cpp(CCEdit_MOCSRC ${CCEdit_MOCS})
qt4_add_resources(CCEdit_RCCSRC ${CCEdit_RCCS})
add_executable(CCEdit WIN32 MACOSX_BUNDLE
               ${CCEdit_SOURCES} ${CCEdit_MOCSRC} ${CCEdit_RCCSRC})
target_link_libraries(CCEdit ${QT_LIBRARIES})
set_target_properties(CCEdit PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CCTools_SOURCE_DIR}/bin")

if(APPLE)
    add_custom_command(TARGET CCEdit POST_BUILD
                       COMMAND cp ARGS -R ${CCTools_SOURCE_DIR}/share ${CCTools_SOURCE_DIR}/bin/CCEdit.app/Contents/ )
endif()
