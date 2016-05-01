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

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_EXPOSE_NATIVE_WGL
    #define OVR_OS_WIN32
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #define GLFW_EXPOSE_NATIVE_NSGL
    #define OVR_OS_MAC
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
    #define OVR_OS_LINUX
#endif

#include "glad.h"      // Extensions loading library

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "OculusSDK/LibOVR/Include/OVR_CAPI_GL.h"    // Oculus SDK for OpenGL

//#include "GL/CAPI_GLE.h"        // stripped-down GLEW/GLAD library to manage extensions (really required?)
//#include "Extras/OVR_Math.h"    // math utilities C++ (really required?)

#define RLGL_STANDALONE
#include "rlgl.h"

#include <stdlib.h>
#include <stdio.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct OculusBuffer {
    ovrTextureSwapChain textureChain;
    GLuint depthId;
    GLuint fboId;
    int width;
    int height;
} OculusBuffer;

typedef enum { LOG_INFO = 0, LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_OTHER } TraceLogType;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static OculusBuffer LoadOculusBuffer(ovrSession session, int width, int height);
static void UnloadOculusBuffer(ovrSession session, OculusBuffer buffer);
static void SetOculusBuffer(ovrSession session, OculusBuffer buffer);
static void UnsetOculusBuffer(OculusBuffer buffer);

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

//----------------------------------------------------------------------------------
// Main Entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    ovrSession session;
    ovrGraphicsLuid luid;   // Useless for OpenGL since SDK 0.7
    ovrHmdDesc hmdDesc;
    
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
    
    int screenWidth = hmdDesc.Resolution.w/2 + 100;		// Added 100 pixels for testing
    int screenHeight = hmdDesc.Resolution.h/2 + 100;	// Added 100 pixels for testing

    // GLFW3 Initialization + OpenGL 3.3 Context + Extensions
    //--------------------------------------------------------
    GLFWwindow *window;
    
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit())
    {
        TraceLog(LOG_WARNING, "GLFW3: Can not initialize GLFW");
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: GLFW initialized successfully");
    
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);   // Mandatory on Oculus Rift to avoid program crash!
   
    window = glfwCreateWindow(screenWidth, screenHeight, "rlgl standalone", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    else TraceLog(LOG_INFO, "GLFW3: Window created successfully");
    
    glfwSetKeyCallback(window, KeyCallback);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        TraceLog(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
        exit(1);
    }
    else TraceLog(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");
    
    rlglInit();
    rlglInitGraphics(0, 0, screenWidth, screenHeight);
    rlClearColor(245, 245, 245, 255); // Define clear color
    
    Vector2 position = { screenWidth/2 - 100, screenHeight/2 - 100 };
    Vector2 size = { 200, 200 };
    Color color = { 180, 20, 20, 255 };
    //---------------------------------------------------------------------------
    
    OculusBuffer eyeRenderBuffer[2];

    GLuint mirrorFBO = 0;
    ovrMirrorTexture mirrorTexture = NULL;

    bool isVisible = true;
    long long frameIndex = 0;

    // Make eyes render buffers
    ovrSizei recommendedTexSizeLeft = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0], 1.0f);
    eyeRenderBuffer[0] = LoadOculusBuffer(session, recommendedTexSizeLeft.w, recommendedTexSizeLeft.h);
    ovrSizei recommendedTexSizeRight = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1], 1.0f);
    eyeRenderBuffer[1] = LoadOculusBuffer(session, recommendedTexSizeRight.w, recommendedTexSizeRight.h);
    
    // Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
    ovrSizei windowSize = { hmdDesc.Resolution.w/2, hmdDesc.Resolution.h/2 };

    // Define mirror texture descriptor
    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Width = windowSize.w;
    mirrorDesc.Height = windowSize.h;
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

    // Create mirror texture and an FBO used to copy mirror texture to back buffer
    result = ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirrorTexture);
    if (!OVR_SUCCESS(result)) TraceLog(LOG_WARNING, "OVR: Failed to create mirror texture");

    // Configure the mirror read buffer
    GLuint texId;
    ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);

    glGenFramebuffers(1, &mirrorFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &mirrorFBO);
        TraceLog(LOG_WARNING, "OVR: Could not initialize mirror framebuffers");
    }
    
    glClearColor(1.0f, 0.1f, 0.1f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    ovr_RecenterTrackingOrigin(session);
    
    // FloorLevel will give tracking poses where the floor height is 0
    ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Update
        //----------------------------------------------------------------------------------
        frameIndex++;
        
        // TODO: Update game here!
        
	    // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyeOffset) may change at runtime.
	    ovrEyeRenderDesc eyeRenderDesc[2];
	    eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	    eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

        // Get eye poses, feeding in correct IPD offset
        ovrPosef eyeRenderPose[2];
        ovrVector3f hmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset, eyeRenderDesc[1].HmdToEyeOffset };

        double sensorSampleTime;    // sensorSampleTime is fed into the layer later
        ovr_GetEyePoses(session, frameIndex, ovrTrue, hmdToEyeOffset, eyeRenderPose, &sensorSampleTime);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        
        // Clear screen to red color
        //glClearColor(1.0f, 0.1f, 0.1f, 0.0f);   
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (isVisible)
        {
            for (int eye = 0; eye < 2; ++eye)
            {
                SetOculusBuffer(session, eyeRenderBuffer[eye]);

                // TODO: Get view and projection matrices for the eye
                // Sample using Oculus OVR_Math.h (C++)
                /*
                Matrix4f projection[eye] = Matrix4f(ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, ovrProjection_None));
                Matrix4f eyeOrientation[eye] = Matrix4f(Quatf(eyeRenderPose[eye].Orientation).Inverted());
                Matrix4f eyePose[eye] = Matrix4f::Translation(-Vector3f(eyeRenderPose[eye].Position));
                Matrix4f mvp = projection[eye]*eyeOrientation[eye]*eyePose[eye];
				*/

                // Sample using custom raymath.h (C) -INCOMPLETE-
                /*
                Matrix projection = MatrixPerspective(eyeRenderDesc[eye].Fov, ((double)screenWidth/(double)screenHeight), 0.01, 1000.0);
                Matrix eyeOrientation = QuaternionToMatrix((Quaternion){ -eyeRenderPose[eye].Orientation.x, -eyeRenderPose[eye].Orientation.y, 
                                                                         -eyeRenderPose[eye].Orientation.z, -eyeRenderPose[eye].Orientation.w });
                Matrix eyePose = MatrixTranslate(-eyeRenderPose[eye].Position.x, -eyeRenderPose[eye].Position.y, -eyeRenderPose[eye].Position.z);
                Matrix mvp = MatrixMultiply(projection, MatrixMultiply(eyeOrientation, eyePose));
                */
                
                // Render everything
                // TODO: Pass calculated mvp matrix to default shader to consider projection and orientation! 
                //DrawRectangleV(position, size, color);
                //rlglDraw();

                UnsetOculusBuffer(eyeRenderBuffer[eye]);
                
                // Commit changes to the textures so they get picked up frame
                ovr_CommitTextureSwapChain(session, eyeRenderBuffer[eye].textureChain);
            }
        }
        
        // Set up positional data
        ovrViewScaleDesc viewScaleDesc;
        viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
        viewScaleDesc.HmdToEyeOffset[0] = hmdToEyeOffset[0];
        viewScaleDesc.HmdToEyeOffset[1] = hmdToEyeOffset[1];

        // Create the main eye layer
        ovrLayerEyeFov eyeLayer;
        eyeLayer.Header.Type  = ovrLayerType_EyeFov;
        eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL

        for (int eye = 0; eye < 2; eye++)
        {
            eyeLayer.ColorTexture[eye] = eyeRenderBuffer[eye].textureChain;
            eyeLayer.Viewport[eye] = (ovrRecti){ eyeRenderBuffer[eye].width, eyeRenderBuffer[eye].height };
            eyeLayer.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
            eyeLayer.RenderPose[eye] = eyeRenderPose[eye];
            eyeLayer.SensorSampleTime = sensorSampleTime;
        }

        // Append all the layers to global list
        ovrLayerHeader *layerList = &eyeLayer.Header;
        ovrResult result = ovr_SubmitFrame(session, frameIndex, NULL, &layerList, 1);
        
        // exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
        if (!OVR_SUCCESS(result)) return 1;

        isVisible = (result == ovrSuccess);

        // Get session status information
        ovrSessionStatus sessionStatus;
        ovr_GetSessionStatus(session, &sessionStatus);
        if (sessionStatus.ShouldQuit) TraceLog(LOG_WARNING, "OVR: Session should quit.");
        if (sessionStatus.ShouldRecenter) ovr_RecenterTrackingOrigin(session);
        
        // Blit mirror texture to back buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        GLint w = mirrorDesc.Width;
        GLint h = mirrorDesc.Height;
        glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
    if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
    for (int eye = 0; eye < 2; eye++) UnloadOculusBuffer(session, eyeRenderBuffer[eye]);
    
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
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &buffer.textureChain);

    int textureCount = 0;
    ovr_GetTextureSwapChainLength(session, buffer.textureChain, &textureCount);

    if (OVR_SUCCESS(result))
    {
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
    }
    
    // Generate framebuffer
    glGenFramebuffers(1, &buffer.fboId);

    // Create Depth texture
    glGenTextures(1, &buffer.depthId);
    glBindTexture(GL_TEXTURE_2D, buffer.depthId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, buffer.width, buffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

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

    if (buffer.depthId)
    {
        glDeleteTextures(1, &buffer.depthId);
        buffer.depthId = 0;
    }

    if (buffer.fboId)
    {
        glDeleteFramebuffers(1, &buffer.fboId);
        buffer.fboId = 0;
    }
}

// Set current Oculus buffer
static void SetOculusBuffer(ovrSession session, OculusBuffer buffer)
{
    GLuint currentTexId;
    int currentIndex;
    
    ovr_GetTextureSwapChainCurrentIndex(session, buffer.textureChain, &currentIndex);
    ovr_GetTextureSwapChainBufferGL(session, buffer.textureChain, currentIndex, &currentTexId);

    glBindFramebuffer(GL_FRAMEBUFFER, buffer.fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthId, 0);

    glViewport(0, 0, buffer.width, buffer.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

// Unset Oculus buffer
static void UnsetOculusBuffer(OculusBuffer buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
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