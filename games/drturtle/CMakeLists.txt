# Setup the project and settings
project(drturtle)

include("../../utils.cmake")


# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
include_directories("../../build/release")

# Executable & linking
add_executable(drturtle 06_drturtle_final.c)
link_libraries_to_executable(drturtle)

# Resources
# Copy all of the resource files to the destination
file(COPY "resources/"  DESTINATION "resources/")

