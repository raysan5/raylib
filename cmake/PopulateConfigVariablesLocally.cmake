macro(populate_config_variables_locally target)
  get_property(raylib_INCLUDE_DIRS TARGET ${target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  #get_property(raylib_LIBRARIES    TARGET ${target} PROPERTY LOCATION) # only works for SHARED
  get_property(raylib_LDFLAGS      TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
  get_property(raylib_DEFINITIONS  TARGET ${target} PROPERTY DEFINITIONS)

  set(raylib_INCLUDE_DIRS "${raylib_INCLUDE_DIRS}" PARENT_SCOPE)
  #set(raylib_LIBRARIES   "${raylib_INCLUDE_DIRS}" PARENT_SCOPE)
  set(raylib_LDFLAGS      "${raylib_LDFLAGS}" PARENT_SCOPE)
  set(raylib_DEFINITIONS  "${raylib_DEFINITIONS}" PARENT_SCOPE)
endmacro()
