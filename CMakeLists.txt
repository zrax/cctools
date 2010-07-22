project(CCTools)
cmake_minimum_required(VERSION 2.6)

include(TestBigEndian)
test_big_endian(BYTES_BIG_ENDIAN)
if(${BYTES_BIG_ENDIAN})
    add_definitions(-DBYTES_BIG_ENDIAN)
endif()

add_subdirectory(src/CCEdit)
