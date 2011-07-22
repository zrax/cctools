find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCHack_SOURCES
    ../Stream.cpp
    ../ChipsHax.cpp
    ../Win16Rsrc.cpp
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

qt4_wrap_cpp(CCHack_MOCSRC ${CCHack_MOCS})
qt4_add_resources(CCHack_RCCSRC ${CCHack_RCCS})
add_executable(CCHack WIN32 MACOSX_BUNDLE
               ${CCHack_SOURCES} ${CCHack_MOCSRC} ${CCHack_RCCSRC})
target_link_libraries(CCHack ${QT_LIBRARIES})

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
elseif(NOT WIN32)
    install(TARGETS CCHack
            RUNTIME DESTINATION bin
    )
endif()
