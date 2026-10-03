# Define the environment for cross-compiling with 32-bit MinGW-w64 Clang
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER     "i686-w64-mingw32-clang")
SET(CMAKE_CXX_COMPILER   "i686-w64-mingw32-clang++")
SET(CMAKE_RC_COMPILER    "i686-w64-mingw32-windres")
SET(CMAKE_RANLIB         "i686-w64-mingw32-ranlib")

# Configure the behaviour of the find commands
SET(CMAKE_FIND_ROOT_PATH "/usr/i686-w64-mingw32")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
