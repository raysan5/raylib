# Setup the project and settings
project(raylib C)
include(GNUInstallDirs)
include(JoinPaths)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(PROJECT_VERSION 3.5.0)
set(API_VERSION 351)

include("CMakeOptions.txt")
include(BuildType)
configure_file(config.h.in ${CMAKE_BINARY_DIR}/cmake/config.h)
include_directories(${CMAKE_BINARY_DIR} .)

# Get the sources together
file(GLOB raylib_sources *.c)
list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/rglfw.c)

if(USE_EXTERNAL_GLFW STREQUAL "ON")
  find_package(glfw3 3.2.1 REQUIRED)
elseif(USE_EXTERNAL_GLFW STREQUAL "IF_POSSIBLE")
  find_package(glfw3 3.2.1 QUIET)
endif()
if (glfw3_FOUND)
  set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
endif()

# Explicitly check against "ON", because USE_EXTERNAL_GLFW is a tristate option
if(NOT glfw3_FOUND AND NOT USE_EXTERNAL_GLFW STREQUAL "ON" AND "${PLATFORM}" MATCHES "Desktop")
  MESSAGE(STATUS "Using raylib's GLFW")
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  set(GLFW_USE_WAYLAND ${USE_WAYLAND} CACHE BOOL "" FORCE)

  add_subdirectory(external/glfw)

  list(APPEND raylib_sources $<TARGET_OBJECTS:glfw_objlib>)
  include_directories(BEFORE SYSTEM external/glfw/include)
else()
  MESSAGE(STATUS "Using external GLFW")
  set(GLFW_PKG_DEPS glfw3)
endif()

add_definitions("-DRAYLIB_CMAKE=1")

if(USE_AUDIO)
  MESSAGE(STATUS "Audio Backend: miniaudio")
  set(sources ${raylib_sources})
else()
  MESSAGE(STATUS "Audio Backend: None (-DUSE_AUDIO=OFF)")
  set(RAYLIB_MODULE_AUDIO 0)
  list(REMOVE_ITEM raylib_sources ${CMAKE_CURRENT_SOURCE_DIR}/raudio.c)
  set(sources ${raylib_sources})
endif()

### Config options ###
# Translate the config options to what raylib wants
if(${PLATFORM} MATCHES "Desktop")
  set(PLATFORM_CPP "PLATFORM_DESKTOP")

  if(APPLE)
    # Need to force OpenGL 3.3 on OS X
    # See: https://github.com/raysan5/raylib/issues/341
    set(GRAPHICS "GRAPHICS_API_OPENGL_33")
    find_library(OPENGL_LIBRARY OpenGL)
    set(LIBS_PRIVATE ${OPENGL_LIBRARY})
    link_libraries("${LIBS_PRIVATE}")
    if (NOT CMAKE_SYSTEM STRLESS "Darwin-18.0.0")
      add_definitions(-DGL_SILENCE_DEPRECATION)
      MESSAGE(AUTHOR_WARNING "OpenGL is deprecated starting with macOS 10.14 (Mojave)!")
    endif()
  elseif(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(LIBS_PRIVATE ${LIBS_PRIVATE} winmm)
  else()
    find_library(pthread NAMES pthread)
    find_package(OpenGL QUIET)
    if ("${OPENGL_LIBRARIES}" STREQUAL "")
      set(OPENGL_LIBRARIES "GL")
    endif()

    if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD")
      find_library(OSS_LIBRARY ossaudio)
    endif()

    set(LIBS_PRIVATE m pthread ${OPENGL_LIBRARIES} ${OSS_LIBRARY})
  endif()

elseif(${PLATFORM} MATCHES "Web")
  set(PLATFORM_CPP "PLATFORM_WEB")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

  set(CMAKE_C_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 --profiling")

  # Change the name of the output library

elseif(${PLATFORM} MATCHES "Android")
  set(PLATFORM_CPP "PLATFORM_ANDROID")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
  include(AddIfFlagCompiles)
  add_if_flag_compiles(-ffunction-sections CMAKE_C_FLAGS)
  add_if_flag_compiles(-funwind-tables CMAKE_C_FLAGS)
  add_if_flag_compiles(-fstack-protector-strong CMAKE_C_FLAGS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  add_if_flag_compiles(-Wa,--noexecstack CMAKE_C_FLAGS)
  add_if_flag_compiles(-no-canonical-prefixes CMAKE_C_FLAGS)
  add_definitions(-DANDROID -D__ANDROID_API__=21)
  include_directories(external/android/native_app_glue)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,libatomic.a -Wl,--build-id -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings -uANativeActivity_onCreate")

  find_library(OPENGL_LIBRARY OpenGL)
  set(LIBS_PRIVATE m log android EGL GLESv2 OpenSLES atomic c)

elseif(${PLATFORM} MATCHES "Raspberry Pi")
  set(PLATFORM_CPP "PLATFORM_RPI")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

  add_definitions(-D_DEFAULT_SOURCE)
  
  find_library(GLESV2 brcmGLESv2 HINTS /opt/vc/lib)
  find_library(EGL brcmEGL HINTS /opt/vc/lib)
  find_library(BCMHOST bcm_host HINTS /opt/vc/lib)
  include_directories(/opt/vc/include /opt/vc/include/interface/vmcs_host/linux /opt/vc/include/interface/vcos/pthreads)
  link_directories(/opt/vc/lib)
  set(LIBS_PRIVATE ${GLESV2} ${EGL} ${BCMHOST} pthread rt m dl)

  elseif(${PLATFORM} MATCHES "DRM")
  set(PLATFORM_CPP "PLATFORM_DRM")
  set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")

  add_definitions(-D_DEFAULT_SOURCE)
  add_definitions(-DEGL_NO_X11)
  add_definitions(-DPLATFORM_DRM)
      
  find_library(GLESV2 GLESv2)
  find_library(EGL EGL)
  find_library(DRM drm)
  find_library(GBM gbm)

  include_directories(/usr/include/libdrm)
  set(LIBS_PRIVATE ${GLESV2} ${EGL} ${DRM} ${GBM} pthread m dl)

endif()

if (${OPENGL_VERSION})
  set(${SUGGESTED_GRAPHICS} "${GRAPHICS}")
  if (${OPENGL_VERSION} MATCHES "3.3")
    set(GRAPHICS "GRAPHICS_API_OPENGL_33")
  elseif (${OPENGL_VERSION} MATCHES "2.1")
    set(GRAPHICS "GRAPHICS_API_OPENGL_21")
  elseif (${OPENGL_VERSION} MATCHES "1.1")
    set(GRAPHICS "GRAPHICS_API_OPENGL_11")
  elseif (${OPENGL_VERSION} MATCHES "ES 2.0")
    set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
  endif()
  if ("${SUGGESTED_GRAPHICS}" AND NOT "${SUGGESTED_GRAPHICS}" STREQUAL "${GRAPHICS}")
    message(WARNING "You are overriding the suggested GRAPHICS=${SUGGESTED_GRAPHICS} with ${GRAPHICS}! This may fail")
  endif()
endif()

if(NOT GRAPHICS)
  set(GRAPHICS "GRAPHICS_API_OPENGL_33")
endif()

include_directories(${OPENGL_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR})
set(LIBS_PRIVATE ${LIBS_PRIVATE} ${OPENAL_LIBRARY})
include(LibraryPathToLinkerFlags)
library_path_to_linker_flags(__PKG_CONFIG_LIBS_PRIVATE "${LIBS_PRIVATE}")

if(STATIC)
  MESSAGE(STATUS "Building raylib static library")
  if(${PLATFORM} MATCHES "Web")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
  endif()

  add_library(raylib_static STATIC ${sources})

  target_compile_definitions(raylib_static
      PUBLIC ${PLATFORM_CPP}
      PUBLIC PLATFORM=${PLATFORM_CPP}
      PUBLIC ${GRAPHICS}
      PUBLIC GRAPHICS=${GRAPHICS}
  )

  target_link_libraries(raylib_static
    PUBLIC ${LIBS_PRIVATE}
  )

  set(PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE} ${GLFW_PKG_LIBS})
  string (REPLACE ";" " " PKG_CONFIG_LIBS_PRIVATE "${PKG_CONFIG_LIBS_PRIVATE}")
  if (${PLATFORM} MATCHES "Desktop")
    target_link_libraries(raylib_static PRIVATE glfw ${GLFW_LIBRARIES} ${LIBS_PRIVATE})
  endif()

  if (WITH_PIC)
    set_property(TARGET raylib_static PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()
  set_target_properties(raylib_static PROPERTIES PUBLIC_HEADER "raylib.h")
  if(NOT MSVC) # Keep lib*.(a|dll) name, but avoid *.lib files overwriting each other on Windows
    set_target_properties(raylib_static PROPERTIES OUTPUT_NAME raylib)
  endif()
  install(
      TARGETS raylib_static
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
  set_target_properties(raylib_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")

  add_test("pkg-config--static" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh --static)
endif(STATIC)


if(SHARED)
  MESSAGE(STATUS "Building raylib shared library")
  add_library(raylib SHARED ${sources})

  target_compile_definitions(raylib
      PUBLIC ${PLATFORM_CPP}
      PUBLIC ${GRAPHICS}
  )

  if(MSVC)
    target_compile_definitions(raylib
      PRIVATE $<BUILD_INTERFACE:BUILD_LIBTYPE_SHARED>
      INTERFACE $<INSTALL_INTERFACE:USE_LIBTYPE_SHARED>
    )
  endif()

  set(PKG_CONFIG_LIBS_EXTRA "")

  set_property(TARGET raylib PROPERTY POSITION_INDEPENDENT_CODE ON)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_LIBDIR}")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_MACOSX_RPATH ON)

  target_link_libraries(raylib ${LIBS_PRIVATE})
  if (${PLATFORM} MATCHES "Desktop")
    target_link_libraries(raylib glfw)
  endif()
  set_target_properties(raylib PROPERTIES
      PUBLIC_HEADER "raylib.h"
      VERSION   ${PROJECT_VERSION}
      SOVERSION ${API_VERSION}
  )

  if (WIN32)
    install(
      TARGETS raylib
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
      PUBLIC_HEADER DESTINATION "include"
    )
  else()
    install(
        TARGETS raylib
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
  endif()
  set_target_properties(raylib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")

  add_test("pkg-config" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh)
else(SHARED)
  add_library(raylib ALIAS raylib_static)
endif(SHARED)

if (NOT DEFINED PKG_CONFIG_LIBS_EXTRA)
  set(PKG_CONFIG_LIBS_EXTRA "${PKG_CONFIG_LIBS_PRIVATE}")
endif()

join_paths(libdir_for_pc_file "\${exec_prefix}" "${CMAKE_INSTALL_LIBDIR}")
join_paths(includedir_for_pc_file "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}")

configure_file(../raylib.pc.in raylib.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/raylib.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
configure_file(../cmake/raylib-config-version.cmake raylib-config-version.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/raylib-config-version.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raylib")
install(FILES ${PROJECT_SOURCE_DIR}/../cmake/raylib-config.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/raylib")

# populates raylib_{FOUND, INCLUDE_DIRS, LIBRARIES, LDFLAGS, DEFINITIONS}
include(PopulateConfigVariablesLocally)
populate_config_variables_locally(raylib)

# Copy the header files to the build directory
file(COPY "raylib.h" DESTINATION ".")
file(COPY "rlgl.h" DESTINATION ".")
file(COPY "physac.h" DESTINATION ".")
file(COPY "raymath.h" DESTINATION ".")
file(COPY "raudio.h" DESTINATION ".")

# Also install them
install(FILES "raylib.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "rlgl.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "physac.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "raymath.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "raudio.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# Print the flags for the user
if (DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Generated build type: ${CMAKE_BUILD_TYPE}")
else()
  message(STATUS "Generated config types: ${CMAKE_CONFIGURATION_TYPES}")
endif()
message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM_CPP})
message(STATUS "  GRAPHICS=" ${GRAPHICS})

# Packaging
SET(CPACK_PACKAGE_NAME "raylib")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple and easy-to-use library to enjoy videogames programming")
SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/../README.md")
SET(CPACK_RESOURCE_FILE_WELCOME "${PROJECT_SOURCE_DIR}/../README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/../LICENSE")
SET(CPACK_PACKAGE_FILE_NAME "raylib-${PROJECT_VERSION}$ENV{RAYLIB_PACKAGE_SUFFIX}")
SET(CPACK_GENERATOR "ZIP;TGZ") # Remove this, if you want the NSIS installer on Windows
include(CPack)

enable_testing()
