# Setup the project and settings
project(raylib C)
include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(PROJECT_VERSION 2.0.0)
set(API_VERSION 2)
set(RAYLIB raylib)    # Name of the generated library

include("CMakeOptions.txt")
include(BuildType)
configure_file(config.h.in ${CMAKE_BINARY_DIR}/cmake/config.h)
include_directories(${CMAKE_BINARY_DIR})
include_directories(SYSTEM .)

# Get the sources together
file(GLOB raylib_sources *.c)
list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/rglfw.c)

# Explicitly check against "ON", because USE_EXTERNAL_GLFW is a tristate option
if(NOT glfw3_FOUND AND NOT USE_EXTERNAL_GLFW STREQUAL "ON" AND "${PLATFORM}" MATCHES "Desktop")
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  set(GLFW_USE_WAYLAND ${USE_WAYLAND} CACHE BOOL "" FORCE)
  if (WITH_PIC OR SHARED)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

  add_subdirectory(external/glfw)
  include_directories(external/glfw/include)

  list(APPEND raylib_sources $<TARGET_OBJECTS:glfw_objlib>)
else()
  set(GLFW_PKG_DEPS glfw)
endif()

include(utils)

if(USE_AUDIO)
  file(GLOB stb_vorbis external/stb_vorbis.c)
  file(GLOB mini_al external/mini_al.c ${stb_vorbis})
  set(sources ${raylib_sources} ${mini_al})
else()
  set(INCLUDE_AUDIO_MODULE 0)
  list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio.c)
  set(sources ${raylib_sources})
endif()

include(AddIfFlagCompiles)

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

  if(APPLE)
    # Need to force OpenGL 3.3 on OS X
    # See: https://github.com/raysan5/raylib/issues/341
    set(GRAPHICS "GRAPHICS_API_OPENGL_33")
    link_libraries("${LIBS_PRIVATE}")
  elseif(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif()

elseif(${PLATFORM} MATCHES "Web")
  set(PLATFORM "PLATFORM_WEB")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

  set(CMAKE_C_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 --profiling")

  # Change the name of the output library

elseif(${PLATFORM} MATCHES "Android")
  set(PLATFORM "PLATFORM_ANDROID")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
  add_if_flag_compiles(-ffunction-sections CMAKE_C_FLAGS)
  add_if_flag_compiles(-funwind-tables CMAKE_C_FLAGS)
  add_if_flag_compiles(-fstack-protector-strong CMAKE_C_FLAGS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  add_if_flag_compiles(-Wa,--noexecstack CMAKE_C_FLAGS)
  add_if_flag_compiles(-no-canonical-prefixes CMAKE_C_FLAGS)
  add_definitions(-DANDROID -D__ANDROID_API__=21)
  include_directories(external/android/native_app_glue)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,libatomic.a -Wl,--build-id -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings -uANativeActivity_onCreate")

elseif(${PLATFORM} MATCHES "Raspberry Pi")
  set(PLATFORM "PLATFORM_RPI")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
endif()

if(${SHARED})
  add_library(${RAYLIB}_shared SHARED ${sources})

  target_compile_definitions(${RAYLIB}_shared
      PUBLIC ${PLATFORM}
      PUBLIC ${GRAPHICS}
  )

  set(PKG_CONFIG_LIBS_EXTRA "")

  set_property(TARGET ${RAYLIB}_shared PROPERTY POSITION_INDEPENDENT_CODE ON)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_MACOSX_RPATH ON)

  target_link_libraries(${RAYLIB}_shared ${LIBS_PRIVATE})
  if (${PLATFORM} MATCHES "PLATFORM_DESKTOP")
    target_link_libraries(${RAYLIB}_shared glfw)
  endif()
  if (UNIX AND ${FILESYSTEM_LACKS_SYMLINKS})
    MESSAGE(WARNING "Can't version UNIX shared library on file system without symlink support")
  else()
      set_target_properties(${RAYLIB}_shared PROPERTIES
          VERSION   ${PROJECT_VERSION}
          SOVERSION ${API_VERSION}
      )
  endif()
  set_target_properties(${RAYLIB}_shared PROPERTIES
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

  add_test("pkg-config" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh)
endif(${SHARED})

if(${STATIC})
  if(${PLATFORM} MATCHES "PLATFORM_WEB")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
  endif()

  add_library(${RAYLIB} STATIC ${sources})

  target_compile_definitions(${RAYLIB}
      PUBLIC ${PLATFORM}
      PUBLIC ${GRAPHICS}
  )

  set(PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE} ${GLFW_PKG_LIBS})
  string (REPLACE ";" " " PKG_CONFIG_LIBS_PRIVATE "${PKG_CONFIG_LIBS_PRIVATE}")
  if (${PLATFORM} MATCHES "PLATFORM_DESKTOP")
    target_link_libraries(${RAYLIB} glfw ${GLFW_LIBRARIES})
  endif()

  if (WITH_PIC)
    set_property(TARGET ${RAYLIB} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()
  set_target_properties(${RAYLIB} PROPERTIES PUBLIC_HEADER "raylib.h")
  install(TARGETS ${RAYLIB}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )

  add_test("pkg-config--static" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh --static)
endif(${STATIC})

if (NOT DEFINED PKG_CONFIG_LIBS_EXTRA)
  set(PKG_CONFIG_LIBS_EXTRA "${PKG_CONFIG_LIBS_PRIVATE}")
endif()
configure_file(../raylib.pc.in raylib.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/release/raylib.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# Copy the header files to the build directory
file(COPY "raylib.h" DESTINATION ".")
file(COPY "rlgl.h" DESTINATION ".")
file(COPY "physac.h" DESTINATION ".")
file(COPY "raymath.h" DESTINATION ".")
file(COPY "audio.h" DESTINATION ".")

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

enable_testing()
