/*******************************************************************************************
*
*   raylib [shapes] example - rlgl color wheel
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Robin (@RobinsAviary) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Robin (@RobinsAviary)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    // The minimum/maximum points the circle can have
    const unsigned int pointsMin = 3;
    const unsigned int pointsMax = 256;

    // The current number of points and the radius of the circle
    unsigned int triangleCount = 64;
    float pointScale = 150.0f;

    // Slider value, literally maps to value in HSV
    float value = 1.0f;

    // The center of the screen
    Vector2 center = { (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    // The location of the color wheel
    Vector2 circlePosition = center;

    // The currently selected color
    Color color = { 255, 255, 255, 255 };

    // Indicates if the slider is being clicked
    bool sliderClicked = false;

    // Indicates if the current color going to be updated, as well as the handle position
    bool settingColor = false;

    // How the color wheel will be rendered
    unsigned int renderType = RL_TRIANGLES;

    // Enable anti-aliasing
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - rlgl color wheel");

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        triangleCount += (unsigned int)GetMouseWheelMove();
        triangleCount = (unsigned int)Clamp((float)triangleCount, (float)pointsMin, (float)pointsMax);

        Rectangle sliderRectangle = { 42.0f, 16.0f + 64.0f + 45.0f, 64.0f, 16.0f };
        Vector2 mousePosition = GetMousePosition();

        // Checks if the user is hovering over the value slider
        bool sliderHover = (mousePosition.x >= sliderRectangle.x && mousePosition.y >= sliderRectangle.y && mousePosition.x < sliderRectangle.x + sliderRectangle.width && mousePosition.y < sliderRectangle.y + sliderRectangle.height);

        // Copy color as hex
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_C))
        {
            if (IsKeyPressed(KEY_C))
            {
                SetClipboardText(TextFormat("#%02X%02X%02X", color.r, color.g, color.b));
            }
        }

        // Scale up the color wheel, adjusting the handle visually
        if (IsKeyDown(KEY_UP))
        {
            pointScale *= 1.025f;

            if (pointScale > (float)screenHeight/2.0f)
            {
                pointScale = (float)screenHeight/2.0f;
            }
            else
            {
                circlePosition = Vector2Add(Vector2Multiply(Vector2Subtract(circlePosition, center), (Vector2){ 1.025f, 1.025f }), center);
            }
        }

        // Scale down the wheel, adjusting the handle visually
        if (IsKeyDown(KEY_DOWN))
        {
            pointScale *= 0.975f;

            if (pointScale < 32.0f)
            {
                pointScale = 32.0f;
            }
            else
            {
                circlePosition = Vector2Add(Vector2Multiply(Vector2Subtract(circlePosition, center), (Vector2){ 0.975f, 0.975f }), center);
            }

            float distance = Vector2Distance(center, circlePosition)/pointScale;
            float angle = ((Vector2Angle((Vector2){ 0.0f, -pointScale }, Vector2Subtract(center, circlePosition))/PI + 1.0f)/2.0f);

            if (distance > 1.0f)
            {
                circlePosition = Vector2Add((Vector2){ sinf(angle*(PI*2.0f))*pointScale, -cosf(angle*(PI*2.0f))*pointScale }, center);
            }
        }

        // Checks if the user clicked on the color wheel
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Vector2Distance(GetMousePosition(), center) <= pointScale + 10.0f)
        {
            settingColor = true;
        }

        // Update flag when mouse button is released
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) settingColor = false;

        // Check if the user clicked/released the slider for the color's value
        if (sliderHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) sliderClicked = true;

        if (sliderClicked && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) sliderClicked = false;

        // Update render mode accordingly
        if (IsKeyPressed(KEY_SPACE)) renderType = RL_LINES;

        if (IsKeyReleased(KEY_SPACE)) renderType = RL_TRIANGLES;

        // If the slider or the wheel was clicked, update the current color
        if (settingColor || sliderClicked)
        {
            if (settingColor) circlePosition = GetMousePosition();

            float distance = Vector2Distance(center, circlePosition)/pointScale;

            float angle = ((Vector2Angle((Vector2){ 0.0f, -pointScale }, Vector2Subtract(center, circlePosition))/PI + 1.0f)/2.0f);
            if (settingColor && distance > 1.0f) circlePosition = Vector2Add((Vector2){ sinf(angle*(PI*2.0f))*pointScale, -cosf(angle*(PI* 2.0f))*pointScale }, center);

            float angle360 = angle*360.0f;
            float valueActual = Clamp(distance, 0.0f, 1.0f);
            color = ColorLerp((Color){ (int)(value*255.0f), (int)(value*255.0f), (int)(value*255.0f), 255 }, ColorFromHSV(angle360, Clamp(distance, 0.0f, 1.0f), 1.0f), valueActual);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Begin rendering color wheel
        rlBegin(renderType);
        for (unsigned int i = 0; i < triangleCount; i++)
        {
            float angleOffset = ((PI*2.0f)/(float)triangleCount);
            float angle = angleOffset*(float)i;
            float angleOffsetCalculated = ((float)i + 1)*angleOffset;
            Vector2 scale = (Vector2){ pointScale, pointScale };

            Vector2 offset = Vector2Multiply((Vector2){ sinf(angle), -cosf(angle) }, scale);
            Vector2 offset2 = Vector2Multiply((Vector2){ sinf(angleOffsetCalculated), -cosf(angleOffsetCalculated) }, scale);

            Vector2 position = Vector2Add(center, offset);
            Vector2 position2 = Vector2Add(center, offset2);

            float angleNonRadian = (angle/(2.0f*PI))*360.0f;
            float angleNonRadianOffset = (angleOffset/(2.0f*PI))*360.0f;

            Color currentColor = ColorFromHSV(angleNonRadian, 1.0f, 1.0f);
            Color offsetColor = ColorFromHSV(angleNonRadian + angleNonRadianOffset, 1.0f, 1.0f);

            // Input vertices differently depending on mode
            if (renderType == RL_TRIANGLES)
            {
                // RL_TRIANGLES expects three vertices per triangle
                rlColor4ub(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
                rlVertex2f(position.x, position.y);
                rlColor4f(value, value, value, 1.0f);
                rlVertex2f(center.x, center.y);
                rlColor4ub(offsetColor.r, offsetColor.g, offsetColor.b, offsetColor.a);
                rlVertex2f(position2.x, position2.y);
            }
            else if (renderType == RL_LINES)
            {
                // RL_LINES expects two vertices per line
                rlColor4ub(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
                rlVertex2f(position.x, position.y);
                rlColor4ub(WHITE.r, WHITE.g, WHITE.b, WHITE.a);
                rlVertex2f(center.x, center.y);

                rlVertex2f(center.x, center.y);
                rlColor4ub(offsetColor.r, offsetColor.g, offsetColor.b, offsetColor.a);
                rlVertex2f(position2.x, position2.y);

                rlVertex2f(position2.x, position2.y);
                rlColor4ub(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
                rlVertex2f(position.x, position.y);
            }
        }
        rlEnd();

        // Make the handle slightly more visible overtop darker colors
        Color handleColor = BLACK;

        if (Vector2Distance(center, circlePosition)/pointScale <= 0.5f && value <= 0.5f)
        {
            handleColor = DARKGRAY;
        }

        // Draw the color handle
        DrawCircleLinesV(circlePosition, 4.0f, handleColor);

        // Draw the color in a preview, with a darkened outline.
        DrawRectangleV((Vector2){ 8.0f, 8.0f }, (Vector2){ 64.0f, 64.0f }, color);
        DrawRectangleLinesEx((Rectangle){ 8.0f, 8.0f, 64.0f, 64.0f }, 2.0f, ColorLerp(color, BLACK, 0.5f));

        // Draw current color as hex and decimal
        DrawText(TextFormat("#%02X%02X%02X\n(%d, %d, %d)", color.r, color.g, color.b, color.r, color.g, color.b), 8, 8 + 64 + 8, 20, DARKGRAY);

        // Update the visuals for the copying text
        Color copyColor = DARKGRAY;
        unsigned int offset = 0;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_C))
        {
            copyColor = DARKGREEN;
            offset = 4;
        }

        // Draw the copying text
        DrawText("press ctrl+c to copy!", 8, 425 - offset, 20, copyColor);

        // Display the number of rendered triangles
        DrawText(TextFormat("triangle count: %d", triangleCount), 8, 395, 20, DARKGRAY);

        // Slider to change color's value
        GuiSliderBar(sliderRectangle, "value: ", "", &value, 0.0f, 1.0f);

        // Draw FPS next to outlined color preview
        DrawFPS(64 + 16, 8);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}