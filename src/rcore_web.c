#include <stdlib.h>

#include "rcore.h"

#define GLFW_INCLUDE_ES2 // GLFW3: Enable OpenGL ES 2.0 (translated to WebGL)
// #define GLFW_INCLUDE_ES3            // GLFW3: Enable OpenGL ES 3.0 (transalted to WebGL2?)
#include "GLFW/glfw3.h" // GLFW3: Windows, OpenGL context and Input management
#include <sys/time.h>   // Required for: timespec, nanosleep(), select() - POSIX

#include <emscripten/emscripten.h> // Emscripten functionality for C
#include <emscripten/html5.h>      // Emscripten HTML5 library

static bool InitGraphicsDevice(int width, int height); // Initialize graphics device

static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData);
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData);

static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);

static void ErrorCallback(int error, const char *description); // GLFW3 Error Callback, runs on GLFW3 error
// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);         // GLFW3 WindowSize Callback, runs when window is resized
static void WindowMaximizeCallback(GLFWwindow *window, int maximized);             // GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);              // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowFocusCallback(GLFWwindow *window, int focused);                  // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths); // GLFW3 Window Drop Callback, runs when drop files into window
// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods); // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                           // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);    // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);               // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);      // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                           // GLFW3 Cursor Enter Callback, cursor enters client area
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error
// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowFocusCallback(GLFWwindow *window, int focused);                          // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window
// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area

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
    CORE.Input.Mouse.scale = (Vector2){1.0f, 1.0f};
    CORE.Input.Mouse.cursor = MOUSE_CURSOR_ARROW;
    CORE.Input.Gamepad.lastButtonPressed = 0; // GAMEPAD_BUTTON_UNKNOWN
#if defined(SUPPORT_EVENTS_WAITING)
    CORE.Window.eventWaiting = true;
#endif
    // Initialize graphics device (display device and OpenGL context)
    // NOTE: returns true if window and graphic device has been initialized successfully
    CORE.Window.ready = InitGraphicsDevice(width, height);

    // If graphic device is no properly initialized, we end program
    if (!CORE.Window.ready)
    {
        TRACELOG(LOG_FATAL, "Failed to initialize Graphic Device");
        return;
    }
    else SetWindowPosition(GetMonitorWidth(GetCurrentMonitor()) / 2 - CORE.Window.screen.width / 2, GetMonitorHeight(GetCurrentMonitor()) / 2 - CORE.Window.screen.height / 2);

    // Initialize hi-res timer
    InitTimer();

    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Initialize base path for storage
    CORE.Storage.basePath = GetWorkingDirectory();

#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
    // Load default font
    // WARNING: External function: Module required: rtext
    LoadFontDefault();
#if defined(SUPPORT_MODULE_RSHAPES)
    // Set font white rectangle for shapes drawing, so shapes and text can be batched together
    // WARNING: rshapes module is required, if not available, default internal white rectangle is used
    Rectangle rec = GetFontDefault().recs[95];
    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        // NOTE: We try to maxime rec padding to avoid pixel bleeding on MSAA filtering
        SetShapesTexture(GetFontDefault().texture, (Rectangle){rec.x + 2, rec.y + 2, 1, 1});
    }
    else
    {
        // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
        SetShapesTexture(GetFontDefault().texture, (Rectangle){rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2});
    }
#endif
#else
#if defined(SUPPORT_MODULE_RSHAPES)
    // Set default texture and rectangle to be used for shapes drawing
    // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
    Texture2D texture = {rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    SetShapesTexture(texture, (Rectangle){0.0f, 0.0f, 1.0f, 1.0f}); // WARNING: Module required: rshapes
#endif
#endif
#if defined(SUPPORT_MODULE_RTEXT) && defined(SUPPORT_DEFAULT_FONT)
    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
    {
        // Set default font texture filter for HighDPI (blurry)
        // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlTextureParameters(GetFontDefault().texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
    }
#endif

    // Setup callback functions for the DOM events
    emscripten_set_fullscreenchange_callback("#canvas", NULL, 1, EmscriptenFullscreenChangeCallback);

    // WARNING: Below resize code was breaking fullscreen mode for sample games and examples, it needs review
    // Check fullscreen change events(note this is done on the window since most browsers don't support this on #canvas)
    // emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    // Check Resize event (note this is done on the window since most browsers don't support this on #canvas)
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    // Trigger this once to get initial window sizing
    EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);

    // Support keyboard events -> Not used, GLFW.JS takes care of that
    // emscripten_set_keypress_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);
    // emscripten_set_keydown_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);

    // Support mouse events
    emscripten_set_click_callback("#canvas", NULL, 1, EmscriptenMouseCallback);

    // Support touch events
    emscripten_set_touchstart_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchend_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchmove_callback("#canvas", NULL, 1, EmscriptenTouchCallback);
    emscripten_set_touchcancel_callback("#canvas", NULL, 1, EmscriptenTouchCallback);

    // Support gamepad events (not provided by GLFW3 on emscripten)
    emscripten_set_gamepadconnected_callback(NULL, 1, EmscriptenGamepadCallback);
    emscripten_set_gamepaddisconnected_callback(NULL, 1, EmscriptenGamepadCallback);

#if defined(SUPPORT_EVENTS_AUTOMATION)
    events = (AutomationEvent *)RL_CALLOC(MAX_CODE_AUTOMATION_EVENTS, sizeof(AutomationEvent));
    CORE.Time.frameCounter = 0;
#endif
}

// Register fullscreen change events
static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData)
{
    // TODO: Implement EmscriptenFullscreenChangeCallback()?

    return 1; // The event was consumed by the callback handler
}

// Register window resize event
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // TODO: Implement EmscriptenWindowResizedCallback()?

    return 1; // The event was consumed by the callback handler
}

EM_JS(int, GetWindowInnerWidth, (), { return window.innerWidth; });
EM_JS(int, GetWindowInnerHeight, (), { return window.innerHeight; });

// Register DOM element resize event
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // Don't resize non-resizeable windows
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) == 0) return 1;

    // This event is called whenever the window changes sizes,
    // so the size of the canvas object is explicitly retrieved below
    int width = GetWindowInnerWidth();
    int height = GetWindowInnerHeight();

    if (width < CORE.Window.screenMin.width) width = CORE.Window.screenMin.width;
    else if (width > CORE.Window.screenMax.width && CORE.Window.screenMax.width > 0) width = CORE.Window.screenMax.width;

    if (height < CORE.Window.screenMin.height) height = CORE.Window.screenMin.height;
    else if (height > CORE.Window.screenMax.height && CORE.Window.screenMax.height > 0) height = CORE.Window.screenMax.height;

    emscripten_set_canvas_element_size("#canvas", width, height);

    SetupViewport(width, height); // Reset viewport and projection matrix for new size

    CORE.Window.currentFbo.width = width;
    CORE.Window.currentFbo.height = height;
    CORE.Window.resizedLastFrame = true;

    if (IsWindowFullscreen()) return 1;

    // Set current screen size
    CORE.Window.screen.width = width;
    CORE.Window.screen.height = height;

    // NOTE: Postprocessing texture is not scaled to new size

    return 0;
}

// Register mouse input events
static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    // This is only for registering mouse click events with emscripten and doesn't need to do anything

    return 1; // The event was consumed by the callback handler
}

// Register connected/disconnected gamepads events
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData)
{
    /*
    TRACELOGD("%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%s\", mapping: \"%s\"",
           eventType != 0? emscripten_event_type_to_string(eventType) : "Gamepad state",
           gamepadEvent->timestamp, gamepadEvent->connected, gamepadEvent->index, gamepadEvent->numAxes, gamepadEvent->numButtons, gamepadEvent->id, gamepadEvent->mapping);

    for (int i = 0; i < gamepadEvent->numAxes; ++i) TRACELOGD("Axis %d: %g", i, gamepadEvent->axis[i]);
    for (int i = 0; i < gamepadEvent->numButtons; ++i) TRACELOGD("Button %d: Digital: %d, Analog: %g", i, gamepadEvent->digitalButton[i], gamepadEvent->analogButton[i]);
    */

    if ((gamepadEvent->connected) && (gamepadEvent->index < MAX_GAMEPADS))
    {
        CORE.Input.Gamepad.ready[gamepadEvent->index] = true;
        sprintf(CORE.Input.Gamepad.name[gamepadEvent->index], "%s", gamepadEvent->id);
    }
    else CORE.Input.Gamepad.ready[gamepadEvent->index] = false;

    return 1; // The event was consumed by the callback handler
}

// Register touch input events
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    // Register touch points count
    CORE.Input.Touch.pointCount = touchEvent->numTouches;

    double canvasWidth = 0.0;
    double canvasHeight = 0.0;
    // NOTE: emscripten_get_canvas_element_size() returns canvas.width and canvas.height but
    // we are looking for actual CSS size: canvas.style.width and canvas.style.height
    // EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    for (int i = 0; (i < CORE.Input.Touch.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        // Register touch points id
        CORE.Input.Touch.pointId[i] = touchEvent->touches[i].identifier;

        // Register touch points position
        CORE.Input.Touch.position[i] = (Vector2){touchEvent->touches[i].targetX, touchEvent->touches[i].targetY};

        // Normalize gestureEvent.position[x] for CORE.Window.screen.width and CORE.Window.screen.height
        CORE.Input.Touch.position[i].x *= ((float)GetScreenWidth() / (float)canvasWidth);
        CORE.Input.Touch.position[i].y *= ((float)GetScreenHeight() / (float)canvasHeight);

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) CORE.Input.Touch.currentTouchState[i] = 1;
        else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) CORE.Input.Touch.currentTouchState[i] = 0;
    }

#if defined(SUPPORT_GESTURES_SYSTEM) // PLATFORM_WEB
    GestureEvent gestureEvent = {0};

    gestureEvent.pointCount = CORE.Input.Touch.pointCount;

    // Register touch actions
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) gestureEvent.touchAction = TOUCH_ACTION_UP;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) gestureEvent.touchAction = TOUCH_ACTION_MOVE;
    else if (eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL) gestureEvent.touchAction = TOUCH_ACTION_CANCEL;

    for (int i = 0; (i < gestureEvent.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        gestureEvent.pointId[i] = CORE.Input.Touch.pointId[i];
        gestureEvent.position[i] = CORE.Input.Touch.position[i];

        // Normalize gestureEvent.position[i]
        gestureEvent.position[i].x /= (float)GetScreenWidth();
        gestureEvent.position[i].y /= (float)GetScreenHeight();
    }

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);

    // Reset the pointCount for web, if it was the last Touch End event
    if (eventType == EMSCRIPTEN_EVENT_TOUCHEND && CORE.Input.Touch.pointCount == 1) CORE.Input.Touch.pointCount = 0;
#endif

    return 1; // The event was consumed by the callback handler
}

// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
// NOTE: returns false in case graphic device could not be created
static bool InitGraphicsDevice(int width, int height)
{
    CORE.Window.screen.width = width;           // User desired width
    CORE.Window.screen.height = height;         // User desired height
    CORE.Window.screenScale = MatrixIdentity(); // No draw scaling required by default

    // Set the screen minimum and maximum default values to 0
    CORE.Window.screenMin.width  = 0;
    CORE.Window.screenMin.height = 0;
    CORE.Window.screenMax.width  = 0;
    CORE.Window.screenMax.height = 0;

    // NOTE: Framebuffer (render area - CORE.Window.render.width, CORE.Window.render.height) could include black bars...
    // ...in top-down or left-right to match display aspect ratio (no weird scaling)

// HERE 1 //#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
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
// HERE 2 //#if defined(__APPLE__)
// HERE 2 //    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
// HERE 2 //#endif

    if (!glfwInit())
    {
        TRACELOG(LOG_WARNING, "GLFW: Failed to initialize GLFW");
        return false;
    }

    glfwDefaultWindowHints(); // Set default windows hints
    // glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    // glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    // glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    // glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    // glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    // glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    // glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Check window creation flags
    if ((CORE.Window.flags & FLAG_FULLSCREEN_MODE) > 0) CORE.Window.fullscreen = true;

    if ((CORE.Window.flags & FLAG_WINDOW_HIDDEN) > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); // Window initially hidden

    if ((CORE.Window.flags & FLAG_WINDOW_UNDECORATED) > 0) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // Decorated window

    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) > 0) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Avoid window being resizable

    // Disable FLAG_WINDOW_MINIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MINIMIZED;

    // Disable FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if ((CORE.Window.flags & FLAG_WINDOW_MAXIMIZED) > 0) CORE.Window.flags &= ~FLAG_WINDOW_MAXIMIZED;

    if ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) > 0) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if ((CORE.Window.flags & FLAG_WINDOW_TOPMOST) > 0) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

        // NOTE: Some GLFW flags are not supported on HTML5
// HERE 3 //#if defined(PLATFORM_DESKTOP)
// HERE 3 //    if ((CORE.Window.flags & FLAG_WINDOW_TRANSPARENT) > 0) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // Transparent framebuffer
// HERE 3 //    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE); // Opaque framebuffer
// HERE 3 //
// HERE 3 //    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
// HERE 3 //    {
// HERE 3 //        // Resize window content area based on the monitor content scale.
// HERE 3 //        // NOTE: This hint only has an effect on platforms where screen coordinates and pixels always map 1:1 such as Windows and X11.
// HERE 3 //        // On platforms like macOS the resolution of the framebuffer is changed independently of the window size.
// HERE 3 //        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE); // Scale content area based on the monitor content scale where window is placed on
// HERE 3 //#if defined(__APPLE__)
// HERE 3 //        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
// HERE 3 //#endif
// HERE 3 //    }
// HERE 3 //    else glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
// HERE 3 //
// HERE 3 //    // Mouse passthrough
// HERE 3 //    if ((CORE.Window.flags & FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
// HERE 3 //    else glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
// HERE 3 //#endif

    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4); // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

    // Check selection OpenGL version
    if (rlGetVersion() == RL_OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // Choose OpenGL minor version (just hint)
    }
    else if (rlGetVersion() == RL_OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                 // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
// HERE 4 //#if defined(__APPLE__)
// HERE 4 //        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // OSX Requires forward compatibility
// HERE 4 //#else
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Forward Compatibility Hint: Only 3.3 and above!
// HERE 4 //#endif
        // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlGetVersion() == RL_OPENGL_43)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Enable OpenGL Debug Context
#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_20) // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
// HERE 5 //#if defined(PLATFORM_DESKTOP)
// HERE 5 //        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
// HERE 5 //#else
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
// HERE 5 //#endif
    }
    else if (rlGetVersion() == RL_OPENGL_ES_30) // Request OpenGL ES 3.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
// HERE 6 //#if defined(PLATFORM_DESKTOP)
// HERE 6 //        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
// HERE 6 //#else
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
// HERE 6 //#endif
    }

// HERE 7 //#if defined(PLATFORM_DESKTOP)
// HERE 7 //    // NOTE: GLFW 3.4+ defers initialization of the Joystick subsystem on the first call to any Joystick related functions.
// HERE 7 //    // Forcing this initialization here avoids doing it on PollInputEvents() called by EndDrawing() after first frame has been just drawn.
// HERE 7 //    // The initialization will still happen and possible delays still occur, but before the window is shown, which is a nicer experience.
// HERE 7 //    // REF: https://github.com/raysan5/raylib/issues/1554
// HERE 7 //    if (MAX_GAMEPADS > 0) glfwSetJoystickCallback(NULL);
// HERE 7 //#endif

// HERE 8 //#if defined(PLATFORM_DESKTOP)
// HERE 8 //    // Find monitor resolution
// HERE 8 //    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
// HERE 8 //    if (!monitor)
// HERE 8 //    {
// HERE 8 //        TRACELOG(LOG_WARNING, "GLFW: Failed to get primary monitor");
// HERE 8 //        return false;
// HERE 8 //    }
// HERE 8 //
// HERE 8 //    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
// HERE 8 //
// HERE 8 //    CORE.Window.display.width = mode->width;
// HERE 8 //    CORE.Window.display.height = mode->height;
// HERE 8 //
// HERE 8 //    // Set screen width/height to the display width/height if they are 0
// HERE 8 //    if (CORE.Window.screen.width == 0) CORE.Window.screen.width = CORE.Window.display.width;
// HERE 8 //    if (CORE.Window.screen.height == 0) CORE.Window.screen.height = CORE.Window.display.height;
// HERE 8 //#endif // PLATFORM_DESKTOP

// HERE 9 //#if defined(PLATFORM_WEB)
    // NOTE: Getting video modes is not implemented in emscripten GLFW3 version
    CORE.Window.display.width = CORE.Window.screen.width;
    CORE.Window.display.height = CORE.Window.screen.height;
// HERE 9 //#endif // PLATFORM_WEB

    if (CORE.Window.fullscreen)
    {
        // remember center for switchinging from fullscreen to window
        if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
        {
            // If screen width/height equal to the display, we can't calculate the window pos for toggling full-screened/windowed.
            // Toggling full-screened/windowed with pos(0, 0) can cause problems in some platforms, such as X11.
            CORE.Window.position.x = CORE.Window.display.width / 4;
            CORE.Window.position.y = CORE.Window.display.height / 4;
        }
        else
        {
            CORE.Window.position.x = CORE.Window.display.width / 2 - CORE.Window.screen.width / 2;
            CORE.Window.position.y = CORE.Window.display.height / 2 - CORE.Window.screen.height / 2;
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

        CORE.Window.handle = glfwCreateWindow(CORE.Window.display.width, CORE.Window.display.height, (CORE.Window.title != 0) ? CORE.Window.title : " ", glfwGetPrimaryMonitor(), NULL);

        // NOTE: Full-screen change, not working properly...
        // glfwSetWindowMonitor(CORE.Window.handle, glfwGetPrimaryMonitor(), 0, 0, CORE.Window.screen.width, CORE.Window.screen.height, GLFW_DONT_CARE);
    }
    else
    {
// HERE 10 //#if defined(PLATFORM_DESKTOP)
// HERE 10 //        // If we are windowed fullscreen, ensures that window does not minimize when focus is lost
// HERE 10 //        if ((CORE.Window.screen.height == CORE.Window.display.height) && (CORE.Window.screen.width == CORE.Window.display.width))
// HERE 10 //        {
// HERE 10 //            glfwWindowHint(GLFW_AUTO_ICONIFY, 0);
// HERE 10 //        }
// HERE 10 //#endif
        // No-fullscreen window creation
        CORE.Window.handle = glfwCreateWindow(CORE.Window.screen.width, CORE.Window.screen.height, (CORE.Window.title != 0) ? CORE.Window.title : " ", NULL, NULL);

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
// HERE 11 //#if defined(PLATFORM_WEB)
    emscripten_set_window_title((CORE.Window.title != 0) ? CORE.Window.title : " ");
// HERE 11 //#endif

    // Set window callback events
    glfwSetWindowSizeCallback(CORE.Window.handle, WindowSizeCallback); // NOTE: Resizing not allowed by default!
// HERE 12 //#if !defined(PLATFORM_WEB)
// HERE 12 //    glfwSetWindowMaximizeCallback(CORE.Window.handle, WindowMaximizeCallback);
// HERE 12 //#endif
    glfwSetWindowIconifyCallback(CORE.Window.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(CORE.Window.handle, WindowFocusCallback);
    glfwSetDropCallback(CORE.Window.handle, WindowDropCallback);

    // Set input callback events
    glfwSetKeyCallback(CORE.Window.handle, KeyCallback);
    glfwSetCharCallback(CORE.Window.handle, CharCallback);
    glfwSetMouseButtonCallback(CORE.Window.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(CORE.Window.handle, MouseCursorPosCallback); // Track mouse position changes
    glfwSetScrollCallback(CORE.Window.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(CORE.Window.handle, CursorEnterCallback);

    glfwMakeContextCurrent(CORE.Window.handle);

// HERE 13 //#if !defined(PLATFORM_WEB)
// HERE 13 //    glfwSetInputMode(CORE.Window.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE); // Enable lock keys modifiers (CAPS, NUM)
// HERE 13 //
// HERE 13 //    glfwSwapInterval(0); // No V-Sync by default
// HERE 13 //#endif

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration, it doesn't need
    // to be activated on web platforms since VSync is enforced there.
// HERE 14 //#if !defined(PLATFORM_WEB)
// HERE 14 //    if (CORE.Window.flags & FLAG_VSYNC_HINT)
// HERE 14 //    {
// HERE 14 //        // WARNING: It seems to hit a critical render path in Intel HD Graphics
// HERE 14 //        glfwSwapInterval(1);
// HERE 14 //        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable VSYNC");
// HERE 14 //    }
// HERE 14 //#endif

    int fbWidth = CORE.Window.screen.width;
    int fbHeight = CORE.Window.screen.height;

// HERE 15 //#if defined(PLATFORM_DESKTOP)
// HERE 15 //    if ((CORE.Window.flags & FLAG_WINDOW_HIGHDPI) > 0)
// HERE 15 //    {
// HERE 15 //        // NOTE: On APPLE platforms system should manage window/input scaling and also framebuffer scaling.
// HERE 15 //        // Framebuffer scaling should be activated with: glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
// HERE 15 //#if !defined(__APPLE__)
// HERE 15 //        glfwGetFramebufferSize(CORE.Window.handle, &fbWidth, &fbHeight);
// HERE 15 //
// HERE 15 //        // Screen scaling matrix is required in case desired screen area is different from display area
// HERE 15 //        CORE.Window.screenScale = MatrixScale((float)fbWidth / CORE.Window.screen.width, (float)fbHeight / CORE.Window.screen.height, 1.0f);
// HERE 15 //
// HERE 15 //        // Mouse input scaling for the new screen size
// HERE 15 //        SetMouseScale((float)CORE.Window.screen.width / fbWidth, (float)CORE.Window.screen.height / fbHeight);
// HERE 15 //#endif
// HERE 15 //    }
// HERE 15 //#endif

    CORE.Window.render.width = fbWidth;
    CORE.Window.render.height = fbHeight;
    CORE.Window.currentFbo.width = fbWidth;
    CORE.Window.currentFbo.height = fbHeight;

    TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
    TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
    TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
    TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);

// HERE 1 //#endif // PLATFORM_DESKTOP || PLATFORM_WEB

// HERE 16 //#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
// HERE 16 //    CORE.Window.fullscreen = true;
// HERE 16 //    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
// HERE 16 //
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //    CORE.Window.fd = -1;
// HERE 16 //    CORE.Window.connector = NULL;
// HERE 16 //    CORE.Window.modeIndex = -1;
// HERE 16 //    CORE.Window.crtc = NULL;
// HERE 16 //    CORE.Window.gbmDevice = NULL;
// HERE 16 //    CORE.Window.gbmSurface = NULL;
// HERE 16 //    CORE.Window.prevBO = NULL;
// HERE 16 //    CORE.Window.prevFB = 0;
// HERE 16 //
// HERE 16 //#if defined(DEFAULT_GRAPHIC_DEVICE_DRM)
// HERE 16 //    CORE.Window.fd = open(DEFAULT_GRAPHIC_DEVICE_DRM, O_RDWR);
// HERE 16 //#else
// HERE 16 //    TRACELOG(LOG_INFO, "DISPLAY: No graphic card set, trying platform-gpu-card");
// HERE 16 //    CORE.Window.fd = open("/dev/dri/by-path/platform-gpu-card", O_RDWR); // VideoCore VI (Raspberry Pi 4)
// HERE 16 //
// HERE 16 //    if ((-1 == CORE.Window.fd) || (drmModeGetResources(CORE.Window.fd) == NULL))
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_INFO, "DISPLAY: Failed to open platform-gpu-card, trying card1");
// HERE 16 //        CORE.Window.fd = open("/dev/dri/card1", O_RDWR); // Other Embedded
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    if ((-1 == CORE.Window.fd) || (drmModeGetResources(CORE.Window.fd) == NULL))
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_INFO, "DISPLAY: Failed to open graphic card1, trying card0");
// HERE 16 //        CORE.Window.fd = open("/dev/dri/card0", O_RDWR); // VideoCore IV (Raspberry Pi 1-3)
// HERE 16 //    }
// HERE 16 //#endif
// HERE 16 //    if (-1 == CORE.Window.fd)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to open graphic card");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    drmModeRes *res = drmModeGetResources(CORE.Window.fd);
// HERE 16 //    if (!res)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed get DRM resources");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    TRACELOG(LOG_TRACE, "DISPLAY: Connectors found: %i", res->count_connectors);
// HERE 16 //    for (size_t i = 0; i < res->count_connectors; i++)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_TRACE, "DISPLAY: Connector index %i", i);
// HERE 16 //        drmModeConnector *con = drmModeGetConnector(CORE.Window.fd, res->connectors[i]);
// HERE 16 //        TRACELOG(LOG_TRACE, "DISPLAY: Connector modes detected: %i", con->count_modes);
// HERE 16 //        if ((con->connection == DRM_MODE_CONNECTED) && (con->encoder_id))
// HERE 16 //        {
// HERE 16 //            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode connected");
// HERE 16 //            CORE.Window.connector = con;
// HERE 16 //            break;
// HERE 16 //        }
// HERE 16 //        else
// HERE 16 //        {
// HERE 16 //            TRACELOG(LOG_TRACE, "DISPLAY: DRM mode NOT connected (deleting)");
// HERE 16 //            drmModeFreeConnector(con);
// HERE 16 //        }
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    if (!CORE.Window.connector)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: No suitable DRM connector found");
// HERE 16 //        drmModeFreeResources(res);
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    drmModeEncoder *enc = drmModeGetEncoder(CORE.Window.fd, CORE.Window.connector->encoder_id);
// HERE 16 //    if (!enc)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode encoder");
// HERE 16 //        drmModeFreeResources(res);
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    CORE.Window.crtc = drmModeGetCrtc(CORE.Window.fd, enc->crtc_id);
// HERE 16 //    if (!CORE.Window.crtc)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get DRM mode crtc");
// HERE 16 //        drmModeFreeEncoder(enc);
// HERE 16 //        drmModeFreeResources(res);
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    // If InitWindow should use the current mode find it in the connector's mode list
// HERE 16 //    if ((CORE.Window.screen.width <= 0) || (CORE.Window.screen.height <= 0))
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_TRACE, "DISPLAY: Selecting DRM connector mode for current used mode...");
// HERE 16 //
// HERE 16 //        CORE.Window.modeIndex = FindMatchingConnectorMode(CORE.Window.connector, &CORE.Window.crtc->mode);
// HERE 16 //
// HERE 16 //        if (CORE.Window.modeIndex < 0)
// HERE 16 //        {
// HERE 16 //            TRACELOG(LOG_WARNING, "DISPLAY: No matching DRM connector mode found");
// HERE 16 //            drmModeFreeEncoder(enc);
// HERE 16 //            drmModeFreeResources(res);
// HERE 16 //            return false;
// HERE 16 //        }
// HERE 16 //
// HERE 16 //        CORE.Window.screen.width = CORE.Window.display.width;
// HERE 16 //        CORE.Window.screen.height = CORE.Window.display.height;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    const bool allowInterlaced = CORE.Window.flags & FLAG_INTERLACED_HINT;
// HERE 16 //    const int fps = (CORE.Time.target > 0) ? (1.0 / CORE.Time.target) : 60;
// HERE 16 //
// HERE 16 //    // Try to find an exact matching mode
// HERE 16 //    CORE.Window.modeIndex = FindExactConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);
// HERE 16 //
// HERE 16 //    // If nothing found, try to find a nearly matching mode
// HERE 16 //    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindNearestConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, allowInterlaced);
// HERE 16 //
// HERE 16 //    // If nothing found, try to find an exactly matching mode including interlaced
// HERE 16 //    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindExactConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);
// HERE 16 //
// HERE 16 //    // If nothing found, try to find a nearly matching mode including interlaced
// HERE 16 //    if (CORE.Window.modeIndex < 0) CORE.Window.modeIndex = FindNearestConnectorMode(CORE.Window.connector, CORE.Window.screen.width, CORE.Window.screen.height, fps, true);
// HERE 16 //
// HERE 16 //    // If nothing found, there is no suitable mode
// HERE 16 //    if (CORE.Window.modeIndex < 0)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable DRM connector mode");
// HERE 16 //        drmModeFreeEncoder(enc);
// HERE 16 //        drmModeFreeResources(res);
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    CORE.Window.display.width = CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay;
// HERE 16 //    CORE.Window.display.height = CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay;
// HERE 16 //
// HERE 16 //    TRACELOG(LOG_INFO, "DISPLAY: Selected DRM connector mode %s (%ux%u%c@%u)", CORE.Window.connector->modes[CORE.Window.modeIndex].name,
// HERE 16 //             CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay, CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay,
// HERE 16 //             (CORE.Window.connector->modes[CORE.Window.modeIndex].flags & DRM_MODE_FLAG_INTERLACE) ? 'i' : 'p',
// HERE 16 //             CORE.Window.connector->modes[CORE.Window.modeIndex].vrefresh);
// HERE 16 //
// HERE 16 //    // Use the width and height of the surface for render
// HERE 16 //    CORE.Window.render.width = CORE.Window.screen.width;
// HERE 16 //    CORE.Window.render.height = CORE.Window.screen.height;
// HERE 16 //
// HERE 16 //    drmModeFreeEncoder(enc);
// HERE 16 //    enc = NULL;
// HERE 16 //
// HERE 16 //    drmModeFreeResources(res);
// HERE 16 //    res = NULL;
// HERE 16 //
// HERE 16 //    CORE.Window.gbmDevice = gbm_create_device(CORE.Window.fd);
// HERE 16 //    if (!CORE.Window.gbmDevice)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM device");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    CORE.Window.gbmSurface = gbm_surface_create(CORE.Window.gbmDevice, CORE.Window.connector->modes[CORE.Window.modeIndex].hdisplay,
// HERE 16 //                                                CORE.Window.connector->modes[CORE.Window.modeIndex].vdisplay, GBM_FORMAT_ARGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
// HERE 16 //    if (!CORE.Window.gbmSurface)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create GBM surface");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //#endif
// HERE 16 //
// HERE 16 //    EGLint samples = 0;
// HERE 16 //    EGLint sampleBuffer = 0;
// HERE 16 //    if (CORE.Window.flags & FLAG_MSAA_4X_HINT)
// HERE 16 //    {
// HERE 16 //        samples = 4;
// HERE 16 //        sampleBuffer = 1;
// HERE 16 //        TRACELOG(LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    const EGLint framebufferAttribs[] =
// HERE 16 //    {
// HERE 16 //        EGL_RENDERABLE_TYPE,
// HERE 16 //        (rlGetVersion() == RL_OPENGL_ES_30) ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT, // Type of context support
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //        EGL_SURFACE_TYPE,
// HERE 16 //        EGL_WINDOW_BIT, // Don't use it on Android!
// HERE 16 //#endif
// HERE 16 //        EGL_RED_SIZE,
// HERE 16 //        8, // RED color bit depth (alternative: 5)
// HERE 16 //        EGL_GREEN_SIZE,
// HERE 16 //        8, // GREEN color bit depth (alternative: 6)
// HERE 16 //        EGL_BLUE_SIZE,
// HERE 16 //        8, // BLUE color bit depth (alternative: 5)
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //        EGL_ALPHA_SIZE,
// HERE 16 //        8, // ALPHA bit depth (required for transparent framebuffer)
// HERE 16 //#endif
// HERE 16 //        // EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
// HERE 16 //        EGL_DEPTH_SIZE,
// HERE 16 //        16, // Depth buffer size (Required to use Depth testing!)
// HERE 16 //        // EGL_STENCIL_SIZE, 8,      // Stencil buffer size
// HERE 16 //        EGL_SAMPLE_BUFFERS,
// HERE 16 //        sampleBuffer, // Activate MSAA
// HERE 16 //        EGL_SAMPLES,
// HERE 16 //        samples, // 4x Antialiasing if activated (Free on MALI GPUs)
// HERE 16 //        EGL_NONE
// HERE 16 //    };
// HERE 16 //
// HERE 16 //    const EGLint contextAttribs[] =
// HERE 16 //        {
// HERE 16 //            EGL_CONTEXT_CLIENT_VERSION, 2,
// HERE 16 //            EGL_NONE};
// HERE 16 //
// HERE 16 //#if defined(PLATFORM_ANDROID) || defined(PLATFORM_DRM)
// HERE 16 //    EGLint numConfigs = 0;
// HERE 16 //
// HERE 16 //    // Get an EGL device connection
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //    CORE.Window.device = eglGetDisplay((EGLNativeDisplayType)CORE.Window.gbmDevice);
// HERE 16 //#else
// HERE 16 //    CORE.Window.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
// HERE 16 //#endif
// HERE 16 //    if (CORE.Window.device == EGL_NO_DISPLAY)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    // Initialize the EGL device connection
// HERE 16 //    if (eglInitialize(CORE.Window.device, NULL, NULL) == EGL_FALSE)
// HERE 16 //    {
// HERE 16 //        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //    if (!eglChooseConfig(CORE.Window.device, NULL, NULL, 0, &numConfigs))
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config count: 0x%x", eglGetError());
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    TRACELOG(LOG_TRACE, "DISPLAY: EGL configs available: %d", numConfigs);
// HERE 16 //
// HERE 16 //    EGLConfig *configs = RL_CALLOC(numConfigs, sizeof(*configs));
// HERE 16 //    if (!configs)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to get memory for EGL configs");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    EGLint matchingNumConfigs = 0;
// HERE 16 //    if (!eglChooseConfig(CORE.Window.device, framebufferAttribs, configs, numConfigs, &matchingNumConfigs))
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to choose EGL config: 0x%x", eglGetError());
// HERE 16 //        free(configs);
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    TRACELOG(LOG_TRACE, "DISPLAY: EGL matching configs available: %d", matchingNumConfigs);
// HERE 16 //
// HERE 16 //    // find the EGL config that matches the previously setup GBM format
// HERE 16 //    int found = 0;
// HERE 16 //    for (EGLint i = 0; i < matchingNumConfigs; ++i)
// HERE 16 //    {
// HERE 16 //        EGLint id = 0;
// HERE 16 //        if (!eglGetConfigAttrib(CORE.Window.device, configs[i], EGL_NATIVE_VISUAL_ID, &id))
// HERE 16 //        {
// HERE 16 //            TRACELOG(LOG_WARNING, "DISPLAY: Failed to get EGL config attribute: 0x%x", eglGetError());
// HERE 16 //            continue;
// HERE 16 //        }
// HERE 16 //
// HERE 16 //        if (GBM_FORMAT_ARGB8888 == id)
// HERE 16 //        {
// HERE 16 //            TRACELOG(LOG_TRACE, "DISPLAY: Using EGL config: %d", i);
// HERE 16 //            CORE.Window.config = configs[i];
// HERE 16 //            found = 1;
// HERE 16 //            break;
// HERE 16 //        }
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    RL_FREE(configs);
// HERE 16 //
// HERE 16 //    if (!found)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to find a suitable EGL config");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //#else
// HERE 16 //    // Get an appropriate EGL framebuffer configuration
// HERE 16 //    eglChooseConfig(CORE.Window.device, framebufferAttribs, &CORE.Window.config, 1, &numConfigs);
// HERE 16 //#endif
// HERE 16 //
// HERE 16 //    // Set rendering API
// HERE 16 //    eglBindAPI(EGL_OPENGL_ES_API);
// HERE 16 //
// HERE 16 //    // Create an EGL rendering context
// HERE 16 //    CORE.Window.context = eglCreateContext(CORE.Window.device, CORE.Window.config, EGL_NO_CONTEXT, contextAttribs);
// HERE 16 //    if (CORE.Window.context == EGL_NO_CONTEXT)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //#endif
// HERE 16 //
// HERE 16 //    // Create an EGL window surface
// HERE 16 //    //---------------------------------------------------------------------------------
// HERE 16 //#if defined(PLATFORM_ANDROID)
// HERE 16 //    EGLint displayFormat = 0;
// HERE 16 //
// HERE 16 //    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
// HERE 16 //    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
// HERE 16 //    eglGetConfigAttrib(CORE.Window.device, CORE.Window.config, EGL_NATIVE_VISUAL_ID, &displayFormat);
// HERE 16 //
// HERE 16 //    // At this point we need to manage render size vs screen size
// HERE 16 //    // NOTE: This function use and modify global module variables:
// HERE 16 //    //  -> CORE.Window.screen.width/CORE.Window.screen.height
// HERE 16 //    //  -> CORE.Window.render.width/CORE.Window.render.height
// HERE 16 //    //  -> CORE.Window.screenScale
// HERE 16 //    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);
// HERE 16 //
// HERE 16 //    ANativeWindow_setBuffersGeometry(CORE.Android.app->window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);
// HERE 16 //    // ANativeWindow_setBuffersGeometry(CORE.Android.app->window, 0, 0, displayFormat);       // Force use of native display size
// HERE 16 //
// HERE 16 //    CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, CORE.Android.app->window, NULL);
// HERE 16 //#endif // PLATFORM_ANDROID
// HERE 16 //
// HERE 16 //#if defined(PLATFORM_DRM)
// HERE 16 //    CORE.Window.surface = eglCreateWindowSurface(CORE.Window.device, CORE.Window.config, (EGLNativeWindowType)CORE.Window.gbmSurface, NULL);
// HERE 16 //    if (EGL_NO_SURFACE == CORE.Window.surface)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL window surface: 0x%04x", eglGetError());
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //
// HERE 16 //    // At this point we need to manage render size vs screen size
// HERE 16 //    // NOTE: This function use and modify global module variables:
// HERE 16 //    //  -> CORE.Window.screen.width/CORE.Window.screen.height
// HERE 16 //    //  -> CORE.Window.render.width/CORE.Window.render.height
// HERE 16 //    //  -> CORE.Window.screenScale
// HERE 16 //    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);
// HERE 16 //#endif // PLATFORM_DRM
// HERE 16 //
// HERE 16 //    // There must be at least one frame displayed before the buffers are swapped
// HERE 16 //    // eglSwapInterval(CORE.Window.device, 1);
// HERE 16 //
// HERE 16 //    if (eglMakeCurrent(CORE.Window.device, CORE.Window.surface, CORE.Window.surface, CORE.Window.context) == EGL_FALSE)
// HERE 16 //    {
// HERE 16 //        TRACELOG(LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
// HERE 16 //        return false;
// HERE 16 //    }
// HERE 16 //    else
// HERE 16 //    {
// HERE 16 //        CORE.Window.render.width = CORE.Window.screen.width;
// HERE 16 //        CORE.Window.render.height = CORE.Window.screen.height;
// HERE 16 //        CORE.Window.currentFbo.width = CORE.Window.render.width;
// HERE 16 //        CORE.Window.currentFbo.height = CORE.Window.render.height;
// HERE 16 //
// HERE 16 //        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
// HERE 16 //        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
// HERE 16 //        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
// HERE 16 //        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
// HERE 16 //        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
// HERE 16 //    }
// HERE 16 //#endif // PLATFORM_ANDROID || PLATFORM_DRM

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
// HERE 17 //#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    rlLoadExtensions(glfwGetProcAddress);
// HERE 17 //#else
// HERE 17 //    rlLoadExtensions(eglGetProcAddress);
// HERE 17 //#endif

    // Initialize OpenGL context (states and resources)
    // NOTE: CORE.Window.currentFbo.width and CORE.Window.currentFbo.height not used, just stored as globals in rlgl
    rlglInit(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

    // Setup default viewport
    // NOTE: It updated CORE.Window.render.width and CORE.Window.render.height
    SetupViewport(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

// HERE 18 //#if defined(PLATFORM_ANDROID)
// HERE 18 //    CORE.Window.ready = true;
// HERE 18 //#endif

    if ((CORE.Window.flags & FLAG_WINDOW_MINIMIZED) > 0) MinimizeWindow();

    return true;
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
    UnloadFontDefault(); // WARNING: Module required: rtext
#endif

    rlglClose(); // De-init rlgl

    glfwDestroyWindow(CORE.Window.handle);
    glfwTerminate();

#if defined(_WIN32) && defined(SUPPORT_WINMM_HIGHRES_TIMER) && !defined(SUPPORT_BUSY_WAIT_LOOP)
    timeEndPeriod(1); // Restore time period
#endif

#if defined(SUPPORT_EVENTS_AUTOMATION)
    RL_FREE(events);
#endif

    CORE.Window.ready = false;
    TRACELOG(LOG_INFO, "Window closed successfully");
}

// Check if KEY_ESCAPE pressed or Close icon pressed
bool WindowShouldClose(void)
{
    // Emterpreter-Async required to run sync code
    // https://github.com/emscripten-core/emscripten/wiki/Emterpreter#emterpreter-async-run-synchronous-code
    // By default, this function is never called on a web-ready raylib example because we encapsulate
    // frame code in a UpdateDrawFrame() function, to allow browser manage execution asynchronously
    // but now emscripten allows sync code to be executed in an interpreted way, using emterpreter!
    emscripten_sleep(16);
    return false;
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
    return ((CORE.Window.flags & FLAG_WINDOW_UNFOCUSED) == 0);
}

// Check if window has been resizedLastFrame
bool IsWindowResized(void)
{
    return CORE.Window.resizedLastFrame;
}

// Toggle fullscreen mode (only PLATFORM_DESKTOP)
void ToggleFullscreen(void)
{
    /*
        EM_ASM
        (
            // This strategy works well while using raylib minimal web shell for emscripten,
            // it re-scales the canvas to fullscreen using monitor resolution, for tools this
            // is a good strategy but maybe games prefer to keep current canvas resolution and
            // display it in fullscreen, adjusting monitor resolution if possible
            if (document.fullscreenElement) document.exitFullscreen();
            else Module.requestFullscreen(true, true); //false, true);
        );
    */
    // EM_ASM(Module.requestFullscreen(false, false););
    /*
        if (!CORE.Window.fullscreen)
        {
            // Option 1: Request fullscreen for the canvas element
            // This option does not seem to work at all:
            // emscripten_request_pointerlock() and emscripten_request_fullscreen() are affected by web security,
            // the user must click once on the canvas to hide the pointer or transition to full screen
            //emscripten_request_fullscreen("#canvas", false);

            // Option 2: Request fullscreen for the canvas element with strategy
            // This option does not seem to work at all
            // Ref: https://github.com/emscripten-core/emscripten/issues/5124
            // EmscriptenFullscreenStrategy strategy = {
                // .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, //EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT,
                // .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                // .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                // .canvasResizedCallback = EmscriptenWindowResizedCallback,
                // .canvasResizedCallbackUserData = NULL
            // };
            //emscripten_request_fullscreen_strategy("#canvas", EM_FALSE, &strategy);

            // Option 3: Request fullscreen for the canvas element with strategy
            // It works as expected but only inside the browser (client area)
            EmscriptenFullscreenStrategy strategy = {
                .scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT,
                .canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF,
                .filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT,
                .canvasResizedCallback = EmscriptenWindowResizedCallback,
                .canvasResizedCallbackUserData = NULL
            };
            emscripten_enter_soft_fullscreen("#canvas", &strategy);

            int width, height;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Enter fullscreen: Canvas size: %i x %i", width, height);

            CORE.Window.fullscreen = true;          // Toggle fullscreen flag
            CORE.Window.flags |= FLAG_FULLSCREEN_MODE;
        }
        else
        {
            //emscripten_exit_fullscreen();
            //emscripten_exit_soft_fullscreen();

            int width, height;
            emscripten_get_canvas_element_size("#canvas", &width, &height);
            TRACELOG(LOG_WARNING, "Emscripten: Exit fullscreen: Canvas size: %i x %i", width, height);

            CORE.Window.fullscreen = false;          // Toggle fullscreen flag
            CORE.Window.flags &= ~FLAG_FULLSCREEN_MODE;
        }
    */

    CORE.Window.fullscreen = !CORE.Window.fullscreen; // Toggle fullscreen flag
}

// Set window state: maximized, if resizable (only PLATFORM_DESKTOP)
void MaximizeWindow(void)
{
    TRACELOG(LOG_INFO, "MaximizeWindow not implemented in rcore_web.c");
}

// Set window state: minimized (only PLATFORM_DESKTOP)
void MinimizeWindow(void)
{
    TRACELOG(LOG_INFO, "MinimizeWindow not implemented in rcore_web.c");
}

// Set window state: not minimized/maximized (only PLATFORM_DESKTOP)
void RestoreWindow(void)
{
    TRACELOG(LOG_INFO, "RestoreWindow not implemented in rcore_web.c");
}

// Toggle borderless windowed mode (only PLATFORM_DESKTOP)
void ToggleBorderlessWindowed(void)
{
    TRACELOG(LOG_INFO, "ToggleBorderlessWindows not implemented in rcore_web.c");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    TRACELOG(LOG_INFO, "SetWindowState not implemented in rcore_web.c");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    TRACELOG(LOG_INFO, "ClearWindowState not implemented in rcore_web.c");
}

// Set icon for window (only PLATFORM_DESKTOP)
// NOTE 1: Image must be in RGBA format, 8bit per channel
// NOTE 2: Image is scaled by the OS for all required sizes
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_INFO, "SetWindowIcon not implemented in rcore_web.c");
}

// Set icon for window (multiple images, only PLATFORM_DESKTOP)
// NOTE 1: Images must be in RGBA format, 8bit per channel
// NOTE 2: The multiple images are used depending on provided sizes
// Standard Windows icon sizes: 256, 128, 96, 64, 48, 32, 24, 16
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_INFO, "SetWindowIcons not implemented in rcore_web.c");
}

// Set title for window (only PLATFORM_DESKTOP and PLATFORM_WEB)
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
    emscripten_set_window_title(title);
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_INFO, "SetWindowPosition not implemented in rcore_web.c");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_INFO, "SetWindowMonitor not implemented in rcore_web.c");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
    // Trigger the resize event once to update the window minimum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
    // Trigger the resize event once to update the window maximum width and height
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) != 0) EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    glfwSetWindowSize(CORE.Window.handle, width, height);
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_INFO, "SetWindowOpacity not implemented in rcore_web.c");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_INFO, "SetWindowFocused not implemented in rcore_web.c");
}

// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    TRACELOG(LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
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
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
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
    // Security check to (partially) avoid malicious code
    if (strchr(text, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided Clipboard could be potentially malicious, avoid [\'] character");
    else EM_ASM( { navigator.clipboard.writeText(UTF8ToString($0)); }, text);
}


// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
/*
    // Accessing clipboard data from browser is tricky due to security reasons
    // The method to use is navigator.clipboard.readText() but this is an asynchronous method
    // that will return at some moment after the function is called with the required data
    emscripten_run_script_string("navigator.clipboard.readText() \
        .then(text => { document.getElementById('clipboard').innerText = text; console.log('Pasted content: ', text); }) \
        .catch(err => { console.error('Failed to read clipboard contents: ', err); });"
    );

    // The main issue is getting that data, one approach could be using ASYNCIFY and wait
    // for the data but it requires adding Asyncify emscripten library on compilation

    // Another approach could be just copy the data in a HTML text field and try to retrieve it
    // later on if available... and clean it for future accesses
*/
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
    emscripten_exit_pointerlock();

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void)
{
    // TODO: figure out how not to hard code the canvas ID here.
    emscripten_request_pointerlock("#canvas", 1);

    // Set cursor position in the middle
    SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

    CORE.Input.Mouse.cursorHidden = true;
}

// Get elapsed time measure in seconds since InitTimer()
// NOTE: On PLATFORM_DESKTOP InitTimer() is called on InitWindow()
// NOTE: On PLATFORM_DESKTOP, timer is initialized on glfwInit()
double GetTime(void)
{
    double time = glfwGetTime();   // Elapsed time since glfwInit()
    return time;
}

// NOTE TRACELOG() function is located in [utils.h]

// Takes a screenshot of current screen (saved a .png)
void TakeScreenshot(const char *fileName)
{
#if defined(SUPPORT_MODULE_RTEXTURES)
    // Security check to (partially) avoid malicious code on PLATFORM_WEB
    if (strchr(fileName, '\'') != NULL) { TRACELOG(LOG_WARNING, "SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character"); return; }

    Vector2 scale = GetWindowScaleDPI();
    unsigned char *imgData = rlReadScreenPixels((int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y));
    Image image = { imgData, (int)((float)CORE.Window.render.width*scale.x), (int)((float)CORE.Window.render.height*scale.y), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    char path[2048] = { 0 };
    strcpy(path, TextFormat("%s/%s", CORE.Storage.basePath, fileName));

    ExportImage(image, path);           // WARNING: Module required: rtextures
    RL_FREE(imgData);

    // Download file from MEMFS (emscripten memory filesystem)
    // saveFileFromMEMFSToDisk() function is defined in raylib/src/shell.html
    emscripten_run_script(TextFormat("saveFileFromMEMFSToDisk('%s','%s')", GetFileName(path), GetFileName(path)));

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
        emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
    }
}

// Get gamepad internal name id
const char *GetGamepadName(int gamepad)
{
    const char *name = NULL;

    name = CORE.Input.Gamepad.name[gamepad];

    return name;
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

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    return 0;
}

// Get mouse position X
int GetMouseX(void)
{
    return (int)((CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x);
}

// Get mouse position Y
int GetMouseY(void)
{
    return (int)((CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y);
}

// Get mouse position XY
Vector2 GetMousePosition(void)
{
    Vector2 position = { 0 };

    // TODO: Review touch position on PLATFORM_WEB

    // NOTE: On PLATFORM_WEB, even on canvas scaling, mouse position is proportionally returned
    position.x = (CORE.Input.Mouse.currentPosition.x + CORE.Input.Mouse.offset.x)*CORE.Input.Mouse.scale.x;
    position.y = (CORE.Input.Mouse.currentPosition.y + CORE.Input.Mouse.offset.y)*CORE.Input.Mouse.scale.y;

    return position;
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // NOTE: emscripten not implemented
    glfwSetCursorPos(CORE.Window.handle, CORE.Input.Mouse.currentPosition.x, CORE.Input.Mouse.currentPosition.y);
}


// Get mouse wheel movement Y
float GetMouseWheelMove(void)
{
    float result = 0.0f;

    if (fabsf(CORE.Input.Mouse.currentWheelMove.x) > fabsf(CORE.Input.Mouse.currentWheelMove.y)) result = (float)CORE.Input.Mouse.currentWheelMove.x;
    else result = (float)CORE.Input.Mouse.currentWheelMove.y;

    return result;
}


// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    glfwSwapBuffers(CORE.Window.handle);
}


// Register all input events
void PollInputEvents(void)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because ProcessGestureEvent() is just called on an event, not every frame
    UpdateGestures();
#endif

    // Reset keys/chars pressed registered
    CORE.Input.Keyboard.keyPressedQueueCount = 0;
    CORE.Input.Keyboard.charPressedQueueCount = 0;
    // Reset key repeats
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++) CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset last gamepad button/axis registered state
    CORE.Input.Gamepad.lastButtonPressed = 0;       // GAMEPAD_BUTTON_UNKNOWN
    CORE.Input.Gamepad.axisCount = 0;
    // Keyboard/Mouse input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < MAX_KEYBOARD_KEYS; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    CORE.Input.Mouse.previousWheelMove = CORE.Input.Mouse.currentWheelMove;
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };

    // Register previous mouse position
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on PLATFORM_WEB the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    CORE.Window.resizedLastFrame = false;

// Gamepad support using emscripten API
// NOTE: GLFW3 joystick functionality not available in web
    // Get number of gamepads connected
    int numGamepads = 0;
    if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS) numGamepads = emscripten_get_num_gamepads();

    for (int i = 0; (i < numGamepads) && (i < MAX_GAMEPADS); i++)
    {
        // Register previous gamepad button states
        for (int k = 0; k < MAX_GAMEPAD_BUTTONS; k++) CORE.Input.Gamepad.previousButtonState[i][k] = CORE.Input.Gamepad.currentButtonState[i][k];

        EmscriptenGamepadEvent gamepadState;

        int result = emscripten_get_gamepad_status(i, &gamepadState);

        if (result == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Register buttons data for every connected gamepad
            for (int j = 0; (j < gamepadState.numButtons) && (j < MAX_GAMEPAD_BUTTONS); j++)
            {
                GamepadButton button = -1;

                // Gamepad Buttons reference: https://www.w3.org/TR/gamepad/#gamepad-interface
                switch (j)
                {
                    case 0: button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case 1: button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case 2: button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;
                    case 3: button = GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case 4: button = GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case 5: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;
                    case 6: button = GAMEPAD_BUTTON_LEFT_TRIGGER_2; break;
                    case 7: button = GAMEPAD_BUTTON_RIGHT_TRIGGER_2; break;
                    case 8: button = GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case 9: button = GAMEPAD_BUTTON_MIDDLE_RIGHT; break;
                    case 10: button = GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case 11: button = GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    case 12: button = GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case 13: button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case 14: button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;
                    case 15: button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (gamepadState.digitalButton[j] == 1)
                    {
                        CORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        CORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else CORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }

                //TRACELOGD("INPUT: Gamepad %d, button %d: Digital: %d, Analog: %g", gamepadState.index, j, gamepadState.digitalButton[j], gamepadState.analogButton[j]);
            }

            // Register axis data for every connected gamepad
            for (int j = 0; (j < gamepadState.numAxes) && (j < MAX_GAMEPAD_AXIS); j++)
            {
                CORE.Input.Gamepad.axisState[i][j] = gamepadState.axis[j];
            }

            CORE.Input.Gamepad.axisCount = gamepadState.numAxes;
        }
    }
}
