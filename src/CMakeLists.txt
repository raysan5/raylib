# Setup the project and settings
project(raylib C)
set(PROJECT_VERSION 5.5.0)
set(API_VERSION 550)

include(GNUInstallDirs)
include(JoinPaths)

# Sets build type if not set by now
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    if(PROJECT_IS_TOP_LEVEL)
        set(default_build_type Debug)
    else()
        message(WARNING "Default build type is not set (CMAKE_BUILD_TYPE)")
    endif()

    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Used as public API to be included into other projects
set(raylib_public_headers
    raylib.h
    rcamera.h
    rlgl.h
    raymath.h
    )

# Sources to be compiled
set(raylib_sources
    raudio.c
    rcore.c
    rmodels.c
    rshapes.c
    rtext.c
    rtextures.c
    utils.c
    )

if (SUPPORT_VULKAN AND Vulkan_FOUND)
    list(APPEND raylib_sources rlvk.c)
    message(STATUS "Vulkan source (rlvk.c) added to compilation.")
endif()

# <root>/cmake/GlfwImport.cmake handles the details around the inclusion of glfw
if (NOT ${PLATFORM} MATCHES "Web")
    include(GlfwImport)
endif ()

# Sets additional platform options and link libraries for each platform
# also selects the proper graphics API and version for that platform
# Produces a variable LIBS_PRIVATE that will be used later
include(LibraryConfigurations)

if (SUPPORT_VULKAN AND Vulkan_FOUND)
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(raylib PUBLIC Vulkan::Vulkan)
        message(STATUS "Linking raylib with Vulkan::Vulkan target.")
    elseif(Vulkan_LIBRARIES)
        # For older CMake or non-imported target setups for Vulkan
        target_link_libraries(raylib PUBLIC ${Vulkan_LIBRARIES})
        message(STATUS "Linking raylib with Vulkan libraries: ${Vulkan_LIBRARIES}")
    else()
        message(WARNING "Vulkan support is enabled, but no Vulkan link target or libraries (Vulkan::Vulkan or Vulkan_LIBRARIES) were found/specified by find_package(Vulkan).")
    endif()
    # Ensure Vulkan include directories are available to the raylib target
    if (Vulkan_INCLUDE_DIRS)
        target_include_directories(raylib PUBLIC $<BUILD_INTERFACE:${Vulkan_INCLUDE_DIRS}>)
        message(STATUS "Adding Vulkan include directories: ${Vulkan_INCLUDE_DIRS}")
    endif()

    # Shader compilation for Vulkan
    find_program(GLSLANG_VALIDATOR NAMES glslangValidator glslangValidator.exe PATHS ENV PATH NO_DEFAULT_PATH DOC "Path to glslangValidator executable")

    if(NOT GLSLANG_VALIDATOR)
        message(WARNING "glslangValidator not found. Cannot compile Vulkan GLSL shaders to SPIR-V. Vulkan backend may not work correctly if precompiled shaders are not up-to-date or available.")
    else()
        message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR}")

        set(VULKAN_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Shaders are in src/
        set(SPIRV_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/spirv_shaders) # Output to build directory
        file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

        set(SHAPES_VERT_GLSL ${VULKAN_SHADER_DIR}/shapes_vert.glsl)
        set(SHAPES_FRAG_GLSL ${VULKAN_SHADER_DIR}/shapes_frag.glsl)

        set(SHAPES_VERT_SPV_H ${SPIRV_OUTPUT_DIR}/shapes_vert.spv.h)
        set(SHAPES_FRAG_SPV_H ${SPIRV_OUTPUT_DIR}/shapes_frag.spv.h)

        # Custom command to compile shapes_vert.glsl to shapes_vert.spv.h
        add_custom_command(
            OUTPUT ${SHAPES_VERT_SPV_H}
            COMMAND ${GLSLANG_VALIDATOR} -V -x shapes_vert_spv_data -o ${SHAPES_VERT_SPV_H} ${SHAPES_VERT_GLSL}
            DEPENDS ${SHAPES_VERT_GLSL}
            COMMENT "Compiling ${SHAPES_VERT_GLSL} to ${SHAPES_VERT_SPV_H}"
            VERBATIM)

        # Custom command to compile shapes_frag.glsl to shapes_frag.spv.h
        add_custom_command(
            OUTPUT ${SHAPES_FRAG_SPV_H}
            COMMAND ${GLSLANG_VALIDATOR} -V -x shapes_frag_spv_data -o ${SHAPES_FRAG_SPV_H} ${SHAPES_FRAG_GLSL}
            DEPENDS ${SHAPES_FRAG_GLSL}
            COMMENT "Compiling ${SHAPES_FRAG_GLSL} to ${SHAPES_FRAG_SPV_H}"
            VERBATIM)

        # Add generated headers to a custom target to ensure they are built
        add_custom_target(VulkanShaders ALL DEPENDS ${SHAPES_VERT_SPV_H} ${SHAPES_FRAG_SPV_H})

        # Add the output directory to include directories for raylib target
        # This allows rlvk.c to include the generated .spv.h files
        target_include_directories(raylib PRIVATE ${SPIRV_OUTPUT_DIR})

        # Make raylib target depend on VulkanShaders
        # This ensures shaders are compiled before raylib tries to compile rlvk.c
        add_dependencies(raylib VulkanShaders)
    endif()
endif()

if (SUPPORT_MODULE_RAUDIO)
    MESSAGE(STATUS "Audio Backend: miniaudio")
else ()
    MESSAGE(STATUS "Audio Backend: None (-DCUSTOMIZE_BUILD=ON -DSUPPORT_MODULE_RAUDIO=OFF)")
endif ()

add_library(raylib ${raylib_sources} ${raylib_public_headers})

if (NOT BUILD_SHARED_LIBS)
    MESSAGE(STATUS "Building raylib static library")
    add_library(raylib_static ALIAS raylib)
else()
    MESSAGE(STATUS "Building raylib shared library")
    target_compile_definitions(raylib
                               PRIVATE $<BUILD_INTERFACE:BUILD_LIBTYPE_SHARED>
                               INTERFACE $<INSTALL_INTERFACE:USE_LIBTYPE_SHARED>
                               )
endif()

if (${PLATFORM} MATCHES "Web")
    target_link_options(raylib PUBLIC "-sUSE_GLFW=3")
    if(${GRAPHICS} MATCHES "GRAPHICS_API_OPENGL_ES3")
        target_link_options(raylib PUBLIC "-sMIN_WEBGL_VERSION=2")
        target_link_options(raylib PUBLIC "-sMAX_WEBGL_VERSION=2")
    endif()
endif()

set_target_properties(raylib PROPERTIES
                      PUBLIC_HEADER "${raylib_public_headers}"
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${API_VERSION}
                      )

if (WITH_PIC OR BUILD_SHARED_LIBS)
    set_property(TARGET raylib PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

if (BUILD_SHARED_LIBS)
    # Hide raylib's symbols by default so RLAPI can expose them
    set_property(TARGET raylib PROPERTY C_VISIBILITY_PRESET hidden)
endif ()

target_link_libraries(raylib "${LIBS_PRIVATE}")

# Sets some compile time definitions for the pre-processor
# If CUSTOMIZE_BUILD option is on you will not use config.h by default
# and you will be able to select more build options
include(CompileDefinitions)

# Registering include directories
target_include_directories(raylib
                           PUBLIC
                           $<INSTALL_INTERFACE:include>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${OPENGL_INCLUDE_DIR}
                           ${OPENAL_INCLUDE_DIR}
                           )

# Copy the header files to the build directory for convenience
file(COPY ${raylib_public_headers} DESTINATION "include")

# Includes information on how the library will be installed on the system
# when cmake --install is run
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

# Options if you want to create an installer using CPack
include(PackConfigurations)

enable_testing()
