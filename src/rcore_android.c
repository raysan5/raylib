#include <stdlib.h>

#include "rcore.h"

//#include <android/sensor.h>           // Required for: Android sensors functions (accelerometer, gyroscope, light...)
#include <android/window.h>             // Required for: AWINDOW_FLAG_FULLSCREEN definition and others
#include <android_native_app_glue.h>    // Required for: android_app struct and activity management
#include <jni.h>                        // Required for: JNIEnv and JavaVM [Used in OpenURL()]

#include <EGL/egl.h>                    // Native platform windowing system interface
//#include <GLES2/gl2.h>                // OpenGL ES 2.0 library (not required in this module, only in rlgl)

static bool InitGraphicsDevice(int width, int height);                                     // Initialize graphics device
static void AndroidCommandCallback(struct android_app *app, int32_t cmd);                  // Process Android activity lifecycle commands
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);          // Process Android inputs

// Initialize window and OpenGL context
// NOTE: data parameter could be used to pass any kind of required data to the initialization
void InitWindow(int width, int height, const char *title)
{
    TRACELOG(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);

    TRACELOG(LOG_INFO, "Supported raylib modules:");
    TRACELOG(LOG_INFO, "    > rcore:..... loaded (mandatory)");
    TRACELOG(LOG_INFO, "    > rlgl:...... loaded (mandatory)");
#if defined(SUPPORT_MODULE_RSHAPES)
    TRACELOG(LOG_INFO, "    > rshapes:... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rshapes:... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RTEXTURES)
    TRACELOG(LOG_INFO, "    > rtextures:. loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rtextures:. not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RTEXT)
    TRACELOG(LOG_INFO, "    > rtext:..... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rtext:..... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RMODELS)
    TRACELOG(LOG_INFO, "    > rmodels:... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > rmodels:... not loaded (optional)");
#endif
#if defined(SUPPORT_MODULE_RAUDIO)
    TRACELOG(LOG_INFO, "    > raudio:.... loaded (optional)");
#else
    TRACELOG(LOG_INFO, "    > raudio:.... not loaded (optional)");
#endif

    if ((title != NULL) && (title[0] != 0)) CORE.Window.title = title;

    // Initialize global input state
    memset(&CORE.Input, 0, sizeof(CORE.Input));
    CORE.Input.Keyboard.exitKey = KEY_ESCAPE;
    CORE.Input.Mouse.scale = (Vector2){ 1.0f, 1.0f };
    CORE.Input.Mouse.cursor = MOUSE_CURSOR_ARROW;
    CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
#if defined(SUPPORT_EVENTS_WAITING)
    CORE.Window.eventWaiting = true;
#endif

    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;
    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;

    // Set desired windows flags before initializing anything
    ANativeActivity_setWindowFlags(CORE.Android.app->activity, AWINDOW_FLAG_FULLSCREEN, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER

    int orientation = AConfiguration_getOrientation(CORE.Android.app->config);

    if (orientation == ACONFIGURATION_ORIENTATION_PORT) TRACELOG(LOG_INFO, "ANDROID: Window orientation set as portrait");
    else if (orientation == ACONFIGURATION_ORIENTATION_LAND) TRACELOG(LOG_INFO, "ANDROID: Window orientation set as landscape");

    // TODO: Automatic orientation doesn't seem to work
    if (width <= height)
    {
        AConfiguration_setOrientation(CORE.Android.app->config, ACONFIGURATION_ORIENTATION_PORT);
        TRACELOG(LOG_WARNING, "ANDROID: Window orientation changed to portrait");
    }
    else
    {
        AConfiguration_setOrientation(CORE.Android.app->config, ACONFIGURATION_ORIENTATION_LAND);
        TRACELOG(LOG_WARNING, "ANDROID: Window orientation changed to landscape");
    }

    //AConfiguration_getDensity(CORE.Android.app->config);
    //AConfiguration_getKeyboard(CORE.Android.app->config);
    //AConfiguration_getScreenSize(CORE.Android.app->config);
    //AConfiguration_getScreenLong(CORE.Android.app->config);

    // Initialize App command system
    // NOTE: On APP_CMD_INIT_WINDOW -> InitGraphicsDevice(), InitTimer(), LoadFontDefault()...
    CORE.Android.app->onAppCmd = AndroidCommandCallback;

    // Initialize input events system
    CORE.Android.app->onInputEvent = AndroidInputCallback;

    // Initialize assets manager
    InitAssetManager(CORE.Android.app->activity->assetManager, CORE.Android.app->activity->internalDataPath);

    // Initialize base path for storage
    CORE.Storage.basePath = CORE.Android.app->activity->internalDataPath;

    TRACELOG(LOG_INFO, "ANDROID: App initialized successfully");

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Wait for window to be initialized (display and context)
    while (!CORE.Window.ready)
    {
        // Process events loop
        while ((pollResult = ALooper_pollAll(0, NULL, &pollEvents, (void**)&CORE.Android.source)) >= 0)
        {
            // Process this event
            if (CORE.Android.source != NULL) CORE.Android.source->process(CORE.Android.app, CORE.Android.source);

            // NOTE: Never close window, native activity is controlled by the system!
            //if (CORE.Android.app->destroyRequested != 0) CORE.Window.shouldClose = true;
        }
    }
}


// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
// NOTE: returns false in case graphic device could not be created
static bool InitGraphicsDevice(int width, int height)
{
    CORE.Window.screen.width = width;            // User desired width
    CORE.Window.screen.height = height;          // User desired height
    CORE.Window.screenScale = MatrixIdentity();  // No draw scaling required by default

    // Set the window minimum and maximum default values to 0
    CORE.Window.windowMin.width  = 0;
    CORE.Window.windowMin.height = 0;
    CORE.Window.windowMax.width  = 0;
    CORE.Window.windowMax.height = 0;

    // NOTE: Framebuffer (render area - CORE.Window.render.width, CORE.Window.render.height) could include black bars...
    // ...in top-down or left-right to match display aspect ratio (no weird scaling)

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    glfwSetErrorCallback(ErrorCallback);
/*
    // TODO: Setup GLFW custom allocators to match raylib ones
    const GLFWallocator allocator = {
        .allocate = MemAlloc,
        .deallocate = MemFree,
        .reallocate = MemRealloc,
        .user = NULL
    };

    glfwInitAllocator(&allocator);
*/
#if defined(__APPLE__)
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

    if (!glfwInit())
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW");
        return false;
    }

    glfwDefaultWindowHints();                       // Set default windows hints
    //glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    //glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    //glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    //glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    //glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) CORE.Window.fullscreen = true;

    if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);     // Window initially hidden

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);   // Decorated window

    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Avoid window being resizable

    // Disable FLAG_WINDOW_MINIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;

    // Disable FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;

    if ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if ((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    // NOTE: Some GLFW flags are not supported on HTML5
#if defined(PLATFORM_DESKTOP)
    if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);     // Transparent framebuffer
    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);  // Opaque framebuffer

    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        // Resize window content area based on the monitor content scale.
        // NOTE: This hint only has an effect on platforms where screen coordinates and pixels always map 1:1 such as Windows and X11.
        // On platforms like macOS the resolution of the framebuffer is changed independently of the window size.
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);   // Scale content area based on the monitor content scale where window is placed on
    #if defined(__APPLE__)
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #endif
    }
    else glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    // Mouse passthrough
    if ((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    else glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
#endif

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4);   // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);          // Choose OpenGL minor version (just hint)
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);  // OSX Requires forward compatibility
#else
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Forward Compatibility Hint: Only 3.3 and above!
#endif
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // Enable OpenGL Debug Context
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20)                 // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#if defined(PLATFORM_DESKTOP)
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30)                 // Request OpenGL ES 3.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#if defined(PLATFORM_DESKTOP)
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
#endif
    }

#if defined(PLATFORM_DESKTOP)
    // NOTE: GLFW 3.4+ defers initialization of the Joystick subsystem on the first call to any Joystick related functions.
    // Forcing this initialization here avoids doing it on PollInputEvents() called by EndDrawing() after first frame has been just drawn.
    // The initialization will still happen and possible delays still occur, but before the window is shown, which is a nicer experience.
    // REF: https://github.com/raysan5/raylib/issues/1554
    if (MAX_GAMEPADS > 0) glfwSetJoystickCallback(NULL);
#endif

#if defined(PLATFORM_DESKTOP)
    // Find monitor resolution
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to get primary monitor");
        return false;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    CORE.Window.display.width = mode->width;
    CORE.Window.display.height = mode->height;

    // Set screen width/height to the display width/height if they are 0
    if (CORE.Window.screen.width == 0) CORE.Window.screen.width = CORE.Window.display.width;
    if (CORE.Window.screen.height == 0) CORE.Window.screen.height = CORE.Window.display.height;
#endif  // PLATFORM_DESKTOP

#if defined(PLATFORM_WEB)
    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
#endif  // PLATFORM_WEB

    if (CORE.Window.fullscreen)
    {
        // remember center for switchinging from fullscreen to window
        if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
        {
            // If screen width/height equal to the display, we can't calculate the window pos for toggling full-screened/windowed.
            // Toggling full-screened/windowed with pos(0, 0) can cause problems in some platforms, such as X11.
            CORE.Window.position.x = CORE.Window.display.width/4;
            CORE.Window.position.y = CORE.Window.display.height/4;
        }
        else
        {
            CORE.Window.position.x = CORE.Window.display.width/2 - CORE.Window.screen.width/2;
            CORE.Window.position.y = CORE.Window.display.height/2 - CORE.Window.screen.height/2;
        }

        if (CORE.Window.position.x < 0) CORE.Window.position.x = 0;
        if (CORE.Window.position.y < 0) CORE.Window.position.y = 0;

        // Obtain recommended CORE.Window.display.width/CORE.Window.display.height from a valid videomode for the monitor
        int count = 0;
        const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

        // Get closest video mode to desired CORE.Window.screen.width/CORE.Window.screen.height
        for (int i = 0; i < count; i++)
        {
            if ((unsigned int)modes[i].width >= CORE.Window.screen.width)
            {
                if ((unsigned int)modes[i].height >= CORE.Window.screen.height)
                {
                    CORE.Window.display.width = modes[i].width;
                    CORE.Window.display.height = modes[i].height;
                    break;
                }
            }
        }
        TRACELOG(LOG_WARNING, "SYSTEM: Closest fullscreen videomode: %i x %i", CORE.Window.display.width, CORE.Window.display.height);

        // NOTE: ISSUE: Closest videomode could not match monitor aspect-ratio, for example,
        // for a desired screen size of 800x450 (16:9), closest supported videomode is 800x600 (4:3),
        // framebuffer is rendered correctly but once displayed on a 16:9 monitor, it gets stretched
        // by the sides to fit all monitor space...

        // Try to setup the most appropriate fullscreen framebuffer for the requested screenWidth/screenHeight
        // It considers device display resolution mode and setups a framebuffer with black bars if required (render size/offset)
        // Modified global variables: CORE.Window.screen.width/CORE.Window.screen.height - CORE.Window.render.width/CORE.Window.render.height - CORE.Window.renderOffset.x/CORE.Window.renderOffset.y - CORE.Window.screenScale
        // TODO: It is a quite cumbersome solution to display size vs requested size, it should be reviewed or removed...
        // HighDPI monitors are properly considered in a following similar function: SetupViewport()
        SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

        CORE.Window.handle = glfwCreateWindow(CORE.Window.display.width, CORE.Window.display.height, (CORE.Window.title != 0)? CORE.Window.title : " ", glfwGetPrimaryMonitor(), NULL);

        // NOTE: Full-screen change, not working properly...
        //glfwSetWindowMonitor(CORE.Window.handle, glfwGetPrimaryMonitor(), 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
    }
    else
    {
#if defined(PLATFORM_DESKTOP)
        // If we are windowed fullscreen, ensures that window does not minimize when focus is lost
        if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
        {
            glfwWindowHint(GLFW_AUTO_ICONIFY, 0);
        }
#endif
        // No-fullscreen window creation
        CORE.Window.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, (CORE.Window.title != 0)? CORE.Window.title : " ", NULL, NULL);

        if (CORE.Window.handle)
        {
            CORE.Window.render.width = CORE.Window.screen.width;
            CORE.Window.render.height = CORE.Window.screen.height;
        }
    }

    if (!CORE.Window.handle)
    {
        glfwTerminate();
        TRACELOG(LOG_WARNING, "GLFW: Failed to initialize Window");
        return false;
    }

// glfwCreateWindow title doesn't work with emscripten.
#if defined(PLATFORM_WEB)
    emscripten_set_window_title((CORE.Window.title != 0)? CORE.Window.title : " ");
#endif

    // Set window callback events
    glfwSetWindowSizeCallback(CORE.Window.handle, WindowSizeCallback);      // NOTE: Resizing not allowed by default!
#if !defined(PLATFORM_WEB)
    glfwSetWindowMaximizeCallback(CORE.Window.handle, WindowMaximizeCallback);
#endif
    glfwSetWindowIconifyCallback(CORE.Window.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(CORE.Window.handle, WindowFocusCallback);
    glfwSetDropCallback(CORE.Window.handle, WindowDropCallback);

    // Set input callback events
    glfwSetKeyCallback(CORE.Window.handle, KeyCallback);
    glfwSetCharCallback(CORE.Window.handle, CharCallback);
    glfwSetMouseButtonCallback(CORE.Window.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(CORE.Window.handle, MouseCursorPosCallback);   // Track mouse position changes
    glfwSetScrollCallback(CORE.Window.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(CORE.Window.handle, CursorEnterCallback);

    glfwMakeContextCurrent(CORE.Window.handle);

#if !defined(PLATFORM_WEB)
    glfwSetInputMode(CORE.Window.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);    // Enable lock keys modifiers (CAPS, NUM)

    glfwSwapInterval(0);        // No V-Sync by default
#endif

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration, it doesn't need
    // to be activated on web platforms since VSync is enforced there.
#if !defined(PLATFORM_WEB)
    if (CORE.Window.flags & FLAG_VSYNC_HINT)
    {
        // WARNING: It seems to hit a critical render path in Intel HD Graphics
        glfwSwapInterval(1);
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable VSYNC");
    }
#endif

    int fbWidth = CORE.Window.screen.width;
    int fbHeight = CORE.Window.screen.height;

#if defined(PLATFORM_DESKTOP)
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        // NOTE: On APPLE platforms system should manage window/input scaling and also framebuffer scaling.
        // Framebuffer scaling should be activated with: glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #if !defined(__APPLE__)
        glfwGetFramebufferSize(CORE.Window.handle, &fbWidth, &fbHeight);

        // Screen scaling matrix is required in case desired screen area is different from display area
        CORE.Window.screenScale = MatrixScale((float)fbWidth/CORE.Window.screen.width, (float)fbHeight/CORE.Window.screen.height, 1.0f);

        // Mouse input scaling for the new screen size
        SetMouseScale((float)CORE.Window.screen.width/fbWidth, (float)CORE.Window.screen.height/fbHeight);
    #endif
    }
#endif

    CORE.Window.render.width = fbWidth;
    CORE.Window.render.height = fbHeight;
    CORE.Window.currentFbo.width = fbWidth;
    CORE.Window.currentFbo.height = fbHeight;

    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

#endif  // PLATFORM_DESKTOP || PLATFORM_WEB

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

#if defined(PLATFORM_DRM)
    CORE.Window.fd = -1;
    CORE.Window.connector = NULL;
    CORE.Window.modeIndex = -1;
    CORE.Window.crtc = NULL;
    CORE.Window.gbmDevice = NULL;
    CORE.Window.gbmSurface = NULL;
    CORE.Window.prevBO = NULL;
    CORE.Window.prevFB = 0;

#if defined(DEFAULT_GRAPHIC_DEVICE_DRM)
    CORE.Window.fd = open(DEFAULT_GRAPHIC_DEVICE_DRM, O_RDWR);
#else
    TRACELOG(LOG_INFO, "DISPLAY: No graphic card set, trying platform-gpu-card");
    CORE.Window.fd = open("/dev/dri/by-path/platform-gpu-card",  O_RDWR); // VideoCore VI (Raspberry Pi 4)

    if ((-1 == CORE.Window.fd) || (drmModeGetResources(CORE.Window.fd) == NULL))
    {
        TRACELOG(LOG_INFO, "DISPLAY: Failed to open platform-gpu-card, trying card1");
        CORE.Window.fd = open("/dev/dri/card1", O_RDWR); // Other Embedded
    }

    if ((-1 == CORE.Window.fd) || (drmModeGetResources(CORE.Window.fd) == NULL))
    {
        TRACELOG(LOG_INFO, "DISPLAY: Failed to open graphic card1, trying card0");
        CORE.Window.fd = open("/dev/dri/card0", O_RDWR); // VideoCore IV (Raspberry Pi 1-3)
    }
#endif
    if (-1 == CORE.Window.fd)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to open graphic card");
        return false;
    }

    drmModeRes *res = drmModeGetResources(CORE.Window.fd);
    if (!res)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed get DRM resources");
        return false;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: Connectors found: %i", res->count_connectors);
    for (size_t i = 0; i < res->count_connectors; i++)
    {
        TRACELOG(LOG_TRACE, "DISPLAY: Connector index %i", i);
        drmModeConnector *con = drmModeGetConnector(CORE.Window.fd, res->connectors[i]);
        TRACELOG(LOG_TRACE, "DISPLAY: Connector modes detected: %i", con->count_modes);
        if ((con->connection == DRM_MODE_CONNECTED) && (con->encoder_id))
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode connected");
            CORE.Window.connector = con;
            break;
        }
        else
        {
            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode NOT connected (deleting)");
            drmModeFreeConnector(con);
        }
    }

    if (!CORE.Window.connector)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: No suitable DRM connector found");
        drmModeFreeResources(res);
        return false;
    }

    drmModeEncoder *enc = drmModeGetEncoder(CORE.Window.fd, CORE.Window.connector->encoder_id);
    if (!enc)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode encoder");
        drmModeFreeResources(res);
        return false;
    }

    CORE.Window.crtc = drmModeGetCrtc(CORE.Window.fd, enc->crtc_id);
    if (!CORE.Window.crtc)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode crtc");
        drmModeFreeEncoder(enc);
        drmModeFreeResources(res);
        return false;
    }

    // If InitWindow should use the current mode find it in the connector's mode list
    if ((CORE.Window.screen.width <= 0) || (CORE.Window.screen.height <= 0))
    {
        TRACELOG(LOG_TRACE, "DISPLAY: Selecting DRM connector mode for current used mode...");

        CORE.Window.modeIndex = FindMatchingConnectorMode(CORE.Window.connector, &CORE.Window.crtc->mode);

        if (CORE.Window.modeIndex < 0)
        {
            TRACELOG(LOG_WARNING, "DISPLAY: No matching DRM connector mode found");
            drmModeFreeEncoder(enc);
            drmModeFreeResources(res);
            return false;
        }

        CORE.Window.screen.width = CORE.Window.display.width;
        CORE.Window.screen.height = CORE.Window.display.height;
    }

    const bool allowInterlaced = CORE.Window.flags & FLAG_INTERLACED_HINT;
    const int fps = (CORE.Time.target > 0) ? (1.0/CORE.Time.target) : 60;

    // Try to find an exact matching mode
    CORE.Window.modeIndex = FindExactConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);

    // If nothing found, try to find a nearly matching mode
    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindNearestConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);

    // If nothing found, try to find an exactly matching mode including interlaced
    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindExactConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);

    // If nothing found, try to find a nearly matching mode including interlaced
    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindNearestConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);

    // If nothing found, there is no suitable mode
    if (CORE.Window.modeIndex < 0)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable DRM connector mode");
        drmModeFreeEncoder(enc);
        drmModeFreeResources(res);
        return false;
    }

    CORE.Window.display.width = CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay;
    CORE.Window.display.height = CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay;

    TRACELOG(LOG_INFO, "DISPLAY: Selected DRM connector mode %s (%ux%u%c@%u)", CORE.Window.connector->modes[CORE.Window.modeIndex].name,
        CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay, CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay,
        (CORE.Window.connector->modes[CORE.Window.modeIndex].flags & DRM_MODE_FLAG_INTERLACE) ? 'i' : 'p',
        CORE.Window.connector->modes[CORE.Window.modeIndex].vrefresh);

    // Use the width and height of the surface for render
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;

    drmModeFreeEncoder(enc);
    enc = NULL;

    drmModeFreeResources(res);
    res = NULL;

    CORE.Window.gbmDevice = gbm_create_device(CORE.Window.fd);
    if (!CORE.Window.gbmDevice)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM device");
        return false;
    }

    CORE.Window.gbmSurface = gbm_surface_create(CORE.Window.gbmDevice, CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay,
        CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay, GBM_FORMAT_ARGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if (!CORE.Window.gbmSurface)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM surface");
        return false;
    }
#endif

    EGLint samples = 0;
    EGLint sampleBuffer = 0;
    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        samples = 4;
        sampleBuffer = 1;
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
    }

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, (rlGetVersion() == RL_OPENGL_ES_30)? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,      // Type of context support
#if defined(PLATFORM_DRM)
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,          // Don't use it on Android!
#endif
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
#if defined(PLATFORM_DRM)
        EGL_ALPHA_SIZE, 8,        // ALPHA bit depth (required for transparent framebuffer)
#endif
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    const EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
    EGLint numConfigs = 0;

    // Get an EGL device connection
#if defined(PLATFORM_DRM)
    CORE.Window.device = eglGetDisplay((EGLNativeDisplayType)CORE.Window.gbmDevice);
#else
    CORE.Window.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif
    if (CORE.Window.device == EGL_NO_DISPLAY)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

    // Initialize the EGL device connection
    if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

#if defined(PLATFORM_DRM)
    if (!eglChooseConfig(CORE.Window.device, NULL, NULL, 0, &numConfigs))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config count: 0x%x", eglGetError());
        return false;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: EGL configs available: %d", numConfigs);

    EGLConfig *configs = RL_CALLOC(numConfigs, sizeof(*configs));
    if (!configs)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get memory for EGL configs");
        return false;
    }

    EGLint matchingNumConfigs = 0;
    if (!eglChooseConfig(CORE.Window.device, framebufferAttribs, configs, numConfigs, &matchingNumConfigs))
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to choose EGL config: 0x%x", eglGetError());
        free(configs);
        return false;
    }

    TRACELOG(LOG_TRACE, "DISPLAY: EGL matching configs available: %d", matchingNumConfigs);

    // find the EGL config that matches the previously setup GBM format
    int found = 0;
    for (EGLint i = 0; i < matchingNumConfigs; ++i)
    {
        EGLint id = 0;
        if (!eglGetConfigAttrib(CORE.Window.device, configs[i], EGL_NATIVE_VISUAL_ID, &id))
        {
            TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config attribute: 0x%x", eglGetError());
            continue;
        }

        if (GBM_FORMAT_ARGB8888 == id)
        {
            TRACELOG(LOG_TRACE, "DISPLAY: Using EGL config: %d", i);
            CORE.Window.config = configs[i];
            found = 1;
            break;
        }
    }

    RL_FREE(configs);

    if (!found)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable EGL config");
        return false;
    }
#else
    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(CORE.Window.device, framebufferAttribs, &CORE.Window.config, 1, &numConfigs);
#endif

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    CORE.Window.context = eglCreateContext(CORE.Window.device, CORE.Window.config, EGL_NO_CONTEXT, contextAttribs);
    if (CORE.Window.context == EGL_NO_CONTEXT)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
        return false;
    }
#endif

    // Create an EGL window surface
    //---------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    EGLint displayFormat = 0;

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(CORE.Window.device, CORE.Window.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables:
    //  -> CORE.Window.screen.width/CORE.Window.screen.height
    //  -> CORE.Window.render.width/CORE.Window.render.height
    //  -> CORE.Window.screenScale
    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

    ANativeWindow_setBuffersGeometry(CORE.Android.app->window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);
    //ANativeWindow_setBuffersGeometry(CORE.Android.app->window, 0, 0, displayFormat);       // Force use of native display size

    CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, CORE.Android.app->window, NULL);
#endif  // PLATFORM_ANDROID

#if defined(PLATFORM_DRM)
    CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, (EGLNativeWindowType)CORE.Window.gbmSurface, NULL);
    if (EGL_NO_SURFACE == CORE.Window.surface)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL window surface: 0x%04x", eglGetError());
        return false;
    }

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables:
    //  -> CORE.Window.screen.width/CORE.Window.screen.height
    //  -> CORE.Window.render.width/CORE.Window.render.height
    //  -> CORE.Window.screenScale
    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);
#endif  // PLATFORM_DRM

    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(CORE.Window.device, 1);

    if (eglMakeCurrent(CORE.Window.device, CORE.Window.surface, CORE.Window.surface, CORE.Window.context) == EGL_FALSE)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
        return false;
    }
    else
    {
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;

        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
    }
#endif  // PLATFORM_ANDROID || PLATFORM_DRM

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    rlLoadExtensions(glfwGetProcAddress);
#else
    rlLoadExtensions(eglGetProcAddress);
#endif

    // Initialize OpenGL context (states and resources)
    // NOTE: CORE.Window.currentFbo.width and CORE.Window.currentFbo.height not used, just stored as globals in rlgl
    rlglInit(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

    // Setup default viewport
    // NOTE: It updated CORE.Window.render.width and CORE.Window.render.height
    SetupViewport(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

#if defined(PLATFORM_ANDROID)
    CORE.Window.ready = true;
#endif

    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) MinimizeWindow();

    return true;
}

// ANDROID: Process activity lifecycle commands
static void AndroidCommandCallback(struct android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_START:
        {
            //rendering = true;
        } break;
        case APP_CMD_RESUME: break;
        case APP_CMD_INIT_WINDOW:
        {
            if (app->window != NULL)
            {
                if (CORE.Android.contextRebindRequired)
                {
                    // Reset screen scaling to full display size
                    EGLint displayFormat = 0;
                    eglGetConfigAttrib(CORE.Window.device, CORE.Window.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

                    // Adding renderOffset here feels rather hackish, but the viewport scaling is wrong after the
                    // context rebinding if the screen is scaled unless offsets are added. There's probably a more
                    // appropriate way to fix this
                    ANativeWindow_setBuffersGeometry(app->window,
                        CORE.Window.render.width + CORE.Window.renderOffset.x,
                        CORE.Window.render.height + CORE.Window.renderOffset.y,
                        displayFormat);

                    // Recreate display surface and re-attach OpenGL context
                    CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, app->window, NULL);
                    eglMakeCurrent(CORE.Window.device, CORE.Window.surface, CORE.Window.surface, CORE.Window.context);

                    CORE.Android.contextRebindRequired = false;
                }
                else
                {
                    CORE.Window.display.width = ANativeWindow_getWidth(CORE.Android.app->window);
                    CORE.Window.display.height = ANativeWindow_getHeight(CORE.Android.app->window);

                    // Initialize graphics device (display device and OpenGL context)
                    InitGraphicsDevice(CORE.Window.screen.width, CORE.Window.screen.height);

                    // Initialize hi-res timer
                    InitTimer();

                    // Initialize random seed
                    srand((unsigned int)time(NULL));

                #if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
                    // Load default font
                    // WARNING: External function: Module required: rtext
                    LoadFontDefault();
                    Rectangle rec = GetFontDefault().recs[95];
                    // NOTE: We setup a 1px padding on char rectangle to avoid pixel bleeding on MSAA filtering
                    #if defined(SUPPORT_MODULE_RSHAPES)
                    SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });  // WARNING: Module required: rshapes
                    #endif
                #endif

                    // TODO: GPU assets reload in case of lost focus (lost context)
                    // NOTE: This problem has been solved just unbinding and rebinding context from display
                    /*
                    if (assetsReloadRequired)
                    {
                        for (int i = 0; i < assetCount; i++)
                        {
                            // TODO: Unload old asset if required

                            // Load texture again to pointed texture
                            (*textureAsset + i) = LoadTexture(assetPath[i]);
                        }
                    }
                    */
                }
            }
        } break;
        case APP_CMD_GAINED_FOCUS:
        {
            CORE.Android.appEnabled = true;
            //ResumeMusicStream();
        } break;
        case APP_CMD_PAUSE: break;
        case APP_CMD_LOST_FOCUS:
        {
            CORE.Android.appEnabled = false;
            //PauseMusicStream();
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            // Detach OpenGL context and destroy display surface
            // NOTE 1: This case is used when the user exits the app without closing it. We detach the context to ensure everything is recoverable upon resuming.
            // NOTE 2: Detaching context before destroying display surface avoids losing our resources (textures, shaders, VBOs...)
            // NOTE 3: In some cases (too many context loaded), OS could unload context automatically... :(
            if (CORE.Window.device != EGL_NO_DISPLAY)
            {
                eglMakeCurrent(CORE.Window.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

                if (CORE.Window.surface != EGL_NO_SURFACE)
                {
                    eglDestroySurface(CORE.Window.device, CORE.Window.surface);
                    CORE.Window.surface = EGL_NO_SURFACE;
                }

                CORE.Android.contextRebindRequired = true;
            }
            // If 'CORE.Window.device' is already set to 'EGL_NO_DISPLAY'
            // this means that the user has already called 'CloseWindow()'

        } break;
        case APP_CMD_SAVE_STATE: break;
        case APP_CMD_STOP: break;
        case APP_CMD_DESTROY: break;
        case APP_CMD_CONFIG_CHANGED:
        {
            //AConfiguration_fromAssetManager(CORE.Android.app->config, CORE.Android.app->activity->assetManager);
            //print_cur_config(CORE.Android.app);

            // Check screen orientation here!
        } break;
        default: break;
    }
}

static GamepadButton AndroidTranslateGamepadButton(int button)
{
    switch (button)
    {
        case AKEYCODE_BUTTON_A: return GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
        case AKEYCODE_BUTTON_B: return GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
        case AKEYCODE_BUTTON_X: return GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
        case AKEYCODE_BUTTON_Y: return GAMEPAD_BUTTON_RIGHT_FACE_UP;
        case AKEYCODE_BUTTON_L1: return GAMEPAD_BUTTON_LEFT_TRIGGER_1;
        case AKEYCODE_BUTTON_R1: return GAMEPAD_BUTTON_RIGHT_TRIGGER_1;
        case AKEYCODE_BUTTON_L2: return GAMEPAD_BUTTON_LEFT_TRIGGER_2;
        case AKEYCODE_BUTTON_R2: return GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
        case AKEYCODE_BUTTON_THUMBL: return GAMEPAD_BUTTON_LEFT_THUMB;
        case AKEYCODE_BUTTON_THUMBR: return GAMEPAD_BUTTON_RIGHT_THUMB;
        case AKEYCODE_BUTTON_START: return GAMEPAD_BUTTON_MIDDLE_RIGHT;
        case AKEYCODE_BUTTON_SELECT: return GAMEPAD_BUTTON_MIDDLE_LEFT;
        case AKEYCODE_BUTTON_MODE: return GAMEPAD_BUTTON_MIDDLE;
        // On some (most?) gamepads dpad events are reported as axis motion instead
        case AKEYCODE_DPAD_DOWN: return GAMEPAD_BUTTON_LEFT_FACE_DOWN;
        case AKEYCODE_DPAD_RIGHT: return GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
        case AKEYCODE_DPAD_LEFT: return GAMEPAD_BUTTON_LEFT_FACE_LEFT;
        case AKEYCODE_DPAD_UP: return GAMEPAD_BUTTON_LEFT_FACE_UP;
        default: return GAMEPAD_BUTTON_UNKNOWN;
    }
}

// ANDROID: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    // If additional inputs are required check:
    // https://developer.android.com/ndk/reference/group/input
    // https://developer.android.com/training/game-controllers/controller-input

    int type = AInputEvent_getType(event);
    int source = AInputEvent_getSource(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        if (((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) ||
            ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD))
        {
            // For now we'll assume a single gamepad which we "detect" on its input event
            CORE.Input.Gamepad.ready[0] = true;

            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_LEFT_X] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_X, 0);
            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_LEFT_Y] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_Y, 0);
            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_RIGHT_X] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_Z, 0);
            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_RIGHT_Y] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_RZ, 0);
            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_LEFT_TRIGGER] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_BRAKE, 0) * 2.0f - 1.0f;
            CORE.Input.Gamepad.axisState[0][GAMEPAD_AXIS_RIGHT_TRIGGER] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_GAS, 0) * 2.0f - 1.0f;

            // dpad is reported as an axis on android
            float dpadX = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0);
            float dpadY = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0);

            if (dpadX == 1.0f)
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 1;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 0;
            }
            else if (dpadX == -1.0f)
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 0;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 1;
            }
            else
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 0;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 0;
            }

            if (dpadY == 1.0f)
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 1;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_UP] = 0;
            }
            else if (dpadY == -1.0f)
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 0;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_UP] = 1;
            }
            else
            {
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 0;
                CORE.Input.Gamepad.currentButtonState[0][GAMEPAD_BUTTON_LEFT_FACE_UP] = 0;
            }

            return 1; // Handled gamepad axis motion
        }
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);

        // Handle gamepad button presses and releases
        if (((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) ||
            ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD))
        {
            // For now we'll assume a single gamepad which we "detect" on its input event
            CORE.Input.Gamepad.ready[0] = true;

            GamepadButton button = AndroidTranslateGamepadButton(keycode);

            if (button == GAMEPAD_BUTTON_UNKNOWN) return 1;

            if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            {
                CORE.Input.Gamepad.currentButtonState[0][button] = 1;
            }
            else CORE.Input.Gamepad.currentButtonState[0][button] = 0;  // Key up

            return 1; // Handled gamepad button
        }

        // Save current button and its state
        // NOTE: Android key action is 0 for down and 1 for up
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
        {
            CORE.Input.Keyboard.currentKeyState[keycode] = 1;   // Key down

            CORE.Input.Keyboard.keyPressedQueue[CORE.Input.Keyboard.keyPressedQueueCount] = keycode;
            CORE.Input.Keyboard.keyPressedQueueCount++;
        }
        else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_MULTIPLE) CORE.Input.Keyboard.keyRepeatInFrame[keycode] = 1;
        else CORE.Input.Keyboard.currentKeyState[keycode] = 0;  // Key up

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

        return 0;
    }

    // Register touch points count
    CORE.Input.Touch.pointCount = AMotionEvent_getPointerCount(event);

    for (int i = 0; (i < CORE.Input.Touch.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        // Register touch points id
        CORE.Input.Touch.pointId[i] = AMotionEvent_getPointerId(event, i);

        // Register touch points position
        CORE.Input.Touch.position[i] = (Vector2){ AMotionEvent_getX(event, i), AMotionEvent_getY(event, i) };

        // Normalize CORE.Input.Touch.position[i] for CORE.Window.screen.width and CORE.Window.screen.height
        float widthRatio = (float)(CORE.Window.screen.width + CORE.Window.renderOffset.x) / (float)CORE.Window.display.width;
        float heightRatio = (float)(CORE.Window.screen.height + CORE.Window.renderOffset.y) / (float)CORE.Window.display.height;
        CORE.Input.Touch.position[i].x = CORE.Input.Touch.position[i].x * widthRatio - (float)CORE.Window.renderOffset.x / 2;
        CORE.Input.Touch.position[i].y = CORE.Input.Touch.position[i].y * heightRatio - (float)CORE.Window.renderOffset.y / 2;
    }

    int32_t action = AMotionEvent_getAction(event);
    unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

#if defined(SUPPORT_GESTURES_SYSTEM)        // PLATFORM_ANDROID
    GestureEvent gestureEvent = { 0 };

    gestureEvent.pointCount = CORE.Input.Touch.pointCount;

    // Register touch actions
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_ACTION_UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_ACTION_MOVE;
    else if (flags == AMOTION_EVENT_ACTION_CANCEL) gestureEvent.touchAction = TOUCH_ACTION_CANCEL;

    for (int i = 0; (i < gestureEvent.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        gestureEvent.pointId[i] = CORE.Input.Touch.pointId[i];
        gestureEvent.position[i] = CORE.Input.Touch.position[i];
    }

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#endif

    int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    if (flags == AMOTION_EVENT_ACTION_POINTER_UP || flags == AMOTION_EVENT_ACTION_UP)
    {
        // One of the touchpoints is released, remove it from touch point arrays
        for (int i = pointerIndex; (i < CORE.Input.Touch.pointCount - 1) && (i < MAX_TOUCH_POINTS); i++)
        {
            CORE.Input.Touch.pointId[i] = CORE.Input.Touch.pointId[i+1];
            CORE.Input.Touch.position[i] = CORE.Input.Touch.position[i+1];
        }

        CORE.Input.Touch.pointCount--;
    }

    // When all touchpoints are tapped and released really quickly, this event is generated
    if (flags == AMOTION_EVENT_ACTION_CANCEL) CORE.Input.Touch.pointCount = 0;

    if (CORE.Input.Touch.pointCount > 0) CORE.Input.Touch.currentTouchState[MOUSE_BUTTON_LEFT] = 1;
    else CORE.Input.Touch.currentTouchState[MOUSE_BUTTON_LEFT] = 0;

    return 0;
}

// To allow easier porting to android, we allow the user to define a
// main function which we call from android_main, defined by ourselves
extern int main(int argc, char *argv[]);

void android_main(struct android_app *app)
{
    char arg0[] = "raylib";     // NOTE: argv[] are mutable
    CORE.Android.app = app;

    // NOTE: Return from main is ignored
    (void)main(1, (char *[]) { arg0, NULL });

    // Request to end the native activity
    ANativeActivity_finish(app->activity);

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Waiting for application events before complete finishing
    while (!app->destroyRequested)
    {
        while ((pollResult = ALooper_pollAll(0, NULL, &pollEvents, (void **)&CORE.Android.source)) >= 0)
        {
            if (CORE.Android.source != NULL) CORE.Android.source->process(app, CORE.Android.source);
        }
    }
}

// NOTE: Add this to header (if apps really need it)
struct android_app *GetAndroidApp(void)
{
    return CORE.Android.app;
}

// Close window and unload OpenGL context
void CloseWindow(void)
{
#if defined(SUPPORT_GIF_RECORDING)
    if (gifRecording)
    {
        MsfGifResult result = msf_gif_end(&gifState);
        msf_gif_free(result);
        gifRecording = false;
    }
#endif

#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
    UnloadFontDefault();        // WARNING: Module required: rtext
#endif

    rlglClose();                // De-init rlgl

#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    timeEndPeriod(1);           // Restore time period
#endif

    // Close surface, context and display
    if (CORE.Window.device != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(CORE.Window.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (CORE.Window.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(CORE.Window.device, CORE.Window.surface);
            CORE.Window.surface = EGL_NO_SURFACE;
        }

        if (CORE.Window.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(CORE.Window.device, CORE.Window.context);
            CORE.Window.context = EGL_NO_CONTEXT;
        }

        eglTerminate(CORE.Window.device);
        CORE.Window.device = EGL_NO_DISPLAY;
    }

#if defined(SUPPORT_EVENTS_AUTOMATION)
    RL_FREE(events);
#endif

    CORE.Window.ready = false;
    TRACELOG(LOG_INFO, "Window closed successfully");
}


// Check if KEY_ESCAPE pressed or Close icon pressed
bool WindowShouldClose(void)
{
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Check if window is currently hidden
bool IsWindowHidden(void)
{
    return false;
}

// Check if window has been minimized
bool IsWindowMinimized(void)
{
    return false;
}


// Check if window has been maximized (only PLATFORM_DESKTOP)
bool IsWindowMaximized(void)
{
    return false;
}

// Check if window has the focus
bool IsWindowFocused(void)
{
    return CORE.Android.appEnabled;
}

// Check if window has been resizedLastFrame
bool IsWindowResized(void)
{
    return false;
}

// Toggle fullscreen mode (only PLATFORM_DESKTOP)
void ToggleFullscreen(void)
{
    TRACELOG(LOG_WARNING, "SYSTEM: Failed to toggle to windowed mode");
}

// Set window state: maximized, if resizable (only PLATFORM_DESKTOP)
void MaximizeWindow(void)
{
    TRACELOG(LOG_INFO, "MaximizeWindow not implemented in rcore_android.c");
}

// Set window state: minimized (only PLATFORM_DESKTOP)
void MinimizeWindow(void)
{
    TRACELOG(LOG_INFO, "MinimizeWindow not implemented in rcore_android.c");
}

// Set window state: not minimized/maximized (only PLATFORM_DESKTOP)
void RestoreWindow(void)
{
    TRACELOG(LOG_INFO, "RestoreWindow not implemented in rcore_android.c");
}


// Toggle borderless windowed mode (only PLATFORM_DESKTOP)
void ToggleBorderlessWindowed(void)
{
    TRACELOG(LOG_INFO, "ToggleBorderlessWindowed not implemented in rcore_android.c");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    TRACELOG(LOG_INFO, "SetWindowState not implemented in rcore_android.c");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    TRACELOG(LOG_INFO, "ClearWindowState not implemented in rcore_android.c");
}

// Set icon for window (only PLATFORM_DESKTOP)
// NOTE 1: Image must be in RGBA format, 8bit per channel
// NOTE 2: Image is scaled by the OS for all required sizes
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_INFO, "SetWindowIcon not implemented in rcore_android.c");
}

// Set icon for window (multiple images, only PLATFORM_DESKTOP)
// NOTE 1: Images must be in RGBA format, 8bit per channel
// NOTE 2: The multiple images are used depending on provided sizes
// Standard Windows icon sizes: 256, 128, 96, 64, 48, 32, 24, 16
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_INFO, "SetWindowIcons not implemented in rcore_android.c");
}

// Set title for window (only PLATFORM_DESKTOP and PLATFORM_WEB)
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_INFO, "SetWindowPosition not implemented in rcore_android.c");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_INFO, "SetWindowMonitor not implemented in rcore_android.c");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.windowMin.width = width;
    CORE.Window.windowMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.windowMax.width = width;
    CORE.Window.windowMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    TRACELOG(LOG_INFO, "SetWindowSize not implemented in rcore_android.c");
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_INFO, "SetWindowOpacity not implemented in rcore_android.c");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_INFO, "SetWindowFocused not implemented in rcore_android.c");

}

// Get native window handle
void *GetWindowHandle(void)
{
    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    return 1;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    if (CORE.Android.app->window != NULL)
    {
        return ANativeWindow_getWidth(CORE.Android.app->window);
    }
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    if (CORE.Android.app->window != NULL)
    {
        return ANativeWindow_getHeight(CORE.Android.app->window);
    }
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    return 0;
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    return (Vector2){ 1.0f, 1.0f };
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    return "";
}


// Set clipboard text content
void SetClipboardText(const char *text)
{
}


// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    return NULL;
}

// Show mouse cursor
void ShowCursor(void)
{
    CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void)
{
    CORE.Input.Mouse.cursorHidden = true;
}


// Enables cursor (unlock cursor)
void EnableCursor(void)
{
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = true;
}

// Get elapsed time measure in seconds since InitTimer()
// NOTE: On PLATFORM_DESKTOP InitTimer() is called on InitWindow()
// NOTE: On PLATFORM_DESKTOP, timer is initialized on glfwInit()
double GetTime(void)
{
    double time = 0.0;
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()
    return time;
}

// NOTE TRACELOG() function is located in [utils.h]

// Takes a screenshot of current screen (saved a .png)
void TakeScreenshot(const char *fileName)
{
#if defined(SUPPORT_MODULE_RTEXTURES)
    // Security check to (partially) avoid malicious code on PLATFORM_WEB
    if (strchr(fileName, '\'') != NULL) { TRACELOG(LOG_WARNING, "SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character");  return; }

    Vector2 scale = GetWindowScaleDPI();
    unsigned char *imgData = rlReadScreenPixels((int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
    Image image = { imgData, (int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    char path[2048] = { 0 };
    strcpy(path, TextFormat("%s/%s", CORE.Storage.basePath, fileName));

    ExportImage(image, path);           // WARNING: Module required: rtextures
    RL_FREE(imgData);

    TRACELOG(LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", path);
#else
    TRACELOG(LOG_WARNING,"IMAGE: ExportImage() requires module: rtextures");
#endif
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on PLATFORM_WEB
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        JNIEnv *env = NULL;
        JavaVM *vm = CORE.Android.app->activity->vm;
        (*vm)->AttachCurrentThread(vm, &env, NULL);

        jstring urlString = (*env)->NewStringUTF(env, url);
        jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
        jmethodID uriParse = (*env)->GetStaticMethodID(env, uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, uriParse, urlString);

        jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
        jfieldID actionViewId = (*env)->GetStaticFieldID(env, intentClass, "ACTION_VIEW", "Ljava/lang/String;");
        jobject actionView = (*env)->GetStaticObjectField(env, intentClass, actionViewId);
        jmethodID newIntent = (*env)->GetMethodID(env, intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
        jobject intent = (*env)->AllocObject(env, intentClass);

        (*env)->CallVoidMethod(env, intent, newIntent, actionView, uri);
        jclass activityClass = (*env)->FindClass(env, "android/app/Activity");
        jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity", "(Landroid/content/Intent;)V");
        (*env)->CallVoidMethod(env, CORE.Android.app->activity->clazz, startActivity, intent);

        (*vm)->DetachCurrentThread(vm);
    }
}

// Get gamepad internal name id
const char *GetGamepadName(int gamepad)
{
    return NULL;
}


// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    return 0;
}


// Get gamepad axis count
int GetGamepadAxisCount(int gamepad)
{
    return CORE.Input.Gamepad.axisCount;
}
