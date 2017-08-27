# Setup the project and settings
project(games)

include("../utils.cmake")

# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
# TODO place somewhere else?
include_directories("../build/release")

# Get the source toegher
file(GLOB sources *.c)

# Do each game
foreach(game_source ${sources})
  # Create the basename for the game
  get_filename_component(game_name ${game_source} NAME)
  string(REPLACE ".c" "" game_name ${game_name})
  
  # Setup the game
  add_executable(${game_name} ${game_source})

  # Link the libraries
  link_libraries_to_executable(${game_name})
endforeach()

# Do the games with subdirectories
add_subdirectory(drturtle)
add_subdirectory(just_do)
add_subdirectory(koala_seasons)
add_subdirectory(light_my_ritual)
add_subdirectory(skully_escape)
add_subdirectory(wave_collector)
