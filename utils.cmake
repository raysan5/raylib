# All sorts of things that we need cross project
cmake_minimum_required(VERSION 2.8.0)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
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
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  # Elsewhere (such as Linux), need `-lopenal -lGL`, etc...
  set(LIBS_PRIVATE
      m pthread dl
      GL
      X11 Xrandr Xinerama Xi Xxf86vm Xcursor)  # X11 stuff
endif()

if(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
   find_package(OpenGL REQUIRED)
   find_package(OpenAL REQUIRED)
   include_directories(${OPENGL_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR})

   find_package(X11 REQUIRED)
   find_library(OpenAL REQUIRED)
   find_library(pthread NAMES pthread)
   find_library(Xrandr NAMES Xrandr)
   find_library(Xi NAMES Xi)
   find_library(Xinerama NAMES Xinerama)
   find_library(Xxf86vm NAMES Xxf86vm)
   find_library(Xcursor NAMES Xcursor)

   set(LIBS_PRIVATE m ${pthread} ${OPENAL_LIBRARY} ${X11_LIBRARIES} ${Xrandr} ${Xinerama} ${Xi} ${Xxf86vm} ${Xcursor})
endif()

# TODO Support Windows

# Do the linking for executables that are meant to link raylib
function(link_libraries_to_executable executable)
  # And raylib
  target_link_libraries(${executable} raylib)

  # Link the libraries
  target_link_libraries(${executable} ${LIBS_PRIVATE})
endfunction()

