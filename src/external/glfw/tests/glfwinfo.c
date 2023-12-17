//========================================================================
// Context creation and information tool
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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

#include <glad/gl.h>
#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

#ifdef _MSC_VER
#define strcasecmp(x, y) _stricmp(x, y)
#endif

#define API_NAME_OPENGL     "gl"
#define API_NAME_OPENGL_ES  "es"

#define API_NAME_NATIVE     "native"
#define API_NAME_EGL        "egl"
#define API_NAME_OSMESA     "osmesa"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE  "none"
#define STRATEGY_NAME_LOSE  "lose"

#define BEHAVIOR_NAME_NONE  "none"
#define BEHAVIOR_NAME_FLUSH "flush"

static void usage(void)
{
    printf("Usage: glfwinfo [OPTION]...\n");
    printf("Options:\n");
    printf("  -a, --client-api=API      the client API to use ("
                                        API_NAME_OPENGL " or "
                                        API_NAME_OPENGL_ES ")\n");
    printf("  -b, --behavior=BEHAVIOR   the release behavior to use ("
                                        BEHAVIOR_NAME_NONE " or "
                                        BEHAVIOR_NAME_FLUSH ")\n");
    printf("  -c, --context-api=API     the context creation API to use ("
                                        API_NAME_NATIVE " or "
                                        API_NAME_EGL " or "
                                        API_NAME_OSMESA ")\n");
    printf("  -d, --debug               request a debug context\n");
    printf("  -f, --forward             require a forward-compatible context\n");
    printf("  -h, --help                show this help\n");
    printf("  -l, --list-extensions     list all Vulkan and client API extensions\n");
    printf("      --list-layers         list all Vulkan layers\n");
    printf("  -m, --major=MAJOR         the major number of the required "
                                        "client API version\n");
    printf("  -n, --minor=MINOR         the minor number of the required "
                                        "client API version\n");
    printf("  -p, --profile=PROFILE     the OpenGL profile to use ("
                                        PROFILE_NAME_CORE " or "
                                        PROFILE_NAME_COMPAT ")\n");
    printf("  -s, --robustness=STRATEGY the robustness strategy to use ("
                                        STRATEGY_NAME_NONE " or "
                                        STRATEGY_NAME_LOSE ")\n");
    printf("  -v, --version             print version information\n");
    printf("      --red-bits=N          the number of red bits to request\n");
    printf("      --green-bits=N        the number of green bits to request\n");
    printf("      --blue-bits=N         the number of blue bits to request\n");
    printf("      --alpha-bits=N        the number of alpha bits to request\n");
    printf("      --depth-bits=N        the number of depth bits to request\n");
    printf("      --stencil-bits=N      the number of stencil bits to request\n");
    printf("      --accum-red-bits=N    the number of red bits to request\n");
    printf("      --accum-green-bits=N  the number of green bits to request\n");
    printf("      --accum-blue-bits=N   the number of blue bits to request\n");
    printf("      --accum-alpha-bits=N  the number of alpha bits to request\n");
    printf("      --aux-buffers=N       the number of aux buffers to request\n");
    printf("      --samples=N           the number of MSAA samples to request\n");
    printf("      --stereo              request stereo rendering\n");
    printf("      --srgb                request an sRGB capable framebuffer\n");
    printf("      --singlebuffer        request single-buffering\n");
    printf("      --no-error            request a context that does not emit errors\n");
    printf("      --graphics-switching  request macOS graphics switching\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static const char* get_device_type_name(VkPhysicalDeviceType type)
{
    if (type == VK_PHYSICAL_DEVICE_TYPE_OTHER)
        return "other";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return "integrated GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return "discrete GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        return "virtual GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_CPU)
        return "CPU";

    return "unknown";
}

static const char* get_api_name(int api)
{
    if (api == GLFW_OPENGL_API)
        return "OpenGL";
    else if (api == GLFW_OPENGL_ES_API)
        return "OpenGL ES";

    return "Unknown API";
}

static const char* get_profile_name_gl(GLint mask)
{
    if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        return PROFILE_NAME_COMPAT;
    if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_profile_name_glfw(int profile)
{
    if (profile == GLFW_OPENGL_COMPAT_PROFILE)
        return PROFILE_NAME_COMPAT;
    if (profile == GLFW_OPENGL_CORE_PROFILE)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_strategy_name_gl(GLint strategy)
{
    if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
        return STRATEGY_NAME_LOSE;
    if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static const char* get_strategy_name_glfw(int strategy)
{
    if (strategy == GLFW_LOSE_CONTEXT_ON_RESET)
        return STRATEGY_NAME_LOSE;
    if (strategy == GLFW_NO_RESET_NOTIFICATION)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static void list_context_extensions(int client, int major, int minor)
{
    int i;
    GLint count;
    const GLubyte* extensions;

    printf("%s context extensions:\n", get_api_name(client));

    if (client == GLFW_OPENGL_API && major > 2)
    {
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
            printf(" %s\n", (const char*) glGetStringi(GL_EXTENSIONS, i));
    }
    else
    {
        extensions = glGetString(GL_EXTENSIONS);
        while (*extensions != '\0')
        {
            putchar(' ');

            while (*extensions != '\0' && *extensions != ' ')
            {
                putchar(*extensions);
                extensions++;
            }

            while (*extensions == ' ')
                extensions++;

            putchar('\n');
        }
    }
}

static void list_vulkan_instance_layers(void)
{
    uint32_t i, lp_count = 0;
    VkLayerProperties* lp;

    printf("Vulkan instance layers:\n");

    if (vkEnumerateInstanceLayerProperties(&lp_count, NULL) != VK_SUCCESS)
        return;

    lp = calloc(lp_count, sizeof(VkLayerProperties));

    if (vkEnumerateInstanceLayerProperties(&lp_count, lp) != VK_SUCCESS)
    {
        free(lp);
        return;
    }

    for (i = 0;  i < lp_count;  i++)
    {
        printf(" %s (spec version %u.%u) \"%s\"\n",
               lp[i].layerName,
               VK_VERSION_MAJOR(lp[i].specVersion),
               VK_VERSION_MINOR(lp[i].specVersion),
               lp[i].description);
    }

    free(lp);
}

static void list_vulkan_device_layers(VkInstance instance, VkPhysicalDevice device)
{
    uint32_t i, lp_count;
    VkLayerProperties* lp;

    printf("Vulkan device layers:\n");

    if (vkEnumerateDeviceLayerProperties(device, &lp_count, NULL) != VK_SUCCESS)
        return;

    lp = calloc(lp_count, sizeof(VkLayerProperties));

    if (vkEnumerateDeviceLayerProperties(device, &lp_count, lp) != VK_SUCCESS)
    {
        free(lp);
        return;
    }

    for (i = 0;  i < lp_count;  i++)
    {
        printf(" %s (spec version %u.%u) \"%s\"\n",
               lp[i].layerName,
               VK_VERSION_MAJOR(lp[i].specVersion),
               VK_VERSION_MINOR(lp[i].specVersion),
               lp[i].description);
    }

    free(lp);
}

static int valid_version(void)
{
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);

    if (major != GLFW_VERSION_MAJOR)
    {
        printf("*** ERROR: GLFW major version mismatch! ***\n");
        return GLFW_FALSE;
    }

    if (minor != GLFW_VERSION_MINOR || revision != GLFW_VERSION_REVISION)
        printf("*** WARNING: GLFW version mismatch! ***\n");

    return GLFW_TRUE;
}

static void print_version(void)
{
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);

    printf("GLFW header version: %u.%u.%u\n",
           GLFW_VERSION_MAJOR,
           GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);
    printf("GLFW library version: %u.%u.%u\n", major, minor, revision);
    printf("GLFW library version string: \"%s\"\n", glfwGetVersionString());
}

int main(int argc, char** argv)
{
    int ch, client, major, minor, revision, profile;
    GLint redbits, greenbits, bluebits, alphabits, depthbits, stencilbits;
    int list_extensions = GLFW_FALSE, list_layers = GLFW_FALSE;
    GLenum error;
    GLFWwindow* window;

    enum { CLIENT, CONTEXT, BEHAVIOR, DEBUG_CONTEXT, FORWARD, HELP,
           EXTENSIONS, LAYERS,
           MAJOR, MINOR, PROFILE, ROBUSTNESS, VERSION,
           REDBITS, GREENBITS, BLUEBITS, ALPHABITS, DEPTHBITS, STENCILBITS,
           ACCUMREDBITS, ACCUMGREENBITS, ACCUMBLUEBITS, ACCUMALPHABITS,
           AUXBUFFERS, SAMPLES, STEREO, SRGB, SINGLEBUFFER, NOERROR_SRSLY,
           GRAPHICS_SWITCHING };
    const struct option options[] =
    {
        { "behavior",           1, NULL, BEHAVIOR },
        { "client-api",         1, NULL, CLIENT },
        { "context-api",        1, NULL, CONTEXT },
        { "debug",              0, NULL, DEBUG_CONTEXT },
        { "forward",            0, NULL, FORWARD },
        { "help",               0, NULL, HELP },
        { "list-extensions",    0, NULL, EXTENSIONS },
        { "list-layers",        0, NULL, LAYERS },
        { "major",              1, NULL, MAJOR },
        { "minor",              1, NULL, MINOR },
        { "profile",            1, NULL, PROFILE },
        { "robustness",         1, NULL, ROBUSTNESS },
        { "version",            0, NULL, VERSION },
        { "red-bits",           1, NULL, REDBITS },
        { "green-bits",         1, NULL, GREENBITS },
        { "blue-bits",          1, NULL, BLUEBITS },
        { "alpha-bits",         1, NULL, ALPHABITS },
        { "depth-bits",         1, NULL, DEPTHBITS },
        { "stencil-bits",       1, NULL, STENCILBITS },
        { "accum-red-bits",     1, NULL, ACCUMREDBITS },
        { "accum-green-bits",   1, NULL, ACCUMGREENBITS },
        { "accum-blue-bits",    1, NULL, ACCUMBLUEBITS },
        { "accum-alpha-bits",   1, NULL, ACCUMALPHABITS },
        { "aux-buffers",        1, NULL, AUXBUFFERS },
        { "samples",            1, NULL, SAMPLES },
        { "stereo",             0, NULL, STEREO },
        { "srgb",               0, NULL, SRGB },
        { "singlebuffer",       0, NULL, SINGLEBUFFER },
        { "no-error",           0, NULL, NOERROR_SRSLY },
        { "graphics-switching", 0, NULL, GRAPHICS_SWITCHING },
        { NULL, 0, NULL, 0 }
    };

    // Initialize GLFW and create window

    if (!valid_version())
        exit(EXIT_FAILURE);

    glfwSetErrorCallback(error_callback);

    glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    while ((ch = getopt_long(argc, argv, "a:b:c:dfhlm:n:p:s:v", options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'a':
            case CLIENT:
                if (strcasecmp(optarg, API_NAME_OPENGL) == 0)
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                else if (strcasecmp(optarg, API_NAME_OPENGL_ES) == 0)
                    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
            case BEHAVIOR:
                if (strcasecmp(optarg, BEHAVIOR_NAME_NONE) == 0)
                {
                    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR,
                                   GLFW_RELEASE_BEHAVIOR_NONE);
                }
                else if (strcasecmp(optarg, BEHAVIOR_NAME_FLUSH) == 0)
                {
                    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR,
                                   GLFW_RELEASE_BEHAVIOR_FLUSH);
                }
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
            case CONTEXT:
                if (strcasecmp(optarg, API_NAME_NATIVE) == 0)
                    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
                else if (strcasecmp(optarg, API_NAME_EGL) == 0)
                    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
                else if (strcasecmp(optarg, API_NAME_OSMESA) == 0)
                    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
            case DEBUG_CONTEXT:
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
                break;
            case 'f':
            case FORWARD:
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                break;
            case 'h':
            case HELP:
                usage();
                exit(EXIT_SUCCESS);
            case 'l':
            case EXTENSIONS:
                list_extensions = GLFW_TRUE;
                break;
            case LAYERS:
                list_layers = GLFW_TRUE;
                break;
            case 'm':
            case MAJOR:
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, atoi(optarg));
                break;
            case 'n':
            case MINOR:
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, atoi(optarg));
                break;
            case 'p':
            case PROFILE:
                if (strcasecmp(optarg, PROFILE_NAME_CORE) == 0)
                {
                    glfwWindowHint(GLFW_OPENGL_PROFILE,
                                   GLFW_OPENGL_CORE_PROFILE);
                }
                else if (strcasecmp(optarg, PROFILE_NAME_COMPAT) == 0)
                {
                    glfwWindowHint(GLFW_OPENGL_PROFILE,
                                   GLFW_OPENGL_COMPAT_PROFILE);
                }
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
            case ROBUSTNESS:
                if (strcasecmp(optarg, STRATEGY_NAME_NONE) == 0)
                {
                    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,
                                   GLFW_NO_RESET_NOTIFICATION);
                }
                else if (strcasecmp(optarg, STRATEGY_NAME_LOSE) == 0)
                {
                    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS,
                                   GLFW_LOSE_CONTEXT_ON_RESET);
                }
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
            case VERSION:
                print_version();
                exit(EXIT_SUCCESS);
            case REDBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_RED_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_RED_BITS, atoi(optarg));
                break;
            case GREENBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_GREEN_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_GREEN_BITS, atoi(optarg));
                break;
            case BLUEBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_BLUE_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_BLUE_BITS, atoi(optarg));
                break;
            case ALPHABITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_ALPHA_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_ALPHA_BITS, atoi(optarg));
                break;
            case DEPTHBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_DEPTH_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_DEPTH_BITS, atoi(optarg));
                break;
            case STENCILBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_STENCIL_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_STENCIL_BITS, atoi(optarg));
                break;
            case ACCUMREDBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_ACCUM_RED_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_ACCUM_RED_BITS, atoi(optarg));
                break;
            case ACCUMGREENBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_ACCUM_GREEN_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_ACCUM_GREEN_BITS, atoi(optarg));
                break;
            case ACCUMBLUEBITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_ACCUM_BLUE_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_ACCUM_BLUE_BITS, atoi(optarg));
                break;
            case ACCUMALPHABITS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, atoi(optarg));
                break;
            case AUXBUFFERS:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_AUX_BUFFERS, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_AUX_BUFFERS, atoi(optarg));
                break;
            case SAMPLES:
                if (strcmp(optarg, "-") == 0)
                    glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);
                else
                    glfwWindowHint(GLFW_SAMPLES, atoi(optarg));
                break;
            case STEREO:
                glfwWindowHint(GLFW_STEREO, GLFW_TRUE);
                break;
            case SRGB:
                glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
                break;
            case SINGLEBUFFER:
                glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
                break;
            case NOERROR_SRSLY:
                glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
                break;
            case GRAPHICS_SWITCHING:
                glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_TRUE);
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    print_version();

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(200, 200, "Version", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    error = glGetError();
    if (error != GL_NO_ERROR)
        printf("*** OpenGL error after make current: 0x%08x ***\n", error);

    // Report client API version

    client = glfwGetWindowAttrib(window, GLFW_CLIENT_API);
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    revision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

    printf("%s context version string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_VERSION));

    printf("%s context version parsed by GLFW: %u.%u.%u\n",
           get_api_name(client),
           major, minor, revision);

    // Report client API context properties

    if (client == GLFW_OPENGL_API)
    {
        if (major >= 3)
        {
            GLint flags;

            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            printf("%s context flags (0x%08x):", get_api_name(client), flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                printf(" forward-compatible");
            if (flags & 2/*GL_CONTEXT_FLAG_DEBUG_BIT*/)
                printf(" debug");
            if (flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB)
                printf(" robustness");
            if (flags & 8/*GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR*/)
                printf(" no-error");
            putchar('\n');

            printf("%s context flags parsed by GLFW:", get_api_name(client));

            if (glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT))
                printf(" forward-compatible");
            if (glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT))
                printf(" debug");
            if (glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS) == GLFW_LOSE_CONTEXT_ON_RESET)
                printf(" robustness");
            if (glfwGetWindowAttrib(window, GLFW_CONTEXT_NO_ERROR))
                printf(" no-error");
            putchar('\n');
        }

        if (major >= 4 || (major == 3 && minor >= 2))
        {
            GLint mask;
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

            printf("%s profile mask (0x%08x): %s\n",
                   get_api_name(client),
                   mask,
                   get_profile_name_gl(mask));

            printf("%s profile mask parsed by GLFW: %s\n",
                   get_api_name(client),
                   get_profile_name_glfw(profile));
        }

        if (GLAD_GL_ARB_robustness)
        {
            const int robustness = glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS);
            GLint strategy;
            glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            printf("%s robustness strategy (0x%08x): %s\n",
                   get_api_name(client),
                   strategy,
                   get_strategy_name_gl(strategy));

            printf("%s robustness strategy parsed by GLFW: %s\n",
                   get_api_name(client),
                   get_strategy_name_glfw(robustness));
        }
    }

    printf("%s context renderer string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_RENDERER));
    printf("%s context vendor string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_VENDOR));

    if (major >= 2)
    {
        printf("%s context shading language version: \"%s\"\n",
               get_api_name(client),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    printf("%s framebuffer:\n", get_api_name(client));

    if (client == GLFW_OPENGL_API && profile == GLFW_OPENGL_CORE_PROFILE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
                                              &redbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,
                                              &greenbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,
                                              &bluebits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
                                              &alphabits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_DEPTH,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
                                              &depthbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_STENCIL,
                                              GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                              &stencilbits);
    }
    else
    {
        glGetIntegerv(GL_RED_BITS, &redbits);
        glGetIntegerv(GL_GREEN_BITS, &greenbits);
        glGetIntegerv(GL_BLUE_BITS, &bluebits);
        glGetIntegerv(GL_ALPHA_BITS, &alphabits);
        glGetIntegerv(GL_DEPTH_BITS, &depthbits);
        glGetIntegerv(GL_STENCIL_BITS, &stencilbits);
    }

    printf(" red: %u green: %u blue: %u alpha: %u depth: %u stencil: %u\n",
           redbits, greenbits, bluebits, alphabits, depthbits, stencilbits);

    if (client == GLFW_OPENGL_ES_API ||
        GLAD_GL_ARB_multisample ||
        major > 1 || minor >= 3)
    {
        GLint samples, samplebuffers;
        glGetIntegerv(GL_SAMPLES, &samples);
        glGetIntegerv(GL_SAMPLE_BUFFERS, &samplebuffers);

        printf(" samples: %u sample buffers: %u\n", samples, samplebuffers);
    }

    if (client == GLFW_OPENGL_API && profile != GLFW_OPENGL_CORE_PROFILE)
    {
        GLint accumredbits, accumgreenbits, accumbluebits, accumalphabits;
        GLint auxbuffers;

        glGetIntegerv(GL_ACCUM_RED_BITS, &accumredbits);
        glGetIntegerv(GL_ACCUM_GREEN_BITS, &accumgreenbits);
        glGetIntegerv(GL_ACCUM_BLUE_BITS, &accumbluebits);
        glGetIntegerv(GL_ACCUM_ALPHA_BITS, &accumalphabits);
        glGetIntegerv(GL_AUX_BUFFERS, &auxbuffers);

        printf(" accum red: %u accum green: %u accum blue: %u accum alpha: %u aux buffers: %u\n",
               accumredbits, accumgreenbits, accumbluebits, accumalphabits, auxbuffers);
    }

    if (list_extensions)
        list_context_extensions(client, major, minor);

    glfwDestroyWindow(window);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(200, 200, "Version", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    printf("Vulkan loader: %s\n",
           glfwVulkanSupported() ? "available" : "missing");

    if (glfwVulkanSupported())
    {
        uint32_t loader_version = VK_API_VERSION_1_0;
        int portability_enumeration = GLFW_FALSE;
        uint32_t i, j, glfw_re_count, re_count, pd_count, ep_count;
        const char** glfw_re;
        const char** re;
        VkApplicationInfo ai = {0};
        VkInstanceCreateInfo ici = {0};
        VkInstance instance;
        VkPhysicalDevice* pd;

        gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, NULL);

        if (vkEnumerateInstanceVersion)
        {
            uint32_t version;
            if (vkEnumerateInstanceVersion(&version) == VK_SUCCESS)
                loader_version = version;
        }

        printf("Vulkan loader API version: %i.%i\n",
               VK_VERSION_MAJOR(loader_version),
               VK_VERSION_MINOR(loader_version));

        glfw_re = glfwGetRequiredInstanceExtensions(&glfw_re_count);

        re_count = glfw_re_count;
        re = calloc(glfw_re_count, sizeof(char*));

        printf("Vulkan window surface required instance extensions:\n");
        if (glfw_re)
        {
            for (i = 0;  i < glfw_re_count;  i++)
            {
                printf(" %s\n", glfw_re[i]);
                re[i] = glfw_re[i];
            }
        }
        else
            printf(" missing\n");

        vkEnumerateInstanceExtensionProperties(NULL, &ep_count, NULL);
        VkExtensionProperties* ep = calloc(ep_count, sizeof(VkExtensionProperties));
        vkEnumerateInstanceExtensionProperties(NULL, &ep_count, ep);

        if (list_extensions)
        {
            printf("Vulkan instance extensions:\n");

            for (i = 0;  i < ep_count;  i++)
                printf(" %s (spec version %u)\n", ep[i].extensionName, ep[i].specVersion);
        }

        for (i = 0;  i < ep_count;  i++)
        {
            if (strcmp(ep[i].extensionName, "VK_KHR_portability_enumeration") != 0)
                continue;

            re_count++;
            re = realloc((void*) re, sizeof(char*) * re_count);
            re[re_count - 1] = "VK_KHR_portability_enumeration";
            portability_enumeration = GLFW_TRUE;
        }

        free(ep);

        if (list_layers)
            list_vulkan_instance_layers();

        ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        ai.pApplicationName = "glfwinfo";
        ai.applicationVersion = VK_MAKE_VERSION(GLFW_VERSION_MAJOR,
                                                GLFW_VERSION_MINOR,
                                                GLFW_VERSION_REVISION);

        if (loader_version >= VK_API_VERSION_1_1)
            ai.apiVersion = VK_API_VERSION_1_1;
        else
            ai.apiVersion = VK_API_VERSION_1_0;

        ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ici.pApplicationInfo = &ai;
        ici.enabledExtensionCount = re_count;
        ici.ppEnabledExtensionNames = re;

        if (portability_enumeration)
            ici.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        if (vkCreateInstance(&ici, NULL, &instance) != VK_SUCCESS)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        if (re)
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;

            if (glfwCreateWindowSurface(instance, window, NULL, &surface) == VK_SUCCESS)
            {
                printf("Vulkan window surface created successfully\n");
                vkDestroySurfaceKHR(instance, surface, NULL);
            }
            else
                printf("Failed to create Vulkan window surface\n");
        }

        free((void*) re);

        gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, instance);

        vkEnumeratePhysicalDevices(instance, &pd_count, NULL);
        pd = calloc(pd_count, sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(instance, &pd_count, pd);

        for (i = 0;  i < pd_count;  i++)
        {
            VkPhysicalDeviceProperties pdp;
            uint32_t qfp_count, ep_count;

            vkGetPhysicalDeviceProperties(pd[i], &pdp);

            printf("Vulkan %s device: \"%s\" API version %i.%i\n",
                   get_device_type_name(pdp.deviceType),
                   pdp.deviceName,
                   VK_VERSION_MAJOR(pdp.apiVersion),
                   VK_VERSION_MINOR(pdp.apiVersion));

            vkGetPhysicalDeviceQueueFamilyProperties(pd[i], &qfp_count, NULL);

            vkEnumerateDeviceExtensionProperties(pd[i], NULL, &ep_count, NULL);
            VkExtensionProperties* ep = calloc(ep_count, sizeof(VkExtensionProperties));
            vkEnumerateDeviceExtensionProperties(pd[i], NULL, &ep_count, ep);

            if (portability_enumeration)
            {
                int conformant = GLFW_TRUE;

                for (j = 0; j < ep_count; j++)
                {
                    if (strcmp(ep[j].extensionName, "VK_KHR_portability_subset") == 0)
                    {
                        conformant = GLFW_FALSE;
                        break;
                    }
                }

                printf("Vulkan %s %s device: \"%s\" (API version %i.%i)\n",
                       conformant ? "conformant" : "non-conformant",
                       get_device_type_name(pdp.deviceType),
                       pdp.deviceName,
                       VK_VERSION_MAJOR(pdp.apiVersion),
                       VK_VERSION_MINOR(pdp.apiVersion));
            }
            else
            {
                printf("Vulkan %s device: \"%s\" (API version %i.%i)\n",
                       get_device_type_name(pdp.deviceType),
                       pdp.deviceName,
                       VK_VERSION_MAJOR(pdp.apiVersion),
                       VK_VERSION_MINOR(pdp.apiVersion));
            }

            if (glfw_re_count)
            {
                printf("Vulkan device queue family presentation support:\n");
                for (j = 0;  j < qfp_count;  j++)
                {
                    printf(" %u: ", j);
                    if (glfwGetPhysicalDevicePresentationSupport(instance, pd[i], j))
                        printf("supported\n");
                    else
                        printf("no\n");
                }
            }

            if (list_extensions)
            {
                printf("Vulkan device extensions:\n");
                for (j = 0;  j < ep_count;  j++)
                    printf(" %s (spec version %u)\n", ep[j].extensionName, ep[j].specVersion);
            }

            free(ep);

            if (list_layers)
                list_vulkan_device_layers(instance, pd[i]);
        }

        free(pd);
        vkDestroyInstance(instance, NULL);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

