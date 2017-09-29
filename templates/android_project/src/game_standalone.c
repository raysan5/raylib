
//#include <android/sensor.h>           // Android sensors functions (accelerometer, gyroscope, light...)
#include <errno.h>
#include <android/log.h>
#include <android/window.h>             // Defines AWINDOW_FLAG_FULLSCREEN and others
#include <android/asset_manager.h>
#include <android_native_app_glue.h>    // Defines basic app state struct and manages activity

#include <EGL/egl.h>        // Khronos EGL library - Native platform display device control functions
#include <GLES2/gl2.h>      // Khronos OpenGL ES 2.0 library

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Required for: malloc(), free(), rand(), atexit()
#include <stdint.h>         // Required for: typedef unsigned long long int uint64_t, used by hi-res timer
#include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#include <time.h>           // Required for: time() - Android/RPI hi-res timer (NOTE: Linux only!)
#include <math.h>           // Required for: tan() [Used in Begin3dMode() to set perspective]
#include <string.h>         // Required for: strlen(), strrchr(), strcmp()

//#define RLGL_STANDALONE
//#include "rlgl.h"           // rlgl library: OpenGL 1.1 immediate-mode style coding

#ifndef __cplusplus
// Boolean type
    #if !defined(_STDBOOL_H) || !defined(__STDBOOL_H)   // CLang uses second form
        typedef enum { false, true } bool;
    #endif
#endif

// Trace log type
typedef enum { 
    LOG_INFO = 0,
    LOG_WARNING, 
    LOG_ERROR, 
    LOG_DEBUG, 
    LOG_OTHER 
} LogType;

static int screenWidth;
static int screenHeight;

static struct android_app *app;                 // Android activity
static struct android_poll_source *source;      // Android events polling source
static int ident, events;                       // Android ALooper_pollAll() variables
static const char *internalDataPath;            // Android internal data path to write data (/data/data/<package>/files)

static bool windowReady = false;                // Used to detect display initialization
static bool appEnabled = true;                  // Used to detec if app is active
static bool contextRebindRequired = false;      // Used to know context rebind required

static EGLDisplay display;              // Native display device (physical screen connection)
static EGLSurface surface;              // Surface to draw on, framebuffers (connected to context)
static EGLContext context;              // Graphic context, mode in which drawing can be done
static EGLConfig config;                // Graphic config
static uint64_t baseTime;               // Base time measure for hi-res timer
static bool windowShouldClose = false;  // Flag to set window for closing

static AAssetManager *assetManager;

static void AndroidCommandCallback(struct android_app *app, int32_t cmd);                  // Process Android activity lifecycle commands
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);          // Process Android inputs

static void TraceLog(int msgType, const char *text, ...);

static int android_read(void *cookie, char *buf, int size);
static int android_write(void *cookie, const char *buf, int size);
static fpos_t android_seek(void *cookie, fpos_t offset, int whence);
static int android_close(void *cookie);

static void InitWindow(int width, int height, void *state);     // Initialize Android activity
static void InitGraphicsDevice(int width, int height);          // Initialize graphic device
static void CloseWindow(void);                                  // Close window and unload OpenGL context
static bool WindowShouldClose(void);    // Check if KEY_ESCAPE pressed or Close icon pressed

static void BeginDrawing(void);            // Setup canvas (framebuffer) to start drawing
static void EndDrawing(void);              // End canvas drawing and swap buffers (double buffering)
static void SwapBuffers(void);      // Copy back buffer to front buffers
static void PollInputEvents(void);  // Poll all input events

//----------------------------------------------------------------------------------
// Android Main entry point
//----------------------------------------------------------------------------------
void android_main(struct android_app *app) 
{
    InitWindow(1280, 720, app);
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        
            
        EndDrawing();
    }
    
    CloseWindow();
}

// Initialize Android activity
static void InitWindow(int width, int height, void *state)
{
    TraceLog(LOG_INFO, "Initializing raylib stripped");

    screenWidth = width;
    screenHeight = height;

    app = (struct android_app *)state;
    internalDataPath = app->activity->internalDataPath;

    // Set desired windows flags before initializing anything
    ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER
    //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FORCE_NOT_FULLSCREEN, AWINDOW_FLAG_FULLSCREEN);

    int orientation = AConfiguration_getOrientation(app->config);

    if (orientation == ACONFIGURATION_ORIENTATION_PORT) TraceLog(LOG_INFO, "PORTRAIT window orientation");
    else if (orientation == ACONFIGURATION_ORIENTATION_LAND) TraceLog(LOG_INFO, "LANDSCAPE window orientation");

    // TODO: Automatic orientation doesn't seem to work
    if (width <= height)
    {
        AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_PORT);
        TraceLog(LOG_WARNING, "Window set to portraid mode");
    }
    else
    {
        AConfiguration_setOrientation(app->config, ACONFIGURATION_ORIENTATION_LAND);
        TraceLog(LOG_WARNING, "Window set to landscape mode");
    }

    //AConfiguration_getDensity(app->config);
    //AConfiguration_getKeyboard(app->config);
    //AConfiguration_getScreenSize(app->config);
    //AConfiguration_getScreenLong(app->config);

    //state->userData = &engine;
    app->onAppCmd = AndroidCommandCallback;
    app->onInputEvent = AndroidInputCallback;

    assetManager = app->activity->assetManager;

    TraceLog(LOG_INFO, "Android app initialized successfully");

    // Wait for window to be initialized (display and context)
    while (!windowReady)
    {
        // Process events loop
        while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0)
        {
            // Process this event
            if (source != NULL) source->process(app, source);

            // NOTE: Never close window, native activity is controlled by the system!
            //if (app->destroyRequested != 0) windowShouldClose = true;
        }
    }
}

// Close window and unload OpenGL context
static void CloseWindow(void)
{
    //rlglClose();                // De-init rlgl
    
    // Close surface, context and display
    if (display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(display, surface);
            surface = EGL_NO_SURFACE;
        }

        if (context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(display, context);
            context = EGL_NO_CONTEXT;
        }

        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }

    TraceLog(LOG_INFO, "Window closed successfully");
}

// Check if KEY_ESCAPE pressed or Close icon pressed
static bool WindowShouldClose(void)
{
    return windowShouldClose;
}

static void InitGraphicsDevice(int width, int height)
{
    screenWidth = width;        // User desired width
    screenHeight = height;      // User desired height

    EGLint samples = 0;
    EGLint sampleBuffer = 0;

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,     // Type of context support -> Required on RPI?
        //EGL_SURFACE_TYPE, EGL_WINDOW_BIT,          // Don't use it on Android!
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        //EGL_ALPHA_SIZE, 8,        // ALPHA bit depth (required for transparent framebuffer)
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs;

    // Get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // Initialize the EGL display connection
    eglInitialize(display, NULL, NULL);

    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(display, framebufferAttribs, &config, 1, &numConfigs);

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    // Create an EGL window surface
    //---------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    EGLint displayFormat;

    int displayWidth = ANativeWindow_getWidth(app->window);
    int displayHeight = ANativeWindow_getHeight(app->window);

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables: screenWidth/screenHeight and renderWidth/renderHeight and downscaleView
    
    //SetupFramebufferSize(displayWidth, displayHeight);

    //ANativeWindow_setBuffersGeometry(app->window, renderWidth, renderHeight, displayFormat);
    ANativeWindow_setBuffersGeometry(app->window, 0, 0, displayFormat);       // Force use of native display size

    surface = eglCreateWindowSurface(display, config, app->window, NULL);
#endif  // defined(PLATFORM_ANDROID)

    //eglSwapInterval(display, 1);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        TraceLog(LOG_ERROR, "Unable to attach EGL rendering context to EGL surface");
    }
    else
    {
        // Grab the width and height of the surface
        //eglQuerySurface(display, surface, EGL_WIDTH, &renderWidth);
        //eglQuerySurface(display, surface, EGL_HEIGHT, &renderHeight);

        TraceLog(LOG_INFO, "Display device initialized successfully");
        TraceLog(LOG_INFO, "Display size: %i x %i", displayWidth, displayHeight);
    }
/*
    // Initialize OpenGL context (states and resources)
    // NOTE: screenWidth and screenHeight not used, just stored as globals
    rlglInit(screenWidth, screenHeight);

    // Setup default viewport
    rlViewport(0, 0, screenWidth, screenHeight);

    // Initialize internal projection and modelview matrices
    // NOTE: Default to orthographic projection mode with top-left corner at (0,0)
    rlMatrixMode(RL_PROJECTION);                // Switch to PROJECTION matrix
    rlLoadIdentity();                           // Reset current matrix (PROJECTION)
    rlOrtho(0, screenWidth, screenHeight, 0, 0.0f, 1.0f);
    rlMatrixMode(RL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    rlLoadIdentity();                           // Reset current matrix (MODELVIEW)

    // Clear full framebuffer (not only render area) to color
    rlClearColor(245, 245, 245, 255);
*/
    glClearColor(1, 0, 0, 1);
    
#if defined(PLATFORM_ANDROID)
    windowReady = true;             // IMPORTANT!
#endif
}

// Copy back buffer to front buffers
static void SwapBuffers(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    eglSwapBuffers(display, surface);
#endif
}

#if defined(PLATFORM_ANDROID)
// Android: Process activity lifecycle commands
static void AndroidCommandCallback(struct android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_START:
        {
            //rendering = true;
            TraceLog(LOG_INFO, "APP_CMD_START");
        } break;
        case APP_CMD_RESUME:
        {
            TraceLog(LOG_INFO, "APP_CMD_RESUME");
        } break;
        case APP_CMD_INIT_WINDOW:
        {
            TraceLog(LOG_INFO, "APP_CMD_INIT_WINDOW");

            if (app->window != NULL)
            {
                if (contextRebindRequired)
                {
                    // Reset screen scaling to full display size
                    EGLint displayFormat;
                    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &displayFormat);
                    ANativeWindow_setBuffersGeometry(app->window, screenWidth, screenHeight, displayFormat);

                    // Recreate display surface and re-attach OpenGL context
                    surface = eglCreateWindowSurface(display, config, app->window, NULL);
                    eglMakeCurrent(display, surface, surface, context);

                    contextRebindRequired = false;
                }
                else
                {
                    // Init graphics device (display device and OpenGL context)
                    InitGraphicsDevice(screenWidth, screenHeight);

                    // TODO: GPU assets reload in case of lost focus (lost context)
                    // NOTE: This problem has been solved just unbinding and rebinding context from display
                    /*
                    if (assetsReloadRequired)
                    {
                        for (int i = 0; i < assetsCount; i++)
                        {
                            // TODO: Unload old asset if required

                            // Load texture again to pointed texture
                            (*textureAsset + i) = LoadTexture(assetPath[i]);
                        }
                    }
                    */

                    // Init hi-res timer
                    //InitTimer();          // TODO.
                }
            }
        } break;
        case APP_CMD_GAINED_FOCUS:
        {
            TraceLog(LOG_INFO, "APP_CMD_GAINED_FOCUS");
            appEnabled = true;
            //ResumeMusicStream();
        } break;
        case APP_CMD_PAUSE:
        {
            TraceLog(LOG_INFO, "APP_CMD_PAUSE");
        } break;
        case APP_CMD_LOST_FOCUS:
        {
            //DrawFrame();
            TraceLog(LOG_INFO, "APP_CMD_LOST_FOCUS");
            appEnabled = false;
            //PauseMusicStream();
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            // Dettach OpenGL context and destroy display surface
            // NOTE 1: Detaching context before destroying display surface avoids losing our resources (textures, shaders, VBOs...)
            // NOTE 2: In some cases (too many context loaded), OS could unload context automatically... :(
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroySurface(display, surface);

            contextRebindRequired = true;

            TraceLog(LOG_INFO, "APP_CMD_TERM_WINDOW");
        } break;
        case APP_CMD_SAVE_STATE:
        {
            TraceLog(LOG_INFO, "APP_CMD_SAVE_STATE");
        } break;
        case APP_CMD_STOP:
        {
            TraceLog(LOG_INFO, "APP_CMD_STOP");
        } break;
        case APP_CMD_DESTROY:
        {
            // TODO: Finish activity?
            //ANativeActivity_finish(app->activity);

            TraceLog(LOG_INFO, "APP_CMD_DESTROY");
        } break;
        case APP_CMD_CONFIG_CHANGED:
        {
            //AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
            //print_cur_config(app);

            // Check screen orientation here!

            TraceLog(LOG_INFO, "APP_CMD_CONFIG_CHANGED");
        } break;
        default: break;
    }
}

// Android: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    //http://developer.android.com/ndk/reference/index.html

    int type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        /*
        // Get first touch position
        touchPosition[0].x = AMotionEvent_getX(event, 0);
        touchPosition[0].y = AMotionEvent_getY(event, 0);

        // Get second touch position
        touchPosition[1].x = AMotionEvent_getX(event, 1);
        touchPosition[1].y = AMotionEvent_getY(event, 1);
        */
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);

        // Save current button and its state
        // NOTE: Android key action is 0 for down and 1 for up
        if (AKeyEvent_getAction(event) == 0)
        {
            //currentKeyState[keycode] = 1;  // Key down
            //lastKeyPressed = keycode;
        }
        //else currentKeyState[keycode] = 0;  // Key up

        if (keycode == AKEYCODE_POWER)
        {
            // Let the OS handle input to avoid app stuck. Behaviour: CMD_PAUSE -> CMD_SAVE_STATE -> CMD_STOP -> CMD_CONFIG_CHANGED -> CMD_LOST_FOCUS
            // Resuming Behaviour: CMD_START -> CMD_RESUME -> CMD_CONFIG_CHANGED -> CMD_CONFIG_CHANGED -> CMD_GAINED_FOCUS
            // It seems like locking mobile, screen size (CMD_CONFIG_CHANGED) is affected.
            // NOTE: AndroidManifest.xml must have <activity android:configChanges="orientation|keyboardHidden|screenSize" >
            // Before that change, activity was calling CMD_TERM_WINDOW and CMD_DESTROY when locking mobile, so that was not a normal behaviour
            return 0;
        }
        else if ((keycode == AKEYCODE_BACK) || (keycode == AKEYCODE_MENU))
        {
            // Eat BACK_BUTTON and AKEYCODE_MENU, just do nothing... and don't let to be handled by OS!
            return 1;
        }
        else if ((keycode == AKEYCODE_VOLUME_UP) || (keycode == AKEYCODE_VOLUME_DOWN))
        {
            // Set default OS behaviour
            return 0;
        }
    }

    int32_t action = AMotionEvent_getAction(event);
    unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;
/*
    GestureEvent gestureEvent;

    // Register touch actions
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_MOVE;

    // Register touch points count
    gestureEvent.pointCount = AMotionEvent_getPointerCount(event);

    // Register touch points id
    gestureEvent.pointerId[0] = AMotionEvent_getPointerId(event, 0);
    gestureEvent.pointerId[1] = AMotionEvent_getPointerId(event, 1);

    // Register touch points position
    // NOTE: Only two points registered
    gestureEvent.position[0] = (Vector2){ AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0) };
    gestureEvent.position[1] = (Vector2){ AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1) };

    // Normalize gestureEvent.position[x] for screenWidth and screenHeight
    gestureEvent.position[0].x /= (float)GetScreenWidth();
    gestureEvent.position[0].y /= (float)GetScreenHeight();

    gestureEvent.position[1].x /= (float)GetScreenWidth();
    gestureEvent.position[1].y /= (float)GetScreenHeight();

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
*/
    return 0;   // return 1;
}
#endif

#if defined(PLATFORM_ANDROID)
/*
// Initialize asset manager from android app
void InitAssetManager(AAssetManager *manager)
{
    assetManager = manager;
}

// Replacement for fopen
FILE *android_fopen(const char *fileName, const char *mode)
{
    if (mode[0] == 'w') return NULL;

    AAsset *asset = AAssetManager_open(assetManager, fileName, 0);

    if (!asset) return NULL;

    return funopen(asset, android_read, android_write, android_seek, android_close);
}
*/
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
static int android_read(void *cookie, char *buf, int size)
{
    return AAsset_read((AAsset *)cookie, buf, size);
}

static int android_write(void *cookie, const char *buf, int size)
{
    TraceLog(LOG_ERROR, "Can't provide write access to the APK");

    return EACCES;
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset *)cookie, offset, whence);
}

static int android_close(void *cookie)
{
    AAsset_close((AAsset *)cookie);
    return 0;
}
#endif

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
static void TraceLog(int msgType, const char *text, ...)
{
    static char buffer[128];
    int traceDebugMsgs = 0;
    
#if defined(SUPPORT_TRACELOG_DEBUG)
    traceDebugMsgs = 1;
#endif

    switch(msgType)
    {
        case LOG_INFO: strcpy(buffer, "INFO: "); break;
        case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        default: break;
    }

    strcat(buffer, text);
    strcat(buffer, "\n");

    va_list args;
    va_start(args, text);

#if defined(PLATFORM_ANDROID)
    switch(msgType)
    {
        case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", buffer, args); break;
        case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", buffer, args); break;
        case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", buffer, args); break;
        case LOG_DEBUG: if (traceDebugMsgs) __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", buffer, args); break;
        default: break;
    }
#else
    if ((msgType != LOG_DEBUG) || ((msgType == LOG_DEBUG) && (traceDebugMsgs))) vprintf(buffer, args);
#endif

    va_end(args);

    if (msgType == LOG_ERROR) exit(1);  // If LOG_ERROR message, exit program
}

// Setup canvas (framebuffer) to start drawing
static void BeginDrawing(void)
{
/*
    currentTime = GetTime();            // Number of elapsed seconds since InitTimer() was called
    updateTime = currentTime - previousTime;
    previousTime = currentTime;
*/
    //rlClearScreenBuffers();             // Clear current framebuffers
    //rlLoadIdentity();                   // Reset current matrix (MODELVIEW)
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// End canvas drawing and swap buffers (double buffering)
static void EndDrawing(void)
{
    //rlglDraw();                     // Draw Buffers (Only OpenGL 3+ and ES2)
    
    SwapBuffers();                  // Copy back buffer to front buffer
    PollInputEvents();              // Poll user events

/*
    // Frame time control system
    currentTime = GetTime();
    drawTime = currentTime - previousTime;
    previousTime = currentTime;
    
    frameTime = updateTime + drawTime;

    // Wait for some milliseconds...
    if (frameTime < targetTime)
    {
        Wait((targetTime - frameTime)*1000.0f);

        currentTime = GetTime();
        double extraTime = currentTime - previousTime;
        previousTime = currentTime;

        frameTime += extraTime;
    }
*/
}

// Poll (store) all input events
static void PollInputEvents(void)
{
    // Reset last key pressed registered
    //lastKeyPressed = -1;
    
#if defined(PLATFORM_ANDROID)
    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    //for (int i = 0; i < 260; i++) previousKeyState[i] = currentKeyState[i];

    // Poll Events (registered events)
    // NOTE: Activity is paused if not enabled (appEnabled)
    while ((ident = ALooper_pollAll(appEnabled ? 0 : -1, NULL, &events,(void**)&source)) >= 0)
    {
        // Process this event
        if (source != NULL) source->process(app, source);

        // NOTE: Never close window, native activity is controlled by the system!
        if (app->destroyRequested != 0)
        {
            //TraceLog(LOG_INFO, "Closing Window...");
            //windowShouldClose = true;
            //ANativeActivity_finish(app->activity);
        }
    }
#endif
}


