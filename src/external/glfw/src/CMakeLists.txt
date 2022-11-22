
add_library(glfw ${GLFW_LIBRARY_TYPE}
                 "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
                 "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h"
                 internal.h platform.h mappings.h
                 context.c init.c input.c monitor.c platform.c vulkan.c window.c
                 egl_context.c osmesa_context.c null_platform.h null_joystick.h
                 null_init.c null_monitor.c null_window.c null_joystick.c)

# The time, thread and module code is shared between all backends on a given OS,
# including the null backend, which still needs those bits to be functional
if (APPLE)
    target_sources(glfw PRIVATE cocoa_time.h cocoa_time.c posix_thread.h
                                posix_module.c posix_thread.c)
elseif (WIN32)
    target_sources(glfw PRIVATE win32_time.h win32_thread.h win32_module.c
                                win32_time.c win32_thread.c)
else()
    target_sources(glfw PRIVATE posix_time.h posix_thread.h posix_module.c
                                posix_time.c posix_thread.c)
endif()

add_custom_target(update_mappings
    COMMAND "${CMAKE_COMMAND}" -P "${GLFW_SOURCE_DIR}/CMake/GenerateMappings.cmake" mappings.h.in mappings.h
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Updating gamepad mappings from upstream repository"
    SOURCES mappings.h.in "${GLFW_SOURCE_DIR}/CMake/GenerateMappings.cmake"
    VERBATIM)

set_target_properties(update_mappings PROPERTIES FOLDER "GLFW3")

if (GLFW_BUILD_COCOA)
    target_compile_definitions(glfw PRIVATE _GLFW_COCOA)
    target_sources(glfw PRIVATE cocoa_platform.h cocoa_joystick.h cocoa_init.m
                                cocoa_joystick.m cocoa_monitor.m cocoa_window.m
                                nsgl_context.m)
endif()

if (GLFW_BUILD_WIN32)
    target_compile_definitions(glfw PRIVATE _GLFW_WIN32)
    target_sources(glfw PRIVATE win32_platform.h win32_joystick.h win32_init.c
                                win32_joystick.c win32_monitor.c win32_window.c
                                wgl_context.c)
endif()

if (GLFW_BUILD_X11)
    target_compile_definitions(glfw PRIVATE _GLFW_X11)
    target_sources(glfw PRIVATE x11_platform.h xkb_unicode.h x11_init.c
                                x11_monitor.c x11_window.c xkb_unicode.c
                                glx_context.c)
endif()

if (GLFW_BUILD_WAYLAND)
    target_compile_definitions(glfw PRIVATE _GLFW_WAYLAND)
    target_sources(glfw PRIVATE wl_platform.h xkb_unicode.h wl_init.c
                                wl_monitor.c wl_window.c xkb_unicode.c)
endif()

if (GLFW_BUILD_X11 OR GLFW_BUILD_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(glfw PRIVATE linux_joystick.h linux_joystick.c)
    endif()
    target_sources(glfw PRIVATE posix_poll.h posix_poll.c)
endif()

if (GLFW_BUILD_WAYLAND)
    include(CheckIncludeFiles)
    include(CheckFunctionExists)
    check_function_exists(memfd_create HAVE_MEMFD_CREATE)
    if (HAVE_MEMFD_CREATE)
        target_compile_definitions(glfw PRIVATE HAVE_MEMFD_CREATE)
    endif()

    find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

    include(FindPkgConfig)
    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
    pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)
    pkg_get_variable(WAYLAND_CLIENT_PKGDATADIR wayland-client pkgdatadir)

    macro(wayland_generate protocol_file output_file)
        add_custom_command(OUTPUT "${output_file}.h"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_file}" "${output_file}.h"
            DEPENDS "${protocol_file}"
            VERBATIM)

        add_custom_command(OUTPUT "${output_file}-code.h"
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_file}" "${output_file}-code.h"
            DEPENDS "${protocol_file}"
            VERBATIM)

        target_sources(glfw PRIVATE "${output_file}.h" "${output_file}-code.h")
    endmacro()

    wayland_generate(
        "${WAYLAND_CLIENT_PKGDATADIR}/wayland.xml"
        "${GLFW_BINARY_DIR}/src/wayland-client-protocol")
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

if (WIN32 AND GLFW_BUILD_SHARED_LIBRARY)
    configure_file(glfw.rc.in glfw.rc @ONLY)
    target_sources(glfw PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/glfw.rc")
endif()

if (UNIX AND GLFW_BUILD_SHARED_LIBRARY)
    # On Unix-like systems, shared libraries can use the soname system.
    set(GLFW_LIB_NAME glfw)
else()
    set(GLFW_LIB_NAME glfw3)
endif()

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
                           "${GLFW_BINARY_DIR}/src")
target_link_libraries(glfw PRIVATE Threads::Threads)

# Workaround for CMake not knowing about .m files before version 3.16
if (CMAKE_VERSION VERSION_LESS "3.16" AND APPLE)
    set_source_files_properties(cocoa_init.m cocoa_joystick.m cocoa_monitor.m
                                cocoa_window.m nsgl_context.m PROPERTIES
                                LANGUAGE C)
endif()

if (GLFW_BUILD_WIN32)
    list(APPEND glfw_PKG_LIBS "-lgdi32")
endif()

if (GLFW_BUILD_COCOA)
    target_link_libraries(glfw PRIVATE "-framework Cocoa"
                                       "-framework IOKit"
                                       "-framework CoreFoundation")

    set(glfw_PKG_DEPS "")
    set(glfw_PKG_LIBS "-framework Cocoa -framework IOKit -framework CoreFoundation")
endif()

if (GLFW_BUILD_WAYLAND)
    pkg_check_modules(Wayland REQUIRED
        wayland-client>=0.2.7
        wayland-cursor>=0.2.7
        wayland-egl>=0.2.7
        xkbcommon>=0.5.0)

    target_include_directories(glfw PRIVATE ${Wayland_INCLUDE_DIRS})

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(EpollShim)
        if (EPOLLSHIM_FOUND)
            target_include_directories(glfw PRIVATE ${EPOLLSHIM_INCLUDE_DIRS})
            target_link_libraries(glfw PRIVATE ${EPOLLSHIM_LIBRARIES})
        endif()
    endif()
endif()

if (GLFW_BUILD_X11)
    find_package(X11 REQUIRED)
    target_include_directories(glfw PRIVATE "${X11_X11_INCLUDE_PATH}")

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_INCLUDE_PATH)
        message(FATAL_ERROR "RandR headers not found; install libxrandr development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xrandr_INCLUDE_PATH}")

    # Check for Xinerama (legacy multi-monitor support)
    if (NOT X11_Xinerama_INCLUDE_PATH)
        message(FATAL_ERROR "Xinerama headers not found; install libxinerama development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xinerama_INCLUDE_PATH}")

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_INCLUDE_PATH)
        message(FATAL_ERROR "XKB headers not found; install X11 development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xkb_INCLUDE_PATH}")

    # Check for Xcursor (cursor creation from RGBA images)
    if (NOT X11_Xcursor_INCLUDE_PATH)
        message(FATAL_ERROR "Xcursor headers not found; install libxcursor development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xcursor_INCLUDE_PATH}")

    # Check for XInput (modern HID input)
    if (NOT X11_Xi_INCLUDE_PATH)
        message(FATAL_ERROR "XInput headers not found; install libxi development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xi_INCLUDE_PATH}")

    # Check for X Shape (custom window input shape)
    if (NOT X11_Xshape_INCLUDE_PATH)
        message(FATAL_ERROR "X Shape headers not found; install libxext development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xshape_INCLUDE_PATH}")
endif()

if (UNIX AND NOT APPLE)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(glfw PRIVATE "${RT_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lrt")
    endif()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(glfw PRIVATE "${MATH_LIBRARY}")
        list(APPEND glfw_PKG_LIBS "-lm")
    endif()

    if (CMAKE_DL_LIBS)
        target_link_libraries(glfw PRIVATE "${CMAKE_DL_LIBS}")
        list(APPEND glfw_PKG_LIBS "-l${CMAKE_DL_LIBS}")
    endif()
endif()

# Make GCC warn about declarations that VS 2010 and 2012 won't accept for all
# source files that VS will build (Clang ignores this because we set -std=c99)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(context.c init.c input.c monitor.c platform.c vulkan.c
                                window.c null_init.c null_joystick.c null_monitor.c
                                null_window.c win32_init.c win32_joystick.c win32_module.c
                                win32_monitor.c win32_time.c win32_thread.c win32_window.c
                                wgl_context.c egl_context.c osmesa_context.c PROPERTIES
                                COMPILE_FLAGS -Wdeclaration-after-statement)
endif()

if (WIN32)
    if (GLFW_USE_HYBRID_HPG)
        target_compile_definitions(glfw PRIVATE _GLFW_USE_HYBRID_HPG)
    endif()
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

if (GLFW_BUILD_WIN32)
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

# Workaround for -std=c99 on Linux disabling _DEFAULT_SOURCE (POSIX 2008 and more)
if (GLFW_BUILD_X11 OR GLFW_BUILD_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(glfw PRIVATE _DEFAULT_SOURCE)
    endif()
endif()

if (GLFW_BUILD_SHARED_LIBRARY)
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

foreach(arg ${glfw_PKG_DEPS})
    string(APPEND deps " ${arg}")
endforeach()
foreach(arg ${glfw_PKG_LIBS})
    string(APPEND libs " ${arg}")
endforeach()

set(GLFW_PKG_CONFIG_REQUIRES_PRIVATE "${deps}" CACHE INTERNAL
    "GLFW pkg-config Requires.private")
set(GLFW_PKG_CONFIG_LIBS_PRIVATE "${libs}" CACHE INTERNAL
    "GLFW pkg-config Libs.private")

configure_file("${GLFW_SOURCE_DIR}/CMake/glfw3.pc.in" glfw3.pc @ONLY)

if (GLFW_INSTALL)
    install(TARGETS glfw
            EXPORT glfwTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

