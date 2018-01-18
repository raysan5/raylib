cmake_minimum_required(VERSION 3.0)

# Config options
set(BUILD_EXAMPLES ON CACHE BOOL "Build the examples.")
set(BUILD_GAMES ON CACHE BOOL "Build the example games.")

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
  endif()
else()
  set (CMAKE_C_STANDARD 99)
endif()
include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-Werror=pointer-arith" COMPILER_HAS_POINTER_ARITH_TOGGLE)
if(COMPILER_HAS_POINTER_ARITH_TOGGLE)
    set(CMAKE_C_FLAGS "-Werror=pointer-arith ${CMAKE_C_FLAGS}")
endif()

add_subdirectory(src release)

if (${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

if (${BUILD_GAMES})
  add_subdirectory(games)
endif()

