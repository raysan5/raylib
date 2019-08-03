/***********************************************************************************
*
*   KING GAME JAM - GRAY TEAM
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
************************************************************************************/

#include "raylib.h"
#include "player.h"

#define PLAYER_ANIM_FRAMES     4
#define PLAYER_ANIM_SEQ        6

//----------------------------------------------------------------------------------
// Module Variables Definition
//----------------------------------------------------------------------------------

// Player mouse moving variables
static bool movingAnim;
static int moveDirection;
static int nextMovePoint;

// Mouse pointer variables
static Vector2 pointerPosition;
static bool pointerAnim;
static float pointerAlpha;

static int framesCounter;
static bool outControl = false;

static int animTimer = 0;

static Texture2D texLife;

static void DrawLifes(void);

// player initialitaction definition
void InitPlayer(void)
{
    // NOTE: Some player variables are only initialized once
    player.texture = LoadTexture("resources/textures/skully.png");
    player.position = (Vector2){ 350, 400 };
    player.numLifes = 4;
    
    ResetPlayer();
    
    framesCounter = 0;

    texLife = LoadTexture("resources/textures/skully_icon.png");
}

// player update definition
void UpdatePlayer(void)
{
    if (!outControl)
    {
        if ((IsKeyDown(KEY_LEFT)) || (IsKeyDown(KEY_RIGHT)))
        {
            moveDirection = -1;
            movingAnim = false;
        }
        
        if ((IsKeyDown(KEY_RIGHT)) || (moveDirection == 0))
        {
            player.currentSeq = WALK_RIGHT;
            framesCounter++;
            
            if (framesCounter > 15)
            {
                player.currentFrame++;
                framesCounter = 0;
                
                if (player.currentFrame > PLAYER_ANIM_FRAMES - 1) player.currentFrame = 0;
            }
            
            player.position.x += 4;
        }
        else if ((IsKeyDown(KEY_LEFT)) || (moveDirection == 1))
        {
            player.currentSeq = WALK_LEFT;
            framesCounter++;
            
            if (framesCounter > 15)
            {
                player.currentFrame++;
                framesCounter = 0;
                
                if (player.currentFrame > PLAYER_ANIM_FRAMES - 1) player.currentFrame = 0;
            }
            
            player.position.x -= 4;
        }
        else player.currentFrame = 0;
    }
    else
    {
        framesCounter++;
        animTimer++;
        
        if (framesCounter > 10)
        {
            player.currentFrame++;
            framesCounter = 0;
            
            if (player.currentFrame > PLAYER_ANIM_FRAMES - 1) player.currentFrame = 0;
            
            // We can adjust animation playing time depending on sequence
            switch (player.currentSeq)
            {
                case SCARE_RIGHT:
                {
                    if (animTimer > 180)
                    {
                        animTimer = 0;
                        outControl = false;
                        player.currentSeq = WALK_LEFT;
                    }
                } break;
                case SCARE_LEFT:
                {
                    if (animTimer > 240)
                    {
                        animTimer = 0;
                        outControl = false;
                        player.currentSeq = WALK_RIGHT;
                    }
                } break;
                case SEARCH:
                case FIND_KEY:
                {
                    if (animTimer > 240)
                    {
                        animTimer = 0;
                        outControl = false;
                        player.currentSeq = WALK_RIGHT;
                    }
                } break;
            }
        }
    }
    
    if (player.position.x < 30) player.position.x = 30;
    else if (player.position.x > (GetScreenWidth() - 200)) player.position.x = GetScreenWidth() - 200;
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {      
        pointerPosition = GetMousePosition();
        pointerAnim = true;
        pointerAlpha = 1.0f;
        
        nextMovePoint = (int)pointerPosition.x;
        movingAnim = true;
    }
    
    if (movingAnim)
    {
        if (nextMovePoint > (player.position.x + (player.frameRec.width/2) + 5))  moveDirection = 0;  // Move Left
        else if (nextMovePoint < (player.position.x + (player.frameRec.width/2) - 5)) moveDirection = 1;  // Move Right
        else 
        {
            moveDirection = -1;
            movingAnim = 0;
        }
    }
    
    player.frameRec.x = player.currentFrame*player.texture.width/PLAYER_ANIM_FRAMES;
    player.frameRec.y = (player.currentSeq - 1)*player.texture.height/PLAYER_ANIM_SEQ;
    
    // Update player bounds
    player.bounds = (Rectangle){ player.position.x + 50, player.position.y - 60, 100, 300 };
    
    // Mouse pointer alpha animation
    if (pointerAnim)
    {
        pointerAlpha -= 0.1f;
        
        if (pointerAlpha <= 0.0f) 
        {
            pointerAlpha = 0.0f;
            pointerAnim = false;
        }
    }
}
//
void DrawPlayer(void)
{
    DrawTextureRec(player.texture, player.frameRec, player.position, WHITE);
    
    // Draw mouse pointer on click
    if (pointerAnim) DrawCircleV(pointerPosition, 20, Fade(RED, pointerAlpha));
    
    DrawLifes();
}

void UnloadPlayer(void)
{
    UnloadTexture(player.texture);
    UnloadTexture(texLife);
}

void ResetPlayer(void)
{   
    // Reset player variables
    player.frameRec = (Rectangle){ 0, 0, player.texture.width/PLAYER_ANIM_FRAMES, player.texture.height/PLAYER_ANIM_SEQ };
    player.currentFrame = 0;
    player.currentSeq = WALK_RIGHT;
    
    player.key = false;
    player.dead = false;

    // Reset player position
    if (player.position.x < 400) player.position.x = GetScreenWidth() - 350;
    if (player.position.x > (GetScreenWidth() - 400)) player.position.x = 350;
    
    // Reset moving variables
    movingAnim = false;
    moveDirection = -1;
    nextMovePoint = 0;
    framesCounter = 0;
    outControl = false;
    animTimer = 0;
    
    // Reset pointer   
    pointerAlpha = 0.0f;
    pointerAnim = false;
}

void ScarePlayer(void)
{
    player.currentFrame = 0;

    if (moveDirection == 0) player.currentSeq = SCARE_RIGHT;
    else if (moveDirection == 1) player.currentSeq = SCARE_LEFT;
    else player.currentSeq = SCARE_RIGHT;
    
    player.numLifes--;
    
    if (player.numLifes <= 0) player.dead = true;
    
    outControl = true;
}

void SearchKeyPlayer(void)
{
    moveDirection = -1;
    movingAnim = 0;
            
    player.currentFrame = 0;
    player.currentSeq = SEARCH;
    
    outControl = true;
}

void FindKeyPlayer(void)
{
    player.currentFrame = 0;
    player.currentSeq = FIND_KEY;
    player.key = true;
    
    outControl = true;
}

static void DrawLifes(void)
{
    if (player.numLifes != 0)
    {
        Vector2 position = { 20, GetScreenHeight() - texLife.height - 20 };
        
        for(int i = 0; i < player.numLifes; i++)
        {
            DrawTexture(texLife, position.x + i*texLife.width, position.y, Fade(RAYWHITE, 0.7f));
        }
    }
}