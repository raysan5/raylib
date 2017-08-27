# Setup the project and settings
project(examples)

include("../utils.cmake")

# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
# TODO place somewhere else?
include_directories("../build/release")

# Get the sources together
set(example_dirs audio core models others physac shaders text texutures)
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
