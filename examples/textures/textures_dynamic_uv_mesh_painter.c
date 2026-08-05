/*******************************************************************************************
*
*   raylib [textures] example - 3D dynamic UV mesh painter
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   NOTE: Raycasts against 3D mesh geometry to interpolate UV coordinates using barycentric
*   coordinates, then dynamically renders continuous paint strokes into an in-memory canvas
*   texture mapped onto the model material.
*
*   Example contributed by PanicTitan and reviewed by Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>     // Required for: NULL
#include <string.h>     // Required for: memset()
#include <math.h>       // Required for: fabsf(), floorf()

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//--------------------------------------------------------------------------------------
// Global Definitions
//--------------------------------------------------------------------------------------
#define CANVAS_SIZE         512
#define MAX_PALETTE_COLORS  8

//--------------------------------------------------------------------------------------
// Types and Structures Definition
//--------------------------------------------------------------------------------------
typedef enum ToolMode {
    TOOL_PAINT = 0,
    TOOL_COLOR_PICKER
} ToolMode;

typedef enum ShapeType {
    SHAPE_SPHERE = 0,
    SHAPE_CUBE,
    SHAPE_CYLINDER,
    SHAPE_TORUS
} ShapeType;

typedef struct AppContext {
    Camera camera;                      // 3D camera positioning
    Model model;                        // Current active 3D model
    Image canvasImage;                  // CPU RAM pixel buffer
    Texture2D canvasTexture;            // GPU VRAM texture bound to model material
    BoundingBox modelBBox;              // Transformed bounding box for fast ray testing

    // Painter tool state
    ToolMode currentTool;               // Active tool mode
    ShapeType currentShape;             // Active mesh shape
    Color activeColor;                  // Selected drawing color
    int brushRadius;                    // Brush stroke radius (in pixels)

    // Stroke interpolation state
    Vector2 lastHitUV;                  // Last valid UV hit coordinate
    bool hasLastHit;                    // Stroke continuity flag

    // Swatch palette
    Color palette[MAX_PALETTE_COLORS];  // Palette color array
    int paletteCount;                   // Total color swatches
} AppContext;

//--------------------------------------------------------------------------------------
// Module Global Variables
//--------------------------------------------------------------------------------------
static AppContext g_App = { 0 };

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
static Vector3 CalculateBarycentric(Vector3 p, Vector3 a, Vector3 b, Vector3 c);
static bool GetMeshHitUV(Ray ray, Model model, Vector2 *outUV);
static void ChangeMeshShape(ShapeType newShape);
static bool DrawCustomButton(Rectangle rec, const char *label, bool active);

void UpdateDrawFrame(void);             // Main frame loop (web & desktop)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 960;
    const int screenHeight = 580;

    // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - 3d dynamic mesh painter");

    // Camera setup (Zoomed out to comfortably frame 3D primitives)
    g_App.camera.position = (Vector3){ 0.0f, 3.5f, 6.5f };
    g_App.camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    g_App.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    g_App.camera.fovy = 45.0f;
    g_App.camera.projection = CAMERA_PERSPECTIVE;

    // Initialize CPU canvas buffer and dynamic GPU texture
    g_App.canvasImage = GenImageColor(CANVAS_SIZE, CANVAS_SIZE, RAYWHITE);
    ImageDrawRectangleLines(&g_App.canvasImage, 0, 0, CANVAS_SIZE, CANVAS_SIZE, LIGHTGRAY);

    g_App.canvasTexture = LoadTextureFromImage(g_App.canvasImage);
    SetTextureFilter(g_App.canvasTexture, TEXTURE_FILTER_BILINEAR);

    // Generate starting 3D primitive mesh
    ChangeMeshShape(SHAPE_SPHERE);

    // Default painter configuration
    g_App.currentTool = TOOL_PAINT;
    g_App.activeColor = RED;
    g_App.brushRadius = 12;
    g_App.hasLastHit = false;

    // Palette swatches initialization
    g_App.palette[0] = RED;
    g_App.palette[1] = ORANGE;
    g_App.palette[2] = GOLD;
    g_App.palette[3] = LIME;
    g_App.palette[4] = SKYBLUE;
    g_App.palette[5] = PURPLE;
    g_App.palette[6] = DARKGRAY;
    g_App.palette[7] = WHITE;
    g_App.paletteCount = MAX_PALETTE_COLORS;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(g_App.model);
    UnloadImage(g_App.canvasImage);
    UnloadTexture(g_App.canvasTexture);
    CloseWindow();
    //--------------------------------------------------------------------------------------
#endif

    return 0;
}

//--------------------------------------------------------------------------------------
// Frame Update and Render Logic
//--------------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    Vector2 mousePos = GetMousePosition();

    // UI Panel collision boundary check
    Rectangle uiPanelRec = (Rectangle){ 10.0f, 10.0f, 230.0f, 490.0f };
    bool isMouseOverUI = CheckCollisionPointRec(mousePos, uiPanelRec);

    // Orbit camera controls (Right Mouse Button Drag)
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        UpdateCamera(&g_App.camera, CAMERA_THIRD_PERSON);
    }

    // 3D Painting & Color Picker Processing
    if (!isMouseOverUI && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Ray ray = GetScreenToWorldRay(mousePos, g_App.camera);
        Vector2 hitUV = { 0 };

        if (GetMeshHitUV(ray, g_App.model, &hitUV))
        {
            int imgW = g_App.canvasImage.width;
            int imgH = g_App.canvasImage.height;

            int px = (int)(hitUV.x * (float)imgW);
            int py = (int)(hitUV.y * (float)imgH);

            if (g_App.currentTool == TOOL_PAINT)
            {
                float du = fabsf(hitUV.x - g_App.lastHitUV.x);
                float dv = fabsf(hitUV.y - g_App.lastHitUV.y);

                // Seam jump guard: prevent continuous stroke rendering across UV boundaries
                if (g_App.hasLastHit && (du < 0.25f) && (dv < 0.25f))
                {
                    int lastPx = (int)(g_App.lastHitUV.x * (float)imgW);
                    int lastPy = (int)(g_App.lastHitUV.y * (float)imgH);

                    float dist = Vector2Distance((Vector2){ (float)lastPx, (float)lastPy }, (Vector2){ (float)px, (float)py });
                    int steps = (int)(dist / 2.0f) + 1;

                    for (int i = 0; i <= steps; i++)
                    {
                        float t = (float)i / (float)steps;
                        int cx = (int)Lerp((float)lastPx, (float)px, t);
                        int cy = (int)Lerp((float)lastPy, (float)py, t);

                        ImageDrawCircle(&g_App.canvasImage, cx, cy, g_App.brushRadius, g_App.activeColor);
                    }
                }
                else
                {
                    ImageDrawCircle(&g_App.canvasImage, px, py, g_App.brushRadius, g_App.activeColor);
                }

                UpdateTexture(g_App.canvasTexture, g_App.canvasImage.data);

                g_App.lastHitUV = hitUV;
                g_App.hasLastHit = true;
            }
            else if (g_App.currentTool == TOOL_COLOR_PICKER)
            {
                g_App.activeColor = GetImageColor(g_App.canvasImage, px, py);
                g_App.currentTool = TOOL_PAINT;
                g_App.hasLastHit = false;
            }
        }
        else
        {
            g_App.hasLastHit = false;
        }
    }
    else
    {
        g_App.hasLastHit = false;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground((Color){ 30, 32, 40, 255 });

        // 3D Scene Rendering
        BeginMode3D(g_App.camera);

            DrawModel(g_App.model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
            DrawGrid(10, 1.0f);

        EndMode3D();

        // 2D Controls & UI Panel
        DrawRectangleRec(uiPanelRec, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(uiPanelRec, 2.0f, DARKGRAY);

        DrawText("3D MESH PAINTER", 25, 22, 18, GOLD);

        // Tool Mode Toggles
        if (DrawCustomButton((Rectangle){ 25, 55, 95, 32 }, "PAINT", g_App.currentTool == TOOL_PAINT))
        {
            g_App.currentTool = TOOL_PAINT;
        }

        if (DrawCustomButton((Rectangle){ 125, 55, 95, 32 }, "PICKER", g_App.currentTool == TOOL_COLOR_PICKER))
        {
            g_App.currentTool = TOOL_COLOR_PICKER;
        }

        // Primitive Shape Selector
        DrawText("Mesh Shape:", 25, 100, 12, LIGHTGRAY);
        if (DrawCustomButton((Rectangle){ 25, 120, 95, 28 }, "SPHERE", g_App.currentShape == SHAPE_SPHERE))
        {
            ChangeMeshShape(SHAPE_SPHERE);
        }
        if (DrawCustomButton((Rectangle){ 125, 120, 95, 28 }, "CUBE", g_App.currentShape == SHAPE_CUBE))
        {
            ChangeMeshShape(SHAPE_CUBE);
        }
        if (DrawCustomButton((Rectangle){ 25, 153, 95, 28 }, "CYLINDER", g_App.currentShape == SHAPE_CYLINDER))
        {
            ChangeMeshShape(SHAPE_CYLINDER);
        }
        if (DrawCustomButton((Rectangle){ 125, 153, 95, 28 }, "TORUS", g_App.currentShape == SHAPE_TORUS))
        {
            ChangeMeshShape(SHAPE_TORUS);
        }

        // Active Color Display
        DrawText("Active Color:", 25, 195, 12, LIGHTGRAY);
        DrawRectangle(125, 193, 95, 20, g_App.activeColor);
        DrawRectangleLines(125, 193, 95, 20, WHITE);

        // Color Palette Grid
        DrawText("Palette Swatches:", 25, 225, 12, LIGHTGRAY);
        for (int i = 0; i < g_App.paletteCount; i++)
        {
            int sx = 25 + (i % 4) * 48;
            int sy = 245 + (i / 4) * 45;
            Rectangle swatchRec = (Rectangle){ (float)sx, (float)sy, 40.0f, 38.0f };

            DrawRectangleRec(swatchRec, g_App.palette[i]);
            DrawRectangleLinesEx(swatchRec, 1.0f, WHITE);

            if (CheckCollisionPointRec(mousePos, swatchRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                g_App.activeColor = g_App.palette[i];
            }
        }

        // Brush Size Adjustments
        DrawText(TextFormat("Brush Size: %dpx", g_App.brushRadius), 25, 345, 12, LIGHTGRAY);
        if (DrawCustomButton((Rectangle){ 25, 365, 95, 30 }, "SIZE -", false))
        {
            if (g_App.brushRadius > 2) g_App.brushRadius -= 2;
        }
        if (DrawCustomButton((Rectangle){ 125, 365, 95, 30 }, "SIZE +", false))
        {
            if (g_App.brushRadius < 64) g_App.brushRadius += 2;
        }

        // Canvas Actions
        if (DrawCustomButton((Rectangle){ 25, 410, 95, 30 }, "CLEAR", false))
        {
            ImageClearBackground(&g_App.canvasImage, RAYWHITE);
            ImageDrawRectangleLines(&g_App.canvasImage, 0, 0, CANVAS_SIZE, CANVAS_SIZE, LIGHTGRAY);
            UpdateTexture(g_App.canvasTexture, g_App.canvasImage.data);
        }

        if (DrawCustomButton((Rectangle){ 125, 410, 95, 30 }, "FILL", false))
        {
            ImageClearBackground(&g_App.canvasImage, g_App.activeColor);
            UpdateTexture(g_App.canvasTexture, g_App.canvasImage.data);
        }

        // Instructions Footer & Overlay Info
        DrawText("Left Click: Paint / Pick Color | Right Click Drag: Orbit Camera", 260, screenHeight - 25, 12, RAYWHITE);
        DrawFPS(screenWidth - 90, 15);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

//--------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------

// Dynamic Primitive Mesh Generator
static void ChangeMeshShape(ShapeType newShape)
{
    if (g_App.model.meshCount > 0)
    {
        UnloadModel(g_App.model);
    }

    Mesh mesh = { 0 };

    switch (newShape)
    {
        case SHAPE_SPHERE:   mesh = GenMeshSphere(1.5f, 48, 48); break;
        case SHAPE_CUBE:     mesh = GenMeshCube(2.2f, 2.2f, 2.2f); break;
        case SHAPE_CYLINDER: mesh = GenMeshCylinder(1.2f, 2.5f, 36); break;
        case SHAPE_TORUS:    mesh = GenMeshTorus(0.6f, 1.6f, 32, 48); break;
        default:             mesh = GenMeshSphere(1.5f, 48, 48); break;
    }

    g_App.model = LoadModelFromMesh(mesh);

    // GenMeshCylinder builds upwards from Y=0 to Y=height (2.5). Translate down by height/2 to center at origin.
    if (newShape == SHAPE_CYLINDER)
    {
        g_App.model.transform = MatrixTranslate(0.0f, -1.25f, 0.0f);
    }

    g_App.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = g_App.canvasTexture;

    // Compute bounding box and apply model transformation
    BoundingBox bbox = GetMeshBoundingBox(g_App.model.meshes[0]);
    bbox.min = Vector3Transform(bbox.min, g_App.model.transform);
    bbox.max = Vector3Transform(bbox.max, g_App.model.transform);
    g_App.modelBBox = bbox;

    g_App.currentShape = newShape;
    g_App.hasLastHit = false;
}

// Computes Barycentric Weights for UV Interpolation inside a 3D Triangle
static Vector3 CalculateBarycentric(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 v0 = Vector3Subtract(b, a);
    Vector3 v1 = Vector3Subtract(c, a);
    Vector3 v2 = Vector3Subtract(p, a);

    float d00 = Vector3DotProduct(v0, v0);
    float d01 = Vector3DotProduct(v0, v1);
    float d11 = Vector3DotProduct(v1, v1);
    float d20 = Vector3DotProduct(v2, v0);
    float d21 = Vector3DotProduct(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    if (fabsf(denom) < 1e-6f) return (Vector3){ 0.33f, 0.33f, 0.34f };

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return (Vector3){ u, v, w };
}

// Raycasts against the model mesh and calculates exact UV texture space coordinates
static bool GetMeshHitUV(Ray ray, Model model, Vector2 *outUV)
{
    if (model.meshCount == 0 || model.meshes[0].texcoords == NULL) return false;

    // Fast bounding box pre-pass using transformed bounding box
    RayCollision boxHit = GetRayCollisionBox(ray, g_App.modelBBox);
    if (!boxHit.hit) return false;

    Mesh mesh = model.meshes[0];
    float closestDistance = 999999.0f;
    bool foundHit = false;
    Vector2 interpolatedUV = { 0 };

    for (int tri = 0; tri < mesh.triangleCount; tri++)
    {
        int idx0, idx1, idx2;

        if (mesh.indices != NULL)
        {
            idx0 = mesh.indices[3 * tri + 0];
            idx1 = mesh.indices[3 * tri + 1];
            idx2 = mesh.indices[3 * tri + 2];
        }
        else
        {
            idx0 = 3 * tri + 0;
            idx1 = 3 * tri + 1;
            idx2 = 3 * tri + 2;
        }

        Vector3 a = Vector3Transform((Vector3){ mesh.vertices[3 * idx0 + 0], mesh.vertices[3 * idx0 + 1], mesh.vertices[3 * idx0 + 2] }, model.transform);
        Vector3 b = Vector3Transform((Vector3){ mesh.vertices[3 * idx1 + 0], mesh.vertices[3 * idx1 + 1], mesh.vertices[3 * idx1 + 2] }, model.transform);
        Vector3 c = Vector3Transform((Vector3){ mesh.vertices[3 * idx2 + 0], mesh.vertices[3 * idx2 + 1], mesh.vertices[3 * idx2 + 2] }, model.transform);

        // Backface culling check: ignore triangles oriented away from camera ray
        Vector3 edge1 = Vector3Subtract(b, a);
        Vector3 edge2 = Vector3Subtract(c, a);
        Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

        if (Vector3DotProduct(normal, ray.direction) >= 0.0f) continue;

        RayCollision triHit = GetRayCollisionTriangle(ray, a, b, c);

        if (triHit.hit && triHit.distance < closestDistance)
        {
            closestDistance = triHit.distance;
            foundHit = true;

            Vector2 uvA = (Vector2){ mesh.texcoords[2 * idx0 + 0], mesh.texcoords[2 * idx0 + 1] };
            Vector2 uvB = (Vector2){ mesh.texcoords[2 * idx1 + 0], mesh.texcoords[2 * idx1 + 1] };
            Vector2 uvC = (Vector2){ mesh.texcoords[2 * idx2 + 0], mesh.texcoords[2 * idx2 + 1] };

            Vector3 weights = CalculateBarycentric(triHit.point, a, b, c);

            interpolatedUV.x = weights.x * uvA.x + weights.y * uvB.x + weights.z * uvC.x;
            interpolatedUV.y = weights.x * uvA.y + weights.y * uvB.y + weights.z * uvC.y;

            // Wrap UV values within [0.0..1.0] range
            interpolatedUV.x = interpolatedUV.x - floorf(interpolatedUV.x);
            interpolatedUV.y = interpolatedUV.y - floorf(interpolatedUV.y);
        }
    }

    if (foundHit && (outUV != NULL))
    {
        *outUV = interpolatedUV;
    }

    return foundHit;
}

// Custom GUI Button Control
static bool DrawCustomButton(Rectangle rec, const char *label, bool active)
{
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rec);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color btnColor = active ? GOLD : (hovered ? LIGHTGRAY : GRAY);
    Color textColor = active ? BLACK : (hovered ? BLACK : RAYWHITE);

    DrawRectangleRec(rec, btnColor);
    DrawRectangleLinesEx(rec, 1.5f, DARKGRAY);

    int fontSize = 10;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label, (int)(rec.x + rec.width * 0.5f - textWidth * 0.5f), (int)(rec.y + rec.height * 0.5f - fontSize * 0.5f), fontSize, textColor);

    return clicked;
}