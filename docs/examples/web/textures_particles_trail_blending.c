/*******************************************************************************************
*
*   raylib [core] example - Basic window (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define MAX_PARTICLES 200

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

// Particle structure with basic data
typedef struct {
    Vector2 position;
    Color color;
    float alpha;
    float size;
    float rotation;
    bool active;        // NOTE: Use it to activate/deactive particle
} Particle;

Particle mouseTail[MAX_PARTICLES]; 

float gravity = 3.0f;

Texture2D smoke;

int blending = BLEND_ALPHA;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    
     // Initialize particles
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        mouseTail[i].position = (Vector2){ 0, 0 };
        mouseTail[i].color = (Color){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        mouseTail[i].alpha = 1.0f;
        mouseTail[i].size = (float)GetRandomValue(1, 30)/20.0f;
        mouseTail[i].rotation = GetRandomValue(0, 360);
        mouseTail[i].active = false;
    }
    
    smoke = LoadTexture("resources/smoke.png");
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
	UnloadTexture(smoke);	// Texture unloading
	 
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // Activate one particle every frame and Update active particles
    // NOTE: Particles initial position should be mouse position when activated
    // NOTE: Particles fall down with gravity and rotation... and disappear after 2 seconds (alpha = 0)
    // NOTE: When a particle disappears, active = false and it can be reused.
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!mouseTail[i].active)
        {
            mouseTail[i].active = true;
            mouseTail[i].alpha = 1.0f;
            mouseTail[i].position = GetMousePosition();
            i = MAX_PARTICLES;
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (mouseTail[i].active)
        {
            mouseTail[i].position.y += gravity;
            mouseTail[i].alpha -= 0.01f;
            
            if (mouseTail[i].alpha <= 0.0f) mouseTail[i].active = false;
            
            mouseTail[i].rotation += 5.0f;
        }
    }
    
    if (IsKeyPressed(KEY_SPACE))
    {
        if (blending == BLEND_ALPHA) blending = BLEND_ADDITIVE;
        else blending = BLEND_ALPHA;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(DARKGRAY);
            
        BeginBlendMode(blending);

	        // Draw active particles
	        for (int i = 0; i < MAX_PARTICLES; i++)
	        {
	            if (mouseTail[i].active) DrawTexturePro(smoke, (Rectangle){ 0, 0, smoke.width, smoke.height }, 
	                                                   (Rectangle){ mouseTail[i].position.x, mouseTail[i].position.y, smoke.width*mouseTail[i].size, smoke.height*mouseTail[i].size },
	                                                   (Vector2){ smoke.width*mouseTail[i].size/2, smoke.height*mouseTail[i].size/2 }, mouseTail[i].rotation,
	                                                   Fade(mouseTail[i].color, mouseTail[i].alpha));
	        }
        
        EndBlendMode();
		
        DrawText("PRESS SPACE to CHANGE BLENDING MODE", 180, 20, 20, BLACK);
        
        if (blending == BLEND_ALPHA) DrawText("ALPHA BLENDING", 290, screenHeight - 40, 20, BLACK);
        else DrawText("ADDITIVE BLENDING", 280, screenHeight - 40, 20, RAYWHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------
}