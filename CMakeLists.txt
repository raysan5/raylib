cmake_minimum_required(VERSION 3.0)

# Config options
set(BUILD_EXAMPLES ON CACHE BOOL "Build the examples.")
set(BUILD_GAMES ON CACHE BOOL "Build the example games.")

add_subdirectory(src release)

if (${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

if (${BUILD_GAMES})
  add_subdirectory(games)
endif()

