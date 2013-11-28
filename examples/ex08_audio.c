/*******************************************************************************************
*
*   raylib example 08 - Audio loading and playing
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
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 08 - audio loading and playing");
    
    InitAudioDevice();      // Initialize audio device
    
    Sound fx = LoadSound("resources/coin.wav");         // Load WAV audio file
    
    bool currentKeyState = false;
    bool previousKeyState = currentKeyState;
    //----------------------------------------------------------
    
=======
	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = 800;
	int screenHeight = 450;
	
	InitWindow(screenWidth, screenHeight, "raylib example 08 - audio loading and playing");
	
    InitAudioDevice();      // Initialize audio device
	
	Sound fx = LoadSound("resources/coin.wav");         // Load WAV audio file
	//--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
<<<<<<< HEAD
        // Update
        //-----------------------------------------------------
        currentKeyState = IsKeyPressed(KEY_SPACE);      // Check if Space have been pressed
    
        if (currentKeyState != previousKeyState)
        {
            if (currentKeyState) PlaySound(fx);         // Play the sound!
            
            previousKeyState = currentKeyState;
        }
        
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("Press SPACE to PLAY the SOUND!", 240, 200, 20, 1, LIGHTGRAY);
        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    UnloadSound(fx);        // Unload sound data
    
    CloseAudioDevice();     // Close audio device
    
    CloseWindow();            // Close window and OpenGL context
    //----------------------------------------------------------
    
=======
		// Update
		//----------------------------------------------------------------------------------
		if (IsKeyPressed(KEY_SPACE)) PlaySound(fx);     // Play the sound!	
		//----------------------------------------------------------------------------------
		
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		
			ClearBackground(RAYWHITE);
			
			DrawText("Press SPACE to PLAY the SOUND!", 240, 200, 20, LIGHTGRAY);
		
        EndDrawing();
		//----------------------------------------------------------------------------------
    }

	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadSound(fx);        // Unload sound data
    
	CloseAudioDevice();     // Close audio device
	
    CloseWindow();          // Close window and OpenGL context
	//--------------------------------------------------------------------------------------
	
>>>>>>> Added some functions and examples update
    return 0;
}