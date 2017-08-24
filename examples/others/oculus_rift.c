/*******************************************************************************************
*
*   raylib [core] example - Oculus Rift CV1
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) -I..\src\external -I..\src\external\OculusSDK\LibOVR\Include /
*       -L. -L..\src\external\OculusSDK\LibOVR -lLibOVRRT32_1 -lraylib -lglfw3 -lopengl32 -lgdi32 -std=c99 /
*       -Wl,-allow-multiple-definition
*
*   #define SUPPORT_OCULUS_RIFT_CV1 / RLGL_OCULUS_SUPPORT
*       Enable Oculus Rift CV1 functionality
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "glad.h"                   // Required for: OpenGL types and functions declarations
#include "raymath.h"                // Required for: Vector3, Quaternion and Matrix functionality

#include <string.h>                 // Required for: memset()
#include <stdlib.h>                 // Required for: exit()
#include <stdio.h>                  // required for: vfprintf()
#include <stdarg.h>                 // Required for: va_list, va_start(), vfprintf(), va_end()

#define RLGL_OCULUS_SUPPORT         // Enable Oculus Rift code
#if defined(RLGL_OCULUS_SUPPORT)
    #include "OVR_CAPI_GL.h"        // Oculus SDK for OpenGL
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if defined(RLGL_OCULUS_SUPPORT)
// Oculus buffer type
typedef struct OculusBuffer {
    ovrTextureSwapChain textureChain;
    GLuint depthId;
    GLuint fboId;
    int width;
    int height;
} OculusBuffer;

// Oculus mirror texture type
typedef struct OculusMirror {
    ovrMirrorTexture texture;
    GLuint fboId;
    int width;
    int height;
} OculusMirror;

// Oculus layer type
typedef struct OculusLayer {
    ovrViewScaleDesc viewScaleDesc;
    ovrLayerEyeFov eyeLayer;      // layer 0
    //ovrLayerQuad quadLayer;     // TODO: layer 1: '2D' quad for GUI
    Matrix eyeProjections[2];
    int width;
    int height;
} OculusLayer;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(RLGL_OCULUS_SUPPORT)
// OVR device variables
static ovrSession session;              // Oculus session (pointer to ovrHmdStruct)
static ovrHmdDesc hmdDesc;              // Oculus device descriptor parameters
static ovrGraphicsLuid luid;            // Oculus locally unique identifier for the program (64 bit)
static OculusLayer layer;               // Oculus drawing layer (similar to photoshop)
static OculusBuffer buffer;             // Oculus internal buffers (texture chain and fbo)
static OculusMirror mirror;             // Oculus mirror texture and fbo
static unsigned int frameIndex = 0;     // Oculus frames counter, used to discard frames from chain
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(RLGL_OCULUS_SUPPORT)
static bool InitOculusDevice(void);                 // Initialize Oculus device (returns true if success)
static void CloseOculusDevice(void);                // Close Oculus device
static void UpdateOculusTracking(Camera *camera);   // Update Oculus head position-orientation tracking
static void BeginOculusDrawing(void);               // Setup Oculus buffers for drawing
static void EndOculusDrawing(void);                 // Finish Oculus drawing and blit framebuffer to mirror

static OculusBuffer LoadOculusBuffer(ovrSession session, int width, int height);    // Load Oculus required buffers
static void UnloadOculusBuffer(ovrSession session, OculusBuffer buffer);            // Unload texture required buffers
static OculusMirror LoadOculusMirror(ovrSession session, int width, int height);    // Load Oculus mirror buffers
static void UnloadOculusMirror(ovrSession session, OculusMirror mirror);            // Unload Oculus mirror buffers
static void BlitOculusMirror(ovrSession session, OculusMirror mirror);              // Copy Oculus screen buffer to mirror texture
static OculusLayer InitOculusLayer(ovrSession session);                             // Init Oculus layer (similar to photoshop)
static Matrix FromOvrMatrix(ovrMatrix4f ovrM);  // Convert from Oculus ovrMatrix4f struct to raymath Matrix struct
#endif

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1080;
    int screenHeight = 600;
    
    // NOTE: screenWidth/screenHeight should match VR device aspect ratio
    
    InitWindow(screenWidth, screenHeight, "raylib [core] example - oculus rift");
    
    bool vrDeviceReady = InitOculusDevice();            // Init VR device Oculus Rift CV1
    
    if (!vrDeviceReady) InitVrSimulator(HMD_OCULUS_RIFT_CV1); // Init VR simulator if device fails
    
    // Define the camera to look into our 3d world
    Camera camera;
    camera.position = (Vector3){ 5.0f, 2.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    SetCameraMode(camera, CAMERA_FIRST_PERSON);         // Set first person camera mode
    
    SetTargetFPS(90);                   // Set our game to run at 90 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (!vrDeviceReady) UpdateCamera(&camera);      // Update camera (simulator mode)
        else UpdateOculusTracking(&camera);             // Update camera with device tracking data
    
        if (IsKeyPressed(KEY_SPACE)) ToggleVrMode();    // Toggle VR mode
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            if (vrDeviceReady) BeginOculusDrawing();
            else BeginVrDrawing();

            Begin3dMode(camera);

                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(40, 1.0f);

            End3dMode();
            
            if (vrDeviceReady) EndOculusDrawing();
            else EndVrDrawing();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (vrDeviceReady) CloseOculusDevice();
    else CloseVrSimulator();
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(RLGL_OCULUS_SUPPORT)
// Set internal projection and modelview matrix depending on eyes tracking data
static void SetStereoView(int eye, Matrix matProjection, Matrix matModelView)
{
    Matrix eyeProjection = matProjection;
    Matrix eyeModelView = matModelView;

    glViewport(layer.eyeLayer.Viewport[eye].Pos.x, layer.eyeLayer.Viewport[eye].Pos.y,
               layer.eyeLayer.Viewport[eye].Size.w, layer.eyeLayer.Viewport[eye].Size.h);

    Quaternion eyeRenderPose = (Quaternion){ layer.eyeLayer.RenderPose[eye].Orientation.x,
                                             layer.eyeLayer.RenderPose[eye].Orientation.y,
                                             layer.eyeLayer.RenderPose[eye].Orientation.z,
                                             layer.eyeLayer.RenderPose[eye].Orientation.w };
    QuaternionInvert(&eyeRenderPose);
    Matrix eyeOrientation = QuaternionToMatrix(eyeRenderPose);
    Matrix eyeTranslation = MatrixTranslate(-layer.eyeLayer.RenderPose[eye].Position.x,
                                            -layer.eyeLayer.RenderPose[eye].Position.y,
                                            -layer.eyeLayer.RenderPose[eye].Position.z);

    Matrix eyeView = MatrixMultiply(eyeTranslation, eyeOrientation);    // Matrix containing eye-head movement
    eyeModelView = MatrixMultiply(matModelView, eyeView);               // Combine internal camera matrix (modelview) wih eye-head movement

    eyeProjection = layer.eyeProjections[eye];
}

// Initialize Oculus device (returns true if success)
static bool InitOculusDevice(void)
{
    bool oculusReady = false;

    ovrResult result = ovr_Initialize(NULL);

    if (OVR_FAILURE(result)) TraceLog(WARNING, "OVR: Could not initialize Oculus device");
    else
    {
        result = ovr_Create(&session, &luid);
        if (OVR_FAILURE(result))
        {
            TraceLog(LOG_WARNING, "OVR: Could not create Oculus session");
            ovr_Shutdown();
        }
        else
        {
            hmdDesc = ovr_GetHmdDesc(session);

            TraceLog(LOG_INFO, "OVR: Product Name: %s", hmdDesc.ProductName);
            TraceLog(LOG_INFO, "OVR: Manufacturer: %s", hmdDesc.Manufacturer);
            TraceLog(LOG_INFO, "OVR: Product ID: %i", hmdDesc.ProductId);
            TraceLog(LOG_INFO, "OVR: Product Type: %i", hmdDesc.Type);
            //TraceLog(LOG_INFO, "OVR: Serial Number: %s", hmdDesc.SerialNumber);
            TraceLog(LOG_INFO, "OVR: Resolution: %ix%i", hmdDesc.Resolution.w, hmdDesc.Resolution.h);

            // NOTE: Oculus mirror is set to defined screenWidth and screenHeight...
            // ...ideally, it should be (hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2)

            // Initialize Oculus Buffers
            layer = InitOculusLayer(session);
            buffer = LoadOculusBuffer(session, layer.width, layer.height);
            mirror = LoadOculusMirror(session, hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2);     // NOTE: hardcoded...
            layer.eyeLayer.ColorTexture[0] = buffer.textureChain;     //SetOculusLayerTexture(eyeLayer, buffer.textureChain);

            // Recenter OVR tracking origin
            ovr_RecenterTrackingOrigin(session);

            oculusReady = true;
        }
    }

    return oculusReady;
}

// Close Oculus device (and unload buffers)
static void CloseOculusDevice(void)
{
    UnloadOculusMirror(session, mirror);    // Unload Oculus mirror buffer
    UnloadOculusBuffer(session, buffer);    // Unload Oculus texture buffers

    ovr_Destroy(session);   // Free Oculus session data
    ovr_Shutdown();         // Close Oculus device connection
}

// Update Oculus head position-orientation tracking
static void UpdateOculusTracking(Camera *camera)
{
    frameIndex++;

    ovrPosef eyePoses[2];
    ovr_GetEyePoses(session, frameIndex, ovrTrue, layer.viewScaleDesc.HmdToEyeOffset, eyePoses, &layer.eyeLayer.SensorSampleTime);

    layer.eyeLayer.RenderPose[0] = eyePoses[0];
    layer.eyeLayer.RenderPose[1] = eyePoses[1];

    // TODO: Update external camera with eyePoses data (position, orientation)
    // NOTE: We can simplify to simple camera if we consider IPD and HMD device configuration again later
    // it will be useful for the user to draw, lets say, billboards oriented to camera

    // Get session status information
    ovrSessionStatus sessionStatus;
    ovr_GetSessionStatus(session, &sessionStatus);

    if (sessionStatus.ShouldQuit) TraceLog(LOG_WARNING, "OVR: Session should quit...");
    if (sessionStatus.ShouldRecenter) ovr_RecenterTrackingOrigin(session);
    //if (sessionStatus.HmdPresent)  // HMD is present.
    //if (sessionStatus.DisplayLost) // HMD was unplugged or the display driver was manually disabled or encountered a TDR.
    //if (sessionStatus.HmdMounted)  // HMD is on the user's head.
    //if (sessionStatus.IsVisible)   // the game or experience has VR focus and is visible in the HMD.
}

// Setup Oculus buffers for drawing
static void BeginOculusDrawing(void)
{
    GLuint currentTexId;
    int currentIndex;

    ovr_GetTextureSwapChainCurrentIndex(session, buffer.textureChain, &currentIndex);
    ovr_GetTextureSwapChainBufferGL(session, buffer.textureChain, currentIndex, &currentTexId);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.fboId);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTexId, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthId, 0);    // Already binded
}

// Finish Oculus drawing and blit framebuffer to mirror
static void EndOculusDrawing(void)
{
    // Unbind current framebuffer (Oculus buffer)
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    ovr_CommitTextureSwapChain(session, buffer.textureChain);

    ovrLayerHeader *layers = &layer.eyeLayer.Header;
    ovr_SubmitFrame(session, frameIndex, &layer.viewScaleDesc, &layers, 1);

    // Blit mirror texture to back buffer
    BlitOculusMirror(session, mirror);
}

// Load Oculus required buffers: texture-swap-chain, fbo, texture-depth
static OculusBuffer LoadOculusBuffer(ovrSession session, int width, int height)
{
    OculusBuffer buffer;
    buffer.width = width;
    buffer.height = height;

    // Create OVR texture chain
    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;   // Requires glEnable(GL_FRAMEBUFFER_SRGB);
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &buffer.textureChain);

    if (!OVR_SUCCESS(result)) TraceLog(LOG_WARNING, "OVR: Failed to create swap textures buffer");

    int textureCount = 0;
    ovr_GetTextureSwapChainLength(session, buffer.textureChain, &textureCount);

    if (!OVR_SUCCESS(result) || !textureCount) TraceLog(LOG_WARNING, "OVR: Unable to count swap chain textures");

    for (int i = 0; i < textureCount; ++i)
    {
        GLuint chainTexId;
        ovr_GetTextureSwapChainBufferGL(session, buffer.textureChain, i, &chainTexId);
        glBindTexture(GL_TEXTURE_2D, chainTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    /*
    // Setup framebuffer object (using depth texture)
    glGenFramebuffers(1, &buffer.fboId);
    glGenTextures(1, &buffer.depthId);
    glBindTexture(GL_TEXTURE_2D, buffer.depthId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, buffer.width, buffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    */

    // Setup framebuffer object (using depth renderbuffer)
    glGenFramebuffers(1, &buffer.fboId);
    glGenRenderbuffers(1, &buffer.depthId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.fboId);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer.depthId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, buffer.width, buffer.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.depthId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return buffer;
}

// Unload texture required buffers
static void UnloadOculusBuffer(ovrSession session, OculusBuffer buffer)
{
    if (buffer.textureChain)
    {
        ovr_DestroyTextureSwapChain(session, buffer.textureChain);
        buffer.textureChain = NULL;
    }

    if (buffer.depthId != 0) glDeleteTextures(1, &buffer.depthId);
    if (buffer.fboId != 0) glDeleteFramebuffers(1, &buffer.fboId);
}

// Load Oculus mirror buffers
static OculusMirror LoadOculusMirror(ovrSession session, int width, int height)
{
    OculusMirror mirror;
    mirror.width = width;
    mirror.height = height;

    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    mirrorDesc.Width = mirror.width;
    mirrorDesc.Height = mirror.height;

    if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirror.texture))) TraceLog(LOG_WARNING, "Could not create mirror texture");

    glGenFramebuffers(1, &mirror.fboId);

    return mirror;
}

// Unload Oculus mirror buffers
static void UnloadOculusMirror(ovrSession session, OculusMirror mirror)
{
    if (mirror.fboId != 0) glDeleteFramebuffers(1, &mirror.fboId);
    if (mirror.texture) ovr_DestroyMirrorTexture(session, mirror.texture);
}

// Copy Oculus screen buffer to mirror texture
static void BlitOculusMirror(ovrSession session, OculusMirror mirror)
{
    GLuint mirrorTextureId;

    ovr_GetMirrorTextureBufferGL(session, mirror.texture, &mirrorTextureId);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror.fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
#if defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glBlitFramebuffer() requires extension: GL_EXT_framebuffer_blit (not available in OpenGL ES 2.0)
    glBlitFramebuffer(0, 0, mirror.width, mirror.height, 0, mirror.height, mirror.width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
#endif
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// Init Oculus layer (similar to photoshop)
static OculusLayer InitOculusLayer(ovrSession session)
{
    OculusLayer layer = { 0 };

    layer.viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

    memset(&layer.eyeLayer, 0, sizeof(ovrLayerEyeFov));
    layer.eyeLayer.Header.Type = ovrLayerType_EyeFov;
    layer.eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    ovrEyeRenderDesc eyeRenderDescs[2];

    for (int eye = 0; eye < 2; eye++)
    {
        eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmdDesc.DefaultEyeFov[eye]);
        ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(eyeRenderDescs[eye].Fov, 0.01f, 10000.0f, ovrProjection_None); //ovrProjection_ClipRangeOpenGL);
        layer.eyeProjections[eye] = FromOvrMatrix(ovrPerspectiveProjection);      // NOTE: struct ovrMatrix4f { float M[4][4] } --> struct Matrix

        layer.viewScaleDesc.HmdToEyeOffset[eye] = eyeRenderDescs[eye].HmdToEyeOffset;
        layer.eyeLayer.Fov[eye] = eyeRenderDescs[eye].Fov;

        ovrSizei eyeSize = ovr_GetFovTextureSize(session, eye, layer.eyeLayer.Fov[eye], 1.0f);
        layer.eyeLayer.Viewport[eye].Size = eyeSize;
        layer.eyeLayer.Viewport[eye].Pos.x = layer.width;
        layer.eyeLayer.Viewport[eye].Pos.y = 0;

        layer.height = eyeSize.h;     //std::max(renderTargetSize.y, (uint32_t)eyeSize.h);
        layer.width += eyeSize.w;
    }

    return layer;
}

// Convert from Oculus ovrMatrix4f struct to raymath Matrix struct
static Matrix FromOvrMatrix(ovrMatrix4f ovrmat)
{
    Matrix rmat;

    rmat.m0 = ovrmat.M[0][0];
    rmat.m1 = ovrmat.M[1][0];
    rmat.m2 = ovrmat.M[2][0];
    rmat.m3 = ovrmat.M[3][0];
    rmat.m4 = ovrmat.M[0][1];
    rmat.m5 = ovrmat.M[1][1];
    rmat.m6 = ovrmat.M[2][1];
    rmat.m7 = ovrmat.M[3][1];
    rmat.m8 = ovrmat.M[0][2];
    rmat.m9 = ovrmat.M[1][2];
    rmat.m10 = ovrmat.M[2][2];
    rmat.m11 = ovrmat.M[3][2];
    rmat.m12 = ovrmat.M[0][3];
    rmat.m13 = ovrmat.M[1][3];
    rmat.m14 = ovrmat.M[2][3];
    rmat.m15 = ovrmat.M[3][3];

    MatrixTranspose(&rmat);

    return rmat;
}
#endif
