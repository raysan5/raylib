-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Initialize 3d camera free
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
--------------------------------------------------------------------------------------------

-- Initialization
----------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free")

-- Define the camera to look into our 3d world
local camera = {}
camera.position = Vector3(10.0, 10.0, 10.0)  -- Camera position
camera.target = Vector3(0.0, 0.0, 0.0)      -- Camera looking at point
camera.up = Vector3(0.0, 1.0, 0.0)          -- Camera up vector (rotation towards target)
camera.fovy = 45.0                          -- Camera field-of-view Y

local cubePosition = Vector3(0.0, 0.0, 0.0)

SetCameraMode(camera, CameraMode.FREE)      -- Set a free camera mode

SetTargetFPS(60)                            -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    camera = UpdateCamera(camera)           -- Update camera
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)

            DrawCube(cubePosition, 2.0, 2.0, 2.0, RED)
            DrawCubeWires(cubePosition, 2.0, 2.0, 2.0, MAROON)

            DrawGrid(10, 1.0)

        End3dMode()
        
        DrawRectangle( 10, 10, 320, 133, Fade(SKYBLUE, 0.5))
        DrawRectangleLines( 10, 10, 320, 133, BLUE)
        
        DrawText("Free camera default controls:", 20, 20, 10, BLACK)
        DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY)
        DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY)
        DrawText("- Alt + Mouse Wheel Pressed to Rotate", 40, 80, 10, DARKGRAY)
        DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 40, 100, 10, DARKGRAY)
        DrawText("- Z to zoom to (0, 0, 0)", 40, 120, 10, DARKGRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------