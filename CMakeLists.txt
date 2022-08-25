cmake_minimum_required(VERSION 3.0)
project(raylib)

# Avoid excessive expansion of variables in conditionals. In particular, if
# "PLATFORM" is "DRM" than:
#
# if (${PLATFORM} MATCHES "DRM")
#
# may expand e.g to:
#
# if (/usr/lib/aarch64-linux-gnu/libdrm.so MATCHES "DRM")
#
# See https://cmake.org/cmake/help/latest/policy/CMP0054.html
cmake_policy(SET CMP0054 NEW)

# Directory for easier includes
# Anywhere you see include(...) you can check <root>/cmake for that file
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# RAYLIB_IS_MAIN determines whether the project is being used from root
# or if it is added as a dependency (through add_subdirectory for example).
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(RAYLIB_IS_MAIN TRUE)
else()
  set(RAYLIB_IS_MAIN FALSE)
endif()

# Sets compiler flags and language standard
include(CompilerFlags)

# Registers build options that are exposed to cmake
include(CMakeOptions.txt)

# Enforces a few environment and compiler configurations
include(BuildOptions)

# Main sources directory (the second parameter sets the output directory name to raylib)
add_subdirectory(src raylib)

if (${BUILD_EXAMPLES})
  MESSAGE(STATUS "Building examples is enabled")
  add_subdirectory(examples)
endif()

enable_testing()
