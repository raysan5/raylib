# Define the environment for cross compiling from Linux to Win32
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_C_COMPILER     "i686-pc-mingw32-gcc")
SET(CMAKE_CXX_COMPILER   "i686-pc-mingw32-g++")
SET(CMAKE_RC_COMPILER    "i686-pc-mingw32-windres")
SET(CMAKE_RANLIB         "i686-pc-mingw32-ranlib")

#Configure the behaviour of the find commands
SET(CMAKE_FIND_ROOT_PATH "/opt/mingw/usr/i686-pc-mingw32")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
