/*******************************************************************************************
*
*   raylib Oculus minimum sample (OpenGL 3.3 Core)
*
*   NOTE: This example requires raylib module [rlgl]
*
*   Compile rlgl module using:
*   gcc -c rlgl.c -Wall -std=c99 -DRLGL_STANDALONE -DRAYMATH_IMPLEMENTATION -DGRAPHICS_API_OPENGL_33
*
*   Compile example using:
*   gcc -o oculus_glfw_sample.exe oculus_glfw_sample.c rlgl.o -L. -lLibOVRRT32_1 -lglfw3 -lopengl32 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>         // Windows/Context and inputs management

#define RLGL_STANDALONE
#include "rlgl.h"               // rlgl library: OpenGL 1.1 immediate-mode style coding

#define PLATFORM_OCULUS

#if defined(PLATFORM_OCULUS)
    #include "OculusSDK/LibOVR/Include/OVR_CAPI_GL.h"    // Oculus SDK for OpenGL
#endif

#if defined(PLATFORM_OCULUS)
// OVR device variables
ovrSession session;
ovrHmdDesc hmdDesc;
ovrGraphicsLuid luid;
#endif

unsigned int frameIndex = 0;

#define RED        (Color){ 230, 41, 55, 255 }     // Red
#define MAROON     (Color){ 190, 33, 55, 255 }     // Maroon
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_OCULUS)
typedef struct OculusBuffer {
    ovrTextureSwapChain textureChain;
    GLuint depthId;
    GLuint fboId;
    int width;
    int height;
} OculusBuffer;

typedef struct OculusMirror {
    ovrMirrorTexture texture;
    GLuint fboId;
    int width;
    int height;
} OculusMirror;

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
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char* description);
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Drawing functions (uses rlgl functionality)
static void DrawGrid(int slices, float spacing);
static void DrawCube(Vector3 position, float width, float height, float length, Color color);
static void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
static void DrawRectangleV(Vector2 position, Vector2 size, Color color);

#if defined(PLATFORM_OCULUS)
// Oculus Rift functions
static Matrix FromOvrMatrix(ovrMatrix4f ovrM);
static OculusBuffer LoadOculusBuffer(ovrSession session, int width, int height);
static void UnloadOculusBuffer(ovrSession session, OculusBuffer buffer);
static void SetOculusBuffer(ovrSession session, OculusBuffer buffer);
static void UnsetOculusBuffer(OculusBuffer buffer);
static OculusMirror LoadOculusMirror(ovrSession session, int width, int height);    // Load Oculus mirror buffers
static void UnloadOculusMirror(ovrSession session, OculusMirror mirror);            // Unload Oculus mirror buffers
static void BlitOculusMirror(ovrSession session, OculusMirror mirror);
static OculusLayer InitOculusLayer(ovrSession session);
#endif

//----------------------------------------------------------------------------------
// Main Entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1080;     // Mirror screen width (set to hmdDesc.Resolution.w/2)
    int screenHeight = 600;     // Mirror screen height (set to hmdDesc.Resolution.h/2)
    
    // NOTE: Mirror screen size can be set to any desired resolution!
    
    // GLFW3 Initialization + OpenGL 3.3 Context + Extensions
    //--------------------------------------------------------
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit())
    {
        TraceLog(WARNING, "GLFW3: Can not initialize GLFW");
        return 1;
    }
    else TraceLog(INFO, "GLFW3: GLFW initialized successfully");
    
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
   
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "raylib oculus sample", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return 2;
    }
    else TraceLog(INFO, "GLFW3: Window created successfully");
    
    glfwSetKeyCallback(window, KeyCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // Load OpenGL 3.3 extensions
    rlglLoadExtensions(glfwGetProcAddress);
    
    // Initialize rlgl internal buffers and OpenGL state
    rlglInit();
    rlglInitGraphics(0, 0, screenWidth, screenHeight);
    rlClearColor(245, 245, 245, 255);   // Define clear color
    rlEnableDepthTest();                // Enable DEPTH_TEST for 3D
    //--------------------------------------------------------
    
#if defined(PLATFORM_OCULUS)
    ovrResult result = ovr_Initialize(NULL);
    if (OVR_FAILURE(result)) TraceLog(ERROR, "OVR: Could not initialize Oculus device");

    result = ovr_Create(&session, &luid);
    if (OVR_FAILURE(result))
    {
        TraceLog(WARNING, "OVR: Could not create Oculus session");
        ovr_Shutdown();
    }

    hmdDesc = ovr_GetHmdDesc(session);
    
    TraceLog(INFO, "OVR: Product Name: %s", hmdDesc.ProductName);
    TraceLog(INFO, "OVR: Manufacturer: %s", hmdDesc.Manufacturer);
    TraceLog(INFO, "OVR: Product ID: %i", hmdDesc.ProductId);
    TraceLog(INFO, "OVR: Product Type: %i", hmdDesc.Type);
    TraceLog(INFO, "OVR: Serian Number: %s", hmdDesc.SerialNumber);
    TraceLog(INFO, "OVR: Resolution: %ix%i", hmdDesc.Resolution.w, hmdDesc.Resolution.h);
    
    //screenWidth = hmdDesc.Resolution.w/2;
    //screenHeight = hmdDesc.Resolution.h/2;

    // Initialize Oculus Buffers
    OculusLayer layer = InitOculusLayer(session);   
    OculusBuffer buffer = LoadOculusBuffer(session, layer.width, layer.height);
    OculusMirror mirror = LoadOculusMirror(session, screenWidth, screenHeight);
    layer.eyeLayer.ColorTexture[0] = buffer.textureChain;     //SetOculusLayerTexture(eyeLayer, buffer.textureChain);

    // Recenter OVR tracking origin
    ovr_RecenterTrackingOrigin(session);
#endif
    
    Camera camera;
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    //--------------------------------------------------------------------------------------    

    // Main game loop    
    while (!glfwWindowShouldClose(window)) 
    {
        // Update
        //----------------------------------------------------------------------------------
#if defined(PLATFORM_OCULUS)
        frameIndex++;
        
        ovrPosef eyePoses[2];
        ovr_GetEyePoses(session, frameIndex, ovrTrue, layer.viewScaleDesc.HmdToEyeOffset, eyePoses, &layer.eyeLayer.SensorSampleTime);
        
        layer.eyeLayer.RenderPose[0] = eyePoses[0];
        layer.eyeLayer.RenderPose[1] = eyePoses[1];
#endif
        Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
#if defined(PLATFORM_OCULUS)
        SetOculusBuffer(session, buffer);
#endif
        rlClearScreenBuffers();             // Clear current framebuffer(s)

#if defined(PLATFORM_OCULUS)
        for (int eye = 0; eye < 2; eye++)
        {
            rlViewport(layer.eyeLayer.Viewport[eye].Pos.x, layer.eyeLayer.Viewport[eye].Pos.y, layer.eyeLayer.Viewport[eye].Size.w, layer.eyeLayer.Viewport[eye].Size.h);

            Quaternion eyeRPose = (Quaternion){ layer.eyeLayer.RenderPose[eye].Orientation.x, 
                                            layer.eyeLayer.RenderPose[eye].Orientation.y, 
                                            layer.eyeLayer.RenderPose[eye].Orientation.z, 
                                            layer.eyeLayer.RenderPose[eye].Orientation.w };
            QuaternionInvert(&eyeRPose);
            Matrix eyeOrientation = QuaternionToMatrix(eyeRPose);
            Matrix eyeTranslation = MatrixTranslate(-layer.eyeLayer.RenderPose[eye].Position.x, 
                                                    -layer.eyeLayer.RenderPose[eye].Position.y, 
                                                    -layer.eyeLayer.RenderPose[eye].Position.z);
                                                
            Matrix eyeView = MatrixMultiply(eyeTranslation, eyeOrientation);
            Matrix modelview = MatrixMultiply(matView, eyeView);

			SetMatrixModelview(modelview);
			SetMatrixProjection(layer.eyeProjections[eye]);
#else
            // Calculate projection matrix (from perspective) and view matrix from camera look at
            Matrix matProj = MatrixPerspective(camera.fovy, (double)screenWidth/(double)screenHeight, 0.01, 1000.0);
            MatrixTranspose(&matProj);
			
            SetMatrixModelview(matView);    // Replace internal modelview matrix by a custom one
            SetMatrixProjection(matProj);   // Replace internal projection matrix by a custom one
#endif
            DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
            DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, RAYWHITE);
            DrawGrid(10, 1.0f);

            // NOTE: Internal buffers drawing (3D data)
            rlglDraw();
            
#if !defined(PLATFORM_OCULUS)
            // Draw '2D' elements in the scene (GUI)
            // TODO: 2D drawing on Oculus Rift: requires an ovrLayerQuad layer
            rlMatrixMode(RL_PROJECTION);                            // Enable internal projection matrix
            rlLoadIdentity();                                       // Reset internal projection matrix
            rlOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0); // Recalculate internal projection matrix
            rlMatrixMode(RL_MODELVIEW);                             // Enable internal modelview matrix
            rlLoadIdentity();                                       // Reset internal modelview matrix
            
            DrawRectangleV((Vector2){ 10.0f, 10.0f }, (Vector2){ 600.0f, 20.0f }, DARKGRAY);

            // NOTE: Internal buffers drawing (2D data)
            rlglDraw();
#endif
#if defined(PLATFORM_OCULUS)
        }
        
        UnsetOculusBuffer(buffer);
        
        ovr_CommitTextureSwapChain(session, buffer.textureChain);
        
        ovrLayerHeader *layers = &layer.eyeLayer.Header;
        ovr_SubmitFrame(session, frameIndex, &layer.viewScaleDesc, &layers, 1);

        // Blit mirror texture to back buffer
        BlitOculusMirror(session, mirror);

        // Get session status information
        ovrSessionStatus sessionStatus;
        ovr_GetSessionStatus(session, &sessionStatus);
        if (sessionStatus.ShouldQuit) TraceLog(WARNING, "OVR: Session should quit...");
        if (sessionStatus.ShouldRecenter) ovr_RecenterTrackingOrigin(session);
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
#if defined(PLATFORM_OCULUS)
    UnloadOculusMirror(session, mirror);    // Unload Oculus mirror buffer
    UnloadOculusBuffer(session, buffer);    // Unload Oculus texture buffers

    ovr_Destroy(session);   // Must be called after glfwTerminate() --> no
    ovr_Shutdown();
#endif

    rlglClose();                            // Unload rlgl internal buffers and default shader/texture
    
    glfwDestroyWindow(window);
    glfwTerminate();
    //--------------------------------------------------------------------------------------
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definitions
//----------------------------------------------------------------------------------

// GLFW3: Error callback
static void ErrorCallback(int error, const char* description)
{
    TraceLog(ERROR, description);
}

// GLFW3: Keyboard callback
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// Draw rectangle using rlgl OpenGL 1.1 style coding (translated to OpenGL 3.3 internally)
static void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2i(position.x, position.y);
        rlVertex2i(position.x, position.y + size.y);
        rlVertex2i(position.x + size.x, position.y + size.y);

        rlVertex2i(position.x, position.y);
        rlVertex2i(position.x + size.x, position.y + size.y);
        rlVertex2i(position.x + size.x, position.y);
    rlEnd();
}

// Draw a grid centered at (0, 0, 0)
static void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for(int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        // NOTE: Be careful! Function order matters (rotate -> scale -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlScalef(2.0f, 2.0f, 2.0f);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Back Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Top Face -------------------------------------------------------
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Bottom Right

            // Bottom Face ----------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left

            // Right face -----------------------------------------------------
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Left

            // Left Face ------------------------------------------------------
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Right

            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlPushMatrix();

        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}

#if defined(PLATFORM_OCULUS)
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
    
    if (!OVR_SUCCESS(result)) TraceLog(WARNING, "OVR: Failed to create swap textures buffer");

    int textureCount = 0;
    ovr_GetTextureSwapChainLength(session, buffer.textureChain, &textureCount);
    
    if (!OVR_SUCCESS(result) || !textureCount) TraceLog(WARNING, "OVR: Unable to count swap chain textures");

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

// Set current Oculus buffer
static void SetOculusBuffer(ovrSession session, OculusBuffer buffer)
{
    GLuint currentTexId;
    int currentIndex;
    
    ovr_GetTextureSwapChainCurrentIndex(session, buffer.textureChain, &currentIndex);
    ovr_GetTextureSwapChainBufferGL(session, buffer.textureChain, currentIndex, &currentTexId);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.fboId);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTexId, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthId, 0);    // Already binded

    //glViewport(0, 0, buffer.width, buffer.height);        // Useful if rendering to separate framebuffers (every eye)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Same as rlClearScreenBuffers()
    
    // NOTE: If your application is configured to treat the texture as a linear format (e.g. GL_RGBA) 
    // and performs linear-to-gamma conversion in GLSL or does not care about gamma-correction, then:
    //     - Require OculusBuffer format to be OVR_FORMAT_R8G8B8A8_UNORM_SRGB
    //     - Do NOT enable GL_FRAMEBUFFER_SRGB
    //glEnable(GL_FRAMEBUFFER_SRGB);
}

// Unset Oculus buffer
static void UnsetOculusBuffer(OculusBuffer buffer)
{
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
    
    if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirror.texture))) TraceLog(WARNING, "Could not create mirror texture");

    glGenFramebuffers(1, &mirror.fboId);

    return mirror;
}

// Unload Oculus mirror buffers
static void UnloadOculusMirror(ovrSession session, OculusMirror mirror)
{
    if (mirror.fboId != 0) glDeleteFramebuffers(1, &mirror.fboId);
    if (mirror.texture) ovr_DestroyMirrorTexture(session, mirror.texture);
}

static void BlitOculusMirror(ovrSession session, OculusMirror mirror)
{
    GLuint mirrorTextureId;
    
    ovr_GetMirrorTextureBufferGL(session, mirror.texture, &mirrorTextureId);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirror.fboId);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
    glBlitFramebuffer(0, 0, mirror.width, mirror.height, 0, mirror.height, mirror.width, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

// Requires: session, hmdDesc
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
#endif