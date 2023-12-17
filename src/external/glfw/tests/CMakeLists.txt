
link_libraries(glfw)

include_directories("${GLFW_SOURCE_DIR}/deps")

if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h"
            "${GLFW_SOURCE_DIR}/deps/glad_gl.c")
set(GLAD_VULKAN "${GLFW_SOURCE_DIR}/deps/glad/vulkan.h"
                "${GLFW_SOURCE_DIR}/deps/glad_vulkan.c")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
                "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

if (${CMAKE_VERSION} VERSION_EQUAL "3.1.0" OR
    ${CMAKE_VERSION} VERSION_GREATER "3.1.0")
    set(CMAKE_C_STANDARD 99)
else()
    # Remove this fallback when removing support for CMake version less than 3.1
    add_compile_options("$<$<C_COMPILER_ID:AppleClang>:-std=c99>"
                        "$<$<C_COMPILER_ID:Clang>:-std=c99>"
                        "$<$<C_COMPILER_ID:GNU>:-std=c99>")

endif()

add_executable(clipboard clipboard.c ${GETOPT} ${GLAD_GL})
add_executable(events events.c ${GETOPT} ${GLAD_GL})
add_executable(msaa msaa.c ${GETOPT} ${GLAD_GL})
add_executable(glfwinfo glfwinfo.c ${GETOPT} ${GLAD_GL} ${GLAD_VULKAN})
add_executable(iconify iconify.c ${GETOPT} ${GLAD_GL})
add_executable(monitors monitors.c ${GETOPT} ${GLAD_GL})
add_executable(reopen reopen.c ${GLAD_GL})
add_executable(cursor cursor.c ${GLAD_GL})

add_executable(empty WIN32 MACOSX_BUNDLE empty.c ${TINYCTHREAD} ${GLAD_GL})
add_executable(gamma WIN32 MACOSX_BUNDLE gamma.c ${GLAD_GL})
add_executable(icon WIN32 MACOSX_BUNDLE icon.c ${GLAD_GL})
add_executable(inputlag WIN32 MACOSX_BUNDLE inputlag.c ${GETOPT} ${GLAD_GL})
add_executable(joysticks WIN32 MACOSX_BUNDLE joysticks.c ${GLAD_GL})
add_executable(opacity WIN32 MACOSX_BUNDLE opacity.c ${GLAD_GL})
add_executable(tearing WIN32 MACOSX_BUNDLE tearing.c ${GLAD_GL})
add_executable(threads WIN32 MACOSX_BUNDLE threads.c ${TINYCTHREAD} ${GLAD_GL})
add_executable(timeout WIN32 MACOSX_BUNDLE timeout.c ${GLAD_GL})
add_executable(title WIN32 MACOSX_BUNDLE title.c ${GLAD_GL})
add_executable(triangle-vulkan WIN32 triangle-vulkan.c ${GLAD_VULKAN})
add_executable(windows WIN32 MACOSX_BUNDLE windows.c ${GETOPT} ${GLAD_GL})

target_link_libraries(empty "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries(threads "${CMAKE_THREAD_LIBS_INIT}")
if (RT_LIBRARY)
    target_link_libraries(empty "${RT_LIBRARY}")
    target_link_libraries(threads "${RT_LIBRARY}")
endif()

set(GUI_ONLY_BINARIES empty gamma icon inputlag joysticks opacity tearing
    threads timeout title triangle-vulkan windows)
set(CONSOLE_BINARIES clipboard events msaa glfwinfo iconify monitors reopen
                     cursor)

set_target_properties(${GUI_ONLY_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                      FOLDER "GLFW3/Tests")

if (MSVC)
    # Tell MSVC to use main instead of WinMain
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
elseif (CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    # Tell Clang using MS CRT to use main instead of WinMain
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          LINK_FLAGS "-Wl,/entry:mainCRTStartup")
endif()

if (APPLE)
    set_target_properties(empty PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Empty Event")
    set_target_properties(gamma PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Gamma")
    set_target_properties(inputlag PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Input Lag")
    set_target_properties(joysticks PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Joysticks")
    set_target_properties(opacity PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Opacity")
    set_target_properties(tearing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Tearing")
    set_target_properties(threads PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Threads")
    set_target_properties(timeout PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Timeout")
    set_target_properties(title PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Title")
    set_target_properties(windows PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Windows")

    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

