# Setup the project and settings
project(examples)

include("../cmake/utils.cmake")

# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
# TODO place somewhere else?
include_directories("../build/release")
include_directories("../src/external")
include_directories("../src/external/glfw/include")

# Get the sources together
set(example_dirs audio core models others shaders shapes text textures)
set(CMAKE_REQUIRED_DEFINITIONS -D_POSIX_C_SOURCE=199309L)
  include(CheckSymbolExists)
  check_symbol_exists(CLOCK_MONOTONIC time.h HAVE_CLOCK_MONOTONIC)
  check_symbol_exists(QueryPerformanceCounter windows.h HAVE_QPC)
set(CMAKE_REQUIRED_DEFINITIONS)
if(HAVE_QPC OR HAVE_CLOCK_MONOTONIC)
  set(example_dirs ${example_dirs} physac)
endif()

set(example_sources)
set(example_resources)
foreach(example_dir ${example_dirs})
  # Get the .c files
  file(GLOB sources ${example_dir}/*.c)
  list(APPEND example_sources ${sources})

  # Any any resources
  file(GLOB resources ${example_dir}/resources/*)
  list(APPEND example_resources ${resources})
endforeach()

include(CheckIncludeFiles)
list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/standard_lighting.c)

if(${PLATFORM} MATCHES "Android")
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/rlgl_standalone.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_picking.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_vr_simulator.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_camera_free.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_camera_first_person.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_world_screen.c)

  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_picking.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_material_pbr.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_cubicmap.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_skybox.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_picking.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_generation.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_heightmap.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_billboard.c)

  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_custom_uniform.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_model_shader.c)
  list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_postprocessing.c)
  
endif()


# Do each example
foreach(example_source ${example_sources})
  # Create the basename for the example
  get_filename_component(example_name ${example_source} NAME)
  string(REPLACE ".c" "" example_name ${example_name})

  # Setup the example
  add_executable(${example_name} ${example_source})

  # Link the libraries
  link_libraries_to_executable(${example_name})
endforeach()

# Copy all of the resource files to the destination
file(COPY ${example_resources} DESTINATION "resources/")
