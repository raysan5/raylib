/*!

@page compat_guide Standards conformance

@tableofcontents

This guide describes the various API extensions used by this version of GLFW.
It lists what are essentially implementation details, but which are nonetheless
vital knowledge for developers intending to deploy their applications on a wide
range of machines.

The information in this guide is not a part of GLFW API, but merely
preconditions for some parts of the library to function on a given machine.  Any
part of this information may change in future versions of GLFW and that will not
be considered a breaking API change.


@section compat_x11 X11 extensions, protocols and IPC standards

As GLFW uses Xlib directly, without any intervening toolkit
library, it has sole responsibility for interacting well with the many and
varied window managers in use on Unix-like systems.  In order for applications
and window managers to work well together, a number of standards and
conventions have been developed that regulate behavior outside the scope of the
X11 API; most importantly the
[Inter-Client Communication Conventions Manual](https://www.tronche.com/gui/x/icccm/)
(ICCCM) and
[Extended Window Manager Hints](https://standards.freedesktop.org/wm-spec/wm-spec-latest.html)
(EWMH) standards.

GLFW uses the `_MOTIF_WM_HINTS` window property to support borderless windows.
If the running window manager does not support this property, the
`GLFW_DECORATED` hint will have no effect.

GLFW uses the ICCCM `WM_DELETE_WINDOW` protocol to intercept the user
attempting to close the GLFW window.  If the running window manager does not
support this protocol, the close callback will never be called.

GLFW uses the EWMH `_NET_WM_PING` protocol, allowing the window manager notify
the user when the application has stopped responding, i.e. when it has ceased to
process events.  If the running window manager does not support this protocol,
the user will not be notified if the application locks up.

GLFW uses the EWMH `_NET_WM_STATE_FULLSCREEN` window state to tell the window
manager to make the GLFW window full screen.  If the running window manager does
not support this state, full screen windows may not work properly.  GLFW has
a fallback code path in case this state is unavailable, but every window manager
behaves slightly differently in this regard.

GLFW uses the EWMH `_NET_WM_BYPASS_COMPOSITOR` window property to tell a
compositing window manager to un-redirect full screen GLFW windows.  If the
running window manager uses compositing but does not support this property then
additional copying may be performed for each buffer swap of full screen windows.

GLFW uses the
[clipboard manager protocol](https://www.freedesktop.org/wiki/ClipboardManager/)
to push a clipboard string (i.e. selection) owned by a GLFW window about to be
destroyed to the clipboard manager.  If there is no running clipboard manager,
the clipboard string will be unavailable once the window has been destroyed.

GLFW uses the
[X drag-and-drop protocol](https://www.freedesktop.org/wiki/Specifications/XDND/)
to provide file drop events.  If the application originating the drag does not
support this protocol, drag and drop will not work.

GLFW uses the XRandR 1.3 extension to provide multi-monitor support.  If the
running X server does not support this version of this extension, multi-monitor
support will not function and only a single, desktop-spanning monitor will be
reported.

GLFW uses the XRandR 1.3 and Xf86vidmode extensions to provide gamma ramp
support.  If the running X server does not support either or both of these
extensions, gamma ramp support will not function.

GLFW uses the Xkb extension and detectable auto-repeat to provide keyboard
input.  If the running X server does not support this extension, a non-Xkb
fallback path is used.

GLFW uses the XInput2 extension to provide raw, non-accelerated mouse motion
when the cursor is disabled.  If the running X server does not support this
extension, regular accelerated mouse motion will be used.

GLFW uses both the XRender extension and the compositing manager to support
transparent window framebuffers.  If the running X server does not support this
extension or there is no running compositing manager, the
`GLFW_TRANSPARENT_FRAMEBUFFER` framebuffer hint will have no effect.


@section compat_wayland Wayland protocols and IPC standards

As GLFW uses libwayland directly, without any intervening toolkit library, it
has sole responsibility for interacting well with every compositor in use on
Unix-like systems.  Most of the features are provided by the core protocol,
while cursor support is provided by the libwayland-cursor helper library, EGL
integration by libwayland-egl, and keyboard handling by
[libxkbcommon](https://xkbcommon.org/).  In addition, GLFW uses some protocols
from wayland-protocols to provide additional features if the compositor
supports them.

GLFW uses xkbcommon 0.5.0 to provide compose key support.  When it has been
built against an older xkbcommon, the compose key will be disabled even if it
has been configured in the compositor.

GLFW uses the [xdg-shell
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/stable/xdg-shell/xdg-shell.xml)
to provide better window management.  This protocol is part of
wayland-protocols 1.12, and mandatory at build time.

GLFW uses the [relative pointer
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/relative-pointer/relative-pointer-unstable-v1.xml)
alongside the [pointer constraints
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml)
to implement disabled cursor.  These two protocols are part of
wayland-protocols 1.1, and mandatory at build time.  If the running compositor
does not support both of these protocols, disabling the cursor will have no
effect.

GLFW uses the [idle inhibit
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml)
to prohibit the screensaver from starting.  This protocol is part of
wayland-protocols 1.6, and mandatory at build time.  If the running compositor
does not support this protocol, the screensaver may start even for full screen
windows.

GLFW uses the [libdecor library](https://gitlab.freedesktop.org/libdecor/libdecor)
for window decorations, where available.  This in turn provides good quality
client-side decorations (drawn by the application) on desktop systems that do
not support server-side decorations (drawn by the window manager).  On systems
that do not provide either libdecor or xdg-decoration, very basic window
decorations are provided.  These do not include the window title or any caption
buttons.

GLFW uses the [xdg-decoration
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml)
to request decorations to be drawn around its windows.  This protocol is part
of wayland-protocols 1.15, and mandatory at build time.  If the running
compositor does not support this protocol, a very simple frame will be drawn by
GLFW itself, using the [viewporter
protocol](https://cgit.freedesktop.org/wayland/wayland-protocols/tree/stable/viewporter/viewporter.xml)
alongside
[subsurfaces](https://cgit.freedesktop.org/wayland/wayland/tree/protocol/wayland.xml#n2598).
This protocol is part of wayland-protocols 1.4, and mandatory at build time.
If the running compositor does not support this protocol either, no decorations
will be drawn around windows.


@section compat_glx GLX extensions

The GLX API is the default API used to create OpenGL contexts on Unix-like
systems using the X Window System.

GLFW uses the GLX 1.3 `GLXFBConfig` functions to enumerate and select framebuffer pixel
formats.  If GLX 1.3 is not supported, @ref glfwInit will fail.

GLFW uses the `GLX_MESA_swap_control,` `GLX_EXT_swap_control` and
`GLX_SGI_swap_control` extensions to provide vertical retrace synchronization
(or _vsync_), in that order of preference.  When none of these extensions are
available, calling @ref glfwSwapInterval will have no effect.

GLFW uses the `GLX_ARB_multisample` extension to create contexts with
multisampling anti-aliasing.  Where this extension is unavailable, the
`GLFW_SAMPLES` hint will have no effect.

GLFW uses the `GLX_ARB_create_context` extension when available, even when
creating OpenGL contexts of version 2.1 and below.  Where this extension is
unavailable, the `GLFW_CONTEXT_VERSION_MAJOR` and `GLFW_CONTEXT_VERSION_MINOR`
hints will only be partially supported, the `GLFW_OPENGL_DEBUG_CONTEXT` hint
will have no effect, and setting the `GLFW_OPENGL_PROFILE` or
`GLFW_OPENGL_FORWARD_COMPAT` hints to `GLFW_TRUE` will cause @ref
glfwCreateWindow to fail.

GLFW uses the `GLX_ARB_create_context_profile` extension to provide support for
context profiles.  Where this extension is unavailable, setting the
`GLFW_OPENGL_PROFILE` hint to anything but `GLFW_OPENGL_ANY_PROFILE`, or setting
`GLFW_CLIENT_API` to anything but `GLFW_OPENGL_API` or `GLFW_NO_API` will cause
@ref glfwCreateWindow to fail.

GLFW uses the `GLX_ARB_context_flush_control` extension to provide control over
whether a context is flushed when it is released (made non-current).  Where this
extension is unavailable, the `GLFW_CONTEXT_RELEASE_BEHAVIOR` hint will have no
effect and the context will always be flushed when released.

GLFW uses the `GLX_ARB_framebuffer_sRGB` and `GLX_EXT_framebuffer_sRGB`
extensions to provide support for sRGB framebuffers.  Where both of these
extensions are unavailable, the `GLFW_SRGB_CAPABLE` hint will have no effect.


@section compat_wgl WGL extensions

The WGL API is used to create OpenGL contexts on Microsoft Windows and other
implementations of the Win32 API, such as Wine.

GLFW uses either the `WGL_EXT_extension_string` or the
`WGL_ARB_extension_string` extension to check for the presence of all other WGL
extensions listed below.  If both are available, the EXT one is preferred.  If
neither is available, no other extensions are used and many GLFW features
related to context creation will have no effect or cause errors when used.

GLFW uses the `WGL_EXT_swap_control` extension to provide vertical retrace
synchronization (or _vsync_).  Where this extension is unavailable, calling @ref
glfwSwapInterval will have no effect.

GLFW uses the `WGL_ARB_pixel_format` and `WGL_ARB_multisample` extensions to
create contexts with multisampling anti-aliasing.  Where these extensions are
unavailable, the `GLFW_SAMPLES` hint will have no effect.

GLFW uses the `WGL_ARB_create_context` extension when available, even when
creating OpenGL contexts of version 2.1 and below.  Where this extension is
unavailable, the `GLFW_CONTEXT_VERSION_MAJOR` and `GLFW_CONTEXT_VERSION_MINOR`
hints will only be partially supported, the `GLFW_OPENGL_DEBUG_CONTEXT` hint
will have no effect, and setting the `GLFW_OPENGL_PROFILE` or
`GLFW_OPENGL_FORWARD_COMPAT` hints to `GLFW_TRUE` will cause @ref
glfwCreateWindow to fail.

GLFW uses the `WGL_ARB_create_context_profile` extension to provide support for
context profiles.  Where this extension is unavailable, setting the
`GLFW_OPENGL_PROFILE` hint to anything but `GLFW_OPENGL_ANY_PROFILE` will cause
@ref glfwCreateWindow to fail.

GLFW uses the `WGL_ARB_context_flush_control` extension to provide control over
whether a context is flushed when it is released (made non-current).  Where this
extension is unavailable, the `GLFW_CONTEXT_RELEASE_BEHAVIOR` hint will have no
effect and the context will always be flushed when released.

GLFW uses the `WGL_ARB_framebuffer_sRGB` and `WGL_EXT_framebuffer_sRGB`
extensions to provide support for sRGB framebuffers.  When both of these
extensions are unavailable, the `GLFW_SRGB_CAPABLE` hint will have no effect.


@section compat_osx OpenGL on macOS

Support for OpenGL 3.2 and above was introduced with OS X 10.7 and even then
only forward-compatible, core profile contexts are supported.  Support for
OpenGL 4.1 was introduced with OS X 10.9, also limited to forward-compatible,
core profile contexts.  There is also still no mechanism for requesting debug
contexts or no-error contexts.  Versions of Mac OS X earlier than 10.7 support
at most OpenGL version 2.1.

Because of this, on OS X 10.7 and later, the `GLFW_CONTEXT_VERSION_MAJOR` and
`GLFW_CONTEXT_VERSION_MINOR` hints will cause @ref glfwCreateWindow to fail if
given version 3.0 or 3.1.  The `GLFW_OPENGL_FORWARD_COMPAT` hint must be set to
`GLFW_TRUE` and the `GLFW_OPENGL_PROFILE` hint must be set to
`GLFW_OPENGL_CORE_PROFILE` when creating OpenGL 3.2 and later contexts.  The
`GLFW_OPENGL_DEBUG_CONTEXT` and `GLFW_CONTEXT_NO_ERROR` hints are ignored.

Also, on Mac OS X 10.6 and below, the `GLFW_CONTEXT_VERSION_MAJOR` and
`GLFW_CONTEXT_VERSION_MINOR` hints will fail if given a version above 2.1,
setting the `GLFW_OPENGL_PROFILE` or `GLFW_OPENGL_FORWARD_COMPAT` hints to
a non-default value will cause @ref glfwCreateWindow to fail and the
`GLFW_OPENGL_DEBUG_CONTEXT` hint is ignored.


@section compat_vulkan Vulkan loader and API

By default, GLFW uses the standard system-wide Vulkan loader to access the
Vulkan API on all platforms except macOS.  This is installed by both graphics
drivers and Vulkan SDKs.  If either the loader or at least one minimally
functional ICD is missing, @ref glfwVulkanSupported will return `GLFW_FALSE` and
all other Vulkan-related functions will fail with an @ref GLFW_API_UNAVAILABLE
error.


@section compat_wsi Vulkan WSI extensions

The Vulkan WSI extensions are used to create Vulkan surfaces for GLFW windows on
all supported platforms.

GLFW uses the `VK_KHR_surface` and `VK_KHR_win32_surface` extensions to create
surfaces on Microsoft Windows.  If any of these extensions are not available,
@ref glfwGetRequiredInstanceExtensions will return an empty list and window
surface creation will fail.

GLFW uses the `VK_KHR_surface` and either the `VK_MVK_macos_surface` or
`VK_EXT_metal_surface` extensions to create surfaces on macOS.  If any of these
extensions are not available, @ref glfwGetRequiredInstanceExtensions will
return an empty list and window surface creation will fail.

GLFW uses the `VK_KHR_surface` and either the `VK_KHR_xlib_surface` or
`VK_KHR_xcb_surface` extensions to create surfaces on X11.  If `VK_KHR_surface`
or both `VK_KHR_xlib_surface` and `VK_KHR_xcb_surface` are not available, @ref
glfwGetRequiredInstanceExtensions will return an empty list and window surface
creation will fail.

GLFW uses the `VK_KHR_surface` and `VK_KHR_wayland_surface` extensions to create
surfaces on Wayland.  If any of these extensions are not available, @ref
glfwGetRequiredInstanceExtensions will return an empty list and window surface
creation will fail.

*/
