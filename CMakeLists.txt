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
foreach(option -Werror=pointer-arith;-Werror=implicit-function-declaration)
    CHECK_C_COMPILER_FLAG("${option}" COMPILER_HAS_THOSE_TOGGLES)
    set(outcome "Failed")
    if(COMPILER_HAS_THOSE_TOGGLES)
        set(CMAKE_C_FLAGS "${option} ${CMAKE_C_FLAGS}")
        set(outcome "works")
    endif()
    message(STATUS "Testing if ${option} can be used -- ${outcome}")
endforeach()


add_subdirectory(src release)

if (${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

if (${BUILD_GAMES})
  add_subdirectory(games)
endif()

