/*******************************************************************************************
*
*   raylib [shapes] example - math sine cosine
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Jopestpe (@jopestpe)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2018-2025 Jopestpe (@jopestpe)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT); 
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - math sine cosine");

    const int wavePoints = 36;
    Vector2 sinePoints[wavePoints];
    Vector2 cosPoints[wavePoints];
    Vector2 center = { (screenWidth/2.0f) - 30.f, screenHeight/2.0f };
    Rectangle start = { 20.f, screenHeight - 120.f , 200.0f, 100.0f};
    float radius = 130.0f;
    float angle = 0.0f;
    bool pause = false;

    for (int i = 0; i < wavePoints; i++)
    {
        float t = i/(float)(wavePoints - 1);
        float angleDeg = t*360.0f;
        float s = sinf(angleDeg*DEG2RAD);
        float c = cosf(angleDeg*DEG2RAD);
        sinePoints[i] = (Vector2){ start.x + t*start.width, start.y + start.height/2.0f - s*(start.height/2.0f) };
        cosPoints[i] = (Vector2){ start.x + t*start.width, start.y + start.height/2.0f - c*(start.height/2.0f) };
    }
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float angleRad = angle*DEG2RAD;
        float cosRad = cosf(angleRad);
        float sinRad = sinf(angleRad);
        
        Vector2 point = { center.x + cosRad*radius, center.y - sinRad*radius };
        Vector2 limitMin = { center.x - radius, center.y - radius };
        Vector2 limitMax = { center.x + radius, center.y + radius };
        
        float complementary = 90.0f - angle;
        float supplementary = 180.0f - angle;
        float explementary = 360.0f - angle;
        
        float tangent = Clamp(tanf(angleRad), -10.0f, 10.0f);
        float cotangent = Clamp(fabs(tangent) < 0.001f ? 0.0f : 1.0f/tangent, -radius, radius);
        Vector2 tangentPoint = { center.x + radius, center.y - tangent * radius };
        Vector2 cotangentPoint = { center.x + cotangent * radius, center.y - radius };

        angle = Wrap(angle + (!pause ? 1.0f : 0.0f), 0.0f, 360.0f);
        //----------------------------------------------------------------------------------
       
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawCircleLinesV(center, radius, GRAY);
            DrawCircleSectorLines(center, radius/2 , -angle, -360.f , 36.0f, PINK);
            DrawCircleSectorLines(center, radius/2 , -angle, -180.f , 36.0f, DARKBLUE);
            DrawCircleSectorLines(center, radius/2 , -angle, -90.f , 36.0f, BEIGE); 
            DrawCircleSectorLines(center, radius/2 , -angle, 0.f, 36.0f, LIME); 
            DrawLineEx((Vector2){ center.x, limitMin.y }, (Vector2){ center.x, limitMax.y }, 1.0f, GRAY);      
            DrawLineEx((Vector2){ limitMin.x, center.y }, (Vector2){ limitMax.x, center.y }, 1.f, GRAY);
            DrawLineEx((Vector2){ center.x, center.y }, (Vector2){ center.x, point.y }, 2.0f, RED);  
            DrawLineDashed((Vector2){ point.x, center.y }, (Vector2){ point.x, point.y }, 10.0f, 4.0f, RED);
            DrawLineEx((Vector2){ center.x, center.y }, (Vector2){ point.x, center.y }, 2.0f, BLUE);         
            DrawLineDashed((Vector2){ center.x , point.y }, (Vector2){ point.x, point.y }, 10.0f, 4.0f, BLUE);
            DrawLineEx((Vector2){ center.x , limitMin.y }, (Vector2){ cotangentPoint.x, limitMin.y }, 2.0f, ORANGE);
            DrawLineEx((Vector2){ limitMax.x , center.y }, (Vector2){ limitMax.x, tangentPoint.y }, 2.0f, PURPLE);
            DrawLineDashed(center, cotangentPoint, 10.0f, 4.0f, ORANGE);
            DrawLineDashed(center, tangentPoint, 10.0f, 4.0f, PURPLE);
            DrawLineEx((Vector2){ center.x , center.y }, point, 2.0f, BLACK);
            DrawCircleV(point, 4.0f, BLACK);
            DrawLineEx((Vector2){ start.x , start.y }, (Vector2){ start.x , start.y + start.height }, 2.0f, GRAY);
            DrawLineEx((Vector2){ start.x + start.width, start.y }, (Vector2){ start.x + start.width, start.y + start.height }, 2.0f, GRAY);
            DrawLineEx((Vector2){ start.x, start.y + start.height/2 }, (Vector2){ start.x + start.width, start.y + start.height/2 }, 2.0f, GRAY);
            DrawCircleV((Vector2){ start.x + (angle/360.0f)*start.width, start.y + ((-sinRad + 1)*start.height / 2.0f) }, 4.0f, RED);
            DrawCircleV((Vector2){ start.x + (angle/360.0f)*start.width, start.y + ((-cosRad + 1)*start.height / 2.0f) }, 4.0f, BLUE);
            DrawSplineLinear(sinePoints, wavePoints, 1.0f, RED);
            DrawSplineLinear(cosPoints, wavePoints, 1.0f, BLUE);
            DrawLine(580, 0, 580, GetScreenHeight(), (Color){ 218, 218, 218, 255 });
            DrawRectangle(580, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 232, 232, 232, 255 });
            // Draw GUI controls
            //------------------------------------------------------------------------------
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(GRAY));
            GuiToggle((Rectangle){ 640, 100, 120, 20}, TextFormat("Pause"), &pause);   
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(LIME));
            GuiSliderBar((Rectangle){ 640, 70, 120, 20}, "Angle", TextFormat("%.0f°", angle), &angle, 0.0f, 360.f);
            DrawText(TextFormat("Supplementary  %0.f°",supplementary), 640, 130, 6, DARKBLUE);
            DrawText(TextFormat("Complementary  %0.f°",complementary), 640, 150, 6, BEIGE);
            DrawText(TextFormat("Explementary  %0.f°",explementary), 640, 170, 6, PINK);
            DrawText(TextFormat("Sine %.2f", sinRad), 640, 190, 6, RED);
            DrawText(TextFormat("Cosine %.2f", cosRad), 640, 210, 6, BLUE);
            DrawText(TextFormat("Tangent %.2f", tangent), 640, 230, 6, PURPLE);
            DrawText(TextFormat("Cotangent %.2f", cotangent), 640, 250, 6, ORANGE);
            DrawText("1", start.x - 8,  start.y, 6, GRAY);
            DrawText("0", start.x - 8,  start.y + start.height/2 - 6, 6, GRAY);
            DrawText("-1", start.x - 12, start.y + start.height - 8, 6, GRAY);
            DrawText("0", start.x - 2,  start.y + start.height + 4, 6, GRAY);
            DrawText("360", start.x + start.width - 8,  start.y + start.height + 4, 6, GRAY);
            //------------------------------------------------------------------------------
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