/*******************************************************************************************
*
*   raylib [models] example - loading vox
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 4.0, last time updated with raylib 4.0
*
*   Example contributed by Johann Nadalutti (@procfxgen) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2025 Johann Nadalutti (@procfxgen) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"        // Required for: MatrixTranslate()

#define MAX_VOX_FILES  4

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	const char *voxFileNames[] = {
		"resources/models/vox/chr_knight.vox",
		"resources/models/vox/chr_sword.vox",
		"resources/models/vox/monu9.vox",
		"resources/models/vox/fez.vox"
	};

	InitWindow(screenWidth, screenHeight, "raylib [models] example - loading vox");

	// Define the camera to look into our 3d world
	Camera camera = { 0 };
	camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

	// Load MagicaVoxel files
	Model models[MAX_VOX_FILES] = { 0 };

	for (int i = 0; i < MAX_VOX_FILES; i++)
	{
		// Load VOX file and measure time
		double t0 = GetTime()*1000.0;
		models[i] = LoadModel(voxFileNames[i]);
		double t1 = GetTime()*1000.0;

		TraceLog(LOG_WARNING, TextFormat("[%s] File loaded in %.3f ms", voxFileNames[i], t1 - t0));

		// Compute model translation matrix to center model on draw position (0, 0 , 0)
		BoundingBox bb = GetModelBoundingBox(models[i]);
		Vector3 center = { 0 };
		center.x = bb.min.x + (((bb.max.x - bb.min.x)/2));
		center.z = bb.min.z + (((bb.max.z - bb.min.z)/2));

		Matrix matTranslate = MatrixTranslate(-center.x, 0, -center.z);
		models[i].transform = matTranslate;
	}

	int currentModel = 0;

	// Load voxel shader
	Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/voxel_lighting.vs", GLSL_VERSION),
		TextFormat("resources/shaders/glsl%i/voxel_lighting.fs", GLSL_VERSION));

	// Get some required shader locations
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	// NOTE: "matModel" location name is automatically assigned on shader loading,
	// no need to get the location again if using that uniform name
	//shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");

	// Ambient light level (some basic lighting)
	int ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, (float[4]) { 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

	// Assign out lighting shader to model
	for (int i = 0; i < MAX_VOX_FILES; i++)
	{
		Model m = models[i];
		for (int j = 0; j < m.materialCount; j++)
		{
			m.materials[j].shader = shader;
		}
	}

	// Create lights
	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_POINT, (Vector3) { -20, 20, -20 }, Vector3Zero(), GRAY, shader);
	lights[1] = CreateLight(LIGHT_POINT, (Vector3) { 20, -20, 20 }, Vector3Zero(), GRAY, shader);
	lights[2] = CreateLight(LIGHT_POINT, (Vector3) { -20, 20, 20 }, Vector3Zero(), GRAY, shader);
	lights[3] = CreateLight(LIGHT_POINT, (Vector3) { 20, -20, -20 }, Vector3Zero(), GRAY, shader);


	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	//--------------------------------------------------------------------------------------
	Vector3 modelpos = { 0 };
	Vector3 camerarot = { 0 };

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------
		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
		{
			const Vector2 mouseDelta = GetMouseDelta();
			camerarot.x = mouseDelta.x*0.05f;
			camerarot.y = mouseDelta.y*0.05f;
		}
		else
		{
			camerarot.x = 0;
			camerarot.y = 0;
		}

		UpdateCameraPro(&camera,
			(Vector3) {
			(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))*0.1f -      // Move forward-backward
				(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))*0.1f,
				(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))*0.1f -   // Move right-left
				(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))*0.1f,
				0.0f                                                // Move up-down
		},
			camerarot,
			GetMouseWheelMove()*-2.0f);                              // Move to target (zoom)

		// Cycle between models on mouse click
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) currentModel = (currentModel + 1) % MAX_VOX_FILES;

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		// Update light values (actually, only enable/disable them)
		for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(shader, lights[i]);

		//----------------------------------------------------------------------------------
		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		// Draw 3D model
		BeginMode3D(camera);

		DrawModel(models[currentModel], modelpos, 1.0f, WHITE);
		DrawGrid(10, 1.0);

		// Draw spheres to show where the lights are
		for (int i = 0; i < MAX_LIGHTS; i++)
		{
			if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
			else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		}

		EndMode3D();

		// Display info
		DrawRectangle(10, 400, 340, 60, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(10, 400, 340, 60, Fade(DARKBLUE, 0.5f));
		DrawText("MOUSE LEFT BUTTON to CYCLE VOX MODELS", 40, 410, 10, BLUE);
		DrawText("MOUSE MIDDLE BUTTON to ZOOM OR ROTATE CAMERA", 40, 420, 10, BLUE);
		DrawText("UP-DOWN-LEFT-RIGHT KEYS to MOVE CAMERA", 40, 430, 10, BLUE);
		DrawText(TextFormat("File: %s", GetFileName(voxFileNames[currentModel])), 10, 10, 20, GRAY);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	// Unload models data (GPU VRAM)
	for (int i = 0; i < MAX_VOX_FILES; i++) UnloadModel(models[i]);

	CloseWindow();          // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}


