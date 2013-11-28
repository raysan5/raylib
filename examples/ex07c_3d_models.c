/*******************************************************************************************
*
*   raylib example 07c - Load and draw a 3d model (OBJ)
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
=======
	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = 800;
	int screenHeight = 450;
>>>>>>> Added some functions and examples update

    Vector3 position = { 0.0, 0.0, 0.0 };
    
    // Define the camera to look into our 3d world
<<<<<<< HEAD
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 07c - 3d models");
    
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    
    Texture2D texture = LoadTexture("resources/catwhite.png");
    Model cat = LoadModel("resources/cat.obj");
    //----------------------------------------------------------
    
=======
	Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
	
	InitWindow(screenWidth, screenHeight, "raylib example 07c - 3d models");
	
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    
    Texture2D texture = LoadTexture("resources/catwhite.png");
	Model cat = LoadModel("resources/cat.obj");
    //--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
<<<<<<< HEAD
        // Update
        //-----------------------------------------------------
        if (IsKeyPressed(KEY_LEFT)) position.x -= 0.2;
        if (IsKeyPressed(KEY_RIGHT)) position.x += 0.2;
        if (IsKeyPressed(KEY_UP)) position.z -= 0.2;
        if (IsKeyPressed(KEY_DOWN)) position.z += 0.2;
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
            
                DrawModelEx(cat, texture, position, 0.1f, WHITE);   // Draw 3d model with texture
=======
		// Update
		//----------------------------------------------------------------------------------
		if (IsKeyPressed(KEY_LEFT)) position.x -= 0.2;
		if (IsKeyPressed(KEY_RIGHT)) position.x += 0.2;
        if (IsKeyPressed(KEY_UP)) position.z -= 0.2;
        if (IsKeyPressed(KEY_DOWN)) position.z += 0.2;
		//----------------------------------------------------------------------------------
		
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		
			ClearBackground(RAYWHITE);
			
			Begin3dMode(camera);
			
				DrawModelEx(cat, texture, position, 0.1f, WHITE);   // Draw 3d model with texture
>>>>>>> Added some functions and examples update

                DrawGrid(10.0, 1.0);        // Draw a grid
                
                DrawGizmo(position, false); 
<<<<<<< HEAD
                
            End3dMode();
            
            DrawFps(10, 10);
        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
=======
				
			End3dMode();
			
			DrawFPS(10, 10);
		
        EndDrawing();
		//----------------------------------------------------------------------------------
    }

	// De-Initialization
	//--------------------------------------------------------------------------------------
>>>>>>> Added some functions and examples update
    UnloadTexture(texture);     // Unload texture
    UnloadModel(cat);           // Unload model
    
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------
    
<<<<<<< HEAD
=======
    CloseWindow();		        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    return 0;
}