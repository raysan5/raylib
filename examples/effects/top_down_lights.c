/*******************************************************************************************
*
*   raylib [effects] example - Top Down Lights
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Jeffery Myers
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

// Custom Blend Modes
#define RLGL_SRC_ALPHA 0x0302
#define RLGL_MIN 0x8007
#define RLGL_MAX 0x8008

#define MAX_BOXES 20

typedef struct  
{
    Vector2 Vertecies[4];
}ShadowGeometry;

#define MAX_SHADOWS MAX_BOXES*3 // MAX_BOXES *3. Each box can cast up to two shadow volumes for the edges it is away from, and one for the box itself

typedef struct 
{
    // is this light slot active
    bool Active;

    // does this light need to be updated
    bool Dirty;

    // is this light in a valid position
    bool Valid;

    // Light position
    Vector2 Position;

    // alpha mask for the light
	RenderTexture Mask;

    // the distance the light touches
	float OuterRadius;

    // a cached rectangle of the light bounds to help with culling
	Rectangle Bounds;

    ShadowGeometry Shadows[MAX_SHADOWS];
    int ShadowCount;
}LightInfo;

#define MAX_LIGHTS 16
LightInfo Lights[MAX_LIGHTS] = { 0 };

// move a light and mark it as dirty so that we update it's mask next frame
void MoveLight(int slot, float x, float y)
{
    Lights[slot].Dirty = true;
    Lights[slot].Position.x = x; 
    Lights[slot].Position.y = y;

    // update the cached bounds
    Lights[slot].Bounds.x = x - Lights[slot].OuterRadius;
    Lights[slot].Bounds.y = y - Lights[slot].OuterRadius;
}

// compute a shadow volume for the edge
// takes the edge and projects it back by the light radius and turns it into a quad
void ComputeShadowVolumeForEdge(int slot, Vector2 sp, Vector2 ep)
{
    if (Lights[slot].ShadowCount >= MAX_SHADOWS)
        return;

	float extension = Lights[slot].OuterRadius*2;

	Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, Lights[slot].Position));
	Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

	Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, Lights[slot].Position));
	Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

    Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[0] = sp;
	Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[1] = ep;
	Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[2] = epProjection;
    Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[3] = spProjection;

    Lights[slot].ShadowCount++;
}

// draw the light and shadows to the mask for a light
void UpdateLightMask(int slot)
{
    // use the light mask
	BeginTextureMode(Lights[slot].Mask);

	ClearBackground(WHITE);

	// force the blend mode to only set the alpha of the destination
	rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
	rlSetBlendMode(BLEND_CUSTOM);

    // if we are valid, then draw the light radius to the alpha mask
	if (Lights[slot].Valid)
		DrawCircleGradient((int)Lights[slot].Position.x, (int)Lights[slot].Position.y, Lights[slot].OuterRadius, ColorAlpha(WHITE, 0), WHITE);
	rlDrawRenderBatchActive();

    // cut out the shadows from the light radius by forcing the alpha to maximum
	rlSetBlendMode(BLEND_ALPHA);
	rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MAX);
	rlSetBlendMode(BLEND_CUSTOM);

    // draw the shadows to the alpha mask
	for (int i = 0; i < Lights[slot].ShadowCount; i++)
	{
		DrawTriangleFan(Lights[slot].Shadows[i].Vertecies, 4, WHITE);
	}

	rlDrawRenderBatchActive();
	// go back to normal blend mode
	rlSetBlendMode(BLEND_ALPHA);

	EndTextureMode();
}

// setup a light
void SetUpLight(int slot, float x, float y, float radius)
{
	Lights[slot].Active = true;
    Lights[slot].Valid = false; // the light must prove it is valid
	Lights[slot].Mask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	Lights[slot].OuterRadius = radius;

	Lights[slot].Bounds.width = radius * 2;
	Lights[slot].Bounds.height = radius * 2;

	MoveLight(slot, x, y);

    // force the render texture to have something in it
    UpdateLightMask(slot);
}

// see if a light needs to update it's mask
bool UpdateLight(int slot, Rectangle* boxes, int count)
{
    if (!Lights[slot].Active || !Lights[slot].Dirty)
        return false;

    Lights[slot].Dirty = false;
    Lights[slot].ShadowCount = 0;
    Lights[slot].Valid = false;

	for (int i = 0; i < count; i++)
	{
		// are we in a box, if so we are not valid
		if (CheckCollisionPointRec(Lights[slot].Position, boxes[i]))
			return false;

        // if this box is outside our bounds, we can skip it
		if (!CheckCollisionRecs(Lights[slot].Bounds, boxes[i]))
			continue;

        // check the edges that are on the same side we are, and cast shadow volumes out from them.
        
		// top
		Vector2 sp = (Vector2){ boxes[i].x, boxes[i].y };
		Vector2 ep = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y };

		if (Lights[slot].Position.y > ep.y)    
            ComputeShadowVolumeForEdge(slot, sp, ep);

		// right
		sp = ep;
		ep.y += boxes[i].height;
		if (Lights[slot].Position.x < ep.x)
            ComputeShadowVolumeForEdge(slot, sp, ep);

		// bottom
		sp = ep;
		ep.x -= boxes[i].width;
		if (Lights[slot].Position.y < ep.y)
            ComputeShadowVolumeForEdge(slot, sp, ep);

		// left
		sp = ep;
		ep.y -= boxes[i].height;
		if (Lights[slot].Position.x > ep.x)
            ComputeShadowVolumeForEdge(slot, sp, ep);

		// the box itself
		Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[0] = (Vector2){ boxes[i].x, boxes[i].y };
		Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[1] = (Vector2){ boxes[i].x, boxes[i].y + boxes[i].height };
		Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[2] = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y + boxes[i].height };
		Lights[slot].Shadows[Lights[slot].ShadowCount].Vertecies[3] = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y };
		Lights[slot].ShadowCount++;
	}

    Lights[slot].Valid = true;

    UpdateLightMask(slot);

    return true;
}

// set up some boxes
void SetupBoxes(Rectangle* boxes, int *count)
{
	boxes[0] = (Rectangle){ 150,80, 40, 40 };
	boxes[1] = (Rectangle){ 1200, 700, 40, 40 };
	boxes[2] = (Rectangle){ 200, 600, 40, 40 };
	boxes[3] = (Rectangle){ 1000, 50, 40, 40 };
	boxes[4] = (Rectangle){ 500, 350, 40, 40 };

	for (int i = 5; i < MAX_BOXES; i++)
	{
		boxes[i] = (Rectangle){(float)GetRandomValue(0,GetScreenWidth()), (float)GetRandomValue(0,GetScreenHeight()), (float)GetRandomValue(10,100), (float)GetRandomValue(10,100) };
	}

    *count = MAX_BOXES;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "raylib [effects] example - top down lights");
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // initialize our 'world' of boxes
	Rectangle boxes[MAX_BOXES];
    int boxCount = 0;
    SetupBoxes(boxes, &boxCount);

    // create a checkerboard ground texture
	Image img = GenImageChecked(64, 64, 32, 32, DARKBROWN, DARKGRAY);
	Texture2D backgroundTexture = LoadTextureFromImage(img);
	UnloadImage(img);

	// create a global light mask to hold all the blended lights
	RenderTexture lightMask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    // setup initial light
    SetUpLight(0, 600, 400, 300);
    int nextLight = 1;

	bool showLines = false;

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        // drag light 0
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			MoveLight(0, GetMousePosition().x, GetMousePosition().y);

		// make a new light
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && nextLight < MAX_LIGHTS)
        {
            SetUpLight(nextLight, GetMousePosition().x, GetMousePosition().y, 200);
            nextLight++;
        }

		// toggle debug info
		if (IsKeyPressed(KEY_F1))
			showLines = !showLines;

        // update the lights and keep track if any were dirty so we know if we need to update the master light mask
		bool dirtyLights = false;
		for (int i = 0; i < MAX_LIGHTS; i++)
		{
			if (UpdateLight(i, boxes, boxCount))
				dirtyLights = true;
		}

		// update the light mask
		if (dirtyLights)
		{
			// build up the light mask
			BeginTextureMode(lightMask);
			ClearBackground(BLACK);

			// force the blend mode to only set the alpha of the destination
			rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
			rlSetBlendMode(BLEND_CUSTOM);

			// merge in all the light masks
			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				if (Lights[i].Active)
					DrawTextureRec(Lights[i].Mask.texture, (Rectangle){ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), WHITE);
			}

			rlDrawRenderBatchActive();

			// go back to normal
			rlSetBlendMode(BLEND_ALPHA);
			EndTextureMode();
		}

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
			
			// draw the tile background
			DrawTextureRec(backgroundTexture, (Rectangle){ 0,0,(float)GetScreenWidth(),(float)GetScreenHeight() }, Vector2Zero(), WHITE);
			
			// overlay the shadows from all the lights
			DrawTextureRec(lightMask.texture, (Rectangle){ 0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight() }, Vector2Zero(), ColorAlpha(WHITE, showLines ? 0.75f : 1.0f));

			// draw the lights
			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				if (Lights[i].Active)
					DrawCircle((int)Lights[i].Position.x, (int)Lights[i].Position.y, 10, i == 0 ? YELLOW : WHITE);
			}

			if (showLines)
			{
				for (int s = 0; s < Lights[0].ShadowCount; s++)
				{
					DrawTriangleFan(Lights[0].Shadows[s].Vertecies, 4, DARKPURPLE);
				}

				for (int b = 0; b < boxCount; b++)
				{
					if (CheckCollisionRecs(boxes[b],Lights[0].Bounds))
						DrawRectangleRec(boxes[b], PURPLE);

					DrawRectangleLines((int)boxes[b].x, (int)boxes[b].y, (int)boxes[b].width, (int)boxes[b].height, DARKBLUE);
				}

				DrawText("(F1) Hide Shadow Volumes", 0, 60, 20, GREEN);
			}
			else
			{
				DrawText("(F1) Show Shadow Volumes", 0, 60, 20, GREEN);
			}

            DrawFPS(0, 0);
            DrawText("Drag to move light #1", 0, 20, 20, DARKGREEN);
            DrawText("Right click to add new light", 0, 40, 20, DARKGREEN);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(backgroundTexture);
	UnloadRenderTexture(lightMask);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (Lights[i].Active)
			UnloadRenderTexture(Lights[i].Mask);
	}

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}