find_package(Qt4 REQUIRED)
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCEdit_SOURCES
    ../Levelset.cpp
    CCEdit.cpp
)

set(CCEdit_MOCS
    CCEdit.h
)

set(CCEdit_RCCS
    CCEdit.qrc
)

qt4_wrap_cpp(CCEdit_MOCSRC ${CCEdit_MOCS})
qt4_add_resources(CCEdit_RCCSRC ${CCEdit_RCCS})
add_executable(CCEdit ${CCEdit_SOURCES} ${CCEdit_MOCSRC} ${CCEdit_RCCSRC})
target_link_libraries(CCEdit ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
set_target_properties(CCEdit PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CCTools_SOURCE_DIR}/bin")
