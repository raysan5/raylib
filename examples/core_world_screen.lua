-------------------------------------------------------------------------------------------
--
--  raylib [core] example - World to screen
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(0.0, 10.0, 10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local cubePosition = Vector3(0.0, 0.0, 0.0)

local cubeScreenPosition = Vector2(0, 0)

SetCameraMode(camera, CameraMode.FREE)  -- Set a free camera mode

SetTargetFPS(60)                        -- Set our game to run at 60 frames-per-second
----------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do        -- Detect window close button or ESC key
    -- Update
    ------------------------------------------------------------------------------------
    camera = UpdateCamera(camera)       -- Update camera
    
    -- Calculate cube screen space position (with a little offset to be in top)
    cubeScreenPosition = GetWorldToScreen(Vector3(cubePosition.x, cubePosition.y + 2.5, cubePosition.z), camera)
    ------------------------------------------------------------------------------------

    -- Draw
    ------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)

            DrawCube(cubePosition, 2.0, 2.0, 2.0, RED)
            DrawCubeWires(cubePosition, 2.0, 2.0, 2.0, MAROON)

            DrawGrid(10, 1.0)

        End3dMode()
        
        DrawText("Enemy: 100 / 100", cubeScreenPosition.x//1 - MeasureText("Enemy: 100 / 100", 20)//2, cubeScreenPosition.y//1, 20, BLACK)
        DrawText("Text is always on top of the cube", (screenWidth - MeasureText("Text is always on top of the cube", 20))//2, 25, 20, GRAY)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
----------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
----------------------------------------------------------------------------------------