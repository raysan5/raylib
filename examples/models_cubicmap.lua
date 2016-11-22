-------------------------------------------------------------------------------------------
--
--  raylib [models] example - Cubicmap loading and drawing
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

InitWindow(screenWidth, screenHeight, "raylib [models] example - cubesmap loading and drawing")

-- Define the camera to look into our 3d world
local camera = Camera(Vector3(16.0, 14.0, 16.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), 45.0)

local image = LoadImage("resources/cubicmap.png")      -- Load cubicmap image (RAM)
local cubicmap = LoadTextureFromImage(image)           -- Convert image to texture to display (VRAM)
local map = LoadCubicmap(image)                        -- Load cubicmap model (generate model from image)

-- NOTE: By default each cube is mapped to one part of texture atlas
local texture = LoadTexture("resources/cubicmap_atlas.png")    -- Load map texture
map.material.texDiffuse = texture                      -- Set map diffuse texture

local mapPosition = Vector3(-16.0, 0.0, -8.0)          -- Set model position

UnloadImage(image)     -- Unload cubesmap image from RAM, already uploaded to VRAM

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

            DrawModel(map, mapPosition, 1.0, WHITE)

        End3dMode()
        
        DrawTextureEx(cubicmap, (Vector2)(screenWidth - cubicmap.width*4 - 20, 20), 0.0, 4.0, WHITE)
        DrawRectangleLines(screenWidth - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN)
        
        DrawText("cubicmap image used to", 658, 90, 10, GRAY)
        DrawText("generate map 3d model", 658, 104, 10, GRAY)

        DrawFPS(10, 10)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(cubicmap)    -- Unload cubicmap texture
UnloadTexture(texture)     -- Unload map texture
UnloadModel(map)           -- Unload map model

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------