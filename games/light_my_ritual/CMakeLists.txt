# Setup the project and settings
project(light_my_ritual)

include("../../utils.cmake")


# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
include_directories("../../build/release")

# Grab the screens
file(GLOB screen_sources "screens/*.c")

# Executable & linking
add_executable(light_my_ritual light_my_ritual.c ${screen_sources})
link_libraries_to_executable(light_my_ritual)

# Resources
# Copy all of the resource files to the destination
file(COPY "resources/"  DESTINATION "resources/")

