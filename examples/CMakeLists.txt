# Setup the project and settings
project(examples)

# Directories that contain examples
set(example_dirs
    audio
    core
    models
    others
    shaders
    shapes
    text
    textures
    )

# Next few lines will check for existence of symbols or header files
# They are needed for the physac example and threads examples
set(CMAKE_REQUIRED_DEFINITIONS -D_POSIX_C_SOURCE=199309L)
include(CheckSymbolExists)
check_symbol_exists(CLOCK_MONOTONIC time.h HAVE_CLOCK_MONOTONIC)
check_symbol_exists(QueryPerformanceCounter windows.h HAVE_QPC)
set(CMAKE_REQUIRED_DEFINITIONS)

if (HAVE_QPC OR HAVE_CLOCK_MONOTONIC)
    set(example_dirs ${example_dirs} physac)
endif ()

include(CheckIncludeFile)
CHECK_INCLUDE_FILE("stdatomic.h" HAVE_STDATOMIC_H)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT AND HAVE_STDATOMIC_H)
    add_if_flag_compiles("-std=c11" CMAKE_C_FLAGS)
    if (THREADS_HAVE_PTHREAD_ARG)
        add_if_flag_compiles("-pthread" CMAKE_C_FLAGS)
    endif ()
    if (CMAKE_THREAD_LIBS_INIT)
        link_libraries("${CMAKE_THREAD_LIBS_INIT}")
    endif ()
endif ()

if (APPLE AND NOT CMAKE_SYSTEM STRLESS "Darwin-18.0.0")
    add_definitions(-DGL_SILENCE_DEPRECATION)
    MESSAGE(AUTHOR_WARNING "OpenGL is deprecated starting with macOS 10.14 (Mojave)!")
endif ()

# Collect all source files and resource files
# into a CMake variable
set(example_sources)
set(example_resources)
foreach (example_dir ${example_dirs})
    # Get the .c files
    file(GLOB sources ${example_dir}/*.c)
    list(APPEND example_sources ${sources})

    # Any any resources
    file(GLOB resources ${example_dir}/resources/*)
    list(APPEND example_resources ${resources})
endforeach ()

if(NOT CMAKE_USE_PTHREADS_INIT OR NOT HAVE_STDATOMIC_H)
    # Items requiring pthreads
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_loading_thread.c)
endif ()

if (${PLATFORM} MATCHES "Android")
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/rlgl_standalone.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/standard_lighting.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_picking.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_vr_simulator.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_camera_free.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_3d_camera_first_person.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_world_screen.c)

    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_picking.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_cubicmap.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_skybox.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_picking.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_mesh_generation.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_heightmap.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_billboard.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_rlgl_solar_system.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_rlgl_full_solar_system.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_solar_system.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_obj_viewer.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_animation.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_first_person_maze.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/models/models_magicavoxel_loading.c)


    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_custom_uniform.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_model_shader.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_view_depth.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_postprocessing.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_raymarching.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_palette_switch.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shaders_basic_lighting.c)

elseif (${PLATFORM} MATCHES "Web")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    # Since WASM is used, ALLOW_MEMORY_GROWTH has no extra overheads
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/raylib_opengl_interop.c)

    # Remove the -rdynamic flag because otherwise emscripten
    # does not generate HTML+JS+WASM files, only a non-working
    # and fat HTML
    string(REPLACE "-rdynamic" "" CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")

elseif ("${PLATFORM}" STREQUAL "DRM")
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/rlgl_standalone.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/raylib_opengl_interop.c)

elseif (NOT SUPPORT_GESTURES_SYSTEM)
    # Items requiring gestures system
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/textures/textures_mouse_painting.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_basic_screen_manager.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_input_gestures_web.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/core/core_input_gestures.c)

endif ()

# The rlgl_standalone example only targets desktop, without shared libraries.
if (BUILD_SHARED_LIBS OR NOT ${PLATFORM} MATCHES "Desktop")
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/rlgl_standalone.c)
endif()

# The audio examples fail to link if raylib is built without raudio
if (NOT SUPPORT_MODULE_RAUDIO)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_mixed_processor.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_module_playing.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_music_stream.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_raw_stream.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_sound_loading.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_sound_multi.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/audio/audio_stream_effects.c)

    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/others/embedded_files_loading.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/textures/textures_sprite_button.c)
    list(REMOVE_ITEM example_sources ${CMAKE_CURRENT_SOURCE_DIR}/textures/textures_sprite_explosion.c)
endif()

include_directories(BEFORE SYSTEM others/external/include)

if (NOT TARGET raylib)
    find_package(raylib 2.0 REQUIRED)
endif ()

# Do each example
foreach (example_source ${example_sources})
    # Create the basename for the example
    get_filename_component(example_name ${example_source} NAME)
    string(REPLACE ".c" "" example_name ${example_name})

    # Setup the example
    add_executable(${example_name} ${example_source})

    target_link_libraries(${example_name} raylib)
    if (NOT WIN32)
        target_link_libraries(${example_name} m)
    endif()

    string(REGEX MATCH ".*/.*/" resources_dir ${example_source})
    string(APPEND resources_dir "resources")

    if (${PLATFORM} MATCHES "Web" AND EXISTS ${resources_dir})
        # The local resources path needs to be mapped to /resources virtual path
        string(APPEND resources_dir "@resources")
        set_target_properties(${example_name} PROPERTIES LINK_FLAGS "--preload-file ${resources_dir}")
    endif ()
endforeach ()

# Copy all of the resource files to the destination
file(COPY ${example_resources} DESTINATION "resources/")
