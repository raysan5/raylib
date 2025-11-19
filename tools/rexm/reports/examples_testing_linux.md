# EXAMPLES COLLECTION - TESTING REPORT

## Tested Platform: Linux

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
| text_font_loading                |    0    |    10    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| text_codepoints_loading          |    0    |    1    |   ✔   |    ✔    |   ✔    |   ✔   |   ✔   |   ✔   |   ✔   |
| models_animation_playing         |    0    |    1    |   ✔   |    ✔    |   ❌    |   ✔   |   ✔   |   ✔   |   ✔   |

