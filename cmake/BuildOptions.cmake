if(NOT (STATIC OR SHARED))
  message(FATAL_ERROR "Nothing to do if both -DSHARED=OFF and -DSTATIC=OFF...")
endif()

if (DEFINED BUILD_SHARED_LIBS)
  set(SHARED ${BUILD_SHARED_LIBS})
  if (${BUILD_SHARED_LIBS})
    set(STATIC OFF)
  else()
    set(STATIC ON)
  endif()
endif()
if(DEFINED SHARED_RAYLIB)
  set(SHARED ${SHARED_RAYLIB})
  message(DEPRECATION "-DSHARED_RAYLIB is deprecated. Please use -DSHARED instead.")
endif()
if(DEFINED STATIC_RAYLIB)
  set(STATIC ${STATIC_RAYLIB})
  message(DEPRECATION "-DSTATIC_RAYLIB is deprecated. Please use -DSTATIC instead.")
endif()

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
