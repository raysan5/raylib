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
| textures_sprite_stacking         |    0    |    0    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_sprite_fonts                |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_font_loading                |    0    |    3    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |
| text_font_sdf                    |    0    |    22    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| text_codepoints_loading          |    0    |    1    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_animation_playing         |    0    |    1    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_mesh_generation           |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_loading_gltf              |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| models_bone_socket               |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| models_decals                    |    0    |    0    |   ❌   |    ❌    |   ❌    |   ❌   |   ❌   |   ❌   |   ❌   |
| shaders_postprocessing           |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| shaders_deferred_rendering       |    0    |    2    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| shaders_shadowmap_rendering      |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| shaders_basic_pbr                |    0    |    0    |   ✔   |    ❌    |   ✔    |   ✔   |   ✔   |   ✔   |   ❌   |
| audio_module_playing             |    0    |    1    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |
| audio_fft_spectrum_visualizer    |    0    |    0    |   ❌   |    ❌    |   ❌    |   ❌   |   ❌   |   ❌   |   ❌   |

