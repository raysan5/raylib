-------------------------------------------------------------------------------------------
--
--   raylib [audio] example - Module playing (streaming)
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

MAX_CIRCLES = 64

--[[
typedef struct {        -- TODO: Find a Lua alternative: TABLES?
    Vector2 position
    float radius
    float alpha
    float speed
    Color color
} CircleWave
--]]

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [audio] example - module playing (streaming)")

InitAudioDevice()              -- Initialize audio device

local colors = { ORANGE, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK,
                 YELLOW, GREEN, SKYBLUE, PURPLE, BEIGE }

-- Creates ome circles for visual effect
local circles = {}

for i = MAX_CIRCLES, 1, -1 do
    circles[i] = {}
    circles[i].alpha = 0.0
    circles[i].radius = GetRandomValue(10, 40)
    circles[i].position = Vector2(0, 0)
    circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius)
    circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius)
    circles[i].speed = GetRandomValue(1, 100)/20000.0
    circles[i].color = colors[GetRandomValue(1, 14)]
end

-- Load postprocessing bloom shader
local shader = LoadShader("resources/shaders/glsl330/base.vs", 
                          "resources/shaders/glsl330/bloom.fs")

-- Create a RenderTexture2D to be used for render to texture
local target = LoadRenderTexture(screenWidth, screenHeight)

local xm = LoadMusicStream("resources/audio/mini1111.xm")

PlayMusicStream(xm)

local timePlayed = 0.0

SetTargetFPS(60)               -- Set our game to run at 60 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do        -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    for i = MAX_CIRCLES, 1, -1 do
        circles[i].alpha = circles[i].alpha + circles[i].speed
        circles[i].radius = circles[i].radius + circles[i].speed*10.0
        
        if (circles[i].alpha > 1.0) then circles[i].speed = circles[i].speed*-1 end
        
        if (circles[i].alpha <= 0.0) then
            circles[i].alpha = 0.0
            circles[i].radius = GetRandomValue(10, 40)
            circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius)
            circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius)
            circles[i].color = colors[GetRandomValue(0, 13)]
            circles[i].speed = GetRandomValue(1, 100)/20000.0
        end
    end

    -- Get timePlayed scaled to bar dimensions
    timePlayed = (GetMusicTimePlayed(xm)/GetMusicTimeLength(xm)*(screenWidth - 40))*2
    
    UpdateMusicStream(xm)        -- Update music buffer with new stream data
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(BLACK)
        
        BeginTextureMode(target)   -- Enable drawing to texture

            for i = MAX_CIRCLES, 1, -1 do
                DrawCircleV(circles[i].position, circles[i].radius, Fade(circles[i].color, circles[i].alpha))
            end
            
        EndTextureMode()           -- End drawing to texture (now we have a texture available for next passes)
        
        BeginShaderMode(shader)

            -- NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
            DrawTextureRec(target.texture, Rectangle(0, 0, target.texture.width, -target.texture.height), Vector2(0, 0), WHITE)
            
        EndShaderMode()

        -- Draw time bar
        DrawRectangle(20, screenHeight - 20 - 12, screenWidth - 40, 12, LIGHTGRAY)
        DrawRectangle(20, screenHeight - 20 - 12, timePlayed, 12, MAROON)
        DrawRectangleLines(20, screenHeight - 20 - 12, screenWidth - 40, 12, WHITE)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadShader(shader)           -- Unload shader
UnloadRenderTexture(target)    -- Unload render texture

UnloadMusicStream(xm)          -- Unload music stream buffers from RAM

CloseAudioDevice()     -- Close audio device (music streaming is automatically stopped)

CloseWindow()          -- Close window and OpenGL context
-------------------------------------------------------------------------------------------
