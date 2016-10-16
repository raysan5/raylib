-------------------------------------------------------------------------------------------
--
--  raylib [textures] example - Image loading and drawing on it
--
--  NOTE: Images are loaded in CPU memory (RAM) textures are loaded in GPU memory (VRAM)
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

InitWindow(screenWidth, screenHeight, "raylib [textures] example - image drawing")

-- NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

local cat = LoadImage("resources/cat.png")             -- Load image in CPU memory (RAM)
cat = ImageCrop(cat, Rectangle(100, 10, 280, 380))     -- Crop an image piece
cat = ImageFlipHorizontal(cat)                         -- Flip cropped image horizontally
cat = ImageResize(cat, 150, 200)                       -- Resize flipped-cropped image

local parrots = LoadImage("resources/parrots.png")     -- Load image in CPU memory (RAM)

-- Draw one image over the other with a scaling of 1.5f
parrots = ImageDraw(parrots, cat, Rectangle(0, 0, cat.width, cat.height), Rectangle(30, 40, cat.width*1.5, cat.height*1.5))
parrots = ImageCrop(parrots, Rectangle(0, 50, parrots.width, parrots.height - 100)) -- Crop resulting image

UnloadImage(cat)       -- Unload image from RAM

local texture = LoadTextureFromImage(parrots)      -- Image converted to texture, uploaded to GPU memory (VRAM)
UnloadImage(parrots)   -- Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

SetTargetFPS(60)
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

        DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2 - 40, WHITE)
        DrawRectangleLines(screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2 - 40, texture.width, texture.height, DARKGRAY)

        DrawText("We are drawing only one texture from various images composed!", 240, 350, 10, DARKGRAY)
        DrawText("Source images have been cropped, scaled, flipped and copied one over the other.", 190, 370, 10, DARKGRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(texture)       -- Texture unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------