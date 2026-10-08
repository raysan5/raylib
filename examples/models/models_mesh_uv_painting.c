/*******************************************************************************************
*
*   raylib [models] example - mesh uv painting
*
*   Example demonstrates painting directly onto a mesh's texture: a ray is cast from the
*   mouse into the scene, the hit triangle is found, and the hit point is converted into
*   UV space using barycentric interpolation so a brush stroke can be drawn on the texture
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by PanicTitan (@PanicTitan) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 PanicTitan (@PanicTitan)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <math.h>       // Required for: fabsf(), floorf()
#include <stddef.h>     // Required for: NULL

//--------------------------------------------------------------------------------------
// Global Definitions
//--------------------------------------------------------------------------------------
#define CANVAS_SIZE     512
#define PALETTE_COUNT   8

//--------------------------------------------------------------------------------------
// Types and Structures Definition
//--------------------------------------------------------------------------------------
typedef enum { TOOL_PAINT = 0, TOOL_PICKER } ToolMode;
typedef enum { SHAPE_SPHERE = 0, SHAPE_CUBE, SHAPE_CYLINDER, SHAPE_TORUS } ShapeType;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void ChangeShape(Model *model, BoundingBox *bbox, ShapeType *currentShape, ShapeType newShape, Texture2D canvasTexture);
static bool GetMeshHitUV(Ray ray, Model model, BoundingBox bbox, Vector2 *outUV);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh uv painting");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 3.5f, 6.5f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // CPU-side canvas plus the GPU texture it gets pushed to after every stroke
    Image canvasImage = GenImageColor(CANVAS_SIZE, CANVAS_SIZE, RAYWHITE);
    Texture2D canvasTexture = LoadTextureFromImage(canvasImage);
    SetTextureFilter(canvasTexture, TEXTURE_FILTER_BILINEAR);

    Model model = { 0 };
    BoundingBox modelBBox = { 0 };
    ShapeType currentShape = SHAPE_SPHERE;
    ChangeShape(&model, &modelBBox, &currentShape, SHAPE_SPHERE, canvasTexture);

    ToolMode currentTool = TOOL_PAINT;
    Color activeColor = RED;
    int brushRadius = 12;
    Vector2 lastHitUV = { 0 };
    bool hasLastHit = false;

    Color palette[PALETTE_COUNT] = { RED, ORANGE, GOLD, LIME, SKYBLUE, PURPLE, DARKGRAY, WHITE };
    Rectangle uiPanelRec = { 10.0f, 10.0f, 230.0f, 490.0f };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePos = GetMousePosition();
        bool isMouseOverUI = CheckCollisionPointRec(mousePos, uiPanelRec);

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        if (!isMouseOverUI && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Ray ray = GetScreenToWorldRay(mousePos, camera);
            Vector2 hitUV = { 0 };

            if (GetMeshHitUV(ray, model, modelBBox, &hitUV))
            {
                int px = (int)(hitUV.x * CANVAS_SIZE);
                int py = (int)(hitUV.y * CANVAS_SIZE);

                if (currentTool == TOOL_PAINT)
                {
                    // Stroke from the last hit to this one, guarding against jumps across a UV seam
                    if (hasLastHit && (fabsf(hitUV.x - lastHitUV.x) < 0.25f) && (fabsf(hitUV.y - lastHitUV.y) < 0.25f))
                    {
                        int lastPx = (int)(lastHitUV.x * CANVAS_SIZE);
                        int lastPy = (int)(lastHitUV.y * CANVAS_SIZE);
                        float dist = Vector2Distance((Vector2){ (float)lastPx, (float)lastPy }, (Vector2){ (float)px, (float)py });
                        int steps = (int)(dist / 2.0f) + 1;

                        for (int i = 0; i <= steps; i++)
                        {
                            float t = (float)i / (float)steps;
                            ImageDrawCircle(&canvasImage, (int)Lerp((float)lastPx, (float)px, t),
                                (int)Lerp((float)lastPy, (float)py, t), brushRadius, activeColor);
                        }
                    }
                    else ImageDrawCircle(&canvasImage, px, py, brushRadius, activeColor);

                    UpdateTexture(canvasTexture, canvasImage.data);
                    lastHitUV = hitUV;
                    hasLastHit = true;
                }
                else
                {
                    activeColor = GetImageColor(canvasImage, px, py);
                    currentTool = TOOL_PAINT;
                    hasLastHit = false;
                }
            }
            else hasLastHit = false;
        }
        else hasLastHit = false;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground((Color){ 30, 32, 40, 255 });

            BeginMode3D(camera);
                DrawModel(model, Vector3Zero(), 1.0f, WHITE);
                DrawGrid(10, 1.0f);
            EndMode3D();

            // Side panel
            DrawRectangleRec(uiPanelRec, Fade(BLACK, 0.8f));
            DrawRectangleLinesEx(uiPanelRec, 2.0f, DARKGRAY);
            DrawText("MESH UV PAINTER", 25, 22, 20, GOLD);

            // Tool selection toggles
            bool paintActive = (currentTool == TOOL_PAINT);
            bool pickerActive = (currentTool == TOOL_PICKER);
            if (GuiToggle((Rectangle){ 25, 55, 95, 32 }, "PAINT", &paintActive)) currentTool = TOOL_PAINT;
            if (GuiToggle((Rectangle){ 125, 55, 95, 32 }, "PICKER", &pickerActive)) currentTool = TOOL_PICKER;

            // Shape selection toggles
            DrawText("Mesh Shape:", 25, 100, 10, LIGHTGRAY);
            bool sphereActive = (currentShape == SHAPE_SPHERE);
            bool cubeActive = (currentShape == SHAPE_CUBE);
            bool cylinderActive = (currentShape == SHAPE_CYLINDER);
            bool torusActive = (currentShape == SHAPE_TORUS);

            if (GuiToggle((Rectangle){ 25, 120, 95, 28 }, "SPHERE", &sphereActive))
                ChangeShape(&model, &modelBBox, &currentShape, SHAPE_SPHERE, canvasTexture);
            if (GuiToggle((Rectangle){ 125, 120, 95, 28 }, "CUBE", &cubeActive))
                ChangeShape(&model, &modelBBox, &currentShape, SHAPE_CUBE, canvasTexture);
            if (GuiToggle((Rectangle){ 25, 153, 95, 28 }, "CYLINDER", &cylinderActive))
                ChangeShape(&model, &modelBBox, &currentShape, SHAPE_CYLINDER, canvasTexture);
            if (GuiToggle((Rectangle){ 125, 153, 95, 28 }, "TORUS", &torusActive))
                ChangeShape(&model, &modelBBox, &currentShape, SHAPE_TORUS, canvasTexture);

            // Color display
            DrawText("Active Color:", 25, 195, 10, LIGHTGRAY);
            DrawRectangle(125, 193, 95, 20, activeColor);
            DrawRectangleLines(125, 193, 95, 20, WHITE);

            // Color swatches
            DrawText("Palette Swatches:", 25, 225, 10, LIGHTGRAY);
            for (int i = 0; i < PALETTE_COUNT; i++)
            {
                Rectangle swatchRec = { 25.0f + (i%4)*48, 245.0f + (i/4)*45, 40.0f, 38.0f };
                DrawRectangleRec(swatchRec, palette[i]);
                DrawRectangleLinesEx(swatchRec, 1.0f, WHITE);
                if (CheckCollisionPointRec(mousePos, swatchRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) activeColor = palette[i];
            }

            // Brush size buttons
            DrawText(TextFormat("Brush Size: %dpx", brushRadius), 25, 345, 10, LIGHTGRAY);
            if (GuiButton((Rectangle){ 25, 365, 95, 30 }, "SIZE -") && (brushRadius > 2)) brushRadius -= 2;
            if (GuiButton((Rectangle){ 125, 365, 95, 30 }, "SIZE +") && (brushRadius < 64)) brushRadius += 2;

            // Canvas action buttons
            if (GuiButton((Rectangle){ 25, 410, 95, 30 }, "CLEAR"))
            {
                ImageClearBackground(&canvasImage, RAYWHITE);
                UpdateTexture(canvasTexture, canvasImage.data);
            }
            if (GuiButton((Rectangle){ 125, 410, 95, 30 }, "FILL"))
            {
                ImageClearBackground(&canvasImage, activeColor);
                UpdateTexture(canvasTexture, canvasImage.data);
            }

            DrawText("Left click: paint / pick color   |   Right drag: orbit camera", 260, screenHeight - 25, 10, RAYWHITE);
            DrawFPS(screenWidth - 90, 15);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);
    UnloadImage(canvasImage);
    UnloadTexture(canvasTexture);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Unloads the current model and builds a new primitive shape in its place, sharing the
// same canvas texture, and recomputes the transformed bounding box used for ray testing
static void ChangeShape(Model *model, BoundingBox *bbox, ShapeType *currentShape, ShapeType newShape, Texture2D canvasTexture)
{
    UnloadModel(*model);

    Mesh mesh = { 0 };

    switch (newShape)
    {
        case SHAPE_SPHERE:   mesh = GenMeshSphere(1.5f, 32, 32); break;
        case SHAPE_CUBE:     mesh = GenMeshCube(2.2f, 2.2f, 2.2f); break;
        case SHAPE_CYLINDER: mesh = GenMeshCylinder(1.2f, 2.5f, 24); break;
        case SHAPE_TORUS:    mesh = GenMeshTorus(0.6f, 1.6f, 24, 36); break;
        default:             mesh = GenMeshSphere(1.5f, 32, 32); break;
    }

    *model = LoadModelFromMesh(mesh);

    // GenMeshCylinder builds upward from y = 0; shift it down to center on the origin
    if (newShape == SHAPE_CYLINDER) model->transform = MatrixTranslate(0.0f, -1.25f, 0.0f);

    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = canvasTexture;

    BoundingBox box = GetMeshBoundingBox(model->meshes[0]);
    box.min = Vector3Transform(box.min, model->transform);
    box.max = Vector3Transform(box.max, model->transform);
    *bbox = box;

    *currentShape = newShape;
}

// Raycasts against the model's mesh triangles and returns the UV coordinate of the
// closest hit, interpolated from the hit triangle's vertices with barycentric weights
static bool GetMeshHitUV(Ray ray, Model model, BoundingBox bbox, Vector2 *outUV)
{
    if (!GetRayCollisionBox(ray, bbox).hit) return false; // Fast reject

    Mesh mesh = model.meshes[0];
    float closestDistance = 1e9f;
    bool found = false;
    Vector2 hitUV = { 0 };

    for (int tri = 0; tri < mesh.triangleCount; tri++)
    {
        int i0, i1, i2;

        if (mesh.indices != NULL)
        {
            i0 = mesh.indices[3*tri + 0];
            i1 = mesh.indices[3*tri + 1];
            i2 = mesh.indices[3*tri + 2];
        }
        else { i0 = 3*tri; i1 = 3*tri + 1; i2 = 3*tri + 2; }

        Vector3 a = Vector3Transform((Vector3){ mesh.vertices[3*i0], mesh.vertices[3*i0 + 1], mesh.vertices[3*i0 + 2] }, model.transform);
        Vector3 b = Vector3Transform((Vector3){ mesh.vertices[3*i1], mesh.vertices[3*i1 + 1], mesh.vertices[3*i1 + 2] }, model.transform);
        Vector3 c = Vector3Transform((Vector3){ mesh.vertices[3*i2], mesh.vertices[3*i2 + 1], mesh.vertices[3*i2 + 2] }, model.transform);

        RayCollision hit = GetRayCollisionTriangle(ray, a, b, c);

        if (hit.hit && (hit.distance < closestDistance))
        {
            closestDistance = hit.distance;
            found = true;

            Vector2 uvA = { mesh.texcoords[2*i0], mesh.texcoords[2*i0 + 1] };
            Vector2 uvB = { mesh.texcoords[2*i1], mesh.texcoords[2*i1 + 1] };
            Vector2 uvC = { mesh.texcoords[2*i2], mesh.texcoords[2*i2 + 1] };

            Vector3 w = Vector3Barycenter(hit.point, a, b, c);

            hitUV.x = w.x*uvA.x + w.y*uvB.x + w.z*uvC.x;
            hitUV.y = w.x*uvA.y + w.y*uvB.y + w.z*uvC.y;

            // Wrap into [0, 1] in case of minor floating point drift at UV seams
            hitUV.x -= floorf(hitUV.x);
            hitUV.y -= floorf(hitUV.y);
        }
    }

    if (found && (outUV != NULL)) *outUV = hitUV;

    return found;
}
