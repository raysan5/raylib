-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Gestures Detection
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

MAX_GESTURE_STRINGS = 20

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [core] example - gestures detection")

local touchPosition = Vector2(0, 0)
local touchArea = Rectangle(220, 10, screenWidth - 230, screenHeight - 20)

local gesturesCount = 0
local gestureStrings = {}

for i = 1, MAX_GESTURE_STRINGS do gestureStrings[i] = "" end

local currentGesture = Gestures.NONE
local lastGesture = Gestures.NONE

--SetGesturesEnabled(0b0000000000001001)   -- Enable only some gestures to be detected

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    lastGesture = currentGesture
    currentGesture = GetGestureDetected()
    touchPosition = GetTouchPosition(0)

    if (CheckCollisionPointRec(touchPosition, touchArea) and (currentGesture ~= Gestures.NONE)) then
        if (currentGesture ~= lastGesture) then
            -- Store gesture string
            if (currentGesture == Gestures.TAP) then gestureStrings[gesturesCount] = "GESTURE TAP"
            elseif (currentGesture == Gestures.DOUBLETAP) then gestureStrings[gesturesCount] = "GESTURE DOUBLETAP"
            elseif (currentGesture == Gestures.HOLD) then gestureStrings[gesturesCount] = "GESTURE HOLD"
            elseif (currentGesture == Gestures.DRAG) then gestureStrings[gesturesCount] = "GESTURE DRAG"
            elseif (currentGesture == Gestures.SWIPE_RIGHT) then gestureStrings[gesturesCount] = "GESTURE SWIPE RIGHT"
            elseif (currentGesture == Gestures.SWIPE_LEFT) then gestureStrings[gesturesCount] = "GESTURE SWIPE LEFT"
            elseif (currentGesture == Gestures.SWIPE_UP) then gestureStrings[gesturesCount] = "GESTURE SWIPE UP"
            elseif (currentGesture == Gestures.SWIPE_DOWN) then gestureStrings[gesturesCount] = "GESTURE SWIPE DOWN"
            elseif (currentGesture == Gestures.PINCH_IN) then gestureStrings[gesturesCount] = "GESTURE PINCH IN"
            elseif (currentGesture == Gestures.PINCH_OUT) then gestureStrings[gesturesCount] = "GESTURE PINCH OUT"
            end
            
            gesturesCount = gesturesCount + 1
            
            -- Reset gestures strings
            if (gesturesCount >= MAX_GESTURE_STRINGS) then
                for i = 1, MAX_GESTURE_STRINGS do gestureStrings[i] = "\0" end
                gesturesCount = 0
            end
        end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)
        
        DrawRectangleRec(touchArea, GRAY)
        DrawRectangle(225, 15, screenWidth - 240, screenHeight - 30, RAYWHITE)
        
        DrawText("GESTURES TEST AREA", screenWidth - 270, screenHeight - 40, 20, Fade(GRAY, 0.5))
        
        for i = 1, gesturesCount do
            if ((i - 1)%2 == 0) then DrawRectangle(10, 30 + 20*(i - 1), 200, 20, Fade(LIGHTGRAY, 0.5))
            else DrawRectangle(10, 30 + 20*(i - 1), 200, 20, Fade(LIGHTGRAY, 0.3)) end
            
            if (i < gesturesCount) then DrawText(gestureStrings[i], 35, 36 + 20*(i - 1), 10, DARKGRAY)
            else DrawText(gestureStrings[i], 35, 36 + 20*(i - 1), 10, MAROON) end
        end
        
        DrawRectangleLines(10, 29, 200, screenHeight - 50, GRAY)
        DrawText("DETECTED GESTURES", 50, 15, 10, GRAY)
        
        if (currentGesture ~= GESTURE_NONE) then DrawCircleV(touchPosition, 30, MAROON) end
        
    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------