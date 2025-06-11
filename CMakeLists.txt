cmake_minimum_required(VERSION 3.14...3.31)

project(mcpelauncher_cm_api LANGUAGES C CXX ASM)

option(CM_API_BUILD_SHARED_LIB "Build shared library" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_STANDARD 23)

set(CPM_SOURCE_CACHE ${PROJECT_SOURCE_DIR}/cpm-cache CACHE PATH "")
include(cmake/CPM.cmake)

CPMAddPackage("gh:yugr/Implib.so#ecf7bb51a92a0fb16834c5b698570ab25f9f1d21")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mcpelauncher_cm_api.init.c
    ${CMAKE_CURRENT_BINARY_DIR}/mcpelauncher_cm_api.tramp.S
    COMMAND ${Implib.so_SOURCE_DIR}/implib-gen.py -q
    --target ${CMAKE_C_COMPILER_TARGET}
    ${PROJECT_SOURCE_DIR}/mcpelauncher_cm_api.def
    DEPENDS ${PROJECT_SOURCE_DIR}/mcpelauncher_cm_api.def
)

add_library(mcpelauncher_cm_api_import STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/mcpelauncher_cm_api.init.c
    ${CMAKE_CURRENT_BINARY_DIR}/mcpelauncher_cm_api.tramp.S
)

target_include_directories(mcpelauncher_cm_api_import PUBLIC include)

if(CM_API_BUILD_SHARED_LIB)
    add_library(mcpelauncher_cm_api SHARED
        src/main.cpp
        src/rtti.cpp
        src/shared_object.cpp
        src/string_hash.hpp
    )

    target_include_directories(mcpelauncher_cm_api PRIVATE include)

    CPMAddPackage("gh:BasedInc/libhat@0.5.0")
    target_link_libraries(mcpelauncher_cm_api PRIVATE libhat::libhat)

    target_compile_options(mcpelauncher_cm_api PRIVATE -Wall -Wextra -Wconversion -Wpedantic)

    target_link_options(mcpelauncher_cm_api PRIVATE
        -Wl,--version-script,${PROJECT_SOURCE_DIR}/map.txt
        -Wl,--no-undefined-version
    )
    set_target_properties(mcpelauncher_cm_api PROPERTIES
        LINK_DEPENDS ${PROJECT_SOURCE_DIR}/map.txt
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(mcpelauncher_cm_api PRIVATE -flto)
        target_link_options(mcpelauncher_cm_api PRIVATE -flto)
    endif()

    install(TARGETS mcpelauncher_cm_api LIBRARY DESTINATION mods)
endif()
