# Setup the project and settings
project(skully_escape)

include("../../utils.cmake")


# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
include_directories("../../build/release")

# Grab the screens
file(GLOB screen_sources "screens/*.c")

# Executable & linking
add_executable(skully_escape skully_escape.c player.c monster.c ${screen_sources})
link_libraries_to_executable(skully_escape)

# Resources
# Copy all of the resource files to the destination
file(COPY "resources/"  DESTINATION "resources/")

