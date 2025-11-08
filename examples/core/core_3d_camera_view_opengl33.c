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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char *vert =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "out vec2 fragTexCoord;\n"
    "out vec4 fragColor;\n"
    "uniform int useVertexColors;\n"
    "void main()\n"
    "{\n"
    "    if (useVertexColors == 1) {\n"
    "        fragColor = vertexColor;\n"
    "    } else {\n"
    "        fragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "    }\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *frag =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec4 colDiffuse;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);\n"
    "    vec4 outColor = texelColor*fragColor*colDiffuse;\n"
    "    if (outColor.a <= 0.0) discard; \n"
    "    finalColor = outColor;\n"
    "}\n";

static Shader customShader = { 0 };
static int useVertexColorsLoc = -1;

#define BAHAMA_BLUE CLITERAL(Color){ 0, 102, 153, 255 }
#define SUNFLOWER CLITERAL(Color){ 255, 204, 153, 255 }
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
    FLAG_ASPECT = 1u<<0,
    FLAG_PAUSE = 1u<<1,
    FLAG_JUGEMU = 1u<<2,
    FLAG_ORTHO = 1u<<3,
    GEN_CUBE = 1u<<4,
    GEN_SPHERE = 1u<<5,
    GEN_KNOT = 1u<<6
};

static unsigned int gflags = FLAG_ASPECT | FLAG_JUGEMU | GEN_CUBE;

#define ASPECT_CORRECT() ((gflags & FLAG_ASPECT) != 0)
#define PAUSED() ((gflags & FLAG_PAUSE) != 0)
#define JUGEMU_MODE() ((gflags & FLAG_JUGEMU) != 0)
#define ORTHO_MODE() ((gflags & FLAG_ORTHO) != 0)
#define TOGGLE(K, F) do { if (IsKeyPressed(K)) { gflags ^= (F); } } while (0)

static unsigned int targetMesh = 0;
#define NUM_MODELS  3
#define CYCLE_MESH(K, I, F) do { if (IsKeyPressed(K)) { targetMesh = (I); gflags = (gflags & ~(GEN_CUBE|GEN_SPHERE|GEN_KNOT)) | (F); } } while (0)

static int fontSize = 20;
static float angularVelocity = 1.25f;
static float fovyPerspective = 60.0f;
static float nearPlaneHeightOrthographic = 1.0f;
static float blendScalar = 5.0f;
static Vector3 yAxis = { 0.0f, 1.0f, 0.0f };
static Vector3 modelPos = { 0.0f, 0.0f, 0.0f };
static Vector3 modelScale = { 1.0f, 1.0f, 1.0f };
static Vector3 mainPos = { 0.0f, 0.0f, 2.0f };
static Vector3 jugemuPosIso = { 3.0f, 1.0f, 3.0f };

static void UpdateSpatialFrame(Camera3D *main, float aspect, float near, float far, Mesh *spatialFrame);
static void DrawSpatialFrame(Mesh *spatialFrame);
static void PerspectiveCorrectCapture(Camera3D *main, Model *model, RenderTexture2D *perspectiveCorrectRenderTexture, float rotation);
static void FillVertexColors(Mesh *mesh);
static void OrbitSpace(Camera3D *jugemu, float dt);
static Vector3 TranslateRotateScale(Vector3 coordinate, Vector3 pos, Vector3 scale, float rotation);
static Vector3 Intersect(Camera3D *main, float near, Vector3 worldCoord);
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - fixed function didactic");
    customShader = LoadShaderFromMemory(vert, frag);
    useVertexColorsLoc = GetShaderLocation(customShader, "useVertexColors");
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
    jugemu.position = jugemuPosIso;
    jugemu.target = modelPos;
    jugemu.up = yAxis;
    jugemu.fovy = fovyPerspective;
    jugemu.projection = CAMERA_PERSPECTIVE;

    Model models[NUM_MODELS] = { 0 };
    models[0] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    models[1] = LoadModelFromMesh(GenMeshSphere(0.5f, 8, 8));
    models[2] = LoadModelFromMesh(GenMeshKnot(1.0f, 1.0f, 16, 128));
    for (int i = 0; i < NUM_MODELS; i++)
    {
        Mesh *mesh = &models[i].meshes[0];
        mesh->vaoId = 0;
        if (!mesh->indices)
        {
            mesh->indices = RL_CALLOC(mesh->vertexCount, sizeof(unsigned short));
            for (int j = 0; j < mesh->vertexCount; j++) mesh->indices[j] = (unsigned short)j;
            mesh->triangleCount = mesh->vertexCount/3;
        }
        FillVertexColors(mesh);
        UploadMesh(mesh, true);
    }

    Mesh tempCube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Mesh spatialFrame = { 0 };
    spatialFrame.vertexCount = tempCube.vertexCount;
    spatialFrame.triangleCount = tempCube.triangleCount;
    spatialFrame.vertices = RL_MALLOC(spatialFrame.vertexCount * 3 * sizeof(float));
    spatialFrame.normals = RL_MALLOC(spatialFrame.vertexCount * 3 * sizeof(float));
    spatialFrame.texcoords = RL_MALLOC(spatialFrame.vertexCount * 2 * sizeof(float));
    spatialFrame.indices = RL_MALLOC(spatialFrame.triangleCount * 3 * sizeof(unsigned short));
    spatialFrame.colors = RL_CALLOC(spatialFrame.vertexCount, sizeof(Color));
    memcpy(spatialFrame.vertices, tempCube.vertices, spatialFrame.vertexCount * 3 * sizeof(float));
    memcpy(spatialFrame.normals, tempCube.normals, spatialFrame.vertexCount * 3 * sizeof(float));
    memcpy(spatialFrame.texcoords, tempCube.texcoords, spatialFrame.vertexCount * 2 * sizeof(float));
    memcpy(spatialFrame.indices, tempCube.indices, spatialFrame.triangleCount * 3 * sizeof(unsigned short));
    for (int i = 0; i < spatialFrame.vertexCount; i++) ((Color *)spatialFrame.colors)[i] = (Color){ 255, 255, 255, 0 };
    for (int i = 0; i < 4; i++) ((Color *)spatialFrame.colors)[i].a = 255;
    UnloadMesh(tempCube);
    UploadMesh(&spatialFrame, true);
    Model spatialFrameModel = LoadModelFromMesh(spatialFrame);
    spatialFrameModel.materials[0].shader = customShader;
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        aspect = (float)GetScreenWidth()/(float)GetScreenHeight();
        TOGGLE(KEY_Q, FLAG_ASPECT);
        TOGGLE(KEY_SPACE, FLAG_PAUSE);
        TOGGLE(KEY_J, FLAG_JUGEMU);
        TOGGLE(KEY_O, FLAG_ORTHO);
        CYCLE_MESH(KEY_ONE, 0, GEN_CUBE);
        CYCLE_MESH(KEY_TWO, 1, GEN_SPHERE);
        CYCLE_MESH(KEY_THREE, 2, GEN_KNOT);

        OrthoBlendFactor(GetFrameTime());

        if (!PAUSED()) meshRotation -= angularVelocity*GetFrameTime();

        OrbitSpace(&jugemu, GetFrameTime());
        main.projection = (ORTHO_MODE())? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;
        main.fovy = (ORTHO_MODE())? nearPlaneHeightOrthographic : fovyPerspective;

        Model *displayModel = &models[targetMesh];

        PerspectiveCorrectCapture(&main, displayModel, &perspectiveCorrectRenderTexture, meshRotation);

        UpdateSpatialFrame(&main, aspect, near, far, &spatialFrame);
        UpdateMeshBuffer(spatialFrameModel.meshes[0], RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, spatialFrame.vertices, spatialFrame.vertexCount*sizeof(Vector3), 0);
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
                DrawLine3D(main.position, Vector3Add(main.position, up), LILAC);
                DrawLine3D(main.position, Vector3Add(main.position, depth), MARINER);

                if (JUGEMU_MODE()) DrawSpatialFrame(&spatialFrame);

                DrawModelEx(*displayModel, modelPos, yAxis, RAD2DEG*meshRotation, modelScale, WHITE);

                unsigned int cacheID = displayModel->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id;
                displayModel->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
                DrawModelWiresEx(*displayModel, modelPos, yAxis, RAD2DEG*meshRotation, modelScale, MARINER);
                rlSetPointSize(4.0f);
                DrawModelPointsEx(*displayModel, modelPos, yAxis, RAD2DEG*meshRotation, modelScale, LILAC);
                displayModel->materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = cacheID;

                if (JUGEMU_MODE())
                {
                    Vector3 *vertices = (Vector3 *)displayModel->meshes[0].vertices;
                    Triangle *triangles = (Triangle *)displayModel->meshes[0].indices;
                    for (int i = 0; i < displayModel->meshes[0].triangleCount; i++)
                    {
                        Vector3 a = TranslateRotateScale(vertices[triangles[i][0]], modelPos, modelScale, meshRotation);
                        Vector3 b = TranslateRotateScale(vertices[triangles[i][1]], modelPos, modelScale, meshRotation);
                        Vector3 c = TranslateRotateScale(vertices[triangles[i][2]], modelPos, modelScale, meshRotation);
                        if (Vector3DotProduct(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a))), depth) > 0.0f) continue;
                        DrawLine3D(a, Intersect(&main, near, a), (Color){ RED_DAMASK.r, RED_DAMASK.g, RED_DAMASK.b, 20 });
                        DrawLine3D(b, Intersect(&main, near, b), (Color){ RED_DAMASK.r, RED_DAMASK.g, RED_DAMASK.b, 20 });
                        DrawLine3D(c, Intersect(&main, near, c), (Color){ RED_DAMASK.r, RED_DAMASK.g, RED_DAMASK.b, 20 });
                    }
                }
                spatialFrameModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = perspectiveCorrectRenderTexture.texture;
                //NOTE: there is still the alpha threshold issue without custom frag shader...
                int useColors = 1;
                SetShaderValue(spatialFrameModel.materials[0].shader, useVertexColorsLoc, &useColors, SHADER_UNIFORM_INT);
                if (JUGEMU_MODE()) DrawModel(spatialFrameModel, modelPos, 1.0f, WHITE);

            EndMode3D();

            DrawText("[1]: CUBE [2]: SPHERE [3]: KNOT", 12, 12, fontSize, NEON_CARROT);
            DrawText("ARROWS: MOVE | SPACEBAR: PAUSE", 12, 38, fontSize, NEON_CARROT);
            DrawText("W S : ZOOM ", 12, 64, fontSize, NEON_CARROT);
            DrawText((targetMesh == 0)? "GEN_CUBE" : (targetMesh == 1)? "GEN_SPHERE" : "GEN_KNOT", 12, 205, fontSize, NEON_CARROT);
            DrawText("ASPECT [ Q ]:", 12, 392, fontSize, SUNFLOWER);
            DrawText((ASPECT_CORRECT())? "CORRECT" : "INCORRECT", 230, 392, fontSize, (ASPECT_CORRECT())? ANAKIWA : CHESTNUT_ROSE);
            DrawText("LENS [ O ]:", 510, 366, fontSize, SUNFLOWER);
            DrawText((ORTHO_MODE())? "ORTHOGRAPHIC" : "PERSPECTIVE", 630, 366, fontSize, (ORTHO_MODE())? BAHAMA_BLUE : ANAKIWA);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(models[0]);
    UnloadModel(models[1]);
    UnloadModel(models[2]);
    UnloadModel(spatialFrameModel);
    if (perspectiveCorrectRenderTexture.id) UnloadRenderTexture(perspectiveCorrectRenderTexture);
    UnloadShader(customShader);
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}

static void UpdateSpatialFrame(Camera3D *main, float aspect, float near, float far, Mesh *spatialFrame)
{
    Vector3 depth = Vector3Normalize(Vector3Subtract(main->target, main->position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(depth, main->up));
    Vector3 up = Vector3Normalize(Vector3CrossProduct(right, depth));
    float halfHNear = Lerp(near*tanf(DEG2RAD*fovyPerspective*0.5f), 0.5f*nearPlaneHeightOrthographic, OrthoBlendFactor(0.0f));
    float halfWNear = halfHNear*aspect;
    float halfHFar = Lerp(far*tanf(DEG2RAD*fovyPerspective*0.5f), 0.5f*nearPlaneHeightOrthographic, OrthoBlendFactor(0.0f));
    float halfWFar = halfHFar*aspect;
    float halfDepth = 0.5f*(far - near);
    Vector3 centerNear = Vector3Add(main->position, Vector3Scale(depth, near));

    for (int i = 0; i < spatialFrame->vertexCount; ++i)
    {
        Vector3 offset = Vector3Subtract(((Vector3 *)spatialFrame->vertices)[i], centerNear);
        float xSign = (Vector3DotProduct(offset, right) >= 0.0f)? 1.0f : -1.0f;
        float ySign = (Vector3DotProduct(offset, up) >= 0.0f)? 1.0f : -1.0f;
        float farMask = (Vector3DotProduct(offset, depth) > halfDepth)? 1.0f : 0.0f;
        float finalHalfW = halfWNear + farMask*(halfWFar - halfWNear);
        float finalHalfH = halfHNear + farMask*(halfHFar - halfHNear);
        Vector3 center = Vector3Add(centerNear, Vector3Scale(depth, farMask*2.0f*halfDepth));
        ((Vector3 *)spatialFrame->vertices)[i] = Vector3Add(center, Vector3Add(Vector3Scale(right, xSign*finalHalfW), Vector3Scale(up, ySign*finalHalfH)));
    }
}

static void DrawSpatialFrame(Mesh *spatialFrame)
{
    static int frontFaces[4][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };
    static int backFaces[4][2] = { { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 } };
    static int ribFaces[4][2] = { { 0, 4 }, { 1, 7 }, { 2, 6 }, { 3, 5 } };
    static int (*faces[3])[2] = { frontFaces, backFaces, ribFaces };

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
        {
            Vector3 startPosition = ((Vector3 *)spatialFrame->vertices)[faces[i][j][0]];
            Vector3 endPosition = ((Vector3 *)spatialFrame->vertices)[faces[i][j][1]];
            DrawLine3D(startPosition, endPosition, (i == 0)? NEON_CARROT : (i == 1)? EGGPLANT : HOPBUSH);
        }
}

static void PerspectiveCorrectCapture(Camera3D *main, Model *model, RenderTexture2D *perspectiveCorrectRenderTexture, float rotation)
{
    BeginTextureMode(*perspectiveCorrectRenderTexture);
        ClearBackground(BLANK);
        BeginMode3D(*main);
            DrawModelEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, WHITE);
            DrawModelWiresEx(*model, modelPos, yAxis, RAD2DEG*rotation, modelScale, MARINER);
        EndMode3D();
    EndTextureMode();
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

static Vector3 TranslateRotateScale(Vector3 coordinate, Vector3 pos, Vector3 scale, float rotation)
{
    return Vector3Transform(coordinate, MatrixMultiply(MatrixMultiply(MatrixScale(scale.x, scale.y, scale.z), MatrixRotateY(rotation)), MatrixTranslate(pos.x, pos.y, pos.z)));
}

static Vector3 Intersect(Camera3D *main, float near, Vector3 worldCoord)
{
    Vector3 viewDir = Vector3Normalize(Vector3Subtract(main->target, main->position));
    Vector3 mainCameraToPoint = Vector3Subtract(worldCoord, main->position);
    float depthAlongView = Vector3DotProduct(mainCameraToPoint, viewDir);
    Vector3 centerNearPlane = Vector3Add(main->position, Vector3Scale(viewDir, near));
    if (depthAlongView <= 0.0f) return centerNearPlane;
    float scaleToNear = near/depthAlongView;
    Vector3 resultPerspective = Vector3Add(main->position, Vector3Scale(mainCameraToPoint, scaleToNear));
    Vector3 resultOrtho = Vector3Add(worldCoord, Vector3Scale(viewDir, Vector3DotProduct(Vector3Subtract(centerNearPlane, worldCoord), viewDir)));
    return Vector3Lerp(resultPerspective, resultOrtho, OrthoBlendFactor(0.0f));
}

static float OrthoBlendFactor(float dt)
{
    static float blend = 0.0f;
    if (dt > 0.0f) blend = Clamp(blend + ((ORTHO_MODE())? 1.0f : -1.0f)*blendScalar*dt, 0.0f, 1.0f);
    return blend;
}