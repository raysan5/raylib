/*******************************************************************************************
*
*   raylib [core] example - split screen
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
*   This example has been created using raylib 3.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

Texture2D GridTexture;
Camera Player1Camera = { 0 };
Camera Player2Camera = { 0 };

void DrawScene()
{
    // grid of cube trees on a plane to make a "world"
    DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ 50, 50 }, BEIGE); // simple world plane
    float spacing = 4;
    int count = 5;

    for (float x = -count * spacing; x <= count * spacing; x += spacing)
    {
        for (float z = -count * spacing; z <= count * spacing; z += spacing)
        {
            Vector3 pos = { x, 0.5f, z };

            Vector3 min = { x - 0.5f,0,z - 0.5f };
            Vector3 max = { x + 0.5f,1,z + 0.5f };
 
            DrawCubeTexture(GridTexture, (Vector3) { x, 1.5f, z }, 1, 1, 1, GREEN);
            DrawCubeTexture(GridTexture, (Vector3) { x, 0.5f, z }, 0.25f, 1, 0.25f, BROWN);
        }
    }

    // draw a cube at each player's position
    DrawCube(Player1Camera.position, 1, 1, 1, RED);
    DrawCube(Player2Camera.position, 1, 1, 1, BLUE);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - split screen");
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // generate a simple texture to use for trees
    Image img = GenImageChecked(256, 256, 32, 32, DARKGRAY, WHITE);
    GridTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(GridTexture, TEXTURE_FILTER_ANISOTROPIC_16X);
    SetTextureWrap(GridTexture, TEXTURE_WRAP_CLAMP);


    // setup player 1 camera and screen
    Player1Camera.fovy = 45;
    Player1Camera.up.y = 1;
    Player1Camera.target.y = 1;
    Player1Camera.position.z = -3;
    Player1Camera.position.y = 1;

    RenderTexture player1Screen = LoadRenderTexture(screenWidth / 2, screenHeight);

    // setup player two camera and screen
    Player2Camera.fovy = 45;
    Player2Camera.up.y = 1;
    Player2Camera.target.y =3;
    Player2Camera.position.x = -3;
    Player2Camera.position.y = 3;

    RenderTexture player2Screen = LoadRenderTexture(screenWidth / 2, screenHeight);
   
    // build a flipped rectangle the size of the split view to use for drawing later
    Rectangle splitScreenRect = { 0,0, (float)player1Screen.texture.width, (float)-player1Screen.texture.height };

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // if anyone moves this frame, how far will they move based on the time since the last frame
        // this moves thigns at 10 world units per second, regardless of the actual FPS
        float offsetThisFrame = 10 * GetFrameTime();


        // move player 1 forward and backwards (no turning)
        if (IsKeyDown(KEY_W))
        {
            Player1Camera.position.z += offsetThisFrame;
            Player1Camera.target.z += offsetThisFrame;
        }
        else if (IsKeyDown(KEY_S))
        {
            Player1Camera.position.z -= offsetThisFrame;
            Player1Camera.target.z -= offsetThisFrame;
        }

        // move player 2 forward and backwards (no turning)
        if (IsKeyDown(KEY_UP))
        {
            Player2Camera.position.x += offsetThisFrame;
            Player2Camera.target.x += offsetThisFrame;
        }
        else if (IsKeyDown(KEY_DOWN))
        {
            Player2Camera.position.x -= offsetThisFrame;
            Player2Camera.target.x -= offsetThisFrame;
        }

        // draw player 1's view to the render texture
        BeginTextureMode(player1Screen);
        ClearBackground(SKYBLUE);
        BeginMode3D(Player1Camera);
        DrawScene();
        EndMode3D();
        DrawText("PLAYER1 W/S to move", 0, 0, 20, RED);
        EndTextureMode();

        // draw player 2's view to the render texture
        BeginTextureMode(player2Screen);
        ClearBackground(SKYBLUE);
        BeginMode3D(Player2Camera);
        DrawScene();
        EndMode3D();
        DrawText("PLAYER2 UP/DOWN to move", 0, 0, 20, BLUE);
        EndTextureMode();

        // draw both view render textures to the screen side by side
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(player1Screen.texture, splitScreenRect, (Vector2) { 0, 0 }, WHITE);
        DrawTextureRec(player2Screen.texture, splitScreenRect, (Vector2) { screenWidth/2.0f, 0 }, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(player1Screen);
    UnloadRenderTexture(player2Screen);
    UnloadTexture(GridTexture);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}