-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Keyboard input
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window")

local ballPosition = Vector2(screenWidth/2, screenHeight/2)

SetTargetFPS(60)                -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do                -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyDown(KEY.RIGHT)) then ballPosition.x = ballPosition.x + 0.8 end
    if (IsKeyDown(KEY.LEFT)) then ballPosition.x = ballPosition.x - 0.8 end
    if (IsKeyDown(KEY.UP)) then ballPosition.y = ballPosition.y - 0.8 end
    if (IsKeyDown(KEY.DOWN)) then ballPosition.y = ballPosition.y + 0.8 end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY)

        DrawCircleV(ballPosition, 50, MAROON)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()           -- Close window and OpenGL context
-------------------------------------------------------------------------------------------
