/*******************************************************************************************
*
*   raylib [core] example - Fixed-function didactic
*
*   RESOURCES:
*    - https://en.wikipedia.org/wiki/Fixed-function_(computer_graphics)
*    - https://en.wikipedia.org/wiki/Texture_mapping#Perspective_correctness
*    - Etay Meiri (OGLDEV) Perspective Projection Tutorial: https://www.youtube.com/watch?v=LhQ85bPCAJ8
*    - Keenan Crane Computer Graphics (CMU 15-462/662): https://www.youtube.com/watch?v=_4Q4O2Kgdo4
*
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
// TODO list:
//  1. add proper clipping to the target meshes to show intuition there (e.g. move mesh out of clip planes or allow moving the main camera's target away from the meshes)
//  2. improve didactic annotations (ideally with spatial labeling rather than simple flat screen overlay)
//  3. improve code didactic, code should read in order of fixed function staging... difficult but long term goal...
//  4. add scripted toggling/navigation of ordered fixed function staging visualization (a "play button"-like thing)
//  5. add some sort of ghosting effect between fixed function stages, to emphasize previous stages perhaps)
//  6. general improvements to toggling and space navigation

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BAHAMA_BLUE CLITERAL(Color){ 0, 102, 153, 255 }
#define SUNFLOWER CLITERAL(Color){ 255, 204, 153, 255 }
#define PALE_CANARY CLITERAL(Color){ 255, 255, 153, 255 }
#define ANAKIWA CLITERAL(Color){ 153, 204, 255, 255 }
#define MARINER CLITERAL(Color){ 51, 102, 204, 255 }
#define NEON_CARROT CLITERAL(Color){ 255, 153, 51, 255 }
#define EGGPLANT CLITERAL(Color){ 102, 68, 102, 255 }
#define HOPBUSH CLITERAL(Color){ 204, 102, 153, 255 }
#define LILAC CLITERAL(Color){ 204, 153, 204, 255 }
#define RED_DAMASK CLITERAL(Color){ 221, 102, 68, 255 }
#define CHESTNUT_ROSE CLITERAL(Color){ 204, 102, 102, 255 }

typedef unsigned short Triangle[3];

enum Flags
{
    FLAG_NDC = 1u<<0,
    FLAG_REFLECT_Y = 1u<<1,
    FLAG_ASPECT = 1u<<2,
    FLAG_PERSPECTIVE_CORRECT = 1u<<3,
    FLAG_PAUSE = 1u<<4,
    FLAG_COLOR_MODE = 1u<<5,
    FLAG_TEXTURE_MODE = 1u<<6
};

static unsigned int gflags = FLAG_ASPECT | FLAG_COLOR_MODE;

#define NDC_SPACE() ((gflags & FLAG_NDC) != 0)
#define REFLECT_Y() ((gflags & FLAG_REFLECT_Y) != 0)
#define ASPECT_CORRECT() ((gflags & FLAG_ASPECT) != 0)
#define PERSPECTIVE_CORRECT() ((gflags & FLAG_PERSPECTIVE_CORRECT) != 0)
#define PAUSED() ((gflags & FLAG_PAUSE) != 0)
#define COLOR_MODE() ((gflags & FLAG_COLOR_MODE) != 0)
#define TEXTURE_MODE() ((gflags & FLAG_TEXTURE_MODE) != 0)
#define TOGGLE(K, F) do { if (IsKeyPressed(K)) { gflags ^= (F); } } while (0)

static int fontSize = 20;
static float angularVelocity = 1.25f;
static float fovy = 60.0f;
static float blendScalar = 5.0f;
static Vector3 yAxis = { 0.0f, 1.0f, 0.0f };
static Vector3 modelPos = { 0.0f, 0.0f, 0.0f };
static Vector3 modelScale = { 1.0f, 1.0f, 1.0f };
static Vector3 mainPos = { 0.0f, 0.0f, 2.0f };
static Vector3 jugemuPosIso = { 3.0f, 1.0f, 3.0f };

static void BasisVector(Camera3D *main, Vector3 *depthOut, Vector3 *rightOut, Vector3 *upOut);
static void WorldToNDCSpace(Camera3D *main, float aspect, float near, float far, Model *world, Model *ndc, float rotation);

static void DrawModelFilled(Model *model, Texture2D texture, float rotation);
static void DrawModelWiresAndPoints(Model *model, float rotation);
static void DrawNearPlanePoints(Camera3D *main, float aspect, float near, Model *nearPlanePointsModel, Mesh *mesh, float rotation);

static void UpdateSpatialFrame(Camera3D *main, float aspect, float near, float far, Mesh *spatialFrame);
static void DrawSpatialFrame(Mesh spatialFrame);

static void PerspectiveIncorrectCapture(Camera3D *main, float aspect, float near, Mesh *mesh, Texture2D meshTexture, float rotation);
static void PerspectiveCorrectCapture(Camera3D *main, Model *model, Texture2D meshTexture, Texture2D *perspectiveCorrectTexture, float rotation);

static void AlphaMaskPunchOut(Image *rgba, Image *mask, unsigned char threshold);
static void FillVertexColors(Mesh *mesh);
static void OrbitSpace(Camera3D *jugemu, float dt);
static Vector3 AspectCorrectAndReflectNearPlane(Vector3 intersect, Vector3 center, Vector3 right, Vector3 up, float xAspect, float yReflect);
static Vector3 TranslateRotateScale(int inverse, Vector3 coordinate, Vector3 pos, Vector3 scale, float rotation);
static Vector3 Intersect(Camera3D *main, float near, Vector3 worldCoord);
static float SpaceBlendFactor(float dt);
static float AspectBlendFactor(float dt);
static float ReflectBlendFactor(float dt);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - fixed function didactic");

    float aspect = (float)GetScreenWidth()/(float)GetScreenHeight();
    float meshRotation = 0.0f;

    Camera3D main = { 0 };
    main.position = mainPos;
    main.target = modelPos;
    main.up = yAxis;
    main.fovy = fovy;
    main.projection = CAMERA_PERSPECTIVE;

    Camera3D jugemu = (Camera3D){ 0 };
    jugemu.position = jugemuPosIso;
    jugemu.target = modelPos;
    jugemu.up = yAxis;
    jugemu.fovy = fovy;
    jugemu.projection = CAMERA_PERSPECTIVE;

    // Model worldModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    // Image textureImage = GenImageChecked(4, 4, 1, 1, BLACK, WHITE);

    // Model worldModel = LoadModelFromMesh(GenMeshSphere(0.5f, 8, 8));
    // Image textureImage = GenImageChecked(16, 16, 1, 1, BLACK, WHITE);

    // Model worldModel = LoadModelFromMesh(GenMeshKnot(1.0f, 1.0f, 8, 64));
    Model worldModel = LoadModelFromMesh(GenMeshKnot(1.0f, 1.0f, 16, 128));
    Image textureImage = GenImageChecked(32, 32, 1, 1, BLACK, WHITE);

    Texture2D meshTexture = LoadTextureFromImage(textureImage);
    UnloadImage(textureImage);

    if (!worldModel.meshes[0].indices)
    {
        worldModel.meshes[0].indices = RL_CALLOC(worldModel.meshes[0].vertexCount, sizeof(unsigned short));
        for (int i = 0; i < worldModel.meshes[0].vertexCount; i++) worldModel.meshes[0].indices[i] = (unsigned short)i;
        worldModel.meshes[0].triangleCount = worldModel.meshes[0].vertexCount/3;
    }
    FillVertexColors(&worldModel.meshes[0]);

    Mesh ndcMesh = (Mesh){ 0 };
    ndcMesh.vertexCount = worldModel.meshes[0].vertexCount;
    ndcMesh.triangleCount = worldModel.meshes[0].triangleCount;
    ndcMesh.vertices = RL_CALLOC(ndcMesh.vertexCount, sizeof(Vector3));
    ndcMesh.texcoords = RL_CALLOC(ndcMesh.vertexCount, sizeof(Vector2));
    ndcMesh.indices = RL_CALLOC(ndcMesh.triangleCount, sizeof(Triangle));
    ndcMesh.colors = RL_CALLOC(ndcMesh.vertexCount, sizeof(Color));
    memcpy(ndcMesh.colors, worldModel.meshes[0].colors, ndcMesh.vertexCount*sizeof(Color));
    memcpy(ndcMesh.texcoords, worldModel.meshes[0].texcoords, ndcMesh.vertexCount*sizeof(Vector2));
    memcpy(ndcMesh.indices, worldModel.meshes[0].indices, ndcMesh.triangleCount*sizeof(Triangle));
    Model ndcModel = LoadModelFromMesh(ndcMesh);

    Mesh nearPlanePoints = (Mesh){ 0 };
    nearPlanePoints.vertexCount = worldModel.meshes[0].triangleCount*3;
    nearPlanePoints.vertices = RL_CALLOC(nearPlanePoints.vertexCount, sizeof(Vector3));
    Model nearPlanePointsModel = LoadModelFromMesh(nearPlanePoints);

    worldModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = meshTexture;
    ndcModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = meshTexture;

    Texture2D perspectiveCorrectTexture = (Texture2D){ 0 };
    Mesh spatialFrame = GenMeshCube(1.0f, 1.0f, 1.0f);
    spatialFrame.colors = RL_CALLOC(spatialFrame.vertexCount, sizeof(Color));
    for (int i = 0; i < spatialFrame.vertexCount; i++) ((Color *)spatialFrame.colors)[i] = (Color){ 255, 255, 255, 0 };
    for (int i = 0; i < 4; i++) ((Color *)spatialFrame.colors)[i].a = 255;
    Model spatialFrameModel = LoadModelFromMesh(spatialFrame);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float far = 3.0f;
        float near = 1.0f;
        aspect = (float)GetScreenWidth()/(float)GetScreenHeight();
        TOGGLE(KEY_N, FLAG_NDC);
        if (NDC_SPACE()) TOGGLE(KEY_F, FLAG_REFLECT_Y);
        TOGGLE(KEY_Q, FLAG_ASPECT);
        TOGGLE(KEY_P, FLAG_PERSPECTIVE_CORRECT);
        TOGGLE(KEY_SPACE, FLAG_PAUSE);
        TOGGLE(KEY_C, FLAG_COLOR_MODE);
        TOGGLE(KEY_T, FLAG_TEXTURE_MODE);

        float sBlend = SpaceBlendFactor(GetFrameTime());
        AspectBlendFactor(GetFrameTime());
        ReflectBlendFactor(GetFrameTime());

        if (!PAUSED()) meshRotation -= angularVelocity*GetFrameTime();

        OrbitSpace(&jugemu, GetFrameTime());

        WorldToNDCSpace(&main, aspect, near, far, &worldModel, &ndcModel, meshRotation);

        for (int i = 0; i < ndcModel.meshes[0].vertexCount; i++)
        {
            Vector3 *worldVertices = (Vector3 *)worldModel.meshes[0].vertices;
            Vector3 *ndcVertices = (Vector3 *)ndcModel.meshes[0].vertices;
            ndcVertices[i].x = Lerp(worldVertices[i].x, ndcVertices[i].x, sBlend);
            ndcVertices[i].y = Lerp(worldVertices[i].y, ndcVertices[i].y, sBlend);
            ndcVertices[i].z = Lerp(worldVertices[i].z, ndcVertices[i].z, sBlend);
        }

        Model *displayModel = &ndcModel;
        Mesh *displayMesh = &ndcModel.meshes[0];

        if (PERSPECTIVE_CORRECT() && TEXTURE_MODE())
        {
            PerspectiveCorrectCapture(&main, displayModel, meshTexture, &perspectiveCorrectTexture, meshRotation);
        }

        UpdateSpatialFrame(&main, aspect, near, far, &spatialFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(jugemu);
                Vector3 depth, right, up;
                BasisVector(&main, &depth, &right, &up);

                DrawLine3D(main.position, Vector3Add(main.position, right), NEON_CARROT);
                DrawLine3D(main.position, Vector3Add(main.position, up), LILAC);
                DrawLine3D(main.position, Vector3Add(main.position, depth), MARINER);

                DrawSpatialFrame(spatialFrame);

                DrawModelFilled(displayModel, meshTexture, meshRotation);
                DrawModelWiresAndPoints(displayModel, meshRotation);

                DrawNearPlanePoints(&main, aspect, near, &nearPlanePointsModel, displayMesh, meshRotation);

                if (PERSPECTIVE_CORRECT() && TEXTURE_MODE())
                {
                    spatialFrameModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = perspectiveCorrectTexture;
                    DrawModel(spatialFrameModel, modelPos, 1.0f, WHITE);
                }
                else
                {
                    PerspectiveIncorrectCapture(&main, aspect, near, displayMesh, meshTexture, meshRotation);
                }
            EndMode3D();

            DrawText("ARROWS: MOVE | SPACEBAR: PAUSE", 12, 12, fontSize, NEON_CARROT);
            DrawText("W A : ZOOM", 12, 38, fontSize, NEON_CARROT);
            DrawText("TEXTURE [ T ]:", 570, 12, fontSize, SUNFLOWER);
            DrawText((TEXTURE_MODE())? "ON" : "OFF", 740, 12, fontSize, (TEXTURE_MODE())? ANAKIWA : CHESTNUT_ROSE);
            DrawText("COLORS [ C ]:", 570, 38, fontSize, SUNFLOWER);
            DrawText((COLOR_MODE())? "ON" : "OFF", 740, 38, fontSize, (COLOR_MODE())? ANAKIWA : CHESTNUT_ROSE);
            DrawText("ASPECT [ Q ]:", 12, 392, fontSize, SUNFLOWER);
            DrawText((ASPECT_CORRECT())? "CORRECT" : "INCORRECT", 230, 392, fontSize, (ASPECT_CORRECT())? ANAKIWA : CHESTNUT_ROSE);
            DrawText("PERSPECTIVE [ P ]:", 12, 418, fontSize, SUNFLOWER);
            DrawText((PERSPECTIVE_CORRECT())? "CORRECT" : "INCORRECT", 230, 418, fontSize, (PERSPECTIVE_CORRECT())? ANAKIWA : CHESTNUT_ROSE);
            DrawText("SPACE [ N ]:", 530, 392, fontSize, SUNFLOWER);
            DrawText((NDC_SPACE())? "NDC" : "WORLD", 665, 392, fontSize, (NDC_SPACE())? BAHAMA_BLUE : ANAKIWA);
            if (NDC_SPACE())
            {
                DrawText("REFLECT [ F ]:", 530, 418, fontSize, SUNFLOWER);
                DrawText((REFLECT_Y())? "Y_DOWN" : "Y_UP", 695, 418, fontSize, (REFLECT_Y())? ANAKIWA : CHESTNUT_ROSE);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(worldModel);
    UnloadModel(ndcModel);
    UnloadModel(nearPlanePointsModel);
    UnloadModel(spatialFrameModel);
    UnloadTexture(meshTexture);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void BasisVector(Camera3D *main, Vector3 *depthOut, Vector3 *rightOut, Vector3 *upOut)
{
    Vector3 depth = Vector3Normalize(Vector3Subtract(main->target, main->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(depth, main->up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, depth));
    *depthOut = depth;
    *rightOut = right;
    *upOut = up;
}

static void WorldToNDCSpace(Camera3D *main, float aspect, float near, float far, Model *world, Model *ndc, float rotation)
{
    Vector3 depth, right, up;
    BasisVector(main, &depth, &right, &up);
    float halfHNear = near*tanf(DEG2RAD*main->fovy*0.5f);
    float halfWNear = Lerp(halfHNear, halfHNear*aspect, AspectBlendFactor(0.0f));
    float halfDepthNDC = Lerp(halfHNear, 0.5f*(far - near), AspectBlendFactor(0.0f));
    Vector3 centerNearPlane = Vector3Add(main->position, Vector3Scale(depth, near));
    Vector3 centerNDCCube = Vector3Add(centerNearPlane, Vector3Scale(depth, halfDepthNDC));

    for (int i = 0; i < world->meshes[0].vertexCount; i++)
    {
        Vector3 worldVertex = TranslateRotateScale(0, ((Vector3 *)world->meshes[0].vertices)[i], modelPos, modelScale, rotation);
        float signedDepth = Vector3DotProduct(Vector3Subtract(worldVertex, main->position), depth);
        Vector3 intersectionCoord = Intersect(main, near, worldVertex);
        Vector3 clipPlaneVector = Vector3Subtract(intersectionCoord, centerNearPlane);
        float xNDC = Vector3DotProduct(clipPlaneVector, right)/halfWNear;
        float yNDC = Vector3DotProduct(clipPlaneVector, up)/halfHNear;
        float zNDC = (far + near - 2.0f*far*near/signedDepth)/(far - near);
        Vector3 scaledRight = Vector3Scale(right, xNDC*halfWNear);
        Vector3 scaledUp = Vector3Scale(up, yNDC*halfHNear);
        Vector3 scaledDepth = Vector3Scale(depth, zNDC*halfDepthNDC);
        Vector3 offset = Vector3Add(Vector3Add(scaledRight, scaledUp), scaledDepth);
        Vector3 scaledNDCCoord = Vector3Add(centerNDCCube, offset);
        ((Vector3 *)ndc->meshes[0].vertices)[i] = TranslateRotateScale(1, scaledNDCCoord, modelPos, modelScale, rotation);
    }
}

static void DrawModelFilled(Model *model, Texture2D texture, float rotation)
{
    if (!(COLOR_MODE() || TEXTURE_MODE())) return;
    Color *cacheColors = (Color *)model->meshes[0].colors;
    if (TEXTURE_MODE() && !COLOR_MODE()) model->meshes[0].colors = NULL;

    model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = (TEXTURE_MODE())? texture.id : 0;
    DrawModelEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, WHITE);
    model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = 0;

    model->meshes[0].colors = (unsigned char *)cacheColors;
}

static void DrawModelWiresAndPoints(Model *model, float rotation)
{
    Color *cacheColors = (Color *)model->meshes[0].colors;
    unsigned int cacheID = model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id;
    model->meshes[0].colors = NULL;
    model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = 0;

    DrawModelWiresEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, MARINER);
    rlSetPointSize(4.0f);
    DrawModelPointsEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, LILAC);

    model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = cacheID;
    model->meshes[0].colors = (unsigned char *)cacheColors;
}

static void UpdateSpatialFrame(Camera3D *main, float aspect, float near, float far, Mesh *spatialFrame)
{
    Vector3 depth, right, up;
    BasisVector(main, &depth, &right, &up);
    float halfHNear = near*tanf(DEG2RAD*main->fovy*0.5f);
    float halfWNear = Lerp(halfHNear, halfHNear*aspect, AspectBlendFactor(0.0f));
    float halfHFar = far*tanf(DEG2RAD*main->fovy*0.5f);
    float halfWFar = Lerp(halfHFar, halfHFar*aspect, AspectBlendFactor(0.0f));
    float halfDepthNdc = Lerp(halfHNear, 0.5f*(far - near), AspectBlendFactor(0.0f));
    float halfDepth = Lerp(0.5f*(far - near), halfDepthNdc, SpaceBlendFactor(0.0f));
    float farHalfW = Lerp(halfWFar, halfWNear, SpaceBlendFactor(0.0f));
    float farHalfH = Lerp(halfHFar, halfHNear, SpaceBlendFactor(0.0f));
    Vector3 centerNear = Vector3Add(main->position, Vector3Scale(depth, near));

    for (int i = 0; i < spatialFrame->vertexCount; ++i)
    {
        Vector3 offset = Vector3Subtract(((Vector3 *)spatialFrame->vertices)[i], centerNear);
        float xSign = (Vector3DotProduct(offset, right) >= 0.0f)? 1.0f : -1.0f;
        float ySign = (Vector3DotProduct(offset, up) >= 0.0f)? 1.0f : -1.0f;
        float farMask = (Vector3DotProduct(offset, depth) > halfDepth)? 1.0f : 0.0f;
        float finalHalfW = halfWNear + farMask*(farHalfW - halfWNear);
        float finalHalfH = halfHNear + farMask*(farHalfH - halfHNear);
        Vector3 center = Vector3Add(centerNear, Vector3Scale(depth, farMask*2.0f*halfDepth));
        ((Vector3 *)spatialFrame->vertices)[i] = Vector3Add(center, Vector3Add(Vector3Scale(right, xSign*finalHalfW), Vector3Scale(up, ySign*finalHalfH)));
    }
}

static void DrawSpatialFrame(Mesh spatialFrame)
{
    static int frontFaces[4][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };
    static int backFaces[4][2] = { { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 } };
    static int ribFaces[4][2] = { { 0, 4 }, { 1, 7 }, { 2, 6 }, { 3, 5 } };
    static int (*faces[3])[2] = { frontFaces, backFaces, ribFaces };

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
        {
            Vector3 startPosition = ((Vector3 *)spatialFrame.vertices)[faces[i][j][0]];
            Vector3 endPosition = ((Vector3 *)spatialFrame.vertices)[faces[i][j][1]];
            DrawLine3D(startPosition, endPosition, (i == 0)? NEON_CARROT : (i == 1)? EGGPLANT : HOPBUSH);
        }
}

static void DrawNearPlanePoints(Camera3D *main, float aspect, float near, Model *nearPlanePointsModel, Mesh *mesh, float rotation)
{
    Vector3 depth, right, up;
    BasisVector(main, &depth, &right, &up);
    int nearPlaneVertexCount = 0;
    int capacity = mesh->triangleCount*3;
    Mesh *nearPlanePointsMesh = &nearPlanePointsModel->meshes[0];
    Vector3 centerNearPlane = Vector3Add(main->position, Vector3Scale(depth, near));
    float xAspect = Lerp(1.0f/aspect, 1.0f, AspectBlendFactor(0.0f));
    float yReflect = Lerp(1.0f, -1.0f, ReflectBlendFactor(0.0f));

    for (int i = 0; i < mesh->triangleCount; i++)
    {
        Vector3 *vertices = (Vector3 *)mesh->vertices;
        Triangle *triangles = (Triangle *)mesh->indices;

        Vector3 a = TranslateRotateScale(0, vertices[triangles[i][0]], modelPos, modelScale, rotation);
        Vector3 b = TranslateRotateScale(0, vertices[triangles[i][1]], modelPos, modelScale, rotation);
        Vector3 c = TranslateRotateScale(0, vertices[triangles[i][2]], modelPos, modelScale, rotation);

        // test if front facing or not (ugly one-liner -- comment out will ~double the rays, which is fine)
        if (Vector3DotProduct(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a))), depth) > 0.0f) continue;
        Vector3 intersectionPoints[3] = { Intersect(main, near, a), Intersect(main, near, b), Intersect(main, near, c) };

        for (int j = 0; j < 3 && nearPlaneVertexCount < capacity; ++j)
        {
            Vector3 corrected = AspectCorrectAndReflectNearPlane(intersectionPoints[j], centerNearPlane, right, up, xAspect, yReflect);
            DrawLine3D((Vector3[]){ a, b, c }[j], corrected, (Color){ RED_DAMASK.r, RED_DAMASK.g, RED_DAMASK.b, 20 });
            ((Vector3 *)nearPlanePointsMesh->vertices)[nearPlaneVertexCount] = corrected;
            nearPlaneVertexCount++;
        }
    }

    nearPlanePointsMesh->vertexCount = nearPlaneVertexCount;
    rlSetPointSize(3.0f);
    DrawModelPoints(*nearPlanePointsModel, modelPos, 1.0f, LILAC);
}

static void PerspectiveIncorrectCapture(Camera3D *main, float aspect, float near, Mesh *mesh, Texture2D meshTexture, float rotation)
{
    Vector3 depth, right, up;
    BasisVector(main, &depth, &right, &up);
    Vector3 centerNearPlane = Vector3Add(main->position, Vector3Scale(depth, near));
    float xAspect = Lerp(1.0f/aspect, 1.0f, AspectBlendFactor(0.0f));
    float yReflect = Lerp(1.0f, -1.0f, ReflectBlendFactor(0.0f));

    rlColor4ub(WHITE.r, WHITE.g, WHITE.b, WHITE.a); // just to emphasize raylib Colors are ub 0~255 not floats
    if (TEXTURE_MODE())
        rlEnableTexture(meshTexture.id);
    else
        rlDisableTexture();

    if (!TEXTURE_MODE() && !COLOR_MODE())
    {
        rlEnableWireMode();
        rlColor4ub(MARINER.r, MARINER.g, MARINER.b, MARINER.a);
    }
    rlBegin(RL_TRIANGLES);

    for (int i = 0; i < mesh->triangleCount; i++)
    {
        Triangle *triangles = (Triangle *)mesh->indices;
        Vector3 *vertices = (Vector3 *)mesh->vertices;
        Color *colors = (Color *)mesh->colors;
        Vector2 *texcoords = (Vector2 *)mesh->texcoords;

        Vector3 a = TranslateRotateScale(0, vertices[triangles[i][0]], modelPos, modelScale, rotation);
        Vector3 b = TranslateRotateScale(0, vertices[triangles[i][1]], modelPos, modelScale, rotation);
        Vector3 c = TranslateRotateScale(0, vertices[triangles[i][2]], modelPos, modelScale, rotation);

        a = AspectCorrectAndReflectNearPlane(Intersect(main, near, a), centerNearPlane, right, up, xAspect, yReflect);
        b = AspectCorrectAndReflectNearPlane(Intersect(main, near, b), centerNearPlane, right, up, xAspect, yReflect);
        c = AspectCorrectAndReflectNearPlane(Intersect(main, near, c), centerNearPlane, right, up, xAspect, yReflect);

        if (COLOR_MODE()) rlColor4ub(colors[triangles[i][0]].r, colors[triangles[i][0]].g, colors[triangles[i][0]].b, colors[triangles[i][0]].a);
        if (TEXTURE_MODE()) rlTexCoord2f(texcoords[triangles[i][0]].x, texcoords[triangles[i][0]].y);
        rlVertex3f(a.x, a.y, a.z);
        // vertex winding!! to account for reflection toggle (will draw the inside of the geometry otherwise)
        int secondIndex = (NDC_SPACE() && REFLECT_Y())? triangles[i][2] : triangles[i][1];
        Vector3 secondVertex = (NDC_SPACE() && REFLECT_Y())? c : b;
        if (COLOR_MODE()) rlColor4ub(colors[secondIndex].r, colors[secondIndex].g, colors[secondIndex].b, colors[secondIndex].a);
        if (TEXTURE_MODE()) rlTexCoord2f(texcoords[secondIndex].x, texcoords[secondIndex].y);
        rlVertex3f(secondVertex.x, secondVertex.y, secondVertex.z);

        int thirdIndex = (NDC_SPACE() && REFLECT_Y())? triangles[i][1] : triangles[i][2];
        Vector3 thirdVertex = (NDC_SPACE() && REFLECT_Y())? b : c;
        if (COLOR_MODE()) rlColor4ub(colors[thirdIndex].r, colors[thirdIndex].g, colors[thirdIndex].b, colors[thirdIndex].a);
        if (TEXTURE_MODE()) rlTexCoord2f(texcoords[thirdIndex].x, texcoords[thirdIndex].y);
        rlVertex3f(thirdVertex.x, thirdVertex.y, thirdVertex.z);
    }

    rlEnd();
    rlDisableTexture();
    rlDisableWireMode();
}

static void PerspectiveCorrectCapture(Camera3D *main, Model *model, Texture2D meshTexture, Texture2D *perspectiveCorrectTexture, float rotation)
{
    unsigned char *cacheColors = model->meshes[0].colors;
    if (TEXTURE_MODE() && !COLOR_MODE()) model->meshes[0].colors = NULL;

    ClearBackground(BLACK);

    BeginMode3D(*main);
        Texture2D previousTexture = model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture;
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture = meshTexture;
        DrawModelEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, WHITE);
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture = previousTexture;
    EndMode3D();

    Image rgba = LoadImageFromScreen();
    ImageFormat(&rgba, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    model->meshes[0].colors = cacheColors;

    ClearBackground(BLACK);

    BeginMode3D(*main);
        Texture2D cacheTexture = model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture;
        Color cacheMaterialColor = model->materials[0].maps[MATERIAL_MAP_ALBEDO].color;
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture = (Texture2D){ 0 };
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        DrawModelEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, WHITE);
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].texture = cacheTexture;
        model->materials[0].maps[MATERIAL_MAP_ALBEDO].color = cacheMaterialColor;
    EndMode3D();

    Image mask = LoadImageFromScreen();
    AlphaMaskPunchOut(&rgba, &mask, 1);
    ImageFlipVertical(&rgba);
    if ((NDC_SPACE() && REFLECT_Y())) ImageFlipVertical(&rgba); // FLIP AGAIN.. it works visually, but is not clear and feels hacked/ugly
    if ((perspectiveCorrectTexture->id != 0))
        UpdateTexture(*perspectiveCorrectTexture, rgba.data);
    else
        *perspectiveCorrectTexture = LoadTextureFromImage(rgba);

    UnloadImage(mask);
    UnloadImage(rgba);
}

static void OrbitSpace(Camera3D *jugemu, float dt)
{
    float radius = Vector3Length(jugemu->position);
    float azimuth = atan2f(jugemu->position.z, jugemu->position.x);
    float horizontalRadius = sqrtf(jugemu->position.x*jugemu->position.x + jugemu->position.z*jugemu->position.z);
    float elevation = atan2f(jugemu->position.y, horizontalRadius);
    if (IsKeyDown(KEY_LEFT)) azimuth += 1.5f*dt;
    if (IsKeyDown(KEY_RIGHT)) azimuth -= 1.5f*dt;
    if (IsKeyDown(KEY_UP)) elevation += 1.0f*dt;
    if (IsKeyDown(KEY_DOWN)) elevation -= 1.0f*dt;
    if (IsKeyDown(KEY_W)) radius -= 2.0f*dt;
    if (IsKeyDown(KEY_S)) radius += 2.0f*dt;
    elevation = Clamp(elevation, -M_PI_2 + 0.1f, M_PI_2 - 0.1f);
    jugemu->position.x = Clamp(radius, 0.25f, 10.0f)*cosf(elevation)*cosf(azimuth);
    jugemu->position.y = Clamp(radius, 0.25f, 10.0f)*sinf(elevation);
    jugemu->position.z = Clamp(radius, 0.25f, 10.0f)*cosf(elevation)*sinf(azimuth);
}

static void AlphaMaskPunchOut(Image *rgba, Image *mask, unsigned char threshold)
{
    Image maskCopy = ImageCopy(*mask);
    ImageFormat(&maskCopy, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    ImageFormat(rgba, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    unsigned char *maskGrayScale = maskCopy.data;
    Color *colors = rgba->data;
    int pixelCount = rgba->width*rgba->height;
    for (size_t i = 0; i < pixelCount; ++i) colors[i].a = (maskGrayScale[i] > threshold)? 255 : 0;
    UnloadImage(maskCopy);
}

static void FillVertexColors(Mesh *mesh)
{
    if (!mesh->colors) mesh->colors = RL_CALLOC(mesh->vertexCount, sizeof(Color));
    Color *colors = (Color *)mesh->colors;
    Vector3 *vertices = (Vector3 *)mesh->vertices;
    BoundingBox bounds = GetMeshBoundingBox(*mesh);

    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        Vector3 vertex = vertices[i];
        float nx = (vertex.x - 0.5f*(bounds.min.x + bounds.max.x))/(0.5f*(bounds.max.x - bounds.min.x));
        float ny = (vertex.y - 0.5f*(bounds.min.y + bounds.max.y))/(0.5f*(bounds.max.y - bounds.min.y));
        float nz = (vertex.z - 0.5f*(bounds.min.z + bounds.max.z))/(0.5f*(bounds.max.z - bounds.min.z));
        float len = sqrtf(nx*nx + ny*ny + nz*nz);
        colors[i] = (Color){ lrintf(127.5f*(nx/len + 1.0f)), lrintf(127.5f*(ny/len + 1.0f)), lrintf(127.5f*(nz/len + 1.0f)), 255 };
    }
}

static Vector3 AspectCorrectAndReflectNearPlane(Vector3 intersect, Vector3 center, Vector3 right, Vector3 up, float xAspect, float yReflect)
{
    Vector3 centerDistance = Vector3Subtract(intersect, center);
    float x = Vector3DotProduct(centerDistance, right);
    float y = Vector3DotProduct(centerDistance, up);
    return Vector3Add(center, Vector3Add(Vector3Scale(right, x*xAspect), Vector3Scale(up, y*yReflect)));
}

static Vector3 TranslateRotateScale(int inverse, Vector3 coordinate, Vector3 pos, Vector3 scale, float rotation)
{
    Matrix matrix = MatrixMultiply(MatrixMultiply(MatrixScale(scale.x, scale.y, scale.z), MatrixRotateY(rotation)), MatrixTranslate(pos.x, pos.y, pos.z));
    Matrix result = inverse ? MatrixInvert(matrix) : matrix;
    return Vector3Transform(coordinate, result);
}

static Vector3 Intersect(Camera3D *main, float near, Vector3 worldCoord)
{
    Vector3 viewDir = Vector3Normalize(Vector3Subtract(main->target, main->position));
    Vector3 mainCameraToPoint = Vector3Subtract(worldCoord, main->position);
    float depthAlongView = Vector3DotProduct(mainCameraToPoint, viewDir);
    if (depthAlongView <= 0.0f) return Vector3Add(main->position, Vector3Scale(viewDir, near));
    float scaleToNear = near/depthAlongView;
    return Vector3Add(main->position, Vector3Scale(mainCameraToPoint, scaleToNear));
}

static float SpaceBlendFactor(float dt)
{
    static float blend = 0.0f;
    if (dt > 0.0f) blend = Clamp(blend + ((NDC_SPACE())? 1.0f : -1.0f)*blendScalar*dt, 0.0f, 1.0f);
    return blend;
}

static float AspectBlendFactor(float dt)
{
    static float blend = 0.0f;
    if (dt > 0.0f) blend = Clamp(blend + ((ASPECT_CORRECT())? 1.0f : -1.0f)*blendScalar*dt, 0.0f, 1.0f);
    return blend;
}

static float ReflectBlendFactor(float dt)
{
    static float blend = 0.0f;
    if (dt > 0.0f)
    {
        float target = (NDC_SPACE() && REFLECT_Y())? 1.0f : 0.0f;
        float direction = (blend < target)? 1.0f : (blend > target)? -1.0f : 0.0f;
        blend = Clamp(blend + direction*blendScalar*dt, 0.0f, 1.0f);
    }
    return blend;
}
