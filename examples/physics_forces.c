/*******************************************************************************************
*
*   raylib [physac] example - Forces
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Victor Fisac and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "math.h"

#define FORCE_AMOUNT    5.0f
#define FORCE_RADIUS    150
#define LINE_LENGTH     100

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [physac] example - forces");
    InitPhysics((Vector2){ 0.0f, -9.81f/2 });      // Initialize physics module
    
    SetTargetFPS(60);
    
    // Global variables
    Vector2 mousePosition;
    bool isDebug = false;
    
    // Create rectangle physic objects
    PhysicObject *rectangles[3];
    for (int i = 0; i < 3; i++)
    {
        rectangles[i] = CreatePhysicObject((Vector2){ screenWidth/4*(i+1), (((i % 2) == 0) ? (screenHeight/3) : (screenHeight/1.5f)) }, 0.0f, (Vector2){ 50, 50 });
        rectangles[i]->rigidbody.enabled = true;       // Enable physic object rigidbody behaviour
        rectangles[i]->rigidbody.friction = 0.1f;
    }
    
    // Create circles physic objects
    PhysicObject *circles[3];
    for (int i = 0; i < 3; i++)
    {
        circles[i] = CreatePhysicObject((Vector2){ screenWidth/4*(i+1), (((i % 2) == 0) ? (screenHeight/1.5f) : (screenHeight/4)) }, 0.0f, (Vector2){ 0, 0 });
        circles[i]->rigidbody.enabled = true;       // Enable physic object rigidbody behaviour
        circles[i]->rigidbody.friction = 0.1f;
        circles[i]->collider.type = COLLIDER_CIRCLE;
        circles[i]->collider.radius = 25;
    }
    
    // Create walls physic objects
    PhysicObject *leftWall = CreatePhysicObject((Vector2){ -25, screenHeight/2 }, 0.0f, (Vector2){ 50, screenHeight });
    PhysicObject *rightWall = CreatePhysicObject((Vector2){ screenWidth + 25, screenHeight/2 }, 0.0f, (Vector2){ 50, screenHeight });
    PhysicObject *topWall = CreatePhysicObject((Vector2){ screenWidth/2, -25 }, 0.0f, (Vector2){ screenWidth, 50 });
    PhysicObject *bottomWall = CreatePhysicObject((Vector2){ screenWidth/2, screenHeight + 25 }, 0.0f, (Vector2){ screenWidth, 50 });
    
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdatePhysics();    // Update all created physic objects
        
        // Update mouse position value
        mousePosition = GetMousePosition();
        
        // Check force input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ApplyForceAtPosition(mousePosition, FORCE_AMOUNT, FORCE_RADIUS);
        
        // Check reset input
        if (IsKeyPressed('R'))
        {
            // Reset rectangle physic objects positions
            for (int i = 0; i < 3; i++)
            {
                rectangles[i]->transform.position = (Vector2){ screenWidth/4*(i+1) - rectangles[i]->transform.scale.x/2, (((i % 2) == 0) ? (screenHeight/3) : (screenHeight/1.5f)) - rectangles[i]->transform.scale.y/2 };
                rectangles[i]->rigidbody.velocity =(Vector2){ 0.0f, 0.0f };
            }
            
            // Reset circles physic objects positions
            for (int i = 0; i < 3; i++)
            {
                circles[i]->transform.position = (Vector2){ screenWidth/4*(i+1), (((i % 2) == 0) ? (screenHeight/1.5f) : (screenHeight/4)) };
                circles[i]->rigidbody.velocity =(Vector2){ 0.0f, 0.0f };
            }
        }
        
        // Check debug switch input
        if (IsKeyPressed('P')) isDebug = !isDebug;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw rectangles
            for (int i = 0; i < 3; i++)
            {
                // Convert transform values to rectangle data type variable
                DrawRectangleRec(TransformToRectangle(rectangles[i]->transform), RED);
                if (isDebug) DrawRectangleLines(rectangles[i]->collider.bounds.x, rectangles[i]->collider.bounds.y, rectangles[i]->collider.bounds.width, rectangles[i]->collider.bounds.height, GREEN);

                // Draw force radius
                DrawCircleLines(mousePosition.x, mousePosition.y, FORCE_RADIUS, BLACK);
                
                // Draw direction line
                if (CheckCollisionPointCircle((Vector2){ rectangles[i]->transform.position.x + rectangles[i]->transform.scale.x/2, rectangles[i]->transform.position.y + rectangles[i]->transform.scale.y/2 }, mousePosition, FORCE_RADIUS))
                {
                    Vector2 direction = { rectangles[i]->transform.position.x + rectangles[i]->transform.scale.x/2 - mousePosition.x, rectangles[i]->transform.position.y + rectangles[i]->transform.scale.y/2 - mousePosition.y };
                    float angle = atan2l(direction.y, direction.x);
                    
                    DrawLineV((Vector2){ rectangles[i]->transform.position.x + rectangles[i]->transform.scale.x/2, rectangles[i]->transform.position.y + rectangles[i]->transform.scale.y/2 },
                    (Vector2){ rectangles[i]->transform.position.x + rectangles[i]->transform.scale.x/2 + (cos(angle)*LINE_LENGTH), rectangles[i]->transform.position.y + rectangles[i]->transform.scale.y/2 + (sin(angle)*LINE_LENGTH) }, BLACK);
                }
            }
            
            // Draw circles
            for (int i = 0; i < 3; i++)
            {
                DrawCircleV(circles[i]->transform.position, circles[i]->collider.radius, BLUE);
                if (isDebug) DrawCircleLines(circles[i]->transform.position.x, circles[i]->transform.position.y, circles[i]->collider.radius, GREEN);

                // Draw force radius
                DrawCircleLines(mousePosition.x, mousePosition.y, FORCE_RADIUS, BLACK);
                
                // Draw direction line
                if (CheckCollisionPointCircle((Vector2){ circles[i]->transform.position.x, circles[i]->transform.position.y }, mousePosition, FORCE_RADIUS))
                {
                    Vector2 direction = { circles[i]->transform.position.x - mousePosition.x, circles[i]->transform.position.y - mousePosition.y };
                    float angle = atan2l(direction.y, direction.x);
                    
                    DrawLineV((Vector2){ circles[i]->transform.position.x, circles[i]->transform.position.y },
                    (Vector2){ circles[i]->transform.position.x + (cos(angle)*LINE_LENGTH), circles[i]->transform.position.y + (sin(angle)*LINE_LENGTH) }, BLACK);
                }
            }
            
            // Draw help messages
            DrawText("Use LEFT MOUSE BUTTON to apply a force", screenWidth/2 - MeasureText("Use LEFT MOUSE BUTTON to apply a force", 20)/2, screenHeight*0.075f, 20, LIGHTGRAY);
            DrawText("Use R to reset objects position", screenWidth/2 - MeasureText("Use R to reset objects position", 20)/2, screenHeight*0.875f, 20, GRAY);    

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    ClosePhysics();       // Unitialize physics module
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}