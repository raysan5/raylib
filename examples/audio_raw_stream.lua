-------------------------------------------------------------------------------------------
--
--  raylib [audio] example - Raw audio streaming
--
--  NOTE: This example requires OpenAL Soft library installed
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

MAX_SAMPLES = 20000
DEG2RAD = math.pi/180.0

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw audio streaming")

InitAudioDevice()              -- Initialize audio device

-- Init raw audio stream (sample rate: 22050, sample size: 32bit-float, channels: 1-mono)
local stream = InitAudioStream(22050, 32, 1)

-- Fill audio stream with some samples (sine wave)
local data = {}

for i = 1, MAX_SAMPLES do
    data[i] = math.sin(((2*math.pi*i)/2)*DEG2RAD)
end

-- NOTE: The generated MAX_SAMPLES do not fit to close a perfect loop
-- for that reason, there is a clip everytime audio stream is looped

PlayAudioStream(stream)

local totalSamples = MAX_SAMPLES
local samplesLeft = totalSamples

local position = Vector2(0, 0)

SetTargetFPS(30)               -- Set our game to run at 30 frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    
    -- Refill audio stream if required
    if (IsAudioBufferProcessed(stream)) then
        local numSamples = 0
        
        if (samplesLeft >= 4096) then numSamples = 4096
        else numSamples = samplesLeft end

        UpdateAudioStream(stream, data + (totalSamples - samplesLeft), numSamples)
        
        samplesLeft = samplesLeft - numSamples
        
        -- Reset samples feeding (loop audio)
        if (samplesLeft <= 0) then samplesLeft = totalSamples end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("SINE WAVE SHOULD BE PLAYING!", 240, 140, 20, LIGHTGRAY)
        
        -- NOTE: Draw a part of the sine wave (only screen width)
        for i = 1, GetScreenWidth() do
            position.x = (i - 1)
            position.y = 250 + 50*data[i]
            
            DrawPixelV(position, RED)
        end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseAudioStream(stream)   -- Close raw audio stream and delete buffers from RAM

CloseAudioDevice()         -- Close audio device (music streaming is automatically stopped)

CloseWindow()              -- Close window and OpenGL context
-------------------------------------------------------------------------------------------