## Building the Examples

The examples assume you have already built the `raylib` library in `../src`.

### With GNU make

- `make` builds all examples
- `make [module]` builds all examples for a particular module (e.g `make core`)

### With Zig

The [Zig](https://ziglang.org/) toolchain can compile `C` and `C++` in addition to `Zig`.
You may find it easier to use than other toolchains, especially when it comes to cross-compiling.

- `zig build` to compile all examples
- `zig build [module]` to compile all examples for a module (e.g. `zig build core`)
- `zig build [example]` to compile _and run_ a particular example (e.g. `zig build core_basic_window`)

## EXAMPLES LIST

### category: core

Examples using raylib core platform functionality like window creation, inputs, drawing modes and system functionality.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 01 | [core_basic_window](core/core_basic_window.c)                     | <img src="core/core_basic_window.png" alt="core_basic_window" width="200">                     | ray                                               |        |
| 02 | [core_input_keys](core/core_input_keys.c)                         | <img src="core/core_input_keys.png" alt="core_input_keys" width="200">                         | ray                                               |        |
| 03 | [core_input_mouse](core/core_input_mouse.c)                       | <img src="core/core_input_mouse.png" alt="core_input_mouse" width="200">                       | ray                                               |        |
| 04 | [core_input_mouse_wheel](core/core_input_mouse_wheel.c)           | <img src="core/core_input_mouse_wheel.png" alt="core_input_mouse_wheel" width="200">           | ray                                               |        |
| 05 | [core_input_gamepad](core/core_input_gamepad.c)                   | <img src="core/core_input_gamepad.png" alt="core_input_gamepad" width="200">                   | ray                                               |        |
| 06 | [core_input_multitouch](core/core_input_multitouch.c)             | <img src="core/core_input_multitouch.png" alt="core_input_multitouch" width="200">             | [Berni](https://github.com/Berni8k)               |        |
| 07 | [core_input_gestures](core/core_input_gestures.c)                 | <img src="core/core_input_gestures.png" alt="core_input_gestures" width="200">                 | ray                                               |        |
| 08 | [core_2d_camera](core/core_2d_camera.c)                           | <img src="core/core_2d_camera.png" alt="core_2d_camera" width="200">                           | ray                                               |        |
| 09 | [core_2d_camera_platformer](core/core_2d_camera_platformer.c)     | <img src="core/core_2d_camera_platformer.png" alt="core_2d_camera_platformer" width="200">     | [avyy](https://github.com/avyy)                   | ⭐️     |
| 10 | [core_3d_camera_mode](core/core_3d_camera_mode.c)                 | <img src="core/core_3d_camera_mode.png" alt="core_3d_camera_mode" width="200">                 | ray                                               |        |
| 11 | [core_3d_camera_free](core/core_3d_camera_free.c)                 | <img src="core/core_3d_camera_free.png" alt="core_3d_camera_free" width="200">                 | ray                                               |        |
| 12 | [core_3d_camera_first_person](core/core_3d_camera_first_person.c) | <img src="core/core_3d_camera_first_person.png" alt="core_3d_camera_first_person" width="200"> | ray                                               |        |
| 13 | [core_3d_picking](core/core_3d_picking.c)                         | <img src="core/core_3d_picking.png" alt="core_3d_picking" width="200">                         | ray                                               |        |
| 14 | [core_world_screen](core/core_world_screen.c)                     | <img src="core/core_world_screen.png" alt="core_world_screen" width="200">                     | ray                                               |        |
| 15 | [core_custom_logging](core/core_custom_logging.c)                 | <img src="core/core_custom_logging.png" alt="core_custom_logging" width="200">                 | [Pablo Marcos](https://github.com/pamarcos)       |        |
| 16 | [core_window_letterbox](core/core_window_letterbox.c)             | <img src="core/core_window_letterbox.png" alt="core_window_letterbox" width="200">             | [Anata](https://github.com/anatagawa)             |        |
| 17 | [core_drop_files](core/core_drop_files.c)                         | <img src="core/core_drop_files.png" alt="core_drop_files" width="200">                         | ray                                               |        |
| 18 | [core_random_values](core/core_random_values.c)                   | <img src="core/core_random_values.png" alt="core_random_values" width="200">                   | ray                                               |        |
| 19 | [core_scissor_test](core/core_scissor_test.c)                     | <img src="core/core_scissor_test.png" alt="core_scissor_test" width="200">                     | [Chris Dill](https://github.com/MysteriousSpace)  |        |
| 20 | [core_storage_values](core/core_storage_values.c)                 | <img src="core/core_storage_values.png" alt="core_storage_values" width="200">                 | ray                                               |        |
| 21 | [core_vr_simulator](core/core_vr_simulator.c)                     | <img src="core/core_vr_simulator.png" alt="core_vr_simulator" width="200">                     | ray                                               | ⭐️     |
| 22 | [core_loading_thread](core/core_loading_thread.c)                 | <img src="core/core_loading_thread.png" alt="core_loading_thread" width="200">                 | ray                                               |        |
| 23 | [core/core_quat_conversion](core/core_quat_conversion.c)          | <img src="core/core_quat_conversion.png" alt="core_quat_conversion" width="200">               | [Chris Camacho](https://github.com/codifies)      |        |
| 24 | [core/core_window_flags](core/core_window_flags.c)                | <img src="core/core_window_flags.png" alt="core_window_flags" width="200">                     | ray                                               |        | 

### category: shapes

Examples using raylib shapes drawing functionality, provided by raylib [shapes](../src/shapes.c) module.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 25 | [shapes_basic_shapes](shapes/shapes_basic_shapes.c)                       | <img src="shapes/shapes_basic_shapes.png" alt="shapes_basic_shapes" width="200">                       | ray                                        |        |
| 26 | [shapes_bouncing_ball](shapes/shapes_bouncing_ball.c)                     | <img src="shapes/shapes_bouncing_ball.png" alt="shapes_bouncing_ball" width="200">                     | ray                                        |        |
| 27 | [shapes_colors_palette](shapes/shapes_colors_palette.c)                   | <img src="shapes/shapes_colors_palette.png" alt="shapes_colors_palette" width="200">                   | ray                                        |        |
| 28 | [shapes_logo_raylib](shapes/shapes_logo_raylib.c)                         | <img src="shapes/shapes_logo_raylib.png" alt="shapes_logo_raylib" width="200">                         | ray                                        |        |
| 29 | [shapes_logo_raylib_anim](shapes/shapes_logo_raylib_anim.c)               | <img src="shapes/shapes_logo_raylib_anim.png" alt="shapes_logo_raylib_anim" width="200">               | ray                                        |        |
| 30 | [shapes_rectangle_scaling](shapes/shapes_rectangle_scaling.c)             | <img src="shapes/shapes_rectangle_scaling.png" alt="shapes_rectangle_scaling" width="200">             | [Vlad Adrian](https://github.com/demizdor) |        |
| 31 | [shapes_lines_bezier](shapes/shapes_lines_bezier.c)                       | <img src="shapes/shapes_lines_bezier.png" alt="shapes_lines_bezier" width="200">                       | ray                                        |        |
| 32 | [shapes_collision_area](shapes/shapes_collision_area.c)                   | <img src="shapes/shapes_collision_area.png" alt="shapes_collision_area" width="200">                   | ray                                        |        |
| 33 | [shapes_following_eyes](shapes/shapes_following_eyes.c)                   | <img src="shapes/shapes_following_eyes.png" alt="shapes_following_eyes" width="200">                   | ray                                        |        |
| 34 | [shapes_easings_ball_anim](shapes/shapes_easings_ball_anim.c)             | <img src="shapes/shapes_easings_ball_anim.png" alt="shapes_easings_ball_anim" width="200">             | ray                                        |        |
| 35 | [shapes_easings_box_anim](shapes/shapes_easings_box_anim.c)               | <img src="shapes/shapes_easings_box_anim.png" alt="shapes_easings_box_anim" width="200">               | ray                                        |        |
| 36 | [shapes_easings_rectangle_array](shapes/shapes_easings_rectangle_array.c) | <img src="shapes/shapes_easings_rectangle_array.png" alt="shapes_easings_rectangle_array" width="200"> | ray                                        |        |
| 37 | [shapes_draw_ring](shapes/shapes_draw_ring.c)                             | <img src="shapes/shapes_draw_ring.png" alt="shapes_draw_ring" width="200">                             | [Vlad Adrian](https://github.com/demizdor) |        |
| 38 | [shapes_draw_circle_sector](shapes/shapes_draw_circle_sector.c)           | <img src="shapes/shapes_draw_circle_sector.png" alt="shapes_draw_circle_sector" width="200">           | [Vlad Adrian](https://github.com/demizdor) |        |
| 39 | [shapes_draw_rectangle_rounded](shapes/shapes_draw_rectangle_rounded.c)   | <img src="shapes/shapes_draw_rectangle_rounded.png" alt="shapes_draw_rectangle_rounded" width="200">   | [Vlad Adrian](https://github.com/demizdor) |        |

### category: textures

Examples using raylib textures functionality, including image/textures loading/generation and drawing, provided by raylib [textures](../src/textures.c) module.

| ## | example                                                                   | image                                                                                                  | developer                                        | new |
|----|---------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------|:------------------------------------------------:|:---:|
| 40 | [textures_logo_raylib](textures/textures_logo_raylib.c)                   | <img src="textures/textures_logo_raylib.png" alt="textures_logo_raylib" width="200">                   | ray                                              |     |
| 41 | [textures_rectangle](textures/textures_rectangle.c)                       | <img src="textures/textures_rectangle.png" alt="textures_rectangle" width="200">                       | ray                                              |     |
| 42 | [textures_srcrec_dstrec](textures/textures_srcrec_dstrec.c)               | <img src="textures/textures_srcrec_dstrec.png" alt="textures_srcrec_dstrec" width="200">               | ray                                              |     |
| 43 | [textures_image_drawing](textures/textures_image_drawing.c)               | <img src="textures/textures_image_drawing.png" alt="textures_image_drawing" width="200">               | ray                                              |     |
| 44 | [textures_image_generation](textures/textures_image_generation.c)         | <img src="textures/textures_image_generation.png" alt="textures_image_generation" width="200">         | ray                                              |     |
| 45 | [textures_image_loading](textures/textures_image_loading.c)               | <img src="textures/textures_image_loading.png" alt="textures_image_loading" width="200">               | ray                                              |     |
| 46 | [textures_image_processing](textures/textures_image_processing.c)         | <img src="textures/textures_image_processing.png" alt="textures_image_processing" width="200">         | ray                                              |     |
| 47 | [textures_image_text](textures/textures_image_text.c)                     | <img src="textures/textures_image_text.png" alt="textures_image_text" width="200">                     | ray                                              |     |
| 48 | [textures_to_image](textures/textures_to_image.c)                         | <img src="textures/textures_to_image.png" alt="textures_to_image" width="200">                         | ray                                              |     |
| 49 | [textures_raw_data](textures/textures_raw_data.c)                         | <img src="textures/textures_raw_data.png" alt="textures_raw_data" width="200">                         | ray                                              |     |
| 50 | [textures_particles_blending](textures/textures_particles_blending.c)     | <img src="textures/textures_particles_blending.png" alt="textures_particles_blending" width="200">     | ray                                              |     |
| 51 | [textures_npatch_drawing](textures/textures_npatch_drawing.c)             | <img src="textures/textures_npatch_drawing.png" alt="textures_npatch_drawing" width="200">             | [Jorge A. Gomes](https://github.com/overdev)     |     |
| 52 | [textures_background_scrolling](textures/textures_background_scrolling.c) | <img src="textures/textures_background_scrolling.png" alt="textures_background_scrolling" width="200"> | ray                                              |     |
| 53 | [textures_sprite_button](textures/textures_sprite_button.c)               | <img src="textures/textures_sprite_button.png" alt="textures_sprite_button" width="200">               | ray                                              |     |
| 54 | [textures_sprite_explosion](textures/textures_sprite_explosion.c)         | <img src="textures/textures_sprite_explosion.png" alt="textures_sprite_explosion" width="200">         | ray                                              |     |
| 55 | [textures_bunnymark](textures/textures_bunnymark.c)                       | <img src="textures/textures_bunnymark.png" alt="textures_bunnymark" width="200">                       | ray                                              |     |
| 56 | [textures_mouse_painting](textures/textures_mouse_painting.c)             | <img src="textures/textures_mouse_painting.png" alt="textures_mouse_painting" width="200">             | [Chris Dill](https://github.com/MysteriousSpace) |     |
| 57 | [textures_blend_modes](textures/textures_blend_modes.c)                   | <img src="textures/textures_blend_modes.png" alt="textures_blend_modes" width="200">                   | [Karlo Licudine](https://github.com/accidentalrebel) |     |
| 58 | [textures_draw_tiled](textures/textures_draw_tiled.c)                     | <img src="textures/textures_draw_tiled.png" alt="textures_draw_tiled" width="200">                     | [Vlad Adrian](https://github.com/demizdor)       |       |
| -- | [textures_poly](textures/textures_poly.c)                                 | <img src="textures/textures_poly.png" alt="textures_poly" width="200">                                 | [Chris Camacho](https://github.com/codifies)     | ⭐️   |

### category: text

Examples using raylib text functionality, including sprite fonts loading/generation and text drawing, provided by raylib [text](../src/text.c) module.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 59 | [text_raylib_fonts](text/text_raylib_fonts.c)         | <img src="text/text_raylib_fonts.png" alt="text_raylib_fonts" width="200">         | ray                                        |        |
| 60 | [text_font_spritefont](text/text_font_spritefont.c)   | <img src="text/text_font_spritefont.png" alt="text_font_spritefont" width="200">   | ray                                        |        |
| 61 | [text_font_filters](text/text_font_filters.c)         | <img src="text/text_font_filters.png" alt="text_font_filters" width="200">         | ray                                        |        |
| 62 | [text_font_loading](text/text_font_loading.c)         | <img src="text/text_font_loading.png" alt="text_font_loading" width="200">         | ray                                        |        |
| 63 | [text_font_sdf](text/text_font_sdf.c)                 | <img src="text/text_font_sdf.png" alt="text_font_sdf" width="200">                 | ray                                        |        |
| 64 | [text_format_text](text/text_format_text.c)           | <img src="text/text_format_text.png" alt="text_format_text" width="200">           | ray                                        |        |
| 65 | [text_input_box](text/text_input_box.c)               | <img src="text/text_input_box.png" alt="text_input_box" width="200">               | ray                                        |        |
| 66 | [text_writing_anim](text/text_writing_anim.c)         | <img src="text/text_writing_anim.png" alt="text_writing_anim" width="200">         | ray                                        |        |
| 67 | [text_rectangle_bounds](text/text_rectangle_bounds.c) | <img src="text/text_rectangle_bounds.png" alt="text_rectangle_bounds" width="200"> | [Vlad Adrian](https://github.com/demizdor) |        |
| 68 | [text_unicode](text/text_unicode.c)                   | <img src="text/text_unicode.png" alt="text_unicode" width="200">                   | [Vlad Adrian](https://github.com/demizdor) |        |
| -- | [text_draw_3d](text/text_draw_3d.c)                   | <img src="text/text_draw_3d.png" alt="text_draw_3d" width="200">                   | [Vlad Adrian](https://github.com/demizdor) | ⭐️    |    
    
### category: models

Examples using raylib models functionality, including models loading/generation and drawing, provided by raylib [models](../src/models.c) module.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 69 | [models_animation](models/models_animation.c)                             | <img src="models/models_animation.png" alt="models_animation" width="200">                             | [culacant](https://github.com/culacant)          |        |
| 70 | [models_billboard](models/models_billboard.c)                             | <img src="models/models_billboard.png" alt="models_billboard" width="200">                             | ray                                              |        |
| 71 | [models_box_collisions](models/models_box_collisions.c)                   | <img src="models/models_box_collisions.png" alt="models_box_collisions" width="200">                   | ray                                              |        |
| 72 | [models_cubicmap](models/models_cubicmap.c)                               | <img src="models/models_cubicmap.png" alt="models_cubicmap" width="200">                               | ray                                              |        |
| 73 | [models_first_person_maze](models/models_first_person_maze.c)             | <img src="models/models_first_person_maze.png" alt="models_first_person_maze" width="200">             | ray                                              |        |
| 74 | [models_geometric_shapes](models/models_geometric_shapes.c)               | <img src="models/models_geometric_shapes.png" alt="models_geometric_shapes" width="200">               | ray                                              |        |
| 75 | [...]()                       |                        | ray                                              |        |
| 76 | [models_mesh_generation](models/models_mesh_generation.c)                 | <img src="models/models_mesh_generation.png" alt="models_mesh_generation" width="200">                 | ray                                              |        |
| 77 | [models_mesh_picking](models/models_mesh_picking.c)                       | <img src="models/models_mesh_picking.png" alt="models_mesh_picking" width="200">                       | [Joel Davis](https://github.com/joeld42)         |        |
| 78 | [models_loading](models/models_loading.c)                                 | <img src="models/models_loading.png" alt="models_loading" width="200">                                 | ray                                              |        |
| 79 | [models_orthographic_projection](models/models_orthographic_projection.c) | <img src="models/models_orthographic_projection.png" alt="models_orthographic_projection" width="200"> | [Max Danielsson](https://github.com/autious)     |        |
| 80 | [models_rlgl_solar_system](models/models_rlgl_solar_system.c)             | <img src="models/models_rlgl_solar_system.png" alt="models_rlgl_solar_system" width="200">             | ray                                              |        |
| 81 | [models_skybox](models/models_skybox.c)                                   | <img src="models/models_skybox.png" alt="models_skybox" width="200">                                   | ray                                              |        |
| 82 | [models_yaw_pitch_roll](models/models_yaw_pitch_roll.c)                   | <img src="models/models_yaw_pitch_roll.png" alt="models_yaw_pitch_roll" width="200">                   | [Berni](https://github.com/Berni8k)              | ⭐️    |
| 83 | [models_heightmap](models/models_heightmap.c)                             | <img src="models/models_heightmap.png" alt="models_heightmap" width="200">                             | ray                                              |        |
| 84 | [models_waving_cubes](models/models_waving_cubes.c)                       | <img src="models/models_waving_cubes.png" alt="models_waving_cubes" width="200">                       | [codecat](https://github.com/codecat)            |        |
| -- | [models_gltf_model](models/models_gltf_model.c)                           | <img src="models/models_gltf_model.png" alt="models_gltf_model" width="200">                           | [object71](https://github.com/object71)          | ⭐️     |

### category: shaders

Examples using raylib shaders functionality, including shaders loading, parameters configuration and drawing using them (model shaders and postprocessing shaders). This functionality is directly provided by raylib [rlgl](../src/rlgl.c) module.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 85 | [shaders_basic_lighting](shaders/shaders_basic_lighting.c)   | <img src="shaders/shaders_basic_lighting.png" alt="shaders_basic_lighting" width="200">   | [Chris Camacho](https://github.com/codifies) |      |
| 86 | [shaders_model_shader](shaders/shaders_model_shader.c)       | <img src="shaders/shaders_model_shader.png" alt="shaders_model_shader" width="200">       | ray                                          |      |
| 87 | [shaders_shapes_textures](shaders/shaders_shapes_textures.c) | <img src="shaders/shaders_shapes_textures.png" alt="shaders_shapes_textures" width="200"> | ray                                          |      |
| 88 | [shaders_custom_uniform](shaders/shaders_custom_uniform.c)   | <img src="shaders/shaders_custom_uniform.png" alt="shaders_custom_uniform" width="200">   | ray                                          |      |
| 89 | [shaders_postprocessing](shaders/shaders_postprocessing.c)   | <img src="shaders/shaders_postprocessing.png" alt="shaders_postprocessing" width="200">   | ray                                          |      |
| 90 | [shaders_palette_switch](shaders/shaders_palette_switch.c)   | <img src="shaders/shaders_palette_switch.png" alt="shaders_palette_switch" width="200">   | [Marco Lizza](https://github.com/MarcoLizza) |      |
| 91 | [shaders_raymarching](shaders/shaders_raymarching.c)         | <img src="shaders/shaders_raymarching.png" alt="shaders_raymarching" width="200">         | Shader by Iñigo Quilez                       |      |
| 92 | [shaders_texture_drawing](shaders/shaders_texture_drawing.c) | <img src="shaders/shaders_texture_drawing.png" alt="shaders_texture_drawing" width="200"> | Michał Ciesielski                            |      |
| 93 | [shaders_texture_waves](shaders/shaders_texture_waves.c)     | <img src="shaders/shaders_texture_waves.png" alt="shaders_texture_waves" width="200">     | [Anata](https://github.com/anatagawa)        |      |
| 94 | [shaders_julia_set](shaders/shaders_julia_set.c)             | <img src="shaders/shaders_julia_set.png" alt="shaders_julia_set" width="200">             | [eggmund](https://github.com/eggmund)        |      |
| 95 | [shaders_eratosthenes](shaders/shaders_eratosthenes.c)       | <img src="shaders/shaders_eratosthenes.png" alt="shaders_eratosthenes" width="200">       | [ProfJski](https://github.com/ProfJski)      |      |
| 96 | [shaders_fog](shaders/shaders_fog.c)                         | <img src="shaders/shaders_fog.png" alt="shaders_fog" width="200">                         | [Chris Camacho](https://github.com/codifies) |      |
| 97 | [shaders_simple_mask](shaders/shaders_simple_mask.c)         | <img src="shaders/shaders_simple_mask.png" alt="shaders_simple_mask" width="200">         | [Chris Camacho](https://github.com/codifies) |      |
| 98 | [shaders_spotlight](shaders/shaders_spotlight.c)             | <img src="shaders/shaders_spotlight.png" alt="shaders_spotlight" width="200">             | [Chris Camacho](https://github.com/codifies) |      |
| 99 | [shaders_hot_reloading](shaders/shaders_hot_reloading.c)     | <img src="shaders/shaders_hot_reloading.png" alt="shaders_hot_reloading" width="200">     | ray                                          |      |
| 100 | [shaders_mesh_instancing](shaders/shaders_mesh_instancing.c) | <img src="shaders/shaders_mesh_instancing.png" alt="shaders_mesh_instancing" width="200"> | [seanpringle](https://github.com/seanpringle), [moliad](https://github.com/moliad) | ⭐️     |
| 101 | [shaders_multi_sample2d](shaders/shaders_multi_sample2d.c)   | <img src="shaders/shaders_multi_sample2d.png" alt="shaders_multi_sample2d" width="200">  | ray                                          |      |
    
### category: audio

Examples using raylib audio functionality, including sound/music loading and playing. This functionality is provided by raylib [raudio](../src/raudio.c) module. Note this module can be used standalone independently of raylib, check [raudio_standalone](others/raudio_standalone.c) example.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 102 | [audio_module_playing](audio/audio_module_playing.c)         | <img src="audio/audio_module_playing.png" alt="audio_module_playing" width="200">         | ray                                          |        |
| 103 | [audio_music_stream](audio/audio_music_stream.c)             | <img src="audio/audio_music_stream.png" alt="audio_music_stream" width="200">             | ray                                          |        |
| 104 | [audio_raw_stream](audio/audio_raw_stream.c)                 | <img src="audio/audio_raw_stream.png" alt="audio_raw_stream" width="200">                 | ray                                          |        |
| 105 | [audio_sound_loading](audio/audio_sound_loading.c)           | <img src="audio/audio_sound_loading.png" alt="audio_sound_loading" width="200">           | ray                                          |        |
| 106 | [audio_multichannel_sound](audio/audio_multichannel_sound.c) | <img src="audio/audio_multichannel_sound.png" alt="audio_multichannel_sound" width="200"> | [Chris Camacho](https://github.com/codifies) | ⭐️     |

### category: physics

Examples showing physics functionality with raylib. This functionality is provided by [physac](https://github.com/victorfisac/Physac) library, included with raylib [sources](../src/physac.h). Note this library is not linked with raylib by default, it should be manually included in user code.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 107 | [physics_demo](physics/physics_demo.c)               | <img src="physics/physics_demo.png" alt="physics_demo" width="200">               | [Victor Fisac](https://github.com/victorfisac) |        |
| 108 | [physics_friction](physics/physics_friction.c)       | <img src="physics/physics_friction.png" alt="physics_friction" width="200">       | [Victor Fisac](https://github.com/victorfisac) |        |
| 109 | [physics_movement](physics/physics_movement.c)       | <img src="physics/physics_movement.png" alt="physics_movement" width="200">       | [Victor Fisac](https://github.com/victorfisac) |        |
| 110 | [physics_restitution](physics/physics_restitution.c) | <img src="physics/physics_restitution.png" alt="physics_restitution" width="200"> | [Victor Fisac](https://github.com/victorfisac) |        |
| 111 | [physics_shatter](physics/physics_shatter.c)         | <img src="physics/physics_shatter.png" alt="physics_shatter" width="200">         | [Victor Fisac](https://github.com/victorfisac) |        |

### category: others

Examples showing raylib misc functionality that does not fit in other categories, like standalone modules usage or examples integrating external libraries.

| ## | example  | image  | developer  | new |
|----|----------|--------|:----------:|:---:|
| 119 | [raudio_standalone](others/raudio_standalone.c) |       | ray         |        |
| 120 | [rlgl_standalone](others/rlgl_standalone.c)     |       | ray         |        |
| 121 | [easings_testbed](others/easings_testbed.c)     |       | [Juan Miguel López](https://github.com/flashback-fx)       |        |
| 122 | [embedded_files_loading](others/embedded_files_loading.c) |    | [Kristian Holmgren](https://github.com/defutura)   |        |

As always contributions are welcome, feel free to send new examples! Here it is an [examples template](examples_template.c) to start with!

