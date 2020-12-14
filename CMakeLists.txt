cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(RAYLIB_IS_MAIN TRUE)
else()
  set(RAYLIB_IS_MAIN FALSE)
endif()

# Config options
option(BUILD_EXAMPLES "Build the examples." ${RAYLIB_IS_MAIN})
option(ENABLE_ASAN  "Enable AddressSanitizer (ASAN) for debugging (degrades performance)" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (UBSan) for debugging" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer (MSan) for debugging (not recommended to run with ASAN)" OFF)

# This helps support the case where emsdk toolchain file is used
# either by setting it with -DCMAKE_TOOLCHAIN_FILE=<path_to_emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# or by using "emcmake cmake -B build -S ." as described in https://emscripten.org/docs/compiling/Building-Projects.html
if(EMSCRIPTEN)
  SET(PLATFORM Web CACHE STRING "Forcing PLATFORM_WEB because EMSCRIPTEN was detected")
endif()

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
  endif()
else()
  set (CMAKE_C_STANDARD 99)
endif()

include(AddIfFlagCompiles)
add_if_flag_compiles(-Werror=pointer-arith CMAKE_C_FLAGS)
add_if_flag_compiles(-Werror=implicit-function-declaration CMAKE_C_FLAGS)
# src/external/jar_xm.h does shady stuff
add_if_flag_compiles(-fno-strict-aliasing CMAKE_C_FLAGS)

if (ENABLE_ASAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=address CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_UBSAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=undefined CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_MSAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=memory CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()

if (ENABLE_MSAN AND ENABLE_ASAN)
    MESSAGE(WARNING "Compiling with both AddressSanitizer and MemorySanitizer is not recommended")
endif()

add_subdirectory(src)

if (${BUILD_EXAMPLES})
  MESSAGE(STATUS "Building examples is enabled")
  add_subdirectory(examples)
endif()

enable_testing()
