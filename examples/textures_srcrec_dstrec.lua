-------------------------------------------------------------------------------------------
--
--  raylib [textures] example - Texture source and destination rectangles
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

InitWindow(screenWidth, screenHeight, "raylib [textures] examples - texture source and destination rectangles")

-- NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
local guybrush = LoadTexture("resources/guybrush.png")        -- Texture loading

local frameWidth = guybrush.width/7
local frameHeight = guybrush.height

-- NOTE: Source rectangle (part of the texture to use for drawing)
local sourceRec = Rectangle(0, 0, frameWidth, frameHeight)

-- NOTE: Destination rectangle (screen rectangle where drawing part of texture)
local destRec = Rectangle(screenWidth/2, screenHeight/2, frameWidth*2, frameHeight*2)

-- NOTE: Origin of the texture (rotation/scale point), it's relative to destination rectangle size
local origin = Vector2(frameWidth, frameHeight)

local rotation = 0

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    rotation = rotation + 1
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        -- NOTE: Using DrawTexturePro() we can easily rotate and scale the part of the texture we draw
        -- sourceRec defines the part of the texture we use for drawing
        -- destRec defines the rectangle where our texture part will fit (scaling it to fit)
        -- origin defines the point of the texture used as reference for rotation and scaling
        -- rotation defines the texture rotation (using origin as rotation point)
        DrawTexturePro(guybrush, sourceRec, destRec, origin, rotation, WHITE)

        DrawLine(destRec.x, 0, destRec.x, screenHeight, GRAY)
        DrawLine(0, destRec.y, screenWidth, destRec.y, GRAY)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(guybrush)       -- Texture unloading

CloseWindow()                -- Close window and OpenGL context
-------------------------------------------------------------------------------------------