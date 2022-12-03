/*******************************************************************************************
*
*   raylib [textures] example - Draw a texture along a segmented curve
*
*   Example originally created with raylib 4.5
*
*   Example contributed by Jeffery Myers and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2022 Jeffery Myers and Ramon Santamaria (@raysan5)
*
********************************************************************************************/


#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

Texture RoadTexture = { 0 };

bool ShowCurve = false;

float Width = 50;
int Segments = 24;

Vector2 SP = { 0 };
Vector2 SPTangent = { 0 };

Vector2 EP = { 0 };
Vector2 EPTangent = { 0 };

Vector2* Selected = NULL;

void DrawCurve()
{
	if (ShowCurve)
		DrawLineBezierCubic(SP, EP, SPTangent, EPTangent, 2, BLUE);

	// draw the various control points and highlight where the mouse is
	DrawLineV(SP, SPTangent, SKYBLUE);
	DrawLineV(EP, EPTangent, PURPLE);
	Vector2 mouse = GetMousePosition();

	if (CheckCollisionPointCircle(mouse, SP, 6))
		DrawCircleV(SP, 7, YELLOW);
	DrawCircleV(SP, 5, RED);

	if (CheckCollisionPointCircle(mouse, SPTangent, 6))
		DrawCircleV(SPTangent, 7, YELLOW);
	DrawCircleV(SPTangent, 5, MAROON);

	if (CheckCollisionPointCircle(mouse, EP, 6))
		DrawCircleV(EP, 7, YELLOW);
	DrawCircleV(EP, 5, GREEN);

	if (CheckCollisionPointCircle(mouse, EPTangent, 6))
		DrawCircleV(EPTangent, 7, YELLOW);
	DrawCircleV(EPTangent, 5, DARKGREEN);
}

void EditCurve()
{
	// if the mouse is not down, we are not editing the curve so clear the selection
	if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON))
	{
		Selected = NULL;
		return;
	}

	// if a point was selected, move it
	if (Selected)
	{
		*Selected = Vector2Add(*Selected, GetMouseDelta());
		return;
	}

	// the mouse is down, and nothing was selected, so see if anything was picked
	Vector2 mouse = GetMousePosition();

	if (CheckCollisionPointCircle(mouse, SP, 6))
		Selected = &SP;
	else if (CheckCollisionPointCircle(mouse, SPTangent, 6))
		Selected = &SPTangent;
	else if (CheckCollisionPointCircle(mouse, EP, 6))
		Selected = &EP;
	else if (CheckCollisionPointCircle(mouse, EPTangent, 6))
		Selected = &EPTangent;
}

void DrawTexturedCurve()
{
	const float step = 1.0f / Segments;

	Vector2 previous = SP;
	Vector2 previousTangent = { 0 };
	float previousV = 0;

	// we can't compute a tangent for the first point, so we need to reuse the tangent from the first segment
	bool tangentSet = false;

	Vector2 current = { 0 };
	float t = 0.0f;

	for (int i = 1; i <= Segments; i++)
	{
		// segment the curve
		t = step * i;
		float a = powf(1 - t, 3);
		float b = 3 * powf(1 - t, 2) * t;
		float c = 3 * (1 - t) * powf(t, 2);
		float d = powf(t, 3);

		// compute the endpoint for this segment
		current.y = a * SP.y + b * SPTangent.y + c * EPTangent.y + d * EP.y;
		current.x = a * SP.x + b * SPTangent.x + c * EPTangent.x + d * EP.x;

		// vector from previous to current
		Vector2 delta = { current.x - previous.x, current.y - previous.y };

		// the right hand normal to the delta vector
		Vector2 normal = Vector2Normalize((Vector2){ -delta.y, delta.x });

		// the v texture coordinate of the segment (add up the length of all the segments so far)
		float v = previousV + Vector2Length(delta);

		// make sure the start point has a normal
		if (!tangentSet)
		{
			previousTangent = normal;
			tangentSet = true;
		}

		// extend out the normals from the previous and current points to get the quad for this segment
		Vector2 prevPosNormal = Vector2Add(previous, Vector2Scale(previousTangent, Width));
		Vector2 prevNegNormal = Vector2Add(previous, Vector2Scale(previousTangent, -Width));

		Vector2 currentPosNormal = Vector2Add(current, Vector2Scale(normal, Width));
		Vector2 currentNegNormal = Vector2Add(current, Vector2Scale(normal, -Width));

		// draw the segment as a quad
		rlSetTexture(RoadTexture.id);
		rlBegin(RL_QUADS);

		rlColor4ub(255,255,255,255);
		rlNormal3f(0.0f, 0.0f, 1.0f);

		rlTexCoord2f(0, previousV);
		rlVertex2f(prevNegNormal.x, prevNegNormal.y);

		rlTexCoord2f(1, previousV);
		rlVertex2f(prevPosNormal.x, prevPosNormal.y);

		rlTexCoord2f(1, v);
		rlVertex2f(currentPosNormal.x, currentPosNormal.y);

		rlTexCoord2f(0, v);
		rlVertex2f(currentNegNormal.x, currentNegNormal.y);

		rlEnd();

		// the current step is the start of the next step
		previous = current;
		previousTangent = normal;
		previousV = v;
	}
}

void UpdateOptions()
{
	if (IsKeyPressed(KEY_SPACE))
		ShowCurve = !ShowCurve;

	// width
	if (IsKeyPressed(KEY_EQUAL))
		Width += 2;

	if (IsKeyPressed(KEY_MINUS))
		Width -= 2;

	if (Width < 2)
		Width = 2;

	// segments

	if (IsKeyPressed(KEY_LEFT_BRACKET))
		Segments -= 2;

	if (IsKeyPressed(KEY_RIGHT_BRACKET))
		Segments += 2;

	if (Segments < 2)
		Segments = 2;
}

int main ()
{
	// set up the window
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "raylib [textures] examples - textured curve");
	SetTargetFPS(144);

	// load the road texture
	RoadTexture = LoadTexture("resources/roadTexture_01.png");

	// setup the curve
	SP = (Vector2){ 80, 400 };
	SPTangent = (Vector2){ 600, 100 };

	EP = (Vector2){ 1200, 400 };
	EPTangent = (Vector2){ 600, 700 };

	// game loop
	while (!WindowShouldClose())
	{
		EditCurve();
		UpdateOptions();

		BeginDrawing();

		ClearBackground(BLACK);

		DrawTexturedCurve();
		DrawCurve();
	
		DrawText("Drag points to move curve, press space to show/hide base curve", 10, 0, 20, WHITE);
		DrawText(TextFormat("Width %2.0f + and - to adjust", Width), 10, 20, 20, WHITE);
		DrawText(TextFormat("Segments %d [ and ] to adjust", Segments), 10, 40, 20, WHITE);
		DrawFPS(10, 60);

		EndDrawing();
	}

	// cleanup
	UnloadTexture(RoadTexture);
	CloseWindow();
	return 0;
}

   