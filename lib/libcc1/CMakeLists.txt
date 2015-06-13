find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)

set(libcc1_SOURCES
    Stream.cpp
    Levelset.cpp
    DacFile.cpp
    IniFile.cpp
    ChipsHax.cpp
    GameLogic.cpp
    CCMetaData.cpp
    Tileset.cpp
)

set(libcc1_MOCS
    Tileset.h
)
qt5_wrap_cpp(libcc1_MOCSRC ${libcc1_MOCS})

add_library(libcc1 STATIC ${libcc1_SOURCES} ${libcc1_MOCSRC})
target_link_libraries(libcc1 Qt5::Core Qt5::Gui Qt5::Xml)
