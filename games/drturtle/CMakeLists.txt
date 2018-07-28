cmake_minimum_required(VERSION 2.6)
project(drturtle)

# Executable & linking
add_executable(${PROJECT_NAME} 06_drturtle_final.c)
if (NOT TARGET raylib)
  find_package(raylib 2.0 REQUIRED)
endif()
target_link_libraries(${PROJECT_NAME} raylib)

# Resources
# Copy all of the resource files to the destination
file(COPY "resources/"  DESTINATION "resources/")
