/*******************************************************************************************
*
*   raylib [core] example - Input Gestures for Web
*
*   Example complexity rating: [★★★☆] 3/4
* 
*   Example originally created with raylib 5.5
*
*   Example contributed by Agnis Aldins (@nezvers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Agnis Aldins (@nezvers)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

//#define PLATFORM_WEB

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif


/* Movement constants */
#define GRAVITY 32.f
#define MAX_SPEED 20.f
#define CROUCH_SPEED 5.f
#define JUMP_FORCE 12.f
#define MAX_ACCEL 150.f
/* Grounded drag */
#define FRICTION 0.86f
/* Increasing air drag, increases strafing speed */
#define AIR_DRAG 0.98f
/* Responsiveness for turning movement direction to looked direction */
#define CONTROL 15.f
#define CROUCH_HEIGHT 0.f
#define STAND_HEIGHT 1.f
#define BOTTOM_HEIGHT 0.5f

#define NORMALIZE_INPUT 0

typedef struct {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool isGrounded;
    Sound soundJump;
}Body;

const int screenWidth = 800;
const int screenHeight = 450;
Vector2 sensitivity = { 0.001f, 0.001f };

Body player;
Camera camera;
Vector2 lookRotation = { 0 };
float headTimer;
float walkLerp;
float headLerp;
Vector2 lean;

void UpdateDrawFrame(void);     // Update and Draw one frame

void DrawLevel();

void UpdateCameraAngle(Camera* camera, Vector2* rot, float headTimer, float walkLerp, Vector2 lean);

void UpdateBody(Body* body, float rot, char side, char forward, bool jumpPressed, bool crouchHold);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Raylib Quake-like controller");
    InitAudioDevice();

    player = (Body){ Vector3Zero(), Vector3Zero(), Vector3Zero(), false, LoadSound("resources/huh_jump.wav")};
    camera = (Camera){ 0 };
    camera.fovy = 60.f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    lookRotation = Vector2Zero();
    headTimer = 0.f;
    walkLerp = 0.f;
    headLerp = STAND_HEIGHT;
    lean = Vector2Zero();

    camera.position = (Vector3){
            player.position.x,
            player.position.y + (BOTTOM_HEIGHT + headLerp),
            player.position.z,
    };
    UpdateCameraAngle(&camera, &lookRotation, headTimer, walkLerp, lean);
    
    DisableCursor();  // Limit cursor to relative movement inside the window

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSound(player.soundJump);
    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------

    Vector2 mouse_delta = GetMouseDelta();
    lookRotation.x -= mouse_delta.x * sensitivity.x;
    lookRotation.y += mouse_delta.y * sensitivity.y;

    char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
    UpdateBody(&player, lookRotation.x, sideway, forward, IsKeyPressed(KEY_SPACE), crouching);

    float delta = GetFrameTime();
    headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 20.f * delta);
    camera.position = (Vector3){
        player.position.x,
        player.position.y + (BOTTOM_HEIGHT + headLerp),
        player.position.z,
    };

    if (player.isGrounded && (forward != 0 || sideway != 0)) {
        headTimer += delta * 3.f;
        walkLerp = Lerp(walkLerp, 1.f, 10.f * delta);
        camera.fovy = Lerp(camera.fovy, 55.f, 5.f * delta);
    }
    else {
        walkLerp = Lerp(walkLerp, 0.f, 10.f * delta);
        camera.fovy = Lerp(camera.fovy, 60.f, 5.f * delta);
    }

    lean.x = Lerp(lean.x, sideway * 0.02f, 10.f * delta);
    lean.y = Lerp(lean.y, forward * 0.015f, 10.f * delta);

    UpdateCameraAngle(&camera, &lookRotation, headTimer, walkLerp, lean);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawLevel();

    EndMode3D();

    // Draw info box
    DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(5, 5, 330, 100, BLUE);

    DrawText("Camera controls:", 15, 15, 10, BLACK);
    DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
    DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
    DrawText(TextFormat("- Velocity Len: (%06.3f)", Vector2Length((Vector2) { player.velocity.x, player.velocity.z })), 15, 60, 10, BLACK);


    EndDrawing();
    //----------------------------------------------------------------------------------
}

void UpdateBody(Body* body, float rot, char side, char forward, bool jumpPressed, bool crouchHold) 
{
    Vector2 input = (Vector2){ (float)side, (float)-forward };
#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if (side != 0 & forward != 0) 
    {
        input = Vector2Normalize(input);
    }
#endif

    float delta = GetFrameTime();

    if (!body->isGrounded) 
    {
        body->velocity.y -= GRAVITY * delta;
    }
    if (body->isGrounded && jumpPressed) 
    {
        body->velocity.y = JUMP_FORCE;
        body->isGrounded = false;
        SetSoundPitch(body->soundJump, 1.f + (GetRandomValue(-100, 100) * 0.001));
        PlaySound(body->soundJump);
    }

    Vector3 front_vec = (Vector3){ sin(rot), 0.f, cos(rot) };
    Vector3 right_vec = (Vector3){ cos(-rot), 0.f, sin(-rot) };

    Vector3 desired_dir = (Vector3){
        input.x * right_vec.x + input.y * front_vec.x,
        0.f,
        input.x * right_vec.z + input.y * front_vec.z,
    };

    body->dir = Vector3Lerp(body->dir, desired_dir, CONTROL * delta);

    float decel = body->isGrounded ? FRICTION : AIR_DRAG;
    Vector3 hvel = (Vector3){
        body->velocity.x * decel,
        0.f,
        body->velocity.z * decel
    };

    float hvel_length = Vector3Length(hvel); // a.k.a. magnitude
    if (hvel_length < MAX_SPEED * 0.01f) {
        hvel = (Vector3){ 0 };
    }

    /* This is what creates strafing */
    float speed = Vector3DotProduct(hvel, body->dir);

    /*
    Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
    a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
    More info here: https://youtu.be/v3zT3Z5apaM?t=165
    */
    float max_speed = crouchHold ? CROUCH_SPEED : MAX_SPEED;
    float accel = Clamp(max_speed - speed, 0.f, MAX_ACCEL * delta);
    hvel.x += body->dir.x * accel;
    hvel.z += body->dir.z * accel;

    body->velocity.x = hvel.x;
    body->velocity.z = hvel.z;

    body->position.x += body->velocity.x * delta;
    body->position.y += body->velocity.y * delta;
    body->position.z += body->velocity.z * delta;

    /* Fancy collision system against "THE FLOOR" */
    if (body->position.y <= 0.f) 
    {
        body->position.y = 0.f;
        body->velocity.y = 0.f;
        body->isGrounded = true; // <= enables jumping
    }
}

void UpdateCameraAngle(Camera* camera, Vector2* rot, float headTimer, float walkLerp, Vector2 lean)
{
    const Vector3 up = (Vector3){ 0.f, 1.f, 0.f };
    const Vector3 targetOffset = (Vector3){ 0.f, 0.f, -1.f };

    /* Left & Right */
    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, rot->x);

    // Clamp view up
    float maxAngleUp = Vector3Angle(up, yaw);
    maxAngleUp -= 0.001f; // avoid numerical errors
    if ( -(rot->y) > maxAngleUp) { rot->y = -maxAngleUp; }

    // Clamp view down
    float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
    maxAngleDown *= -1.0f; // downwards angle is negative
    maxAngleDown += 0.001f; // avoid numerical errors
    if ( -(rot->y) < maxAngleDown) { rot->y = -maxAngleDown; }

    /* Up & Down */
    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

    // Rotate view vector around right axis
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, -rot->y - lean.y);

    // Head animation
    // Rotate up direction around forward axis
    float _sin = sin(headTimer * PI);
    float _cos = cos(headTimer * PI);
    const float stepRotation = 0.01f;
    camera->up = Vector3RotateByAxisAngle(up, pitch, _sin * stepRotation + lean.x);

    /* BOB */
    const float bobSide = 0.1f;
    const float bobUp = 0.15f;
    Vector3 bobbing = Vector3Scale(right, _sin * bobSide);
    bobbing.y = fabsf(_cos * bobUp);
    camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));

    camera->target = Vector3Add(camera->position, pitch);
}


void DrawLevel() 
{
    const int floorExtent = 25;
    const float tileSize = 5.f;
    const Color tileColor1 = (Color){ 150, 200, 200, 255 };
    // Floor tiles
    for (int y = -floorExtent; y < floorExtent; y++) 
    {
        for (int x = -floorExtent; x < floorExtent; x++) 
        {
            if ((y & 1) && (x & 1)) 
            {
                DrawPlane((Vector3) { x * tileSize, 0.f, y * tileSize}, 
                (Vector2) {tileSize, tileSize}, tileColor1);
            }
            else if(!(y & 1) && !(x & 1)) 
            {
                DrawPlane((Vector3) { x * tileSize, 0.f, y * tileSize},
                (Vector2) {tileSize, tileSize}, LIGHTGRAY);
            }
        }
    }
    
    const Vector3 towerSize = (Vector3){ 16.f, 32.f, 16.f };
    const Color towerColor = (Color){ 150, 200, 200, 255 };

    Vector3 towerPos = (Vector3){ 16.f, 16.f, 16.f };
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.z *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    towerPos.x *= -1;
    DrawCubeV(towerPos, towerSize, towerColor);
    DrawCubeWiresV(towerPos, towerSize, DARKBLUE);

    // Red sun
    DrawSphere((Vector3) { 300.f, 300.f, 0.f }, 100.f, (Color) { 255, 0, 0, 255 });
}
