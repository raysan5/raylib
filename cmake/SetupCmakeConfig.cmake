include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Setup install of exported targets
install(EXPORT raylib-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/raylib
)

# Macro to write config
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/raylib-config-version.cmake"
  VERSION ${raylib_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Setup install of version config
install(
  FILES
    "../cmake/raylib-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/raylib-config-version.cmake"
  DESTINATION
   ${CMAKE_INSTALL_LIBDIR}/cmake/raylib
)
