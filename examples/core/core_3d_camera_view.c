/*******************************************************************************************
*
*   raylib [core] example - Camera View
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 6.0? (target)
*
*   Example contributed by IANN (@meisei4) and reviewed by Ramon Santamaria (@raysan5) and community
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 @meisei4
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdbool.h>

#define BAHAMA_BLUE CLITERAL(Color){ 0, 102, 153, 255 }
#define SUNFLOWER CLITERAL(Color){ 255, 204, 153, 255 }
#define ANAKIWA CLITERAL(Color){ 153, 204, 255, 255 }
#define MARINER CLITERAL(Color){ 51, 102, 204, 255 }
#define NEON_CARROT CLITERAL(Color){ 255, 153, 51, 255 }
#define EGGPLANT CLITERAL(Color){ 102, 68, 102, 255 }
#define HOPBUSH CLITERAL(Color){ 204, 102, 153, 255 }

typedef unsigned short Triangle[3];

enum Flags
{
    FLAG_PAUSE = 1u<<1,
    FLAG_JUGEMU = 1u<<2,
    FLAG_ORTHO = 1u<<3,
    GEN_CUBE = 1u<<4,
    GEN_SPHERE = 1u<<5,
    GEN_KNOT = 1u<<6
};

static unsigned int gflags = FLAG_JUGEMU | GEN_CUBE;

#define PAUSED() ((gflags & FLAG_PAUSE) != 0)
#define JUGEMU_MODE() ((gflags & FLAG_JUGEMU) != 0)
#define ORTHO_MODE() ((gflags & FLAG_ORTHO) != 0)
#define TOGGLE(K, F) do { if (IsKeyPressed(K)) { gflags ^= (F); } } while (0)

static unsigned int targetMesh = 0;
#define NUM_MODELS  3
#define CYCLE_MESH(K, I, F) do { if (IsKeyPressed(K)) { targetMesh = (I); gflags = (gflags & ~(GEN_CUBE|GEN_SPHERE|GEN_KNOT)) | (F); } } while (0)

static float fovyPerspective = 60.0f;
static float nearPlaneHeightOrthographic = 1.0f;
static Vector3 yAxis = { 0.0f, 1.0f, 0.0f };
static Vector3 modelPos = { 0.0f, 0.0f, 0.0f };
static Vector3 modelScale = { 1.0f, 1.0f, 1.0f };
static Vector3 mainPos = { 0.0f, 0.0f, 2.0f };
static Vector3 jugemuPos = { 3.0f, 1.0f, 3.0f };

static Vector3 *ComputeFrustumCorners(const Camera3D *main, float aspect, float near, float far);
static void OrbitSpace(Camera3D *jugemu, float dt);
static float OrthoBlendFactor(float dt);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - camera view");
    RenderTexture2D perspectiveCorrectRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    float near = 1.0f;
    float far = 3.0f;
    float aspect = (float)GetScreenWidth()/(float)GetScreenHeight();
    nearPlaneHeightOrthographic = 2.0f*near*tanf(DEG2RAD*fovyPerspective*0.5f);
    float meshRotation = 0.0f;

    Camera3D main = { 0 };
    main.position = mainPos;
    main.target = modelPos;
    main.up = yAxis;
    main.projection = (ORTHO_MODE())? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;
    main.fovy = (ORTHO_MODE())? nearPlaneHeightOrthographic: fovyPerspective;

    Camera3D jugemu = (Camera3D){ 0 };
    jugemu.position = jugemuPos;
    jugemu.target = modelPos;
    jugemu.up = yAxis;
    jugemu.fovy = fovyPerspective;
    jugemu.projection = CAMERA_PERSPECTIVE;

    Model models[NUM_MODELS] = { 0 };
    models[0] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    models[1] = LoadModelFromMesh(GenMeshSphere(0.5f, 8, 8));
    models[2] = LoadModelFromMesh(GenMeshKnot(1.0f, 1.0f, 16, 128));

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        aspect = (float)GetScreenWidth()/(float)GetScreenHeight();
        TOGGLE(KEY_SPACE, FLAG_PAUSE);
        TOGGLE(KEY_J, FLAG_JUGEMU);
        TOGGLE(KEY_O, FLAG_ORTHO);
        CYCLE_MESH(KEY_ONE, 0, GEN_CUBE);
        CYCLE_MESH(KEY_TWO, 1, GEN_SPHERE);
        CYCLE_MESH(KEY_THREE, 2, GEN_KNOT);

        OrthoBlendFactor(GetFrameTime());

        if (!PAUSED()) meshRotation -= 1.25f*GetFrameTime();

        OrbitSpace(&jugemu, GetFrameTime());
        main.projection = (ORTHO_MODE())? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;
        main.fovy = (ORTHO_MODE())? nearPlaneHeightOrthographic : fovyPerspective;

        Model *displayModel = &models[targetMesh];

        BeginTextureMode(perspectiveCorrectRenderTexture);
            ClearBackground(BLANK);
            BeginMode3D(main);
                DrawModelWiresEx(*displayModel, modelPos, yAxis, RAD2DEG*meshRotation, modelScale, MARINER);
            EndMode3D();
        EndTextureMode();

        Vector3 *corners = ComputeFrustumCorners(&main, aspect, near, far);
        Vector3 *nearPts = corners;
        Vector3 *farPts  = corners + 4;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            if (JUGEMU_MODE()) BeginMode3D(jugemu); else BeginMode3D(main);
                Vector3 depth = Vector3Normalize(Vector3Subtract(main.target, main.position));
                Vector3 right = Vector3Normalize(Vector3CrossProduct(depth, main.up));
                Vector3 up = Vector3Normalize(Vector3CrossProduct(right, depth));
                DrawLine3D(main.position, Vector3Add(main.position, right), NEON_CARROT);
                DrawLine3D(main.position, Vector3Add(main.position, up), NEON_CARROT);
                DrawLine3D(main.position, Vector3Add(main.position, depth), MARINER);

                DrawModelWiresEx(*displayModel, modelPos, yAxis, RAD2DEG*meshRotation, modelScale, MARINER);

                if (JUGEMU_MODE())
                {
                    // Draw Frustum wires
                    DrawLine3D(nearPts[0], nearPts[1], NEON_CARROT);
                    DrawLine3D(nearPts[1], nearPts[2], NEON_CARROT);
                    DrawLine3D(nearPts[2], nearPts[3], NEON_CARROT);
                    DrawLine3D(nearPts[3], nearPts[0], NEON_CARROT);

                    DrawLine3D(farPts[0], farPts[1], EGGPLANT);
                    DrawLine3D(farPts[1], farPts[2], EGGPLANT);
                    DrawLine3D(farPts[2], farPts[3], EGGPLANT);
                    DrawLine3D(farPts[3], farPts[0], EGGPLANT);

                    DrawLine3D(nearPts[0], farPts[0], HOPBUSH);
                    DrawLine3D(nearPts[1], farPts[1], HOPBUSH);
                    DrawLine3D(nearPts[2], farPts[2], HOPBUSH);
                    DrawLine3D(nearPts[3], farPts[3], HOPBUSH);

                    //Capture RenderTexture for near clip plane
                    rlSetTexture(perspectiveCorrectRenderTexture.texture.id);
                    rlBegin(RL_QUADS);
                        rlColor4ub(255, 255, 255, 255);
                        rlTexCoord2f(0.0f, 1.0f); rlVertex3f(nearPts[0].x, nearPts[0].y, nearPts[0].z);
                        rlTexCoord2f(0.0f, 0.0f); rlVertex3f(nearPts[3].x, nearPts[3].y, nearPts[3].z);
                        rlTexCoord2f(1.0f, 0.0f); rlVertex3f(nearPts[2].x, nearPts[2].y, nearPts[2].z);
                        rlTexCoord2f(1.0f, 1.0f); rlVertex3f(nearPts[1].x, nearPts[1].y, nearPts[1].z);
                    rlEnd();
                    rlSetTexture(0);
                }

            EndMode3D();

            DrawText("[1]: CUBE [2]: SPHERE [3]: KNOT", 12, 12, 20, NEON_CARROT);
            DrawText("ARROWS: MOVE | SPACEBAR: PAUSE", 12, 38, 20, NEON_CARROT);
            DrawText("W S : ZOOM ", 12, 64, 20, NEON_CARROT);
            DrawText((targetMesh == 0)? "GEN_CUBE" : (targetMesh == 1)? "GEN_SPHERE" : "GEN_KNOT", 12, 205, 20, NEON_CARROT);
            DrawText("LENS [ O ]:", 510, 366, 20, SUNFLOWER);
            DrawText((ORTHO_MODE())? "ORTHOGRAPHIC" : "PERSPECTIVE", 630, 366, 20, (ORTHO_MODE())? BAHAMA_BLUE : ANAKIWA);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(models[0]);
    UnloadModel(models[1]);
    UnloadModel(models[2]);
    if (perspectiveCorrectRenderTexture.id) UnloadRenderTexture(perspectiveCorrectRenderTexture);
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}

static Vector3 *ComputeFrustumCorners(const Camera3D *main, float aspect, float near, float far)
{

    Vector3 depth = Vector3Normalize(Vector3Subtract(main->target, main->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(depth, main->up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, depth));

    float perspHalfHNear = near*tanf(DEG2RAD*fovyPerspective*0.5f);
    float orthoHalfH = 0.5f*nearPlaneHeightOrthographic;
    float halfHNear = Lerp(perspHalfHNear, orthoHalfH, OrthoBlendFactor(0.0f));
    float halfWNear = halfHNear*aspect;

    float perspHalfHFar = far*tanf(DEG2RAD*fovyPerspective*0.5f);
    float halfHFar = Lerp(perspHalfHFar, orthoHalfH, OrthoBlendFactor(0.0f));
    float halfWFar = halfHFar*aspect;

    Vector3 centerNear = Vector3Add(main->position, Vector3Scale(depth, near));
    Vector3 centerFar = Vector3Add(main->position, Vector3Scale(depth, far));

    static Vector3 corners[8];
    corners[0] = Vector3Add(centerNear, Vector3Add(Vector3Scale(up,  halfHNear), Vector3Scale(right, -halfWNear)));
    corners[1] = Vector3Add(centerNear, Vector3Add(Vector3Scale(up,  halfHNear), Vector3Scale(right,  halfWNear)));
    corners[2] = Vector3Add(centerNear, Vector3Add(Vector3Scale(up, -halfHNear), Vector3Scale(right,  halfWNear)));
    corners[3] = Vector3Add(centerNear, Vector3Add(Vector3Scale(up, -halfHNear), Vector3Scale(right, -halfWNear)));

    corners[4] = Vector3Add(centerFar, Vector3Add(Vector3Scale(up,  halfHFar), Vector3Scale(right, -halfWFar)));
    corners[5] = Vector3Add(centerFar, Vector3Add(Vector3Scale(up,  halfHFar), Vector3Scale(right,  halfWFar)));
    corners[6] = Vector3Add(centerFar, Vector3Add(Vector3Scale(up, -halfHFar), Vector3Scale(right,  halfWFar)));
    corners[7] = Vector3Add(centerFar, Vector3Add(Vector3Scale(up, -halfHFar), Vector3Scale(right, -halfWFar)));

    return corners;
}

static void OrbitSpace(Camera3D *jugemu, float dt)
{
    float radius = Vector3Length(jugemu->position);
    float azimuth = atan2f(jugemu->position.z, jugemu->position.x);
    float horizontalRadius = sqrtf(jugemu->position.x*jugemu->position.x + jugemu->position.z*jugemu->position.z);
    float elevation = atan2f(jugemu->position.y, horizontalRadius);
    if (IsKeyDown(KEY_LEFT)) azimuth += 1.0f*dt;
    if (IsKeyDown(KEY_RIGHT)) azimuth -= 1.0f*dt;
    if (IsKeyDown(KEY_UP)) elevation += 1.0f*dt;
    if (IsKeyDown(KEY_DOWN)) elevation -= 1.0f*dt;
    if (IsKeyDown(KEY_W)) radius -= 1.0f*dt;
    if (IsKeyDown(KEY_S)) radius += 1.0f*dt;
    elevation = Clamp(elevation, -PI/2 + 0.1f, PI/2 - 0.1f);
    jugemu->position.x = Clamp(radius, 0.25f, 10.0f)*cosf(elevation)*cosf(azimuth);
    jugemu->position.y = Clamp(radius, 0.25f, 10.0f)*sinf(elevation);
    jugemu->position.z = Clamp(radius, 0.25f, 10.0f)*cosf(elevation)*sinf(azimuth);
}

static float OrthoBlendFactor(float dt)
{
    static float blend = 0.0f;
    if (dt > 0.0f) blend = Clamp(blend + ((ORTHO_MODE())? 1.0f : -1.0f)*5.0f*dt, 0.0f, 1.0f);
    return blend;
}