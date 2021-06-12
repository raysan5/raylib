
add_library(glfw "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
                 "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h"
                 internal.h mappings.h context.c init.c input.c monitor.c
                 vulkan.c window.c)

if (_GLFW_COCOA)
    target_sources(glfw PRIVATE cocoa_platform.h cocoa_joystick.h posix_thread.h
                                nsgl_context.h egl_context.h osmesa_context.h
                                cocoa_init.m cocoa_joystick.m cocoa_monitor.m
                                cocoa_window.m cocoa_time.c posix_thread.c
                                nsgl_context.m egl_context.c osmesa_context.c)
elseif (_GLFW_WIN32)
    target_sources(glfw PRIVATE win32_platform.h win32_joystick.h wgl_context.h
                                egl_context.h osmesa_context.h win32_init.c
                                win32_joystick.c win32_monitor.c win32_time.c
                                win32_thread.c win32_window.c wgl_context.c
                                egl_context.c osmesa_context.c)
elseif (_GLFW_X11)
    target_sources(glfw PRIVATE x11_platform.h xkb_unicode.h posix_time.h
                                posix_thread.h glx_context.h egl_context.h
                                osmesa_context.h x11_init.c x11_monitor.c
                                x11_window.c xkb_unicode.c posix_time.c
                                posix_thread.c glx_context.c egl_context.c
                                osmesa_context.c)
elseif (_GLFW_WAYLAND)
    target_sources(glfw PRIVATE wl_platform.h posix_time.h posix_thread.h
                                xkb_unicode.h egl_context.h osmesa_context.h
                                wl_init.c wl_monitor.c wl_window.c posix_time.c
                                posix_thread.c xkb_unicode.c egl_context.c
                                osmesa_context.c)
elseif (_GLFW_OSMESA)
    target_sources(glfw PRIVATE null_platform.h null_joystick.h posix_time.h
                                posix_thread.h osmesa_context.h null_init.c
                                null_monitor.c null_window.c null_joystick.c
                                posix_time.c posix_thread.c osmesa_context.c)
endif()

if (_GLFW_X11 OR _GLFW_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(glfw PRIVATE linux_joystick.h linux_joystick.c)
    else()
        target_sources(glfw PRIVATE null_joystick.h null_joystick.c)
    endif()
endif()

if (_GLFW_WAYLAND)
    find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
    pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)

    macro(wayland_generate protocol_file output_file)
        add_custom_command(OUTPUT "${output_file}.h"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_file}" "${output_file}.h"
            DEPENDS "${protocol_file}"
            VERBATIM)

        add_custom_command(OUTPUT "${output_file}.c"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_file}" "${output_file}.c"
            DEPENDS "${protocol_file}"
            VERBATIM)

        target_sources(glfw PRIVATE "${output_file}.h" "${output_file}.c")
    endmacro()

    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/stable/xdg-shell/xdg-shell.xml"
        "${GLFW_BINARY_DIR}/src/wayland-xdg-shell-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
        "${GLFW_BINARY_DIR}/src/wayland-xdg-decoration-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/stable/viewporter/viewporter.xml"
        "${GLFW_BINARY_DIR}/src/wayland-viewporter-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
        "${GLFW_BINARY_DIR}/src/wayland-relative-pointer-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
        "${GLFW_BINARY_DIR}/src/wayland-pointer-constraints-unstable-v1-client-protocol")
    wayland_generate(
        "${WAYLAND_PROTOCOLS_BASE}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml"
        "${GLFW_BINARY_DIR}/src/wayland-idle-inhibit-unstable-v1-client-protocol")
endif()

if (WIN32 AND BUILD_SHARED_LIBS)
    configure_file(glfw.rc.in glfw.rc @ONLY)
    target_sources(glfw PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/glfw.rc")
endif()

configure_file(glfw_config.h.in glfw_config.h @ONLY)
target_compile_definitions(glfw PRIVATE _GLFW_USE_CONFIG_H)
target_sources(glfw PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/glfw_config.h")

set_target_properties(glfw PROPERTIES
                      OUTPUT_NAME ${GLFW_LIB_NAME}
                      VERSION ${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}
                      SOVERSION ${GLFW_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99
                      C_EXTENSIONS OFF
                      DEFINE_SYMBOL _GLFW_BUILD_DLL
                      FOLDER "GLFW3")

target_include_directories(glfw PUBLIC
                           "$<BUILD_INTERFACE:${GLFW_SOURCE_DIR}/include>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_include_directories(glfw PRIVATE
                           "${GLFW_SOURCE_DIR}/src"
                           "${GLFW_BINARY_DIR}/src"
                           ${glfw_INCLUDE_DIRS})
target_link_libraries(glfw PRIVATE Threads::Threads ${glfw_LIBRARIES})

# Workaround for CMake not knowing about .m files before version 3.16
if ("${CMAKE_VERSION}" VERSION_LESS "3.16" AND APPLE)
    set_source_files_properties(cocoa_init.m cocoa_joystick.m cocoa_monitor.m
                                cocoa_window.m nsgl_context.m PROPERTIES
                                LANGUAGE C)
endif()

# Make GCC warn about declarations that VS 2010 and 2012 won't accept for all
# source files that VS will build (Clang ignores this because we set -std=c99)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(context.c init.c input.c monitor.c vulkan.c
                                window.c win32_init.c win32_joystick.c
                                win32_monitor.c win32_time.c win32_thread.c
                                win32_window.c wgl_context.c egl_context.c
                                osmesa_context.c PROPERTIES
                                COMPILE_FLAGS -Wdeclaration-after-statement)
endif()

# Enable a reasonable set of warnings
# NOTE: The order matters here, Clang-CL matches both MSVC and Clang
if (MSVC)
    target_compile_options(glfw PRIVATE "/W3")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
        CMAKE_C_COMPILER_ID STREQUAL "AppleClang")

    target_compile_options(glfw PRIVATE "-Wall")
endif()

if (_GLFW_WIN32)
    target_compile_definitions(glfw PRIVATE UNICODE _UNICODE)
endif()

# HACK: When building on MinGW, WINVER and UNICODE need to be defined before
# the inclusion of stddef.h (by glfw3.h), which is itself included before
# win32_platform.h.  We define them here until a saner solution can be found
# NOTE: MinGW-w64 and Visual C++ do /not/ need this hack.
if (MINGW)
    target_compile_definitions(glfw PRIVATE WINVER=0x0501)
endif()

# Workaround for legacy MinGW not providing XInput and DirectInput
if (MINGW)
    include(CheckIncludeFile)
    check_include_file(dinput.h DINPUT_H_FOUND)
    check_include_file(xinput.h XINPUT_H_FOUND)
    if (NOT DINPUT_H_FOUND OR NOT XINPUT_H_FOUND)
        target_include_directories(glfw PRIVATE "${GLFW_SOURCE_DIR}/deps/mingw")
    endif()
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Workaround for VS 2008 not shipping with stdint.h
if (MSVC90)
    target_include_directories(glfw PUBLIC "${GLFW_SOURCE_DIR}/deps/vs2008")
endif()

# Check for the DirectX 9 SDK as it is not included with VS 2008
if (MSVC90)
    include(CheckIncludeFile)
    check_include_file(dinput.h DINPUT_H_FOUND)
    if (NOT DINPUT_H_FOUND)
        message(FATAL_ERROR "DirectX 9 headers not found; install DirectX 9 SDK")
    endif()
endif()

if (BUILD_SHARED_LIBS)
    if (WIN32)
        if (MINGW)
            # Remove the dependency on the shared version of libgcc
            # NOTE: MinGW-w64 has the correct default but MinGW needs this
            target_link_libraries(glfw PRIVATE "-static-libgcc")

            # Remove the lib prefix on the DLL (but not the import library)
            set_target_properties(glfw PROPERTIES PREFIX "")

            # Add a suffix to the import library to avoid naming conflicts
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.a")
        else()
            # Add a suffix to the import library to avoid naming conflicts
            set_target_properties(glfw PROPERTIES IMPORT_SUFFIX "dll.lib")
        endif()

        target_compile_definitions(glfw INTERFACE GLFW_DLL)
    endif()

    if (MINGW)
        # Enable link-time exploit mitigation features enabled by default on MSVC
        include(CheckCCompilerFlag)

        # Compatibility with data execution prevention (DEP)
        set(CMAKE_REQUIRED_FLAGS "-Wl,--nxcompat")
        check_c_compiler_flag("" _GLFW_HAS_DEP)
        if (_GLFW_HAS_DEP)
            target_link_libraries(glfw PRIVATE "-Wl,--nxcompat")
        endif()

        # Compatibility with address space layout randomization (ASLR)
        set(CMAKE_REQUIRED_FLAGS "-Wl,--dynamicbase")
        check_c_compiler_flag("" _GLFW_HAS_ASLR)
        if (_GLFW_HAS_ASLR)
            target_link_libraries(glfw PRIVATE "-Wl,--dynamicbase")
        endif()

        # Compatibility with 64-bit address space layout randomization (ASLR)
        set(CMAKE_REQUIRED_FLAGS "-Wl,--high-entropy-va")
        check_c_compiler_flag("" _GLFW_HAS_64ASLR)
        if (_GLFW_HAS_64ASLR)
            target_link_libraries(glfw PRIVATE "-Wl,--high-entropy-va")
        endif()

        # Clear flags again to avoid breaking later tests
        set(CMAKE_REQUIRED_FLAGS)
    endif()

    if (UNIX)
        # Hide symbols not explicitly tagged for export from the shared library
        target_compile_options(glfw PRIVATE "-fvisibility=hidden")
    endif()
endif()

if (GLFW_INSTALL)
    install(TARGETS glfw
            EXPORT glfwTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

