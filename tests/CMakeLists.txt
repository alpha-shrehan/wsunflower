add_executable(test_exe test.c)
include_directories(../)
add_subdirectory(trietest)
# set(CMAKE_C_FLAGS "-O3 -Wall -Wextra")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions( -DSFCONFIG=1 )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions( -DSFCONFIG=2 )
endif()

include_directories(../Include/gc)
target_link_libraries(test_exe sunflower)
add_test(test test_exe)