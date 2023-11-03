install(
        TARGETS raylib EXPORT raylib-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# PKG_CONFIG_LIBS_PRIVATE is used in raylib.pc.in
if (NOT BUILD_SHARED_LIBS)
    include(LibraryPathToLinkerFlags)
    set(PKG_CONFIG_LIBS_PRIVATE ${GLFW_PKG_LIBS})
    string(REPLACE ";" " " PKG_CONFIG_LIBS_PRIVATE "${PKG_CONFIG_LIBS_PRIVATE}")
elseif (BUILD_SHARED_LIBS)
    set(PKG_CONFIG_LIBS_EXTRA "")
endif ()

join_paths(libdir_for_pc_file "\${exec_prefix}" "${CMAKE_INSTALL_LIBDIR}")
join_paths(includedir_for_pc_file "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}")
configure_file(../raylib.pc.in raylib.pc @ONLY)
configure_file(../cmake/raylib-config-version.cmake raylib-config-version.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/raylib.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/raylib-config-version.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raylib")
install(FILES ${PROJECT_SOURCE_DIR}/../cmake/raylib-config.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raylib")

# populates raylib_{FOUND, INCLUDE_DIRS, LIBRARIES, LDFLAGS, DEFINITIONS}
include(PopulateConfigVariablesLocally)
populate_config_variables_locally(raylib)
