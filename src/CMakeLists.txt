cmake_minimum_required (VERSION 3.0)
project (raylib)
SET(PLATFORM_TO_USE "PLATFORM_DESKTOP" CACHE STRING "Platform to compile for")
SET_PROPERTY(CACHE PLATFORM_TO_USE PROPERTY STRINGS PLATFORM_DESKTOP PLATFORM_RPI PLATFORM_WEB)

set(CMAKE_C_FLAGS "-O1 -Wall -std=gnu99 -fgnu89-inline -Wno-missing-braces")

IF(${PLATFORM_TO_USE} MATCHES "PLATFORM_DESKTOP")

     add_definitions(-DPLATFORM_DESKTOP, -DGRAPHICS_API_OPENGL_33)
     include_directories("." "external/" "external/openal_soft/include" "external/glfw3/include")

ENDIF()

IF(${PLATFORM_TO_USE} MATCHES "PLATFORM_RPI")

     add_definitions(-DPLATFORM_RPI, -GRAPHICS_API_OPENGL_ES2)
     include_directories("." "external/" "/opt/vc/include" "/opt/vc/include/interface/vmcs_host/linux" "/opt/vc/include/interface/vcos/pthreads")

ENDIF()

IF(${PLATFORM_TO_USE} MATCHES "PLATFORM_WEB")

     add_definitions(-DPLATFORM_WEB, -GRAPHICS_API_OPENGL_ES2)
     include_directories("." "external/" "external/openal_soft/include" "external/glfw3/include")

ENDIF()


file(GLOB SOURCES "*.c" "external/*.c")
add_library(raylib STATIC ${SOURCES})
install(TARGETS raylib DESTINATION ../lib/)