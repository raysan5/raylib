
if(USE_EXTERNAL_GLFW STREQUAL "ON")
    find_package(glfw3 3.4 REQUIRED)
elseif(USE_EXTERNAL_GLFW STREQUAL "IF_POSSIBLE")
    find_package(glfw3 3.4 QUIET)
endif()
if (glfw3_FOUND)
    set(LIBS_PRIVATE ${LIBS_PRIVATE} glfw)
endif()

# Explicitly check against "ON", because USE_EXTERNAL_GLFW is a tristate option
# Also adding only on desktop (web also uses glfw but it is more limited and is added using an emcc linker flag)
if(NOT glfw3_FOUND AND NOT USE_EXTERNAL_GLFW STREQUAL "ON" AND "${PLATFORM}" MATCHES "Desktop")
    MESSAGE(STATUS "Using raylib's GLFW")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_LIBRARY_TYPE "OBJECT" CACHE STRING "" FORCE)
    

    add_subdirectory(external/glfw)

    # Hide glfw's symbols when building a shared lib
    if (BUILD_SHARED_LIBS)
        set_property(TARGET glfw PROPERTY C_VISIBILITY_PRESET hidden)
    endif()
    
    list(APPEND raylib_sources $<TARGET_OBJECTS:glfw>)
    include_directories(BEFORE SYSTEM external/glfw/include)
elseif("${PLATFORM}" STREQUAL "DRM")
    MESSAGE(STATUS "No GLFW required on PLATFORM_DRM")
else()
    MESSAGE(STATUS "Using external GLFW")
    set(GLFW_PKG_DEPS glfw3)
endif()
