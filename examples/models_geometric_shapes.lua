-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Draw some basic geometric shapes (cube, sphere, cylinder...)
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - geometric shapes")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(0.0, 10.0, 10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

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

        Begin3dMode(camera)  -- ERROR: Lua Error: attempt to index a number value

            DrawCube(Vector3(-4.0, 0.0, 2.0), 2.0, 5.0, 2.0, RED)
            DrawCubeWires(Vector3(-4.0, 0.0, 2.0), 2.0, 5.0, 2.0, GOLD)
            DrawCubeWires(Vector3(-4.0, 0.0, -2.0), 3.0, 6.0, 2.0, MAROON)

            DrawSphere(Vector3(-1.0, 0.0, -2.0), 1.0, GREEN)
            DrawSphereWires(Vector3(1.0, 0.0, 2.0), 2.0, 16, 16, LIME)

            DrawCylinder(Vector3(4.0, 0.0, -2.0), 1.0, 2.0, 3.0, 4, SKYBLUE)
            DrawCylinderWires(Vector3(4.0, 0.0, -2.0), 1.0, 2.0, 3.0, 4, DARKBLUE)
            DrawCylinderWires(Vector3(4.5, -1.0, 2.0), 1.0, 1.0, 2.0, 6, BROWN)

            DrawCylinder(Vector3(1.0, 0.0, -4.0), 0.0, 1.5, 3.0, 8, GOLD)
            DrawCylinderWires(Vector3(1.0, 0.0, -4.0), 0.0, 1.5, 3.0, 8, PINK)

            DrawGrid(10, 1.0)        -- Draw a grid

        End3dMode()

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------