cmake_minimum_required(VERSION 3.20)

project(LLVM_Lang VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${PROJECT_SOURCE_DIR}/compile_commands.json
        DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

option(LLVM_LANG_ENABLE_LLVM "Enable LLVM package discovery" ON)
option(LLVM_LANG_ENABLE_CLANG_TIDY "Enable clang-tidy during compilation when available" ON)
option(LLVM_LANG_BUILD_TESTS "Enable test configuration" ON)
option(LLVM_LANG_USE_MAGIC_ENUM "Enable bundled magic_enum support" ON)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use cmake -S . -B build instead.")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(LLVM_LANG_WARNING_FLAGS)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    list(APPEND LLVM_LANG_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wshadow)
elseif(MSVC)
    list(APPEND LLVM_LANG_WARNING_FLAGS /W4 /permissive-)
endif()

if(LLVM_LANG_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endif()

add_library(llvm_lang STATIC)
target_compile_features(llvm_lang PUBLIC cxx_std_20)
target_compile_options(llvm_lang PRIVATE ${LLVM_LANG_WARNING_FLAGS})
target_include_directories(llvm_lang PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(LLVM_LANG_USE_MAGIC_ENUM)
    set(MAGIC_ENUM_OPT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(MAGIC_ENUM_OPT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(MAGIC_ENUM_OPT_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/external/magic_enum)
    target_link_libraries(llvm_lang PUBLIC magic_enum::magic_enum)
endif()

file(GLOB_RECURSE LLVM_LANG_SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM LLVM_LANG_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_sources(llvm_lang PRIVATE ${LLVM_LANG_SOURCE_FILES})

add_executable(llvm_lang_bin ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(llvm_lang_bin PRIVATE llvm_lang)

if(LLVM_LANG_ENABLE_LLVM)
    set(LLVM_LANG_LLVM_DIR_HINTS
        "$ENV{LLVM_DIR}"
        "$ENV{HOME}/llvm-project/install-release/lib/cmake/llvm"
        "$ENV{HOME}/llvm-project/build_Release/lib/cmake/llvm"
    )

    if(NOT LLVM_DIR)
        foreach(llvm_hint IN LISTS LLVM_LANG_LLVM_DIR_HINTS)
            if(llvm_hint AND EXISTS "${llvm_hint}/LLVMConfig.cmake")
                set(LLVM_DIR "${llvm_hint}" CACHE PATH "Path to LLVMConfig.cmake" FORCE)
                break()
            endif()
        endforeach()
    endif()

    find_package(LLVM CONFIG REQUIRED)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_DIR}")
    target_include_directories(llvm_lang SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
endif()

include(CTest)
if(BUILD_TESTING AND LLVM_LANG_BUILD_TESTS)
    if(EXISTS ${PROJECT_SOURCE_DIR}/test/CMakeLists.txt)
        add_subdirectory(test)
    else()
        message(STATUS "No test CMake configuration found in ${PROJECT_SOURCE_DIR}/test")
    endif()
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE LLVM_LANG_FORMAT_FILES CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/include/*.h
        ${PROJECT_SOURCE_DIR}/test/*.cpp
    )

    if(LLVM_LANG_FORMAT_FILES)
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${LLVM_LANG_FORMAT_FILES}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Formatting project sources"
        )

        add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${LLVM_LANG_FORMAT_FILES}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Checking source formatting"
        )
    endif()
endif()
