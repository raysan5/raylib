#include <stdlib.h>

#include "raylib.h"
#include "rcore.h"

#define GLFW_INCLUDE_ES2            // GLFW3: Enable OpenGL ES 2.0 (translated to WebGL)
//#define GLFW_INCLUDE_ES3            // GLFW3: Enable OpenGL ES 3.0 (transalted to WebGL2?)
#include "GLFW/glfw3.h"             // GLFW3: Windows, OpenGL context and Input management
#include <sys/time.h>               // Required for: timespec, nanosleep(), select() - POSIX

#include <emscripten/emscripten.h>  // Emscripten functionality for C
#include <emscripten/html5.h>       // Emscripten HTML5 library

static EM_BOOL EmscriptenFullscreenChangeCallback(int eventType, const EmscriptenFullscreenChangeEvent *event, void *userData);
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData);
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData);

static EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
static EM_BOOL EmscriptenTouchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
static EM_BOOL EmscriptenGamepadCallback(int eventType, const EmscriptenGamepadEvent *gamepadEvent, void *userData);

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
    // Initialize graphics device (display device and OpenGL context)
    // NOTE: returns true if window and graphic device has been initialized successfully
    CORE.Window.ready = InitGraphicsDevice(width, height);

    // If graphic device is no properly initialized, we end program
    if (!CORE.Window.ready)
    {
        TRACELOG(LOG_FATAL, "Failed to initialize Graphic Device");
        return;
    }
    else SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - CORE.Window.screen.width/2, GetMonitorHeight(GetCurrentMonitor())/2 - CORE.Window.screen.height/2);

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
        SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 2, rec.y + 2, 1, 1 });
    }
    else
    {
        // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
        SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
    }
    #endif
#else
    #if defined(SUPPORT_MODULE_RSHAPES)
    // Set default texture and rectangle to be used for shapes drawing
    // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f });    // WARNING: Module required: rshapes
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
    //emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    // Check Resize event (note this is done on the window since most browsers don't support this on #canvas)
    //emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenResizeCallback);
    // Trigger this once to get initial window sizing
    //EmscriptenResizeCallback(EMSCRIPTEN_EVENT_RESIZE, NULL, NULL);

    // Support keyboard events -> Not used, GLFW.JS takes care of that
    //emscripten_set_keypress_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);
    //emscripten_set_keydown_callback("#canvas", NULL, 1, EmscriptenKeyboardCallback);

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

    return 1;   // The event was consumed by the callback handler
}

// Register window resize event
static EM_BOOL EmscriptenWindowResizedCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // TODO: Implement EmscriptenWindowResizedCallback()?

    return 1;   // The event was consumed by the callback handler
}

EM_JS(int, GetCanvasWidth, (), { return canvas.clientWidth; });
EM_JS(int, GetCanvasHeight, (), { return canvas.clientHeight; });

// Register DOM element resize event
static EM_BOOL EmscriptenResizeCallback(int eventType, const EmscriptenUiEvent *event, void *userData)
{
    // Don't resize non-resizeable windows
    if ((CORE.Window.flags & FLAG_WINDOW_RESIZABLE) == 0) return 1;

    // This event is called whenever the window changes sizes,
    // so the size of the canvas object is explicitly retrieved below
    int width = GetCanvasWidth();
    int height = GetCanvasHeight();
    emscripten_set_canvas_element_size("#canvas",width,height);

    SetupViewport(width, height);    // Reset viewport and projection matrix for new size

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

    return 1;   // The event was consumed by the callback handler
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
        sprintf(CORE.Input.Gamepad.name[gamepadEvent->index],"%s",gamepadEvent->id);
    }
    else CORE.Input.Gamepad.ready[gamepadEvent->index] = false;

    return 1;   // The event was consumed by the callback handler
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
    //EMSCRIPTEN_RESULT res = emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight);
    emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    for (int i = 0; (i < CORE.Input.Touch.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        // Register touch points id
        CORE.Input.Touch.pointId[i] = touchEvent->touches[i].identifier;

        // Register touch points position
        CORE.Input.Touch.position[i] = (Vector2){ touchEvent->touches[i].targetX, touchEvent->touches[i].targetY };

        // Normalize gestureEvent.position[x] for CORE.Window.screen.width and CORE.Window.screen.height
        CORE.Input.Touch.position[i].x *= ((float)GetScreenWidth()/(float)canvasWidth);
        CORE.Input.Touch.position[i].y *= ((float)GetScreenHeight()/(float)canvasHeight);

        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) CORE.Input.Touch.currentTouchState[i] = 1;
        else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) CORE.Input.Touch.currentTouchState[i] = 0;
    }

#if defined(SUPPORT_GESTURES_SYSTEM)        // PLATFORM_WEB
    GestureEvent gestureEvent = { 0 };

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

    return 1;   // The event was consumed by the callback handler
}