/**********************************************************************************************
*
*   rcore_<platform> template - Functions to manage window, graphics device and inputs
*
*   PLATFORM: <PLATFORM>
*       - TODO: Define the target platform for the core
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
*       - TRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - <platform-specific SDK dependency>
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

#include <linux/input.h>

#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h> // must be included before the EGL headers

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include <GLES2/gl2.h>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef enum {
    FINGER_STATE_REMOVED = 0, // state when finger was removed and we handled its removal + default state
    FINGER_STATE_REMOVING, // state when finger is currently being removed from panel (released event)
    FINGER_STATE_TOUCHING, // state when finger is touching panel at any time
} FingerState;

struct finger {
  FingerState state;
  int x;
  int y;
  bool resetNextFrame;
};

struct touch {
  struct finger fingers[MAX_TOUCH_POINTS];
  int fd;
  int canonical;
};

// hold all the low level wayland stuff
struct wayland_platform {
  struct wl_compositor *wl_compositor;
  struct wl_surface *wl_surface;
  struct wl_egl_window *wl_egl_window;
  struct wl_region *wl_region;
  struct wl_shell *wl_shell;
  struct wl_shell_surface *wl_shell_surface;
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
};

// hold all the low level egl stuff
struct egl_platform {
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig config;

  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;

  int native_window_width;
  int native_window_height;
};

typedef struct {
    struct wayland_platform wayland;
    struct egl_platform egl;
    struct touch touch;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern CoreData CORE;                   // Global CORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// comma specific code
//----------------------------------------------------------------------------------

#define CASE_STR( value ) case value: return #value;
const char *eglGetErrorString(EGLint error) {
    switch(error) {
      CASE_STR( EGL_SUCCESS             )
      CASE_STR( EGL_NOT_INITIALIZED     )
      CASE_STR( EGL_BAD_ACCESS          )
      CASE_STR( EGL_BAD_ALLOC           )
      CASE_STR( EGL_BAD_ATTRIBUTE       )
      CASE_STR( EGL_BAD_CONTEXT         )
      CASE_STR( EGL_BAD_CONFIG          )
      CASE_STR( EGL_BAD_CURRENT_SURFACE )
      CASE_STR( EGL_BAD_DISPLAY         )
      CASE_STR( EGL_BAD_SURFACE         )
      CASE_STR( EGL_BAD_MATCH           )
      CASE_STR( EGL_BAD_PARAMETER       )
      CASE_STR( EGL_BAD_NATIVE_PIXMAP   )
      CASE_STR( EGL_BAD_NATIVE_WINDOW   )
      CASE_STR( EGL_CONTEXT_LOST        )
      default: return "Unknown";
    }
}
#undef CASE_STR

void wl_shell_surface_handle_ping (void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
  wl_shell_surface_pong(shell_surface, serial);
}

void wl_shell_surface_handle_configure (void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height) {
  wl_egl_window_resize(platform.egl.native_window, width, height, 0, 0);
}

void wl_shell_surface_handle_popup_done(void *data, struct wl_shell_surface *shell_surface) {
}

static struct wl_shell_surface_listener wl_shell_surface_listener = {
  .ping = &wl_shell_surface_handle_ping,
  .configure = &wl_shell_surface_handle_configure,
  .popup_done = &wl_shell_surface_handle_popup_done
};

static void wl_registry_handle_global (void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
  if (!strcmp(interface, "wl_compositor")) {
    // Need version 3 of wl_compositor in order to do the rotation transform with wl_surface_set_buffer_transform
    platform.wayland.wl_compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 3);
  } else if (!strcmp(interface, "wl_shell")) {
    platform.wayland.wl_shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
  }
}

static void wl_registry_handle_global_remove (void *data, struct wl_registry *registry, uint32_t id) {
}

const struct wl_registry_listener wl_registry_listener = {
  .global = wl_registry_handle_global,
  .global_remove = wl_registry_handle_global_remove
};

static int init_wayland(int width, int height) {
  platform.wayland.wl_display = wl_display_connect(NULL);
  if (platform.wayland.wl_display == NULL) {
    TRACELOG(LOG_WARNING, "COMMA: Failed to create a Wayland display. Failed with: %s", strerror(errno));
    return -1;
  }

  platform.wayland.wl_compositor = NULL;
  platform.wayland.wl_shell = NULL;

  platform.wayland.wl_registry = wl_display_get_registry(platform.wayland.wl_display);
  wl_registry_add_listener(platform.wayland.wl_registry, &wl_registry_listener, NULL);

  wl_display_dispatch(platform.wayland.wl_display);
  wl_display_roundtrip(platform.wayland.wl_display);

  if (platform.wayland.wl_compositor == NULL || platform.wayland.wl_compositor == NULL) {
    TRACELOG(LOG_WARNING, "COMMA: Failed to bind Wayland globals");
    return -1;
  }

  // create a surface with a buffer to do render on it
  platform.wayland.wl_surface = wl_compositor_create_surface(platform.wayland.wl_compositor);

  // apply rotation transform to the buffer of the surface
  wl_surface_set_buffer_transform(platform.wayland.wl_surface, WL_OUTPUT_TRANSFORM_270);

  platform.wayland.wl_shell_surface = wl_shell_get_shell_surface(platform.wayland.wl_shell, platform.wayland.wl_surface);
  wl_shell_surface_add_listener(platform.wayland.wl_shell_surface, &wl_shell_surface_listener, NULL);
  wl_shell_surface_set_toplevel(platform.wayland.wl_shell_surface);

  platform.wayland.wl_region = wl_compositor_create_region(platform.wayland.wl_compositor);
  wl_region_add(platform.wayland.wl_region, 0, 0, width, height);
  wl_surface_set_opaque_region(platform.wayland.wl_surface, platform.wayland.wl_region);

  // the native window for egl is a our wl_surface
  platform.egl.native_window = wl_egl_window_create(platform.wayland.wl_surface, width, height);
  if (platform.egl.native_window == NULL) {
    TRACELOG(LOG_WARNING, "COMMA: Failed to create a Wayland EGL window");
    return -1;
  }
  // the native display for egl is a our wl_display
  platform.egl.native_display = platform.wayland.wl_display;
  platform.egl.native_window_width = width;
  platform.egl.native_window_height = height;

  return 0;
}

static int init_egl () {
   EGLint major;
   EGLint minor;
   EGLConfig config;
   EGLint num_config;
   EGLint frame_buffer_config [] = {
     EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
     EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
     EGL_RED_SIZE,   8,
     EGL_GREEN_SIZE, 8,
     EGL_BLUE_SIZE,  8,
     EGL_DEPTH_SIZE, 24,
     EGL_NONE
   };
   // ask for an OpenGL ES 2 rendering context
   EGLint context_config [] = { EGL_CONTEXT_MAJOR_VERSION, 2, EGL_NONE, EGL_NONE };

   // get an egl display with our native display (in our case, a wl_display)
   platform.egl.display = eglGetDisplay(platform.egl.native_display);
   if (platform.egl.display == EGL_NO_DISPLAY) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to get an EGL display");
     return -1;
   }

   if (!eglInitialize(platform.egl.display, &major, &minor)) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to initialize the EGL display. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }
   TRACELOG(LOG_INFO, "COMMA: Using EGL version %i.%i", major, minor);

   if (!eglChooseConfig(platform.egl.display, frame_buffer_config, &config, 1, &num_config)) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to get a valid EGL display config. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }
   TRACELOG(LOG_INFO, "COMMA: Found %i valid EGL display configs", num_config);

   platform.egl.surface = eglCreateWindowSurface(platform.egl.display, config, platform.egl.native_window, NULL);
   if (platform.egl.surface == EGL_NO_SURFACE) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to create an EGL surface. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }

   platform.egl.context = eglCreateContext(platform.egl.display, config, EGL_NO_CONTEXT, context_config);
   if (platform.egl.context == EGL_NO_CONTEXT) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to create an OpenGL ES context. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }

   if (!eglMakeCurrent(platform.egl.display, platform.egl.surface, platform.egl.surface, platform.egl.context)) {
     TRACELOG(LOG_WARNING, "COMMA: Failed to attach the OpenGL ES context to the EGL surface. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }

   // > 1 is not supported
   EGLBoolean ok = eglSwapInterval(platform.egl.display, (CORE.Window.flags & FLAG_VSYNC_HINT) ? 1 : 0);
   if (ok == EGL_FALSE) {
     TRACELOG(LOG_WARNING, "COMMA: eglSwapInterval failed. Error code: %s", eglGetErrorString(eglGetError()));
     return -1;
   }

   EGLint interval = 0;
   eglQueryString(dpy, EGL_EXTENSIONS);                 // check for EGL_BUFFER_AGE_EXT etc.
   eglQuerySurface(dpy, surface, EGL_SWAP_INTERVAL, &interval);
   TRACELOG(LOG_INFO, "COMMA: swap-interval actually in use = %d", interval);

   // enable depth testing. Not necessary if only doing 2D
   glEnable(GL_DEPTH_TEST);

   return 0;
}

static int init_touch(const char *dev_path, const char *origin_path) {
  platform.touch.fd = open(dev_path, O_RDONLY|O_NONBLOCK);
  if (platform.touch.fd < 0) {
    TRACELOG(LOG_WARNING, "COMMA: Failed to open touch device at %s", dev_path);
    return -1;
  }

  FILE *fp = fopen(origin_path, "r");
  if (fp != NULL) {
    int origin;
    int ret = fscanf(fp, "%d", &origin);
    fclose(fp);
    if (ret != 1) {
      TRACELOG(LOG_WARNING, "COMMA: Failed to test for screen origin");
      return -1;
    } else {
      platform.touch.canonical = origin == 1;
    }
  } else {
    TRACELOG(LOG_WARNING, "COMMA: Failed to open screen origin");
    return -1;
  }

  for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
    platform.touch.fingers[i].x = -1;
    platform.touch.fingers[i].y = -1;
    platform.touch.fingers[i].state = FINGER_STATE_REMOVED;
    platform.touch.fingers[i].resetNextFrame = false;

    CORE.Input.Touch.currentTouchState[0] = 0;
    CORE.Input.Touch.previousTouchState[0] = 0;
  }

  for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
    CORE.Input.Mouse.currentButtonState[i] = 0;
    CORE.Input.Mouse.previousButtonState[i] = 0;
  }

  CORE.Input.Mouse.currentPosition.x = -1;
  CORE.Input.Mouse.currentPosition.y = -1;
  CORE.Input.Mouse.previousPosition.x = -1;
  CORE.Input.Mouse.previousPosition.y = -1;

  return 0;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
bool InitGraphicsDevice(void);   // Initialize graphics device

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool WindowShouldClose(void) {
  return false;
}

// Toggle fullscreen mode
void ToggleFullscreen(void) {
  TRACELOG(LOG_WARNING, "ToggleFullscreen() not available on target platform");
}

// Toggle borderless windowed mode
void ToggleBorderlessWindowed(void) {
  TRACELOG(LOG_WARNING, "ToggleBorderlessWindowed() not available on target platform");
}

// Set window state: maximized, if resizable
void MaximizeWindow(void) {
  TRACELOG(LOG_WARNING, "MaximizeWindow() not available on target platform");
}

// Set window state: minimized
void MinimizeWindow(void) {
  TRACELOG(LOG_WARNING, "MinimizeWindow() not available on target platform");
}

// Set window state: not minimized/maximized
void RestoreWindow(void) {
  TRACELOG(LOG_WARNING, "RestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void SetWindowState(unsigned int flags) {
  TRACELOG(LOG_WARNING, "SetWindowState() not available on target platform");
}

// Clear window configuration state flags
void ClearWindowState(unsigned int flags) {
  TRACELOG(LOG_WARNING, "ClearWindowState() not available on target platform");
}

// Set icon for window
void SetWindowIcon(Image image) {
  TRACELOG(LOG_WARNING, "SetWindowIcon() not available on target platform");
}

// Set icon for window
void SetWindowIcons(Image *images, int count) {
  TRACELOG(LOG_WARNING, "SetWindowIcons() not available on target platform");
}

// Set title for window
void SetWindowTitle(const char *title) {
  CORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void SetWindowPosition(int x, int y) {
  TRACELOG(LOG_WARNING, "SetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void SetWindowMonitor(int monitor) {
  TRACELOG(LOG_WARNING, "SetWindowMonitor() not available on target platform");
}

// Set window minimum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMinSize(int width, int height) {
  CORE.Window.screenMin.width = width;
  CORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (FLAG_WINDOW_RESIZABLE)
void SetWindowMaxSize(int width, int height) {
  CORE.Window.screenMax.width = width;
  CORE.Window.screenMax.height = height;
}

// Set window dimensions
void SetWindowSize(int width, int height) {
  TRACELOG(LOG_WARNING, "SetWindowSize() not available on target platform");
}

// Set window opacity, value opacity is between 0.0 and 1.0
void SetWindowOpacity(float opacity) {
  TRACELOG(LOG_WARNING, "SetWindowOpacity() not available on target platform");
}

// Set window focused
void SetWindowFocused(void) {
  TRACELOG(LOG_WARNING, "SetWindowFocused() not available on target platform");
}

// Get native window handle
void *GetWindowHandle(void) {
  TRACELOG(LOG_WARNING, "GetWindowHandle() not implemented on target platform");
  return NULL;
}

// Get number of monitors
int GetMonitorCount(void) {
  TRACELOG(LOG_WARNING, "GetMonitorCount() not implemented on target platform");
  return 1;
}

// Get number of monitors
int GetCurrentMonitor(void) {
  TRACELOG(LOG_WARNING, "GetCurrentMonitor() not implemented on target platform");
  return 0;
}

// Get selected monitor position
Vector2 GetMonitorPosition(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorPosition() not implemented on target platform");
  return (Vector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int GetMonitorWidth(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorWidth() not implemented on target platform");
  return 0;
}

// Get selected monitor height (currently used by monitor)
int GetMonitorHeight(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorHeight() not implemented on target platform");
  return 0;
}

// Get selected monitor physical width in millimetres
int GetMonitorPhysicalWidth(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorPhysicalWidth() not implemented on target platform");
  return 0;
}

// Get selected monitor physical height in millimetres
int GetMonitorPhysicalHeight(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorPhysicalHeight() not implemented on target platform");
  return 0;
}

// Get selected monitor refresh rate
int GetMonitorRefreshRate(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorRefreshRate() not implemented on target platform");
  return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *GetMonitorName(int monitor) {
  TRACELOG(LOG_WARNING, "GetMonitorName() not implemented on target platform");
  return "";
}

// Get window position XY on monitor
Vector2 GetWindowPosition(void) {
  TRACELOG(LOG_WARNING, "GetWindowPosition() not implemented on target platform");
  return (Vector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
Vector2 GetWindowScaleDPI(void) {
  TRACELOG(LOG_WARNING, "GetWindowScaleDPI() not implemented on target platform");
  return (Vector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void SetClipboardText(const char *text) {
  TRACELOG(LOG_WARNING, "SetClipboardText() not implemented on target platform");
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *GetClipboardText(void) {
  TRACELOG(LOG_WARNING, "GetClipboardText() not implemented on target platform");
  return NULL;
}

// Get clipboard image
Image GetClipboardImage(void) {
  Image image = { 0 };
  TRACELOG(LOG_WARNING, "GetClipboardImage() not implemented on target platform");
  return image;
}

// Show mouse cursor
void ShowCursor(void) {
  CORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void HideCursor(void) {
  CORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void EnableCursor(void) {
  // Set cursor position in the middle
  SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

  CORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void DisableCursor(void) {
  // Set cursor position in the middle
  SetMousePosition(CORE.Window.screen.width/2, CORE.Window.screen.height/2);

  CORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void SwapScreenBuffer(void) {
  eglSwapBuffers(platform.egl.display, platform.egl.surface);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double GetTime(void) {
  double time = 0.0;
  struct timespec ts = { 0 };
  clock_gettime(CLOCK_MONOTONIC, &ts);
  unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

  time = (double)(nanoSeconds - CORE.Time.base)*1e-9;  // Elapsed time since InitTimer()

  return time;
}

void OpenURL(const char *url) {
  TRACELOG(LOG_WARNING, "OpenURL() not implemented on target platform");
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int SetGamepadMappings(const char *mappings) {
  TRACELOG(LOG_WARNING, "SetGamepadMappings() not implemented on target platform");
  return 0;
}

void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration) {
  TRACELOG(LOG_WARNING, "GamepadSetVibration() not implemented on target platform");
}

// Set mouse position XY
void SetMousePosition(int x, int y) {
  CORE.Input.Mouse.currentPosition = (Vector2){ (float)x, (float)y };
  CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void SetMouseCursor(int cursor) {
  TRACELOG(LOG_WARNING, "SetMouseCursor() not implemented on target platform");
}

// Get physical key name.
const char *GetKeyName(int key) {
  TRACELOG(LOG_WARNING, "GetKeyName() not implemented on target platform");
  return "";
}

void PollInputEvents(void) {
  // slot i is for events of finger i
  static int slot = 0;

  for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
    CORE.Input.Touch.previousTouchState[i] = CORE.Input.Touch.currentTouchState[i];
    // caused by single frame down and up events
    if (platform.touch.fingers[i].resetNextFrame) {
      CORE.Input.Touch.currentTouchState[i] = 0;
      platform.touch.fingers[i].resetNextFrame = false;
    }
  }

  for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
    CORE.Input.Mouse.previousButtonState[i] = CORE.Input.Mouse.currentButtonState[i];
  }

  CORE.Input.Mouse.previousPosition = CORE.Input.Mouse.currentPosition;
  CORE.Input.Touch.pointCount = 0;

  struct input_event event = {0};
  while (read(platform.touch.fd, &event, sizeof(struct input_event)) == sizeof(struct input_event)) {
    if (event.type == SYN_REPORT) { // synchronization frame. Expose completed events back to the library

      for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
        if (platform.touch.fingers[i].state == FINGER_STATE_TOUCHING) {

          CORE.Input.Touch.position[i].x = platform.touch.fingers[i].x;
          CORE.Input.Touch.position[i].y = platform.touch.fingers[i].y;
          CORE.Input.Touch.currentTouchState[i] = 1;

          // map main finger on mouse for conveniance. raylib already does that
          // for pressed state, but not pos
          if (i == 0) {
            CORE.Input.Mouse.currentPosition.x = platform.touch.fingers[i].x;
            CORE.Input.Mouse.currentPosition.y = platform.touch.fingers[i].y;
          }

        } else if (platform.touch.fingers[i].state == FINGER_STATE_REMOVING) {
          CORE.Input.Touch.position[i].x = -1;
          CORE.Input.Touch.position[i].y = -1;

          // if we received a touch down and up event in the same frame,
          // delay up event by one frame so that API user needs no special handling
          if (CORE.Input.Touch.previousTouchState[i] == 0) {
            CORE.Input.Touch.currentTouchState[i] = 1;
            platform.touch.fingers[i].resetNextFrame = true;  // mark to be reset next event update loop
          } else {
            CORE.Input.Touch.currentTouchState[i] = 0;
          }

          platform.touch.fingers[i].state = FINGER_STATE_REMOVED;
        }
      }

    } else if (event.type == EV_ABS) { // raw events. Process these untill we get a sync frame

      if (event.code == ABS_MT_SLOT) { // switch finger
        slot = event.value;
      } else if (event.code == ABS_MT_TRACKING_ID) { // finger on screen or not
        platform.touch.fingers[slot].state = event.value == -1 ? FINGER_STATE_REMOVING : FINGER_STATE_TOUCHING;
      } else if (event.code == ABS_MT_POSITION_X) {
        platform.touch.fingers[slot].y = (1 - platform.touch.canonical) * (CORE.Window.screen.height - event.value) + (platform.touch.canonical * event.value);
      } else if (event.code == ABS_MT_POSITION_Y) {
        platform.touch.fingers[slot].x = platform.touch.canonical * (CORE.Window.screen.width - event.value) + ((1 - platform.touch.canonical) * event.value);
      }
    }
  }

  // count how many fingers are left on the screen after processing all events
  for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
    CORE.Input.Touch.pointCount += platform.touch.fingers[i].state == FINGER_STATE_TOUCHING;
  }
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

int InitPlatform(void) {

  // only support fullscreen
  CORE.Window.fullscreen = true;
  CORE.Window.flags |= FLAG_FULLSCREEN_MODE;

  // in our case, all those width/height are the same
  CORE.Window.currentFbo.width = CORE.Window.screen.width;
  CORE.Window.currentFbo.height = CORE.Window.screen.height;
  CORE.Window.display.width = CORE.Window.screen.width;
  CORE.Window.display.height = CORE.Window.screen.height;
  CORE.Window.render.width = CORE.Window.screen.width;
  CORE.Window.render.height = CORE.Window.screen.height;

  if (init_wayland(CORE.Window.currentFbo.width, CORE.Window.currentFbo.height)) {
    TRACELOG(LOG_FATAL, "COMMA: Failed to initialize Wayland");
    return -1;
  }

  if (init_egl()) {
    TRACELOG(LOG_FATAL, "COMMA: Failed to initialize EGL");
    return -1;
  }

  if (init_touch("/dev/input/event2", "/sys/devices/platform/vendor/vendor:gpio-som-id/som_id")) {
    TRACELOG(LOG_FATAL, "COMMA: Failed to initialize touch device");
    return -1;
  }

  SetupFramebuffer(CORE.Window.display.width, CORE.Window.display.height);
  rlLoadExtensions(eglGetProcAddress);
  InitTimer();
  CORE.Storage.basePath = GetWorkingDirectory();

  TRACELOG(LOG_INFO, "COMMA: Initialized successfully");
  return 0;
}

void ClosePlatform(void) {
}
