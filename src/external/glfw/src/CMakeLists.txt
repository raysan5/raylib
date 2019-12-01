
set(common_HEADERS internal.h mappings.h
                   "${GLFW_BINARY_DIR}/src/glfw_config.h"
                   "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
                   "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h")
set(common_SOURCES context.c init.c input.c monitor.c vulkan.c window.c)

if (_GLFW_COCOA)
    set(glfw_HEADERS ${common_HEADERS} cocoa_platform.h cocoa_joystick.h
                     posix_thread.h nsgl_context.h egl_context.h osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} cocoa_init.m cocoa_joystick.m
                     cocoa_monitor.m cocoa_window.m cocoa_time.c posix_thread.c
                     nsgl_context.m egl_context.c osmesa_context.c)
elseif (_GLFW_WIN32)
    set(glfw_HEADERS ${common_HEADERS} win32_platform.h win32_joystick.h
                     wgl_context.h egl_context.h osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} win32_init.c win32_joystick.c
                     win32_monitor.c win32_time.c win32_thread.c win32_window.c
                     wgl_context.c egl_context.c osmesa_context.c)
elseif (_GLFW_X11)
    set(glfw_HEADERS ${common_HEADERS} x11_platform.h xkb_unicode.h posix_time.h
                     posix_thread.h glx_context.h egl_context.h osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} x11_init.c x11_monitor.c x11_window.c
                     xkb_unicode.c posix_time.c posix_thread.c glx_context.c
                     egl_context.c osmesa_context.c)
elseif (_GLFW_WAYLAND)
    set(glfw_HEADERS ${common_HEADERS} wl_platform.h
                     posix_time.h posix_thread.h xkb_unicode.h egl_context.h
                     osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} wl_init.c wl_monitor.c wl_window.c
                     posix_time.c posix_thread.c xkb_unicode.c
                     egl_context.c osmesa_context.c)

    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/stable/xdg-shell/xdg-shell.xml"
        BASENAME xdg-shell)
    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
        BASENAME xdg-decoration)
    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/stable/viewporter/viewporter.xml"
        BASENAME viewporter)
    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
        BASENAME relative-pointer-unstable-v1)
    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
        BASENAME pointer-constraints-unstable-v1)
    ecm_add_wayland_client_protocol(glfw_SOURCES
        PROTOCOL
        "${WAYLAND_PROTOCOLS_PKGDATADIR}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml"
        BASENAME idle-inhibit-unstable-v1)
elseif (_GLFW_OSMESA)
    set(glfw_HEADERS ${common_HEADERS} null_platform.h null_joystick.h
                     posix_time.h posix_thread.h osmesa_context.h)
    set(glfw_SOURCES ${common_SOURCES} null_init.c null_monitor.c null_window.c
                     null_joystick.c posix_time.c posix_thread.c osmesa_context.c)
endif()

if (_GLFW_X11 OR _GLFW_WAYLAND)
    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(glfw_HEADERS ${glfw_HEADERS} linux_joystick.h)
        set(glfw_SOURCES ${glfw_SOURCES} linux_joystick.c)
    else()
        set(glfw_HEADERS ${glfw_HEADERS} null_joystick.h)
        set(glfw_SOURCES ${glfw_SOURCES} null_joystick.c)
    endif()
endif()

if (APPLE)
    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${glfw_SOURCES} PROPERTIES LANGUAGE C)
endif()

# Make GCC and Clang warn about declarations that VS 2010 and 2012 won't accept
# for all source files that VS will build
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")

    if (WIN32)
        set(windows_SOURCES ${glfw_SOURCES})
    else()
        set(windows_SOURCES ${common_SOURCES})
    endif()
    set_source_files_properties(${windows_SOURCES} PROPERTIES
        COMPILE_FLAGS -Wdeclaration-after-statement)
endif()

add_library(glfw ${glfw_SOURCES} ${glfw_HEADERS})
set_target_properties(glfw PROPERTIES
                      OUTPUT_NAME ${GLFW_LIB_NAME}
                      VERSION ${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}
                      SOVERSION ${GLFW_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      FOLDER "GLFW3")

if (${CMAKE_VERSION} VERSION_EQUAL "3.1.0" OR
    ${CMAKE_VERSION} VERSION_GREATER "3.1.0")

    set_target_properties(glfw PROPERTIES C_STANDARD 99)
else()
    # Remove this fallback when removing support for CMake version less than 3.1
    target_compile_options(glfw PRIVATE
                           "$<$<C_COMPILER_ID:AppleClang>:-std=c99>"
                           "$<$<C_COMPILER_ID:Clang>:-std=c99>"
                           "$<$<C_COMPILER_ID:GNU>:-std=c99>")
endif()

target_compile_definitions(glfw PRIVATE _GLFW_USE_CONFIG_H)
target_include_directories(glfw PUBLIC
                           "$<BUILD_INTERFACE:${GLFW_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_include_directories(glfw PRIVATE
                           "${GLFW_SOURCE_DIR}/src"
                           "${GLFW_BINARY_DIR}/src"
                           ${glfw_INCLUDE_DIRS})

# HACK: When building on MinGW, WINVER and UNICODE need to be defined before
# the inclusion of stddef.h (by glfw3.h), which is itself included before
# win32_platform.h.  We define them here until a saner solution can be found
# NOTE: MinGW-w64 and Visual C++ do /not/ need this hack.
target_compile_definitions(glfw PRIVATE
                           "$<$<BOOL:${MINGW}>:UNICODE;WINVER=0x0501>")

# Enable a reasonable set of warnings (no, -Wextra is not reasonable)
target_compile_options(glfw PRIVATE
                       "$<$<C_COMPILER_ID:AppleClang>:-Wall>"
                       "$<$<C_COMPILER_ID:Clang>:-Wall>"
                       "$<$<C_COMPILER_ID:GNU>:-Wall>")

if (BUILD_SHARED_LIBS)
    if (WIN32)
        if (MINGW)
            # Remove the dependency on the shared version of libgcc
            # NOTE: MinGW-w64 has the correct default but MinGW needs this
            target_link_options(glfw PRIVATE "-static-libgcc")

            # Remove the lib prefix on the DLL (but not the import library)
            set_target_properties(glfw PROPERTIES PREFIX "")

            # Add a suffix to the import library to avoid naming conflicts
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.a")
        else()
            # Add a suffix to the import library to avoid naming conflicts
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.lib")
        endif()

	target_compile_definitions(glfw INTERFACE GLFW_DLL)
    elseif (APPLE)
        # Add -fno-common to work around a bug in Apple's GCC
        target_compile_options(glfw PRIVATE "-fno-common")

        set_target_properties(glfw PROPERTIES
                              INSTALL_NAME_DIR "${CMAKE_INSTALL_LIBDIR}")
    endif()

    if (UNIX)
        # Hide symbols not explicitly tagged for export from the shared library
        target_compile_options(glfw PRIVATE "-fvisibility=hidden")
    endif()

    target_link_libraries(glfw PRIVATE ${glfw_LIBRARIES})
else()
    target_link_libraries(glfw INTERFACE ${glfw_LIBRARIES})
endif()

if (MSVC)
    target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (GLFW_INSTALL)
    install(TARGETS glfw
            EXPORT glfwTargets
            RUNTIME DESTINATION "bin"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

