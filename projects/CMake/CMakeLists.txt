cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
if (CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS)$")
  cmake_minimum_required(VERSION 3.22)
  if (NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "17.0" CACHE STRING "Minimum Apple OS version")
  endif ()
endif ()
project(example)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 6.0)
if (CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS)$" AND
    EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../src/raylib_apple.h" AND NOT DEFINED raylib_DIR)
  add_subdirectory(../.. raylib)
else ()
  find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
endif ()
if (NOT TARGET raylib AND NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
  endif()
endif()

# Our Project
if (CMAKE_SYSTEM_NAME MATCHES "^(iOS|tvOS)$")
    include("${CMAKE_CURRENT_LIST_DIR}/AppleApp.cmake")
    raylib_add_apple_app(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/core_basic_window.c" "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    return()
endif ()

add_executable(${PROJECT_NAME} core_basic_window.c)
#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html") # Tell Emscripten to build an example.html file.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
    target_link_libraries(${PROJECT_NAME} "-framework QuartzCore")
endif()
