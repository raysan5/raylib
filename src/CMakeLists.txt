# Setup the project and settings
project(raylib C)
set(PROJECT_VERSION 3.5.0)
set(API_VERSION 351)

include(GNUInstallDirs)
include(JoinPaths)

# Sets build type if not set by now
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    if(RAYLIB_IS_MAIN)
        set(default_build_type Debug)
    endif()

    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Get the sources together
set(raylib_public_headers
    raylib.h
    rlgl.h
    physac.h
    raymath.h
    raudio.h
    )

set(raylib_sources
    core.c
    models.c
    shapes.c
    text.c
    textures.c
    utils.c
    )

# cmake/GlfwImport.cmake handles the details around the inclusion of glfw
include(GlfwImport)


if (USE_AUDIO)
    MESSAGE(STATUS "Audio Backend: miniaudio")
    list(APPEND raylib_sources raudio.c)
else ()
    MESSAGE(STATUS "Audio Backend: None (-DUSE_AUDIO=OFF)")
endif ()

include(LibraryConfigurations)

set(LIBS_PRIVATE ${LIBS_PRIVATE} ${OPENAL_LIBRARY})

add_library(raylib ${raylib_sources} ${raylib_public_headers})

if (NOT BUILD_SHARED_LIBS)
    MESSAGE(STATUS "Building raylib static library")
    add_library(raylib_static ALIAS raylib)
    add_test("pkg-config--static" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh --static)
else()
    MESSAGE(STATUS "Building raylib shared library")
    if (MSVC)
        target_compile_definitions(raylib
                                   PRIVATE $<BUILD_INTERFACE:BUILD_LIBTYPE_SHARED>
                                   INTERFACE $<INSTALL_INTERFACE:USE_LIBTYPE_SHARED>
                                   )
    endif ()

    add_test("pkg-config" ${PROJECT_SOURCE_DIR}/../cmake/test-pkgconfig.sh)
endif()

# Setting target properties
set_target_properties(raylib PROPERTIES
                      PUBLIC_HEADER "${raylib_public_headers}"
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${API_VERSION}
                      )

if (WITH_PIC OR BUILD_SHARED_LIBS)
    set_property(TARGET raylib PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

# Linking libraries
target_link_libraries(raylib "${LIBS_PRIVATE}")
if (${PLATFORM} MATCHES "Desktop")
    target_link_libraries(raylib glfw)
endif ()

# Adding compile definitions
target_compile_definitions(raylib
                           PUBLIC "${PLATFORM_CPP}"
                           PUBLIC "${GRAPHICS}"
                           )

# Registering include directories
target_include_directories(raylib
                           PUBLIC
                           $<INSTALL_INTERFACE:include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_BINARY_DIR} # For cmake/config.h
                           ${OPENGL_INCLUDE_DIR}
                           ${OPENAL_INCLUDE_DIR}
                           )

# Copy the header files to the build directory for convenience
file(COPY ${raylib_public_headers} DESTINATION "include")

include(InstallConfigurations)

# Print the flags for the user
if (DEFINED CMAKE_BUILD_TYPE)
    message(STATUS "Generated build type: ${CMAKE_BUILD_TYPE}")
else ()
    message(STATUS "Generated config types: ${CMAKE_CONFIGURATION_TYPES}")
endif ()

message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM_CPP})
message(STATUS "  GRAPHICS=" ${GRAPHICS})

include(PackConfigurations)

enable_testing()
