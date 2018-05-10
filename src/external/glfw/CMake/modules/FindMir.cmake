# FindMir
# -------
# Finds the Mir library
#
# This will will define the following variables::
#
# MIR_FOUND        - the system has Mir
# MIR_INCLUDE_DIRS - the Mir include directory
# MIR_LIBRARIES    - the Mir libraries
# MIR_DEFINITIONS  - the Mir definitions


find_package (PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules (PC_MIR mirclient>=0.26.2 QUIET)

  find_path(MIR_INCLUDE_DIR NAMES mir_toolkit/mir_client_library.h
                            PATHS ${PC_MIR_INCLUDE_DIRS})

  find_library(MIR_LIBRARY NAMES mirclient
                           PATHS ${PC_MIR_LIBRARIES} ${PC_MIR_LIBRARY_DIRS})

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (MIR
                                     REQUIRED_VARS MIR_LIBRARY MIR_INCLUDE_DIR)

  if (MIR_FOUND)
    set(MIR_LIBRARIES ${MIR_LIBRARY})
    set(MIR_INCLUDE_DIRS ${PC_MIR_INCLUDE_DIRS})
    set(MIR_DEFINITIONS -DHAVE_MIR=1)
  endif()

  mark_as_advanced (MIR_LIBRARY MIR_INCLUDE_DIR)
endif()
