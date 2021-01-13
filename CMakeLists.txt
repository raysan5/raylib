cmake_minimum_required(VERSION 3.0)
project(raylib)

# Directory for easier includes
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# RAYLIB_IS_MAIN determines whether the project is being used from root, or as a dependency.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(RAYLIB_IS_MAIN TRUE)
else()
  set(RAYLIB_IS_MAIN FALSE)
endif()

# Sets compiler flags and language standard
include(CompilerFlags)

# Registers build options that are exposed to cmake
include(CMakeOptions.txt)

# Checks a few environment and compiler configurations
include(BuildOptions)

# Main sources directory (the second parameter sets the output directory name to raylib)
add_subdirectory(src raylib)

if (${BUILD_EXAMPLES})
  MESSAGE(STATUS "Building examples is enabled")
  add_subdirectory(examples)
endif()

enable_testing()
