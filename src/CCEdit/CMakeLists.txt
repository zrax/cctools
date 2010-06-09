find_package(Qt4 REQUIRED)
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(CCEdit_SOURCES
    ../Levelset.cpp
    CCEdit.cpp
)

qt4_automoc(${CCEdit_SOURCES})
add_executable(CCEdit ${CCEdit_SOURCES})
target_link_libraries(CCEdit ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
set_target_properties(CCEdit PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CCTools_SOURCE_DIR}/bin")
