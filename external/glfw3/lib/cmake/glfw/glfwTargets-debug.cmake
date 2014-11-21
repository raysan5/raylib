#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glfw" for configuration "Debug"
set_property(TARGET glfw APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glfw PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG ""
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libglfw.3.0.dylib"
  IMPORTED_SONAME_DEBUG "/Users/ratalaika/Repositories/external/glfw3/lib/lib/libglfw.3.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glfw )
list(APPEND _IMPORT_CHECK_FILES_FOR_glfw "${_IMPORT_PREFIX}/lib/libglfw.3.0.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
