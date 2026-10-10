/*
    WELCOME raylib EXAMPLES CONTRIBUTOR!

    This is a basic template to anyone ready to contribute with some code example for the library,
    here there are some guidelines on how to create an example to be included in raylib

    1. File naming: <module>_<description> - Lower case filename, words separated by underscore,
       no more than 3-4 words in total to describe the example. <module> referes to the primary
       raylib module the example is more related with (code, shapes, textures, models, shaders, raudio)
       i.e: core_input_multitouch, shapes_lines_bezier, shaders_palette_switch

    2. Follow below template structure, example info should list the module, the short description
       and the author of the example, twitter or github info could be also provided for the author
       Short description should also be used on the title of the window

    3. Code should be organized by sections:[Initialization]- [Update] - [Draw] - [De-Initialization]
       Place your code between the dotted lines for every section, please don't mix update logic with drawing
       and remember to unload all loaded resources

    4. Code should follow raylib conventions: https://github.com/raysan5/raylib/wiki/raylib-coding-conventions
       Try to be very organized, using line-breaks appropiately

    5. Add comments to the specific parts of code the example is focus on
       Don't abuse with comments, try to be clear and impersonal on the comments

    6. Try to keep the example simple, under 300 code lines if possible. Try to avoid external dependencies
       Try to avoid defining functions outside the main(). Example should be as self-contained as possible

    7. About external resources, they should be placed in a [resources] folder and those resources
       should be open and free for use and distribution. Avoid propietary content

    8. Try to keep the example simple but with a creative touch
       Simple but beautiful examples are more appealing to users!

    9. In case of additional information is required, just come to raylib Discord channel: example-contributions

    10. Have fun!

    The following files must be updated when adding a new example,
    but it can be automatically done using the raylib provided tool: rexm
    So, no worries if just the .c/.png are provided when adding the example.

     - raylib/examples/<category>/<category>_example_name.c
     - raylib/examples/<category>/<category>_example_name.png
     - raylib/examples/<category>/resources/..
     - raylib/examples/Makefile
     - raylib/examples/Makefile.Web
     - raylib/examples/README.md
     - raylib/projects/VS2022/examples/<category>_example_name.vcxproj
     - raylib/projects/VS2022/raylib.sln
     - raylib.com/common/examples.js
     - raylib.com/examples/<category>/<category>_example_name.html
     - raylib.com/examples/<category>/<category>_example_name.data
     - raylib.com/examples/<category>/<category>_example_name.wasm
     - raylib.com/examples/<category>/<category>_example_name.js
*/

/*******************************************************************************************
*
*   raylib [<module>] example - <name/short description>
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by <author_name> (@<user_github>) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) <year_created>-<year_updated> <author_name> (@<user_github>)
*
********************************************************************************************/

#include "raylib.h"
#include "../shapes/reasings.h"
#include "raymath.h"
#include <stdio.h>

typedef struct {
    char str[100];
    Vector2 position;
    float delta;
    float offset;
    int duration;
} AnimatedText;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [<module>] example - <name>");

    AnimatedText text = { "This is an animated text!", { 150, 200 }, 12.0f, 6.0f, 30 };
    int frameCount = 0;
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCount++;
        if (IsKeyPressed(KEY_DOWN)) text.delta = Clamp(text.delta - 1.0f, 6.0f, 30.0f);
        else if (IsKeyPressed(KEY_UP)) text.delta = Clamp(text.delta + 1.0, 6.0f, 30.0f);
        if (IsKeyPressed(KEY_LEFT)) text.duration = Clamp(text.duration - 2, 10, 60);
        else if (IsKeyPressed(KEY_RIGHT)) text.duration = Clamp(text.duration + 2, 10, 60);
        if (IsKeyPressed(KEY_KP_SUBTRACT)) text.offset = Clamp(text.offset - 1.0f, 0.0f, 15.0f);
        else if (IsKeyPressed(KEY_KP_ADD)) text.offset = Clamp(text.offset + 1.0f, 0.0f, 15.0f);
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(TextFormat("Duration (Left | Right): %d", text.duration), 20, 20, 16, BLACK);
        DrawText(TextFormat("Delta (Down | Up): %.1f", text.delta), 20, 40, 16, BLACK);
        DrawText(TextFormat("Offset (- | +): %.1f", text.offset), 20, 60, 16, BLACK);

        for(int i=0; i<TextLength(text.str); i++)
        {
           char str[2] = {text.str[i], 0};
           int offset = i*text.offset;
           if ((frameCount+offset)%(text.duration*2) < text.duration) 
           {
              float posY = EaseSineIn((float) ((frameCount+offset)%text.duration), text.position.y, text.delta, text.duration);
              DrawText(str, text.position.x+20*i, posY, 24, PURPLE);
           }
           else 
           {
              float posY = EaseSineOut((float) ((frameCount+offset)%text.duration), text.position.y+text.delta, -text.delta, text.duration);
              DrawText(str, text.position.x+20*i, posY, 24, PURPLE);
           }
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
