/*******************************************************************************************
Alexey Karnachev License
---------------------------------------------------------------------------------
MIT License

Copyright (c) 2023 Alexey Karnachev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
---------------------------------------------------------------------------------

Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)

This software is provided "as-is", without any express or implied warranty. In no event
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you
  wrote the original software. If you use this software in a product, an acknowledgment
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.

**********************************************************************************************/

#include <raylib.h>

void loadGizmo();
void unloadGizmo();
Matrix updateAndDrawGizmo(Camera3D camera, Vector3 position);

#ifdef RAYGIZMO_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <rlgl.h>
#include "raymath.h"

// This will be multiplied by the distance from the camera to the gizmo,
// Which keeps the screen-space gizmo size constant.
#define GIZMO_SIZE 0.12f

// Line drawing thicknesses
#define GIZMO_HANDLE_DRAW_THICKNESS 5.0f
#define GIZMO_ACTIVE_AXIS_DRAW_THICKNESS 2.0f

// These sizes are relative to the gizmo radius
#define GIZMO_AXIS_HANDLE_LENGTH 1.2f;
#define GIZMO_AXIS_HANDLE_TIP_LENGTH 0.3f;
#define GIZMO_AXIS_HANDLE_TIP_RADIUS 0.1f;
#define GIZMO_PLANE_HANDLE_OFFSET 0.4f;
#define GIZMO_PLANE_HANDLE_SIZE 0.2f;

#define X_AXIS (Vector3){ 1.0, 0.0, 0.0 }
#define Y_AXIS (Vector3){ 0.0, 1.0, 0.0 }
#define Z_AXIS (Vector3){ 0.0, 0.0, 1.0 }

// Size of the additional buffer to pre-render the gizmo into.
// Used for the mouse picking.
#define MASK_FRAMEBUFFER_WIDTH 500
#define MASK_FRAMEBUFFER_HEIGHT 500

#define ID_TO_RED_COLOR(id) (Color){ id, 0, 0, 0 }

#define SWAP(x, y) \
    do { \
        unsigned char \
            swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp, &y, sizeof(x)); \
        memcpy(&y, &x, sizeof(x)); \
        memcpy(&x, swap_temp, sizeof(x)); \
    } while (0)


#if defined(PLATFORM_DESKTOP)  // Shaders for PLATFORM_DESKTOP
static const char* SHADER_COLOR_VERT = "\
#version 330\n\
in vec3 vertexPosition; \
in vec4 vertexColor; \
out vec4 fragColor; \
out vec3 fragPosition; \
uniform mat4 mvp; \
void main() \
{ \
    fragColor = vertexColor; \
    fragPosition = vertexPosition; \
    gl_Position = mvp * vec4(vertexPosition, 1.0); \
} \
";

static const char* shaderRotHandleColor_FRAG = "\
#version 330\n\
in vec4 fragColor; \
in vec3 fragPosition; \
uniform vec3 cameraPosition; \
uniform vec3 gizmoPosition; \
out vec4 finalColor; \
void main() \
{ \
    vec3 r = normalize(fragPosition - gizmoPosition); \
    vec3 c = normalize(fragPosition - cameraPosition); \
    if (dot(r, c) > 0.1) discard; \
    finalColor = fragColor; \
} \
";

#else  // Shaders for PLATFORM_ANDROID, PLATFORM_WEB

static const char* SHADER_COLOR_VERT = "\
#version 100\n\
attribute vec3 vertexPosition; \
attribute vec4 vertexColor; \
varying vec4 fragColor; \
varying vec3 fragPosition; \
uniform mat4 mvp; \
void main() \
{ \
    fragColor = vertexColor; \
    fragPosition = vertexPosition; \
    gl_Position = mvp * vec4(vertexPosition, 1.0); \
} \
";

static const char* shaderRotHandleColor_FRAG = "\
#version 100\n\
precision mediump float; \
varying vec4 fragColor; \
varying vec3 fragPosition; \
uniform vec3 cameraPosition; \
uniform vec3 gizmoPosition; \
void main() { \
    vec3 r = normalize(fragPosition - gizmoPosition); \
    vec3 c = normalize(fragPosition - cameraPosition); \
    if (dot(r, c) > 0.1) discard; \
    gl_FragColor = fragColor; \
} \
";
#endif

static Shader shaderRotHandleColor;
static int ShaderRotHandleCameraPosLoc;
static int ShaderRotHandleGizmoPosLoc;

static unsigned int maskFramebuffer;
static unsigned int maskTexture;

static bool isGizmoLoaded;
static Vector3 gizmoCurrentAxis;

typedef enum HandleId {
    HANDLE_X,

    ROT_HANDLE_X,
    AXIS_HANDLE_X,
    PLANE_HANDLE_X,

    HANDLE_Y,

    ROT_HANDLE_Y,
    AXIS_HANDLE_Y,
    PLANE_HANDLE_Y,

    HANDLE_Z,

    ROT_HANDLE_Z,
    AXIS_HANDLE_Z,
    PLANE_HANDLE_Z
} HandleId;

typedef enum GizmoState {
    GIZMO_COLD,

    GIZMO_HOT,

    GIZMO_HOT_ROT,
    GIZMO_HOT_AXIS,
    GIZMO_HOT_PLANE,

    GIZMO_ACTIVE,

    GIZMO_ACTIVE_ROT,
    GIZMO_ACTIVE_AXIS,
    GIZMO_ACTIVE_PLANE,
} GizmoState;

typedef struct Handle {
    Vector3 position;
    Vector3 axis;
    Color color;
    float distToCamera;
} Handle;

typedef struct HandleColors {
    Color x;
    Color y;
    Color z;
} HandleColors;

typedef struct Handles {
    Handle arr[3];
} Handles;

static GizmoState gizmoState;

static bool checkIfMouseMoved() {
    Vector2 mouseDelta = GetMouseDelta();
    return (fabs(mouseDelta.x) + fabs(mouseDelta.y)) > EPSILON;
}

static float getAngleBetweenTwoVector2(Vector2 v1, Vector2 v2) {
    Vector2 v1Normal = Vector2Normalize(v1);
    Vector2 v2Normal = Vector2Normalize(v2);
    float dot = Vector2DotProduct(v1Normal, v2Normal);
    if (1.0 - fabs(dot) < EPSILON) return 0.0;

    float angle = acos(dot);
    float z = v1.x * v2.y - v1.y * v2.x;

    if (fabs(z) < EPSILON) return 0.0;
    else if (z > 0) return angle;
    else return -angle;
}

static int intersectLinePlane(
    Vector3* outPoint,
    Vector3 linePoint0,
    Vector3 linePoint1,
    Vector3 planePoint,
    Vector3 planeNormal
) {
    Vector3 u = Vector3Subtract(linePoint1, linePoint0);
    float dot = Vector3DotProduct(planeNormal, u);
    if (fabs(dot) <= EPSILON) return 0;

    Vector3 w = Vector3Subtract(linePoint0, planePoint);
    float k = -Vector3DotProduct(planeNormal, w) / dot;
    u = Vector3Scale(u, k);
    *outPoint = Vector3Add(linePoint0, u);
    return 1;
}

static int getTwoLinesNearestPoint(
    Vector3* outPoint,
    Vector3 line0Point0,
    Vector3 line0Point1,
    Vector3 line1Point0,
    Vector3 line1Point1
) {
    Vector3 vec0 = Vector3Subtract(line0Point1, line0Point0);
    Vector3 vec1 = Vector3Subtract(line1Point1, line1Point0);
    Vector3 planeVec = Vector3Normalize(Vector3CrossProduct(vec0, vec1));
    Vector3 planeNormal = Vector3Normalize(Vector3CrossProduct(vec0, planeVec));

    int isIntersected = intersectLinePlane(
        outPoint, line1Point0, line1Point1, line0Point0, planeNormal
    );

    return isIntersected;
}

static RayCollision getRayPlaneCollision(
    Ray ray, Vector3 planePoint, Vector3 planeNormal
) {
    RayCollision collision = {0};

    // Calculate the parameter t
    float denominator = ray.direction.x * planeNormal.x
                        + ray.direction.y * planeNormal.y
                        + ray.direction.z * planeNormal.z;

    // Ray is parallel to the plane, no collision
    if (denominator == 0) return collision;

    float t = ((planePoint.x - ray.position.x) * planeNormal.x
               + (planePoint.y - ray.position.y) * planeNormal.y
               + (planePoint.z - ray.position.z) * planeNormal.z)
              / denominator;

    // Intersection point is behind the ray's starting point, no collision
    if (t < 0) return collision;

    // Calculate the collision point
    collision.point.x = ray.position.x + t * ray.direction.x;
    collision.point.y = ray.position.y + t * ray.direction.y;
    collision.point.z = ray.position.z + t * ray.direction.z;
    collision.hit = true;

    return collision;
}

static Handles getSortedHandles(Handle h0, Handle h1, Handle h2) {
    if (h0.distToCamera < h1.distToCamera) SWAP(h0, h1);
    if (h1.distToCamera < h2.distToCamera) SWAP(h1, h2);
    if (h0.distToCamera < h1.distToCamera) SWAP(h0, h1);

    Handles handles = {.arr = {h0, h1, h2}};
    return handles;
}

static HandleColors getHandleColors(GizmoState hotState) {
    bool isHot = gizmoState == hotState || gizmoState == hotState + 4;
    Color x = isHot && gizmoCurrentAxis.x == 1.0f ? WHITE : RED;
    Color y = isHot && gizmoCurrentAxis.y == 1.0f ? WHITE : GREEN;
    Color z = isHot && gizmoCurrentAxis.z == 1.0f ? WHITE : BLUE;
    HandleColors colors = {x, y, z};
    return colors;
}

static void drawAxisHandles(
    Camera3D camera,
    Vector3 position,
    float gizmoRadius,
    Color colorX,
    Color colorY,
    Color colorZ
) {
    float length = gizmoRadius * GIZMO_AXIS_HANDLE_LENGTH;
    float tipLength = gizmoRadius * GIZMO_AXIS_HANDLE_TIP_LENGTH;
    float tipRadius = gizmoRadius * GIZMO_AXIS_HANDLE_TIP_RADIUS;

    Vector3 px = Vector3Add(position, Vector3Scale(X_AXIS, length));
    Vector3 py = Vector3Add(position, Vector3Scale(Y_AXIS, length));
    Vector3 pz = Vector3Add(position, Vector3Scale(Z_AXIS, length));

    Handle hx = {px, X_AXIS, colorX, Vector3DistanceSqr(px, camera.position)};
    Handle hy = {py, Y_AXIS, colorY, Vector3DistanceSqr(py, camera.position)};
    Handle hz = {pz, Z_AXIS, colorZ, Vector3DistanceSqr(pz, camera.position)};

    Handles handles = getSortedHandles(hx, hy, hz);

    for (int i = 0; i < 3; ++i) {
        Handle* h = &handles.arr[i];
        Vector3 tipEnd = Vector3Add(
            h->position, Vector3Scale(h->axis, tipLength)
        );
        DrawLine3D(position, h->position, h->color);
        DrawCylinderEx(h->position, tipEnd, tipRadius, 0.0f, 16, h->color);
    }
}

static void drawPlaneHandles(
    Camera3D camera,
    Vector3 position,
    float gizmoRadius,
    Color colorX,
    Color colorY,
    Color colorZ
) {
    float offset = gizmoRadius * GIZMO_PLANE_HANDLE_OFFSET;
    float size = gizmoRadius * GIZMO_PLANE_HANDLE_SIZE;

    Vector3 px = Vector3Add(position, (Vector3){0.0f, offset, offset});
    Vector3 py = Vector3Add(position, (Vector3){offset, 0.0f, offset});
    Vector3 pz = Vector3Add(position, (Vector3){offset, offset, 0.0f});

    Handle hx = {px, Z_AXIS, colorX, Vector3DistanceSqr(px, camera.position)};
    Handle hy = {py, Y_AXIS, colorY, Vector3DistanceSqr(py, camera.position)};
    Handle hz = {pz, X_AXIS, colorZ, Vector3DistanceSqr(pz, camera.position)};

    Handles handles = getSortedHandles(hx, hy, hz);

    rlDisableBackfaceCulling();
    for (int i = 0; i < 3; ++i) {
        Handle* h = &handles.arr[i];
        rlPushMatrix();
            rlTranslatef(h->position.x, h->position.y, h->position.z);
            rlRotatef(90.0f, h->axis.x, h->axis.y, h->axis.z);
            DrawPlane(
                Vector3Zero(), Vector2Scale(Vector2One(), size), h->color
            );
        rlPopMatrix();
    }
}

static void drawRotHandles(
    Camera3D camera,
    Vector3 position,
    float gizmoRadius,
    Color colorX,
    Color colorY,
    Color colorZ
) {
    BeginShaderMode(shaderRotHandleColor);
        SetShaderValue(
            shaderRotHandleColor,
            ShaderRotHandleCameraPosLoc,
            &camera.position,
            SHADER_UNIFORM_VEC3
        );
        SetShaderValue(
            shaderRotHandleColor,
            ShaderRotHandleGizmoPosLoc,
            &position,
            SHADER_UNIFORM_VEC3
        );
        DrawCircle3D(position, gizmoRadius, Y_AXIS, 90.0f, colorX);
        DrawCircle3D(position, gizmoRadius, X_AXIS, 90.0f, colorY);
        DrawCircle3D(position, gizmoRadius, X_AXIS, 0.0f, colorZ);
    EndShaderMode();
}

static void drawGizmo(
    Camera3D camera,
    Vector3 position,
    Color rotHandleColorX,
    Color rotHandleColorY,
    Color rotHandleColorZ,
    Color axisHandleColorX,
    Color axisHandleColorY,
    Color axisHandleColorZ,
    Color planeHandleColorX,
    Color planeHandleColorY,
    Color planeHandleColorZ
) {
    float radius = GIZMO_SIZE * Vector3Distance(camera.position, position);

    // Draw gizmo's handle elements
    BeginMode3D(camera);
        rlSetLineWidth(GIZMO_HANDLE_DRAW_THICKNESS);
        rlDisableDepthTest();

        drawPlaneHandles(
            camera,
            position,
            radius,
            planeHandleColorX,
            planeHandleColorY,
            planeHandleColorZ
        );

        drawRotHandles(
            camera,
            position,
            radius,
            rotHandleColorX,
            rotHandleColorY,
            rotHandleColorZ
        );

        drawAxisHandles(
            camera,
            position,
            radius,
            axisHandleColorX,
            axisHandleColorY,
            axisHandleColorZ
        );
    EndMode3D();

    // Draw long white line which represents current active axis
    if (gizmoState == GIZMO_ACTIVE_ROT || gizmoState == GIZMO_ACTIVE_AXIS) {
        BeginMode3D(camera);
            rlSetLineWidth(GIZMO_ACTIVE_AXIS_DRAW_THICKNESS);
            Vector3 halfAxisLine = Vector3Scale(gizmoCurrentAxis, 1000.0f);
            DrawLine3D(
                Vector3Subtract(position, halfAxisLine),
                Vector3Add(position, halfAxisLine),
                WHITE
            );
        EndMode3D();
    }

    // Draw white line from the gizmo's center to the mouse cursor when rotating
    if (gizmoState == GIZMO_ACTIVE_ROT) {
        rlSetLineWidth(GIZMO_ACTIVE_AXIS_DRAW_THICKNESS);
        DrawLineV(
            GetWorldToScreen(position, camera), GetMousePosition(), WHITE
        );
    }
}

static unsigned char getGizmoMaskPixel(Camera3D camera, Vector3 position) {
    // -------------------------------------------------------------------
    // Draw gizmo into a separate framebuffer for the mouse pixel-picking
    rlEnableFramebuffer(maskFramebuffer);
    rlViewport(0, 0, MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT);
    rlClearScreenBuffers();
    rlDisableColorBlend();

    drawGizmo(
        camera,
        position,
        ID_TO_RED_COLOR(ROT_HANDLE_X),
        ID_TO_RED_COLOR(ROT_HANDLE_Y),
        ID_TO_RED_COLOR(ROT_HANDLE_Z),
        ID_TO_RED_COLOR(AXIS_HANDLE_X),
        ID_TO_RED_COLOR(AXIS_HANDLE_Y),
        ID_TO_RED_COLOR(AXIS_HANDLE_Z),
        ID_TO_RED_COLOR(PLANE_HANDLE_X),
        ID_TO_RED_COLOR(PLANE_HANDLE_Y),
        ID_TO_RED_COLOR(PLANE_HANDLE_Z)
    );

    rlDisableFramebuffer();
    rlEnableColorBlend();
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

    // -------------------------------------------------------------------
    // Pick the pixel under the mouse cursor
    Vector2 mousePosition = GetMousePosition();
    unsigned char* pixels = (unsigned char*)rlReadTexturePixels(
        maskTexture,
        MASK_FRAMEBUFFER_WIDTH,
        MASK_FRAMEBUFFER_HEIGHT,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float xFract = Clamp(mousePosition.x / (float)GetScreenWidth(), 0.0, 1.0);
    float yFract = Clamp(
        1.0 - (mousePosition.y / (float)GetScreenHeight()), 0.0, 1.0
    );
    int x = (int)(MASK_FRAMEBUFFER_WIDTH * xFract);
    int y = (int)(MASK_FRAMEBUFFER_HEIGHT * yFract);
    int idx = 4 * (y * MASK_FRAMEBUFFER_WIDTH + x);
    unsigned char maskVal = pixels[idx];

    free(pixels);
    return maskVal;
}

static Matrix updateGizmoRot(Camera3D camera, Vector3 position) {
    if (!checkIfMouseMoved()) return MatrixIdentity();

    Vector2 posOnScreen = GetWorldToScreen(position, camera);
    Vector2 p1 = Vector2Subtract(GetMousePosition(), posOnScreen);
    Vector2 p0 = Vector2Subtract(p1, GetMouseDelta());
    float angle = getAngleBetweenTwoVector2(p1, p0);

    // If we look at the gizmo from behind, we should flip the rotation
    if (Vector3DotProduct(gizmoCurrentAxis, position)
        > Vector3DotProduct(gizmoCurrentAxis, camera.position)) {
        angle *= -1;
    }

    // We rotate gizmo relative to it's own center, so we first translate it to
    // the center of world coordinates, then rotate and then translate back
    Matrix transform = MatrixMultiply(
        MatrixMultiply(
            MatrixTranslate(-position.x, -position.y, -position.z),
            MatrixRotate(gizmoCurrentAxis, angle)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );

    return transform;
}

static Matrix updateGizmoAxis(Camera3D camera, Vector3 position) {
    if (!checkIfMouseMoved()) return MatrixIdentity();

    Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
    Ray r = GetMouseRay(p, camera);
    Vector3 interesctionPoint = Vector3Zero();
    bool isIntersected = getTwoLinesNearestPoint(
        &interesctionPoint,
        camera.position,
        Vector3Add(camera.position, r.direction),
        position,
        Vector3Add(position, gizmoCurrentAxis)
    );

    if (!isIntersected) return MatrixIdentity();

    Vector3 offset = Vector3Subtract(interesctionPoint, position);
    offset = Vector3Multiply(offset, gizmoCurrentAxis);
    Matrix transform = MatrixTranslate(offset.x, offset.y, offset.z);

    return transform;
}

static Matrix updateGizmoPlane(Camera3D camera, Vector3 position) {
    if (!checkIfMouseMoved()) return MatrixIdentity();

    Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
    Ray r = GetMouseRay(p, camera);
    RayCollision c = getRayPlaneCollision(r, position, gizmoCurrentAxis);
    Vector3 offset = Vector3Subtract(c.point, position);
    Matrix transform = MatrixTranslate(offset.x, offset.y, offset.z);

    return transform;
}

static Matrix updateGizmo(Camera3D camera, Vector3 position, unsigned char maskVal) {
    bool isLMBDown = IsMouseButtonDown(0);

    if (!isLMBDown) gizmoState = GIZMO_COLD;

    if (gizmoState < GIZMO_ACTIVE) {
        if (maskVal < HANDLE_Y) gizmoCurrentAxis = X_AXIS;
        else if (maskVal < HANDLE_Z) gizmoCurrentAxis = Y_AXIS;
        else gizmoCurrentAxis = Z_AXIS;

        if (maskVal % 4 == 1)
            gizmoState = isLMBDown ? GIZMO_ACTIVE_ROT : GIZMO_HOT_ROT;
        else if (maskVal % 4 == 2)
            gizmoState = isLMBDown ? GIZMO_ACTIVE_AXIS : GIZMO_HOT_AXIS;
        else if (maskVal % 4 == 3)
            gizmoState = isLMBDown ? GIZMO_ACTIVE_PLANE : GIZMO_HOT_PLANE;
    }

    switch (gizmoState) {
        case GIZMO_ACTIVE_ROT: return updateGizmoRot(camera, position);
        case GIZMO_ACTIVE_AXIS: return updateGizmoAxis(camera, position);
        case GIZMO_ACTIVE_PLANE: return updateGizmoPlane(camera, position);
        default: return MatrixIdentity();
    }
}

void loadGizmo() {
    if (isGizmoLoaded) return;

    shaderRotHandleColor = LoadShaderFromMemory(
        SHADER_COLOR_VERT, shaderRotHandleColor_FRAG
    );
    ShaderRotHandleCameraPosLoc = GetShaderLocation(
        shaderRotHandleColor, "cameraPosition"
    );
    ShaderRotHandleGizmoPosLoc = GetShaderLocation(
        shaderRotHandleColor, "gizmoPosition"
    );

    maskFramebuffer = rlLoadFramebuffer(
        MASK_FRAMEBUFFER_WIDTH, MASK_FRAMEBUFFER_HEIGHT
    );
    if (!maskFramebuffer) {
        TraceLog(LOG_ERROR, "Failed to create gizmo's mask framebuffer");
        exit(1);
    }
    rlEnableFramebuffer(maskFramebuffer);

    maskTexture = rlLoadTexture(
        NULL,
        MASK_FRAMEBUFFER_WIDTH,
        MASK_FRAMEBUFFER_HEIGHT,
        RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
        1
    );
    rlActiveDrawBuffers(1);
    rlFramebufferAttach(
        maskFramebuffer,
        maskTexture,
        RL_ATTACHMENT_COLOR_CHANNEL0,
        RL_ATTACHMENT_TEXTURE2D,
        0
    );
    if (!rlFramebufferComplete(maskFramebuffer)) {
        TraceLog(LOG_ERROR, "Gizmo's framebuffer is not complete");
        exit(1);
    }

    isGizmoLoaded = true;
}

void unloadGizmo() {
    if (!isGizmoLoaded) return;

    UnloadShader(shaderRotHandleColor);
    rlUnloadFramebuffer(maskFramebuffer);
    rlUnloadTexture(maskTexture);
    isGizmoLoaded = false;
}

Matrix updateAndDrawGizmo(Camera3D camera, Vector3 position) {
    if (!isGizmoLoaded) {
        TraceLog(LOG_ERROR, "Gizmo must be loaded before the update");
        exit(1);
    }

    unsigned char maskVal = getGizmoMaskPixel(camera, position);
    Matrix transform = updateGizmo(camera, position, maskVal);

    // -------------------------------------------------------------------
    // Draw gizmo
    HandleColors rotHandleColors = getHandleColors(GIZMO_HOT_ROT);
    HandleColors axisHandleColors = getHandleColors(GIZMO_HOT_AXIS);
    HandleColors planeHandleColors = getHandleColors(GIZMO_HOT_PLANE);

    drawGizmo(
        camera,
        position,
        rotHandleColors.x,
        rotHandleColors.y,
        rotHandleColors.z,
        axisHandleColors.x,
        axisHandleColors.y,
        axisHandleColors.z,
        planeHandleColors.x,
        planeHandleColors.y,
        planeHandleColors.z
    );

    return transform;
}

#endif

