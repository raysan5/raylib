# All sorts of things that we need cross project
cmake_minimum_required(VERSION 2.8.0)

# Detect linux
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# Linking for OS X -framework options
# Will do nothing on other OSes
if(APPLE)
  find_library(OPENGL_LIBRARY OpenGL)
  find_library(OPENAL_LIBRARY OpenAL)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(COREVIDEO_LIBRARY CoreVideo)

  set(LIBS_PRIVATE ${OPENGL_LIBRARY} ${OPENAL_LIBRARY} ${COCOA_LIBRARY}
                   ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREVIDEO_LIBRARY})
elseif(LINUX)
  # Elsewhere (such as Linux), need `-lopenal -lGL`, etc...
  set(LIBS_PRIVATE
      m pthread dl
      openal
      GL
      X11 Xrandr Xinerama Xi Xxf86vm Xcursor)  # X11 stuff
else()
  # TODO Windows
endif()

# Do the linking for executables that are meant to link raylib
function(link_libraries_to_executable executable)
  # And raylib
  target_link_libraries(${executable} raylib)

  # Link the libraries
  target_link_libraries(${executable} ${LIBS_PRIVATE})
endfunction()

