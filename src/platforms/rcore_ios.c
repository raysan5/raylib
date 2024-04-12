/**********************************************************************************************
*
*   rcore_ios - Functions to manage window, graphics device and touch inputs
*
*   PLATFORM: IOS
*       - iOS (arm64)
*
*   LIMITATIONS:
*       - No keyboard input support
*       - No gamepad input support
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - ANGLE for iOS
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5) and contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/
#include "libEGL/libEGL.h"

// iOS only supports callbacks
// We are not able to give users full control of the game loop
extern void ios_ready();
extern void ios_update();
extern void ios_destroy();

#import <UIKit/UIKit.h>

/* GameViewController */
@interface GameViewController : UIViewController
- (void)update;
@end

/* AppDelegate */
@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    GameViewController *viewController;  // Root view controller
    
    // Display data
    EGLDisplay device;                  // Native display device (physical screen connection)
    EGLSurface surface;                 // Surface to draw on, framebuffers (connected to context)
    EGLContext context;                 // Graphic context, mode in which drawing can be done
    EGLConfig config;                   // Graphic config
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static int MapPointId(UITouch *touch){
    static UITouch* touchs[MAX_TOUCH_POINTS] = { 0 };
    for(int i = 0; i < MAX_TOUCH_POINTS; i++){
        if(touchs[i] == touch) return i + 1;
    }
    // clear unused touch pairs before insert
    for(int i = 0; i < MAX_TOUCH_POINTS; i++){
        if(touchs[i] == NULL) continue;
        bool found = false;
        for(int j = 0; j < MAX_TOUCH_POINTS; j++){
            if(CORE.Input.Touch.pointId[j] == i + 1){
                found = true;
                break;
            }
        }
        if(!found) touchs[i] = NULL;
    }
    for(int i = 0; i < MAX_TOUCH_POINTS; i++){
        if(touchs[i] == NULL){
            touchs[i] = touch;
            return i + 1;
        }
    }
    TRACELOG(LOG_ERROR, "Touch point id overflow. This may be a bug!");
    return 0;
}

int InitPlatform(void);          // Initialize platform (graphics, inputs and more)

// Check if application should close
bool WindowShouldClose(void)
{
    if (CORE.Window.ready) return CORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void ToggleFullscreen(void)
{
    TRACELOG(LOG_WARNING, "ToggleFullscreen() not available on target platform");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void)
{
    TRACELOG(LOG_WARNING, "ToggleBorderlessWindowed() not available on target platform");
}

// Set window state: maximized, if resizable
void MaximizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

// Set window state: minimized
void MinimizeWindow(void)
{
    TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Set window state: not minimized/maximized
void RestoreWindow(void)
{
    TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags)
{
    TRACELOG(LOG_WARNING, "SetWindowState() not available on target platform");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags)
{
    TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

// Set icon for window
void SetWindowIcon(Image image)
{
    TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icon for window
void SetWindowIcons(Image *images, int count)
{
    TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title)
{
    CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y)
{
    TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor)
{
    TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height)
{
    CORE.Window.screenMin.width = width;
    CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height)
{
    CORE.Window.screenMax.width = width;
    CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height)
{
    TRACELOG(LOG_WARNING, "SetWindowSize() not available on target platform");
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity)
{
    TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void)
{
    TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

// Get native window handle
void *GetWindowHandle(void)
{
    TRACELOG(LOG_WARNING, "GetWindowHandle() not implemented on target platform");
    return NULL;
}

// Get number of monitors
int GetMonitorCount(void)
{
    return 1;
}

// Get current monitor id
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
    return CORE.Window.screen.width;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    return CORE.Window.screen.height;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight() not implemented on target platform");
    return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorRefreshRate() not implemented on target platform");
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorName() not implemented on target platform");
    return "";
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void)
{
    TRACELOG(LOG_WARNING, "GetWindowPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void)
{
    CGFloat scale = [[UIScreen mainScreen] scale];
    return (Vector2){ scale, scale };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    pasteboard.string = [NSString stringWithUTF8String:text];
}

// Get clipboard text content
const char *GetClipboardText(void)
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    NSString *clipboard = pasteboard.string;
    return clipboard.UTF8String;
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

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    eglSwapBuffers(platform.device, platform.surface);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{
    double time = 0.0;
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()

    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    NSURL *nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
    [[UIApplication sharedApplication] openURL:nsurl options:@{} completionHandler:nil];
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings)
{
    TRACELOG(LOG_WARNING, "SetGamepadMappings() not implemented on target platform");
    return 0;
}

// Set gamepad vibration
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor)
{
    TRACELOG(LOG_WARNING, "GamepadSetVibration() not implemented on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
    CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor)
{
    TRACELOG(LOG_WARNING, "SetMouseCursor() not implemented on target platform");
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
    CORE.Input.Gamepad.lastButtonPressed = 0; // GAMEPAD_BUTTON_UNKNOWN
    //CORE.Input.Gamepad.axisCount = 0;

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++)
    {
        CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Poll input events for iOS
    // https://developer.apple.com/documentation/uikit/touches_presses_and_gestures
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

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
        EGL_RED_SIZE, 8,            // RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // BLUE color bit depth (alternative: 5)
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };
    
    const EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    EGLint numConfigs = 0;

    // Get an EGL device connection
    platform.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (platform.device == EGL_NO_DISPLAY)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

    // Initialize the EGL device connection
    if (eglInitialize(platform.device, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(platform.device, framebufferAttribs, &platform.config, 1, &numConfigs);

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    platform.context = eglCreateContext(platform.device, platform.config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.context == EGL_NO_CONTEXT)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to create EGL context");
        return false;
    }

    // Create an EGL window surface
    EGLint displayFormat = 0;

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // eglCreateWindowSurface(platform.device, platform.config, platform.app->window, NULL);
    // bridged cast rootViewController.view.layer; to void*
    void* native_window = (__bridge void*)platform.viewController.view.layer;
    platform.surface = eglCreateWindowSurface(platform.device, platform.config, native_window, NULL);

    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(platform.device, 1);

    if (eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context) == EGL_FALSE)
    {
        TRACELOG(LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
        return false;
    }
    else
    {
        CORE.Window.display.width = [[UIScreen mainScreen] nativeBounds].size.width;
        CORE.Window.display.height = [[UIScreen mainScreen] nativeBounds].size.height;
        if(CORE.Window.screen.width == 0){
            CORE.Window.screen.width = [[UIScreen mainScreen] bounds].size.width;
        }
        if(CORE.Window.screen.height == 0){
            CORE.Window.screen.height = [[UIScreen mainScreen] bounds].size.height;
        }
        
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;

        TRACELOG(LOG_INFO, "DISPLAY: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", GetRenderWidth(), GetRenderHeight());
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
        TRACELOG(LOG_INFO, "    > EGL: %s", eglQueryString(platform.device, EGL_VERSION));
    }
    //----------------------------------------------------------------------------
    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlLoadExtensions(eglGetProcAddress);
    CORE.Window.ready = true;
    //----------------------------------------------------------------------------
    // Initialize OpenGL context (states and resources)
    // NOTE: CORE.Window.currentFbo.width and CORE.Window.currentFbo.height not used, just stored as globals in rlgl
    rlglInit(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);

    // Setup default viewport
    // NOTE: It updated CORE.Window.render.width and CORE.Window.render.height
    SetupViewport(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height);
    InitTimer();
    CORE.Storage.basePath = GetWorkingDirectory();
    TRACELOG(LOG_INFO, "PLATFORM: IOS: Initialized successfully");
    return true;
}

// Close platform
void ClosePlatform(void)
{
    // TODO: De-initialize graphics, inputs and more

    // Close surface, context and display
    if (platform.device != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(platform.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (platform.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(platform.device, platform.surface);
            platform.surface = EGL_NO_SURFACE;
        }

        if (platform.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(platform.device, platform.context);
            platform.context = EGL_NO_CONTEXT;
        }

        eglTerminate(platform.device);
        platform.device = EGL_NO_DISPLAY;
    }
}


@implementation GameViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    // self.modalPresentationCapturesStatusBarAppearance = true;
    platform.viewController = self;
    self.view.multipleTouchEnabled = true;
    self.view.contentScaleFactor = [[UIScreen mainScreen] scale];
}

- (bool)prefersStatusBarHidden
{
    return true;
}

- (void)update
{
    ios_update();
}

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    // In iOS 17, Messages allows you to interactively resize iMessage apps with a vertical pan gesture. Messages handles any conflicts between resize gestures and your custom gestures. If your app uses manual touch handling, override those methods in your app’s UIView. You can either change your manual touch handling code to use a gesture recognizer instead, or your UIView can override gestureRecognizerShouldBegin: and return NO when your iMessage app doesn’t own the gesture.
    return false;
}

static void SyncAllTouches(UIEvent* event)
{
    CORE.Input.Touch.pointCount = (int)event.allTouches.count;
    int i = 0;
    for (UITouch *touch in event.allTouches)
    {
        CGPoint location = [touch locationInView:platform.viewController.view];
        CORE.Input.Touch.position[i] = (Vector2){ location.x, location.y };
        CORE.Input.Touch.pointId[i] = MapPointId(touch);
        i++;
        if(i >= MAX_TOUCH_POINTS) break;
    }
    // TODO: Normalize CORE.Input.Touch.position[i] for CORE.Window.screen.width and CORE.Window.screen.height
}

static int IndexOf(int needle, int *haystack, int size)
{
    for (int i = 0; i < size; i++) if(haystack[i] == needle) return i;
    return -1;
}

static void SendGestureEvent(NSSet<UITouch *> * touches, int action)
{
#if defined(SUPPORT_GESTURES_SYSTEM)
    GestureEvent gestureEvent = { 0 };

    gestureEvent.pointCount = CORE.Input.Touch.pointCount;

    // Register touch actions
    gestureEvent.touchAction = action;

    for (int i = 0; (i < gestureEvent.pointCount) && (i < MAX_TOUCH_POINTS); i++)
    {
        gestureEvent.pointId[i] = CORE.Input.Touch.pointId[i];
        gestureEvent.position[i] = CORE.Input.Touch.position[i];
        gestureEvent.position[i].x /= (float)GetScreenWidth();
        gestureEvent.position[i].y /= (float)GetScreenHeight();
    }

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);
#endif

    if(action == TOUCH_ACTION_UP){
        // One of the touchpoints is released, remove it from touch point arrays
        for (UITouch *touch in touches)
        {
            int size = CORE.Input.Touch.pointCount;
            if(size > MAX_TOUCH_POINTS) size = MAX_TOUCH_POINTS;
            int i = IndexOf(MapPointId(touch), CORE.Input.Touch.pointId, size);
            if(i >= 0){
                // remove i-th touch point
                for (int j = i; j < size - 1; j++)
                {
                    CORE.Input.Touch.pointId[j] = CORE.Input.Touch.pointId[j + 1];
                    CORE.Input.Touch.position[j] = CORE.Input.Touch.position[j + 1];
                }
                CORE.Input.Touch.pointCount--;
            }else{
                TRACELOG(LOG_WARNING, "Touch point not found. This may be a bug!");
            }
        }
    }

    if (CORE.Input.Touch.pointCount > 0) CORE.Input.Touch.currentTouchState[MOUSE_BUTTON_LEFT] = 1;
    else CORE.Input.Touch.currentTouchState[MOUSE_BUTTON_LEFT] = 0;

    if(action == TOUCH_ACTION_MOVE){
        CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
    }else{
        CORE.Input.Mouse.previousPosition = CORE.Input.Touch.position[0];
    }

    // Map touch[0] as mouse input for convenience
    CORE.Input.Mouse.currentPosition = CORE.Input.Touch.position[0];
    CORE.Input.Mouse.currentWheelMove = (Vector2){ 0.0f, 0.0f };
}

// touch callbacks
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    SyncAllTouches(event);
    SendGestureEvent(touches, TOUCH_ACTION_DOWN);
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    SyncAllTouches(event);
    SendGestureEvent(touches, TOUCH_ACTION_UP);
    // post sync needed
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    SyncAllTouches(event);
    SendGestureEvent(touches, TOUCH_ACTION_MOVE);
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    SyncAllTouches(event);
    SendGestureEvent(touches, TOUCH_ACTION_CANCEL);
}

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.backgroundColor = [UIColor redColor];
    self.window.rootViewController = [[GameViewController alloc] init];
    [self.window makeKeyAndVisible];
    ios_ready();
    CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self.window.rootViewController selector:@selector(update)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    CORE.Window.flags &= ~FLAG_WINDOW_UNFOCUSED;
}
- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    CORE.Window.flags |= FLAG_WINDOW_UNFOCUSED;
}
- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    ios_destroy();

    if (platform.device != EGL_NO_DISPLAY)
    {
        // the user does not call CloseWindow() before exiting
        TRACELOG(LOG_ERROR, "DISPLAY: CloseWindow() should be called before terminating the application");
    }
    // If 'platform.device' is already set to 'EGL_NO_DISPLAY'
    // this means that the user has already called 'CloseWindow()'
}

@end

/* main() */
int main(int argc, char * argv[]) {
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
}

// EOF
