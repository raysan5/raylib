/*!

@page window_guide Window guide

@tableofcontents

This guide introduces the window related functions of GLFW.  For details on
a specific function in this category, see the @ref window.  There are also
guides for the other areas of GLFW.

 - @ref intro_guide
 - @ref context_guide
 - @ref vulkan_guide
 - @ref monitor_guide
 - @ref input_guide


@section window_object Window objects

The @ref GLFWwindow object encapsulates both a window and a context.  They are
created with @ref glfwCreateWindow and destroyed with @ref glfwDestroyWindow, or
@ref glfwTerminate, if any remain.  As the window and context are inseparably
linked, the object pointer is used as both a context and window handle.

To see the event stream provided to the various window related callbacks, run
the `events` test program.


@subsection window_creation Window creation

A window and its OpenGL or OpenGL ES context are created with @ref
glfwCreateWindow, which returns a handle to the created window object.  For
example, this creates a 640 by 480 windowed mode window:

@code
GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
@endcode

If window creation fails, `NULL` will be returned, so it is necessary to check
the return value.

The window handle is passed to all window related functions and is provided to
along with all input events, so event handlers can tell which window received
the event.


@subsubsection window_full_screen Full screen windows

To create a full screen window, you need to specify which monitor the window
should use.  In most cases, the user's primary monitor is a good choice.
For more information about retrieving monitors, see @ref monitor_monitors.

@code
GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", glfwGetPrimaryMonitor(), NULL);
@endcode

Full screen windows cover the entire display area of a monitor, have no border
or decorations.

Windowed mode windows can be made full screen by setting a monitor with @ref
glfwSetWindowMonitor, and full screen ones can be made windowed by unsetting it
with the same function.

Each field of the @ref GLFWvidmode structure corresponds to a function parameter
or window hint and combine to form the _desired video mode_ for that window.
The supported video mode most closely matching the desired video mode will be
set for the chosen monitor as long as the window has input focus.  For more
information about retrieving video modes, see @ref monitor_modes.

Video mode field        | Corresponds to
----------------        | --------------
GLFWvidmode.width       | `width` parameter of @ref glfwCreateWindow
GLFWvidmode.height      | `height` parameter of @ref glfwCreateWindow
GLFWvidmode.redBits     | @ref GLFW_RED_BITS hint
GLFWvidmode.greenBits   | @ref GLFW_GREEN_BITS hint
GLFWvidmode.blueBits    | @ref GLFW_BLUE_BITS hint
GLFWvidmode.refreshRate | @ref GLFW_REFRESH_RATE hint

Once you have a full screen window, you can change its resolution, refresh rate
and monitor with @ref glfwSetWindowMonitor.  If you only need change its
resolution you can also call @ref glfwSetWindowSize.  In all cases, the new
video mode will be selected the same way as the video mode chosen by @ref
glfwCreateWindow.  If the window has an OpenGL or OpenGL ES context, it will be
unaffected.

By default, the original video mode of the monitor will be restored and the
window iconified if it loses input focus, to allow the user to switch back to
the desktop.  This behavior can be disabled with the
[GLFW_AUTO_ICONIFY](@ref GLFW_AUTO_ICONIFY_hint) window hint, for example if you
wish to simultaneously cover multiple monitors with full screen windows.

If a monitor is disconnected, all windows that are full screen on that monitor
will be switched to windowed mode.  See @ref monitor_event for more information.


@subsubsection window_windowed_full_screen "Windowed full screen" windows

If the closest match for the desired video mode is the current one, the video
mode will not be changed, making window creation faster and application
switching much smoother.  This is sometimes called _windowed full screen_ or
_borderless full screen_ window and counts as a full screen window.  To create
such a window, request the current video mode.

@code
const GLFWvidmode* mode = glfwGetVideoMode(monitor);

glfwWindowHint(GLFW_RED_BITS, mode->redBits);
glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "My Title", monitor, NULL);
@endcode

This also works for windowed mode windows that are made full screen.

@code
const GLFWvidmode* mode = glfwGetVideoMode(monitor);

glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
@endcode

Note that @ref glfwGetVideoMode returns the _current_ video mode of a monitor,
so if you already have a full screen window on that monitor that you want to
make windowed full screen, you need to have saved the desktop resolution before.


@subsection window_destruction Window destruction

When a window is no longer needed, destroy it with @ref glfwDestroyWindow.

@code
glfwDestroyWindow(window);
@endcode

Window destruction always succeeds.  Before the actual destruction, all
callbacks are removed so no further events will be delivered for the window.
All windows remaining when @ref glfwTerminate is called are destroyed as well.

When a full screen window is destroyed, the original video mode of its monitor
is restored, but the gamma ramp is left untouched.


@subsection window_hints Window creation hints

There are a number of hints that can be set before the creation of a window and
context.  Some affect the window itself, others affect the framebuffer or
context.  These hints are set to their default values each time the library is
initialized with @ref glfwInit.  Integer value hints can be set individually
with @ref glfwWindowHint and string value hints with @ref glfwWindowHintString.
You can reset all at once to their defaults with @ref glfwDefaultWindowHints.

Some hints are platform specific.  These are always valid to set on any
platform but they will only affect their specific platform.  Other platforms
will ignore them.  Setting these hints requires no platform specific headers or
calls.

@note Window hints need to be set before the creation of the window and context
you wish to have the specified attributes.  They function as additional
arguments to @ref glfwCreateWindow.


@subsubsection window_hints_hard Hard and soft constraints

Some window hints are hard constraints.  These must match the available
capabilities _exactly_ for window and context creation to succeed.  Hints
that are not hard constraints are matched as closely as possible, but the
resulting context and framebuffer may differ from what these hints requested.

The following hints are always hard constraints:
- @ref GLFW_STEREO
- @ref GLFW_DOUBLEBUFFER
- [GLFW_CLIENT_API](@ref GLFW_CLIENT_API_hint)
- [GLFW_CONTEXT_CREATION_API](@ref GLFW_CONTEXT_CREATION_API_hint)

The following additional hints are hard constraints when requesting an OpenGL
context, but are ignored when requesting an OpenGL ES context:
- [GLFW_OPENGL_FORWARD_COMPAT](@ref GLFW_OPENGL_FORWARD_COMPAT_hint)
- [GLFW_OPENGL_PROFILE](@ref GLFW_OPENGL_PROFILE_hint)


@subsubsection window_hints_wnd Window related hints

@anchor GLFW_RESIZABLE_hint
__GLFW_RESIZABLE__ specifies whether the windowed mode window will be resizable
_by the user_.  The window will still be resizable using the @ref
glfwSetWindowSize function.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.
This hint is ignored for full screen and undecorated windows.

@anchor GLFW_VISIBLE_hint
__GLFW_VISIBLE__ specifies whether the windowed mode window will be initially
visible.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This hint is
ignored for full screen windows.

@anchor GLFW_DECORATED_hint
__GLFW_DECORATED__ specifies whether the windowed mode window will have window
decorations such as a border, a close widget, etc.  An undecorated window will
not be resizable by the user but will still allow the user to generate close
events on some platforms.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.
This hint is ignored for full screen windows.

@anchor GLFW_FOCUSED_hint
__GLFW_FOCUSED__ specifies whether the windowed mode window will be given input
focus when created.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This
hint is ignored for full screen and initially hidden windows.

@anchor GLFW_AUTO_ICONIFY_hint
__GLFW_AUTO_ICONIFY__ specifies whether the full screen window will
automatically iconify and restore the previous video mode on input focus loss.
Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This hint is ignored for
windowed mode windows.

@anchor GLFW_FLOATING_hint
__GLFW_FLOATING__ specifies whether the windowed mode window will be floating
above other regular windows, also called topmost or always-on-top.  This is
intended primarily for debugging purposes and cannot be used to implement proper
full screen windows.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This
hint is ignored for full screen windows.

@anchor GLFW_MAXIMIZED_hint
__GLFW_MAXIMIZED__ specifies whether the windowed mode window will be maximized
when created.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This hint is
ignored for full screen windows.

@anchor GLFW_CENTER_CURSOR_hint
__GLFW_CENTER_CURSOR__ specifies whether the cursor should be centered over
newly created full screen windows.  Possible values are `GLFW_TRUE` and
`GLFW_FALSE`.  This hint is ignored for windowed mode windows.

@anchor GLFW_TRANSPARENT_FRAMEBUFFER_hint
__GLFW_TRANSPARENT_FRAMEBUFFER__ specifies whether the window framebuffer will
be transparent.  If enabled and supported by the system, the window framebuffer
alpha channel will be used to combine the framebuffer with the background.  This
does not affect window decorations.  Possible values are `GLFW_TRUE` and
`GLFW_FALSE`.

@anchor GLFW_FOCUS_ON_SHOW_hint
__GLFW_FOCUS_ON_SHOW__ specifies whether the window will be given input
focus when @ref glfwShowWindow is called. Possible values are `GLFW_TRUE` and
`GLFW_FALSE`.

@anchor GLFW_SCALE_TO_MONITOR
__GLFW_SCALE_TO_MONITOR__ specified whether the window content area should be
resized based on the [monitor content scale](@ref monitor_scale) of any monitor
it is placed on.  This includes the initial placement when the window is
created.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.

This hint only has an effect on platforms where screen coordinates and pixels
always map 1:1 such as Windows and X11.  On platforms like macOS the resolution
of the framebuffer is changed independently of the window size.


@subsubsection window_hints_fb Framebuffer related hints

@anchor GLFW_RED_BITS
@anchor GLFW_GREEN_BITS
@anchor GLFW_BLUE_BITS
@anchor GLFW_ALPHA_BITS
@anchor GLFW_DEPTH_BITS
@anchor GLFW_STENCIL_BITS
__GLFW_RED_BITS__, __GLFW_GREEN_BITS__, __GLFW_BLUE_BITS__, __GLFW_ALPHA_BITS__,
__GLFW_DEPTH_BITS__ and __GLFW_STENCIL_BITS__ specify the desired bit depths of
the various components of the default framebuffer.  A value of `GLFW_DONT_CARE`
means the application has no preference.

@anchor GLFW_ACCUM_RED_BITS
@anchor GLFW_ACCUM_GREEN_BITS
@anchor GLFW_ACCUM_BLUE_BITS
@anchor GLFW_ACCUM_ALPHA_BITS
__GLFW_ACCUM_RED_BITS__, __GLFW_ACCUM_GREEN_BITS__, __GLFW_ACCUM_BLUE_BITS__ and
__GLFW_ACCUM_ALPHA_BITS__ specify the desired bit depths of the various
components of the accumulation buffer.  A value of `GLFW_DONT_CARE` means the
application has no preference.

Accumulation buffers are a legacy OpenGL feature and should not be used in new
code.

@anchor GLFW_AUX_BUFFERS
__GLFW_AUX_BUFFERS__ specifies the desired number of auxiliary buffers.  A value
of `GLFW_DONT_CARE` means the application has no preference.

Auxiliary buffers are a legacy OpenGL feature and should not be used in new
code.

@anchor GLFW_STEREO
__GLFW_STEREO__ specifies whether to use OpenGL stereoscopic rendering.
Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This is a hard constraint.

@anchor GLFW_SAMPLES
__GLFW_SAMPLES__ specifies the desired number of samples to use for
multisampling.  Zero disables multisampling.  A value of `GLFW_DONT_CARE` means
the application has no preference.

@anchor GLFW_SRGB_CAPABLE
__GLFW_SRGB_CAPABLE__ specifies whether the framebuffer should be sRGB capable.
Possible values are `GLFW_TRUE` and `GLFW_FALSE`.

@note __OpenGL:__ If enabled and supported by the system, the
`GL_FRAMEBUFFER_SRGB` enable will control sRGB rendering.  By default, sRGB
rendering will be disabled.

@note __OpenGL ES:__ If enabled and supported by the system, the context will
always have sRGB rendering enabled.

@anchor GLFW_DOUBLEBUFFER
__GLFW_DOUBLEBUFFER__ specifies whether the framebuffer should be double
buffered.  You nearly always want to use double buffering.  This is a hard
constraint.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.


@subsubsection window_hints_mtr Monitor related hints

@anchor GLFW_REFRESH_RATE
__GLFW_REFRESH_RATE__ specifies the desired refresh rate for full screen
windows.  A value of `GLFW_DONT_CARE` means the highest available refresh rate
will be used.  This hint is ignored for windowed mode windows.


@subsubsection window_hints_ctx Context related hints

@anchor GLFW_CLIENT_API_hint
__GLFW_CLIENT_API__ specifies which client API to create the context for.
Possible values are `GLFW_OPENGL_API`, `GLFW_OPENGL_ES_API` and `GLFW_NO_API`.
This is a hard constraint.

@anchor GLFW_CONTEXT_CREATION_API_hint
__GLFW_CONTEXT_CREATION_API__ specifies which context creation API to use to
create the context.  Possible values are `GLFW_NATIVE_CONTEXT_API`,
`GLFW_EGL_CONTEXT_API` and `GLFW_OSMESA_CONTEXT_API`.  This is a hard
constraint.  If no client API is requested, this hint is ignored.

An [extension loader library](@ref context_glext_auto) that assumes it knows
which API was used to create the current context may fail if you change this
hint.  This can be resolved by having it load functions via @ref
glfwGetProcAddress.

@note @wayland The EGL API _is_ the native context creation API, so this hint
will have no effect.

@note @x11 On some Linux systems, creating contexts via both the native and EGL
APIs in a single process will cause the application to segfault.  Stick to one
API or the other on Linux for now.

@note __OSMesa:__ As its name implies, an OpenGL context created with OSMesa
does not update the window contents when its buffers are swapped.  Use OpenGL
functions or the OSMesa native access functions @ref glfwGetOSMesaColorBuffer
and @ref glfwGetOSMesaDepthBuffer to retrieve the framebuffer contents.

@anchor GLFW_CONTEXT_VERSION_MAJOR_hint
@anchor GLFW_CONTEXT_VERSION_MINOR_hint
__GLFW_CONTEXT_VERSION_MAJOR__ and __GLFW_CONTEXT_VERSION_MINOR__ specify the
client API version that the created context must be compatible with.  The exact
behavior of these hints depend on the requested client API.

While there is no way to ask the driver for a context of the highest supported
version, GLFW will attempt to provide this when you ask for a version 1.0
context, which is the default for these hints.

Do not confuse these hints with @ref GLFW_VERSION_MAJOR and @ref
GLFW_VERSION_MINOR, which provide the API version of the GLFW header.

@note __OpenGL:__ These hints are not hard constraints, but creation will fail
if the OpenGL version of the created context is less than the one requested.  It
is therefore perfectly safe to use the default of version 1.0 for legacy code
and you will still get backwards-compatible contexts of version 3.0 and above
when available.

@note __OpenGL ES:__ These hints are not hard constraints, but creation will
fail if the OpenGL ES version of the created context is less than the one
requested.  Additionally, OpenGL ES 1.x cannot be returned if 2.0 or later was
requested, and vice versa.  This is because OpenGL ES 3.x is backward compatible
with 2.0, but OpenGL ES 2.0 is not backward compatible with 1.x.

@note @macos The OS only supports forward-compatible core profile contexts for
OpenGL versions 3.2 and later.  Before creating an OpenGL context of version
3.2 or later you must set the
[GLFW_OPENGL_FORWARD_COMPAT](@ref GLFW_OPENGL_FORWARD_COMPAT_hint) and
[GLFW_OPENGL_PROFILE](@ref GLFW_OPENGL_PROFILE_hint) hints accordingly.  OpenGL
3.0 and 3.1 contexts are not supported at all on macOS.

@anchor GLFW_OPENGL_FORWARD_COMPAT_hint
__GLFW_OPENGL_FORWARD_COMPAT__ specifies whether the OpenGL context should be
forward-compatible, i.e. one where all functionality deprecated in the requested
version of OpenGL is removed.  This must only be used if the requested OpenGL
version is 3.0 or above.  If OpenGL ES is requested, this hint is ignored.

Forward-compatibility is described in detail in the
[OpenGL Reference Manual](https://www.opengl.org/registry/).

@anchor GLFW_OPENGL_DEBUG_CONTEXT_hint
__GLFW_OPENGL_DEBUG_CONTEXT__ specifies whether the context should be created
in debug mode, which may provide additional error and diagnostic reporting
functionality.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.

Debug contexts for OpenGL and OpenGL ES are described in detail by the
[GL_KHR_debug](https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt)
extension.

@anchor GLFW_OPENGL_PROFILE_hint
__GLFW_OPENGL_PROFILE__ specifies which OpenGL profile to create the context
for.  Possible values are one of `GLFW_OPENGL_CORE_PROFILE` or
`GLFW_OPENGL_COMPAT_PROFILE`, or `GLFW_OPENGL_ANY_PROFILE` to not request
a specific profile.  If requesting an OpenGL version below 3.2,
`GLFW_OPENGL_ANY_PROFILE` must be used.  If OpenGL ES is requested, this hint
is ignored.

OpenGL profiles are described in detail in the
[OpenGL Reference Manual](https://www.opengl.org/registry/).

@anchor GLFW_CONTEXT_ROBUSTNESS_hint
__GLFW_CONTEXT_ROBUSTNESS__ specifies the robustness strategy to be used by the
context.  This can be one of `GLFW_NO_RESET_NOTIFICATION` or
`GLFW_LOSE_CONTEXT_ON_RESET`, or `GLFW_NO_ROBUSTNESS` to not request
a robustness strategy.

@anchor GLFW_CONTEXT_RELEASE_BEHAVIOR_hint
__GLFW_CONTEXT_RELEASE_BEHAVIOR__ specifies the release behavior to be
used by the context.  Possible values are one of `GLFW_ANY_RELEASE_BEHAVIOR`,
`GLFW_RELEASE_BEHAVIOR_FLUSH` or `GLFW_RELEASE_BEHAVIOR_NONE`.  If the
behavior is `GLFW_ANY_RELEASE_BEHAVIOR`, the default behavior of the context
creation API will be used.  If the behavior is `GLFW_RELEASE_BEHAVIOR_FLUSH`,
the pipeline will be flushed whenever the context is released from being the
current one.  If the behavior is `GLFW_RELEASE_BEHAVIOR_NONE`, the pipeline will
not be flushed on release.

Context release behaviors are described in detail by the
[GL_KHR_context_flush_control](https://www.opengl.org/registry/specs/KHR/context_flush_control.txt)
extension.

@anchor GLFW_CONTEXT_NO_ERROR_hint
__GLFW_CONTEXT_NO_ERROR__ specifies whether errors should be generated by the
context.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  If enabled,
situations that would have generated errors instead cause undefined behavior.

The no error mode for OpenGL and OpenGL ES is described in detail by the
[GL_KHR_no_error](https://www.opengl.org/registry/specs/KHR/no_error.txt)
extension.


@subsubsection window_hints_osx macOS specific window hints

@anchor GLFW_COCOA_RETINA_FRAMEBUFFER_hint
__GLFW_COCOA_RETINA_FRAMEBUFFER__ specifies whether to use full resolution
framebuffers on Retina displays.  Possible values are `GLFW_TRUE` and
`GLFW_FALSE`.  This is ignored on other platforms.

@anchor GLFW_COCOA_FRAME_NAME_hint
__GLFW_COCOA_FRAME_NAME__ specifies the UTF-8 encoded name to use for autosaving
the window frame, or if empty disables frame autosaving for the window.  This is
ignored on other platforms.  This is set with @ref glfwWindowHintString.

@anchor GLFW_COCOA_GRAPHICS_SWITCHING_hint
__GLFW_COCOA_GRAPHICS_SWITCHING__ specifies whether to in Automatic Graphics
Switching, i.e. to allow the system to choose the integrated GPU for the OpenGL
context and move it between GPUs if necessary or whether to force it to always
run on the discrete GPU.  This only affects systems with both integrated and
discrete GPUs.  Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  This is
ignored on other platforms.

Simpler programs and tools may want to enable this to save power, while games
and other applications performing advanced rendering will want to leave it
disabled.

A bundled application that wishes to participate in Automatic Graphics Switching
should also declare this in its `Info.plist` by setting the
`NSSupportsAutomaticGraphicsSwitching` key to `true`.


@subsubsection window_hints_x11 X11 specific window hints

@anchor GLFW_X11_CLASS_NAME_hint
@anchor GLFW_X11_INSTANCE_NAME_hint
__GLFW_X11_CLASS_NAME__ and __GLFW_X11_INSTANCE_NAME__ specifies the desired
ASCII encoded class and instance parts of the ICCCM `WM_CLASS` window property.  Both
hints need to be set to something other than an empty string for them to take effect.
These are set with @ref glfwWindowHintString.


@subsubsection window_hints_values Supported and default values

Window hint                   | Default value               | Supported values
----------------------------- | --------------------------- | ----------------
GLFW_RESIZABLE                | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_VISIBLE                  | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_DECORATED                | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_FOCUSED                  | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_AUTO_ICONIFY             | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_FLOATING                 | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_MAXIMIZED                | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_CENTER_CURSOR            | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_TRANSPARENT_FRAMEBUFFER  | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_FOCUS_ON_SHOW            | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_SCALE_TO_MONITOR         | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_RED_BITS                 | 8                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_GREEN_BITS               | 8                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_BLUE_BITS                | 8                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_ALPHA_BITS               | 8                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_DEPTH_BITS               | 24                          | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_STENCIL_BITS             | 8                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_ACCUM_RED_BITS           | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_ACCUM_GREEN_BITS         | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_ACCUM_BLUE_BITS          | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_ACCUM_ALPHA_BITS         | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_AUX_BUFFERS              | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_SAMPLES                  | 0                           | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_REFRESH_RATE             | `GLFW_DONT_CARE`            | 0 to `INT_MAX` or `GLFW_DONT_CARE`
GLFW_STEREO                   | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_SRGB_CAPABLE             | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_DOUBLEBUFFER             | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_CLIENT_API               | `GLFW_OPENGL_API`           | `GLFW_OPENGL_API`, `GLFW_OPENGL_ES_API` or `GLFW_NO_API`
GLFW_CONTEXT_CREATION_API     | `GLFW_NATIVE_CONTEXT_API`   | `GLFW_NATIVE_CONTEXT_API`, `GLFW_EGL_CONTEXT_API` or `GLFW_OSMESA_CONTEXT_API`
GLFW_CONTEXT_VERSION_MAJOR    | 1                           | Any valid major version number of the chosen client API
GLFW_CONTEXT_VERSION_MINOR    | 0                           | Any valid minor version number of the chosen client API
GLFW_CONTEXT_ROBUSTNESS       | `GLFW_NO_ROBUSTNESS`        | `GLFW_NO_ROBUSTNESS`, `GLFW_NO_RESET_NOTIFICATION` or `GLFW_LOSE_CONTEXT_ON_RESET`
GLFW_CONTEXT_RELEASE_BEHAVIOR | `GLFW_ANY_RELEASE_BEHAVIOR` | `GLFW_ANY_RELEASE_BEHAVIOR`, `GLFW_RELEASE_BEHAVIOR_FLUSH` or `GLFW_RELEASE_BEHAVIOR_NONE`
GLFW_OPENGL_FORWARD_COMPAT    | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_OPENGL_DEBUG_CONTEXT     | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_OPENGL_PROFILE           | `GLFW_OPENGL_ANY_PROFILE`   | `GLFW_OPENGL_ANY_PROFILE`, `GLFW_OPENGL_COMPAT_PROFILE` or `GLFW_OPENGL_CORE_PROFILE`
GLFW_COCOA_RETINA_FRAMEBUFFER | `GLFW_TRUE`                 | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_COCOA_FRAME_NAME         | `""`                        | A UTF-8 encoded frame autosave name
GLFW_COCOA_GRAPHICS_SWITCHING | `GLFW_FALSE`                | `GLFW_TRUE` or `GLFW_FALSE`
GLFW_X11_CLASS_NAME           | `""`                        | An ASCII encoded `WM_CLASS` class name
GLFW_X11_INSTANCE_NAME        | `""`                        | An ASCII encoded `WM_CLASS` instance name


@section window_events Window event processing

See @ref events.


@section window_properties Window properties and events

@subsection window_userptr User pointer

Each window has a user pointer that can be set with @ref
glfwSetWindowUserPointer and queried with @ref glfwGetWindowUserPointer.  This
can be used for any purpose you need and will not be modified by GLFW throughout
the life-time of the window.

The initial value of the pointer is `NULL`.


@subsection window_close Window closing and close flag

When the user attempts to close the window, for example by clicking the close
widget or using a key chord like Alt+F4, the _close flag_ of the window is set.
The window is however not actually destroyed and, unless you watch for this
state change, nothing further happens.

The current state of the close flag is returned by @ref glfwWindowShouldClose
and can be set or cleared directly with @ref glfwSetWindowShouldClose.  A common
pattern is to use the close flag as a main loop condition.

@code
while (!glfwWindowShouldClose(window))
{
    render(window);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
@endcode

If you wish to be notified when the user attempts to close a window, set a close
callback.

@code
glfwSetWindowCloseCallback(window, window_close_callback);
@endcode

The callback function is called directly _after_ the close flag has been set.
It can be used for example to filter close requests and clear the close flag
again unless certain conditions are met.

@code
void window_close_callback(GLFWwindow* window)
{
    if (!time_to_close)
        glfwSetWindowShouldClose(window, GLFW_FALSE);
}
@endcode


@subsection window_size Window size

The size of a window can be changed with @ref glfwSetWindowSize.  For windowed
mode windows, this sets the size, in
[screen coordinates](@ref coordinate_systems) of the _content area_ or _content
area_ of the window.  The window system may impose limits on window size.

@code
glfwSetWindowSize(window, 640, 480);
@endcode

For full screen windows, the specified size becomes the new resolution of the
window's desired video mode.  The video mode most closely matching the new
desired video mode is set immediately.  The window is resized to fit the
resolution of the set video mode.

If you wish to be notified when a window is resized, whether by the user, the
system or your own code, set a size callback.

@code
glfwSetWindowSizeCallback(window, window_size_callback);
@endcode

The callback function receives the new size, in screen coordinates, of the
content area of the window when the window is resized.

@code
void window_size_callback(GLFWwindow* window, int width, int height)
{
}
@endcode

There is also @ref glfwGetWindowSize for directly retrieving the current size of
a window.

@code
int width, height;
glfwGetWindowSize(window, &width, &height);
@endcode

@note Do not pass the window size to `glViewport` or other pixel-based OpenGL
calls.  The window size is in screen coordinates, not pixels.  Use the
[framebuffer size](@ref window_fbsize), which is in pixels, for pixel-based
calls.

The above functions work with the size of the content area, but decorated
windows typically have title bars and window frames around this rectangle.  You
can retrieve the extents of these with @ref glfwGetWindowFrameSize.

@code
int left, top, right, bottom;
glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
@endcode

The returned values are the distances, in screen coordinates, from the edges of
the content area to the corresponding edges of the full window.  As they are
distances and not coordinates, they are always zero or positive.


@subsection window_fbsize Framebuffer size

While the size of a window is measured in screen coordinates, OpenGL works with
pixels.  The size you pass into `glViewport`, for example, should be in pixels.
On some machines screen coordinates and pixels are the same, but on others they
will not be.  There is a second set of functions to retrieve the size, in
pixels, of the framebuffer of a window.

If you wish to be notified when the framebuffer of a window is resized, whether
by the user or the system, set a size callback.

@code
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
@endcode

The callback function receives the new size of the framebuffer when it is
resized, which can for example be used to update the OpenGL viewport.

@code
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
@endcode

There is also @ref glfwGetFramebufferSize for directly retrieving the current
size of the framebuffer of a window.

@code
int width, height;
glfwGetFramebufferSize(window, &width, &height);
glViewport(0, 0, width, height);
@endcode

The size of a framebuffer may change independently of the size of a window, for
example if the window is dragged between a regular monitor and a high-DPI one.


@subsection window_scale Window content scale

The content scale for a window can be retrieved with @ref
glfwGetWindowContentScale.

@code
float xscale, yscale;
glfwGetWindowContentScale(window, &xscale, &yscale);
@endcode

The content scale is the ratio between the current DPI and the platform's
default DPI.  This is especially important for text and any UI elements.  If the
pixel dimensions of your UI scaled by this look appropriate on your machine then
it should appear at a reasonable size on other machines regardless of their DPI
and scaling settings.  This relies on the system DPI and scaling settings being
somewhat correct.

On systems where each monitors can have its own content scale, the window
content scale will depend on which monitor the system considers the window to be
on.

If you wish to be notified when the content scale of a window changes, whether
because of a system setting change or because it was moved to a monitor with
a different scale, set a content scale callback.

@code
glfwSetWindowContentScaleCallback(window, window_content_scale_callback);
@endcode

The callback function receives the new content scale of the window.

@code
void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
{
    set_interface_scale(xscale, yscale);
}
@endcode

On platforms where pixels and screen coordinates always map 1:1, the window
will need to be resized to appear the same size when it is moved to a monitor
with a different content scale.  To have this done automatically both when the
window is created and when its content scale later changes, set the @ref
GLFW_SCALE_TO_MONITOR window hint.


@subsection window_sizelimits Window size limits

The minimum and maximum size of the content area of a windowed mode window can
be enforced with @ref glfwSetWindowSizeLimits.  The user may resize the window
to any size and aspect ratio within the specified limits, unless the aspect
ratio is also set.

@code
glfwSetWindowSizeLimits(window, 200, 200, 400, 400);
@endcode

To specify only a minimum size or only a maximum one, set the other pair to
`GLFW_DONT_CARE`.

@code
glfwSetWindowSizeLimits(window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
@endcode

To disable size limits for a window, set them all to `GLFW_DONT_CARE`.

The aspect ratio of the content area of a windowed mode window can be enforced
with @ref glfwSetWindowAspectRatio.  The user may resize the window freely
unless size limits are also set, but the size will be constrained to maintain
the aspect ratio.

@code
glfwSetWindowAspectRatio(window, 16, 9);
@endcode

The aspect ratio is specified as a numerator and denominator, corresponding to
the width and height, respectively.  If you want a window to maintain its
current aspect ratio, use its current size as the ratio.

@code
int width, height;
glfwGetWindowSize(window, &width, &height);
glfwSetWindowAspectRatio(window, width, height);
@endcode

To disable the aspect ratio limit for a window, set both terms to
`GLFW_DONT_CARE`.

You can have both size limits and aspect ratio set for a window, but the results
are undefined if they conflict.


@subsection window_pos Window position

The position of a windowed-mode window can be changed with @ref
glfwSetWindowPos.  This moves the window so that the upper-left corner of its
content area has the specified [screen coordinates](@ref coordinate_systems).
The window system may put limitations on window placement.

@code
glfwSetWindowPos(window, 100, 100);
@endcode

If you wish to be notified when a window is moved, whether by the user, the
system or your own code, set a position callback.

@code
glfwSetWindowPosCallback(window, window_pos_callback);
@endcode

The callback function receives the new position, in screen coordinates, of the
upper-left corner of the content area when the window is moved.

@code
void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
}
@endcode

There is also @ref glfwGetWindowPos for directly retrieving the current position
of the content area of the window.

@code
int xpos, ypos;
glfwGetWindowPos(window, &xpos, &ypos);
@endcode


@subsection window_title Window title

All GLFW windows have a title, although undecorated or full screen windows may
not display it or only display it in a task bar or similar interface.  You can
set a UTF-8 encoded window title with @ref glfwSetWindowTitle.

@code
glfwSetWindowTitle(window, "My Window");
@endcode

The specified string is copied before the function returns, so there is no need
to keep it around.

As long as your source file is encoded as UTF-8, you can use any Unicode
characters directly in the source.

@code
glfwSetWindowTitle(window, "ラストエグザイル");
@endcode

If you are using C++11 or C11, you can use a UTF-8 string literal.

@code
glfwSetWindowTitle(window, u8"This is always a UTF-8 string");
@endcode


@subsection window_icon Window icon

Decorated windows have icons on some platforms.  You can set this icon by
specifying a list of candidate images with @ref glfwSetWindowIcon.

@code
GLFWimage images[2];
images[0] = load_icon("my_icon.png");
images[1] = load_icon("my_icon_small.png");

glfwSetWindowIcon(window, 2, images);
@endcode

The image data is 32-bit, little-endian, non-premultiplied RGBA, i.e. eight bits
per channel with the red channel first.  The pixels are arranged canonically as
sequential rows, starting from the top-left corner.

To revert to the default window icon, pass in an empty image array.

@code
glfwSetWindowIcon(window, 0, NULL);
@endcode


@subsection window_monitor Window monitor

Full screen windows are associated with a specific monitor.  You can get the
handle for this monitor with @ref glfwGetWindowMonitor.

@code
GLFWmonitor* monitor = glfwGetWindowMonitor(window);
@endcode

This monitor handle is one of those returned by @ref glfwGetMonitors.

For windowed mode windows, this function returns `NULL`.  This is how to tell
full screen windows from windowed mode windows.

You can move windows between monitors or between full screen and windowed mode
with @ref glfwSetWindowMonitor.  When making a window full screen on the same or
on a different monitor, specify the desired monitor, resolution and refresh
rate.  The position arguments are ignored.

@code
const GLFWvidmode* mode = glfwGetVideoMode(monitor);

glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
@endcode

When making the window windowed, specify the desired position and size.  The
refresh rate argument is ignored.

@code
glfwSetWindowMonitor(window, NULL, xpos, ypos, width, height, 0);
@endcode

This restores any previous window settings such as whether it is decorated,
floating, resizable, has size or aspect ratio limits, etc..  To restore a window
that was originally windowed to its original size and position, save these
before making it full screen and then pass them in as above.


@subsection window_iconify Window iconification

Windows can be iconified (i.e. minimized) with @ref glfwIconifyWindow.

@code
glfwIconifyWindow(window);
@endcode

When a full screen window is iconified, the original video mode of its monitor
is restored until the user or application restores the window.

Iconified windows can be restored with @ref glfwRestoreWindow.  This function
also restores windows from maximization.

@code
glfwRestoreWindow(window);
@endcode

When a full screen window is restored, the desired video mode is restored to its
monitor as well.

If you wish to be notified when a window is iconified or restored, whether by
the user, system or your own code, set an iconify callback.

@code
glfwSetWindowIconifyCallback(window, window_iconify_callback);
@endcode

The callback function receives changes in the iconification state of the window.

@code
void window_iconify_callback(GLFWwindow* window, int iconified)
{
    if (iconified)
    {
        // The window was iconified
    }
    else
    {
        // The window was restored
    }
}
@endcode

You can also get the current iconification state with @ref glfwGetWindowAttrib.

@code
int iconified = glfwGetWindowAttrib(window, GLFW_ICONIFIED);
@endcode


@subsection window_maximize Window maximization

Windows can be maximized (i.e. zoomed) with @ref glfwMaximizeWindow.

@code
glfwMaximizeWindow(window);
@endcode

Full screen windows cannot be maximized and passing a full screen window to this
function does nothing.

Maximized windows can be restored with @ref glfwRestoreWindow.  This function
also restores windows from iconification.

@code
glfwRestoreWindow(window);
@endcode

If you wish to be notified when a window is maximized or restored, whether by
the user, system or your own code, set a maximize callback.

@code
glfwSetWindowMaximizeCallback(window, window_maximize_callback);
@endcode

The callback function receives changes in the maximization state of the window.

@code
void window_maximize_callback(GLFWwindow* window, int maximized)
{
    if (maximized)
    {
        // The window was maximized
    }
    else
    {
        // The window was restored
    }
}
@endcode

You can also get the current maximization state with @ref glfwGetWindowAttrib.

@code
int maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);
@endcode

By default, newly created windows are not maximized.  You can change this
behavior by setting the [GLFW_MAXIMIZED](@ref GLFW_MAXIMIZED_hint) window hint
before creating the window.

@code
glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
@endcode


@subsection window_hide Window visibility

Windowed mode windows can be hidden with @ref glfwHideWindow.

@code
glfwHideWindow(window);
@endcode

This makes the window completely invisible to the user, including removing it
from the task bar, dock or window list.  Full screen windows cannot be hidden
and calling @ref glfwHideWindow on a full screen window does nothing.

Hidden windows can be shown with @ref glfwShowWindow.

@code
glfwShowWindow(window);
@endcode

By default, this function will also set the input focus to that window. Set
the [GLFW_FOCUS_ON_SHOW](@ref GLFW_FOCUS_ON_SHOW_hint) window hint to change
this behavior for all newly created windows, or change the behavior for an
existing window with @ref glfwSetWindowAttrib.

You can also get the current visibility state with @ref glfwGetWindowAttrib.

@code
int visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);
@endcode

By default, newly created windows are visible.  You can change this behavior by
setting the [GLFW_VISIBLE](@ref GLFW_VISIBLE_hint) window hint before creating
the window.

@code
glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
@endcode

Windows created hidden are completely invisible to the user until shown.  This
can be useful if you need to set up your window further before showing it, for
example moving it to a specific location.


@subsection window_focus Window input focus

Windows can be given input focus and brought to the front with @ref
glfwFocusWindow.

@code
glfwFocusWindow(window);
@endcode

Keep in mind that it can be very disruptive to the user when a window is forced
to the top.  For a less disruptive way of getting the user's attention, see
[attention requests](@ref window_attention).

If you wish to be notified when a window gains or loses input focus, whether by
the user, system or your own code, set a focus callback.

@code
glfwSetWindowFocusCallback(window, window_focus_callback);
@endcode

The callback function receives changes in the input focus state of the window.

@code
void window_focus_callback(GLFWwindow* window, int focused)
{
    if (focused)
    {
        // The window gained input focus
    }
    else
    {
        // The window lost input focus
    }
}
@endcode

You can also get the current input focus state with @ref glfwGetWindowAttrib.

@code
int focused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
@endcode

By default, newly created windows are given input focus.  You can change this
behavior by setting the [GLFW_FOCUSED](@ref GLFW_FOCUSED_hint) window hint
before creating the window.

@code
glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
@endcode


@subsection window_attention Window attention request

If you wish to notify the user of an event without interrupting, you can request
attention with @ref glfwRequestWindowAttention.

@code
glfwRequestWindowAttention(window);
@endcode

The system will highlight the specified window, or on platforms where this is
not supported, the application as a whole.  Once the user has given it
attention, the system will automatically end the request.


@subsection window_refresh Window damage and refresh

If you wish to be notified when the contents of a window is damaged and needs
to be refreshed, set a window refresh callback.

@code
glfwSetWindowRefreshCallback(m_handle, window_refresh_callback);
@endcode

The callback function is called when the contents of the window needs to be
refreshed.

@code
void window_refresh_callback(GLFWwindow* window)
{
    draw_editor_ui(window);
    glfwSwapBuffers(window);
}
@endcode

@note On compositing window systems such as Aero, Compiz or Aqua, where the
window contents are saved off-screen, this callback might only be called when
the window or framebuffer is resized.


@subsection window_transparency Window transparency

GLFW supports two kinds of transparency for windows; framebuffer transparency
and whole window transparency.  A single window may not use both methods.  The
results of doing this are undefined.

Both methods require the platform to support it and not every version of every
platform GLFW supports does this, so there are mechanisms to check whether the
window really is transparent.

Window framebuffers can be made transparent on a per-pixel per-frame basis with
the [GLFW_TRANSPARENT_FRAMEBUFFER](@ref GLFW_TRANSPARENT_FRAMEBUFFER_hint)
window hint.

@code
glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
@endcode

If supported by the system, the window content area will be composited with the
background using the framebuffer per-pixel alpha channel.  This requires desktop
compositing to be enabled on the system.  It does not affect window decorations.

You can check whether the window framebuffer was successfully made transparent
with the
[GLFW_TRANSPARENT_FRAMEBUFFER](@ref GLFW_TRANSPARENT_FRAMEBUFFER_attrib)
window attribute.

@code
if (glfwGetWindowAttrib(window, GLFW_TRANSPARENT_FRAMEBUFFER))
{
    // window framebuffer is currently transparent
}
@endcode

GLFW comes with an example that enabled framebuffer transparency called `gears`.

The opacity of the whole window, including any decorations, can be set with @ref
glfwSetWindowOpacity.

@code
glfwSetWindowOpacity(window, 0.5f);
@endcode

The opacity (or alpha) value is a positive finite number between zero and one,
where 0 (zero) is fully transparent and 1 (one) is fully opaque.  The initial
opacity value for newly created windows is 1.

The current opacity of a window can be queried with @ref glfwGetWindowOpacity.

@code
float opacity = glfwGetWindowOpacity(window);
@endcode

If the system does not support whole window transparency, this function always
returns one.

GLFW comes with a test program that lets you control whole window transparency
at run-time called `opacity`.


@subsection window_attribs Window attributes

Windows have a number of attributes that can be returned using @ref
glfwGetWindowAttrib.  Some reflect state that may change as a result of user
interaction, (e.g. whether it has input focus), while others reflect inherent
properties of the window (e.g. what kind of border it has).  Some are related to
the window and others to its OpenGL or OpenGL ES context.

@code
if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
{
    // window has input focus
}
@endcode

The [GLFW_DECORATED](@ref GLFW_DECORATED_attrib),
[GLFW_RESIZABLE](@ref GLFW_RESIZABLE_attrib),
[GLFW_FLOATING](@ref GLFW_FLOATING_attrib),
[GLFW_AUTO_ICONIFY](@ref GLFW_AUTO_ICONIFY_attrib) and
[GLFW_FOCUS_ON_SHOW](@ref GLFW_FOCUS_ON_SHOW_attrib) window attributes can be
changed with @ref glfwSetWindowAttrib.

@code
glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
@endcode



@subsubsection window_attribs_wnd Window related attributes

@anchor GLFW_FOCUSED_attrib
__GLFW_FOCUSED__ indicates whether the specified window has input focus.  See
@ref window_focus for details.

@anchor GLFW_ICONIFIED_attrib
__GLFW_ICONIFIED__ indicates whether the specified window is iconified.
See @ref window_iconify for details.

@anchor GLFW_MAXIMIZED_attrib
__GLFW_MAXIMIZED__ indicates whether the specified window is maximized.  See
@ref window_maximize for details.

@anchor GLFW_HOVERED_attrib
__GLFW_HOVERED__ indicates whether the cursor is currently directly over the
content area of the window, with no other windows between.  See @ref
cursor_enter for details.

@anchor GLFW_VISIBLE_attrib
__GLFW_VISIBLE__ indicates whether the specified window is visible.  See @ref
window_hide for details.

@anchor GLFW_RESIZABLE_attrib
__GLFW_RESIZABLE__ indicates whether the specified window is resizable _by the
user_.  This can be set before creation with the
[GLFW_RESIZABLE](@ref GLFW_RESIZABLE_hint) window hint or after with @ref
glfwSetWindowAttrib.

@anchor GLFW_DECORATED_attrib
__GLFW_DECORATED__ indicates whether the specified window has decorations such
as a border, a close widget, etc.  This can be set before creation with the
[GLFW_DECORATED](@ref GLFW_DECORATED_hint) window hint or after with @ref
glfwSetWindowAttrib.

@anchor GLFW_AUTO_ICONIFY_attrib
__GLFW_AUTO_ICONIFY__ indicates whether the specified full screen window is
iconified on focus loss, a close widget, etc.  This can be set before creation
with the [GLFW_AUTO_ICONIFY](@ref GLFW_AUTO_ICONIFY_hint) window hint or after
with @ref glfwSetWindowAttrib.

@anchor GLFW_FLOATING_attrib
__GLFW_FLOATING__ indicates whether the specified window is floating, also
called topmost or always-on-top.  This can be set before creation with the
[GLFW_FLOATING](@ref GLFW_FLOATING_hint) window hint or after with @ref
glfwSetWindowAttrib.

@anchor GLFW_TRANSPARENT_FRAMEBUFFER_attrib
__GLFW_TRANSPARENT_FRAMEBUFFER__ indicates whether the specified window has
a transparent framebuffer, i.e. the window contents is composited with the
background using the window framebuffer alpha channel.  See @ref
window_transparency for details.

@anchor GLFW_FOCUS_ON_SHOW_attrib
__GLFW_FOCUS_ON_SHOW__ specifies whether the window will be given input
focus when @ref glfwShowWindow is called. This can be set before creation
with the [GLFW_FOCUS_ON_SHOW](@ref GLFW_FOCUS_ON_SHOW_hint) window hint or
after with @ref glfwSetWindowAttrib.

@subsubsection window_attribs_ctx Context related attributes

@anchor GLFW_CLIENT_API_attrib
__GLFW_CLIENT_API__ indicates the client API provided by the window's context;
either `GLFW_OPENGL_API`, `GLFW_OPENGL_ES_API` or `GLFW_NO_API`.

@anchor GLFW_CONTEXT_CREATION_API_attrib
__GLFW_CONTEXT_CREATION_API__ indicates the context creation API used to create
the window's context; either `GLFW_NATIVE_CONTEXT_API`, `GLFW_EGL_CONTEXT_API`
or `GLFW_OSMESA_CONTEXT_API`.

@anchor GLFW_CONTEXT_VERSION_MAJOR_attrib
@anchor GLFW_CONTEXT_VERSION_MINOR_attrib
@anchor GLFW_CONTEXT_REVISION_attrib
__GLFW_CONTEXT_VERSION_MAJOR__, __GLFW_CONTEXT_VERSION_MINOR__ and
__GLFW_CONTEXT_REVISION__ indicate the client API version of the window's
context.

@note Do not confuse these attributes with `GLFW_VERSION_MAJOR`,
`GLFW_VERSION_MINOR` and `GLFW_VERSION_REVISION` which provide the API version
of the GLFW header.

@anchor GLFW_OPENGL_FORWARD_COMPAT_attrib
__GLFW_OPENGL_FORWARD_COMPAT__ is `GLFW_TRUE` if the window's context is an
OpenGL forward-compatible one, or `GLFW_FALSE` otherwise.

@anchor GLFW_OPENGL_DEBUG_CONTEXT_attrib
__GLFW_OPENGL_DEBUG_CONTEXT__ is `GLFW_TRUE` if the window's context is in debug
mode, or `GLFW_FALSE` otherwise.

@anchor GLFW_OPENGL_PROFILE_attrib
__GLFW_OPENGL_PROFILE__ indicates the OpenGL profile used by the context.  This
is `GLFW_OPENGL_CORE_PROFILE` or `GLFW_OPENGL_COMPAT_PROFILE` if the context
uses a known profile, or `GLFW_OPENGL_ANY_PROFILE` if the OpenGL profile is
unknown or the context is an OpenGL ES context.  Note that the returned profile
may not match the profile bits of the context flags, as GLFW will try other
means of detecting the profile when no bits are set.

@anchor GLFW_CONTEXT_RELEASE_BEHAVIOR_attrib
__GLFW_CONTEXT_RELEASE_BEHAVIOR__ indicates the release used by the context.
Possible values are one of `GLFW_ANY_RELEASE_BEHAVIOR`,
`GLFW_RELEASE_BEHAVIOR_FLUSH` or `GLFW_RELEASE_BEHAVIOR_NONE`.  If the
behavior is `GLFW_ANY_RELEASE_BEHAVIOR`, the default behavior of the context
creation API will be used.  If the behavior is `GLFW_RELEASE_BEHAVIOR_FLUSH`,
the pipeline will be flushed whenever the context is released from being the
current one.  If the behavior is `GLFW_RELEASE_BEHAVIOR_NONE`, the pipeline will
not be flushed on release.

@anchor GLFW_CONTEXT_NO_ERROR_attrib
__GLFW_CONTEXT_NO_ERROR__ indicates whether errors are generated by the context.
Possible values are `GLFW_TRUE` and `GLFW_FALSE`.  If enabled, situations that
would have generated errors instead cause undefined behavior.

@anchor GLFW_CONTEXT_ROBUSTNESS_attrib
__GLFW_CONTEXT_ROBUSTNESS__ indicates the robustness strategy used by the
context.  This is `GLFW_LOSE_CONTEXT_ON_RESET` or `GLFW_NO_RESET_NOTIFICATION`
if the window's context supports robustness, or `GLFW_NO_ROBUSTNESS` otherwise.


@subsubsection window_attribs_fb Framebuffer related attributes

GLFW does not expose attributes of the default framebuffer (i.e. the framebuffer
attached to the window) as these can be queried directly with either OpenGL,
OpenGL ES or Vulkan.

If you are using version 3.0 or later of OpenGL or OpenGL ES, the
`glGetFramebufferAttachmentParameteriv` function can be used to retrieve the
number of bits for the red, green, blue, alpha, depth and stencil buffer
channels.  Otherwise, the `glGetIntegerv` function can be used.

The number of MSAA samples are always retrieved with `glGetIntegerv`.  For
contexts supporting framebuffer objects, the number of samples of the currently
bound framebuffer is returned.

Attribute    | glGetIntegerv     | glGetFramebufferAttachmentParameteriv
------------ | ----------------- | -------------------------------------
Red bits     | `GL_RED_BITS`     | `GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE`
Green bits   | `GL_GREEN_BITS`   | `GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE`
Blue bits    | `GL_BLUE_BITS`    | `GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE`
Alpha bits   | `GL_ALPHA_BITS`   | `GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE`
Depth bits   | `GL_DEPTH_BITS`   | `GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE`
Stencil bits | `GL_STENCIL_BITS` | `GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE`
MSAA samples | `GL_SAMPLES`      | _Not provided by this function_

When calling `glGetFramebufferAttachmentParameteriv`, the red, green, blue and
alpha sizes are queried from the `GL_BACK_LEFT`, while the depth and stencil
sizes are queried from the `GL_DEPTH` and `GL_STENCIL` attachments,
respectively.


@section buffer_swap Buffer swapping

GLFW windows are by default double buffered.  That means that you have two
rendering buffers; a front buffer and a back buffer.  The front buffer is
the one being displayed and the back buffer the one you render to.

When the entire frame has been rendered, it is time to swap the back and the
front buffers in order to display what has been rendered and begin rendering
a new frame.  This is done with @ref glfwSwapBuffers.

@code
glfwSwapBuffers(window);
@endcode

Sometimes it can be useful to select when the buffer swap will occur.  With the
function @ref glfwSwapInterval it is possible to select the minimum number of
monitor refreshes the driver should wait from the time @ref glfwSwapBuffers was
called before swapping the buffers:

@code
glfwSwapInterval(1);
@endcode

If the interval is zero, the swap will take place immediately when @ref
glfwSwapBuffers is called without waiting for a refresh.  Otherwise at least
interval retraces will pass between each buffer swap.  Using a swap interval of
zero can be useful for benchmarking purposes, when it is not desirable to
measure the time it takes to wait for the vertical retrace.  However, a swap
interval of one lets you avoid tearing.

Note that this may not work on all machines, as some drivers have
user-controlled settings that override any swap interval the application
requests.

A context that supports either the `WGL_EXT_swap_control_tear` or the
`GLX_EXT_swap_control_tear` extension also accepts _negative_ swap intervals,
which allows the driver to swap immediately even if a frame arrives a little bit
late.  This trades the risk of visible tears for greater framerate stability.
You can check for these extensions with @ref glfwExtensionSupported.

*/
