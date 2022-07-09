# - Try to find raylib
# Options:
#   raylib_USE_STATIC_LIBS - OFF by default
#   raylib_VERBOSE         - OFF by default
# Once done, this defines a raylib target that can be passed to
# target_link_libraries as well as following variables:
#
#   raylib_FOUND - System has raylib installed
#   raylib_INCLUDE_DIRS - The include directories for the raylib header(s)
#   raylib_LIBRARIES - The libraries needed to use raylib
#   raylib_LDFLAGS - The linker flags needed with raylib
#   raylib_DEFINITIONS - Compiler switches required for using raylib

set(XPREFIX PC_RAYLIB)

find_package(PkgConfig QUIET)
pkg_check_modules(${XPREFIX} QUIET raylib)

if (raylib_USE_STATIC_LIBS)
    set(XPREFIX ${XPREFIX}_STATIC)
endif()

set(raylib_DEFINITIONS ${${XPREFIX}_CFLAGS})

find_path(raylib_INCLUDE_DIR
    NAMES raylib.h
    HINTS ${${XPREFIX}_INCLUDE_DIRS}
)

set(RAYLIB_NAMES raylib)

if (raylib_USE_STATIC_LIBS)
    set(RAYLIB_NAMES libraylib.a raylib.lib ${RAYLIB_NAMES})
endif()

find_library(raylib_LIBRARY
    NAMES ${RAYLIB_NAMES}
    HINTS ${${XPREFIX}_LIBRARY_DIRS}
)

set(raylib_LIBRARIES    ${raylib_LIBRARY})
set(raylib_LIBRARY_DIRS ${${XPREFIX}_LIBRARY_DIRS})
set(raylib_LIBRARY_DIR  ${raylib_LIBRARY_DIRS})
set(raylib_INCLUDE_DIRS ${raylib_INCLUDE_DIR})
set(raylib_LDFLAGS      ${${XPREFIX}_LDFLAGS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(raylib DEFAULT_MSG
    raylib_LIBRARY
    raylib_INCLUDE_DIR
)

mark_as_advanced(raylib_LIBRARY raylib_INCLUDE_DIR)

if (raylib_USE_STATIC_LIBS)
  add_library(raylib STATIC IMPORTED GLOBAL)
else()
  add_library(raylib SHARED IMPORTED GLOBAL)
endif()
string (REPLACE ";" " " raylib_LDFLAGS "${raylib_LDFLAGS}")

set_target_properties(raylib
  PROPERTIES
  IMPORTED_LOCATION             "${raylib_LIBRARIES}"
  IMPORTED_IMPLIB               "${raylib_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${raylib_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES      "${raylib_LDFLAGS}"
  INTERFACE_COMPILE_OPTIONS     "${raylib_DEFINITIONS}"
)

if (raylib_VERBOSE)
  message(STATUS "raylib_FOUND: ${raylib_FOUND}")
  message(STATUS "raylib_INCLUDE_DIRS: ${raylib_INCLUDE_DIRS}")
  message(STATUS "raylib_LIBRARIES: ${raylib_LIBRARIES}")
  message(STATUS "raylib_LDFLAGS: ${raylib_LDFLAGS}")
  message(STATUS "raylib_DEFINITIONS: ${raylib_DEFINITIONS}")
endif()
