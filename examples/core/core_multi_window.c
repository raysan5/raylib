/*******************************************************************************************
*
*   raylib [core] example - Multiple window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 5.0, last time updated with raylib 6.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"


// info we are going to store about each window
typedef struct WindowInfo
{
    // the ID for the window context. 
    int contextId;

    // the texture we want to draw.
    // note that each context has it's own set of textures, a texture is only valid in the contet it was created in
    Texture2D texture;

    // where we want draw our thing
    Vector2 position;

    Vector2 velocity;

    // a flag to know if we have closed this window or not
    bool open;
}WindowInfo;

void CheckWindowCollisions(WindowInfo* window)
{
    if (window->position.x + window->texture.width > GetScreenWidth())
    {
        window->position.x = (float)(GetScreenWidth() - window->texture.width);
        window->velocity.x *= -1;
    }

    if (window->position.x  < 0)
    {
        window->position.x = 0;
        window->velocity.x *= -1;
    }

    if (window->position.y + window->texture.height > GetScreenHeight())
    {
        window->position.y = (float)(GetScreenHeight() - window->texture.height);
        window->velocity.y *= -1;
    }

    if (window->position.y < 0)
    {
        window->position.y = 0;
        window->velocity.y *= -1;
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    WindowInfo window1 = { 0 };
    WindowInfo window2 = { 0 };

    // each window returns it's own context ID. You must save these to use them
    // each window must be inited separately
	window1.contextId = InitWindowPro(screenWidth, screenHeight, "raylib [core] example - window 1", FLAG_WINDOW_RESIZABLE);
    window1.open = true;

	window2.contextId = InitWindowPro(screenWidth, screenHeight, "raylib [core] example - window 2", FLAG_WINDOW_RESIZABLE);
    window2.open = true;

    // before we can do anything we have to tell raylib what window context to use.
    // we will be swapping the active context all over the place.
    SetActiveWindowContext(window1.contextId);
    SetWindowPosition(10, (int)GetWindowPosition().y);

    // The FPS and timing systems are only updated by the primary window (lowest open window id),
    // target FPS is a global setting, it can be set from any window context
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    // load the texture we want to draw. Every context has it's own set of textures and texture IDs. so you must load them for each context.
    window1.texture = LoadTexture("resources/wabbit_alpha.png");


    // Setup window 2
    SetActiveWindowContext(window2.contextId);
    // load the texture for window 2, even though this is the same texture file we used in window 1, we have to reload it for context 2.
    window2.texture = LoadTexture("resources/wabbit_alpha.png");

    SetWindowPosition(820, (int)GetWindowPosition().y);
    //--------------------------------------------------------------------------------------

	window1.position = (Vector2){ 300,300 };
    window1.velocity = (Vector2){ 1,1 };

	window2.position = (Vector2){ 500, 300 };
    window2.velocity = (Vector2){ 1,0 };

    // Main game loop
   
    while (window1.open || window2.open)  // run while any window is up
    {
        // Update

        float speed = 100;

        window1.position = Vector2Add(window1.position, Vector2Scale(window1.velocity, speed * GetFrameTime()));
        window2.position = Vector2Add(window2.position, Vector2Scale(window2.velocity, speed * GetFrameTime()));

        // Draw
        // process windows backwards because the lowest window ID (primary window) waits for frame timing and polls input events

        if (window2.open)
        {
            SetActiveWindowContext(window2.contextId);

            // check collisions against window 1 and the window size.
               // we need to do this in this context so it gets the correct window size.
            CheckWindowCollisions(&window2);

            // do we want to close this window
            if (WindowShouldClose())
            {
                // we need to unload this texture before we kill the context
                UnloadTexture(window2.texture);
                CloseWindow();
                window2.open = false;
            }
            else
            {
                BeginDrawing();

                ClearBackground(BLACK);

                DrawText("I am the other window", 190, 200, 20, RAYWHITE);
                DrawTextureV(window2.texture, window2.position, WHITE);
                EndDrawing();
            }
        }

        // If window 1 is open, process it
        if (window1.open)
        {
            SetActiveWindowContext(window1.contextId);

            // check collisions against window 1 and the window size.
            // we need to do this in this context so it gets the correct window size.
            CheckWindowCollisions(&window1);

            // do we want to close this window
            if (WindowShouldClose())
            {
                // we need to unload this texture before we kill the context
                UnloadTexture(window1.texture);
                CloseWindow();
                window1.open = false;
            }
            else
            {
                // process this window
                BeginDrawing();

                ClearBackground(RAYWHITE);

                DrawText("I am the main window", 190, 200, 20, LIGHTGRAY);

                DrawTextureV(window1.texture, window1.position, WHITE);
                DrawFPS(0, 0);

                EndDrawing(); 
            }
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // we shut down each window when it closed, so we have nothing left to cleanup

    return 0;
}