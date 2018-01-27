# All sorts of things that we need cross project
cmake_minimum_required(VERSION 2.8.0)

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
  find_library(pthread NAMES pthread)
  find_package(OpenGL)
  if ("${OPENGL_LIBRARIES}" STREQUAL "")
    # CFLAGS=-m32 cmake on Linux fails for some reason, so fallback to hardcoding
    set(LIBS_PRIVATE m pthread GL X11 Xrandr Xinerama Xi Xxf86vm Xcursor)
  else()
    find_package(X11 REQUIRED X11)
    find_library(XRANDR_LIBRARY Xrandr)
    find_library(XI_LIBRARY Xi)
    find_library(XINERAMA_LIBRARY Xinerama)
    find_library(XXF86VM_LIBRARY Xxf86vm)
    find_library(XCURSOR_LIBRARY Xcursor)

    include_directories(${OPENGL_INCLUDE_DIR})

    set(LIBS_PRIVATE m ${pthread} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${XRANDR_LIBRARY} ${XINERAMA_LIBRARY} ${XI_LIBRARY} ${XXF86VM_LIBRARY} ${XCURSOR_LIBRARY})
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(LINUX TRUE)
  set(LIBS_PRIVATE dl ${LIBS_PRIVATE})
endif()

# Do the linking for executables that are meant to link raylib
function(link_libraries_to_executable executable)
  # And raylib
  target_link_libraries(${executable} raylib)

  # Link the libraries
  target_link_libraries(${executable} ${LIBS_PRIVATE})
endfunction()

