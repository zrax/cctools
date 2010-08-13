find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCPlay_SOURCES
    ../Stream.cpp
    ../Levelset.cpp
    ../Tileset.cpp
    ../DacFile.cpp
    CCPlay.cpp
)

if(WIN32)
    set(CCPlay_SOURCES ${CCPlay_SOURCES} CCPlay.rc)
endif()

set(CCPlay_MOCS
    ../Tileset.h
    CCPlay.h
)

set(CCPlay_RCCS
    CCPlay.qrc
)

qt4_wrap_cpp(CCPlay_MOCSRC ${CCPlay_MOCS})
qt4_add_resources(CCPlay_RCCSRC ${CCPlay_RCCS})
add_executable(CCPlay WIN32 MACOSX_BUNDLE
               ${CCPlay_SOURCES} ${CCPlay_MOCSRC} ${CCPlay_RCCSRC})
target_link_libraries(CCPlay ${QT_LIBRARIES})
set_target_properties(CCPlay PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CCTools_SOURCE_DIR}/bin")

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCPlay)
    set(MACOSX_BUNDLE_INFO_STRING "CCPlay 2.0 ALPHA")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.91.0.48")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.91.0.48")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2010 Michael Hansen")
    #set(MACOSX_BUNDLE_ICON_FILE "boot.icns")

    install(TARGETS CCPlay)
    install(FILES ${CCTools_Tilesets}
            DESTINATION CCPlay.app/Contents
    )
    #install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/boot.icns
    #        DESTINATION CCPlay.app/Contents/Resources
    #)
elseif(NOT WIN32)
    install(TARGETS CCPlay
            RUNTIME DESTINATION bin
    )
endif()