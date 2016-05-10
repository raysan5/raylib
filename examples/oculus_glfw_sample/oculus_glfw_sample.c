/*******************************************************************************************
*
*   raylib Oculus minimum sample (OpenGL 3.3 Core)
*
*   NOTE: This example requires raylib module [rlgl]
*
*   Compile rlgl using:
*   gcc -c rlgl.c -Wall -std=c99 -DRLGL_STANDALONE -DRAYMATH_IMPLEMENTATION -DGRAPHICS_API_OPENGL_33
*
*   Compile example using:
*   gcc -o oculus_glfw_sample.exe oculus_glfw_sample.c rlgl.o glad.o -L. -lLibOVRRT32_1 -lglfw3 -lopengl32 -lgdi32 -std=c99
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

#include "glad.h"               // Extensions loading library
#include <GLFW/glfw3.h>         // Windows/Context and inputs management

#include "OculusSDK/LibOVR/Include/OVR_CAPI_GL.h"    // Oculus SDK for OpenGL

#define RLGL_STANDALONE
#include "rlgl.h"

// OVR device variables
ovrSession session;
ovrHmdDesc hmdDesc;
ovrGraphicsLuid luid;

// OVR OpenGL required variables
GLuint fbo = 0;
GLuint depthBuffer = 0;
ovrTextureSwapChain eyeTexture;

GLuint mirrorFbo = 0;
ovrMirrorTexture mirrorTexture;
ovrEyeRenderDesc eyeRenderDescs[2];
Matrix eyeProjections[2];

ovrLayerEyeFov eyeLayer;
ovrViewScaleDesc viewScaleDesc;

Vector2 renderTargetSize = { 0, 0 };
Vector2 mirrorSize;
unsigned int frame = 0;

// GLFW variables
GLFWwindow *window = NULL;

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { LOG_INFO = 0, LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_OTHER } TraceLogType;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void ErrorCallback(int error, const char* description)
{
    fputs(description, stderr);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void DrawRectangleV(Vector2 position, Vector2 size, Color color);
static void TraceLog(int msgType, const char *text, ...);
static Matrix FromOvrMatrix(ovrMatrix4f ovrM);
void DrawGrid(int slices, float spacing);
void DrawCube(Vector3 position, float width, float height, float length, Color color);

//----------------------------------------------------------------------------------
// Main Entry point
//----------------------------------------------------------------------------------
int main() 
{
    ovrResult result = ovr_Initialize(NULL);
    if (OVR_FAILURE(result)) TraceLog(LOG_ERROR, "OVR: Could not initialize Oculus device");

    result = ovr_Create(&session, &luid);
    if (OVR_FAILURE(result))
    {
        TraceLog(LOG_WARNING, "OVR: Could not create Oculus session");
        ovr_Shutdown();
    }

    hmdDesc = ovr_GetHmdDesc(session);
    
    TraceLog(LOG_INFO, "OVR: Product Name: %s", hmdDesc.ProductName);
    TraceLog(LOG_INFO, "OVR: Manufacturer: %s", hmdDesc.Manufacturer);
    TraceLog(LOG_INFO, "OVR: Product ID: %i", hmdDesc.ProductId);
    TraceLog(LOG_INFO, "OVR: Product Type: %i", hmdDesc.Type);
    TraceLog(LOG_INFO, "OVR: Serian Number: %s", hmdDesc.SerialNumber);
    TraceLog(LOG_INFO, "OVR: Resolution: %ix%i", hmdDesc.Resolution.w, hmdDesc.Resolution.h);
    
    
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    memset(&eyeLayer, 0, sizeof(ovrLayerEyeFov));
    eyeLayer.Header.Type = ovrLayerType_EyeFov;
    eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
    
    for (int eye = 0; eye < 2; eye++)
    {
        eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmdDesc.DefaultEyeFov[eye]);
        ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(eyeRenderDescs[eye].Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
        // NOTE struct ovrMatrix4f { float M[4][4] }
        eyeProjections[eye] = FromOvrMatrix(ovrPerspectiveProjection);
        viewScaleDesc.HmdToEyeOffset[eye] = eyeRenderDescs[eye].HmdToEyeOffset;

        eyeLayer.Fov[eye] = eyeRenderDescs[eye].Fov;
        ovrSizei eyeSize = ovr_GetFovTextureSize(session, eye, eyeLayer.Fov[eye], 1.0f);
        eyeLayer.Viewport[eye].Size = eyeSize;
        eyeLayer.Viewport[eye].Pos.x = renderTargetSize.x;
        eyeLayer.Viewport[eye].Pos.y = 0;

        renderTargetSize.y = eyeSize.h;     //std::max(renderTargetSize.y, (uint32_t)eyeSize.h);
        renderTargetSize.x += eyeSize.w;
    }
    
    // Make the on screen window 1/2 the resolution of the device
    mirrorSize.x = hmdDesc.Resolution.w/2;
    mirrorSize.y = hmdDesc.Resolution.h/2;

    
    // GLFW3 Initialization + OpenGL 3.3 Context + Extensions
    //--------------------------------------------------------
    if (!glfwInit())
    {
        TraceLog(LOG_WARNING, "GLFW3: Can not initialize GLFW");
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: GLFW initialized successfully");
    
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    //glfwWindowHint(GLFW_DECORATED, GL_FALSE);   // Mandatory on Oculus Rift to avoid program crash? --> NO
   
    window = glfwCreateWindow(mirrorSize.x, mirrorSize.y, "raylib oculus sample", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: Window created successfully");
    
    glfwSetErrorCallback(ErrorCallback);
    glfwSetKeyCallback(window, KeyCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        TraceLog(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
        exit(1);
    }
    else TraceLog(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");
    
    // Initialize OVR OpenGL swap chain textures
    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = renderTargetSize.x;
    desc.Height = renderTargetSize.y;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;
    
    result = ovr_CreateTextureSwapChainGL(session, &desc, &eyeTexture);
    eyeLayer.ColorTexture[0] = eyeTexture;
    
    if (!OVR_SUCCESS(result)) TraceLog(LOG_WARNING, "Failed to create swap textures");
    
    int length = 0;
    result = ovr_GetTextureSwapChainLength(session, eyeTexture, &length);
    
    if (!OVR_SUCCESS(result) || !length) TraceLog(LOG_WARNING, "Unable to count swap chain textures");

    for (int i = 0; i < length; ++i) 
    {
        GLuint chainTexId;
        ovr_GetTextureSwapChainBufferGL(session, eyeTexture, i, &chainTexId);
        glBindTexture(GL_TEXTURE_2D, chainTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Setup framebuffer object
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, renderTargetSize.x, renderTargetSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Setup mirror texture
    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    mirrorDesc.Width = mirrorSize.x;
    mirrorDesc.Height = mirrorSize.y;
    
    if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirrorTexture))) TraceLog(LOG_WARNING, "Could not create mirror texture");

    glGenFramebuffers(1, &mirrorFbo);

    // Recenter OVR tracking origin
    ovr_RecenterTrackingOrigin(session);
    
    // Initialize rlgl internal buffers and OpenGL state
    rlglInit();
    rlglInitGraphics(0, 0, mirrorSize.x, mirrorSize.y);
    rlClearColor(245, 245, 245, 255); // Define clear color
    glEnable(GL_DEPTH_TEST);
    
    Vector2 position = { mirrorSize.x/2 - 100, mirrorSize.y/2 - 100 };
    Vector2 size = { 200, 200 };
    Color color = { 180, 20, 20, 255 };
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    while (!glfwWindowShouldClose(window)) 
    {
        // Update
        //----------------------------------------------------------------------------------
        frame++;
        
        ovrPosef eyePoses[2];
        ovr_GetEyePoses(session, frame, ovrTrue, viewScaleDesc.HmdToEyeOffset, eyePoses, &eyeLayer.SensorSampleTime);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        int curIndex;
        ovr_GetTextureSwapChainCurrentIndex(session, eyeTexture, &curIndex);
        GLuint curTexId;
        ovr_GetTextureSwapChainBufferGL(session, eyeTexture, curIndex, &curTexId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        for (int eye = 0; eye < 2; eye++)
        {
            glViewport(eyeLayer.Viewport[eye].Pos.x, eyeLayer.Viewport[eye].Pos.y, 
                       eyeLayer.Viewport[eye].Size.w, eyeLayer.Viewport[eye].Size.h);
            eyeLayer.RenderPose[eye] = eyePoses[eye];
            
            // Convert struct ovrPosef { ovrQuatf Orientation; ovrVector3f Position; } to Matrix 
            // TODO: Review maths!
            Matrix eyeOrientation = QuaternionToMatrix((Quaternion){ -eyePoses[eye].Orientation.x, -eyePoses[eye].Orientation.y, -eyePoses[eye].Orientation.z, -eyePoses[eye].Orientation.w });
            Matrix eyePosition = MatrixTranslate(-eyePoses[eye].Position.x, -eyePoses[eye].Position.y, -eyePoses[eye].Position.z);
            Matrix mvp = MatrixMultiply(eyeProjections[eye], MatrixMultiply(eyeOrientation, eyePosition));

            // NOTE: Nothing is drawn until rlglDraw()
            DrawRectangleV(position, size, color);
            //DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, color);
            //DrawGrid(10, 1.0f);
            
            // NOTE: rlglDraw() must be modified to support an external modelview-projection matrix
            // TODO: Still working on it (now uses internal mvp)
            rlglDraw(mvp);
        }
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ovr_CommitTextureSwapChain(session, eyeTexture);
        ovrLayerHeader *headerList = &eyeLayer.Header;
        ovr_SubmitFrame(session, frame, &viewScaleDesc, &headerList, 1);

        // Blit mirror texture to back buffer
        GLuint mirrorTextureId;
        ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &mirrorTextureId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
        glBlitFramebuffer(0, 0, mirrorSize.x, mirrorSize.y, 0, mirrorSize.y, mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        
        glfwSwapBuffers(window);
        glfwPollEvents();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (mirrorFbo) glDeleteFramebuffers(1, &mirrorFbo);
    if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);

    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (depthBuffer) glDeleteTextures(1, &depthBuffer);
    if (eyeTexture) ovr_DestroyTextureSwapChain(session, eyeTexture);

    rlglClose();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    ovr_Destroy(session);   // Must be called after glfwTerminate()
    ovr_Shutdown();
    //--------------------------------------------------------------------------------------
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definitions
//----------------------------------------------------------------------------------

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

// Output a trace log message
// NOTE: Expected msgType: (0)Info, (1)Error, (2)Warning
static void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    switch(msgType)
    {
        case LOG_INFO: fprintf(stdout, "INFO: "); break;
        case LOG_ERROR: fprintf(stdout, "ERROR: "); break;
        case LOG_WARNING: fprintf(stdout, "WARNING: "); break;
        case LOG_DEBUG: fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    vfprintf(stdout, text, args);
    fprintf(stdout, "\n");

    va_end(args);

    //if (msgType == LOG_ERROR) exit(1);
}

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
    
    //MatrixTranspose(&rmat);
    
    return rmat;
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

// Draw a grid centered at (0, 0, 0)
void DrawGrid(int slices, float spacing)
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
