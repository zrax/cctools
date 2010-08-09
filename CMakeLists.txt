project(CCTools)
cmake_minimum_required(VERSION 2.6)

include(TestBigEndian)
test_big_endian(BYTES_BIG_ENDIAN)
if(${BYTES_BIG_ENDIAN})
    add_definitions(-DBYTES_BIG_ENDIAN)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
elseif(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

add_subdirectory(src/CCEdit)
add_subdirectory(src/CCPlay)
