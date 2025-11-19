# EXAMPLES COLLECTION - TESTING REPORT

## Tested Platform: Web

```
Example automated testing elements validated:
 - [CWARN]  : Compilation WARNING messages
 - [LWARN]  : Log WARNING messages count
 - [INIT]   : Initialization
 - [CLOSE]  : Closing
 - [ASSETS] : Assets loading
 - [RLGL]   : OpenGL-wrapped initialization
 - [PLAT]   : Platform initialization
 - [FONT]   : Font default initialization
 - [TIMER]  : Timer initialization
```
| **EXAMPLE NAME**                 | [CWARN] | [LWARN] | [INIT] | [CLOSE] | [ASSETS] | [RLGL] | [PLAT] | [FONT] | [TIMER] |
|:---------------------------------|:-------:|:-------:|:------:|:-------:|:--------:|:------:|:------:|:------:|:-------:|
| core_monitor_detector            |    0    |    1    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| core_directory_files             |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| core_clipboard_text              |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| core_compute_hash                |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_recursive_tree            |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_ring_drawing              |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_circle_sector_drawing     |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_rounded_rectangle_drawing |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_splines_drawing           |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_triangle_strip            |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_pie_chart                 |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_math_sine_cosine          |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shapes_rlgl_color_wheel          |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| textures_sprite_stacking         |    0    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_sprite_fonts                |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_font_loading                |    0    |    3    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |
| text_font_sdf                    |    0    |    22    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_codepoints_loading          |    0    |    1    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_animation_playing         |    0    |    1    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_mesh_generation           |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_loading_gltf              |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| models_bone_socket               |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| shaders_postprocessing           |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| shaders_color_correction         |    1    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shaders_deferred_rendering       |    0    |    2    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shaders_shadowmap_rendering      |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| shaders_basic_pbr                |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| audio_module_playing             |    0    |    1    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |

