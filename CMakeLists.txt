cmake_minimum_required(VERSION 3.0.0)
project(sunflower VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(tests)
include_directories(.)
include_directories(Include/gc)

# add_compile_options(-O3)

# FILE(GLOB_RECURSE SFCSOURCES *.c)

add_library(
    sunflower 
    "./builtins/built_in.c"
    "./InstructionParser/expr_t.c"
    "./InstructionParser/ipsf_init.c"
    "./Object/osf_mem.c"
    "./Parser/array_t.c"
    "./Parser/dict_t.c"
    "./Parser/psf_byte_t.c"
    "./Parser/psf_gen_ast.c"
    "./Parser/psf_gen_ast_preprocessor.c"
    "./Parser/psf_gen_inst_ast.c"
    "./sunflower.c"
)

if (WIN32)
    target_link_libraries(sunflower ${CMAKE_CURRENT_SOURCE_DIR}/DLLs/gcmt-dll.dll)
elseif (UNIX)
    target_link_libraries(sunflower ${CMAKE_CURRENT_SOURCE_DIR}/StaticLibs/libgc.a ${CMAKE_CURRENT_SOURCE_DIR}/StaticLibs/libcord.a)
endif (WIN32)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
