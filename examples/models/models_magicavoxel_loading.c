/*******************************************************************************************
*
*   raylib [models] example - magicavoxel loader and viewer
*
*   This example has been created using raylib 3.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Johann Nadalutti 
*
*   Copyright (c) 2021 Johann Nadalutti 
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <string.h>


// VOX Files to load and view

#define NUM_VOX_FILES  3

const char* szVoxFiles[] = {
	"resources/vox/chr_knight.vox",
	"resources/vox/chr_sword.vox",
	"resources/vox/monu9.vox"
};


int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [models] example - magicavoxel loading");

	// Load MagicaVoxel files
	Model models[NUM_VOX_FILES] = { 0 };

	for (int i = 0; i < NUM_VOX_FILES; i++)
	{
		// Load MagicaVoxel File and build model
		double t0, t1;
		t0 = GetTime() * 1000.0;

		models[i] = LoadModel(szVoxFiles[i]);

		t1 = GetTime() * 1000.0;
		TraceLog(LOG_INFO, TextFormat("Vox <%s> loaded in %f ms", GetFileName(szVoxFiles[i]), t1 - t0));

		//Compute model's center matrix 
		BoundingBox  bb = GetModelBoundingBox(models[i]);
		Vector3 center;
		center.x = bb.min.x  + (((bb.max.x - bb.min.x) / 2));
		center.z = bb.min.z  + (((bb.max.z - bb.min.z) / 2));

		Matrix matP = MatrixTranslate(-center.x, 0, -center.z);
		models[i].transform = matP;


	}


	// Define the camera to look into our 3d world
	Camera camera = { { 0.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

	// Model drawing position
	Vector3 position = { 0.0f, 0.0f, 0.0f };

	int currentModel = 0;



	SetCameraMode(camera, CAMERA_ORBITAL);  // Set a orbital camera mode

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	//--------------------------------------------------------------------------------------
	// Main game loop
	//--------------------------------------------------------------------------------------
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		//--------------------------------------------------------------------------------------
		// Update
		//----------------------------------------------------------------------------------
		UpdateCamera(&camera);      // Update internal camera and our camera

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			currentModel = (currentModel + 1) % NUM_VOX_FILES; // Cycle between models
		}

		if (IsKeyPressed(KEY_RIGHT))
		{
			currentModel++;
			if (currentModel >= NUM_VOX_FILES) currentModel = 0;
		}
		else if (IsKeyPressed(KEY_LEFT))
		{
			currentModel--;
			if (currentModel < 0) currentModel = NUM_VOX_FILES - 1;
		}

		//----------------------------------------------------------------------------------
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		//Display model
		BeginMode3D(camera);

		Vector3 rotAxis = { 1,0,0 };
		Vector3 scale = { 1,1,1 };


		DrawModelEx(models[currentModel], position, rotAxis, 0, scale, WHITE);
		//DrawModelWiresEx(models[currentModel], position, rotAxis, -90.0f, scale, BLACK);

		DrawGrid(10, 1.0);

		EndMode3D();

		//Display debug infos
		DrawRectangle(30, 400, 310, 30, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(30, 400, 310, 30, Fade(DARKBLUE, 0.5f));
		DrawText("MOUSE LEFT BUTTON to CYCLE VOX MODELS", 40, 410, 10, BLUE);

		DrawText(GetFileName(szVoxFiles[currentModel]), 100, 10, 20, DARKBLUE);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------

	// Unload models data (GPU VRAM)
	for (int i = 0; i < NUM_VOX_FILES; i++) UnloadModel(models[i]);

	CloseWindow();          // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}


