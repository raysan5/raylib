# Setup the project and settings
project(raylib)
include("../utils.cmake")
include(GNUInstallDirs)

set(PROJECT_VERSION 1.9.4)
set(API_VERSION 1)
set(RAYLIB raylib)    # Name of the generated library

### Config options ###
# Shared library is always PIC. Static library should be PIC too if linked into a shared library
option(WITH_PIC "Compile static library as position-independent code" OFF)
# Build a static and/or shared raylib?
option(SHARED "Build raylib as a dynamic library" OFF)
option(STATIC "Build raylib as a static library" ON)
option(USE_AUDIO "Build raylib with audio module" ON)
option(MACOS_FATLIB  "Build fat library for both i386 and x86_64 on macOS" ON)

if(NOT (STATIC OR SHARED))
  message(FATAL_ERROR "Nothing to do if both -DSHARED=OFF and -DSTATIC=OFF...")
endif()

if(DEFINED SHARED_RAYLIB)
  set(SHARED ${SHARED_RAYLIB})
  message(DEPRECATION "-DSHARED_RAYLIB is deprecated. Please use -DSHARED instead.")
endif()
if(DEFINED STATIC_RAYLIB)
  set(STATIC ${STATIC_RAYLIB})
  message(DEPRECATION "-DSTATIC_RAYLIB is deprecated. Please use -DSTATIC instead.")
endif()

# Platform
set(PLATFORM "Desktop" CACHE STRING "Platform to build for.")
set_property(CACHE PLATFORM PROPERTY STRINGS "Desktop" "Web" "Android" "Raspberry Pi")

# OpenGL version
set(OPENGL_VERSION "3.3" CACHE STRING "OpenGL Version to build raylib with")
set_property(CACHE OPENGL_VERSION PROPERTY STRINGS "3.3" "2.1" "1.1" "ES 2.0")

# Get the sources together
file(GLOB raylib_sources *.c)
if(glfw3_FOUND)
  list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/rglfw.c)
else()
  include_directories(external/glfw/include)
endif()

if(USE_AUDIO)
  file(GLOB stb_vorbis external/stb_vorbis.c)
  file(GLOB mini_al external/mini_al.c ${stb_vorbis})
  set(sources ${raylib_sources} ${mini_al})
else()
  set(INCLUDE_AUDIO_MODULE 0) 
  list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio.c)
  set(sources ${raylib_sources})
endif()

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
    set_source_files_properties(rglfw.c PROPERTIES COMPILE_FLAGS "-x objective-c")
    link_libraries("${LIBS_PRIVATE}")
  elseif(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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

if(MACOS_FATLIB)
    if (CMAKE_OSX_ARCHITECTURES)
        message(FATAL_ERROR "User supplied -DCMAKE_OSX_ARCHITECTURES overrides -DMACOS_FATLIB=ON")
    else()
        SET(CMAKE_OSX_ARCHITECTURES "x86_64;i386")
    endif()
endif()

# Which platform?
if(${PLATFORM} MATCHES "PLATFORM_DESKTOP")

  if(${SHARED})
    add_library(${RAYLIB}_shared SHARED ${sources})

    target_compile_definitions(${RAYLIB}_shared
        PUBLIC ${PLATFORM}
        PUBLIC ${GRAPHICS}
    )

    set_property(TARGET ${RAYLIB}_shared PROPERTY POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_MACOSX_RPATH ON)

    target_link_libraries(${RAYLIB}_shared ${LIBS_PRIVATE})
    set_target_properties(${RAYLIB}_shared PROPERTIES
        VERSION   ${PROJECT_VERSION}
        SOVERSION ${API_VERSION}
        PUBLIC_HEADER "raylib.h"
    )
    if(WIN32)
      install(
        TARGETS ${RAYLIB}_shared
        RUNTIME DESTINATION lib
        PUBLIC_HEADER DESTINATION include
      )
    else() # Keep lib*.(a|dll) name, but avoid *.lib files overwriting each other on Windows
      set_target_properties(${RAYLIB}_shared PROPERTIES OUTPUT_NAME ${RAYLIB})
      install(
        TARGETS ${RAYLIB}_shared
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
      )
    endif()
  endif(${SHARED})

  if(${STATIC})
    add_library(${RAYLIB} STATIC ${sources})

    target_compile_definitions(${RAYLIB}
        PUBLIC ${PLATFORM}
        PUBLIC ${GRAPHICS}
    )

    set(PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE})

    if (WITH_PIC)
      set_property(TARGET ${RAYLIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
    set_target_properties(${RAYLIB} PROPERTIES PUBLIC_HEADER "raylib.h")
    install(TARGETS ${RAYLIB}
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
  endif(${STATIC})

  configure_file(../raylib.pc.in raylib.pc @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/release/raylib.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

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

# Print the flags for the user
message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM})
message(STATUS "  GRAPHICS=" ${GRAPHICS})

# Packaging
SET(CPACK_PACKAGE_NAME "raylib")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple and easy-to-use library to learn videogames programming")
SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/../README.md")
SET(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/../README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/../LICENSE.md")
SET(CPACK_PACKAGE_FILE_NAME "raylib-${PROJECT_VERSION}$ENV{RAYLIB_PACKAGE_SUFFIX}")
SET(CPACK_GENERATOR "ZIP;TGZ") # Remove this, if you want the NSIS installer on Windows
include(CPack)
