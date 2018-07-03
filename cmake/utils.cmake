# All sorts of things that we need cross project
cmake_minimum_required(VERSION 2.8.0)

add_definitions("-DRAYLIB_CMAKE=1")

# Linking for OS X -framework options
# Will do nothing on other OSes
if(${PLATFORM} MATCHES "Android")
  find_library(OPENGL_LIBRARY OpenGL)
  set(LIBS_PRIVATE m log android EGL GLESv2 OpenSLES atomic c)
elseif(${PLATFORM} MATCHES "Web")
elseif(APPLE)
  find_library(OPENGL_LIBRARY OpenGL)

  set(LIBS_PRIVATE ${OPENGL_LIBRARY})
elseif(WIN32)
  # no pkg-config --static on Windows yet...
else()
  find_library(pthread NAMES pthread)
  find_package(OpenGL QUIET)
  if ("${OPENGL_LIBRARIES}" STREQUAL "")
    set(OPENGL_LIBRARIES "GL")
  endif()

  include_directories(${OPENGL_INCLUDE_DIR})

  if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Net|Open)BSD")
    find_library(OSS_LIBRARY ossaudio)
  endif()

  set(LIBS_PRIVATE m pthread ${OPENGL_LIBRARIES} ${OSS_LIBRARY})
endif()

if(${PLATFORM} MATCHES "Desktop")
  if(USE_EXTERNAL_GLFW STREQUAL "ON")
    find_package(glfw3 3.2.1 REQUIRED)
  elseif(USE_EXTERNAL_GLFW STREQUAL "IF_POSSIBLE")
    find_package(glfw3 3.2.1 QUIET)
  endif()
  if (glfw3_FOUND)
    set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(LINUX TRUE)
endif()

foreach(L ${LIBS_PRIVATE})
  get_filename_component(DIR  ${L} PATH)
  get_filename_component(LIBFILE ${L} NAME_WE)
  STRING(REGEX REPLACE "^lib" "" FILE ${LIBFILE})

  if (${L} MATCHES "[.]framework$")
      set(FILE_OPT "-framework ${FILE}")
      set(DIR_OPT  "-F${DIR}")
  else()
      set(FILE_OPT "-l${FILE}")
      set(DIR_OPT  "-L${DIR}")
  endif()

  if ("${DIR}" STREQUAL "" OR "${DIR}" STREQUAL "${LASTDIR}")
      set (DIR_OPT "")
  endif()

  set(LASTDIR ${DIR})

  set(__PKG_CONFIG_LIBS_PRIVATE ${__PKG_CONFIG_LIBS_PRIVATE} ${DIR_OPT} ${FILE_OPT})
  string (REPLACE ";" " " __PKG_CONFIG_LIBS_PRIVATE "${__PKG_CONFIG_LIBS_PRIVATE}")
endforeach(L)



# Do the linking for executables that are meant to link raylib
function(link_libraries_to_executable executable)
  # Link raylib
  if (TARGET raylib_shared)
    target_link_libraries(${executable} raylib_shared)
  elseif(${PLATFORM} MATCHES "Web")
    target_link_libraries(${executable} ${__PKG_CONFIG_LIBS_PRIVATE})
    target_link_libraries(${executable} raylib)
  else()
    target_link_libraries(${executable} raylib ${__PKG_CONFIG_LIBS_PRIVATE})
  endif()
endfunction()
