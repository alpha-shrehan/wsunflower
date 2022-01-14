cmake_minimum_required(VERSION 3.0.0)
project(sunflower VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(tests)
include_directories(.)
include_directories(Include/gc)

# add_compile_options(-O3)

FILE(GLOB_RECURSE SFCSOURCES *.c)

add_library(sunflower ${SFCSOURCES})
target_link_libraries(sunflower ${CMAKE_CURRENT_SOURCE_DIR}/DLLs/gcmt-dll.dll)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
