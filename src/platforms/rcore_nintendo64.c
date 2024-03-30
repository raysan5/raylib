/**********************************************************************************************
*
*   rcore_dreamcast - Functions to manage window, graphics device and inputs
*
*   PLATFORM: NINTENDO64
*       - Nintendo 64
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - Port done by Antonio Jose Ramos Marquez aka bigboss @psxdev		
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       Nintendo64 libdragon and toolchains - Provides C API to access Nintendo 64 homebrew functionality
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5) and contributors
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
#include <libdragon.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct rayFontN64
{
    Texture2D fontGLTextures[224];
    surface_t fontSurfaces[224];

}RayFontN64;
#define RAYLIB4N64_MAX_SURFACE_LIMIT 10 //You can change this limit 
typedef struct {
    // TODO: Define the platform specific variables required
    surface_t zbuffer;
    surface_t *disp;
    RayFontN64 rayFonts;
    surface_t surfaces[RAYLIB4N64_MAX_SURFACE_LIMIT];
    unsigned int surface_index;
    // Display data
    //EGLDisplay device;                  // Native display device (physical screen connection)
    //EGLSurface surface;                 // Surface to draw on, framebuffers (connected to context)
    //EGLContext context;                 // Graphic context, mode in which drawing can be done
    //EGLConfig config;                   // Graphic config
} PlatformData;

#define SUPPORT_DEFAULT_FONT 1
//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);   // Initialize graphics device
Texture2D rayDefaultFontsGetTextureFromGlyph(int index);
void rayDefaultFontsInitSurfaceBuffers();
void rayDefaultFontGliphGlTextureInit(Font font,int index);
Texture2D getFontGLTextureId(char *text);

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

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
    TRACELOG(LOG_WARNING, "GetMonitorCount() not implemented on target platform");
    return 1;
}

// Get number of monitors
int GetCurrentMonitor(void)
{
    TRACELOG(LOG_WARNING, "GetCurrentMonitor() not implemented on target platform");
    return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorPosition() not implemented on target platform");
    return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor)
{
    TRACELOG(LOG_WARNING, "GetMonitorHeight() not implemented on target platform");
    return 0;
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
    TRACELOG(LOG_WARNING, "GetWindowScaleDPI() not implemented on target platform");
    return (Vector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void SetClipboardText(const char *text)
{
    TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void)
{
    TRACELOG(LOG_WARNING, "GetClipboardText() not implemented on target platform");
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

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void)
{
    gl_context_end();
    rdpq_detach_show();
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void)
{   
    disable_interrupts();
    double time=(double)(get_ticks_us() - CORE.Time.base)*1e-6;
    enable_interrupts();
    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
   // Security check to (partially) avoid malicious code on target platform
    if (strchr(url, '\'') != NULL) TRACELOG(LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        // TODO:
    }
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
    joypad_style_t style;
    joypad_accessory_type_t accessory_type;
    bool rumble_supported;
    bool rumble_active;
    joypad_inputs_t inputs;

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


        // TODO: Add callbacks for connect / disconnected pads

        // Reset touch positions
        // TODO: It resets on target platform the mouse position and not filled again until a move-event,
        // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
        //for (int i = 0; i < MAX_TOUCH_POINTS; i++) CORE.Input.Touch.position[i] = (Vector2){ 0, 0 };

        // Register previous keys states
        // NOTE: Android supports up to 260 keys
        //for (int i = 0; i < 260; i++)
        //{
        //    CORE.Input.Keyboard.previousKeyState[i] = CORE.Input.Keyboard.currentKeyState[i];
        //    CORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
        //}

        // TODO: Poll input events for current plaform
/*TODO N64
    joypad_poll();

    JOYPAD_PORT_FOREACH (port)
    {
            style = joypad_get_style(port);
            accessory_type = joypad_get_accessory_type(port);
            rumble_supported = joypad_get_rumble_supported(port);
            rumble_active = joypad_get_rumble_active(port);
            inputs = joypad_get_inputs(port);

            if (rumble_supported)
            {
                if (inputs.btn.a && !rumble_active)
                {
                    joypad_set_rumble_active(port, true);
                }
                else if (!inputs.btn.a && rumble_active)
                {
                    joypad_set_rumble_active(port, false);
                }
            }
    }  
*/  
    
}

void CustomLog(int msgType, const char *text, va_list args)
{
    char buffer[1024] = { 0 };

   
    vsnprintf(buffer,1024, text, args);
    buffer[1024-1] = 0;
    
    
    switch (msgType)
    {
        case LOG_TRACE: debugf("[N64][TRACE]: %s\n",buffer); break;
        case LOG_DEBUG: debugf("[N64][DEBUG]: %s\n",buffer); break;
        case LOG_INFO: debugf("[N64][INFO]: %s\n",buffer); break;
        case LOG_WARNING: debugf("[N64][WARNING]: %s\n",buffer); break;
        case LOG_ERROR: debugf("[N64][ERROR]: %s\n",buffer); break;
        case LOG_FATAL: debugf("[N64][FATAL]: %s\n",buffer); break;
        default: break;
    }
}
//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
//Set all surface buffer for each glyph to NULL
void rayDefaultFontsInitSurfaceBuffers()
{
    for(int i=0;i<224;i++)
    {
        platform.rayFonts.fontSurfaces[i].buffer=NULL;
    }
}

void rlLoadTextureN64(const void *data, int width, int height, int format, int mipmapCount)
{
    
    if(platform.surface_index>=RAYLIB4N64_MAX_SURFACE_LIMIT)
    {
        TraceLog(LOG_ERROR,"Limit for surfaces in raylib4Nintendo64 is %d",RAYLIB4N64_MAX_SURFACE_LIMIT);
        return 0;
    }
    //todo load different mipmap level using mipmapCount by now we wil use only level 0
    //todo using different GL format to transform to supported internal format by now we will use only FMT_RGBA32 

    platform.surfaces[platform.surface_index].flags = FMT_RGBA32 | SURFACE_FLAGS_OWNEDBUFFER;

    platform.surfaces[platform.surface_index].width = (uint32_t)width;

    platform.surfaces[platform.surface_index].height = (uint32_t)height;

    platform.surfaces[platform.surface_index].stride = TEX_FORMAT_PIX2BYTES(FMT_RGBA32, width);

    platform.surfaces[platform.surface_index].buffer = data;

    glSurfaceTexImageN64(GL_TEXTURE_2D, 0, &(platform.surfaces[platform.surface_index]), &(rdpq_texparms_t){.s.repeats = 0, .t.repeats = 0});


    platform.surface_index++;

}
void rayDefaultFontGliphGlTextureInit(Font font,int index)
{
    //If gliph was not initialized before generate texture from glyph image
    if(platform.rayFonts.fontSurfaces[index].buffer==NULL)
    {

        platform.rayFonts.fontSurfaces[index].flags = FMT_RGBA32 | SURFACE_FLAGS_OWNEDBUFFER;

        platform.rayFonts.fontSurfaces[index].width = (uint32_t) (font.recs[index].width);

        platform.rayFonts.fontSurfaces[index].height = (uint32_t)(font.recs[index].height);

        platform.rayFonts.fontSurfaces[index].stride = TEX_FORMAT_PIX2BYTES(FMT_RGBA32, platform.rayFonts.fontSurfaces[index].width);

        platform.rayFonts.fontSurfaces[index].buffer =  (font.glyphs[index].image.data);

        glGenTextures(1,&(platform.rayFonts.fontGLTextures[index].id));

        glBindTexture(GL_TEXTURE_2D, platform.rayFonts.fontGLTextures[index].id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR*/

        glSurfaceTexImageN64(GL_TEXTURE_2D, 0, &(platform.rayFonts.fontSurfaces[index]), &(rdpq_texparms_t){.s.repeats = 0, .t.repeats = 0});


        platform.rayFonts.fontGLTextures[index].width=font.recs[index].width;

        platform.rayFonts.fontGLTextures[index].height= font.recs[index].height;

        platform.rayFonts.fontGLTextures[index].mipmaps=1;

        platform.rayFonts.fontGLTextures[index].format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    }
   
}
Texture2D getFontGLTextureId(char *text)
{
    int codepointByteCount;
    int codepoint = GetCodepointNext(text, &codepointByteCount);
    int index = GetGlyphIndex(GetFontDefault(), codepoint);
    if(platform.rayFonts.fontGLTextures[index].id==0)
    {
        TraceLog(LOG_ERROR, "%s texture not initalized for %s", __FUNCTION__,text);
        rayDefaultFontGliphGlTextureInit(GetFontDefault(),index);
    }
    else
    {
        //TraceLog(LOG_INFO, "%s textureinitalized for %s ", __FUNCTION__,text);
    }
    return platform.rayFonts.fontGLTextures[index];
}
Texture2D rayDefaultFontsGetTextureFromGlyph(int index)
{
    Texture2D texture = { 0 };
    texture.id = platform.rayFonts.fontGLTextures[index].id;
    texture.width = platform.rayFonts.fontGLTextures[index].width;
    texture.height = platform.rayFonts.fontGLTextures[index].height;
    texture.mipmaps = platform.rayFonts.fontGLTextures[index].mipmaps;
    texture.format = platform.rayFonts.fontGLTextures[index].format;
    return texture;
}
// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    SetTraceLogCallback(CustomLog);

   
    
    debug_init_isviewer();
    debug_init_usblog();
    TRACELOG(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);
    TRACELOG(LOG_INFO, "Platform backend: NINTENDO64");
    TRACELOG(LOG_INFO, "PLATFORM: Nintendo 64 init");

    dfs_init(DFS_DEFAULT_LOCATION);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);

    rdpq_init();
    gl_init();

#if DEBUG_RDP
    rdpq_debug_start();
    rdpq_debug_log(true);
#endif

    joypad_init();

    rayDefaultFontsInitSurfaceBuffers();
    platform.zbuffer = surface_alloc(FMT_RGBA16, display_get_width(), display_get_height());


    platform.disp = display_get();

    rdpq_attach(platform.disp, &platform.zbuffer);

    gl_context_begin();


    CORE.Window.fullscreen = true;
    CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

    CORE.Window.display.width = 320;//CORE.Window.screen.width;            // User desired width
    CORE.Window.display.height = 240;//CORE.Window.screen.height;          // User desired height
    CORE.Window.screen.width=314;
    CORE.Window.screen.height=240;
    CORE.Window.render.width = CORE.Window.screen.width;
    CORE.Window.render.height = CORE.Window.screen.height;
    CORE.Window.eventWaiting = false;
    CORE.Window.screenScale = MatrixIdentity();     // No draw scaling required by default
    CORE.Window.currentFbo.width = CORE.Window.screen.width;
    CORE.Window.currentFbo.height = CORE.Window.screen.height;
    CORE.Input.Mouse.currentPosition.x = (float)CORE.Window.screen.width/2.0f;
    CORE.Input.Mouse.currentPosition.y = (float)CORE.Window.screen.height/2.0f;
    CORE.Input.Mouse.scale = (Vector2){ 1.0f, 1.0f };


    

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables:
    //  -> CORE.Window.screen.width/CORE.Window.screen.height
    //  -> CORE.Window.render.width/CORE.Window.render.height
    //  -> CORE.Window.screenScale
    SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);

    //ANativeWindow_setBuffersGeometry(platform.app->window, CORE.Window.render.width, CORE.Window.render.height, displayFormat);
    //ANativeWindow_setBuffersGeometry(platform.app->window, 0, 0, displayFormat);       // Force use of native display size

    
    {
        CORE.Window.render.width = CORE.Window.screen.width;
        CORE.Window.render.height = CORE.Window.screen.height;
        CORE.Window.currentFbo.width = CORE.Window.render.width;
        CORE.Window.currentFbo.height = CORE.Window.render.height;

        TRACELOG(LOG_INFO, "PLATFORM: Device initialized successfully");
        TRACELOG(LOG_INFO, "    > Display size: %i x %i", CORE.Window.display.width, CORE.Window.display.height);
        TRACELOG(LOG_INFO, "    > Screen size:  %i x %i", CORE.Window.screen.width, CORE.Window.screen.height);
        TRACELOG(LOG_INFO, "    > Render size:  %i x %i", CORE.Window.render.width, CORE.Window.render.height);
        TRACELOG(LOG_INFO, "    > Viewport offsets: %i, %i", CORE.Window.renderOffset.x, CORE.Window.renderOffset.y);
    }

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    //rlLoadExtensions(eglGetProcAddress);
    //const char *gl_exts = (char *) glGetString(GL_EXTENSIONS);
    //TRACELOG(LOG_INFO,"PLATFORM: GL_VENDOR:   \"%s\"", glGetString(GL_VENDOR));
    //TRACELOG(LOG_INFO,"PLATFORM: GL_VERSION:  \"%s\"", glGetString(GL_VERSION));
    //TRACELOG(LOG_INFO,"PLATFORM: GL_RENDERER: \"%s\"", glGetString(GL_RENDERER));
    //TRACELOG(LOG_INFO,"PLATFORM: SL_VERSION:  \"%s\"", glGetString(GL_SHADING_LANGUAGE_VERSION));
    CORE.Window.ready = true;

    // Initialize hi-res timer
    InitTimer();

    // Initialize base path for storage
    CORE.Storage.basePath = GetWorkingDirectory();
    TRACELOG(LOG_INFO, "PLATFORM: Initialized");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    // TODO: De-initialize graphics, inputs and more
}

// EOF
