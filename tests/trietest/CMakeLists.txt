add_executable(trietest_exe test.c)
include_directories(../../)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions( -DSFCONFIG=1 )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions( -DSFCONFIG=2 )
endif()

include_directories(../../Include/gc)
target_link_libraries(trietest_exe sunflower)
add_test(trietest trietest_exe)