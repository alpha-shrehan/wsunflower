add_executable(hashtest_exe test.c)
include_directories(../../)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions( -DSFCONFIG=1 )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions( -DSFCONFIG=2 )
endif()

include_directories(../../Include/gc)
target_link_libraries(hashtest_exe sunflower)
add_test(hashtest hashtest_exe)