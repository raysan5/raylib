-------------------------------------------------------------------------------------------
--
--  raylib [core] example - Gamepad input
--
--  NOTE: This example requires a Gamepad connected to the system
--        raylib is configured to work with Xbox 360 gamepad, check raylib.h for buttons configuration
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

InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input")

local ballPosition = Vector2(screenWidth/2, screenHeight/2)
local gamepadMovement = Vector2(0, 0)

SetTargetFPS(60)                   -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    if (IsGamepadAvailable(GAMEPAD.PLAYER1)) then
        gamepadMovement.x = GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LEFT_X)
        gamepadMovement.y = GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LEFT_Y)

        ballPosition.x = ballPosition.x + gamepadMovement.x
        ballPosition.y = ballPosition.y - gamepadMovement.y

        if (IsGamepadButtonPressed(GAMEPAD_PLAYER1, GAMEPAD_BUTTON_A)) then
            ballPosition.x = screenWidth/2
            ballPosition.y = screenHeight/2
        end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        DrawText("move the ball with gamepad", 10, 10, 20, DARKGRAY)

        DrawCircleV(ballPosition, 50, MAROON)

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------