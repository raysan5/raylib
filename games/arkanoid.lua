--[[

   raylib - sample game: arkanoid

   Sample game Marc Palau and Ramon Santamaria

   This game has been created using raylib v1.3 (www.raylib.com)
   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)

   Copyright (c) 2015 Ramon Santamaria (@raysan5)
   
   Translated to Lua by Ghassan Al-Mashareqa (ghassan@ghassan.pl)

--]]

------------------------------------------------------------------------------------
-- Some Defines
------------------------------------------------------------------------------------
PLAYER_MAX_LIFE = 5
LINES_OF_BRICKS = 5
BRICKS_PER_LINE = 20

------------------------------------------------------------------------------------
-- Types and Structures Definition
------------------------------------------------------------------------------------

GameScreen = { LOGO = 0, TITLE = 1, GAMEPLAY = 2, ENDING = 3 }

function Player()
	return { position = Vector2(0,0), size = Vector2(0,0), life = 0 }
end

function Ball()
	return { position = Vector2(0,0), speed = Vector2(0,0), radius = 0, active = false }
end

function Brick()
	return { position = Vector2(0,0), active = false }
end

--------------------------------------------------------------------------------------
-- Global Variables Declaration
--------------------------------------------------------------------------------------
screenWidth = 800;
screenHeight = 450;

framesCounter = 0;
gameOver = false;
pause = false;

player = Player()
ball = Ball()
brick = {}--[LINES_OF_BRICKS][BRICKS_PER_LINE];
for i = 0, LINES_OF_BRICKS-1 do
	brick[i] = {}
	for j = 0, BRICKS_PER_LINE-1 do
		brick[i][j] = Brick()
	end
end
brickSize = Vector2(0,0)


--------------------------------------------------------------------------------------
-- Module Functions Definitions (local)
--------------------------------------------------------------------------------------

-- Initialize game variables
function InitGame()

    brickSize = Vector2(GetScreenWidth()/BRICKS_PER_LINE, 40)

    -- Initialize player
    player.position = Vector2(screenWidth/2, screenHeight*7/8)
    player.size = Vector2(screenWidth/10, 20)
    player.life = PLAYER_MAX_LIFE;
    
    -- Initialize ball
    ball.position = Vector2(screenWidth/2, screenHeight*7/8 - 30)
    ball.speed = Vector2(0, 0)
    ball.radius = 7;
    ball.active = false;
    
    -- Initialize bricks
    local initialDownPosition = 50;

    for i = 0, LINES_OF_BRICKS-1 do
        for j = 0, BRICKS_PER_LINE-1 do
            brick[i][j].position = Vector2(j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition)
            brick[i][j].active = true;
        end
    end
end

-- Update game (one frame)
function UpdateGame()

    if (not gameOver) then
        if (IsKeyPressed(KEY.P)) then pause = not pause; end

        if (not pause) then
            -- Player movement
            if (IsKeyDown(KEY.LEFT)) then player.position.x = player.position.x - 5; end
            if ((player.position.x - player.size.x/2) <= 0) then player.position.x = player.size.x/2; end
            if (IsKeyDown(KEY.RIGHT)) then player.position.x = player.position.x + 5; end
            if ((player.position.x + player.size.x/2) >= screenWidth) then player.position.x = screenWidth - player.size.x/2; end

            -- Launch ball
            if (not ball.active) then
                if (IsKeyPressed(KEY.SPACE)) then
                    ball.active = true;
                    ball.speed = Vector2(0, -5)
                end
            end
            
            UpdateBall();

            -- Game over logic
            if (player.life <= 0) then
							gameOver = true;
            else
                gameOver = true;
                
                for i = 0, LINES_OF_BRICKS-1 do
                    for j = 0, BRICKS_PER_LINE-1 do
                        if (brick[i][j].active) then gameOver = false; end
                    end
                end
            end
        end
    else
        if (IsKeyPressed(KEY.ENTER)) then
            InitGame();
            gameOver = false;
        end
    end

end

-- Draw game (one frame)
function DrawGame()

    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (not gameOver) then
            -- Draw player bar
            DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, BLACK);

            -- Draw player lives
            for i = 0, player.life-1 do
							DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, LIGHTGRAY);
						end
            
            -- Draw ball
            DrawCircleV(ball.position, ball.radius, MAROON);
            
            -- Draw bricks
            for i = 0, LINES_OF_BRICKS-1 do
                for j = 0, BRICKS_PER_LINE-1 do
                    if (brick[i][j].active) then
                        if ((i + j) % 2 == 0) then
													DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, GRAY);
                        else 
													DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, DARKGRAY);
												end
                    end
                end
            end
            
            if (pause) then
							DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
						end
        else 
					DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
				end

    EndDrawing();
end

-- Unload game variables
function UnloadGame()
    -- TODO: Unload all dynamic loaded data (textures, sounds, models...)
end

-- Update and Draw (one frame)
function UpdateDrawFrame()
    UpdateGame();
    DrawGame();
end

----------------------------------------------------------------------------------------
-- Additional module functions
----------------------------------------------------------------------------------------
function UpdateBall()
    -- Update position
    if (ball.active) then
        ball.position.x = ball.position.x + ball.speed.x;
        ball.position.y = ball.position.y + ball.speed.y;
    else
        ball.position = Vector2(player.position.x, screenHeight*7/8 - 30);
    end

    -- Bounce in x
    if (((ball.position.x + ball.radius) >= screenWidth) or ((ball.position.x - ball.radius) <= 0)) 
    then
			ball.speed.x = ball.speed.x * -1;
		end

    -- Bounce in y
    if ((ball.position.y - ball.radius) <= 0) then
			ball.speed.y = ball.speed.y * -1;
		end

    -- Ball reaches bottom of the screen
    if ((ball.position.y + ball.radius) >= screenHeight) then
        ball.speed = Vector2(0, 0);
        ball.active = false;

        player.life = player.life - 1;
    end

    -- Collision logic: ball vs player
    if CheckCollisionCircleRec(ball.position, ball.radius, 
				Rectangle(
					player.position.x - player.size.x/2, 
					player.position.y - player.size.y/2, 
					player.size.x, 
					player.size.y)) then
        if (ball.speed.y > 0) then
            ball.speed.y = ball.speed.y * -1;
            ball.speed.x = (ball.position.x - player.position.x)/(player.size.x/2)*5;
        end
    end

    -- Collision logic: ball vs bricks
    for i = 0,LINES_OF_BRICKS-1 do
        for j = 0,BRICKS_PER_LINE-1 do
            if (brick[i][j].active) then
                -- Hit below
                if (((ball.position.y - ball.radius) <= (brick[i][j].position.y + brickSize.y/2)) and
                    ((ball.position.y - ball.radius) > (brick[i][j].position.y + brickSize.y/2 + ball.speed.y)) and
                    ((math.abs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) and (ball.speed.y < 0))
                then
                    brick[i][j].active = false;
                    ball.speed.y = ball.speed.y * -1;
                -- Hit above
                elseif (((ball.position.y + ball.radius) >= (brick[i][j].position.y - brickSize.y/2)) and
                        ((ball.position.y + ball.radius) < (brick[i][j].position.y - brickSize.y/2 + ball.speed.y)) and
                        ((math.abs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) and (ball.speed.y > 0))
                then
                    brick[i][j].active = false;
                    ball.speed.y = ball.speed.y * -1;
                -- Hit left
                elseif (((ball.position.x + ball.radius) >= (brick[i][j].position.x - brickSize.x/2)) and
                        ((ball.position.x + ball.radius) < (brick[i][j].position.x - brickSize.x/2 + ball.speed.x)) and
                        ((math.abs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) and (ball.speed.x > 0))
                then
                    brick[i][j].active = false;
                    ball.speed.x = ball.speed.x * -1;
                -- Hit right
                elseif (((ball.position.x - ball.radius) <= (brick[i][j].position.x + brickSize.x/2)) and
                        ((ball.position.x - ball.radius) > (brick[i][j].position.x + brickSize.x/2 + ball.speed.x)) and
                        ((math.abs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) and (ball.speed.x < 0))
                then
                    brick[i][j].active = false;
                    ball.speed.x = ball.speed.x * -1;
                end
            end
        end
    end
end

InitWindow(screenWidth, screenHeight, "sample game: arkanoid");

InitGame();

SetTargetFPS(60);
----------------------------------------------------------------------------------------

-- Main game loop
while (not WindowShouldClose())    -- Detect window close button or ESC key
do
		-- Update
		------------------------------------------------------------------------------------
		UpdateGame();
		------------------------------------------------------------------------------------

		-- Draw
		------------------------------------------------------------------------------------
		DrawGame();
		------------------------------------------------------------------------------------
end

UnloadGame();         -- Unload loaded data (textures, sounds, models...)

CloseWindow();        -- Close window and OpenGL context
