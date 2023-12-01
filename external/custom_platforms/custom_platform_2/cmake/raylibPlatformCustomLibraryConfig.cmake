message(STATUS "From aylibPlatformCustomLibraryConfig (Custom platform 2 (Desktop)")

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

if("${GLFW_ROOT_DIR}" STREQUAL "")
  message(FATAL_ERROR "This platform requires glfw. Provide its location via GLFW_ROOT_DIR")
endif()

message(STATUS "Using glfw from ${GLFW_ROOT_DIR}")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_USE_WAYLAND ${USE_WAYLAND} CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)

add_subdirectory("${GLFW_ROOT_DIR}")

# Hide glfw's symbols when building a shared lib
if (BUILD_SHARED_LIBS)
  set_property(TARGET glfw PROPERTY C_VISIBILITY_PRESET hidden)
endif()

include_directories(BEFORE SYSTEM "${GLFW_ROOT_DIR}/include")

set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
