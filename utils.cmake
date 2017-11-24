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

  set(LIBS_PRIVATE ${OPENGL_LIBRARY} ${OPENAL_LIBRARY} ${COCOA_LIBRARY})
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
  # Link the libraries
  target_link_libraries(${executable} ${LIBS_PRIVATE})
  
  # And raylib
  target_link_libraries(${executable} raylib)
endfunction()

