
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glad.h"      // Extensions loading library
#include <GLFW/glfw3.h>

#include "OculusSDK/LibOVR/Include/OVR_CAPI_GL.h"    // Oculus SDK for OpenGL

#define FAIL(X) printf(X);

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Matrix {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Matrix;

// RiftManagerApp class
ovrSession session;
ovrHmdDesc hmdDesc;
ovrGraphicsLuid luid;

// RiftApp class
GLuint fbo = 0;
GLuint depthBuffer = 0;
ovrTextureSwapChain eyeTexture;

GLuint mirrorFbo = 0;
ovrMirrorTexture mirrorTexture;
ovrEyeRenderDesc eyeRenderDescs[2];
Matrix eyeProjections[2];

ovrLayerEyeFov eyeLayer;
ovrViewScaleDesc viewScaleDesc;

Vector2 renderTargetSize;
Vector2 mirrorSize;

// GlfwApp class
GLFWwindow *window = NULL;
unsigned int frame = 0;

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

// Execute our example class
int main() 
{
    if (!OVR_SUCCESS(ovr_Initialize(NULL))) FAIL("Failed to initialize the Oculus SDK\n");
    
    //result = ExampleApp().run();      // class ExampleApp : public RiftApp : public GlfwApp, public RiftManagerApp
    
    if (!OVR_SUCCESS(ovr_Create(&session, &luid))) FAIL("Unable to create HMD session\n");
    hmdDesc = ovr_GetHmdDesc(session);
    
    // RiftApp() constructor
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    memset(&eyeLayer, 0, sizeof(ovrLayerEyeFov));
    eyeLayer.Header.Type = ovrLayerType_EyeFov;
    eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    //ovr::for_each_eye([&](ovrEyeType eye) 
    for (int eye = 0; eye < 2; eye++)
    {
        eyeRenderDescs[eye] = ovr_GetRenderDesc(session, eye, hmdDesc.DefaultEyeFov[eye]);
        ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(eyeRenderDescs[eye].Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
        //eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
        viewScaleDesc.HmdToEyeOffset[eye] = eyeRenderDescs[eye].HmdToEyeOffset;

        eyeLayer.Fov[eye] = eyeRenderDescs[eye].Fov;
        ovrSizei eyeSize = ovr_GetFovTextureSize(session, eye, eyeLayer.Fov[eye], 1.0f);
        eyeLayer.Viewport[eye].Size = eyeSize;
        eyeLayer.Viewport[eye].Pos.x = renderTargetSize.x;
        eyeLayer.Viewport[eye].Pos.y = 0;

        renderTargetSize.y = renderTargetSize.y; // std::max(renderTargetSize.y, (uint32_t)eyeSize.h);
        renderTargetSize.x += eyeSize.w;
    }
    
    // Make the on screen window 1/4 the resolution of the render target
    mirrorSize = renderTargetSize;
    mirrorSize.x /= 2;
    mirrorSize.y /= 2;

    // GLFWApp() constructor
    if (!glfwInit()) FAIL("Failed to initialize GLFW\n"); // Initialize the GLFW system for creating and positioning windows
    glfwSetErrorCallback(ErrorCallback);
    
    ////preCreate();
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //***************window = createRenderingTarget(windowSize, windowPosition);    //GLFWwindow *createRenderingTarget(uvec2 & size, ivec2 & pos) = 0; //glfw::createWindow(_mirrorSize);
    /*
    GLFWwindow *createWindow(const uvec2 &size, const ivec2 &position = ivec2(INT_MIN))
    {
        GLFWwindow *window = glfwCreateWindow(size.x, size.y, "glfw", NULL, NULL);  // size = mirrorSize
        
        if (!window) FAIL("Unable to create rendering window\n");

        if ((position.x > INT_MIN) && (position.y > INT_MIN)) // INT_MIN = -32767   // #define INT_MIN (-2147483647 - 1)
        {
            glfwSetWindowPos(window, position.x, position.y);
        }
        
        return window;
    }
    */
    
    window = glfwCreateWindow(mirrorSize.x, mirrorSize.y, "glfw", NULL, NULL);

    if (!window) FAIL("Unable to create OpenGL window\n");

    ////postCreate();
    //glfwSetWindowUserPointer(window, this);  //// Useful to hack input callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);

    // Initialize the OpenGL extensions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) FAIL("GLAD failed\n");
    /*
    glewExperimental = GL_TRUE;
    if (0 != glewInit()) FAIL("Failed to initialize GLEW\n");
    glGetError();

    if (GLEW_KHR_debug) 
    {
        GLint v;
        glGetIntegerv(GL_CONTEXT_FLAGS, &v);
        if (v & GL_CONTEXT_FLAG_DEBUG_BIT) glDebugMessageCallback(glDebugCallbackHandler, this);
    }
    */

    ////initGl();
    {
        // RiftApp::InitGL() ----->
            //GlfwApp::initGl();    // virtual

        // Disable the v-sync for buffer swap
        glfwSwapInterval(0);

        ovrTextureSwapChainDesc desc = {};
        desc.Type = ovrTexture_2D;
        desc.ArraySize = 1;
        desc.Width = renderTargetSize.x;
        desc.Height = renderTargetSize.y;
        desc.MipLevels = 1;
        desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc.SampleCount = 1;
        desc.StaticImage = ovrFalse;
        
        ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &eyeTexture);
        eyeLayer.ColorTexture[0] = eyeTexture;
        
        if (!OVR_SUCCESS(result)) FAIL("Failed to create swap textures");
        
        int length = 0;
        result = ovr_GetTextureSwapChainLength(session, eyeTexture, &length);
        
        if (!OVR_SUCCESS(result) || !length) FAIL("Unable to count swap chain textures");

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

        // Set up the framebuffer object
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &depthBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, renderTargetSize.x, renderTargetSize.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        ovrMirrorTextureDesc mirrorDesc;
        memset(&mirrorDesc, 0, sizeof(mirrorDesc));
        mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        mirrorDesc.Width = mirrorSize.x;
        mirrorDesc.Height = mirrorSize.y;
        
        if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(session, &mirrorDesc, &mirrorTexture))) FAIL("Could not create mirror texture");

        glGenFramebuffers(1, &mirrorFbo);

        // RiftApp::InitGL() <------
        
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        ovr_RecenterTrackingOrigin(session);
        
        // TODO: Init cube scene --> cubeScene = std::shared_ptr<ColorCubeScene>(new ColorCubeScene());
    }

    while (!glfwWindowShouldClose(window)) 
    {
        frame++;
        glfwPollEvents();
        
        //update();
        
        //draw(); ------>
        ovrPosef eyePoses[2];
        ovr_GetEyePoses(session, frame, ovrTrue, viewScaleDesc.HmdToEyeOffset, eyePoses, &eyeLayer.SensorSampleTime);

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
            
            //renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye])); -->  cubeScene->render(projection, glm::inverse(headPose));
        }
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ovr_CommitTextureSwapChain(session, eyeTexture);
        ovrLayerHeader *headerList = &eyeLayer.Header;
        
        ovr_SubmitFrame(session, frame, &viewScaleDesc, &headerList, 1);

        GLuint mirrorTextureId;
        ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &mirrorTextureId);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
        glBlitFramebuffer(0, 0, mirrorSize.x, mirrorSize.y, 0, mirrorSize.y, mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        //draw() <-------------
        
        glfwSwapBuffers(window); //finishFrame();
    }

    //shutdownGl();        // Delete scene:  cubeScene.reset();
    
    glfwDestroyWindow(window);
    glfwTerminate();

    ovr_Destroy(session);
    ovr_Shutdown();
    
    return 0;
}
