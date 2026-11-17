cmake_minimum_required(VERSION 3.22)

if (PLATFORM MATCHES "^(iOS|tvOS)$" AND NOT DEFINED CMAKE_SYSTEM_NAME AND NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_SYSTEM_NAME "${PLATFORM}")
endif ()
if (CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS)$" AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "17.0" CACHE STRING "Minimum Apple OS version")
endif ()

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

if (UNIX AND NOT APPLE AND NOT "${PLATFORM}" MATCHES "DRM" AND NOT "${PLATFORM}" MATCHES "Web" AND NOT "${PLATFORM}" MATCHES "SDL")
  if (NOT GLFW_BUILD_WAYLAND AND NOT GLFW_BUILD_X11)
    message(FATAL_ERROR "Cannot disable both Wayland and X11")
  endif()
endif()

# Main sources directory (the second parameter sets the output directory name to raylib)
add_subdirectory(src raylib)

if (PROJECT_IS_TOP_LEVEL AND CMAKE_HOST_APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_target(raylib_xcframework
        COMMAND ${CMAKE_COMMAND}
            "-DRAYLIB_APPLE_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DRAYLIB_APPLE_OUTPUT=${CMAKE_CURRENT_BINARY_DIR}/apple/$<CONFIG>"
            "-DRAYLIB_APPLE_CONFIG=$<CONFIG>"
            "-DRAYLIB_APPLE_AUDIO=${SUPPORT_MODULE_RAUDIO}"
            "-DRAYLIB_APPLE_MODELS=${SUPPORT_MODULE_RMODELS}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AppleXCFramework.cmake"
        USES_TERMINAL
        VERBATIM
    )
endif ()

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
