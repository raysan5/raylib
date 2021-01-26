if(${PLATFORM} MATCHES "Desktop" AND APPLE)
  if(MACOS_FATLIB)
      if (CMAKE_OSX_ARCHITECTURES)
          message(FATAL_ERROR "User supplied -DCMAKE_OSX_ARCHITECTURES overrides -DMACOS_FATLIB=ON")
      else()
          set(CMAKE_OSX_ARCHITECTURES "x86_64;i386")
      endif()
  endif()
endif()

# This helps support the case where emsdk toolchain file is used
# either by setting it with -DCMAKE_TOOLCHAIN_FILE=<path_to_emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# or by using "emcmake cmake -B build -S ." as described in https://emscripten.org/docs/compiling/Building-Projects.html
if(EMSCRIPTEN)
    SET(PLATFORM Web CACHE STRING "Forcing PLATFORM_WEB because EMSCRIPTEN was detected")
endif()

# vim: ft=cmake
