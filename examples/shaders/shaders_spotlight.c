/*
 * Copyright (c) 2019 Chris Camacho (codifies -  http://bedroomcoders.co.uk/)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
 
/* The shader makes alpha holes in the forground to give the apearance of a top
 * down look at a spotlight casting a pool of light...
 * 
 * The right hand side of the screen there is just enough light to see whats
 * going on without the spot light, great for a stealth type game where you
 * have to avoid the spotlights.
 * 
 * The left hand side of the screen is in pitch dark except for where the spotlights
 * are.
 * 
 * Although this example doesn't scale like the letterbox example, you could integrate
 * the two techniques, but by scaling the actual colour of the render texture rather
 * than using alpha as a mask.
 */

#include <stddef.h>
#include <stdint.h>

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

// single digit only!
#define MAXSPOT 4

#define numStars 400

#define screenWidth 1280
#define screenHeight 720

// the stars in the star field have a position and velocity
typedef struct star {
    Vector2 pos;
    Vector2 vel;
} star;


void updateStar(star *s);
void resetStar(star *s);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    star stars[numStars];

    for (int n=0; n < numStars; n++) {
        resetStar(&stars[n]);
    }

    // progress all the stars on, so they don't all start in the centre
    for (int m=0; m < screenWidth / 2.0; m++) {
        for (int n=0; n<numStars; n++) {
            updateStar(&stars[n]);
        }
    }

    //SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(screenWidth, screenHeight, "raylib - test");

    Texture rayTex = LoadTexture("resources/raysan.png"); 

    // frame counter
    int frame = 0;

    unsigned int spotLoc[MAXSPOT]; // shader locations    
 
    Vector2 spotPos[MAXSPOT];  // position and velocity
    Vector2 spotVel[MAXSPOT];
       
    // use default vert shader
    Shader spotShader = LoadShader(0, FormatText("resources/shaders/glsl%i/spotlight.fs", GLSL_VERSION));
    
	// get the locations of spots in the shader
    char spotName[32] = "spots[x]\0";
    for (int i = 0; i<MAXSPOT; i++) {
		spotName[6] = '0' + i;
		spotLoc[i] = GetShaderLocation(spotShader, spotName);
	}
	
	// tell the shader how wide the screen is so we can have
	// a pitch black half and a dimly lit half.
	{
		unsigned int wLoc = GetShaderLocation(spotShader, "screenWidth");
		float sw = screenWidth;
		SetShaderValue(spotShader, wLoc, &sw, UNIFORM_FLOAT);
	}

    // randomise the locations and velocities of the spotlights
    for (int i=0; i<MAXSPOT; i++) {
		spotPos[i].x = GetRandomValue(128, screenWidth-128);
		spotPos[i].y = GetRandomValue(128, screenHeight-128);
		spotVel[i] = (Vector2){0, 0};
		while (fabs(spotVel[i].x)+fabs(spotVel[i].y)<2) {
			spotVel[i].x = GetRandomValue(-40,40)/10.0;
			spotVel[i].y = GetRandomValue(-40,40)/10.0;
		}
	}
  

    SetTargetFPS(60);               // Set  to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        frame ++;

		// move the stars, resetting them if the go offscreen
        for (int n=0; n<numStars; n++) {
            updateStar(&stars[n]);
        }

		// update the spots, send them to the shader
		for (int i=0; i<MAXSPOT; i++) {

			spotPos[i].x += spotVel[i].x;					
			spotPos[i].y += spotVel[i].y;
			
			if (spotPos[i].x < 128) spotVel[i].x = -spotVel[i].x;					
			if (spotPos[i].x > screenWidth-128) spotVel[i].x = -spotVel[i].x;					
			if (spotPos[i].y < 128) spotVel[i].y = -spotVel[i].y;					
			if (spotPos[i].y > screenHeight-128) spotVel[i].y = -spotVel[i].y;
			
			SetShaderValue(spotShader, spotLoc[i], &spotPos[i].x, UNIFORM_VEC2);				
		}
			
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color){0,32,128,255});

			// stars and bobs
            for (int n=0; n<numStars; n++) {
				// single pixel is just too small these days!
                DrawRectangle(stars[n].pos.x, stars[n].pos.y, 2, 2, WHITE);
            }

            for (int i=0; i<16; i++) {
                DrawTexture(rayTex,
                    (screenWidth/2.0)+cos((frame+i*8)/51.45)*(screenWidth/2.2)-32,
                    (screenHeight/2.0)+sin((frame+i*8)/17.87)*(screenHeight/4.2),
                    WHITE);
            }

			// spot lights
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

    UnloadTexture(rayTex);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


void resetStar(star *s)
{
    (*s).pos = (Vector2){screenWidth/2.0, screenHeight/2.0 };
    do {
        (*s).vel.x = (float)GetRandomValue(-1000,1000) / 100.0;
        (*s).vel.y = (float)GetRandomValue(-1000,1000) / 100.0;
    } while (!(fabs((*s).vel.x)+fabs((*s).vel.y)>1));
    (*s).pos = Vector2Add( (*s).pos,
                    Vector2MultiplyV((*s).vel,(Vector2){8,8} ));
}

void updateStar(star *s)
{
    (*s).pos = Vector2Add((*s).pos, (*s).vel);
    if ((*s).pos.x < 0 || (*s).pos.x > screenWidth ||
        (*s).pos.y < 0 || (*s).pos.y > screenHeight) {
            resetStar(s);
    }
}


