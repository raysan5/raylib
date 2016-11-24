-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Heightmap loading and drawing
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing")

-- Define our custom camera to look into our 3d world
local camera = Camera(Vector3(18.0, 16.0, 18.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local image = LoadImage("resources/heightmap.png")      -- Load heightmap image (RAM)
local texture = LoadTextureFromImage(image)             -- Convert image to texture (VRAM)
local map = LoadHeightmap(image, Vector3(16, 8, 16))    -- Load heightmap model with defined size
map.material.texDiffuse = texture                       -- Set map diffuse texture
local mapPosition = Vector3(-8.0, 0.0, -8.0)            -- Set model position (depends on model scaling!)

UnloadImage(image)                 -- Unload heightmap image from RAM, already uploaded to VRAM

SetCameraMode(camera, CameraMode.ORBITAL)   -- Set an orbital camera mode

SetTargetFPS(60)                            -- Set our game to run at 60 frames-per-second
----------------------------------------------------------------------------------------

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

            -- NOTE: Model is scaled to 1/4 of its original size (128x128 units)
            DrawModel(map, mapPosition, 1.0, RED)

            DrawGrid(20, 1.0)

        End3dMode()
        
        DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE)
        DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(texture)     -- Unload texture
UnloadModel(map)           -- Unload model

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------