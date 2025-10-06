/*******************************************************************************************
*
*   raylib [shapes] example - math sine cosine
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Midiphony (@midiphony) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Zero (@zerohorsepower)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h" // Required for: Vector2 operations and Clamp()
#include <stdlib.h> // Required for: malloc(), free()
#include <math.h> // Required for: cosf(), sinf()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const Color cosineColor = RED;
    const Color sineColor = ORANGE;
    const int pointsSize = 6;
    const int lineThickness = 2;

    const int screenWidth = 800;
    const int screenHeight = 450;

    // Circle
    const int circleX = screenWidth/4 - 20;
    const int circleY = screenHeight/2;
    const Vector2 circlePosition = { circleX, circleY };
    const int circleRadius = 140;
    const int circleLeft = circleX - circleRadius;
    const int circleRight = circleX + circleRadius;
    const int circleTop = circleY - circleRadius;
    const int circleBottom = circleY + circleRadius;

    const int circleTextFontSize = 20;

    // Graph
    const int graphLeft = screenWidth/2;
    const int graphRight = 750;
    const int graphHeight = 200;
    const int graphHalfHeight = graphHeight/2;
    const int graphYMiddle = screenHeight/2;
    const int graphTop = graphYMiddle - graphHalfHeight;
    const int graphBottom = graphYMiddle + graphHalfHeight;
    const int graphWidth = graphRight - graphLeft;

    const int graphTextFontSize = 20;
    const int graphTextPadding = 10;

    Vector2* cosineCurvePoints = (Vector2*)malloc(graphWidth*sizeof(Vector2)); // Points array
    Vector2* sineCurvePoints = (Vector2*)malloc(graphWidth*sizeof(Vector2)); // Points array

    // Initialize cosine curve
    for (int x = 0; x < graphWidth; x++)
    {
        float y = -cosf(((float)x/graphWidth)*2.0*PI);
        int yCoord = graphYMiddle + y*graphHalfHeight;
        cosineCurvePoints[x] = (Vector2){ graphLeft + x, yCoord };
    }

    // Initialize sine curve
    for (int x = 0; x < graphWidth; x++)
    {
        float y = sinf(((float)x/graphWidth)*2.0*PI);
        int yCoord = graphYMiddle - y*graphHalfHeight;
        sineCurvePoints[x] = (Vector2){ graphLeft + x, yCoord };
    }

    const int windowSplitX = ((circleX + circleRadius) + graphLeft)/2;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - math sine cosine");

    const int circleTextMaxLength = MeasureText("-1.000", circleTextFontSize);
    const int graphTextMaxLength = MeasureText("-1.000", graphTextFontSize);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePosition = GetMousePosition();
        float angleInDegrees = 0.0f;
        float angle = 0;

        if (mousePosition.x <= windowSplitX) // Calculate angle relative to the circle
        {
            angle = Vector2Angle(Vector2Subtract(mousePosition, circlePosition), (Vector2) { 1, 0 });
            if (angle < 0.0f)
            {
                angle += 2*PI;
            }
        }
        else // Calculate angle relative to the graph
        {
            angle = Clamp((mousePosition.x - graphLeft)*2*PI/graphWidth, 0.0f, 2*PI);
        }

        angleInDegrees = angle*RAD2DEG;

        float cosine = cosf(angle);
        float sine = sinf(angle);
        int pointX = circleX + circleRadius*cosine;
        int pointY = circleY - circleRadius*sine;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw top angle label
        DrawText(TextFormat("Angle:%.1f", angleInDegrees), 20, 20, 30, GRAY);

        // Trigonometry circle
        // --------------------
        DrawRing(circlePosition, circleRadius - lineThickness/2, circleRadius + lineThickness/2, 0, 360, 0, GRAY); // 0 ring segment to let DrawRing choose a number of segments giving a smooth circle
        DrawLineEx((Vector2) { circleLeft, circleY }, (Vector2) { circleRight, circleY }, lineThickness, GRAY);
        DrawLineEx((Vector2) { circleX, circleTop }, (Vector2) { circleX, circleBottom }, lineThickness, GRAY);

        DrawCircleSectorLines(circlePosition, circleRadius/3, 0, -angleInDegrees, 0, BLUE);

        // Draw line to point
        DrawLine(circleX, circleY, pointX, pointY, GRAY);

        // Draw cosine point
        DrawLineEx((Vector2) { circleX, circleY }, (Vector2) { pointX, circleY }, lineThickness, cosineColor);
        DrawText(TextFormat("%.3f", cosine), ((pointX + circleX)/2) - circleTextMaxLength/2, circleY + 2, circleTextFontSize, cosineColor);
        // Draw sine point
        DrawLineEx((Vector2) { pointX, circleY }, (Vector2) { pointX, pointY }, lineThickness, sineColor);
        DrawText(TextFormat("%.3f", sine), pointX + 5, (pointY + circleY)/2 - circleTextFontSize/2, circleTextFontSize, sineColor);

        // Draw point
        DrawCircle(pointX, pointY, pointsSize, BLACK);
        // --------------------

        // Window split
        DrawLine(windowSplitX, 0, windowSplitX, screenHeight - 1, GRAY);

        // Graph
        // --------------------
        // Draw graph borders
        DrawLineEx((Vector2) { graphLeft, graphTop }, (Vector2) { graphLeft, graphBottom }, 2, GRAY);
        DrawLineEx((Vector2) { graphRight, graphTop }, (Vector2) { graphRight, graphBottom }, 2, GRAY);
        DrawLineEx((Vector2) { graphLeft, graphYMiddle }, (Vector2) { graphRight, graphYMiddle }, 2, GRAY);

        // Draw graph outer texts
        DrawText("1", graphLeft - graphTextPadding - MeasureText("1", graphTextFontSize), graphTop - graphTextFontSize/2, graphTextFontSize, GRAY);
        DrawText("0", graphLeft - graphTextPadding - MeasureText("0", graphTextFontSize), graphYMiddle - graphTextFontSize/2, graphTextFontSize, GRAY);
        DrawText("-1", graphLeft - graphTextPadding - MeasureText("-1", graphTextFontSize), graphBottom - graphTextFontSize/2, graphTextFontSize, GRAY);
        DrawText("0", graphLeft - MeasureText("0", graphTextFontSize)/2, graphBottom + graphTextPadding/2, graphTextFontSize, GRAY);
        DrawText("360", graphRight - MeasureText("360", graphTextFontSize)/2, graphBottom + graphTextPadding/2, graphTextFontSize, GRAY);

        // Draw cosine curve
        DrawSplineLinear(cosineCurvePoints, graphWidth, 2, cosineColor);
        DrawText("cos", graphRight + graphTextPadding, cosineCurvePoints[graphWidth - 1].y - graphTextFontSize/2, graphTextFontSize, cosineColor);

        // Draw sine curve
        DrawSplineLinear(sineCurvePoints, graphWidth, 2, sineColor);
        DrawText("sin", graphRight + graphTextPadding, sineCurvePoints[graphWidth - 1].y - graphTextFontSize/2, graphTextFontSize, sineColor);

        // Draw graph progress line
        int x = graphLeft + graphWidth*angleInDegrees/360.0f;
        DrawLine(x, graphBottom, x, graphTop, BLUE);

        // Draw cosine and sine points on graph
        int cosineY = graphYMiddle - cosine*graphHalfHeight;
        int sineY = graphYMiddle - sine*graphHalfHeight;
        DrawCircle(x, cosineY, pointsSize, cosineColor);
        DrawText(TextFormat("%.3f", cosine), x - circleTextMaxLength/2, cosineY - circleTextFontSize - 5, circleTextFontSize, cosineColor);
        DrawCircle(x, sineY, pointsSize, sineColor);
        DrawText(TextFormat("%.3f", sine), x - circleTextMaxLength/2, sineY + 8, circleTextFontSize, sineColor);
        // --------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    free(cosineCurvePoints);
    free(sineCurvePoints);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}