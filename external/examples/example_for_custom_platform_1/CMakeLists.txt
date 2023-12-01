cmake_minimum_required(VERSION 3.0)

# This should be absolute to where raylib lives
set(RAYLIB_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../..")

if(PLATFORM STREQUAL "Custom")
  set(PLATFORM_CUSTOM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../custom_platforms/custom_platform_1")

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --profiling-funcs -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s SAFE_HEAP=1")
endif()

project(example_for_custom_platform_1 LANGUAGES CXX)

set(target "example_for_custom_platform_1")

set(CMAKE_CXX_STANDARD 17)

#######################################################
# adding raylib
# Note: raylib is bundled in this project but could
#       easily be fetched instead
#######################################################
add_subdirectory("${RAYLIB_ROOT_DIR}" raylib EXCLUDE_FROM_ALL)

#######################################################
# Main executable
#######################################################
add_executable(${target} main.cpp)
target_link_libraries(${target} PUBLIC raylib)

if(PLATFORM STREQUAL "Custom")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(EMSDK "$ENV{EMSDK}" CACHE PATH "Location of the emscript sdk")
  if("${EMSDK}" STREQUAL "")
    message(FATAL_ERROR "Make sure you define the variable EMSDK (either as an environment variable or as a CMake -DEMSDK=... option).")
  endif()
  target_include_directories(${target} PUBLIC "${EMSDK}/emscripten/main/cache/sysroot/include")
endif()
