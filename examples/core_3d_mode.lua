-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Initialize 3d mode
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d mode")

-- Define the camera to look into our 3d world
local camera = {}
camera.position = Vector3(0.0, 10.0, 10.0)   -- Camera position
camera.target = Vector3(0.0, 0.0, 0.0)       -- Camera looking at point
camera.up = Vector3(0.0, 1.0, 0.0)           -- Camera up vector (rotation towards target)
camera.fovy = 45.0                           -- Camera field-of-view Y

local cubePosition = Vector3(0.0, 0.0, 0.0)

SetTargetFPS(60)   -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    -- TODO: Update your variables here
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)    -- ERROR: Lua Error: attempt to index a number value (?)

            DrawCube(cubePosition, 2.0, 2.0, 2.0, RED)
            DrawCubeWires(cubePosition, 2.0, 2.0, 2.0, MAROON)

            DrawGrid(10, 1.0)

        End3dMode()

        DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()          -- Close window and OpenGL context
-------------------------------------------------------------------------------------------