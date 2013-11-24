/*******************************************************************************************
*
*	raylib example 07b - Draw some basic 3d shapes (cube, sphere, cylinder...)
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

	Vector3 position = { 0.0, 0.0, 0.0 };
	
    // Define the camera to look into our 3d world
	Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
	
	// Initialization
	//---------------------------------------------------------
	InitWindow(screenWidth, screenHeight, "raylib example 07b - 3d shapes");
	
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //----------------------------------------------------------
	
    // Main game loop
    while (!WindowShouldClose())	// Detect window close button or ESC key
    {
		// Update
		//-----------------------------------------------------
		// TODO: Update your variables here
		//-----------------------------------------------------
		
		// Draw
		//-----------------------------------------------------
		BeginDrawing();
		
			ClearBackground(RAYWHITE);
			
			Begin3dMode(camera);
			
				DrawCube(position, 2, 2, 2, RED);           // Draw a cube
                DrawCubeWires(position, 2, 2, 2, MAROON);   // Draw a wired-cube
                
                // TODO: Draw some basic 3d shapes
/*         
void DrawCube(Vector3 position, float width, float height, float lenght, Color color);
void DrawCubeV(Vector3 position, Vector3 size, Color color);
void DrawCubeWires(Vector3 position, float width, float height, float lenght, Color color);
void DrawSphere(Vector3 centerPos, float radius, Color color);
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);
void DrawSphereWires(Vector3 centerPos, float radius, Color color);
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color);
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color);
*/
				DrawGrid(10.0, 1.0);        // Draw a grid
				
			End3dMode();
			
			DrawFps(10, 10);
		
        EndDrawing();
		//-----------------------------------------------------
    }

	// De-Initialization
	//---------------------------------------------------------
    CloseWindow();		// Close window and OpenGL context
	//----------------------------------------------------------
	
    return 0;
}