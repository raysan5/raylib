/*******************************************************************************************
*
*   raylib [models] example - rlgl module usage with push/pop matrix transformations
*
*   This example uses [rlgl] module funtionality (pseudo-OpenGL 1.1 style coding)
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2018 Ramon Santamaria (@raysan5)
*   Copyright (c) 2019 Aldrin Martoq (@aldrinmartoq)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#define MAX_BODY_CHILDREN 10
float rotationSpeed = 0.2;

// A celestial body that has children bodies orbiting around
typedef struct Body {
    const char *label;                          // label of the body, for ex: moon
    float radius;                               // object radius
    float orbitRadius;                          // orbit average radius
    float orbitPeriod;                          // time the body takes to do a full loop
    Color color;                                // color of the body
    float orbitPosition;                        // current orbit position
    Vector2 labelPosition;                      // label position in screen
    struct Body *children[MAX_BODY_CHILDREN];   // children array
    int childrenCount;                          // children count
} Body;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
Body CreateBody(float radius, float orbitRadius, float orbitPeriod, Color color, const char *label); // Initializes a new Body with the given parameters
void AddBodyChildren(Body *parent, Body *children); // Add a children body to the parent body
void DrawSphereBasic(Color color);                  // Draw sphere without any matrix transformation
void DrawBody(Body *body, Camera *camera);          // Draw body and its children, updating labelPosition
void DrawLabels(Body *body);                        // Draw body label and its children labels

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1024;
    const int screenHeight = 768;
    const char *text;
    bool gridEnabled = true;
    bool helpEnabled = false;
    bool labelEnabled = true;
    bool cameraParametersEnabled = true;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - rlgl module usage with push/pop matrix transformations");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 8.0f, 8.0f, 8.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FREE);

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Create Bodies
    Body sun        = CreateBody(0.2,     0.0,     0, GOLD,     "sun");
    Body moon       = CreateBody(0.05,  0.200,    24, GRAY,     "moon");
    Body mercury    = CreateBody(0.05,  0.396,    90, GRAY,     "mercury");
    Body venus      = CreateBody(0.05,  0.723,   210, MAGENTA,  "venus");
    Body earth      = CreateBody(0.05,  1.000,   365, BLUE,     "earth");
    Body mars       = CreateBody(0.05,  1.523,   690, RED,      "mars");
    Body jupiter    = CreateBody(0.05,  5.200,  4260, BROWN,    "jupiter");
    Body saturn     = CreateBody(0.05,  9.532, 10620, GREEN,    "saturn");
    Body uranus     = CreateBody(0.05, 19.180, 30270, SKYBLUE,  "uranus");
    Body neptune    = CreateBody(0.05, 30.056, 59370, PURPLE,   "neptune");
    Body pluto      = CreateBody(0.05, 39.463, 89310, DARKGREEN, "pluto");

    AddBodyChildren(&sun, &mercury);
    AddBodyChildren(&sun, &venus);
    AddBodyChildren(&sun, &earth);
    AddBodyChildren(&sun, &mars);
    AddBodyChildren(&sun, &jupiter);
    AddBodyChildren(&sun, &saturn);
    AddBodyChildren(&sun, &uranus);
    AddBodyChildren(&sun, &neptune);
    AddBodyChildren(&sun, &pluto);

    AddBodyChildren(&earth, &moon);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);

        if (IsKeyPressed(KEY_G)) {
          gridEnabled = !gridEnabled;
        }

        if (IsKeyPressed(KEY_H)) {
            helpEnabled = !helpEnabled;
        }

        if (IsKeyPressed(KEY_L)) {
            labelEnabled = !labelEnabled;
        }

        if (IsKeyPressed(KEY_P)) {
          cameraParametersEnabled = !cameraParametersEnabled;
        }

        if (IsKeyPressed(KEY_LEFT)) {
            rotationSpeed -= 0.1;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            rotationSpeed += 0.1;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawBody(&sun, &camera);

                // Some reference elements (not affected by previous matrix transformations)
                if (gridEnabled) {
                    DrawGrid(80, 1.0f);
                }

            EndMode3D();

            if (labelEnabled) {
                DrawLabels(&sun);
            }

            DrawText("FULL SOLAR SYSTEM", 400, 10, 20, MAROON);
            text = FormatText("SPEED: %2.2f", rotationSpeed);
            DrawText(text, 1024 / 2 - MeasureText(text, 20) / 2, 30, 20, MAROON);

            if (cameraParametersEnabled) {
                text = FormatText("Camera\nposition: [%3.3f, %3.3f, %3.3f]\ntarget: [%3.3f, %3.3f, %3.3f]\nup: [%3.3f, %3.3f, %3.3f]",
                                    camera.position.x, camera.position.y, camera.position.z,
                                    camera.target.x, camera.target.y, camera.target.z,
                                    camera.up.x, camera.up.y, camera.up.z);
                DrawText(text, 10, 50, 20, MAROON);
            }

            if (helpEnabled) {
                DrawText("Keys:\n- [g] toggle grid\n- [h] toggle help\n- [l] toggle labels\n- [p] toggle camera parameters\n- [left/right arrows] increase/decrease speed by 0.1", 200, 200, 20, MAROON);
            } else {
                DrawText("press [h] for help", 1016 - MeasureText("press [h] for help", 20), 740, 20, MAROON);
            }
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

//--------------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//--------------------------------------------------------------------------------------------

// Creates a new body
Body CreateBody(float radius, float orbitRadius, float orbitPeriod, Color color, const char *label)
{
    Body body;

    body.label = label;
    body.radius = radius;
    body.orbitRadius = orbitRadius;
    body.orbitPeriod = orbitPeriod;
    body.color = color;
    body.childrenCount = 0;
    body.orbitPosition = 0.0;
    return body;
}

void AddBodyChildren(Body *parent, Body *children) {
    if (parent->childrenCount >= MAX_BODY_CHILDREN) {
        TraceLog(LOG_ERROR, "BODY HAS TOO MANY CHILDREN");
    } else {
        parent->children[parent->childrenCount] = children;
        parent->childrenCount++;
    }
}

// Draw body and its children
void DrawBody(Body *body, Camera *camera)
{
    rlPushMatrix();
        rlScalef(body->radius, body->radius, body->radius);     // Scale Sun
        DrawSphereBasic(body->color);                           // Draw the body
    rlPopMatrix();

    body->labelPosition = GetWorldToScreen((Vector3) { body->orbitRadius, body->radius, 0.0 }, *camera);

    for (int i = 0; i < body->childrenCount; i++) {
        Body *child = body->children[i];
        child->orbitPosition += rotationSpeed * 360 / child->orbitPeriod;
        rlPushMatrix();
            rlRotatef(child->orbitPosition, 0.0, 1.0, 0.0);
            rlTranslatef(child->orbitRadius, 0.0, 0.0);
            rlRotatef(-child->orbitPosition, 0.0, 1.0, 0.0);

            DrawBody(child, camera);
        rlPopMatrix();

        DrawCircle3D((Vector3){ 0.0f, 0.0f, 0.0f }, child->orbitRadius, (Vector3){ 1.0f, 0.0f, 0.0f }, 90.0f, child->color);
    }
}

// Draw body label and its children labels
void DrawLabels(Body *body)
{
    DrawText(body->label, body->labelPosition.x - MeasureText(body->label, 20) / 2, body->labelPosition.y, 20, BLACK);

    for (int i = 0; i < body->childrenCount; i++) {
        Body *child = body->children[i];

        DrawLabels(child);
    }
}


// Draw sphere without any matrix transformation
// NOTE: Sphere is drawn in world position ( 0, 0, 0 ) with radius 1.0f
void DrawSphereBasic(Color color)
{
    int rings = 16;
    int slices = 16;

    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        for (int i = 0; i < (rings + 2); i++)
        {
            for (int j = 0; j < slices; j++)
            {
                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(DEG2RAD*((j+1)*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*(i))),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(DEG2RAD*((j+1)*360/slices)));
                rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                           sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                           cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
            }
        }
    rlEnd();
}
