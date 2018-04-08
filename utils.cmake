# All sorts of things that we need cross project
cmake_minimum_required(VERSION 2.8.0)

add_definitions("-DRAYLIB_CMAKE=1")

# Linking for OS X -framework options
# Will do nothing on other OSes
if(APPLE)
  find_library(OPENGL_LIBRARY OpenGL)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(COREVIDEO_LIBRARY CoreVideo)

  set(LIBS_PRIVATE ${OPENGL_LIBRARY} ${COCOA_LIBRARY}
                   ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREVIDEO_LIBRARY})
elseif(WIN32)
  # no pkg-config --static on Windows yet...
else()
  if(USE_WAYLAND)
    set(_GLFW_WAYLAND 1)
  else()
    set(_GLFW_X11 1)
  endif()

  find_library(pthread NAMES pthread)
  find_package(OpenGL QUIET)
  if ("${OPENGL_LIBRARIES}" STREQUAL "")
    if(NOT USE_WAYLAND)
      # CFLAGS=-m32 cmake on Linux fails for some reason, so fallback to hardcoding
      set(LIBS_PRIVATE m pthread GL X11 Xrandr Xinerama Xi Xxf86vm Xcursor)
    else()
      # CFLAGS=-m32 cmake on Linux fails for some reason, so fallback to hardcoding
      set(LIBS_PRIVATE m pthread GL wayland-client wayland-cursor wayland-egl)
    endif()
  else()
    if(NOT USE_WAYLAND)
      find_package(X11 REQUIRED X11)
      find_library(XRANDR_LIBRARY Xrandr)
      find_library(XI_LIBRARY Xi)
      find_library(XINERAMA_LIBRARY Xinerama)
      find_library(XXF86VM_LIBRARY Xxf86vm)
      find_library(XCURSOR_LIBRARY Xcursor)
    else()
      find_library(WAYLAND_CLIENT_LIBRARY wayland-client)
      find_library(WAYLAND_CURSOR_LIBRARY wayland-cursor)
      find_library(WAYLAND_EGL_LIBRARY wayland-egl)
    endif()

    include_directories(${OPENGL_INCLUDE_DIR})

    if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD")
      find_library(OSS_LIBRARY ossaudio)
    endif()

    if(NOT USE_WAYLAND)
      set(LIBS_PRIVATE m ${pthread} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${XRANDR_LIBRARY} ${XINERAMA_LIBRARY} ${XI_LIBRARY} ${XXF86VM_LIBRARY} ${XCURSOR_LIBRARY} ${OSS_LIBRARY})
    else()
      set(LIBS_PRIVATE m ${pthread} ${OPENGL_LIBRARIES} ${WAYLAND_CLIENT_LIBRARY} ${WAYLAND_CURSOR_LIBRARY} ${WAYLAND_EGL_LIBRARY} ${OSS_LIBRARY})
    endif()
  endif()
endif()

if(USE_EXTERNAL_GLFW STREQUAL "ON")
    find_package(glfw3 3.2.1 REQUIRED)
elseif(USE_EXTERNAL_GLFW STREQUAL "IF_POSSIBLE")
    find_package(glfw3 3.2.1 QUIET)
endif()
if (glfw3_FOUND)
  set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(LINUX TRUE)
  set(LIBS_PRIVATE dl ${LIBS_PRIVATE})
endif()

foreach(L ${LIBS_PRIVATE})
  get_filename_component(DIR  ${L} PATH)
  get_filename_component(LIBFILE ${L} NAME_WE)
  STRING(REGEX REPLACE "^lib" "" FILE ${LIBFILE})

  if (${L} MATCHES "[.]framework$")
      set(FILE_OPT "-framework ${FILE}")
      set(DIR_OPT  "-F${DIR}")
  else()
      set(FILE_OPT "-l${FILE}")
      set(DIR_OPT  "-L${DIR}")
  endif()

  if ("${DIR}" STREQUAL "" OR "${DIR}" STREQUAL "${LASTDIR}")
      set (DIR_OPT "")
  endif()

  set(LASTDIR ${DIR})

  set(__PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE} ${DIR_OPT} ${FILE_OPT})
  string (REPLACE ";" " " __PKG_CONFIG_LIBS_PRIVATE "${__PKG_CONFIG_LIBS_PRIVATE}")
endforeach(L)



# Do the linking for executables that are meant to link raylib
function(link_libraries_to_executable executable)
  # Link raylib
  if (TARGET raylib_shared)
    target_link_libraries(${executable} raylib_shared)
  else()
    target_link_libraries(${executable} raylib ${__PKG_CONFIG_LIBS_PRIVATE})
  endif()
endfunction()
