/*
    WELCOME raylib EXAMPLES CONTRIBUTOR!

    This is a basic template to anyone ready to contribute with some code example for the library,
    here there are some guidelines on how to create an example to be included in raylib

    1. File naming: <module>_<description> - Lower case filename, words separated by underscore,
       no more than 3-4 words in total to describe the example. <module> referes to the primary
       raylib module the example is more related with (code, shapes, textures, models, shaders, raudio).
       i.e: core_input_multitouch, shapes_lines_bezier, shaders_palette_switch

    2. Follow below template structure, example info should list the module, the short description
       and the author of the example, twitter or github info could be also provided for the author.
       Short description should also be used on the title of the window.

    3. Code should be organized by sections:[Initialization]- [Update] - [Draw] - [De-Initialization]
       Place your code between the dotted lines for every section, please don't mix update logic with drawing
       and remember to unload all loaded resources.

    4. Code should follow raylib conventions: https://github.com/raysan5/raylib/wiki/raylib-coding-conventions
       Try to be very organized, using line-breaks appropiately.

    5. Add comments to the specific parts of code the example is focus on.
       Don't abuse with comments, try to be clear and impersonal on the comments.

    6. Try to keep the example simple, under 300 code lines if possible. Try to avoid external dependencies.
       Try to avoid defining functions outside the main(). Example should be as self-contained as possible.

    7. About external resources, they should be placed in a [resources] folder and those resources
       should be open and free for use and distribution. Avoid propietary content.

    8. Try to keep the example simple but with a creative touch.
       Simple but beautiful examples are more appealing to users!

    9. In case of additional information is required, just come to raylib Discord channel: example-contributions

    10. Have fun!
*/

/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Example originally created with raylib 4.2, last time updated with raylib 4.2
*
*   Example contributed by <user_name> (@<user_github>) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 <user_name> (@<user_github>)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // TODO: Load resources / Initialize variables at this point

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update variables / Implement example logic at this point
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // TODO: Draw everything that requires to be drawn at this point:

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);  // Example

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
