/*******************************************************************************************
*
*   raylib [core] example - 2d camera mouse zoom
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2022 Jeffery Myers
*
********************************************************************************************/


#include "raylib.h"
#include "rlgl.h"
#include "raymath.h";


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main ()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera mouse zoom");

	Camera2D camera = { 0 };
	camera.zoom = 1;

	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())
	{
		// translate based on right click
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			Vector2 delta = GetMouseDelta();
			delta = Vector2Scale(delta, -1.0f / camera.zoom);

			camera.target = Vector2Add(camera.target, delta);
		}

		// zoom based on wheel
		float wheel = GetMouseWheelMove();
		if (wheel != 0)
		{
			// get the world point that is under the mouse
			Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
			
			// set the offset to where the mouse is
			camera.offset = GetMousePosition();

			// set the target to match, so that the camera maps the world space point under the cursor to the screen space point under the cursor at any zoom
			camera.target = mouseWorldPos;

			// zoom
			const float zoomIncrement = 0.125f;

			camera.zoom += wheel * zoomIncrement;
			if (camera.zoom < zoomIncrement)
				camera.zoom = zoomIncrement;
		}

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(camera);

		// draw the 3d grid, rotated 90 degrees and centered around 0,0 just so we have something in the XY plane
        rlPushMatrix();
        rlTranslatef(0, 25 * 50, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(100, 50);
        rlPopMatrix();

		// draw a thing
		DrawCircle(100, 100, 50, YELLOW);
		EndMode2D();

		DrawText("Right drag to move, mouse wheel to zoom", 2, 2, 20, WHITE);
		
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
    //--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------
	return 0;
}