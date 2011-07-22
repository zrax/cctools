find_package(Qt4 COMPONENTS QtCore QtGui QtSql QtXml QtMain REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCPlay_SOURCES
    ../Stream.cpp
    ../Levelset.cpp
    ../DacFile.cpp
    ../IniFile.cpp
    ../ChipsHax.cpp
    ../CCMetaData.cpp
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

qt4_wrap_cpp(CCPlay_MOCSRC ${CCPlay_MOCS})
qt4_add_resources(CCPlay_RCCSRC ${CCPlay_RCCS})
add_executable(CCPlay WIN32 MACOSX_BUNDLE
               ${CCPlay_SOURCES} ${CCPlay_MOCSRC} ${CCPlay_RCCSRC})
target_link_libraries(CCPlay ${QT_LIBRARIES})

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
elseif(NOT WIN32)
    install(TARGETS CCPlay
            RUNTIME DESTINATION bin
    )
endif()
