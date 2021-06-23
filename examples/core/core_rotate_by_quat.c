#include "raylib.h"
#include "rlgl.h" // just to change line width
#include "raymath.h"

#define windowWidth 1280
#define windowHeight 720

Vector3 Vector3MultiplyQuaternion(Vector3 v, Quaternion q)
{
    Vector3 r, qv, uv, uuv;
    
    qv.x = q.x;
    qv.y = q.y;
    qv.z = q.z;
    
    uv = Vector3CrossProduct(qv, v);
    uuv = Vector3CrossProduct(qv, uv);
    
    uv = Vector3Scale(uv, 2.f*q.w);
    uuv = Vector3Scale(uuv, 2.0f);
    
    r = Vector3Add(v, uv);
    r = Vector3Add(r, uuv);
    
    return r;
}

int main(void)
{
    SetTraceLogLevel( LOG_ALL );
    SetConfigFlags( FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT );
    InitWindow( windowWidth, windowHeight, "Raylib - template" );
    
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.0f, 4.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    Vector3 ang = { 0 };
    Vector3 qv = { 0 };
    Vector3 mv = { 0 };
    Vector3 bv = { 0 };
    Quaternion q;
    
    float dT = 0;
    
    rlSetLineWidth(4);
    while ( !WindowShouldClose() )
    {
        dT = GetFrameTime();
        
        ang.x += .7f * dT;
        ang.y += .55 * dT;
        ang.z -= 2.75 * dT;
        
        q = QuaternionFromEuler(ang.x, ang.y, ang.z);
        qv = Vector3MultiplyQuaternion((Vector3){0,0,1}, q);
        
        Matrix m = QuaternionToMatrix(q);
        mv = Vector3Transform((Vector3){0,0,1},m);
        
        bv = Vector3RotateByQuaternion((Vector3){0,0,1}, q);
        
        BeginDrawing();
        ClearBackground( (Color){64,128,255,255} );
        
        BeginMode3D( camera );
        DrawLine3D((Vector3){0,0,0}, qv, GREEN);
        DrawLine3D((Vector3){-.1,0,0}, mv, YELLOW);
        DrawLine3D((Vector3){.1,0,0}, bv, RED);
        
        DrawGrid( 10, 1.0f );
        
        EndMode3D();
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}