# Try to find OSMesa on a Unix system
#
# This will define:
#
#   OSMESA_LIBRARIES   - Link these to use OSMesa
#   OSMESA_INCLUDE_DIR - Include directory for OSMesa
#
# Copyright (c) 2014 Brandon Schaefer <brandon.schaefer@canonical.com>

if (NOT WIN32)

  find_package (PkgConfig)
  pkg_check_modules (PKG_OSMESA QUIET osmesa)

  set (OSMESA_INCLUDE_DIR ${PKG_OSMESA_INCLUDE_DIRS})
  set (OSMESA_LIBRARIES   ${PKG_OSMESA_LIBRARIES})

endif ()
