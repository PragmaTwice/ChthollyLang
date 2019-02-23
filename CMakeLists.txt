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

# Set include path
include_directories(
    src
    ${gtest_SOURCE_DIR}/include
)

# Force compiler to enable -std=c++17
set(CMAKE_CXX_STANDARD 17)

# compile Demo
add_executable(parser-demo demo/parser-demo.cpp)
add_executable(wparser-demo demo/wparser-demo.cpp)

# compile Testing
add_executable(parser-test test/parser-test.cpp)
target_link_libraries(parser-test gtest_main)

add_executable(irgen-test test/irgen-test.cpp)
target_link_libraries(irgen-test gtest_main)

# exec Testing
gtest_discover_tests(parser-test)
add_test(NAME parser-test COMMAND parser-test)

gtest_discover_tests(irgen-test)
add_test(NAME irgen-test COMMAND irgen-test)

# For gcc/clang, enable CodeCoverage if needed
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

message(STATUS "Check CodeCoverage option (${ENABLE_CODECOV},${CMAKE_CXX_COMPILER_ID})")
if(ENABLE_CODECOV AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  message(STATUS "CodeCoverage is enabled")

  include(CodeCoverage)
  setup_target_for_coverage(${PROJECT_NAME}_coverage parser-test coverage)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")

endif()
