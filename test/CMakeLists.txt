set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
add_subdirectory(
    ${PROJECT_SOURCE_DIR}/external/googletest
    ${CMAKE_CURRENT_BINARY_DIR}/googletest
    EXCLUDE_FROM_ALL
)

set(_gtest_targets gtest gtest_main)

if(TARGET gmock)
    list(APPEND _gtest_targets gmock)
endif()

if(TARGET gmock_main)
    list(APPEND _gtest_targets gmock_main)
endif()

foreach(target IN LISTS _gtest_targets)
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "")
endforeach()

unset(_gtest_targets)

include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(llvm_lang_tests ${TEST_SOURCES})
target_link_libraries(llvm_lang_tests PRIVATE GTest::gtest_main llvm_lang)

gtest_discover_tests(llvm_lang_tests)
