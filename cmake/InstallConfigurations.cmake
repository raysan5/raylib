if (WIN32 AND SHARED)
    install(
            TARGETS raylib
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
else ()
    install(
            TARGETS raylib
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
endif ()

# PKG_CONFIG_LIBS_PRIVATE is used in raylib.pc.in
if (STATIC)
    include(LibraryPathToLinkerFlags)
    library_path_to_linker_flags(__PKG_CONFIG_LIBS_PRIVATE "${LIBS_PRIVATE}")
    set(PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE} ${GLFW_PKG_LIBS})
    string(REPLACE ";" " " PKG_CONFIG_LIBS_PRIVATE "${PKG_CONFIG_LIBS_PRIVATE}")
elseif (SHARED)
    set(PKG_CONFIG_LIBS_EXTRA "")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_MACOSX_RPATH ON)
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