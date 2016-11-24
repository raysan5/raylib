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

local texPs3Pad = LoadTexture("resources/ps3.png")
local texXboxPad = LoadTexture("resources/xbox.png")

SetTargetFPS(60)                   -- Set target frames-per-second
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do    -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    -- ...
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(RAYWHITE)

        if (IsGamepadAvailable(GAMEPAD.PLAYER1)) then
            DrawText(string.format("GP1: %s", GetGamepadName(GAMEPAD.PLAYER1)), 10, 10, 10, BLACK)

            if (IsGamepadName(GAMEPAD.PLAYER1, "Xbox 360 Controller")) then
                DrawTexture(texXboxPad, 0, 0, DARKGRAY)
                
                -- Draw buttons: xbox home
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_HOME)) then DrawCircle(394, 89, 19, RED) end

                -- Draw buttons: basic
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_START)) then DrawCircle(436, 150, 9, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_SELECT)) then DrawCircle(352, 150, 9, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_X)) then DrawCircle(501, 151, 15, BLUE) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_A)) then DrawCircle(536, 187, 15, LIME) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_B)) then DrawCircle(572, 151, 15, MAROON) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_Y)) then DrawCircle(536, 115, 15, GOLD) end
                
                -- Draw buttons: d-pad
                DrawRectangle(317, 202, 19, 71, BLACK)
                DrawRectangle(293, 228, 69, 19, BLACK)
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_UP)) then DrawRectangle(317, 202, 19, 26, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_DOWN)) then DrawRectangle(317, 202 + 45, 19, 26, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_LEFT)) then DrawRectangle(292, 228, 25, 19, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_RIGHT)) then DrawRectangle(292 + 44, 228, 26, 19, RED) end
                
                -- Draw buttons: left-right back
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_LB)) then DrawCircle(259, 61, 20, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.XBOX_BUTTON_RB)) then DrawCircle(536, 61, 20, RED) end

                -- Draw axis: left joystick
                DrawCircle(259, 152, 39, BLACK)
                DrawCircle(259, 152, 34, LIGHTGRAY)
                DrawCircle(259 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LEFT_X)*20), 
                           152 - (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LEFT_Y)*20), 25, BLACK)
                
                -- Draw axis: right joystick
                DrawCircle(461, 237, 38, BLACK)
                DrawCircle(461, 237, 33, LIGHTGRAY)
                DrawCircle(461 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_RIGHT_X)*20), 
                           237 - (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_RIGHT_Y)*20), 25, BLACK)

                -- Draw axis: left-right triggers
                DrawRectangle(170, 30, 15, 70, GRAY)
                DrawRectangle(604, 30, 15, 70, GRAY)              
                DrawRectangle(170, 30, 15, (((1.0 + GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LT))/2.0)*70), RED)
                DrawRectangle(604, 30, 15, (((1.0 + GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_RT))/2.0)*70), RED)
                
                --DrawText(FormatText("Xbox axis LT: %02.02f", GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_LT)), 10, 40, 10, BLACK)
                --DrawText(FormatText("Xbox axis RT: %02.02f", GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.XBOX_AXIS_RT)), 10, 60, 10, BLACK)
            elseif (IsGamepadName(GAMEPAD.PLAYER1, "PLAYSTATION(R)3 Controller")) then
                DrawTexture(texPs3Pad, 0, 0, DARKGRAY)

                -- Draw buttons: ps
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_PS)) then DrawCircle(396, 222, 13, RED) end
                
                -- Draw buttons: basic
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_SELECT)) then DrawRectangle(328, 170, 32, 13, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_START)) then DrawTriangle((Vector2){ 436, 168 }, (Vector2){ 436, 185 }, (Vector2){ 464, 177 }, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_TRIANGLE)) then DrawCircle(557, 144, 13, LIME) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_CIRCLE)) then DrawCircle(586, 173, 13, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_CROSS)) then DrawCircle(557, 203, 13, VIOLET) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_SQUARE)) then DrawCircle(527, 173, 13, PINK) end

                -- Draw buttons: d-pad
                DrawRectangle(225, 132, 24, 84, BLACK)
                DrawRectangle(195, 161, 84, 25, BLACK)
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_UP)) then DrawRectangle(225, 132, 24, 29, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_DOWN)) then DrawRectangle(225, 132 + 54, 24, 30, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_LEFT)) then DrawRectangle(195, 161, 30, 25, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_RIGHT)) then DrawRectangle(195 + 54, 161, 30, 25, RED) end
                
                -- Draw buttons: left-right back buttons
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_L1)) then DrawCircle(239, 82, 20, RED) end
                if (IsGamepadButtonDown(GAMEPAD.PLAYER1, GAMEPAD.PS3_BUTTON_R1)) then DrawCircle(557, 82, 20, RED) end

                -- Draw axis: left joystick
                DrawCircle(319, 255, 35, BLACK)
                DrawCircle(319, 255, 31, LIGHTGRAY)
                DrawCircle(319 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_LEFT_X)*20), 
                           255 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_LEFT_Y)*20), 25, BLACK)
                
                -- Draw axis: right joystick
                DrawCircle(475, 255, 35, BLACK)
                DrawCircle(475, 255, 31, LIGHTGRAY)
                DrawCircle(475 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_RIGHT_X)*20), 
                           255 + (GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_RIGHT_Y)*20), 25, BLACK)

                -- Draw axis: left-right triggers
                DrawRectangle(169, 48, 15, 70, GRAY)
                DrawRectangle(611, 48, 15, 70, GRAY)              
                DrawRectangle(169, 48, 15, (((1.0 - GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_L2))/2.0)*70), RED)
                DrawRectangle(611, 48, 15, (((1.0 - GetGamepadAxisMovement(GAMEPAD.PLAYER1, GAMEPAD.PS3_AXIS_R2))/2.0)*70), RED)
            else
                DrawText("- GENERIC GAMEPAD -", 280, 180, 20, GRAY)
                
                -- TODO: Draw generic gamepad
            end
            
            DrawText(string.format("DETECTED AXIS [%i]:", GetGamepadAxisCount(GAMEPAD.PLAYER1)), 10, 50, 10, MAROON) 
            
            for i = 1, GetGamepadAxisCount(GAMEPAD.PLAYER1) do    -- Iterate along all the rectangles
                DrawText(string.format("AXIS %i: %.02f", i, GetGamepadAxisMovement(GAMEPAD.PLAYER1, i)), 20, 70 + 20*i, 10, DARKGRAY)
            end
            
            if (GetGamepadButtonPressed() ~= -1) then DrawText(string.format("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RED)
            else DrawText("DETECTED BUTTON: NONE", 10, 430, 10, GRAY) end
        else
            DrawText("GP1: NOT DETECTED", 10, 10, 10, GRAY)
            
            DrawTexture(texXboxPad, 0, 0, LIGHTGRAY)
        end

    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(texPs3Pad)   -- Unload gamepad texture
UnloadTexture(texXboxPad)  -- Unload gamepad texture
    
CloseWindow()               -- Close window and OpenGL context
-------------------------------------------------------------------------------------------