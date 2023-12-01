message(STATUS "From raylibPlatformCustomLibraryConfig (Custom platform 1 (Web)")

set(GRAPHICS "GRAPHICS_API_OPENGL_ES2")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 --profiling")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
