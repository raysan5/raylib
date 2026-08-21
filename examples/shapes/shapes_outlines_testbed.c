/*******************************************************************************************
*
*   raylib [shapes] example - outlines testbed
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 6.1, last time updated with raylib 6.1
*
*   Example contributed by Matthew Roush (@MatthewRoush) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Matthew Roush (@MatthewRoush)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

#include <stdbool.h>

#define COLOR_FILLED  DARKBLUE
#define COLOR_OUTLINE YELLOW

#define SHAPE_SPACING 14.0f
#define SHAPE_SIZE 46.0f
#define BOX_SPACING 10.0f

#define MOUSE_CAMERA_ZOOM_SPEED 0.3f
#define KEYBOARD_CAMERA_MOVE_SPEED 10.0f
#define KEYBOARD_CAMERA_ZOOM_SPEED 0.1f

#define CAMERA_ZOOM_MIN 0.1f
#define CAMERA_ZOOM_MAX 1000.0f

// The shapes are ordered according to their order in this enum, left to right
enum {
    ORDER_RECTANGLE = 0,
    ORDER_RECTANGLE_ROUNDED,
    ORDER_CIRCLE,
    ORDER_ELLIPSE,
    ORDER_CIRCLE_SECTOR,
    ORDER_RING,
    ORDER_TRIANGLE,
    ORDER_POLYGON,
    COUNT_SHAPES
};

// The line style groups are ordered according to their order in this enum, top to bottom
enum {
    ORDER_LINES = 0,
    ORDER_LINES_EX_WORLD,
    ORDER_LINES_EX_SCREEN,
};

#define BOX_WIDTH (SHAPE_SIZE*COUNT_SHAPES + SHAPE_SPACING*(COUNT_SHAPES - 1) + BOX_SPACING*2.0f)
#define BOX_HEIGHT (SHAPE_SIZE + BOX_SPACING*2.0f)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - outlines testbed");

    Camera2D camera = { .zoom = 1 };

    // User configurable options while running the program.
    float lineOpacity          = 130;
    float lineThickness        = 4.0f;
    float rectangleRoundness   = 0.4f;
    float rectangleSegments    = 9;
    float ellipseRadiusY       = 0.5f;
    float circleStartAngle     = 20.0f;
    float circleEndAngle       = 270.0f;
    float circleSegments       = 36;
    float ringInnerRadiusScale = 0.3f;
    float polygonSides         = 6;

    bool disableMouseControl = false;

    const Rectangle optionsBackground = { 510, 0, 290, 450 };

    const Vector2 zoomPoint = {((float)screenWidth - optionsBackground.width)/2.0f, (float)screenHeight/2.0f};

    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(WHITE));

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        const Vector2 mousePosScreen = GetMousePosition();
        const Vector2 mouseWheelVec = GetMouseWheelMoveV();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosScreen, optionsBackground)) disableMouseControl = true;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) disableMouseControl = false;

        if (!disableMouseControl) {
            if (mouseWheelVec.y != 0.0f) {
                const Vector2 prevWorldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);

                camera.zoom *= exp2f(MOUSE_CAMERA_ZOOM_SPEED*mouseWheelVec.y); // Constant zoom rate
                camera.zoom = Clamp(camera.zoom, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);

                const Vector2 worldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);
                camera.target = Vector2Add(camera.target, Vector2Subtract(prevWorldZoomPoint, worldZoomPoint));
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                const Vector2 mouseDelta = GetMouseDelta();
                camera.target.x -= mouseDelta.x/camera.zoom;
                camera.target.y -= mouseDelta.y/camera.zoom;

                const int mouseMaxX = (int)optionsBackground.x;
                const int mouseMaxY = screenHeight;

                int newX = (int)mousePosScreen.x;
                int newY = (int)mousePosScreen.y;

                // In C, the '%' operator computes the remainder, we want the modulus
                newX = (newX%mouseMaxX + mouseMaxX)%mouseMaxX;
                newY = (newY%mouseMaxY + mouseMaxY)%mouseMaxY;

                if ((newX != (int)mousePosScreen.x) || (newY != (int)mousePosScreen.y)) SetMousePosition(newX, newY);
            }
        }

        if (IsKeyDown(KEY_A)) camera.target.x -= KEYBOARD_CAMERA_MOVE_SPEED/camera.zoom;
        if (IsKeyDown(KEY_D)) camera.target.x += KEYBOARD_CAMERA_MOVE_SPEED/camera.zoom;
        if (IsKeyDown(KEY_W)) camera.target.y -= KEYBOARD_CAMERA_MOVE_SPEED/camera.zoom;
        if (IsKeyDown(KEY_S)) camera.target.y += KEYBOARD_CAMERA_MOVE_SPEED/camera.zoom;

        if (IsKeyDown(KEY_UP))
        {
            const Vector2 prevWorldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);

            camera.zoom *= exp2f(KEYBOARD_CAMERA_ZOOM_SPEED);
            camera.zoom = Clamp(camera.zoom, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);

            const Vector2 worldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);
            camera.target = Vector2Add(camera.target, Vector2Subtract(prevWorldZoomPoint, worldZoomPoint));
        }
        if (IsKeyDown(KEY_DOWN))
        {
            const Vector2 prevWorldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);

            camera.zoom *= exp2f(-KEYBOARD_CAMERA_ZOOM_SPEED);
            camera.zoom = Clamp(camera.zoom, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);

            const Vector2 worldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);
            camera.target = Vector2Add(camera.target, Vector2Subtract(prevWorldZoomPoint, worldZoomPoint));
        }

        if (IsKeyPressed(KEY_Z))
        {
            const Vector2 zoomPoint = {((float)screenWidth - optionsBackground.width)/2.0f, (float)screenHeight/2.0f};
            const Vector2 prevWorldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);

            camera.zoom = 1.0f;

            const Vector2 worldZoomPoint = GetScreenToWorld2D(zoomPoint, camera);
            camera.target = Vector2Add(camera.target, Vector2Subtract(prevWorldZoomPoint, worldZoomPoint));
        }

        if (IsKeyPressed(KEY_C)) camera.target = (Vector2){ 0 };
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color){ 50, 50, 55, 255 });

            Color colorOutline = COLOR_OUTLINE;
            colorOutline.a = (unsigned char)lineOpacity;

            BeginMode2D(camera);

            const float shapeOffset = BOX_SPACING*2.0f;
            const float shapePaddingX = SHAPE_SIZE + SHAPE_SPACING;
            const float shapePaddingY = SHAPE_SIZE + SHAPE_SPACING + BOX_SPACING*2.0f;

            const float radiusX = SHAPE_SIZE/2.0f;
            const float radiusY = radiusX*ellipseRadiusY;

            const float ringOuterRadius = SHAPE_SIZE/2.0f;
            const float ringInnerRadius = ringOuterRadius*ringInnerRadiusScale;

            const Vector2 triangleVertex0Offset = { 0.0f, SHAPE_SIZE*0.8f };
            const Vector2 triangleVertex1Offset = { SHAPE_SIZE*0.6f, SHAPE_SIZE };
            const Vector2 triangleVertex2Offset = { SHAPE_SIZE, 0.0f };

            // ----------------------------------------
            // Draw*Lines()
            float posY = shapeOffset + shapePaddingY*ORDER_LINES;

            // Rectangle
            float posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE;
            DrawRectangleRec((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, COLOR_FILLED);
            DrawRectangleLines(posX, posY, SHAPE_SIZE, SHAPE_SIZE, colorOutline);

            // Rectangle Rounded
            posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE_ROUNDED;
            DrawRectangleRounded((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, COLOR_FILLED);
            DrawRectangleRoundedLines((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, colorOutline);

            // Circle
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE;
            DrawCircleV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, COLOR_FILLED);
            DrawCircleLinesV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, colorOutline);

            // Ellipse
            posX = shapeOffset + shapePaddingX*ORDER_ELLIPSE;
            DrawEllipseV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, COLOR_FILLED);
            DrawEllipseLinesV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, colorOutline);

            // Circle Sector
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE_SECTOR;
            DrawCircleSector((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawCircleSectorLines((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, colorOutline);

            // Ring
            posX = shapeOffset + shapePaddingX*ORDER_RING;
            DrawRing((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawRingLines((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, colorOutline);

            // Triangle
            posX = shapeOffset + shapePaddingX*ORDER_TRIANGLE;
            DrawTriangle((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                         (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                         (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                         COLOR_FILLED);
            DrawTriangleLines((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                              (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                              (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                              colorOutline);

            // Polygon
            posX = shapeOffset + shapePaddingX*ORDER_POLYGON;
            DrawPoly((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, COLOR_FILLED);
            DrawPolyLines((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, colorOutline);

            // Group the shapes
            GuiGroupBox((Rectangle){ shapeOffset - BOX_SPACING, posY - BOX_SPACING, BOX_WIDTH, BOX_HEIGHT }, "Draw*Lines()");
            // ----------------------------------------

            // ----------------------------------------
            // Draw*LinesEx() with world space pixel thickness
            posY = shapeOffset + shapePaddingY*ORDER_LINES_EX_WORLD;

            // Rectangle
            posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE;
            DrawRectangleRec((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, COLOR_FILLED);
            DrawRectangleLinesEx((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, lineThickness, colorOutline);

            // Rectangle Rounded
            posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE_ROUNDED;
            DrawRectangleRounded((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, COLOR_FILLED);
            DrawRectangleRoundedLinesEx((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, lineThickness, colorOutline);

            // Circle
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE;
            DrawCircleV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, COLOR_FILLED);
            DrawCircleLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, lineThickness, colorOutline);

            // Ellipse
            posX = shapeOffset + shapePaddingX*ORDER_ELLIPSE;
            DrawEllipseV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, COLOR_FILLED);
            DrawEllipseLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, lineThickness, colorOutline);

            // Circle Sector
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE_SECTOR;
            DrawCircleSector((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawCircleSectorLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, lineThickness, colorOutline);

            // Ring
            posX = shapeOffset + shapePaddingX*ORDER_RING;
            DrawRing((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawRingLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, lineThickness, colorOutline);

            // Triangle
            posX = shapeOffset + shapePaddingX*ORDER_TRIANGLE;
            DrawTriangle((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                         (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                         (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                         COLOR_FILLED);
            DrawTriangleLinesEx((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                                (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                                (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                                lineThickness, colorOutline);

            // Polygon
            posX = shapeOffset + shapePaddingX*ORDER_POLYGON;
            DrawPoly((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, COLOR_FILLED);
            DrawPolyLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, lineThickness, colorOutline);

            // Group the shapes
            GuiGroupBox((Rectangle){ shapeOffset - BOX_SPACING, posY - BOX_SPACING, BOX_WIDTH, BOX_HEIGHT }, "Draw*LinesEx() with *world space* pixel thickness");
            // ----------------------------------------

            // ----------------------------------------
            // Draw*LinesEx() with screen space pixel thickness
            posY = shapeOffset + shapePaddingY*ORDER_LINES_EX_SCREEN;

            float constantThickness = lineThickness/camera.zoom;

            // Rectangle
            posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE;
            DrawRectangleRec((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, COLOR_FILLED);
            DrawRectangleLinesEx((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, constantThickness, colorOutline);

            // Rectangle Rounded
            posX = shapeOffset + shapePaddingX*ORDER_RECTANGLE_ROUNDED;
            DrawRectangleRounded((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, COLOR_FILLED);
            DrawRectangleRoundedLinesEx((Rectangle){ posX, posY, SHAPE_SIZE, SHAPE_SIZE }, rectangleRoundness, (int)rectangleSegments, constantThickness, colorOutline);

            // Circle
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE;
            DrawCircleV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, COLOR_FILLED);
            DrawCircleLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, constantThickness, colorOutline);

            // Ellipse
            posX = shapeOffset + shapePaddingX*ORDER_ELLIPSE;
            DrawEllipseV((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, COLOR_FILLED);
            DrawEllipseLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, radiusY, constantThickness, colorOutline);

            // Circle Sector
            posX = shapeOffset + shapePaddingX*ORDER_CIRCLE_SECTOR;
            DrawCircleSector((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawCircleSectorLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, radiusX, circleStartAngle, circleEndAngle, (int)circleSegments, constantThickness, colorOutline);

            // Ring
            posX = shapeOffset + shapePaddingX*ORDER_RING;
            DrawRing((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, COLOR_FILLED);
            DrawRingLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, ringInnerRadius, ringOuterRadius, circleStartAngle, circleEndAngle, (int)circleSegments, constantThickness, colorOutline);

            // Triangle
            posX = shapeOffset + shapePaddingX*ORDER_TRIANGLE;
            DrawTriangle((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                         (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                         (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                         COLOR_FILLED);
            DrawTriangleLinesEx((Vector2){ posX + triangleVertex0Offset.x, posY + triangleVertex0Offset.y },
                                (Vector2){ posX + triangleVertex1Offset.x, posY + triangleVertex1Offset.y },
                                (Vector2){ posX + triangleVertex2Offset.x, posY + triangleVertex2Offset.y },
                                constantThickness, colorOutline);

            // Polygon
            posX = shapeOffset + shapePaddingX*ORDER_POLYGON;
            DrawPoly((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, COLOR_FILLED);
            DrawPolyLinesEx((Vector2){ posX + SHAPE_SIZE/2.0f, posY + SHAPE_SIZE/2.0f }, (int)polygonSides, radiusX, 0.0f, constantThickness, colorOutline);

            // Group the shapes
            GuiGroupBox((Rectangle){ shapeOffset - BOX_SPACING, posY - BOX_SPACING, BOX_WIDTH, BOX_HEIGHT }, "Draw*LinesEx() with *screen space* pixel thickness");
            // ----------------------------------------

            EndMode2D();

            DrawRectangleRec(optionsBackground, Fade(DARKGRAY, 0.75f));

            DrawRectangleRec((Rectangle){ optionsBackground.x, 360, optionsBackground.width, 90 }, Fade(BLACK, 0.4f));
            DrawLineEx((Vector2){ optionsBackground.x, 360 }, (Vector2){ optionsBackground.x + optionsBackground.width, 360 }, 1.0f, BLACK);

            DrawText("Move with the mouse or WASD keys", (int)optionsBackground.x + 10, 370, 10, ORANGE);
            DrawText("Zoom with the mouse or UP and DOWN keys", (int)optionsBackground.x + 10, 390, 10, ORANGE);
            DrawText("Press C to reset position", (int)optionsBackground.x + 10, 410, 10, ORANGE);
            DrawText("Press Z to reset zoom", (int)optionsBackground.x + 10, 430, 10, ORANGE);

            DrawLineEx((Vector2){ optionsBackground.x, optionsBackground.y }, (Vector2){ optionsBackground.x, optionsBackground.y + optionsBackground.height }, 1.0f, BLACK);

            GuiSliderBar((Rectangle){ 605,  10, 150, 25 }, "Line Opacity"    , TextFormat("%d"  , (int)lineOpacity)      , &lineOpacity         ,  0     , 255);
            GuiSliderBar((Rectangle){ 605,  45, 150, 25 }, "Thickness"       , TextFormat("%.2f", lineThickness)         , &lineThickness       , -20.0f , 40.0f);
            GuiSliderBar((Rectangle){ 605,  80, 150, 25 }, "Rect Roundness"  , TextFormat("%.2f", rectangleRoundness)    , &rectangleRoundness  , -1.0f  , 2.0f);
            GuiSliderBar((Rectangle){ 605, 115, 150, 25 }, "Rect Segments"   , TextFormat("%d"  , (int)rectangleSegments), &rectangleSegments   , -1     , 100);
            GuiSliderBar((Rectangle){ 605, 150, 150, 25 }, "Ellipse Radius Y", TextFormat("%.2f", ellipseRadiusY)        , &ellipseRadiusY      , 0.0f   , 1.25f);
            GuiSliderBar((Rectangle){ 605, 185, 150, 25 }, "Start Angle"     , TextFormat("%.2f", circleStartAngle)      , &circleStartAngle    , -360.0f, 360.0f);
            GuiSliderBar((Rectangle){ 605, 220, 150, 25 }, "End Angle"       , TextFormat("%.2f", circleEndAngle)        , &circleEndAngle      , -360.0f, 360.0f);
            GuiSliderBar((Rectangle){ 605, 255, 150, 25 }, "Circle Segments" , TextFormat("%d"  , (int)circleSegments)   , &circleSegments      , -1     , 100);
            GuiSliderBar((Rectangle){ 605, 290, 150, 25 }, "Ring Radius"     , TextFormat("%.2f", ringInnerRadiusScale)  , &ringInnerRadiusScale, -1.0f  , 2.0f);
            GuiSliderBar((Rectangle){ 605, 325, 150, 25 }, "Poly Sides"      , TextFormat("%d"  , (int)polygonSides)     , &polygonSides        , -1     , 100);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
