-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Oculus Rift CV1
--
--  NOTE: Example requires linkage with LibOVR
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 1080
local screenHeight = 600

-- NOTE: screenWidth/screenHeight should match VR device aspect ratio

InitWindow(screenWidth, screenHeight, "raylib [core] example - oculus rift")

-- NOTE: If device is not available, it fallbacks to default device (simulator)
InitVrDevice(VrDevice.OCULUS_RIFT_CV1)      -- Init VR device (Oculus Rift CV1)

-- Define the camera to look into our 3d world
local camera = {}
camera.position = Vector3(5.0, 5.0, 5.0)    -- Camera position
camera.target = Vector3(0.0, 0.0, 0.0)      -- Camera looking at point
camera.up = Vector3(0.0, 1.0, 0.0)          -- Camera up vector (rotation towards target)
camera.fovy = 60.0                          -- Camera field-of-view Y

local cubePosition = Vector3(0.0, 0.0, 0.0)

SetTargetFPS(90)                    -- Set our game to run at 90 frames-per-second
----------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ------------------------------------------------------------------------------------
    UpdateVrTracking()
    
    if (IsKeyPressed(KEY.SPACE)) then ToggleVrMode() end
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

        DrawFPS(10, 10)

    EndDrawing()
    ------------------------------------------------------------------------------------
end

-- De-Initialization
----------------------------------------------------------------------------------------
CloseVrDevice()        -- Close VR device

CloseWindow()          -- Close window and OpenGL context
----------------------------------------------------------------------------------------