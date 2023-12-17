
link_libraries(glfw)

include_directories("${GLFW_SOURCE_DIR}/deps")

if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
    set(ICON glfw.rc)
elseif (APPLE)
    set(ICON glfw.icns)
endif()

if (${CMAKE_VERSION} VERSION_EQUAL "3.1.0" OR
    ${CMAKE_VERSION} VERSION_GREATER "3.1.0")
    set(CMAKE_C_STANDARD 99)
else()
    # Remove this fallback when removing support for CMake version less than 3.1
    add_compile_options("$<$<C_COMPILER_ID:AppleClang>:-std=c99>"
                        "$<$<C_COMPILER_ID:Clang>:-std=c99>"
                        "$<$<C_COMPILER_ID:GNU>:-std=c99>")

endif()

set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h"
            "${GLFW_SOURCE_DIR}/deps/glad_gl.c")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
                "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

add_executable(boing WIN32 MACOSX_BUNDLE boing.c ${ICON} ${GLAD_GL})
add_executable(gears WIN32 MACOSX_BUNDLE gears.c ${ICON} ${GLAD_GL})
add_executable(heightmap WIN32 MACOSX_BUNDLE heightmap.c ${ICON} ${GLAD_GL})
add_executable(offscreen offscreen.c ${ICON} ${GLAD_GL})
add_executable(particles WIN32 MACOSX_BUNDLE particles.c ${ICON} ${TINYCTHREAD} ${GETOPT} ${GLAD_GL})
add_executable(sharing WIN32 MACOSX_BUNDLE sharing.c ${ICON} ${GLAD_GL})
add_executable(simple WIN32 MACOSX_BUNDLE simple.c ${ICON} ${GLAD_GL})
add_executable(splitview WIN32 MACOSX_BUNDLE splitview.c ${ICON} ${GLAD_GL})
add_executable(wave WIN32 MACOSX_BUNDLE wave.c ${ICON} ${GLAD_GL})

target_link_libraries(particles "${CMAKE_THREAD_LIBS_INIT}")
if (RT_LIBRARY)
    target_link_libraries(particles "${RT_LIBRARY}")
endif()

set(GUI_ONLY_BINARIES boing gears heightmap particles sharing simple splitview
    wave)
set(CONSOLE_BINARIES offscreen)

set_target_properties(${GUI_ONLY_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
                      FOLDER "GLFW3/Examples")

if (GLFW_USE_OSMESA)
    target_compile_definitions(offscreen PRIVATE USE_NATIVE_OSMESA)
endif()

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
    set_target_properties(boing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Boing")
    set_target_properties(gears PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Gears")
    set_target_properties(heightmap PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Heightmap")
    set_target_properties(particles PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Particles")
    set_target_properties(sharing PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Sharing")
    set_target_properties(simple PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Simple")
    set_target_properties(splitview PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "SplitView")
    set_target_properties(wave PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Wave")

    set_source_files_properties(glfw.icns PROPERTIES
                                MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${GUI_ONLY_BINARIES} PROPERTIES
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_ICON_FILE glfw.icns
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

