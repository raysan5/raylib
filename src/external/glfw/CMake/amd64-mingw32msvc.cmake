# Define the environment for cross compiling from Linux to Win64
SET(CMAKE_SYSTEM_NAME    Windows)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER     "amd64-mingw32msvc-gcc")
SET(CMAKE_CXX_COMPILER   "amd64-mingw32msvc-g++")
SET(CMAKE_RC_COMPILER    "amd64-mingw32msvc-windres")
SET(CMAKE_RANLIB         "amd64-mingw32msvc-ranlib")

# Configure the behaviour of the find commands
SET(CMAKE_FIND_ROOT_PATH "/usr/amd64-mingw32msvc")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
