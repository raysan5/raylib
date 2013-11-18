/*******************************************************************************************
*
*	raylib example 03a - Keyboard input 
*
*	This example has been created using raylib 1.0 (www.raylib.com)
*	raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*	Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
	int screenWidth = 800;
	int screenHeight = 450;
	
	Vector2 ballPosition = { screenWidth/2, screenHeight/2 };
	
	// Initialization
	//---------------------------------------------------------
	InitWindow(screenWidth, screenHeight, "raylib example 05 - keyboard input");	// Window and context initialization
	//----------------------------------------------------------
	
    // Main game loop
    while (!WindowShouldClose())	// Detect window close button or ESC key
    {
		// Update
		//-----------------------------------------------------
		if (IsKeyPressed(KEY_RIGHT)) ballPosition.x += 0.8;
		if (IsKeyPressed(KEY_LEFT)) ballPosition.x -= 0.8;
		if (IsKeyPressed(KEY_UP)) ballPosition.y -= 0.8;
		if (IsKeyPressed(KEY_DOWN)) ballPosition.y += 0.8;
		//-----------------------------------------------------
		
		// Draw
		//-----------------------------------------------------
		BeginDrawing();
		
			ClearBackground(RAYWHITE);
			
			DrawText("move the ball with arrow keys", 10, 10, 20, 1, DARKGRAY);
			
			DrawCircleV(ballPosition, 50, MAROON);
		
        EndDrawing();
		//-----------------------------------------------------
    }

	// De-Initialization
	//---------------------------------------------------------
    CloseWindow();		// Close window and OpenGL context
	//----------------------------------------------------------
	
    return 0;
}