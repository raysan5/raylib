## Building the Examples

The examples assume you have already built the `raylib` library in `../src`.

### With GNU make

- `make` builds all examples
- `make [module]` builds all examples for a particular module (e.g `make core`)
- `make [module]/[name]` builds one examples for a particular module (e.g `make core/core_basic_window`)

### With Zig

The [Zig](https://ziglang.org/) toolchain can compile `C` and `C++` in addition to `Zig`.
You may find it easier to use than other toolchains, especially when it comes to cross-compiling.

- `zig build` to compile all examples
- `zig build [module]` to compile all examples for a module (e.g. `zig build core`)
- `zig build [example]` to compile _and run_ a particular example (e.g. `zig build core_basic_window`)

## EXAMPLES COLLECTION [TOTAL: 208]

### category: core [48]

Examples using raylib [core](../src/rcore.c) module platform functionality: window creation, inputs, drawing modes and system functionality.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [core_basic_window](core/core_basic_window.c) | <img src="core/core_basic_window.png" alt="core_basic_window" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_delta_time](core/core_delta_time.c) | <img src="core/core_delta_time.png" alt="core_delta_time" width="80"> | ⭐☆☆☆ | 5.5 | 5.6-dev | [Robin](https://github.com/RobinsAviary) |
| [core_input_keys](core/core_input_keys.c) | <img src="core/core_input_keys.png" alt="core_input_keys" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_input_mouse](core/core_input_mouse.c) | <img src="core/core_input_mouse.png" alt="core_input_mouse" width="80"> | ⭐☆☆☆ | 1.0 | 5.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_input_mouse_wheel](core/core_input_mouse_wheel.c) | <img src="core/core_input_mouse_wheel.png" alt="core_input_mouse_wheel" width="80"> | ⭐☆☆☆ | 1.1 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_input_gamepad](core/core_input_gamepad.c) | <img src="core/core_input_gamepad.png" alt="core_input_gamepad" width="80"> | ⭐☆☆☆ | 1.1 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_input_multitouch](core/core_input_multitouch.c) | <img src="core/core_input_multitouch.png" alt="core_input_multitouch" width="80"> | ⭐☆☆☆ | 2.1 | 2.5 | [Berni](https://github.com/Berni8k) |
| [core_input_gestures](core/core_input_gestures.c) | <img src="core/core_input_gestures.png" alt="core_input_gestures" width="80"> | ⭐⭐☆☆ | 1.4 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_input_gestures_testbed](core/core_input_gestures_testbed.c) | <img src="core/core_input_gestures_testbed.png" alt="core_input_gestures_testbed" width="80"> | ⭐⭐⭐☆ | 5.0 | 5.6-dev | [ubkp](https://github.com/ubkp) |
| [core_input_virtual_controls](core/core_input_virtual_controls.c) | <img src="core/core_input_virtual_controls.png" alt="core_input_virtual_controls" width="80"> | ⭐⭐☆☆ | 5.0 | 5.0 | [GreenSnakeLinux](https://github.com/GreenSnakeLinux) |
| [core_2d_camera](core/core_2d_camera.c) | <img src="core/core_2d_camera.png" alt="core_2d_camera" width="80"> | ⭐⭐☆☆ | 1.5 | 3.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_2d_camera_mouse_zoom](core/core_2d_camera_mouse_zoom.c) | <img src="core/core_2d_camera_mouse_zoom.png" alt="core_2d_camera_mouse_zoom" width="80"> | ⭐⭐☆☆ | 4.2 | 4.2 | [Jeffery Myers](https://github.com/JeffM2501) |
| [core_2d_camera_platformer](core/core_2d_camera_platformer.c) | <img src="core/core_2d_camera_platformer.png" alt="core_2d_camera_platformer" width="80"> | ⭐⭐⭐☆ | 2.5 | 3.0 | [arvyy](https://github.com/arvyy) |
| [core_2d_camera_split_screen](core/core_2d_camera_split_screen.c) | <img src="core/core_2d_camera_split_screen.png" alt="core_2d_camera_split_screen" width="80"> | ⭐⭐⭐⭐️ | 4.5 | 4.5 | [Gabriel dos Santos Sanches](https://github.com/gabrielssanches) |
| [core_3d_camera_mode](core/core_3d_camera_mode.c) | <img src="core/core_3d_camera_mode.png" alt="core_3d_camera_mode" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_3d_camera_free](core/core_3d_camera_free.c) | <img src="core/core_3d_camera_free.png" alt="core_3d_camera_free" width="80"> | ⭐☆☆☆ | 1.3 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_3d_camera_first_person](core/core_3d_camera_first_person.c) | <img src="core/core_3d_camera_first_person.png" alt="core_3d_camera_first_person" width="80"> | ⭐⭐☆☆ | 1.3 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_3d_camera_split_screen](core/core_3d_camera_split_screen.c) | <img src="core/core_3d_camera_split_screen.png" alt="core_3d_camera_split_screen" width="80"> | ⭐⭐⭐☆ | 3.7 | 4.0 | [Jeffery Myers](https://github.com/JeffM2501) |
| [core_3d_camera_fps](core/core_3d_camera_fps.c) | <img src="core/core_3d_camera_fps.png" alt="core_3d_camera_fps" width="80"> | ⭐⭐⭐☆ | 5.5 | 5.5 | [Agnis Aldiņš](https://github.com/nezvers) |
| [core_3d_picking](core/core_3d_picking.c) | <img src="core/core_3d_picking.png" alt="core_3d_picking" width="80"> | ⭐⭐☆☆ | 1.3 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_world_screen](core/core_world_screen.c) | <img src="core/core_world_screen.png" alt="core_world_screen" width="80"> | ⭐⭐☆☆ | 1.3 | 1.4 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_window_flags](core/core_window_flags.c) | <img src="core/core_window_flags.png" alt="core_window_flags" width="80"> | ⭐⭐⭐☆ | 3.5 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_window_letterbox](core/core_window_letterbox.c) | <img src="core/core_window_letterbox.png" alt="core_window_letterbox" width="80"> | ⭐⭐☆☆ | 2.5 | 4.0 | [Anata](https://github.com/anatagawa) |
| [core_window_should_close](core/core_window_should_close.c) | <img src="core/core_window_should_close.png" alt="core_window_should_close" width="80"> | ⭐☆☆☆ | 4.2 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_monitor_detector](core/core_monitor_detector.c) | <img src="core/core_monitor_detector.png" alt="core_monitor_detector" width="80"> | ⭐☆☆☆ | 5.5 | 5.6 | [Maicon Santana](https://github.com/maiconpintoabreu) |
| [core_custom_logging](core/core_custom_logging.c) | <img src="core/core_custom_logging.png" alt="core_custom_logging" width="80"> | ⭐⭐⭐☆ | 2.5 | 2.5 | [Pablo Marcos Oltra](https://github.com/pamarcos) |
| [core_drop_files](core/core_drop_files.c) | <img src="core/core_drop_files.png" alt="core_drop_files" width="80"> | ⭐⭐☆☆ | 1.3 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_random_values](core/core_random_values.c) | <img src="core/core_random_values.png" alt="core_random_values" width="80"> | ⭐☆☆☆ | 1.1 | 1.1 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_storage_values](core/core_storage_values.c) | <img src="core/core_storage_values.png" alt="core_storage_values" width="80"> | ⭐⭐☆☆ | 1.4 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_vr_simulator](core/core_vr_simulator.c) | <img src="core/core_vr_simulator.png" alt="core_vr_simulator" width="80"> | ⭐⭐⭐☆ | 2.5 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_scissor_test](core/core_scissor_test.c) | <img src="core/core_scissor_test.png" alt="core_scissor_test" width="80"> | ⭐☆☆☆ | 2.5 | 3.0 | [Chris Dill](https://github.com/MysteriousSpace) |
| [core_basic_screen_manager](core/core_basic_screen_manager.c) | <img src="core/core_basic_screen_manager.png" alt="core_basic_screen_manager" width="80"> | ⭐☆☆☆ | 4.0 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_custom_frame_control](core/core_custom_frame_control.c) | <img src="core/core_custom_frame_control.png" alt="core_custom_frame_control" width="80"> | ⭐⭐⭐⭐️ | 4.0 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_smooth_pixelperfect](core/core_smooth_pixelperfect.c) | <img src="core/core_smooth_pixelperfect.png" alt="core_smooth_pixelperfect" width="80"> | ⭐⭐⭐☆ | 3.7 | 4.0 | [Giancamillo Alessandroni](https://github.com/NotManyIdeasDev) |
| [core_random_sequence](core/core_random_sequence.c) | <img src="core/core_random_sequence.png" alt="core_random_sequence" width="80"> | ⭐☆☆☆ | 5.0 | 5.0 | [Dalton Overmyer](https://github.com/REDl3east) |
| [core_automation_events](core/core_automation_events.c) | <img src="core/core_automation_events.png" alt="core_automation_events" width="80"> | ⭐⭐⭐☆ | 5.0 | 5.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_highdpi_demo](core/core_highdpi_demo.c) | <img src="core/core_highdpi_demo.png" alt="core_highdpi_demo" width="80"> | ⭐⭐☆☆ | 5.0 | 5.5 | [Jonathan Marler](https://github.com/marler8997) |
| [core_render_texture](core/core_render_texture.c) | <img src="core/core_render_texture.png" alt="core_render_texture" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6-dev | [Ramon Santamaria](https://github.com/raysan5) |
| [core_undo_redo](core/core_undo_redo.c) | <img src="core/core_undo_redo.png" alt="core_undo_redo" width="80"> | ⭐⭐⭐☆ | 5.5 | 5.6 | [Ramon Santamaria](https://github.com/raysan5) |
| [core_viewport_scaling](core/core_viewport_scaling.c) | <img src="core/core_viewport_scaling.png" alt="core_viewport_scaling" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [Agnis Aldiņš](https://github.com/nezvers) |
| [core_input_actions](core/core_input_actions.c) | <img src="core/core_input_actions.png" alt="core_input_actions" width="80"> | ⭐⭐☆☆ | 5.5 | 5.6 | [Jett](https://github.com/JettMonstersGoBoom) |
| [core_directory_files](core/core_directory_files.c) | <img src="core/core_directory_files.png" alt="core_directory_files" width="80"> | ⭐☆☆☆ | 5.5 | 5.6 | [Hugo ARNAL](https://github.com/hugoarnal) |
| [core_highdpi_testbed](core/core_highdpi_testbed.c) | <img src="core/core_highdpi_testbed.png" alt="core_highdpi_testbed" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6-dev | [Ramon Santamaria](https://github.com/raysan5) |
| [core_screen_recording](core/core_screen_recording.c) | <img src="core/core_screen_recording.png" alt="core_screen_recording" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Ramon Santamaria](https://github.com/raysan5) |
| [core_clipboard_text](core/core_clipboard_text.c) | <img src="core/core_clipboard_text.png" alt="core_clipboard_text" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Ananth S](https://github.com/Ananth1839) |
| [core_text_file_loading](core/core_text_file_loading.c) | <img src="core/core_text_file_loading.png" alt="core_text_file_loading" width="80"> | ⭐☆☆☆ | 5.5 | 5.6 | [Aanjishnu Bhattacharyya](https://github.com/NimComPoo-04) |
| [core_compute_hash](core/core_compute_hash.c) | <img src="core/core_compute_hash.png" alt="core_compute_hash" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Ramon Santamaria](https://github.com/raysan5) |
| [core_keyboard_testbed](core/core_keyboard_testbed.c) | <img src="core/core_keyboard_testbed.png" alt="core_keyboard_testbed" width="80"> | ⭐⭐☆☆ | 5.6 | 5.6 | [Ramon Santamaria](https://github.com/raysan5) |

### category: shapes [39]

Examples using raylib shapes drawing functionality, provided by raylib [shapes](../src/rshapes.c) module.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [shapes_basic_shapes](shapes/shapes_basic_shapes.c) | <img src="shapes/shapes_basic_shapes.png" alt="shapes_basic_shapes" width="80"> | ⭐☆☆☆ | 1.0 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_bouncing_ball](shapes/shapes_bouncing_ball.c) | <img src="shapes/shapes_bouncing_ball.png" alt="shapes_bouncing_ball" width="80"> | ⭐☆☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_bullet_hell](shapes/shapes_bullet_hell.c) | <img src="shapes/shapes_bullet_hell.png" alt="shapes_bullet_hell" width="80"> | ⭐☆☆☆ | 5.6 | 5.6 | [Zero](https://github.com/zerohorsepower) |
| [shapes_colors_palette](shapes/shapes_colors_palette.c) | <img src="shapes/shapes_colors_palette.png" alt="shapes_colors_palette" width="80"> | ⭐⭐☆☆ | 1.0 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_logo_raylib](shapes/shapes_logo_raylib.c) | <img src="shapes/shapes_logo_raylib.png" alt="shapes_logo_raylib" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_logo_raylib_anim](shapes/shapes_logo_raylib_anim.c) | <img src="shapes/shapes_logo_raylib_anim.png" alt="shapes_logo_raylib_anim" width="80"> | ⭐⭐☆☆ | 2.5 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_rectangle_scaling](shapes/shapes_rectangle_scaling.c) | <img src="shapes/shapes_rectangle_scaling.png" alt="shapes_rectangle_scaling" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| [shapes_lines_bezier](shapes/shapes_lines_bezier.c) | <img src="shapes/shapes_lines_bezier.png" alt="shapes_lines_bezier" width="80"> | ⭐☆☆☆ | 1.7 | 1.7 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_collision_area](shapes/shapes_collision_area.c) | <img src="shapes/shapes_collision_area.png" alt="shapes_collision_area" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_following_eyes](shapes/shapes_following_eyes.c) | <img src="shapes/shapes_following_eyes.png" alt="shapes_following_eyes" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_easings_ball](shapes/shapes_easings_ball.c) | <img src="shapes/shapes_easings_ball.png" alt="shapes_easings_ball" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_easings_box](shapes/shapes_easings_box.c) | <img src="shapes/shapes_easings_box.png" alt="shapes_easings_box" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_easings_rectangles](shapes/shapes_easings_rectangles.c) | <img src="shapes/shapes_easings_rectangles.png" alt="shapes_easings_rectangles" width="80"> | ⭐⭐⭐☆ | 2.0 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_recursive_tree](shapes/shapes_recursive_tree.c) | <img src="shapes/shapes_recursive_tree.png" alt="shapes_recursive_tree" width="80"> | ⭐⭐⭐☆ | 5.6-dev | 5.6-dev | [Jopestpe](https://github.com/jopestpe) |
| [shapes_ring_drawing](shapes/shapes_ring_drawing.c) | <img src="shapes/shapes_ring_drawing.png" alt="shapes_ring_drawing" width="80"> | ⭐⭐⭐☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| [shapes_circle_sector_drawing](shapes/shapes_circle_sector_drawing.c) | <img src="shapes/shapes_circle_sector_drawing.png" alt="shapes_circle_sector_drawing" width="80"> | ⭐⭐⭐☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| [shapes_rounded_rectangle_drawing](shapes/shapes_rounded_rectangle_drawing.c) | <img src="shapes/shapes_rounded_rectangle_drawing.png" alt="shapes_rounded_rectangle_drawing" width="80"> | ⭐⭐⭐☆ | 2.5 | 2.5 | [Vlad Adrian](https://github.com/demizdor) |
| [shapes_top_down_lights](shapes/shapes_top_down_lights.c) | <img src="shapes/shapes_top_down_lights.png" alt="shapes_top_down_lights" width="80"> | ⭐⭐⭐⭐️ | 4.2 | 4.2 | [Jeffery Myers](https://github.com/JeffM2501) |
| [shapes_rectangle_advanced](shapes/shapes_rectangle_advanced.c) | <img src="shapes/shapes_rectangle_advanced.png" alt="shapes_rectangle_advanced" width="80"> | ⭐⭐⭐⭐️ | 5.5 | 5.5 | [Everton Jr.](https://github.com/evertonse) |
| [shapes_splines_drawing](shapes/shapes_splines_drawing.c) | <img src="shapes/shapes_splines_drawing.png" alt="shapes_splines_drawing" width="80"> | ⭐⭐⭐☆ | 5.0 | 5.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_digital_clock](shapes/shapes_digital_clock.c) | <img src="shapes/shapes_digital_clock.png" alt="shapes_digital_clock" width="80"> | ⭐⭐⭐⭐️ | 5.5 | 5.6 | [Hamza RAHAL](https://github.com/hmz-rhl) |
| [shapes_double_pendulum](shapes/shapes_double_pendulum.c) | <img src="shapes/shapes_double_pendulum.png" alt="shapes_double_pendulum" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [JoeCheong](https://github.com/Joecheong2006) |
| [shapes_dashed_line](shapes/shapes_dashed_line.c) | <img src="shapes/shapes_dashed_line.png" alt="shapes_dashed_line" width="80"> | ⭐☆☆☆ | 5.5 | 5.5 | [Luís Almeida](https://github.com/luis605) |
| [shapes_triangle_strip](shapes/shapes_triangle_strip.c) | <img src="shapes/shapes_triangle_strip.png" alt="shapes_triangle_strip" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Jopestpe](https://github.com/jopestpe) |
| [shapes_vector_angle](shapes/shapes_vector_angle.c) | <img src="shapes/shapes_vector_angle.png" alt="shapes_vector_angle" width="80"> | ⭐⭐☆☆ | 1.0 | 5.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shapes_pie_chart](shapes/shapes_pie_chart.c) | <img src="shapes/shapes_pie_chart.png" alt="shapes_pie_chart" width="80"> | ⭐⭐⭐☆ | 5.5 | 5.6 | [Gideon Serfontein](https://github.com/GideonSerf) |
| [shapes_kaleidoscope](shapes/shapes_kaleidoscope.c) | <img src="shapes/shapes_kaleidoscope.png" alt="shapes_kaleidoscope" width="80"> | ⭐⭐☆☆ | 5.5 | 5.6 | [Hugo ARNAL](https://github.com/hugoarnal) |
| [shapes_clock_of_clocks](shapes/shapes_clock_of_clocks.c) | <img src="shapes/shapes_clock_of_clocks.png" alt="shapes_clock_of_clocks" width="80"> | ⭐⭐☆☆ | 5.5 | 5.6-dev | [JP Mortiboys](https://github.com/themushroompirates) |
| [shapes_math_sine_cosine](shapes/shapes_math_sine_cosine.c) | <img src="shapes/shapes_math_sine_cosine.png" alt="shapes_math_sine_cosine" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Jopestpe](https://github.com/jopestpe) |
| [shapes_mouse_trail](shapes/shapes_mouse_trail.c) | <img src="shapes/shapes_mouse_trail.png" alt="shapes_mouse_trail" width="80"> | ⭐☆☆☆ | 5.6 | 5.6-dev | [Balamurugan R](https://github.com/Bala050814) |
| [shapes_simple_particles](shapes/shapes_simple_particles.c) | <img src="shapes/shapes_simple_particles.png" alt="shapes_simple_particles" width="80"> | ⭐⭐☆☆ | 5.6 | 5.6 | [Jordi Santonja](https://github.com/JordSant) |
| [shapes_starfield_effect](shapes/shapes_starfield_effect.c) | <img src="shapes/shapes_starfield_effect.png" alt="shapes_starfield_effect" width="80"> | ⭐⭐☆☆ | 5.5 | 5.6-dev | [JP Mortiboys](https://github.com/themushroompirates) |
| [shapes_lines_drawing](shapes/shapes_lines_drawing.c) | <img src="shapes/shapes_lines_drawing.png" alt="shapes_lines_drawing" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6 | [Robin](https://github.com/RobinsAviary) |
| [shapes_math_angle_rotation](shapes/shapes_math_angle_rotation.c) | <img src="shapes/shapes_math_angle_rotation.png" alt="shapes_math_angle_rotation" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6 | [Kris](https://github.com/krispy-snacc) |
| [shapes_rlgl_color_wheel](shapes/shapes_rlgl_color_wheel.c) | <img src="shapes/shapes_rlgl_color_wheel.png" alt="shapes_rlgl_color_wheel" width="80"> | ⭐⭐⭐☆ | 5.6-dev | 5.6-dev | [Robin](https://github.com/RobinsAviary) |
| [shapes_rlgl_triangle](shapes/shapes_rlgl_triangle.c) | <img src="shapes/shapes_rlgl_triangle.png" alt="shapes_rlgl_triangle" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Robin](https://github.com/RobinsAviary) |
| [shapes_ball_physics](shapes/shapes_ball_physics.c) | <img src="shapes/shapes_ball_physics.png" alt="shapes_ball_physics" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [David Buzatto](https://github.com/davidbuzatto) |
| [shapes_penrose_tile](shapes/shapes_penrose_tile.c) | <img src="shapes/shapes_penrose_tile.png" alt="shapes_penrose_tile" width="80"> | ⭐⭐⭐⭐️ | 5.5 | 5.6-dev | [David Buzatto](https://github.com/davidbuzatto) |
| [shapes_hilbert_curve](shapes/shapes_hilbert_curve.c) | <img src="shapes/shapes_hilbert_curve.png" alt="shapes_hilbert_curve" width="80"> | ⭐⭐⭐☆ | 5.6 | 5.6 | [Hamza RAHAL](https://github.com/hmz-rhl) |

### category: textures [30]

Examples using raylib textures functionality, including image/textures loading/generation and drawing, provided by raylib [textures](../src/rtextures.c) module.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [textures_logo_raylib](textures/textures_logo_raylib.c) | <img src="textures/textures_logo_raylib.png" alt="textures_logo_raylib" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_srcrec_dstrec](textures/textures_srcrec_dstrec.c) | <img src="textures/textures_srcrec_dstrec.png" alt="textures_srcrec_dstrec" width="80"> | ⭐⭐⭐☆ | 1.3 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_image_drawing](textures/textures_image_drawing.c) | <img src="textures/textures_image_drawing.png" alt="textures_image_drawing" width="80"> | ⭐⭐☆☆ | 1.4 | 1.4 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_image_generation](textures/textures_image_generation.c) | <img src="textures/textures_image_generation.png" alt="textures_image_generation" width="80"> | ⭐⭐☆☆ | 1.8 | 1.8 | [Wilhem Barbier](https://github.com/nounoursheureux) |
| [textures_image_loading](textures/textures_image_loading.c) | <img src="textures/textures_image_loading.png" alt="textures_image_loading" width="80"> | ⭐☆☆☆ | 1.3 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_image_processing](textures/textures_image_processing.c) | <img src="textures/textures_image_processing.png" alt="textures_image_processing" width="80"> | ⭐⭐⭐☆ | 1.4 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_image_text](textures/textures_image_text.c) | <img src="textures/textures_image_text.png" alt="textures_image_text" width="80"> | ⭐⭐☆☆ | 1.8 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_to_image](textures/textures_to_image.c) | <img src="textures/textures_to_image.png" alt="textures_to_image" width="80"> | ⭐☆☆☆ | 1.3 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_raw_data](textures/textures_raw_data.c) | <img src="textures/textures_raw_data.png" alt="textures_raw_data" width="80"> | ⭐⭐⭐☆ | 1.3 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_particles_blending](textures/textures_particles_blending.c) | <img src="textures/textures_particles_blending.png" alt="textures_particles_blending" width="80"> | ⭐☆☆☆ | 1.7 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_npatch_drawing](textures/textures_npatch_drawing.c) | <img src="textures/textures_npatch_drawing.png" alt="textures_npatch_drawing" width="80"> | ⭐⭐⭐☆ | 2.0 | 2.5 | [Jorge A. Gomes](https://github.com/overdev) |
| [textures_background_scrolling](textures/textures_background_scrolling.c) | <img src="textures/textures_background_scrolling.png" alt="textures_background_scrolling" width="80"> | ⭐☆☆☆ | 2.0 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_sprite_animation](textures/textures_sprite_animation.c) | <img src="textures/textures_sprite_animation.png" alt="textures_sprite_animation" width="80"> | ⭐⭐☆☆ | 1.3 | 1.3 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_sprite_button](textures/textures_sprite_button.c) | <img src="textures/textures_sprite_button.png" alt="textures_sprite_button" width="80"> | ⭐⭐☆☆ | 2.5 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_sprite_explosion](textures/textures_sprite_explosion.c) | <img src="textures/textures_sprite_explosion.png" alt="textures_sprite_explosion" width="80"> | ⭐⭐☆☆ | 2.5 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_bunnymark](textures/textures_bunnymark.c) | <img src="textures/textures_bunnymark.png" alt="textures_bunnymark" width="80"> | ⭐⭐⭐☆ | 1.6 | 2.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_mouse_painting](textures/textures_mouse_painting.c) | <img src="textures/textures_mouse_painting.png" alt="textures_mouse_painting" width="80"> | ⭐⭐⭐☆ | 3.0 | 3.0 | [Chris Dill](https://github.com/MysteriousSpace) |
| [textures_blend_modes](textures/textures_blend_modes.c) | <img src="textures/textures_blend_modes.png" alt="textures_blend_modes" width="80"> | ⭐☆☆☆ | 3.5 | 3.5 | [Karlo Licudine](https://github.com/accidentalrebel) |
| [textures_tiled_drawing](textures/textures_tiled_drawing.c) | <img src="textures/textures_tiled_drawing.png" alt="textures_tiled_drawing" width="80"> | ⭐⭐⭐☆ | 3.0 | 4.2 | [Vlad Adrian](https://github.com/demizdor) |
| [textures_polygon_drawing](textures/textures_polygon_drawing.c) | <img src="textures/textures_polygon_drawing.png" alt="textures_polygon_drawing" width="80"> | ⭐☆☆☆ | 3.7 | 3.7 | [Chris Camacho](https://github.com/chriscamacho) |
| [textures_fog_of_war](textures/textures_fog_of_war.c) | <img src="textures/textures_fog_of_war.png" alt="textures_fog_of_war" width="80"> | ⭐⭐⭐☆ | 4.2 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_gif_player](textures/textures_gif_player.c) | <img src="textures/textures_gif_player.png" alt="textures_gif_player" width="80"> | ⭐⭐⭐☆ | 4.2 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_image_kernel](textures/textures_image_kernel.c) | <img src="textures/textures_image_kernel.png" alt="textures_image_kernel" width="80"> | ⭐⭐⭐⭐️ | 1.3 | 1.3 | [Karim Salem](https://github.com/kimo-s) |
| [textures_image_channel](textures/textures_image_channel.c) | <img src="textures/textures_image_channel.png" alt="textures_image_channel" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [Bruno Cabral](https://github.com/brccabral) |
| [textures_image_rotate](textures/textures_image_rotate.c) | <img src="textures/textures_image_rotate.png" alt="textures_image_rotate" width="80"> | ⭐⭐☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [textures_screen_buffer](textures/textures_screen_buffer.c) | <img src="textures/textures_screen_buffer.png" alt="textures_screen_buffer" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [Agnis Aldiņš](https://github.com/nezvers) |
| [textures_textured_curve](textures/textures_textured_curve.c) | <img src="textures/textures_textured_curve.png" alt="textures_textured_curve" width="80"> | ⭐⭐⭐☆ | 4.5 | 4.5 | [Jeffery Myers](https://github.com/JeffM2501) |
| [textures_sprite_stacking](textures/textures_sprite_stacking.c) | <img src="textures/textures_sprite_stacking.png" alt="textures_sprite_stacking" width="80"> | ⭐⭐☆☆ | 5.6-dev | 6.0 | [Robin](https://github.com/RobinsAviary) |
| [textures_cellular_automata](textures/textures_cellular_automata.c) | <img src="textures/textures_cellular_automata.png" alt="textures_cellular_automata" width="80"> | ⭐⭐☆☆ | 5.6 | 5.6 | [Jordi Santonja](https://github.com/JordSant) |
| [textures_framebuffer_rendering](textures/textures_framebuffer_rendering.c) | <img src="textures/textures_framebuffer_rendering.png" alt="textures_framebuffer_rendering" width="80"> | ⭐⭐☆☆ | 5.6 | 5.6 | [Jack Boakes](https://github.com/jackboakes) |

### category: text [16]

Examples using raylib text functionality, including sprite fonts loading/generation and text drawing, provided by raylib [text](../src/rtext.c) module.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [text_sprite_fonts](text/text_sprite_fonts.c) | <img src="text/text_sprite_fonts.png" alt="text_sprite_fonts" width="80"> | ⭐☆☆☆ | 1.7 | 3.7 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_font_spritefont](text/text_font_spritefont.c) | <img src="text/text_font_spritefont.png" alt="text_font_spritefont" width="80"> | ⭐☆☆☆ | 1.0 | 1.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_font_filters](text/text_font_filters.c) | <img src="text/text_font_filters.png" alt="text_font_filters" width="80"> | ⭐⭐☆☆ | 1.3 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_font_loading](text/text_font_loading.c) | <img src="text/text_font_loading.png" alt="text_font_loading" width="80"> | ⭐☆☆☆ | 1.4 | 3.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_font_sdf](text/text_font_sdf.c) | <img src="text/text_font_sdf.png" alt="text_font_sdf" width="80"> | ⭐⭐⭐☆ | 1.3 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_format_text](text/text_format_text.c) | <img src="text/text_format_text.png" alt="text_format_text" width="80"> | ⭐☆☆☆ | 1.1 | 3.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_input_box](text/text_input_box.c) | <img src="text/text_input_box.png" alt="text_input_box" width="80"> | ⭐⭐☆☆ | 1.7 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_writing_anim](text/text_writing_anim.c) | <img src="text/text_writing_anim.png" alt="text_writing_anim" width="80"> | ⭐⭐☆☆ | 1.4 | 1.4 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_rectangle_bounds](text/text_rectangle_bounds.c) | <img src="text/text_rectangle_bounds.png" alt="text_rectangle_bounds" width="80"> | ⭐⭐⭐⭐️ | 2.5 | 4.0 | [Vlad Adrian](https://github.com/demizdor) |
| [text_unicode_emojis](text/text_unicode_emojis.c) | <img src="text/text_unicode_emojis.png" alt="text_unicode_emojis" width="80"> | ⭐⭐⭐⭐️ | 2.5 | 4.0 | [Vlad Adrian](https://github.com/demizdor) |
| [text_unicode_ranges](text/text_unicode_ranges.c) | <img src="text/text_unicode_ranges.png" alt="text_unicode_ranges" width="80"> | ⭐⭐⭐⭐️ | 5.5 | 5.6 | [Vadim Gunko](https://github.com/GuvaCode) |
| [text_3d_drawing](text/text_3d_drawing.c) | <img src="text/text_3d_drawing.png" alt="text_3d_drawing" width="80"> | ⭐⭐⭐⭐️ | 3.5 | 4.0 | [Vlad Adrian](https://github.com/demizdor) |
| [text_codepoints_loading](text/text_codepoints_loading.c) | <img src="text/text_codepoints_loading.png" alt="text_codepoints_loading" width="80"> | ⭐⭐⭐☆ | 4.2 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [text_inline_styling](text/text_inline_styling.c) | <img src="text/text_inline_styling.png" alt="text_inline_styling" width="80"> | ⭐⭐⭐☆ | 5.6-dev | 5.6-dev | [Wagner Barongello](https://github.com/SultansOfCode) |
| [text_words_alignment](text/text_words_alignment.c) | <img src="text/text_words_alignment.png" alt="text_words_alignment" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6-dev | [JP Mortiboys](https://github.com/themushroompirates) |
| [text_strings_management](text/text_strings_management.c) | <img src="text/text_strings_management.png" alt="text_strings_management" width="80"> | ⭐⭐⭐☆ | 5.6-dev | 5.6-dev | [David Buzatto](https://github.com/davidbuzatto) |

### category: models [27]

Examples using raylib models functionality, including models loading/generation and drawing, provided by raylib [models](../src/rmodels.c) module.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [models_animation_playing](models/models_animation_playing.c) | <img src="models/models_animation_playing.png" alt="models_animation_playing" width="80"> | ⭐⭐☆☆ | 2.5 | 3.5 | [Culacant](https://github.com/culacant) |
| [models_billboard_rendering](models/models_billboard_rendering.c) | <img src="models/models_billboard_rendering.png" alt="models_billboard_rendering" width="80"> | ⭐⭐⭐☆ | 1.3 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_box_collisions](models/models_box_collisions.c) | <img src="models/models_box_collisions.png" alt="models_box_collisions" width="80"> | ⭐☆☆☆ | 1.3 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_cubicmap_rendering](models/models_cubicmap_rendering.c) | <img src="models/models_cubicmap_rendering.png" alt="models_cubicmap_rendering" width="80"> | ⭐⭐☆☆ | 1.8 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_first_person_maze](models/models_first_person_maze.c) | <img src="models/models_first_person_maze.png" alt="models_first_person_maze" width="80"> | ⭐⭐☆☆ | 2.5 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_geometric_shapes](models/models_geometric_shapes.c) | <img src="models/models_geometric_shapes.png" alt="models_geometric_shapes" width="80"> | ⭐☆☆☆ | 1.0 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_mesh_generation](models/models_mesh_generation.c) | <img src="models/models_mesh_generation.png" alt="models_mesh_generation" width="80"> | ⭐⭐☆☆ | 1.8 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_mesh_picking](models/models_mesh_picking.c) | <img src="models/models_mesh_picking.png" alt="models_mesh_picking" width="80"> | ⭐⭐⭐☆ | 1.7 | 4.0 | [Joel Davis](https://github.com/joeld42) |
| [models_loading](models/models_loading.c) | <img src="models/models_loading.png" alt="models_loading" width="80"> | ⭐☆☆☆ | 2.0 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_loading_gltf](models/models_loading_gltf.c) | <img src="models/models_loading_gltf.png" alt="models_loading_gltf" width="80"> | ⭐☆☆☆ | 3.7 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_loading_vox](models/models_loading_vox.c) | <img src="models/models_loading_vox.png" alt="models_loading_vox" width="80"> | ⭐☆☆☆ | 4.0 | 4.0 | [Johann Nadalutti](https://github.com/procfxgen) |
| [models_loading_m3d](models/models_loading_m3d.c) | <img src="models/models_loading_m3d.png" alt="models_loading_m3d" width="80"> | ⭐⭐☆☆ | 4.5 | 4.5 | [bzt](https://github.com/bztsrc) |
| [models_orthographic_projection](models/models_orthographic_projection.c) | <img src="models/models_orthographic_projection.png" alt="models_orthographic_projection" width="80"> | ⭐☆☆☆ | 2.0 | 3.7 | [Max Danielsson](https://github.com/autious) |
| [models_point_rendering](models/models_point_rendering.c) | <img src="models/models_point_rendering.png" alt="models_point_rendering" width="80"> | ⭐⭐⭐☆ | 5.0 | 5.0 | [Reese Gallagher](https://github.com/satchelfrost) |
| [models_rlgl_solar_system](models/models_rlgl_solar_system.c) | <img src="models/models_rlgl_solar_system.png" alt="models_rlgl_solar_system" width="80"> | ⭐⭐⭐⭐️ | 2.5 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_yaw_pitch_roll](models/models_yaw_pitch_roll.c) | <img src="models/models_yaw_pitch_roll.png" alt="models_yaw_pitch_roll" width="80"> | ⭐⭐☆☆ | 1.8 | 4.0 | [Berni](https://github.com/Berni8k) |
| [models_waving_cubes](models/models_waving_cubes.c) | <img src="models/models_waving_cubes.png" alt="models_waving_cubes" width="80"> | ⭐⭐⭐☆ | 2.5 | 3.7 | [Codecat](https://github.com/codecat) |
| [models_heightmap_rendering](models/models_heightmap_rendering.c) | <img src="models/models_heightmap_rendering.png" alt="models_heightmap_rendering" width="80"> | ⭐☆☆☆ | 1.8 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_skybox_rendering](models/models_skybox_rendering.c) | <img src="models/models_skybox_rendering.png" alt="models_skybox_rendering" width="80"> | ⭐⭐☆☆ | 1.8 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_textured_cube](models/models_textured_cube.c) | <img src="models/models_textured_cube.png" alt="models_textured_cube" width="80"> | ⭐⭐☆☆ | 4.5 | 4.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [models_animation_gpu_skinning](models/models_animation_gpu_skinning.c) | <img src="models/models_animation_gpu_skinning.png" alt="models_animation_gpu_skinning" width="80"> | ⭐⭐⭐☆ | 4.5 | 4.5 | [Daniel Holden](https://github.com/orangeduck) |
| [models_bone_socket](models/models_bone_socket.c) | <img src="models/models_bone_socket.png" alt="models_bone_socket" width="80"> | ⭐⭐⭐⭐️ | 4.5 | 4.5 | [iP](https://github.com/ipzaur) |
| [models_tesseract_view](models/models_tesseract_view.c) | <img src="models/models_tesseract_view.png" alt="models_tesseract_view" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6-dev | [Timothy van der Valk](https://github.com/arceryz) |
| [models_basic_voxel](models/models_basic_voxel.c) | <img src="models/models_basic_voxel.png" alt="models_basic_voxel" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [Tim Little](https://github.com/timlittle) |
| [models_rotating_cube](models/models_rotating_cube.c) | <img src="models/models_rotating_cube.png" alt="models_rotating_cube" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6-dev | [Jopestpe](https://github.com/jopestpe) |
| [models_decals](models/models_decals.c) | <img src="models/models_decals.png" alt="models_decals" width="80"> | ⭐⭐⭐⭐️ | 5.6-dev | 5.6-dev | [JP Mortiboys](https://github.com/themushroompirates) |
| [models_directional_billboard](models/models_directional_billboard.c) | <img src="models/models_directional_billboard.png" alt="models_directional_billboard" width="80"> | ⭐⭐☆☆ | 5.6-dev | 5.6 | [Robin](https://github.com/RobinsAviary) |

### category: shaders [33]

Examples using raylib shaders functionality, including shaders loading, parameters configuration and drawing using them (model shaders and postprocessing shaders). This functionality is directly provided by raylib [rlgl](../src/rlgl.c) module.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [shaders_ascii_rendering](shaders/shaders_ascii_rendering.c) | <img src="shaders/shaders_ascii_rendering.png" alt="shaders_ascii_rendering" width="80"> | ⭐⭐☆☆ | 5.5 | 5.6 | [Maicon Santana](https://github.com/maiconpintoabreu) |
| [shaders_basic_lighting](shaders/shaders_basic_lighting.c) | <img src="shaders/shaders_basic_lighting.png" alt="shaders_basic_lighting" width="80"> | ⭐⭐⭐⭐️ | 3.0 | 4.2 | [Chris Camacho](https://github.com/chriscamacho) |
| [shaders_model_shader](shaders/shaders_model_shader.c) | <img src="shaders/shaders_model_shader.png" alt="shaders_model_shader" width="80"> | ⭐⭐☆☆ | 1.3 | 3.7 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_shapes_textures](shaders/shaders_shapes_textures.c) | <img src="shaders/shaders_shapes_textures.png" alt="shaders_shapes_textures" width="80"> | ⭐⭐☆☆ | 1.7 | 3.7 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_custom_uniform](shaders/shaders_custom_uniform.c) | <img src="shaders/shaders_custom_uniform.png" alt="shaders_custom_uniform" width="80"> | ⭐⭐☆☆ | 1.3 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_postprocessing](shaders/shaders_postprocessing.c) | <img src="shaders/shaders_postprocessing.png" alt="shaders_postprocessing" width="80"> | ⭐⭐⭐☆ | 1.3 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_palette_switch](shaders/shaders_palette_switch.c) | <img src="shaders/shaders_palette_switch.png" alt="shaders_palette_switch" width="80"> | ⭐⭐⭐☆ | 2.5 | 3.7 | [Marco Lizza](https://github.com/MarcoLizza) |
| [shaders_raymarching_rendering](shaders/shaders_raymarching_rendering.c) | <img src="shaders/shaders_raymarching_rendering.png" alt="shaders_raymarching_rendering" width="80"> | ⭐⭐⭐⭐️ | 2.0 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_texture_rendering](shaders/shaders_texture_rendering.c) | <img src="shaders/shaders_texture_rendering.png" alt="shaders_texture_rendering" width="80"> | ⭐⭐☆☆ | 2.0 | 3.7 | [Michał Ciesielski](https://github.com/ciessielski) |
| [shaders_texture_outline](shaders/shaders_texture_outline.c) | <img src="shaders/shaders_texture_outline.png" alt="shaders_texture_outline" width="80"> | ⭐⭐⭐☆ | 4.0 | 4.0 | [Serenity Skiff](https://github.com/GoldenThumbs) |
| [shaders_texture_waves](shaders/shaders_texture_waves.c) | <img src="shaders/shaders_texture_waves.png" alt="shaders_texture_waves" width="80"> | ⭐⭐☆☆ | 2.5 | 3.7 | [Anata](https://github.com/anatagawa) |
| [shaders_julia_set](shaders/shaders_julia_set.c) | <img src="shaders/shaders_julia_set.png" alt="shaders_julia_set" width="80"> | ⭐⭐⭐☆ | 2.5 | 4.0 | [Josh Colclough](https://github.com/joshcol9232) |
| [shaders_mandelbrot_set](shaders/shaders_mandelbrot_set.c) | <img src="shaders/shaders_mandelbrot_set.png" alt="shaders_mandelbrot_set" width="80"> | ⭐⭐⭐☆ | 5.6 | 5.6 | [Jordi Santonja](https://github.com/JordSant) |
| [shaders_color_correction](shaders/shaders_color_correction.c) | <img src="shaders/shaders_color_correction.png" alt="shaders_color_correction" width="80"> | ⭐⭐☆☆ | 5.6 | 5.6 | [Jordi Santonja](https://github.com/JordSant) |
| [shaders_eratosthenes_sieve](shaders/shaders_eratosthenes_sieve.c) | <img src="shaders/shaders_eratosthenes_sieve.png" alt="shaders_eratosthenes_sieve" width="80"> | ⭐⭐⭐☆ | 2.5 | 4.0 | [ProfJski](https://github.com/ProfJski) |
| [shaders_fog_rendering](shaders/shaders_fog_rendering.c) | <img src="shaders/shaders_fog_rendering.png" alt="shaders_fog_rendering" width="80"> | ⭐⭐⭐☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/chriscamacho) |
| [shaders_simple_mask](shaders/shaders_simple_mask.c) | <img src="shaders/shaders_simple_mask.png" alt="shaders_simple_mask" width="80"> | ⭐⭐☆☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/chriscamacho) |
| [shaders_hot_reloading](shaders/shaders_hot_reloading.c) | <img src="shaders/shaders_hot_reloading.png" alt="shaders_hot_reloading" width="80"> | ⭐⭐⭐☆ | 3.0 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_mesh_instancing](shaders/shaders_mesh_instancing.c) | <img src="shaders/shaders_mesh_instancing.png" alt="shaders_mesh_instancing" width="80"> | ⭐⭐⭐⭐️ | 3.7 | 4.2 | [seanpringle](https://github.com/seanpringle) |
| [shaders_multi_sample2d](shaders/shaders_multi_sample2d.c) | <img src="shaders/shaders_multi_sample2d.png" alt="shaders_multi_sample2d" width="80"> | ⭐⭐☆☆ | 3.5 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [shaders_normalmap_rendering](shaders/shaders_normalmap_rendering.c) | <img src="shaders/shaders_normalmap_rendering.png" alt="shaders_normalmap_rendering" width="80"> | ⭐⭐⭐⭐️ | 5.6 | 5.6 | [Jeremy Montgomery](https://github.com/Sir_Irk) |
| [shaders_spotlight_rendering](shaders/shaders_spotlight_rendering.c) | <img src="shaders/shaders_spotlight_rendering.png" alt="shaders_spotlight_rendering" width="80"> | ⭐⭐☆☆ | 2.5 | 3.7 | [Chris Camacho](https://github.com/chriscamacho) |
| [shaders_deferred_rendering](shaders/shaders_deferred_rendering.c) | <img src="shaders/shaders_deferred_rendering.png" alt="shaders_deferred_rendering" width="80"> | ⭐⭐⭐⭐️ | 4.5 | 4.5 | [Justin Andreas Lacoste](https://github.com/27justin) |
| [shaders_hybrid_rendering](shaders/shaders_hybrid_rendering.c) | <img src="shaders/shaders_hybrid_rendering.png" alt="shaders_hybrid_rendering" width="80"> | ⭐⭐⭐⭐️ | 4.2 | 4.2 | [Buğra Alptekin Sarı](https://github.com/BugraAlptekinSari) |
| [shaders_texture_tiling](shaders/shaders_texture_tiling.c) | <img src="shaders/shaders_texture_tiling.png" alt="shaders_texture_tiling" width="80"> | ⭐⭐☆☆ | 4.5 | 4.5 | [Luis Almeida](https://github.com/luis605) |
| [shaders_shadowmap_rendering](shaders/shaders_shadowmap_rendering.c) | <img src="shaders/shaders_shadowmap_rendering.png" alt="shaders_shadowmap_rendering" width="80"> | ⭐⭐⭐⭐️ | 5.0 | 5.0 | [TheManTheMythTheGameDev](https://github.com/TheManTheMythTheGameDev) |
| [shaders_vertex_displacement](shaders/shaders_vertex_displacement.c) | <img src="shaders/shaders_vertex_displacement.png" alt="shaders_vertex_displacement" width="80"> | ⭐⭐⭐☆ | 5.0 | 4.5 | [Alex ZH](https://github.com/ZzzhHe) |
| [shaders_depth_writing](shaders/shaders_depth_writing.c) | <img src="shaders/shaders_depth_writing.png" alt="shaders_depth_writing" width="80"> | ⭐⭐☆☆ | 4.2 | 4.2 | [Buğra Alptekin Sarı](https://github.com/BugraAlptekinSari) |
| [shaders_basic_pbr](shaders/shaders_basic_pbr.c) | <img src="shaders/shaders_basic_pbr.png" alt="shaders_basic_pbr" width="80"> | ⭐⭐⭐⭐️ | 5.0 | 5.5 | [Afan OLOVCIC](https://github.com/_DevDad) |
| [shaders_lightmap_rendering](shaders/shaders_lightmap_rendering.c) | <img src="shaders/shaders_lightmap_rendering.png" alt="shaders_lightmap_rendering" width="80"> | ⭐⭐⭐☆ | 4.5 | 4.5 | [Jussi Viitala](https://github.com/nullstare) |
| [shaders_rounded_rectangle](shaders/shaders_rounded_rectangle.c) | <img src="shaders/shaders_rounded_rectangle.png" alt="shaders_rounded_rectangle" width="80"> | ⭐⭐⭐☆ | 5.5 | 5.5 | [Anstro Pleuton](https://github.com/anstropleuton) |
| [shaders_depth_rendering](shaders/shaders_depth_rendering.c) | <img src="shaders/shaders_depth_rendering.png" alt="shaders_depth_rendering" width="80"> | ⭐⭐⭐☆ | 5.6-dev | 5.6-dev | [Luís Almeida](https://github.com/luis605) |
| [shaders_game_of_life](shaders/shaders_game_of_life.c) | <img src="shaders/shaders_game_of_life.png" alt="shaders_game_of_life" width="80"> | ⭐⭐⭐☆ | 5.6 | 5.6 | [Jordi Santonja](https://github.com/JordSant) |

### category: audio [9]

Examples using raylib audio functionality, including sound/music loading and playing. This functionality is provided by raylib [raudio](../src/raudio.c) module. Note this module can be used standalone independently of raylib.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [audio_module_playing](audio/audio_module_playing.c) | <img src="audio/audio_module_playing.png" alt="audio_module_playing" width="80"> | ⭐☆☆☆ | 1.5 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [audio_music_stream](audio/audio_music_stream.c) | <img src="audio/audio_music_stream.png" alt="audio_music_stream" width="80"> | ⭐☆☆☆ | 1.3 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [audio_raw_stream](audio/audio_raw_stream.c) | <img src="audio/audio_raw_stream.png" alt="audio_raw_stream" width="80"> | ⭐⭐⭐☆ | 1.6 | 4.2 | [Ramon Santamaria](https://github.com/raysan5) |
| [audio_sound_loading](audio/audio_sound_loading.c) | <img src="audio/audio_sound_loading.png" alt="audio_sound_loading" width="80"> | ⭐☆☆☆ | 1.1 | 3.5 | [Ramon Santamaria](https://github.com/raysan5) |
| [audio_mixed_processor](audio/audio_mixed_processor.c) | <img src="audio/audio_mixed_processor.png" alt="audio_mixed_processor" width="80"> | ⭐⭐⭐⭐️ | 4.2 | 4.2 | [hkc](https://github.com/hatkidchan) |
| [audio_stream_effects](audio/audio_stream_effects.c) | <img src="audio/audio_stream_effects.png" alt="audio_stream_effects" width="80"> | ⭐⭐⭐⭐️ | 4.2 | 5.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [audio_sound_multi](audio/audio_sound_multi.c) | <img src="audio/audio_sound_multi.png" alt="audio_sound_multi" width="80"> | ⭐⭐☆☆ | 5.0 | 5.0 | [Jeffery Myers](https://github.com/JeffM2501) |
| [audio_sound_positioning](audio/audio_sound_positioning.c) | <img src="audio/audio_sound_positioning.png" alt="audio_sound_positioning" width="80"> | ⭐⭐☆☆ | 5.5 | 5.5 | [Le Juez Victor](https://github.com/Bigfoot71) |
| [audio_spectrum_visualizer](audio/audio_spectrum_visualizer.c) | <img src="audio/audio_spectrum_visualizer.png" alt="audio_spectrum_visualizer" width="80"> | ⭐⭐⭐☆ | 6.0 | 5.6-dev | [IANN](https://github.com/meisei4) |

### category: others [6]

Examples showing raylib misc functionality that does not fit in other categories, like standalone modules usage or examples integrating external libraries.

|  example  | image  | difficulty<br>level | version<br>created | last version<br>updated | original<br>developer |
|-----------|--------|:-------------------:|:------------------:|:-----------------------:|:----------------------|
| [rlgl_standalone](others/rlgl_standalone.c) | <img src="others/rlgl_standalone.png" alt="rlgl_standalone" width="80"> | ⭐⭐⭐⭐️ | 1.6 | 4.0 | [Ramon Santamaria](https://github.com/raysan5) |
| [rlgl_compute_shader](others/rlgl_compute_shader.c) | <img src="others/rlgl_compute_shader.png" alt="rlgl_compute_shader" width="80"> | ⭐⭐⭐⭐️ | 4.0 | 4.0 | [Teddy Astie](https://github.com/tsnake41) |
| [easings_testbed](others/easings_testbed.c) | <img src="others/easings_testbed.png" alt="easings_testbed" width="80"> | ⭐⭐⭐☆ | 2.5 | 3.0 | [Juan Miguel López](https://github.com/flashback-fx) |
| [raylib_opengl_interop](others/raylib_opengl_interop.c) | <img src="others/raylib_opengl_interop.png" alt="raylib_opengl_interop" width="80"> | ⭐⭐⭐⭐️ | 3.8 | 4.0 | [Stephan Soller](https://github.com/arkanis) |
| [embedded_files_loading](others/embedded_files_loading.c) | <img src="others/embedded_files_loading.png" alt="embedded_files_loading" width="80"> | ⭐⭐☆☆ | 3.0 | 3.5 | [Kristian Holmgren](https://github.com/defutura) |
| [web_basic_window](others/web_basic_window.c) | <img src="others/web_basic_window.png" alt="web_basic_window" width="80"> | ⭐☆☆☆ | 5.6-dev | 5.6-dev | [Ramon Santamaria](https://github.com/raysan5) |

Some example missing? As always, contributions are welcome, feel free to send new examples!
Here is an [examples template](examples_template.c) with instructions to start with!
