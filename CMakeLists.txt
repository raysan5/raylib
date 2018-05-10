cmake_minimum_required(VERSION 3.0)

# Config options
option(BUILD_EXAMPLES "Build the examples." ON)
option(BUILD_GAMES "Build the example games." ON)
option(ENABLE_ASAN  "Enable AddressSanitizer (ASAN) for debugging (degrades performance)" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (UBSan) for debugging" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer (MSan) for debugging (not recommended for run with ASAN)" OFF)

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
  endif()
else()
  set (CMAKE_C_STANDARD 99)
endif()

include(CheckCCompilerFlag)
function(add_if_flag_works flag)
    CHECK_C_COMPILER_FLAG("${flag}" COMPILER_HAS_THOSE_TOGGLES)
    set(outcome "Failed")
    if(COMPILER_HAS_THOSE_TOGGLES)
        foreach(var ${ARGN})
            set(${var} "${flag} ${${var}}" PARENT_SCOPE)
        endforeach()
        set(outcome "works")
    endif()
    message(STATUS "Testing if ${flag} can be used -- ${outcome}")
endfunction()

add_if_flag_works(-Werror=pointer-arith CMAKE_C_FLAGS)
add_if_flag_works(-Werror=implicit-function-declaration CMAKE_C_FLAGS)
# src/external/jar_xm.h does shady stuff
add_if_flag_works(-fno-strict-aliasing CMAKE_C_FLAGS)

message(STATUS "Check for symlink support in file system")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink CMakeLists.txt "${CMAKE_CURRENT_BINARY_DIR}/TestingIfSymlinkWorks"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE FILESYSTEM_LACKS_SYMLINKS
)
If (FILESYSTEM_LACKS_SYMLINKS)
  message(STATUS "Testing if file system supports symlinks -- unsupported")
else()
  message(STATUS "Testing if file system supports symlinks -- supported")
endif()

if (ENABLE_ASAN)
    add_if_flag_works(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_works(-fsanitize=address CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_UBSAN)
    add_if_flag_works(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_works(-fsanitize=undefined CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_MSAN)
    add_if_flag_works(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_works(-fsanitize=memory CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()

if (ENABLE_MSAN AND ENABLE_ASAN)
    MESSAGE(WARNING "Compiling with both AddressSanitizer and MemorySanitizer is not recommended")
endif()

add_subdirectory(src release)

if (${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

if (${BUILD_GAMES})
  add_subdirectory(games)
endif()
