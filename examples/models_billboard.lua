-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Drawing billboards
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(5.0, 4.0, 5.0), Vector3(0.0, 2.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local bill = LoadTexture("resources/billboard.png")     -- Our texture billboard
local billPosition = Vector3(0.0, 2.0, 0.0)             -- Position where draw billboard

SetCameraMode(camera, CameraMode.ORBITAL)   -- Set an orbital camera mode

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
        
            DrawBillboard(camera, bill, billPosition, 2.0, WHITE)
            
            DrawGrid(10, 1.0)        -- Draw a grid

        End3dMode()

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(bill)        -- Unload texture

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------