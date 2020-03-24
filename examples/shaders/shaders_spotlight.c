/*******************************************************************************************
*
*   raylib [shaders] example - Simple shader mask
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Chris Camacho (@codifies -  http://bedroomcoders.co.uk/) 
*   and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Chris Camacho (@codifies) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   The shader makes alpha holes in the forground to give the apearance of a top
*   down look at a spotlight casting a pool of light...
* 
*   The right hand side of the screen there is just enough light to see whats
*   going on without the spot light, great for a stealth type game where you
*   have to avoid the spotlights.
* 
*   The left hand side of the screen is in pitch dark except for where the spotlights are.
* 
*   Although this example doesn't scale like the letterbox example, you could integrate
*   the two techniques, but by scaling the actual colour of the render texture rather
*   than using alpha as a mask.
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stddef.h>
#include <stdint.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define MAXSPOT           2
#define numStars        400

// Stars in the star field have a position and velocity
typedef struct Star {
    Vector2 pos;
    Vector2 vel;
} Star;

void UpdateStar(Star *s);
void ResetStar(Star *s);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - shader spotlight");

    Texture texRay = LoadTexture("resources/raysan.png");
    
    Star stars[numStars] = { 0 };

    for (int n = 0; n < numStars; n++) ResetStar(&stars[n]);

    // Progress all the stars on, so they don't all start in the centre
    for (int m = 0; m < screenWidth/2.0; m++) 
    {
        for (int n = 0; n < numStars; n++) UpdateStar(&stars[n]);
    }

    int frameCounter = 0;

    unsigned int spotLoc[MAXSPOT];  // shader locations    
 
    Vector2 spotPos[MAXSPOT];       // position and velocity
    Vector2 spotVel[MAXSPOT];
       
    // Use default vert shader
    Shader spotShader = LoadShader(0, FormatText("resources/shaders/glsl%i/spotlight.fs", GLSL_VERSION));
    
	// Get the locations of spots in the shader
    char spotName[32] = "spots[x]\0";
    for (int i = 0; i < MAXSPOT; i++) 
    {
		spotName[6] = '0' + i;
		spotLoc[i] = GetShaderLocation(spotShader, spotName);
	}
	
	// tell the shader how wide the screen is so we can have
	// a pitch black half and a dimly lit half.
	{
		unsigned int wLoc = GetShaderLocation(spotShader, "screenWidth");
		float sw = (float)GetScreenWidth();
		SetShaderValue(spotShader, wLoc, &sw, UNIFORM_FLOAT);
	}

    // randomise the locations and velocities of the spotlights
    for (int i = 0; i < MAXSPOT; i++)
    {
		spotPos[i].x = GetRandomValue(64, screenWidth - 64);
		spotPos[i].y = GetRandomValue(64, screenHeight - 64);
		spotVel[i] = (Vector2){ 0, 0 };
        
		while ((fabs(spotVel[i].x) + fabs(spotVel[i].y)) < 2)
        {
			spotVel[i].x = GetRandomValue(-40, 40)/10.0;
			spotVel[i].y = GetRandomValue(-40, 40)/10.0;
		}
	}

    SetTargetFPS(60);               // Set  to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCounter++;

		// Move the stars, resetting them if the go offscreen
        for (int n = 0; n < numStars; n++) UpdateStar(&stars[n]);

		// Update the spots, send them to the shader
		for (int i = 0; i < MAXSPOT; i++)
        {
			spotPos[i].x += spotVel[i].x;					
			spotPos[i].y += spotVel[i].y;
			
			if (spotPos[i].x < 64) spotVel[i].x = -spotVel[i].x;					
			if (spotPos[i].x > screenWidth - 64) spotVel[i].x = -spotVel[i].x;					
			if (spotPos[i].y < 64) spotVel[i].y = -spotVel[i].y;					
			if (spotPos[i].y > screenHeight - 64) spotVel[i].y = -spotVel[i].y;
			
			SetShaderValue(spotShader, spotLoc[i], &spotPos[i].x, UNIFORM_VEC2);				
		}
			
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKBLUE);

			// Draw stars and bobs
            for (int n = 0; n < numStars; n++)
            {
				// Single pixel is just too small these days!
                DrawRectangle(stars[n].pos.x, stars[n].pos.y, 2, 2, WHITE);
            }

            for (int i = 0; i < 16; i++)
            {
                DrawTexture(texRay,
                    (screenWidth/2.0) + cos((frameCounter + i*8)/51.45f)*(screenWidth/2.2) - 32,
                    (screenHeight/2.0) + sin((frameCounter + i*8)/17.87f)*(screenHeight/4.2),
                    WHITE);
            }

			// Draw spot lights
			BeginShaderMode(spotShader);
				// instead of a blank rectangle you could render here
				// a render texture of the full screen used to do screen
				// scaling (slight adjustment to shader would be required
				// to actually pay attention to the colour!)
				DrawRectangle(0,0,screenWidth,screenHeight,WHITE);
			EndShaderMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texRay);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


void ResetStar(Star *s)
{
    s->pos = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    
    do
    {
        s->vel.x = (float)GetRandomValue(-1000, 1000)/100.0f;
        s->vel.y = (float)GetRandomValue(-1000, 1000)/100.0f;
        
    } while (!(fabs(s->vel.x) + fabs(s->vel.y) > 1));
    
    s->pos = Vector2Add(s->pos, Vector2MultiplyV(s->vel, (Vector2){ 8, 8 }));
}

void UpdateStar(Star *s)
{
    s->pos = Vector2Add(s->pos, s->vel);
    
    if (s->pos.x < 0 || s->pos.x > GetScreenWidth() ||
        s->pos.y < 0 || s->pos.y > GetScreenHeight())
    {
        ResetStar(s);
    }
}


