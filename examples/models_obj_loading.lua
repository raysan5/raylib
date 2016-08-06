-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Load and draw a 3d model (OBJ)
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - obj model loading")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(3.0, 3.0, 3.0), Vector3(0.0, 1.5, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local dwarf = LoadModel("resources/model/dwarf.obj")                -- Load OBJ model
local texture = LoadTexture("resources/model/dwarf_diffuse.png")    -- Load model texture
dwarf.material.texDiffuse = texture                                 -- Set dwarf model diffuse texture
local position = Vector3(0.0, 0.0, 0.0)                             -- Set model position

SetTargetFPS(60)        -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    -- ...
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        Begin3dMode(camera)

            DrawModel(dwarf, position, 2.0, WHITE)   -- Draw 3d model with texture

            DrawGrid(10, 1.0)         -- Draw a grid

            DrawGizmo(position)        -- Draw gizmo

        End3dMode()
        
        DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(texture)     -- Unload texture
UnloadModel(dwarf)         -- Unload model

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------