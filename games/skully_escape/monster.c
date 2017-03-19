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
#include "monster.h"

void UpdateMonster(Monster *monster)
{
    if (!monster->active)
    {
        if (CheckCollisionPointRec(GetMousePosition(), monster->bounds)) monster->selected = true;
        else monster->selected = false;
    }
    else if (monster->spooky)
    {
        monster->framesCounter++;
        monster->currentSeq = 0;
        
        if (monster->framesCounter > 7)
        {
            monster->currentFrame++;
            monster->framesCounter = 0;
            
            if (monster->currentFrame > monster->numFrames - 1) monster->currentFrame = 1;
        }
    }
    
    monster->frameRec.x = monster->currentFrame*monster->texture.width/monster->numFrames;
    monster->frameRec.y = monster->currentSeq*monster->texture.height;
}

void DrawMonster(Monster monster, int scroll)
{
    Vector2 scrollPos = { monster.position.x - scroll, monster.position.y };
    
    if (monster.selected) DrawTextureRec(monster.texture, monster.frameRec, scrollPos, RED);
    else DrawTextureRec(monster.texture, monster.frameRec, scrollPos, WHITE);
}

void UnloadMonster(Monster monster)
{
    UnloadTexture(monster.texture);
}