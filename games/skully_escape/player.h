#ifndef PLAYER_H
#define PLAYER_H

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { NONE, WALK_RIGHT, WALK_LEFT, SCARE_RIGHT, SCARE_LEFT, SEARCH, FIND_KEY } PlayerSequence;

typedef struct Player {
    Vector2 position;
    Rectangle bounds;
    Texture2D texture;
    Color color;
    
    // Animation variables
    Rectangle frameRec;
    int currentFrame;
    int currentSeq;
    
    bool key;
    int numLifes;
    bool dead;
} Player;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
Player player;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Logo Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitPlayer(void);
void UpdatePlayer(void);
void DrawPlayer(void);
void UnloadPlayer(void);
void ResetPlayer(void);

void ScarePlayer(void);
void SearchKeyPlayer(void);
void FindKeyPlayer(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H