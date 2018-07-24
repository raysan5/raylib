# Setup the project and settings
project(games)

include("../cmake/utils.cmake")

# Make sure raylib has been built
# TODO `build` directory should maybe be something else...
# TODO place somewhere else?
include_directories("../build/release")

# Get the source toegher
file(GLOB sources *.c)

message("PLATFORM = ${PLATFORM}")
set(OUTPUT_EXT)

if(${PLATFORM} MATCHES "Web")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --shell-file ${CMAKE_SOURCE_DIR}/templates/web_shell/shell.html")
  set(OUTPUT_EXT ".html")
endif()


# Do each game
foreach(game_source ${sources})
  # Create the basename for the game
  get_filename_component(game_name ${game_source} NAME)
  string(REPLACE ".c" "${OUTPUT_EXT}" game_name ${game_name})
  
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
