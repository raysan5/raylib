/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Screens Functions Declarations (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef MONSTER_H
#define MONSTER_H

#define MONSTER_ANIM_FRAMES     7
#define MONSTER_ANIM_SEQ        2

//----------------------------------------------------------------------------------
// Types and Structures Definition
//---------------------------------------------------------------------------------
typedef struct Monster {
    Vector2 position;
    Texture2D texture;
    Rectangle bounds;
    Rectangle frameRec;
    Color color;
    int framesCounter;
    int currentFrame;
    int currentSeq;
    int numFrames;
    bool active;
    bool selected;
    bool spooky;
} Monster;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------



#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Monster Functions Declaration
//----------------------------------------------------------------------------------

void UpdateMonster(Monster *monster);
void DrawMonster(Monster monster, int scroll);
void UnloadMonster(Monster monster);


#ifdef __cplusplus
}
#endif

#endif // SCREENS_H