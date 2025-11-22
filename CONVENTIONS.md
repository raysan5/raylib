## C Coding Style Conventions

Here is a list with some of the code conventions used by raylib:

Code element | Convention | Example
--- | :---: | ---
Defines | ALL_CAPS | `#define PLATFORM_DESKTOP`
Macros | ALL_CAPS | `#define MIN(a,b) (((a)<(b))?(a):(b))`
Variables | lowerCase | `int screenWidth = 0;`, `float targetFrameTime = 0.016f;`
Local variables | lowerCase | `Vector2 playerPosition = { 0 };`
Global variables | lowerCase | `bool windowReady = false;`
Constants | lowerCase | `const int maxValue = 8;`
Pointers | MyType *pointer | `Texture2D *array = NULL;`
float values | always x.xf | `float gravity = 10.0f` (avoid `10.f`)
Operators | value1*value2 | `int product = value*6;`
Operators | value1/value2 | `int division = value/4;`
Operators | value1 + value2 | `int sum = value + 10;`
Operators | value1 - value2 | `int res = value - 5;`
Enum | TitleCase | `enum TextureFormat`
Enum members | ALL_CAPS | `PIXELFORMAT_UNCOMPRESSED_R8G8B8`
Struct | TitleCase | `struct Texture2D`, `struct Material`
Struct members | lowerCase | `texture.width`, `color.r`
Functions | TitleCase | `InitWindow()`, `LoadImageFromMemory()`
Functions params | lowerCase | `width`, `height`
Ternary Operator | (condition)? result1 : result2 | `printf("Value is 0: %s", (value == 0)? "yes" : "no");`

Some other conventions to follow:
 - **ALWAYS** initialize all defined variables.
 - **Do not use TABS**, use 4 spaces instead.
 - Avoid trailing spaces, please, avoid them
 - Comments always start with space + capital letter and never end with a '.', place them **before** the line(s) they refer to
```c
// This is a comment in raylib or raylib examples
```
 - Control flow statements always are followed **by a space**:
```c
if (condition) value = 0;

while (!WindowShouldClose())
{

}

// Always use accumulators as `x++` instead of `++x`
for (int i = 0; i < NUM_VALUES; i++) printf("%i", i);

// Be careful with the switch formatting!
switch (value)
{
    case 0:
    {

    } break;
    case 2: break;
    default: break;
}
```
 - All conditions checks are **always between parenthesis** but not boolean values:
```c
if ((value > 1) && (value < 50) && valueActive)
{

}
```
 - When dealing with braces or curly brackets, open-close them in aligned mode:
```c
void SomeFunction()
{
   // TODO: Do something here!
}
```

**If proposing new functions, please try to use a clear naming for function-name and functions-parameters, in case of doubt, open an issue for discussion.**

## Files and Directories Naming Conventions

  - Directories will be named using `snake_case`: `resources/models`, `resources/fonts`

  - Files will be named using `snake_case`: `main_title.png`, `cubicmap.png`, `sound.wav`

_NOTE: Avoid any space or special character in the files/dir naming!_

## Games/Examples Directories Organization Conventions

 - Data files should be organized by context and usage in the game, think about the loading requirements for data and put all the resources that need to be loaded at the same time together.
 - Use descriptive names for the files, it would be perfect if just reading the name of the file, it was possible to know what is that file and where fits in the game.
 - Here is an example, note that some resources require to be loaded all at once while other require to be loaded only at initialization (gui, font).

```
resources/audio/fx/long_jump.wav
resources/audio/music/main_theme.ogg
resources/screens/logo/logo.png
resources/screens/title/title.png
resources/screens/gameplay/background.png
resources/characters/player.png
resources/characters/enemy_slime.png
resources/common/font_arial.ttf
resources/common/gui.png
```
