find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CCTools_SOURCE_DIR}/lib)

set(CCHack_SOURCES
    CCHack.cpp
    HackSettings.cpp
    PageGeneral.cpp
)

if(WIN32)
    set(CCHack_SOURCES ${CCHack_SOURCES} CCHack.rc)
endif()

set(CCHack_MOCS
    CCHack.h
    PageGeneral.h
)

set(CCHack_RCCS
    CCHack.qrc
)

qt5_wrap_cpp(CCHack_MOCSRC ${CCHack_MOCS})
qt5_add_resources(CCHack_RCCSRC ${CCHack_RCCS})
add_executable(CCHack WIN32 MACOSX_BUNDLE
               ${CCHack_SOURCES} ${CCHack_MOCSRC} ${CCHack_RCCSRC})
target_link_libraries(CCHack Qt5::Core Qt5::Widgets libcc1)

if(APPLE)
    set(MACOSX_BUNDLE true)
    set(MACOSX_BUNDLE_NAME CCHack)
    set(MACOSX_BUNDLE_INFO_STRING "CCHack 2.1")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "2.0.95")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "2.0.95")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2011 Michael Hansen")
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
