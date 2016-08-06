-------------------------------------------------------------------------------------------
--
--  raylib [textures] example - Image processing
--
--  NOTE: Images are loaded in CPU memory (RAM) textures are loaded in GPU memory (VRAM)
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

--#include <stdlib.h>     -- Required for: free()

NUM_PROCESSES = 8

-- enum ImageProcess
local COLOR_NONE = 1
local COLOR_GRAYSCALE = 2
local COLOR_TINT = 3
local COLOR_INVERT = 4
local COLOR_CONTRAST = 5
local COLOR_BRIGHTNESS = 6
local FLIP_VERTICAL = 7
local FLIP_HORIZONTAL = 8

local processText = {
    "NO PROCESSING",
    "COLOR GRAYSCALE",
    "COLOR TINT",
    "COLOR INVERT",
    "COLOR CONTRAST",
    "COLOR BRIGHTNESS",
    "FLIP VERTICAL",
    "FLIP HORIZONTAL"
}

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [textures] example - image processing")

-- NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

local image = LoadImage("resources/parrots.png")   -- Loaded in CPU memory (RAM)
image = ImageFormat(image, TextureFormat.UNCOMPRESSED_R8G8B8A8)         -- Format image to RGBA 32bit (required for texture update)
local texture = LoadTextureFromImage(image)        -- Image converted to texture, GPU memory (VRAM)

local currentProcess = COLOR_NONE
local textureReload = false

local selectRecs = {}

for i = 1, NUM_PROCESSES do selectRecs[i] = Rectangle(40, 50 + 32*i, 150, 30) end

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsKeyPressed(KEY.DOWN)) then
        currentProcess = currentProcess + 1
        if (currentProcess > NUM_PROCESSES) then currentProcess = 1 end
        textureReload = true
    elseif (IsKeyPressed(KEY.UP)) then
        currentProcess = currentProcess - 1
        if (currentProcess < 1) then currentProcess = NUM_PROCESSES end
        textureReload = true
    end
    
    if (textureReload) then
        UnloadImage(image)                         -- Unload current image data
        image = LoadImage("resources/parrots.png") -- Re-load image data

        -- NOTE: Image processing is a costly CPU process to be done every frame, 
        -- If image processing is required in a frame-basis, it should be done 
        -- with a texture and by shaders
        if (currentProcess == COLOR_GRAYSCALE) then image = ImageColorGrayscale(image)
        elseif (currentProcess == COLOR_TINT) then image = ImageColorTint(image, GREEN)
        elseif (currentProcess == COLOR_INVERT) then image = ImageColorInvert(image)
        elseif (currentProcess == COLOR_CONTRAST) then image = ImageColorContrast(image, -40)
        elseif (currentProcess == COLOR_BRIGHTNESS) then image = ImageColorBrightness(image, -80)
        elseif (currentProcess == FLIP_VERTICAL) then image = ImageFlipVertical(image)
        elseif (currentProcess == FLIP_HORIZONTAL) then image = ImageFlipHorizontal(image)
        end
        
        local pixels = {}
        pixels = GetImageData(image)                -- Get pixel data from image (RGBA 32bit)
        texture = UpdateTexture(texture, pixels)    -- Update texture with new image data

        textureReload = false
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)
        
        DrawText("IMAGE PROCESSING:", 40, 30, 10, DARKGRAY)
        
        -- Draw rectangles
        for i = 1, NUM_PROCESSES do
            if (i == currentProcess) then
                DrawRectangleRec(selectRecs[i], SKYBLUE)
                DrawRectangleLines(selectRecs[i].x, selectRecs[i].y, selectRecs[i].width, selectRecs[i].height, BLUE)
                DrawText(processText[i], selectRecs[i].x + selectRecs[i].width/2 - MeasureText(processText[i], 10)//2, selectRecs[i].y + 11, 10, DARKBLUE)
            else
                DrawRectangleRec(selectRecs[i], LIGHTGRAY)
                DrawRectangleLines(selectRecs[i].x, selectRecs[i].y, selectRecs[i].width, selectRecs[i].height, GRAY)
                DrawText(processText[i], selectRecs[i].x + selectRecs[i].width/2 - MeasureText(processText[i], 10)//2, selectRecs[i].y + 11, 10, DARKGRAY)
            end
        end

        DrawTexture(texture, screenWidth - texture.width - 60, screenHeight/2 - texture.height/2, WHITE)
        DrawRectangleLines(screenWidth - texture.width - 60, screenHeight/2 - texture.height/2, texture.width, texture.height, BLACK)
        
    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(texture)       -- Unload texture from VRAM
UnloadImage(image)           -- Unload image from RAM

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------