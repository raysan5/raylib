/*!

@page context_guide Context guide

@tableofcontents

This guide introduces the OpenGL and OpenGL ES context related functions of
GLFW.  For details on a specific function in this category, see the @ref
context.  There are also guides for the other areas of the GLFW API.

 - @ref intro_guide
 - @ref window_guide
 - @ref vulkan_guide
 - @ref monitor_guide
 - @ref input_guide


@section context_object Context objects

A window object encapsulates both a top-level window and an OpenGL or OpenGL ES
context.  It is created with @ref glfwCreateWindow and destroyed with @ref
glfwDestroyWindow or @ref glfwTerminate.  See @ref window_creation for more
information.

As the window and context are inseparably linked, the window object also serves
as the context handle.

To test the creation of various kinds of contexts and see their properties, run
the `glfwinfo` test program.

@note Vulkan does not have a context and the Vulkan instance is created via the
Vulkan API itself.  If you will be using Vulkan to render to a window, disable
context creation by setting the [GLFW_CLIENT_API](@ref GLFW_CLIENT_API_hint)
hint to `GLFW_NO_API`.  For more information, see the @ref vulkan_guide.


@subsection context_hints Context creation hints

There are a number of hints, specified using @ref glfwWindowHint, related to
what kind of context is created.  See
[context related hints](@ref window_hints_ctx) in the window guide.


@subsection context_sharing Context object sharing

When creating a window and its OpenGL or OpenGL ES context with @ref
glfwCreateWindow, you can specify another window whose context the new one
should share its objects (textures, vertex and element buffers, etc.) with.

@code
GLFWwindow* second_window = glfwCreateWindow(640, 480, "Second Window", NULL, first_window);
@endcode

Object sharing is implemented by the operating system and graphics driver.  On
platforms where it is possible to choose which types of objects are shared, GLFW
requests that all types are shared.

See the relevant chapter of the [OpenGL](https://www.opengl.org/registry/) or
[OpenGL ES](https://www.khronos.org/opengles/) reference documents for more
information.  The name and number of this chapter unfortunately varies between
versions and APIs, but has at times been named _Shared Objects and Multiple
Contexts_.

GLFW comes with a bare-bones object sharing example program called `sharing`.


@subsection context_offscreen Offscreen contexts

GLFW doesn't support creating contexts without an associated window.  However,
contexts with hidden windows can be created with the
[GLFW_VISIBLE](@ref GLFW_VISIBLE_hint) window hint.

@code
glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

GLFWwindow* offscreen_context = glfwCreateWindow(640, 480, "", NULL, NULL);
@endcode

The window never needs to be shown and its context can be used as a plain
offscreen context.  Depending on the window manager, the size of a hidden
window's framebuffer may not be usable or modifiable, so framebuffer
objects are recommended for rendering with such contexts.

You should still [process events](@ref events) as long as you have at least one
window, even if none of them are visible.

@macos The first time a window is created the menu bar is created.  This is not
desirable for example when writing a command-line only application.  Menu bar
creation can be disabled with the @ref GLFW_COCOA_MENUBAR init hint.


@subsection context_less Windows without contexts

You can disable context creation by setting the
[GLFW_CLIENT_API](@ref GLFW_CLIENT_API_hint) hint to `GLFW_NO_API`.

Windows without contexts should not be passed to @ref glfwMakeContextCurrent or
@ref glfwSwapBuffers.  Doing this generates a @ref GLFW_NO_WINDOW_CONTEXT error.


@section context_current Current context

Before you can make OpenGL or OpenGL ES calls, you need to have a current
context of the correct type.  A context can only be current for a single thread
at a time, and a thread can only have a single context current at a time.

When moving a context between threads, you must make it non-current on the old
thread before making it current on the new one.

The context of a window is made current with @ref glfwMakeContextCurrent.

@code
glfwMakeContextCurrent(window);
@endcode

The window of the current context is returned by @ref glfwGetCurrentContext.

@code
GLFWwindow* window = glfwGetCurrentContext();
@endcode

The following GLFW functions require a context to be current.  Calling any these
functions without a current context will generate a @ref GLFW_NO_CURRENT_CONTEXT
error.

 - @ref glfwSwapInterval
 - @ref glfwExtensionSupported
 - @ref glfwGetProcAddress


@section context_swap Buffer swapping

See @ref buffer_swap in the window guide.


@section context_glext OpenGL and OpenGL ES extensions

One of the benefits of OpenGL and OpenGL ES is their extensibility.
Hardware vendors may include extensions in their implementations that extend the
API before that functionality is included in a new version of the OpenGL or
OpenGL ES specification, and some extensions are never included and remain
as extensions until they become obsolete.

An extension is defined by:

- An extension name (e.g. `GL_ARB_gl_spirv`)
- New OpenGL tokens (e.g. `GL_SPIR_V_BINARY_ARB`)
- New OpenGL functions (e.g. `glSpecializeShaderARB`)

Note the `ARB` affix, which stands for Architecture Review Board and is used
for official extensions.  The extension above was created by the ARB, but there
are many different affixes, like `NV` for Nvidia and `AMD` for, well, AMD.  Any
group may also use the generic `EXT` affix.  Lists of extensions, together with
their specifications, can be found at the
[OpenGL Registry](https://www.opengl.org/registry/) and
[OpenGL ES Registry](https://www.khronos.org/registry/gles/).


@subsection context_glext_auto Loading extension with a loader library

An extension loader library is the easiest and best way to access both OpenGL and
OpenGL ES extensions and modern versions of the core OpenGL or OpenGL ES APIs.
They will take care of all the details of declaring and loading everything you
need.  One such library is [glad](https://github.com/Dav1dde/glad) and there are
several others.

The following example will use glad but all extension loader libraries work
similarly.

First you need to generate the source files using the glad Python script.  This
example generates a loader for any version of OpenGL, which is the default for
both GLFW and glad, but loaders for OpenGL ES, as well as loaders for specific
API versions and extension sets can be generated.  The generated files are
written to the `output` directory.

@code{.sh}
python main.py --generator c --no-loader --out-path output
@endcode

The `--no-loader` option is added because GLFW already provides a function for
loading OpenGL and OpenGL ES function pointers, one that automatically uses the
selected context creation API, and glad can call this instead of having to
implement its own.  There are several other command-line options as well.  See
the glad documentation for details.

Add the generated `output/src/glad.c`, `output/include/glad/glad.h` and
`output/include/KHR/khrplatform.h` files to your build.  Then you need to
include the glad header file, which will replace the OpenGL header of your
development environment.  By including the glad header before the GLFW header,
it suppresses the development environment's OpenGL or OpenGL ES header.

@code
#include <glad/glad.h>
#include <GLFW/glfw3.h>
@endcode

Finally, you need to initialize glad once you have a suitable current context.

@code
window = glfwCreateWindow(640, 480, "My Window", NULL, NULL);
if (!window)
{
    ...
}

glfwMakeContextCurrent(window);

gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
@endcode

Once glad has been loaded, you have access to all OpenGL core and extension
functions supported by both the context you created and the glad loader you
generated. After that, you are ready to start rendering.

You can specify a minimum required OpenGL or OpenGL ES version with
[context hints](@ref window_hints_ctx).  If your needs are more complex, you can
check the actual OpenGL or OpenGL ES version with
[context attributes](@ref window_attribs_ctx), or you can check whether
a specific version is supported by the current context with the
`GLAD_GL_VERSION_x_x` booleans.

@code
if (GLAD_GL_VERSION_3_2)
{
    // Call OpenGL 3.2+ specific code
}
@endcode

To check whether a specific extension is supported, use the `GLAD_GL_xxx`
booleans.

@code
if (GLAD_GL_ARB_gl_spirv)
{
    // Use GL_ARB_gl_spirv
}
@endcode


@subsection context_glext_manual Loading extensions manually

__Do not use this technique__ unless it is absolutely necessary.  An
[extension loader library](@ref context_glext_auto) will save you a ton of
tedious, repetitive, error prone work.

To use a certain extension, you must first check whether the context supports
that extension and then, if it introduces new functions, retrieve the pointers
to those functions.  GLFW provides @ref glfwExtensionSupported and @ref
glfwGetProcAddress for manual loading of extensions and new API functions.

This section will demonstrate manual loading of OpenGL extensions.  The loading
of OpenGL ES extensions is identical except for the name of the extension header.


@subsubsection context_glext_header The glext.h header

The `glext.h` extension header is a continually updated file that defines the
interfaces for all OpenGL extensions.  The latest version of this can always be
found at the [OpenGL Registry](https://www.opengl.org/registry/).  There are also
extension headers for the various versions of OpenGL ES at the
[OpenGL ES Registry](https://www.khronos.org/registry/gles/).  It it strongly
recommended that you use your own copy of the extension header, as the one
included in your development environment may be several years out of date and
may not include the extensions you wish to use.

The header defines function pointer types for all functions of all extensions it
supports.  These have names like `PFNGLSPECIALIZESHADERARBPROC` (for
`glSpecializeShaderARB`), i.e. the name is made uppercase and `PFN` (pointer
to function) and `PROC` (procedure) are added to the ends.

To include the extension header, define @ref GLFW_INCLUDE_GLEXT before including
the GLFW header.

@code
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
@endcode


@subsubsection context_glext_string Checking for extensions

A given machine may not actually support the extension (it may have older
drivers or a graphics card that lacks the necessary hardware features), so it
is necessary to check at run-time whether the context supports the extension.
This is done with @ref glfwExtensionSupported.

@code
if (glfwExtensionSupported("GL_ARB_gl_spirv"))
{
    // The extension is supported by the current context
}
@endcode

The argument is a null terminated ASCII string with the extension name.  If the
extension is supported, @ref glfwExtensionSupported returns `GLFW_TRUE`,
otherwise it returns `GLFW_FALSE`.


@subsubsection context_glext_proc Fetching function pointers

Many extensions, though not all, require the use of new OpenGL functions.
These functions often do not have entry points in the client API libraries of
your operating system, making it necessary to fetch them at run time.  You can
retrieve pointers to these functions with @ref glfwGetProcAddress.

@code
PFNGLSPECIALIZESHADERARBPROC pfnSpecializeShaderARB = glfwGetProcAddress("glSpecializeShaderARB");
@endcode

In general, you should avoid giving the function pointer variables the (exact)
same name as the function, as this may confuse your linker.  Instead, you can
use a different prefix, like above, or some other naming scheme.

Now that all the pieces have been introduced, here is what they might look like
when used together.

@code
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#define glSpecializeShaderARB pfnSpecializeShaderARB
PFNGLSPECIALIZESHADERARBPROC pfnSpecializeShaderARB;

// Flag indicating whether the extension is supported
int has_ARB_gl_spirv = 0;

void load_extensions(void)
{
    if (glfwExtensionSupported("GL_ARB_gl_spirv"))
    {
        pfnSpecializeShaderARB = (PFNGLSPECIALIZESHADERARBPROC)
            glfwGetProcAddress("glSpecializeShaderARB");
        has_ARB_gl_spirv = 1;
    }
}

void some_function(void)
{
    if (has_ARB_gl_spirv)
    {
        // Now the extension function can be called as usual
        glSpecializeShaderARB(...);
    }
}
@endcode

*/
