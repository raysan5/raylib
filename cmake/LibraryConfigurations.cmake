# Set OpenGL_GL_PREFERENCE to new "GLVND" even when legacy library exists and
# cmake is <= 3.10
#
# See https://cmake.org/cmake/help/latest/policy/CMP0072.html for more
# information.
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

if (SUPPORT_VULKAN AND Vulkan_FOUND)
    set(GRAPHICS "GRAPHICS_API_VULKAN")
    message(STATUS "Vulkan graphics API selected. GRAPHICS set to GRAPHICS_API_VULKAN.")
    # Any Vulkan-specific LIBS_PRIVATE additions can be handled here or in src/CMakeLists.txt
    # For now, assuming Vulkan::Vulkan and Vulkan_LIBRARIES cover necessary linking.
else()
    # ORIGINAL OPENGL-SPECIFIC LOGIC
    if (${PLATFORM} MATCHES "Desktop")
        set(PLATFORM_CPP "PLATFORM_DESKTOP")

        if (APPLE)
            # Need to force OpenGL 3.3 on OS X
            # See: https://github.com/raysan5/raylib/issues/341
            set(GRAPHICS "GRAPHICS_API_OPENGL_33")
            find_library(OPENGL_LIBRARY OpenGL)
            set(LIBS_PRIVATE ${OPENGL_LIBRARY})
            link_libraries("${LIBS_PRIVATE}")
            if (NOT CMAKE_SYSTEM STRLESS "Darwin-18.0.0")
                add_definitions(-DGL_SILENCE_DEPRECATION)
                MESSAGE(AUTHOR_WARNING "OpenGL is deprecated starting with macOS 10.14 (Mojave)!")
            endif ()
        elseif (WIN32)
            add_definitions(-D_CRT_SECURE_NO_WARNINGS)
            find_package(OpenGL QUIET)
            set(LIBS_PRIVATE ${OPENGL_LIBRARIES} winmm)
        elseif (UNIX)
            find_library(pthread NAMES pthread)
            find_package(OpenGL QUIET)
            if ("${OPENGL_LIBRARIES}" STREQUAL "")
                set(OPENGL_LIBRARIES "GL")
            endif ()

            if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD")
                find_library(OSS_LIBRARY ossaudio)
            endif ()

            set(LIBS_PRIVATE m pthread ${OPENGL_LIBRARIES} ${OSS_LIBRARY})
        else ()
            find_library(pthread NAMES pthread)
            find_package(OpenGL QUIET)
            if ("${OPENGL_LIBRARIES}" STREQUAL "")
                set(OPENGL_LIBRARIES "GL")
            endif ()

            set(LIBS_PRIVATE m atomic pthread ${OPENGL_LIBRARIES} ${OSS_LIBRARY})

            if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD")
                find_library(OSS_LIBRARY ossaudio)
                set(LIBS_PRIVATE m pthread ${OPENGL_LIBRARIES} ${OSS_LIBRARY})
            endif ()

            if (NOT "${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD" AND USE_AUDIO)
                set(LIBS_PRIVATE ${LIBS_PRIVATE} dl)
            endif ()
        endif ()

    elseif (${PLATFORM} MATCHES "Web")
        set(PLATFORM_CPP "PLATFORM_WEB")
        if(NOT GRAPHICS)
            set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
        endif()
        set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

    elseif (${PLATFORM} MATCHES "Android")
        set(PLATFORM_CPP "PLATFORM_ANDROID")
        set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
        list(APPEND raylib_sources ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
        include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

        # NOTE: We remove '-Wl,--no-undefined' (set by default) as it conflicts with '-Wl,-undefined,dynamic_lookup' needed
        #       for compiling with the missing 'void main(void)' declaration in `android_main()`.
        #       We also remove other unnecessary or problematic flags.

        string(REPLACE "-Wl,--no-undefined -Qunused-arguments" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
        string(REPLACE "-static-libstdc++" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,libatomic.a -Wl,--build-id -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings -u ANativeActivity_onCreate -Wl,-undefined,dynamic_lookup")

        find_library(OPENGL_LIBRARY OpenGL)
        set(LIBS_PRIVATE m log android EGL GLESv2 OpenSLES atomic c)

    elseif ("${PLATFORM}" MATCHES "DRM")
        set(PLATFORM_CPP "PLATFORM_DRM")
        set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

        add_definitions(-D_DEFAULT_SOURCE)
        add_definitions(-DEGL_NO_X11)
        add_definitions(-DPLATFORM_DRM)

        find_library(GLESV2 GLESv2)
        find_library(EGL EGL)
        find_library(DRM drm)
        find_library(GBM gbm)

        if (NOT CMAKE_CROSSCOMPILING OR NOT CMAKE_SYSROOT)
            include_directories(/usr/include/libdrm)
        endif ()
        set(LIBS_PRIVATE ${GLESV2} ${EGL} ${DRM} ${GBM} atomic pthread m dl)

    elseif ("${PLATFORM}" MATCHES "SDL")
        find_package(SDL2 REQUIRED)
        set(PLATFORM_CPP "PLATFORM_DESKTOP_SDL")
        set(LIBS_PRIVATE SDL2::SDL2)
        # For SDL, if it's used with OpenGL, ensure GRAPHICS is set appropriately if not already.
        # This might need adjustment if SDL can also host Vulkan. For now, assuming SDL is for OpenGL.
        if(NOT GRAPHICS)
            set(GRAPHICS "GRAPHICS_API_OPENGL_33") # Default for SDL on desktop, or could be ES for other platforms
        endif()
    endif ()

    if (NOT ${OPENGL_VERSION} MATCHES "OFF")
        set(SUGGESTED_GRAPHICS "${GRAPHICS}")

        if (${OPENGL_VERSION} MATCHES "4.3")
            set(GRAPHICS "GRAPHICS_API_OPENGL_43")
        elseif (${OPENGL_VERSION} MATCHES "3.3")
            set(GRAPHICS "GRAPHICS_API_OPENGL_33")
        elseif (${OPENGL_VERSION} MATCHES "2.1")
            set(GRAPHICS "GRAPHICS_API_OPENGL_21")
        elseif (${OPENGL_VERSION} MATCHES "1.1")
            set(GRAPHICS "GRAPHICS_API_OPENGL_11")
        elseif (${OPENGL_VERSION} MATCHES "ES 2.0")
            set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
        elseif (${OPENGL_VERSION} MATCHES "ES 3.0")
            set(GRAPHICS "GRAPHICS_API_OPENGL_ES3")
        endif ()
        if (NOT "${SUGGESTED_GRAPHICS}" STREQUAL "" AND NOT "${SUGGESTED_GRAPHICS}" STREQUAL "${GRAPHICS}")
            message(WARNING "You are overriding the suggested GRAPHICS=${SUGGESTED_GRAPHICS} with ${GRAPHICS}! This may fail.")
        endif ()
    endif ()

    if (NOT GRAPHICS)
        set(GRAPHICS "GRAPHICS_API_OPENGL_33") # Default OpenGL version if nothing else set it
    endif ()
endif() # End of SUPPORT_VULKAN AND Vulkan_FOUND conditional

# Universal appends to LIBS_PRIVATE
set(LIBS_PRIVATE ${LIBS_PRIVATE} ${OPENAL_LIBRARY})

if (${PLATFORM} MATCHES "Desktop")
    # This implies glfw is used for both Vulkan and OpenGL on Desktop.
    # If GLFW is only for OpenGL, this should be in the else block.
    # Given rcore_vulkan_glfw.c, it's likely needed for Vulkan too.
    set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
endif ()
