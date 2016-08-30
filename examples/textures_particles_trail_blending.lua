-------------------------------------------------------------------------------------------
--
--  raylib example - particles trail blending
--
--  This example has been created using raylib 1.6 (www.raylib.com)
--  raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
--
--  Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
--
-------------------------------------------------------------------------------------------

MAX_PARTICLES = 200

-- Initialization
-------------------------------------------------------------------------------------------
local screenWidth = 800
local screenHeight = 450

InitWindow(screenWidth, screenHeight, "raylib [textures] example - particles trail blending")

-- Particles pool, reuse them!
local mouseTail = {}

-- Initialize particles
for i = 1, MAX_PARTICLES do
    mouseTail[i] = {}
    mouseTail[i].position = Vector2(0, 0)
    mouseTail[i].color = Color(GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255)
    mouseTail[i].alpha = 1.0
    mouseTail[i].size = GetRandomValue(1, 30)/20.0
    mouseTail[i].rotation = GetRandomValue(0, 360)
    mouseTail[i].active = false
end

local gravity = 3.0

local smoke = LoadTexture("resources/smoke.png")

local blending = BlendMode.ALPHA

SetTargetFPS(60)
-------------------------------------------------------------------------------------------

-- Main game loop
while not WindowShouldClose() do            -- Detect window close button or ESC key
    -- Update
    ---------------------------------------------------------------------------------------
    
    -- Activate one particle every frame and Update active particles
    -- NOTE: Particles initial position should be mouse position when activated
    -- NOTE: Particles fall down with gravity and rotation... and disappear after 2 seconds (alpha = 0)
    -- NOTE: When a particle disappears, active = false and it can be reused.
    for i = 1, MAX_PARTICLES do
        if (not mouseTail[i].active) then
            mouseTail[i].active = true
            mouseTail[i].alpha = 1.0
            mouseTail[i].position = GetMousePosition()
            break
        end
    end

    for i = 1, MAX_PARTICLES do
        if (mouseTail[i].active) then
            mouseTail[i].position.y = mouseTail[i].position.y + gravity
            mouseTail[i].alpha = mouseTail[i].alpha - 0.01
            
            if (mouseTail[i].alpha <= 0.0) then mouseTail[i].active = false end
            
            mouseTail[i].rotation = mouseTail[i].rotation + 5.0
        end
    end
    
    if (IsKeyPressed(KEY.SPACE)) then
        if (blending == BlendMode.ALPHA) then blending = BlendMode.ADDITIVE
        else blending = BlendMode.ALPHA end
    end
    ---------------------------------------------------------------------------------------

    -- Draw
    ---------------------------------------------------------------------------------------
    BeginDrawing()

        ClearBackground(DARKGRAY)
        
        BeginBlendMode(blending)

            -- Draw active particles
            for i = 1, MAX_PARTICLES do
                if (mouseTail[i].active) then 
                    DrawTexturePro(smoke, Rectangle(0, 0, smoke.width, smoke.height), 
                        Rectangle(mouseTail[i].position.x, mouseTail[i].position.y, 
                                  smoke.width*mouseTail[i].size//1, smoke.height*mouseTail[i].size//1),
                        Vector2(smoke.width*mouseTail[i].size/2, smoke.height*mouseTail[i].size/2), 
                        mouseTail[i].rotation, Fade(mouseTail[i].color, mouseTail[i].alpha)) end
            end
        
        EndBlendMode()
        
        DrawText("PRESS SPACE to CHANGE BLENDING MODE", 180, 20, 20, BLACK)
        
        if (blending == BlendMode.ALPHA) then DrawText("ALPHA BLENDING", 290, screenHeight - 40, 20, BLACK)
        else DrawText("ADDITIVE BLENDING", 280, screenHeight - 40, 20, RAYWHITE) end
        
    EndDrawing()
    ---------------------------------------------------------------------------------------
end

-- De-Initialization
-------------------------------------------------------------------------------------------
UnloadTexture(smoke)

CloseWindow()        -- Close window and OpenGL context
-------------------------------------------------------------------------------------------