//========================================================================
// GLFW 3.3 Mir - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014-2017 Brandon Schaefer <brandon.schaefer@canonical.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

#include <linux/input.h>
#include <stdlib.h>
#include <string.h>


typedef struct EventNode
{
    TAILQ_ENTRY(EventNode) entries;
    const MirEvent*        event;
    _GLFWwindow*           window;
} EventNode;

static void deleteNode(EventQueue* queue, EventNode* node)
{
    mir_event_unref(node->event);
    free(node);
}

static GLFWbool emptyEventQueue(EventQueue* queue)
{
    return queue->head.tqh_first == NULL;
}

// TODO The mir_event_ref is not supposed to be used but ... its needed
//      in this case. Need to wait until we can read from an FD set up by mir
//      for single threaded event handling.
static EventNode* newEventNode(const MirEvent* event, _GLFWwindow* context)
{
    EventNode* newNode = calloc(1, sizeof(EventNode));
    newNode->event     = mir_event_ref(event);
    newNode->window    = context;

    return newNode;
}

static void enqueueEvent(const MirEvent* event, _GLFWwindow* context)
{
    pthread_mutex_lock(&_glfw.mir.eventMutex);

    EventNode* newNode = newEventNode(event, context);
    TAILQ_INSERT_TAIL(&_glfw.mir.eventQueue->head, newNode, entries);

    pthread_cond_signal(&_glfw.mir.eventCond);

    pthread_mutex_unlock(&_glfw.mir.eventMutex);
}

static EventNode* dequeueEvent(EventQueue* queue)
{
    EventNode* node = NULL;

    pthread_mutex_lock(&_glfw.mir.eventMutex);

    node = queue->head.tqh_first;

    if (node)
        TAILQ_REMOVE(&queue->head, node, entries);

    pthread_mutex_unlock(&_glfw.mir.eventMutex);

    return node;
}

static MirPixelFormat findValidPixelFormat(void)
{
    unsigned int i, validFormats, mirPixelFormats = 32;
    MirPixelFormat formats[mir_pixel_formats];

    mir_connection_get_available_surface_formats(_glfw.mir.connection, formats,
                                                 mirPixelFormats, &validFormats);

    for (i = 0;  i < validFormats;  i++)
    {
        if (formats[i] == mir_pixel_format_abgr_8888 ||
            formats[i] == mir_pixel_format_xbgr_8888 ||
            formats[i] == mir_pixel_format_argb_8888 ||
            formats[i] == mir_pixel_format_xrgb_8888)
        {
            return formats[i];
        }
    }

    return mir_pixel_format_invalid;
}

static int mirModToGLFWMod(uint32_t mods)
{
    int publicMods = 0x0;

    if (mods & mir_input_event_modifier_alt)
        publicMods |= GLFW_MOD_ALT;
    else if (mods & mir_input_event_modifier_shift)
        publicMods |= GLFW_MOD_SHIFT;
    else if (mods & mir_input_event_modifier_ctrl)
        publicMods |= GLFW_MOD_CONTROL;
    else if (mods & mir_input_event_modifier_meta)
        publicMods |= GLFW_MOD_SUPER;

    return publicMods;
}

static int toGLFWKeyCode(uint32_t key)
{
    if (key < sizeof(_glfw.mir.keycodes) / sizeof(_glfw.mir.keycodes[0]))
        return _glfw.mir.keycodes[key];

    return GLFW_KEY_UNKNOWN;
}

static void handleKeyEvent(const MirKeyboardEvent* key_event, _GLFWwindow* window)
{
    const int action    = mir_keyboard_event_action   (key_event);
    const int scan_code = mir_keyboard_event_scan_code(key_event);
    const int key_code  = mir_keyboard_event_key_code (key_event);
    const int modifiers = mir_keyboard_event_modifiers(key_event);

    const int  pressed = action == mir_keyboard_action_up ? GLFW_RELEASE : GLFW_PRESS;
    const int  mods    = mirModToGLFWMod(modifiers);
    const long text    = _glfwKeySym2Unicode(key_code);
    const int  plain   = !(mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT));

    _glfwInputKey(window, toGLFWKeyCode(scan_code), scan_code, pressed, mods);

    if (text != -1)
        _glfwInputChar(window, text, mods, plain);
}

static void handlePointerButton(_GLFWwindow* window,
                              int pressed,
                              const MirPointerEvent* pointer_event)
{
    int mods                = mir_pointer_event_modifiers(pointer_event);
    const int publicMods    = mirModToGLFWMod(mods);
    MirPointerButton button = mir_pointer_button_primary;
    static uint32_t oldButtonStates = 0;
    uint32_t newButtonStates        = mir_pointer_event_buttons(pointer_event);
    int publicButton                = GLFW_MOUSE_BUTTON_LEFT;

    // XOR our old button states our new states to figure out what was added or removed
    button = newButtonStates ^ oldButtonStates;

    switch (button)
    {
        case mir_pointer_button_primary:
            publicButton = GLFW_MOUSE_BUTTON_LEFT;
            break;
        case mir_pointer_button_secondary:
            publicButton = GLFW_MOUSE_BUTTON_RIGHT;
            break;
        case mir_pointer_button_tertiary:
            publicButton = GLFW_MOUSE_BUTTON_MIDDLE;
            break;
        case mir_pointer_button_forward:
            // FIXME What is the forward button?
            publicButton = GLFW_MOUSE_BUTTON_4;
            break;
        case mir_pointer_button_back:
            // FIXME What is the back button?
            publicButton = GLFW_MOUSE_BUTTON_5;
            break;
        default:
            break;
    }

    oldButtonStates = newButtonStates;

    _glfwInputMouseClick(window, publicButton, pressed, publicMods);
}

static void handlePointerMotion(_GLFWwindow* window,
                                const MirPointerEvent* pointer_event)
{
    const int hscroll = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_hscroll);
    const int vscroll = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_vscroll);

    if (window->cursorMode == GLFW_CURSOR_DISABLED)
    {
        if (_glfw.mir.disabledCursorWindow != window)
            return;

        const int dx = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_relative_x);
        const int dy = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_relative_y);
        const int current_x = window->virtualCursorPosX;
        const int current_y = window->virtualCursorPosY;

        _glfwInputCursorPos(window, dx + current_x, dy + current_y);
    }
    else
    {
        const int x = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_x);
        const int y = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_y);

        _glfwInputCursorPos(window, x, y);
    }

    if (hscroll != 0 || vscroll != 0)
      _glfwInputScroll(window, hscroll, vscroll);
}

static void handlePointerEvent(const MirPointerEvent* pointer_event,
                             _GLFWwindow* window)
{
    int action = mir_pointer_event_action(pointer_event);

    switch (action)
    {
          case mir_pointer_action_button_down:
              handlePointerButton(window, GLFW_PRESS, pointer_event);
              break;
          case mir_pointer_action_button_up:
              handlePointerButton(window, GLFW_RELEASE, pointer_event);
              break;
          case mir_pointer_action_motion:
              handlePointerMotion(window, pointer_event);
              break;
          case mir_pointer_action_enter:
          case mir_pointer_action_leave:
              break;
          default:
              break;
    }
}

static void handleInput(const MirInputEvent* input_event, _GLFWwindow* window)
{
    int type = mir_input_event_get_type(input_event);

    switch (type)
    {
        case mir_input_event_type_key:
            handleKeyEvent(mir_input_event_get_keyboard_event(input_event), window);
            break;
        case mir_input_event_type_pointer:
            handlePointerEvent(mir_input_event_get_pointer_event(input_event), window);
            break;
        default:
            break;
    }
}

static void handleEvent(const MirEvent* event, _GLFWwindow* window)
{
    int type = mir_event_get_type(event);

    switch (type)
    {
        case mir_event_type_input:
            handleInput(mir_event_get_input_event(event), window);
            break;
        default:
            break;
    }
}

static void addNewEvent(MirWindow* window, const MirEvent* event, void* context)
{
    enqueueEvent(event, context);
}

static GLFWbool createWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;
    MirBufferUsage buffer_usage = mir_buffer_usage_hardware;
    MirPixelFormat pixel_format = findValidPixelFormat();

    if (pixel_format == mir_pixel_format_invalid)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to find a correct pixel format");
        return GLFW_FALSE;
    }

    spec = mir_create_normal_window_spec(_glfw.mir.connection,
                                         window->mir.width,
                                         window->mir.height);

    mir_window_spec_set_pixel_format(spec, pixel_format);
    mir_window_spec_set_buffer_usage(spec, buffer_usage);

    window->mir.window = mir_create_window_sync(spec);
    mir_window_spec_release(spec);

    if (!mir_window_is_valid(window->mir.window))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to create window: %s",
                        mir_window_get_error_message(window->mir.window));

        return GLFW_FALSE;
    }

    mir_window_set_event_handler(window->mir.window, addNewEvent, window);

    return GLFW_TRUE;
}

static void setWindowConfinement(_GLFWwindow* window, MirPointerConfinementState state)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_pointer_confinement(spec, state);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwInitEventQueueMir(EventQueue* queue)
{
    TAILQ_INIT(&queue->head);
}

void _glfwDeleteEventQueueMir(EventQueue* queue)
{
    if (queue)
    {
        EventNode* node, *node_next;
        node = queue->head.tqh_first;

        while (node != NULL)
        {
            node_next = node->entries.tqe_next;

            TAILQ_REMOVE(&queue->head, node, entries);
            deleteNode(queue, node);

            node = node_next;
        }

        free(queue);
    }
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformCreateWindow(_GLFWwindow* window,
                              const _GLFWwndconfig* wndconfig,
                              const _GLFWctxconfig* ctxconfig,
                              const _GLFWfbconfig* fbconfig)
{
    if (window->monitor)
    {
        GLFWvidmode mode;
        _glfwPlatformGetVideoMode(window->monitor, &mode);

        mir_window_set_state(window->mir.window, mir_window_state_fullscreen);

        if (wndconfig->width > mode.width || wndconfig->height > mode.height)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Mir: Requested window size too large: %ix%i",
                            wndconfig->width, wndconfig->height);

            return GLFW_FALSE;
        }
    }

    window->mir.width  = wndconfig->width;
    window->mir.height = wndconfig->height;
    window->mir.currentCursor = NULL;

    if (!createWindow(window))
        return GLFW_FALSE;

    window->mir.nativeWindow = mir_buffer_stream_get_egl_native_window(
        mir_window_get_buffer_stream(window->mir.window));

    if (ctxconfig->client != GLFW_NO_API)
    {
        if (ctxconfig->source == GLFW_EGL_CONTEXT_API ||
            ctxconfig->source == GLFW_NATIVE_CONTEXT_API)
        {
            if (!_glfwInitEGL())
                return GLFW_FALSE;
            if (!_glfwCreateContextEGL(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
        else if (ctxconfig->source == GLFW_OSMESA_CONTEXT_API)
        {
            if (!_glfwInitOSMesa())
                return GLFW_FALSE;
            if (!_glfwCreateContextOSMesa(window, ctxconfig, fbconfig))
                return GLFW_FALSE;
        }
    }

    return GLFW_TRUE;
}

void _glfwPlatformDestroyWindow(_GLFWwindow* window)
{
    if (_glfw.mir.disabledCursorWindow == window)
        _glfw.mir.disabledCursorWindow = NULL;

    if (mir_window_is_valid(window->mir.window))
    {
        mir_window_release_sync(window->mir.window);
        window->mir.window= NULL;
    }

    if (window->context.destroy)
        window->context.destroy(window);
}

void _glfwPlatformSetWindowTitle(_GLFWwindow* window, const char* title)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_name(spec, title);
    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformSetWindowIcon(_GLFWwindow* window,
                                int count, const GLFWimage* images)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowSize(_GLFWwindow* window, int width, int height)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_width (spec, width);
    mir_window_spec_set_height(spec, height);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformSetWindowSizeLimits(_GLFWwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_max_width (spec, maxwidth);
    mir_window_spec_set_max_height(spec, maxheight);
    mir_window_spec_set_min_width (spec, minwidth);
    mir_window_spec_set_min_height(spec, minheight);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformSetWindowAspectRatio(_GLFWwindow* window, int numer, int denom)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowPos(_GLFWwindow* window, int xpos, int ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformGetWindowFrameSize(_GLFWwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformGetWindowPos(_GLFWwindow* window, int* xpos, int* ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformGetWindowSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width  = window->mir.width;
    if (height)
        *height = window->mir.height;
}

void _glfwPlatformGetWindowContentScale(_GLFWwindow* window,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwPlatformIconifyWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_state(spec, mir_window_state_minimized);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformRestoreWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_state(spec, mir_window_state_restored);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformMaximizeWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_state(spec, mir_window_state_maximized);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformHideWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_state(spec, mir_window_state_hidden);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformShowWindow(_GLFWwindow* window)
{
    MirWindowSpec* spec;

    spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_state(spec, mir_window_state_restored);

    mir_window_apply_spec(window->mir.window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformRequestWindowAttention(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformFocusWindow(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowMonitor(_GLFWwindow* window,
                                   _GLFWmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

int _glfwPlatformWindowFocused(_GLFWwindow* window)
{
    return mir_window_get_focus_state(window->mir.window) == mir_window_focus_state_focused;
}

int _glfwPlatformWindowIconified(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
    return GLFW_FALSE;
}

int _glfwPlatformWindowVisible(_GLFWwindow* window)
{
    return mir_window_get_visibility(window->mir.window) == mir_window_visibility_exposed;
}

int _glfwPlatformWindowMaximized(_GLFWwindow* window)
{
    return mir_window_get_state(window->mir.window) == mir_window_state_maximized;
}

int _glfwPlatformFramebufferTransparent(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
    return GLFW_FALSE;
}

void _glfwPlatformSetWindowResizable(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowDecorated(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetWindowFloating(_GLFWwindow* window, GLFWbool enabled)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformPollEvents(void)
{
    EventNode* node = NULL;

    while ((node = dequeueEvent(_glfw.mir.eventQueue)))
    {
        handleEvent(node->event, node->window);
        deleteNode(_glfw.mir.eventQueue, node);
    }
}

void _glfwPlatformWaitEvents(void)
{
    pthread_mutex_lock(&_glfw.mir.eventMutex);

    while (emptyEventQueue(_glfw.mir.eventQueue))
        pthread_cond_wait(&_glfw.mir.eventCond, &_glfw.mir.eventMutex);

    pthread_mutex_unlock(&_glfw.mir.eventMutex);

    _glfwPlatformPollEvents();
}

void _glfwPlatformWaitEventsTimeout(double timeout)
{
    pthread_mutex_lock(&_glfw.mir.eventMutex);

    if (emptyEventQueue(_glfw.mir.eventQueue))
    {
        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += (long) timeout;
        time.tv_nsec += (long) ((timeout - (long) timeout) * 1e9);
        pthread_cond_timedwait(&_glfw.mir.eventCond, &_glfw.mir.eventMutex, &time);
    }

    pthread_mutex_unlock(&_glfw.mir.eventMutex);

    _glfwPlatformPollEvents();
}

void _glfwPlatformPostEmptyEvent(void)
{
}

void _glfwPlatformGetFramebufferSize(_GLFWwindow* window, int* width, int* height)
{
    if (width)
        *width  = window->mir.width;
    if (height)
        *height = window->mir.height;
}

int _glfwPlatformCreateCursor(_GLFWcursor* cursor,
                              const GLFWimage* image,
                              int xhot, int yhot)
{
    MirBufferStream* stream;

    int i_w = image->width;
    int i_h = image->height;

    stream = mir_connection_create_buffer_stream_sync(_glfw.mir.connection,
                                                      i_w, i_h,
                                                      mir_pixel_format_argb_8888,
                                                      mir_buffer_usage_software);

    cursor->mir.conf = mir_cursor_configuration_from_buffer_stream(stream, xhot, yhot);

    MirGraphicsRegion region;
    mir_buffer_stream_get_graphics_region(stream, &region);

    unsigned char* pixels = image->pixels;
    char* dest = region.vaddr;
    int i;

    for (i = 0; i < i_w * i_h; i++, pixels += 4)
    {
        unsigned int alpha = pixels[3];
        *dest++ = (char)(pixels[2] * alpha / 255);
        *dest++ = (char)(pixels[1] * alpha / 255);
        *dest++ = (char)(pixels[0] * alpha / 255);
        *dest++ = (char)alpha;
    }

    mir_buffer_stream_swap_buffers_sync(stream);
    cursor->mir.customCursor = stream;

    return GLFW_TRUE;
}

static const char* getSystemCursorName(int shape)
{
    switch (shape)
    {
        case GLFW_ARROW_CURSOR:
            return mir_arrow_cursor_name;
        case GLFW_IBEAM_CURSOR:
            return mir_caret_cursor_name;
        case GLFW_CROSSHAIR_CURSOR:
            return mir_crosshair_cursor_name;
        case GLFW_HAND_CURSOR:
            return mir_open_hand_cursor_name;
        case GLFW_HRESIZE_CURSOR:
            return mir_horizontal_resize_cursor_name;
        case GLFW_VRESIZE_CURSOR:
            return mir_vertical_resize_cursor_name;
    }

    return NULL;
}

int _glfwPlatformCreateStandardCursor(_GLFWcursor* cursor, int shape)
{
    cursor->mir.conf         = NULL;
    cursor->mir.customCursor = NULL;
    cursor->mir.cursorName   = getSystemCursorName(shape);

    return cursor->mir.cursorName != NULL;
}

void _glfwPlatformDestroyCursor(_GLFWcursor* cursor)
{
    if (cursor->mir.conf)
        mir_cursor_configuration_destroy(cursor->mir.conf);
    if (cursor->mir.customCursor)
        mir_buffer_stream_release_sync(cursor->mir.customCursor);
}

static void setCursorNameForWindow(MirWindow* window, char const* name)
{
    MirWindowSpec* spec = mir_create_window_spec(_glfw.mir.connection);
    mir_window_spec_set_cursor_name(spec, name);
    mir_window_apply_spec(window, spec);
    mir_window_spec_release(spec);
}

void _glfwPlatformSetCursor(_GLFWwindow* window, _GLFWcursor* cursor)
{
    if (cursor)
    {
        window->mir.currentCursor = cursor;

        if (cursor->mir.cursorName)
        {
            setCursorNameForWindow(window->mir.window, cursor->mir.cursorName);
        }
        else if (cursor->mir.conf)
        {
            mir_window_configure_cursor(window->mir.window, cursor->mir.conf);
        }
    }
    else
    {
        setCursorNameForWindow(window->mir.window, mir_default_cursor_name);
    }
}

void _glfwPlatformGetCursorPos(_GLFWwindow* window, double* xpos, double* ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetCursorPos(_GLFWwindow* window, double xpos, double ypos)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetCursorMode(_GLFWwindow* window, int mode)
{
    if (mode == GLFW_CURSOR_DISABLED)
    {
        _glfw.mir.disabledCursorWindow = window;
        setWindowConfinement(window, mir_pointer_confined_to_window);
        setCursorNameForWindow(window->mir.window, mir_disabled_cursor_name);
    }
    else
    {
        // If we were disabled before lets undo that!
        if (_glfw.mir.disabledCursorWindow == window)
        {
            _glfw.mir.disabledCursorWindow = NULL;
            setWindowConfinement(window, mir_pointer_unconfined);
        }

        if (window->cursorMode == GLFW_CURSOR_NORMAL)
        {
            _glfwPlatformSetCursor(window, window->mir.currentCursor);
        }
        else if (window->cursorMode == GLFW_CURSOR_HIDDEN)
        {
            setCursorNameForWindow(window->mir.window, mir_disabled_cursor_name);
        }
    }
}

const char* _glfwPlatformGetScancodeName(int scancode)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
    return NULL;
}

int _glfwPlatformGetKeyScancode(int key)
{
    return _glfw.mir.scancodes[key];
}

void _glfwPlatformSetClipboardString(_GLFWwindow* window, const char* string)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

const char* _glfwPlatformGetClipboardString(_GLFWwindow* window)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);

    return NULL;
}

void _glfwPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (!_glfw.vk.KHR_surface || !_glfw.vk.KHR_mir_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_mir_surface";
}

int _glfwPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceMirPresentationSupportKHR
        vkGetPhysicalDeviceMirPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
    if (!vkGetPhysicalDeviceMirPresentationSupportKHR)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Mir: Vulkan instance missing VK_KHR_mir_surface extension");
        return GLFW_FALSE;
    }

    return vkGetPhysicalDeviceMirPresentationSupportKHR(device,
                                                        queuefamily,
                                                        _glfw.mir.connection);
}

VkResult _glfwPlatformCreateWindowSurface(VkInstance instance,
                                          _GLFWwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    VkResult err;
    VkMirWindowCreateInfoKHR sci;
    PFN_vkCreateMirWindowKHR vkCreateMirWindowKHR;

    vkCreateMirWindowKHR = (PFN_vkCreateMirWindowKHR)
        vkGetInstanceProcAddr(instance, "vkCreateMirWindowKHR");
    if (!vkCreateMirWindowKHR)
    {
        _glfwInputError(GLFW_API_UNAVAILABLE,
                        "Mir: Vulkan instance missing VK_KHR_mir_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_MIR_SURFACE_CREATE_INFO_KHR;
    sci.connection = _glfw.mir.connection;
    sci.mirWindow  = window->mir.window;

    err = vkCreateMirWindowKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Failed to create Vulkan surface: %s",
                        _glfwGetVulkanResultString(err));
    }

    return err;
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI MirConnection* glfwGetMirDisplay(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return _glfw.mir.connection;
}

GLFWAPI MirWindow* glfwGetMirWindow(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(NULL);
    return window->mir.window;
}

