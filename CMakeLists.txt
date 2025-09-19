cmake_minimum_required(VERSION 3.25)
#this change avoid the warning that appear when we include raylib using Cmake fatch content 
project(raylib)

# Avoid excessive expansion of variables in conditionals. In particular, if
# "PLATFORM" is "DRM" then:
#
# if (${PLATFORM} MATCHES "DRM")
#
# may expand e.g to:
#
# if (/usr/lib/aarch64-linux-gnu/libdrm.so MATCHES "DRM")
#
# See https://cmake.org/cmake/help/latest/policy/CMP0054.html
cmake_policy(SET CMP0054 NEW)

# Makes a hidden visibility preset on a static lib respected
# This is used to hide glfw's symbols from the library exports when building an so/dylib
# See https://cmake.org/cmake/help/latest/policy/CMP0063.html
cmake_policy(SET CMP0063 NEW)

# Directory for easier includes
# Anywhere you see include(...) you can check <root>/cmake for that file
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Sets compiler flags and language standard
include(CompilerFlags)

# Registers build options that are exposed to cmake
include(CMakeOptions.txt)

if (UNIX AND NOT APPLE AND NOT "${PLATFORM}" MATCHES "DRM" AND NOT "${PLATFORM}" MATCHES "Web")
  if (NOT GLFW_BUILD_WAYLAND AND NOT GLFW_BUILD_X11)
    message(FATAL_ERROR "Cannot disable both Wayland and X11")
  endif()
endif()

# Main sources directory (the second parameter sets the output directory name to raylib)
add_subdirectory(src raylib)

# Uninstall target, only create when building raylib by itself
# Avoid conflicting target names when using raylib with other libraries
if(NOT TARGET uninstall AND PROJECT_IS_TOP_LEVEL)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Uninstall.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

if (${BUILD_EXAMPLES})
  message(STATUS "Building examples is enabled")
  add_subdirectory(examples)
endif()

enable_testing()
