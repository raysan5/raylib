/*******************************************************************************************
*
*   raylib example 07a - Initialize 3d mode
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
<<<<<<< HEAD
    int screenWidth = 800;
    int screenHeight = 450;

    Vector3 position = { 0.0, 0.0, 0.0 };
    
    Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 07a - 3d mode");
    //----------------------------------------------------------
    
=======
	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = 800;
	int screenHeight = 450;

	Vector3 position = { 0.0, 0.0, 0.0 };
	
	Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
	
	InitWindow(screenWidth, screenHeight, "raylib example 07a - 3d mode");
	//--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
<<<<<<< HEAD
        // Update
        //-----------------------------------------------------
        // TODO: Update your variables here
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(WHITE);
            
            Begin3dMode(camera);
            
                DrawCube(position, 2, 2, 2, RED);

                DrawGrid(10.0, 1.0);
                
            End3dMode();
            
            DrawText("Welcome to the third dimension!", 10, 40, 20, 1, DARKGRAY);
            
            DrawFps();
        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------
    
=======
		// Update
		//----------------------------------------------------------------------------------
		// TODO: Update your variables here
		//----------------------------------------------------------------------------------
		
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		
			ClearBackground(WHITE);
			
			Begin3dMode(camera);
			
				DrawCube(position, 2, 2, 2, RED);

				DrawGrid(10.0, 1.0);
				
			End3dMode();
			
			DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
			
			DrawFPS(10, 10);
		
        EndDrawing();
		//----------------------------------------------------------------------------------
    }

	// De-Initialization
	//--------------------------------------------------------------------------------------
    CloseWindow();		// Close window and OpenGL context
	//--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    return 0;
}