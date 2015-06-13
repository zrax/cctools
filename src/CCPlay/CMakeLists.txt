find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Xml REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CCTools_SOURCE_DIR}/lib)

set(CCPlay_SOURCES
    CCPlay.cpp
    PlaySettings.cpp
)

if(WIN32)
    set(CCPlay_SOURCES ${CCPlay_SOURCES} CCPlay.rc)
endif()

set(CCPlay_MOCS
    CCPlay.h
    PlaySettings.h
)

set(CCPlay_RCCS
    CCPlay.qrc
)

qt5_wrap_cpp(CCPlay_MOCSRC ${CCPlay_MOCS})
qt5_add_resources(CCPlay_RCCSRC ${CCPlay_RCCS})
add_executable(CCPlay WIN32 MACOSX_BUNDLE
               ${CCPlay_SOURCES} ${CCPlay_MOCSRC} ${CCPlay_RCCSRC})
target_link_libraries(CCPlay Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Xml libcc1)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCPlay)
    set(MACOSX_BUNDLE_INFO_STRING "CCPlay 2.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.0.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.0.0")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2010 Michael Hansen")
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
