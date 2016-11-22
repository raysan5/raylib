-------------------------------------------------------------------------------------------
--
--  raylib [text] example - TTF loading and usage
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800;
local screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [text] example - ttf loading")

local msg = "TTF SpriteFont"

-- NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)

-- TTF SpriteFont loading with custom generation parameters
local font = LoadSpriteFontTTF("resources/fonts/KAISG.ttf", 96, 0, 0)

-- Generate mipmap levels to use trilinear filtering
-- NOTE: On 2D drawing it won't be noticeable, it looks like FILTER_BILINEAR
--font.texture = GenTextureMipmaps(font.texture) -- ISSUE: attempt to index a SpriteFont value (local 'font')

local fontSize = font.size
local fontPosition = Vector2(40, screenHeight/2 + 50)
local textSize

SetTextureFilter(font.texture, TextureFilter.POINT)
local currentFontFilter = 0      -- Default: FILTER_POINT

local count = 0
local droppedFiles

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    fontSize = fontSize + GetMouseWheelMove()*4.0
    
    -- Choose font texture filter method
    if (IsKeyPressed(KEY.ONE)) then
        SetTextureFilter(font.texture, TextureFilter.POINT)
        currentFontFilter = 0
    elseif (IsKeyPressed(KEY.TWO)) then
        SetTextureFilter(font.texture, TextureFilter.BILINEAR)
        currentFontFilter = 1
    elseif (IsKeyPressed(KEY.THREE)) then
        -- NOTE: Trilinear filter won't be noticed on 2D drawing
        SetTextureFilter(font.texture, TextureFilter.TRILINEAR)
        currentFontFilter = 2
    end
    
    textSize = MeasureTextEx(font, msg, fontSize, 0)
    
    if (IsKeyDown(KEY.LEFT)) then fontPosition.x = fontPosition.x - 10
    elseif (IsKeyDown(KEY.RIGHT)) then fontPosition.x = fontPosition.x + 10
    end
    
    -- Load a dropped TTF file dynamically (at current fontSize)
    if (IsFileDropped()) then
        droppedFiles = GetDroppedFiles() 
        count = #droppedFiles
        
        if (count == 1) then -- Only support one ttf file dropped
            UnloadSpriteFont(font)
            font = LoadSpriteFontTTF(droppedFiles[1], fontSize, 0, 0)
            ClearDroppedFiles()
        end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("Use mouse wheel to change font size", 20, 20, 10, GRAY)
        DrawText("Use KEY_RIGHT and KEY_LEFT to move text", 20, 40, 10, GRAY)
        DrawText("Use 1, 2, 3 to change texture filter", 20, 60, 10, GRAY)
        DrawText("Drop a new TTF font for dynamic loading", 20, 80, 10, DARKGRAY)

        DrawTextEx(font, msg, fontPosition, fontSize, 0, BLACK)
        
        -- TODO: It seems texSize measurement is not accurate due to chars offsets...
        --DrawRectangleLines(fontPosition.x, fontPosition.y, textSize.x, textSize.y, RED)
        
        DrawRectangle(0, screenHeight - 80, screenWidth, 80, LIGHTGRAY)
        DrawText(string.format("Font size: %02.02f", fontSize), 20, screenHeight - 50, 10, DARKGRAY)
        DrawText(string.format("Text size: [%02.02f, %02.02f]", textSize.x, textSize.y), 20, screenHeight - 30, 10, DARKGRAY)
        DrawText("CURRENT TEXTURE FILTER:", 250, 400, 20, GRAY)
        
        if (currentFontFilter == 0) then DrawText("POINT", 570, 400, 20, BLACK)
        elseif (currentFontFilter == 1) then DrawText("BILINEAR", 570, 400, 20, BLACK)
        elseif (currentFontFilter == 2) then DrawText("TRILINEAR", 570, 400, 20, BLACK)
        end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadSpriteFont(font)     -- SpriteFont unloading

ClearDroppedFiles()        -- Clear internal buffers

CloseWindow()               -- Close window and OpenGL context
-------------------------------------------------------------------------------------------