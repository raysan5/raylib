# Setup the project and settings
project(raylib)
include("../utils.cmake")

set(raylib_VERSION_MAJOR 1)
set(raylib_VERSION_MINOR 8)
set(RAYLIB raylib)    # Name of the generated library


### Config options ###
# Build a static or shared raylib?
set(SHARED_RAYLIB OFF CACHE BOOL "Build raylib as a dynamic library")

# Platform
set(PLATFORM "Desktop" CACHE STRING "Platform to build for.")
set_property(CACHE PLATFORM PROPERTY STRINGS "Desktop" "Web" "Android" "Raspberry Pi")

# OpenGL version
set(OPENGL_VERSION "3.3" CACHE STRING "OpenGL Version to build raylib with")
set_property(CACHE OPENGL_VERSION PROPERTY STRINGS "3.3" "2.1" "1.1" "ES 2.0")
### Config options ###


# Translate the config options to what raylib wants
if(${PLATFORM} MATCHES "Desktop")
  set(PLATFORM "PLATFORM_DESKTOP")

  # OpenGL version
  if (${OPENGL_VERSION} MATCHES "3.3")
    set(GRAPHICS "GRAPHICS_API_OPENGL_33")
  elseif (${OPENGL_VERSION} MATCHES "2.1")
    set(GRAPHICS "GRAPHICS_API_OPENGL_21")
  elseif (${OPENGL_VERSION} MATCHES "1.1")
    set(GRAPHICS "GRAPHICS_API_OPENGL_11")
  elseif (${OPENGL_VERSION} MATCHES "ES 2.0")
    set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
  endif()

  # Need to force OpenGL 3.3 on OS X
  # See: https://github.com/raysan5/raylib/issues/341
  if(APPLE)
    set(GRAPHICS "GRAPHICS_API_OPENGL_33")
  endif()
elseif(${PLATFORM} MATCHES "Web")
  set(PLATFORM "PLATFORM_WEB")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

  # Need to use `emcc`
  set(CMAKE_C_COMPILER "emcc")
  set(CMAKE_CXX_COMPILER "em++")

  # Change the name of the output library
  set(RAYLIB "libraylib.bc")

elseif(${PLATFORM} MATCHES "Android")
  set(PLATFORM "PLATFORM_ANDROID")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
elseif(${PLATFORM} MATCHES "Raspberry Pi")
  set(PLATFORM "PLATFORM_RPI")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
endif()

# Get the sources together
file(GLOB raylib_sources *.c)
file(GLOB stb_vorbis external/stb_vorbis.c)
set(sources ${raylib_sources} ${stb_vorbis})

# Which platform?
if(${PLATFORM} MATCHES "PLATFORM_DESKTOP")
  # Build a static or shared raylib?
  # TODO clean this up a bit?
  if(${SHARED_RAYLIB})
    # Shared library
    add_library(${RAYLIB} SHARED ${sources}) 
  
    # Will link -framework (if on OS X)
    link_os_x_frameworks(raylib)
  else()
    # Static library
    add_library(${RAYLIB} STATIC ${sources}) 
  
    if(LINUX)
      # On Linux, need to link a few extra things for static
      target_link_libraries(${RAYLIB} m pthread dl)
      target_link_libraries(${RAYLIB} X11 Xrandr Xinerama Xi Xxf86vm Xcursor)  # X11 stuff
    endif()
  endif()
  
  # Always need to link OpenAL and OpenGL
  if(LINUX)
    # Elsewhere (such as Linux), need `-lopenal -lGL`
    target_link_libraries(${RAYLIB} openal)
    target_link_libraries(${RAYLIB} GL)
  endif()
  
  # Add in GLFW as a linking target
  target_link_libraries(${RAYLIB} glfw)
  
  # Library file & Header
  set_target_properties(${RAYLIB} PROPERTIES PUBLIC_HEADER "raylib.h")
  install(
    TARGETS ${RAYLIB}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
  )
  
  # Copy the header files to the build directory
  file(COPY "raylib.h" DESTINATION ".")
  file(COPY "rlgl.h" DESTINATION ".")
  file(COPY "physac.h" DESTINATION ".")
  file(COPY "raymath.h" DESTINATION ".")
  file(COPY "audio.h" DESTINATION ".")
elseif(${PLATFORM} MATCHES "PLATFORM_WEB")
  # For the web.
  add_executable(${RAYLIB} ${sources})
endif()


# Set the compile flags to raylib
target_compile_definitions(${RAYLIB}
  PUBLIC ${PLATFORM}
  PUBLIC ${GRAPHICS}
)



# Print the flags for the user
message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM})
message(STATUS "  GRAPHICS=" ${GRAPHICS})

