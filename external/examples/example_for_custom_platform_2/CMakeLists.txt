cmake_minimum_required(VERSION 3.0)

# This should be absolute to where raylib lives
set(RAYLIB_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../..")

if(PLATFORM STREQUAL "Custom")
  # custom_platform_2 requires GLFW_ROOT_DIR to be set
  set(GLFW_ROOT_DIR "${RAYLIB_ROOT_DIR}/src/external/glfw")
  set(PLATFORM_CUSTOM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../custom_platforms/custom_platform_2")
endif()

project(example_for_custom_platform_2 LANGUAGES CXX)

set(target "example_for_custom_platform_2")

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

