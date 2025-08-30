/*******************************************************************************************
*
*   raylib [core] example - 3d camera shake
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Guilherme Silva (@grssilva)
*
********************************************************************************************/

#include "raylib.h"
#include "rcamera.h"

#define MAX_YAW   10.0f	// yaw is rotation around the y axis
#define MAX_PITCH 5.0f  // pitch is rotation around the x axis
#define MAX_ROLL  20.0f // roll is rotation around the z axis
#define MAX_SCALE 1.5f  // max scale is zoom in/out

static Camera camera = { 0 };
static float cameraTrauma = 0.0f;

// Returns a random float between -1.0 and 1.0
inline static float RandomFloat() {
	return (float) GetRandomValue(-1000, 1000) / 1000.0f;
}

void UpdateCameraTrauma(void) {
	// Update camera shake
	if (cameraTrauma > 1.0f)		cameraTrauma = 1.0f;
	else if (cameraTrauma < 0.01f)	cameraTrauma = 0.0f;
	else if (cameraTrauma == 0.0f)	return;

	// Calculate the shake offsets
	float yaw = MAX_YAW * RandomFloat() * cameraTrauma;
	float pitch = MAX_PITCH * RandomFloat() * cameraTrauma;
	float roll = MAX_ROLL * RandomFloat() * cameraTrauma;
	float scale = 1.0f + (MAX_SCALE - 1.0f) * RandomFloat() * cameraTrauma;

	// Camera rotation, based on UpdateCameraPro implementation
	const bool lockView = true;
	const bool rotateAroundTarget = false;
	const bool rotateUp = false;

	CameraPitch(&camera, -pitch * DEG2RAD, lockView, rotateAroundTarget, rotateUp);
	CameraYaw(&camera, -yaw * DEG2RAD, rotateAroundTarget);
	CameraRoll(&camera, roll * DEG2RAD);

	camera.fovy *= scale;

	// Degrade the camera trauma to create the smooth shake effect 
	cameraTrauma *= 0.95f;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	// Seed the random number generator
	SetRandomSeed((unsigned int) time(NULL));

	InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera shake");

	// Define the camera projection
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

	Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update

		// Set the camera position and target
		// Note: We need to set it every frame because the camera shake modifies it
		camera.position = (Vector3) { 10.0f, 10.0f, 10.0f }; // Camera position
		camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };      // Camera looking at point
		camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
		camera.fovy = 45.0f;								 // Camera field-of-view Y

		// Set the camera shake to 1.0f when pressing SPACE
		if (IsKeyPressed(KEY_SPACE)) cameraTrauma = 1.0f;

		UpdateCameraTrauma();

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, BLUE);
		DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

		DrawGrid(10, 1.0f);

		EndMode3D();

		DrawText("Press SPACE to shake the camera!", 10, 40, 20, DARKGRAY);
		DrawText(TextFormat("Camera shake: %.0f", cameraTrauma * 100), 10, 60, 20, DARKGRAY);
		DrawFPS(10, 10);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}