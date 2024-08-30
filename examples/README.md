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

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 01 | [core_basic_window](core/core_basic_window.c) | <img src="core/core_basic_window.png" alt="core_basic_window" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 02 | [core_input_keys](core/core_input_keys.c) | <img src="core/core_input_keys.png" alt="core_input_keys" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 03 | [core_input_mouse](core/core_input_mouse.c) | <img src="core/core_input_mouse.png" alt="core_input_mouse" width="80"> | ⭐️☆☆☆ | 1.0 | **4.0** | [Ray](https://github.com/raysan5) |
| 04 | [core_input_mouse_wheel](core/core_input_mouse_wheel.c) | <img src="core/core_input_mouse_wheel.png" alt="core_input_mouse_wheel" width="80"> | ⭐️☆☆☆ | 1.1 | 1.3 | [Ray](https://github.com/raysan5) |
| 05 | [core_input_gamepad](core/core_input_gamepad.c) | <img src="core/core_input_gamepad.png" alt="core_input_gamepad" width="80"> | ⭐️☆☆☆ | 1.1 | **4.2** | [Ray](https://github.com/raysan5) |
| 06 | [core_input_multitouch](core/core_input_multitouch.c) | <img src="core/core_input_multitouch.png" alt="core_input_multitouch" width="80"> | ⭐️☆☆☆ | 2.1 | 2.5 | [Berni](https://github.com/Berni8k) |
| 07 | [core_input_gestures](core/core_input_gestures.c) | <img src="core/core_input_gestures.png" alt="core_input_gestures" width="80"> | ⭐️⭐️☆☆ | 1.4 | **4.2** | [Ray](https://github.com/raysan5) |
| 08 | [core_2d_camera](core/core_2d_camera.c) | <img src="core/core_2d_camera.png" alt="core_2d_camera" width="80"> | ⭐️⭐️☆☆ | 1.5 | 3.0 | [Ray](https://github.com/raysan5) |
| 09 | [core_2d_camera_mouse_zoom](core/core_2d_camera_mouse_zoom.c) | <img src="core/core_2d_camera_mouse_zoom.png" alt="core_2d_camera_mouse_zoom" width="80"> | ⭐️⭐️☆☆ | **4.2** | **4.2** | [Jeffery Myers](https://github.com/JeffM2501) |
| 10 | [core_2d_camera_platformer](core/core_2d_camera_platformer.c) | <img src="core/core_2d_camera_platformer.png" alt="core_2d_camera_platformer" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 3.0 | [avyy](https://github.com/avyy) |
| 11 | [core_2d_camera_split_screen](core/core_2d_camera_split_screen.c) | <img src="core/core_2d_camera_split_screen.png" alt="core_2d_camera_split_screen" width="80"> | ⭐️⭐️⭐️⭐️ | **4.5** | **4.5** | [Gabriel dos Santos Sanches](https://github.com/gabrielssanches) |
| 12 | [core_3d_camera_mode](core/core_3d_camera_mode.c) | <img src="core/core_3d_camera_mode.png" alt="core_3d_camera_mode" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 13 | [core_3d_camera_free](core/core_3d_camera_free.c) | <img src="core/core_3d_camera_free.png" alt="core_3d_camera_free" width="80"> | ⭐️☆☆☆ | 1.3 | 1.3 | [Ray](https://github.com/raysan5) |
| 14 | [core_3d_camera_first_person](core/core_3d_camera_first_person.c) | <img src="core/core_3d_camera_first_person.png" alt="core_3d_camera_first_person" width="80"> | ⭐️⭐️☆☆ | 1.3 | 1.3 | [Ray](https://github.com/raysan5) |
| 15 | [core_3d_camera:split_screen](core/core_3d_camera_split_screen.c) | <img src="core/core_3d_camera_split_screen.png" alt="core_3d_camera_split_screen" width="80"> | ⭐️⭐️⭐️⭐️ | 3.7 | **4.0** | [Jeffery Myers](https://github.com/JeffM2501) |
| 16 | [core_3d_picking](core/core_3d_picking.c) | <img src="core/core_3d_picking.png" alt="core_3d_picking" width="80"> | ⭐️⭐️☆☆ | 1.3 | **4.0** | [Ray](https://github.com/raysan5) |
| 17 | [core_world_screen](core/core_world_screen.c) | <img src="core/core_world_screen.png" alt="core_world_screen" width="80"> | ⭐️⭐️☆☆ | 1.3 | 1.4 | [Ray](https://github.com/raysan5) |
| 18 | [core_custom_logging](core/core_custom_logging.c) | <img src="core/core_custom_logging.png" alt="core_custom_logging" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 2.5 | [Pablo Marcos Oltra](https://github.com/pamarcos) |
| 19 | [core_window_flags](core/core_window_flags.c) | <img src="core/core_window_flags.png" alt="core_window_flags" width="80"> | ⭐️⭐️⭐️☆ | 3.5 | 3.5 | [Ray](https://github.com/raysan5) |
| 20 | [core_window_letterbox](core/core_window_letterbox.c) | <img src="core/core_window_letterbox.png" alt="core_window_letterbox" width="80"> | ⭐️⭐️☆☆ | 2.5 | **4.0** | [Anata](https://github.com/anatagawa) |
| 21 | [core_window_should_close](core/core_window_should_close.c) | <img src="core/core_window_should_close.png" alt="core_window_should_close" width="80"> | ⭐️☆☆☆ | **4.2** | **4.2** | [Ray](https://github.com/raysan5) |
| 22 | [core_drop_files](core/core_drop_files.c) | <img src="core/core_drop_files.png" alt="core_drop_files" width="80"> | ⭐️⭐️☆☆ | 1.3 | **4.2** | [Ray](https://github.com/raysan5) |
| 23 | [core_random_values](core/core_random_values.c) | <img src="core/core_random_values.png" alt="core_random_values" width="80"> | ⭐️☆☆☆ | 1.1 | 1.1 | [Ray](https://github.com/raysan5) |
| 24 | [core_storage_values](core/core_storage_values.c) | <img src="core/core_storage_values.png" alt="core_storage_values" width="80"> | ⭐️⭐️☆☆ | 1.4 | **4.2** | [Ray](https://github.com/raysan5) |
| 25 | [core_vr_simulator](core/core_vr_simulator.c) | <img src="core/core_vr_simulator.png" alt="core_vr_simulator" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | **4.0** | [Ray](https://github.com/raysan5) |
| 26 | [core_loading_thread](core/core_loading_thread.c) | <img src="core/core_loading_thread.png" alt="core_loading_thread" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 3.0 | [Ray](https://github.com/raysan5) |
| 27 | [core_scissor_test](core/core_scissor_test.c) | <img src="core/core_scissor_test.png" alt="core_scissor_test" width="80"> | ⭐️☆☆☆ | 2.5 | 3.0 | [Chris Dill](https://github.com/MysteriousSpace) |
| 28 | [core_basic_screen_manager](core/core_basic_screen_manager.c) | <img src="core/core_basic_screen_manager.png" alt="core_basic_screen_manager" width="80"> | ⭐️☆☆☆ | **4.0** | **4.0** | [Ray](https://github.com/raysan5) |
| 29 | [core_custom_frame_control](core/core_custom_frame_control.c) | <img src="core/core_custom_frame_control.png" alt="core_custom_frame_control" width="80"> | ⭐️⭐️⭐️⭐️ | **4.0** | **4.0** | [Ray](https://github.com/raysan5) |
| 30 | [core_smooth_pixelperfect](core/core_smooth_pixelperfect.c) | <img src="core/core_smooth_pixelperfect.png" alt="core_smooth_pixelperfect" width="80"> | ⭐️⭐️⭐️☆ | 3.7 | **4.0** | [Giancamillo Alessandroni](https://github.com/NotManyIdeasDev) |
| 31 | [core_window_should_close](core/core_window_should_close.c) | <img src="core/core_window_should_close.png" alt="core_window_should_close" width="80"> | ⭐️⭐️☆☆ | **4.2** | **4.2** | [Ray](https://github.com/raysan5) |
| 32 | [core_random_sequence](core/core_random_sequence.c) | <img src="core/core_random_sequence.png" alt="core_random_sequence" width="80"> | ⭐️☆☆☆ | **5.0** | **5.0** | [REDl3east](https://github.com/REDl3east) |

### category: shapes

Examples using raylib shapes drawing functionality, provided by raylib [shapes](../src/shapes.c) module.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 31 | [shapes_basic_shapes](shapes/shapes_basic_shapes.c) | <img src="shapes/shapes_basic_shapes.png" alt="shapes_basic_shapes" width="80"> | ⭐️☆☆☆ | 1.0 | **4.0** | [Ray](https://github.com/raysan5) |
| 32 | [shapes_bouncing_ball](shapes/shapes_bouncing_ball.c) | <img src="shapes/shapes_bouncing_ball.png" alt="shapes_bouncing_ball" width="80"> | ⭐️☆☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 33 | [shapes_colors_palette](shapes/shapes_colors_palette.c) | <img src="shapes/shapes_colors_palette.png" alt="shapes_colors_palette" width="80"> | ⭐️⭐️☆☆ | 1.0 | 2.5 | [Ray](https://github.com/raysan5) |
| 34 | [shapes_logo_raylib](shapes/shapes_logo_raylib.c) | <img src="shapes/shapes_logo_raylib.png" alt="shapes_logo_raylib" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 35 | [shapes_logo_raylib_anim](shapes/shapes_logo_raylib_anim.c) | <img src="shapes/shapes_logo_raylib_anim.png" alt="shapes_logo_raylib_anim" width="80"> | ⭐️⭐️☆☆ | 2.5 | **4.0** | [Ray](https://github.com/raysan5) |
| 36 | [shapes_rectangle_scaling](shapes/shapes_rectangle_scaling.c) | <img src="shapes/shapes_rectangle_scaling.png" alt="shapes_rectangle_scaling" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| 37 | [shapes_lines_bezier](shapes/shapes_lines_bezier.c) | <img src="shapes/shapes_lines_bezier.png" alt="shapes_lines_bezier" width="80"> | ⭐️☆☆☆ | 1.7 | 1.7 | [Ray](https://github.com/raysan5) |
| 38 | [shapes_collision_area](shapes/shapes_collision_area.c) | <img src="shapes/shapes_collision_area.png" alt="shapes_collision_area" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 39 | [shapes_following_eyes](shapes/shapes_following_eyes.c) | <img src="shapes/shapes_following_eyes.png" alt="shapes_following_eyes" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 40 | [shapes_easings_ball_anim](shapes/shapes_easings_ball_anim.c) | <img src="shapes/shapes_easings_ball_anim.png" alt="shapes_easings_ball_anim" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 41 | [shapes_easings_box_anim](shapes/shapes_easings_box_anim.c) | <img src="shapes/shapes_easings_box_anim.png" alt="shapes_easings_box_anim" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 42 | [shapes_easings_rectangle_array](shapes/shapes_easings_rectangle_array.c) | <img src="shapes/shapes_easings_rectangle_array.png" alt="shapes_easings_rectangle_array" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 43 | [shapes_draw_ring](shapes/shapes_draw_ring.c) | <img src="shapes/shapes_draw_ring.png" alt="shapes_draw_ring" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| 44 | [shapes_draw_circle_sector](shapes/shapes_draw_circle_sector.c) | <img src="shapes/shapes_draw_circle_sector.png" alt="shapes_draw_circle_sector" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| 45 | [shapes_draw_rectangle_rounded](shapes/shapes_draw_rectangle_rounded.c) | <img src="shapes/shapes_draw_rectangle_rounded.png" alt="shapes_draw_rectangle_rounded" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| 46 | [shapes_top_down_lights](shapes/shapes_top_down_lights.c) | <img src="shapes/shapes_top_down_lights.png" alt="shapes_top_down_lights" width="80"> | ⭐️⭐️⭐️⭐️ | **4.2** | **4.2** | [Jeffery Myers](https://github.com/JeffM2501) |

### category: textures

Examples using raylib textures functionality, including image/textures loading/generation and drawing, provided by raylib [textures](../src/textures.c) module.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 47 | [textures_logo_raylib](textures/textures_logo_raylib.c) | <img src="textures/textures_logo_raylib.png" alt="textures_logo_raylib" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 48 | [textures_srcrec_dstrec](textures/textures_srcrec_dstrec.c) | <img src="textures/textures_srcrec_dstrec.png" alt="textures_srcrec_dstrec" width="80"> | ⭐️⭐️⭐️☆ | 1.3 | 1.3 | [Ray](https://github.com/raysan5) |
| 49 | [textures_image_drawing](textures/textures_image_drawing.c) | <img src="textures/textures_image_drawing.png" alt="textures_image_drawing" width="80"> | ⭐️⭐️☆☆ | 1.4 | 1.4 | [Ray](https://github.com/raysan5) |
| 50 | [textures_image_generation](textures/textures_image_generation.c) | <img src="textures/textures_image_generation.png" alt="textures_image_generation" width="80"> | ⭐️⭐️☆☆ | 1.8 | 1.8 | [Ray](https://github.com/raysan5) |
| 51 | [textures_image_loading](textures/textures_image_loading.c) | <img src="textures/textures_image_loading.png" alt="textures_image_loading" width="80"> | ⭐️☆☆☆ | 1.3 | 1.3 | [Ray](https://github.com/raysan5) |
| 52 | [textures_image_processing](textures/textures_image_processing.c) | <img src="textures/textures_image_processing.png" alt="textures_image_processing" width="80"> | ⭐️⭐️⭐️☆ | 1.4 | 3.5 | [Ray](https://github.com/raysan5) |
| 53 | [textures_image_text](textures/textures_image_text.c) | <img src="textures/textures_image_text.png" alt="textures_image_text" width="80"> | ⭐️⭐️☆☆ | 1.8 | **4.0** | [Ray](https://github.com/raysan5) |
| 54 | [textures_to_image](textures/textures_to_image.c) | <img src="textures/textures_to_image.png" alt="textures_to_image" width="80"> | ⭐️☆☆☆ | 1.3 | **4.0** | [Ray](https://github.com/raysan5) |
| 55 | [textures_raw_data](textures/textures_raw_data.c) | <img src="textures/textures_raw_data.png" alt="textures_raw_data" width="80"> | ⭐️⭐️⭐️☆ | 1.3 | 3.5 | [Ray](https://github.com/raysan5) |
| 56 | [textures_particles_blending](textures/textures_particles_blending.c) | <img src="textures/textures_particles_blending.png" alt="textures_particles_blending" width="80"> | ⭐️☆☆☆ | 1.7 | 3.5 | [Ray](https://github.com/raysan5) |
| 57 | [textures_npatch_drawing](textures/textures_npatch_drawing.c) | <img src="textures/textures_npatch_drawing.png" alt="textures_npatch_drawing" width="80"> | ⭐️⭐️⭐️☆ | 2.0 | 2.5 | [Jorge A. Gomes](https://github.com/overdev) |
| 58 | [textures_background_scrolling](textures/textures_background_scrolling.c) | <img src="textures/textures_background_scrolling.png" alt="textures_background_scrolling" width="80"> | ⭐️☆☆☆ | 2.0 | 2.5 | [Ray](https://github.com/raysan5) |
| 59 | [textures_sprite_anim](textures/textures_sprite_anim.c) | <img src="textures/textures_sprite_anim.png" alt="textures_sprite_anim" width="80"> | ⭐️⭐️☆☆ | 1.3 | 1.3 | [Ray](https://github.com/raysan5) |
| 60 | [textures_sprite_button](textures/textures_sprite_button.c) | <img src="textures/textures_sprite_button.png" alt="textures_sprite_button" width="80"> | ⭐️⭐️☆☆ | 2.5 | 2.5 | [Ray](https://github.com/raysan5) |
| 61 | [textures_sprite_explosion](textures/textures_sprite_explosion.c) | <img src="textures/textures_sprite_explosion.png" alt="textures_sprite_explosion" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.5 | [Ray](https://github.com/raysan5) |
| 62 | [textures_bunnymark](textures/textures_bunnymark.c) | <img src="textures/textures_bunnymark.png" alt="textures_bunnymark" width="80"> | ⭐️⭐️⭐️☆ | 1.6 | 2.5 | [Ray](https://github.com/raysan5) |
| 63 | [textures_mouse_painting](textures/textures_mouse_painting.c) | <img src="textures/textures_mouse_painting.png" alt="textures_mouse_painting" width="80"> | ⭐️⭐️⭐️☆ | 3.0 | 3.0 | [Chris Dill](https://github.com/MysteriousSpace) |
| 64 | [textures_blend_modes](textures/textures_blend_modes.c) | <img src="textures/textures_blend_modes.png" alt="textures_blend_modes" width="80"> | ⭐️☆☆☆ | 3.5 | 3.5 | [Karlo Licudine](https://github.com/accidentalrebel) |
| 65 | [textures_draw_tiled](textures/textures_draw_tiled.c) | <img src="textures/textures_draw_tiled.png" alt="textures_draw_tiled" width="80"> | ⭐️⭐️⭐️☆ | 3.0 | **4.2** | [Vlad Adrian](https://github.com/demizdor) |
| 66 | [textures_polygon](textures/textures_polygon.c) | <img src="textures/textures_polygon.png" alt="textures_polygon" width="80"> | ⭐️☆☆☆ | 3.7 | 3.7 | [Chris Camacho](https://github.com/codifies) |
| 67 | [textures_fog_of_war](textures/textures_fog_of_war.c) | <img src="textures/textures_fog_of_war.png" alt="textures_fog_of_war" width="80"> | ⭐️⭐️⭐️☆ | **4.2** | **4.2** | [Ray](https://github.com/raysan5) |
| 68 | [textures_gif_player](textures/textures_gif_player.c) | <img src="textures/textures_gif_player.png" alt="textures_gif_player" width="80"> | ⭐️⭐️⭐️☆ | **4.2** | **4.2** | [Ray](https://github.com/raysan5) |

### category: text

Examples using raylib text functionality, including sprite fonts loading/generation and text drawing, provided by raylib [text](../src/text.c) module.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 69 | [text_raylib_fonts](text/text_raylib_fonts.c) | <img src="text/text_raylib_fonts.png" alt="text_raylib_fonts" width="80"> | ⭐️☆☆☆ | 1.7 | 3.7 | [Ray](https://github.com/raysan5) |
| 70 | [text_font_spritefont](text/text_font_spritefont.c) | <img src="text/text_font_spritefont.png" alt="text_font_spritefont" width="80"> | ⭐️☆☆☆ | 1.0 | 1.0 | [Ray](https://github.com/raysan5) |
| 71 | [text_font_filters](text/text_font_filters.c) | <img src="text/text_font_filters.png" alt="text_font_filters" width="80"> | ⭐️⭐️☆☆ | 1.3 | **4.2** | [Ray](https://github.com/raysan5) |
| 72 | [text_font_loading](text/text_font_loading.c) | <img src="text/text_font_loading.png" alt="text_font_loading" width="80"> | ⭐️☆☆☆ | 1.4 | 3.0 | [Ray](https://github.com/raysan5) |
| 73 | [text_font_sdf](text/text_font_sdf.c) | <img src="text/text_font_sdf.png" alt="text_font_sdf" width="80"> | ⭐️⭐️⭐️☆ | 1.3 | **4.0** | [Ray](https://github.com/raysan5) |
| 74 | [text_format_text](text/text_format_text.c) | <img src="text/text_format_text.png" alt="text_format_text" width="80"> | ⭐️☆☆☆ | 1.1 | 3.0 | [Ray](https://github.com/raysan5) |
| 75 | [text_input_box](text/text_input_box.c) | <img src="text/text_input_box.png" alt="text_input_box" width="80"> | ⭐️⭐️☆☆ | 1.7 | 3.5 | [Ray](https://github.com/raysan5) |
| 76 | [text_writing_anim](text/text_writing_anim.c) | <img src="text/text_writing_anim.png" alt="text_writing_anim" width="80"> | ⭐️⭐️☆☆ | 1.4 | 1.4 | [Ray](https://github.com/raysan5) |
| 77 | [text_rectangle_bounds](text/text_rectangle_bounds.c) | <img src="text/text_rectangle_bounds.png" alt="text_rectangle_bounds" width="80"> | ⭐️⭐️⭐️⭐️ | 2.5 | **4.0** | [Vlad Adrian](https://github.com/demizdor) |
| 78 | [text_unicode](text/text_unicode.c) | <img src="text/text_unicode.png" alt="text_unicode" width="80"> | ⭐️⭐️⭐️⭐️ | 2.5 | **4.0** | [Vlad Adrian](https://github.com/demizdor) |
| 79 | [text_draw_3d](text/text_draw_3d.c) | <img src="text/text_draw_3d.png" alt="text_draw_3d" width="80"> | ⭐️⭐️⭐️⭐️ | 3.5 | **4.0** | [Vlad Adrian](https://github.com/demizdor) |
| 80 | [text_codepoints_loading](text/text_codepoints_loading.c) | <img src="text/text_codepoints_loading.png" alt="text_codepoints_loading" width="80"> | ⭐️⭐️⭐️☆ | **4.2** | **4.2** | [Ray](https://github.com/raysan5) |

### category: models

Examples using raylib models functionality, including models loading/generation and drawing, provided by raylib [models](../src/models.c) module.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 81 | [models_animation](models/models_animation.c) | <img src="models/models_animation.png" alt="models_animation" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.5 | [culacant](https://github.com/culacant) |
| 82 | [models_billboard](models/models_billboard.c) | <img src="models/models_billboard.png" alt="models_billboard" width="80"> | ⭐️⭐️⭐️☆ | 1.3 | 3.5 | [Ray](https://github.com/raysan5) |
| 83 | [models_box_collisions](models/models_box_collisions.c) | <img src="models/models_box_collisions.png" alt="models_box_collisions" width="80"> | ⭐️☆☆☆ | 1.3 | 3.5 | [Ray](https://github.com/raysan5) |
| 84 | [models_cubicmap](models/models_cubicmap.c) | <img src="models/models_cubicmap.png" alt="models_cubicmap" width="80"> | ⭐️⭐️☆☆ | 1.8 | 3.5 | [Ray](https://github.com/raysan5) |
| 85 | [models_first_person_maze](models/models_first_person_maze.c) | <img src="models/models_first_person_maze.png" alt="models_first_person_maze" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.5 | [Ray](https://github.com/raysan5) |
| 86 | [models_geometric_shapes](models/models_geometric_shapes.c) | <img src="models/models_geometric_shapes.png" alt="models_geometric_shapes" width="80"> | ⭐️☆☆☆ | 1.0 | 3.5 | [Ray](https://github.com/raysan5) |
| 87 | [models_mesh_generation](models/models_mesh_generation.c) | <img src="models/models_mesh_generation.png" alt="models_mesh_generation" width="80"> | ⭐️⭐️☆☆ | 1.8 | **4.0** | [Ray](https://github.com/raysan5) |
| 88 | [models_mesh_picking](models/models_mesh_picking.c) | <img src="models/models_mesh_picking.png" alt="models_mesh_picking" width="80"> | ⭐️⭐️⭐️☆ | 1.7 | **4.0** | [Joel Davis](https://github.com/joeld42) |
| 89 | [models_loading](models/models_loading.c) | <img src="models/models_loading.png" alt="models_loading" width="80"> | ⭐️☆☆☆ | 2.5 | **4.0** | [Ray](https://github.com/raysan5) |
| 90 | [models_loading_gltf](models/models_loading_gltf.c) | <img src="models/models_loading_gltf.png" alt="models_loading_gltf" width="80"> | ⭐️☆☆☆ | 3.7 | **4.2** | [Ray](https://github.com/raysan5) |
| 91 | [models_loading_vox](models/models_loading_vox.c) | <img src="models/models_loading_vox.png" alt="models_loading_vox" width="80"> | ⭐️☆☆☆ | **4.0** | **4.0** | [Johann Nadalutti](https://github.com/procfxgen) |
| 92 | [models_loading_m3d](models/models_loading_m3d.c) | <img src="models/models_loading_m3d.png" alt="models_loading_m3d" width="80"> | ⭐️☆☆☆ | **4.2** | **4.2** | [bzt](https://bztsrc.gitlab.io/model3d) |
| 93 | [models_orthographic_projection](models/models_orthographic_projection.c) | <img src="models/models_orthographic_projection.png" alt="models_orthographic_projection" width="80"> | ⭐️☆☆☆ | 2.0 | 3.7 | [Max Danielsson](https://github.com/autious) |
| 94 | [models_rlgl_solar_system](models/models_rlgl_solar_system.c) | <img src="models/models_rlgl_solar_system.png" alt="models_rlgl_solar_system" width="80"> | ⭐️⭐️⭐️⭐️ | 2.5 | **4.0** | [Ray](https://github.com/raysan5) |
| 95 | [models_yaw_pitch_roll](models/models_yaw_pitch_roll.c) | <img src="models/models_yaw_pitch_roll.png" alt="models_yaw_pitch_roll" width="80"> | ⭐️⭐️☆☆ | 1.8 | **4.0** | [Berni](https://github.com/Berni8k) |
| 96 | [models_waving_cubes](models/models_waving_cubes.c) | <img src="models/models_waving_cubes.png" alt="models_waving_cubes" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 3.7 | [codecat](https://github.com/codecat) |
| 97 | [models_heightmap](models/models_heightmap.c) | <img src="models/models_heightmap.png" alt="models_heightmap" width="80"> | ⭐️☆☆☆ | 1.8 | 3.5 | [Ray](https://github.com/raysan5) |
| 98 | [models_skybox](models/models_skybox.c) | <img src="models/models_skybox.png" alt="models_skybox" width="80"> | ⭐️⭐️☆☆ | 1.8 | **4.0** | [Ray](https://github.com/raysan5) |

### category: shaders

Examples using raylib shaders functionality, including shaders loading, parameters configuration and drawing using them (model shaders and postprocessing shaders). This functionality is directly provided by raylib [rlgl](../src/rlgl.c) module.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 99  | [shaders_basic_lighting](shaders/shaders_basic_lighting.c) | <img src="shaders/shaders_basic_lighting.png" alt="shaders_basic_lighting" width="80"> | ⭐️⭐️⭐️⭐️ | 3.0 | **4.2** | [Chris Camacho](https://github.com/codifies) |
| 100 | [shaders_model_shader](shaders/shaders_model_shader.c) | <img src="shaders/shaders_model_shader.png" alt="shaders_model_shader" width="80"> | ⭐️⭐️☆☆ | 1.3 | 3.7 | [Ray](https://github.com/raysan5) |
| 101 | [shaders_shapes_textures](shaders/shaders_shapes_textures.c) | <img src="shaders/shaders_shapes_textures.png" alt="shaders_shapes_textures" width="80"> | ⭐️⭐️☆☆ | 1.7 | 3.7 | [Ray](https://github.com/raysan5) |
| 102 | [shaders_custom_uniform](shaders/shaders_custom_uniform.c) | <img src="shaders/shaders_custom_uniform.png" alt="shaders_custom_uniform" width="80"> | ⭐️⭐️☆☆ | 1.3 | **4.0** | [Ray](https://github.com/raysan5) |
| 103 | [shaders_postprocessing](shaders/shaders_postprocessing.c) | <img src="shaders/shaders_postprocessing.png" alt="shaders_postprocessing" width="80"> | ⭐️⭐️⭐️☆ | 1.3 | **4.0** | [Ray](https://github.com/raysan5) |
| 104 | [shaders_palette_switch](shaders/shaders_palette_switch.c) | <img src="shaders/shaders_palette_switch.png" alt="shaders_palette_switch" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 3.7 | [Marco Lizza](https://github.com/MarcoLizza) |
| 105 | [shaders_raymarching](shaders/shaders_raymarching.c) | <img src="shaders/shaders_raymarching.png" alt="shaders_raymarching" width="80"> | ⭐️⭐️⭐️⭐️ | 2.0 | **4.2** | [Ray](https://github.com/raysan5) |
| 106 | [shaders_texture_drawing](shaders/shaders_texture_drawing.c) | <img src="shaders/shaders_texture_drawing.png" alt="shaders_texture_drawing" width="80"> | ⭐️⭐️☆☆ | 2.0 | 3.7 | [Michał Ciesielski](https://github.com/) |
| 107 | [shaders_texture_outline](shaders/shaders_texture_outline.c) | <img src="shaders/shaders_texture_outline.png" alt="shaders_texture_outline" width="80"> | ⭐️⭐️⭐️☆ | **4.0** | **4.0** | [Samuel Skiff](https://github.com/GoldenThumbs) |
| 108 | [shaders_texture_waves](shaders/shaders_texture_waves.c) | <img src="shaders/shaders_texture_waves.png" alt="shaders_texture_waves" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.7 | [Anata](https://github.com/anatagawa) |
| 109 | [shaders_julia_set](shaders/shaders_julia_set.c) | <img src="shaders/shaders_julia_set.png" alt="shaders_julia_set" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | **4.0** | [eggmund](https://github.com/eggmund) |
| 110 | [shaders_eratosthenes](shaders/shaders_eratosthenes.c) | <img src="shaders/shaders_eratosthenes.png" alt="shaders_eratosthenes" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | **4.0** | [ProfJski](https://github.com/ProfJski) |
| 111 | [shaders_fog](shaders/shaders_fog.c) | <img src="shaders/shaders_fog.png" alt="shaders_fog" width="80"> | ⭐️⭐️⭐️☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/codifies) |
| 112 | [shaders_simple_mask](shaders/shaders_simple_mask.c) | <img src="shaders/shaders_simple_mask.png" alt="shaders_simple_mask" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/codifies) |
| 113 | [shaders_hot_reloading](shaders/shaders_hot_reloading.c) | <img src="shaders/shaders_hot_reloading.png" alt="shaders_hot_reloading" width="80"> | ⭐️⭐️⭐️☆ | 3.0 | 3.5 | [Ray](https://github.com/raysan5) |
| 114 | [shaders_mesh_instancing](shaders/shaders_mesh_instancing.c) | <img src="shaders/shaders_mesh_instancing.png" alt="shaders_mesh_instancing" width="80"> | ⭐️⭐️⭐️⭐️ | 3.7 | **4.2** | [seanpringle](https://github.com/seanpringle) |
| 115 | [shaders_multi_sample2d](shaders/shaders_multi_sample2d.c) | <img src="shaders/shaders_multi_sample2d.png" alt="shaders_multi_sample2d" width="80"> | ⭐️⭐️☆☆ | 3.5 | 3.5 | [Ray](https://github.com/raysan5) |
| 116 | [shaders_spotlight](shaders/shaders_spotlight.c) | <img src="shaders/shaders_spotlight.png" alt="shaders_spotlight" width="80"> | ⭐️⭐️☆☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/codifies) |
| 117 | [shaders_deferred_render](shaders/shaders_deferred_render.c) | <img src="shaders/shaders_deferred_render.png" alt="shaders_deferred_render" width="80"> | ⭐️⭐️⭐️⭐️ | 4.5 | 4.5 | [Justin Andreas Lacoste](https://github.com/27justin) |

### category: audio

Examples using raylib audio functionality, including sound/music loading and playing. This functionality is provided by raylib [raudio](../src/raudio.c) module. Note this module can be used standalone independently of raylib, check [raudio_standalone](others/raudio_standalone.c) example.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 118 | [audio_module_playing](audio/audio_module_playing.c) | <img src="audio/audio_module_playing.png" alt="audio_module_playing" width="80"> | ⭐️☆☆☆ | 1.5 | 3.5 | [Ray](https://github.com/raysan5) |
| 119 | [audio_music_stream](audio/audio_music_stream.c) | <img src="audio/audio_music_stream.png" alt="audio_music_stream" width="80"> | ⭐️☆☆☆ | 1.3 | **4.2** | [Ray](https://github.com/raysan5) |
| 120 | [audio_raw_stream](audio/audio_raw_stream.c) | <img src="audio/audio_raw_stream.png" alt="audio_raw_stream" width="80"> | ⭐️⭐️⭐️☆ | 1.6 | **4.2** | [Ray](https://github.com/raysan5) |
| 121 | [audio_sound_loading](audio/audio_sound_loading.c) | <img src="audio/audio_sound_loading.png" alt="audio_sound_loading" width="80"> | ⭐️☆☆☆ | 1.1 | 3.5 | [Ray](https://github.com/raysan5) |

### category: others

Examples showing raylib misc functionality that does not fit in other categories, like standalone modules usage or examples integrating external libraries.

| ## | example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|----|----------|--------|:-------------------:|:------------------:|:------------------:|:----------|
| 122 | [rlgl_standalone](others/rlgl_standalone.c) | <img src="others/rlgl_standalone.png" alt="rlgl_standalone" width="80"> | ⭐️⭐️⭐️⭐️ | 1.6 | **4.0** | [Ray](https://github.com/raysan5) |
| 123 | [rlgl_compute_shader](others/rlgl_compute_shader.c) | <img src="others/rlgl_compute_shader.png" alt="rlgl_compute_shader" width="80"> | ⭐️⭐️⭐️⭐️ | **4.0** | **4.0** | [Teddy Astie](https://github.com/tsnake41) |
| 124 | [easings_testbed](others/easings_testbed.c) | <img src="others/easings_testbed.png" alt="easings_testbed" width="80"> | ⭐️⭐️⭐️☆ | 3.0 | 3.0 | [Juan Miguel López](https://github.com/flashback-fx) |
| 125 | [raylib_opengl_interop](others/raylib_opengl_interop.c) | <img src="others/raylib_opengl_interop.png" alt="raylib_opengl_interop" width="80"> | ⭐️⭐️⭐️⭐️ | **4.0** | **4.0** | [Stephan Soller](https://github.com/arkanis) |
| 126 | [embedded_files_loading](others/embedded_files_loading.c) | <img src="others/embedded_files_loading.png" alt="embedded_files_loading" width="80"> | ⭐️⭐️☆☆ | 3.5 | 3.5 | [Kristian Holmgren](https://github.com/defutura) |

As always contributions are welcome, feel free to send new examples! Here is an [examples template](examples_template.c) to start with!

