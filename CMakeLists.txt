cmake_minimum_required(VERSION 3.10)

# Download and unpack googletest at configure time
configure_file(googletest/CMakeLists.txt googletest-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# Include CMake/GoogleTest module for `gtest_discover_tests`
include(GoogleTest)

# Enable testing
enable_testing()

# For project ChthollyLang
project(ChthollyLang)

# For gcc/clang, enable CodeCoverage if needed
list(APPEND CMAKE_MODULE_PATH CMakeModules)

if(ENABLE_CODECOV AND CMAKE_CXX_COMPILER_ID MATCHES "clang|gcc")
  include(CodeCoverage)
  setup_target_for_coverage(${PROJECT_NAME}_coverage tests coverage)
endif()

# Set include path
include_directories(
    src
    ${gtest_SOURCE_DIR}/include
)

# Force compiler to enable -std=c++17
set(CMAKE_CXX_STANDARD 17)

# Demo exec
add_executable(parser-demo demo/parser-demo.cpp)
add_executable(wparser-demo demo/wparser-demo.cpp)

# Testing exec
add_executable(parser-test test/parser-test.cpp)
target_link_libraries(parser-test gtest_main)

gtest_discover_tests(parser-test)
add_test(NAME parser-test COMMAND parser-test)
