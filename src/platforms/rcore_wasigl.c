// WASM imported functions can't be used as indirect call targets (function pointers).
// Wrap glfwGetProcAddress in a C-defined function so it lives in the WASM table.
// glfw3.h (included inside rcore_desktop_glfw.c) declares wasigl_proc_loader via
// the macro — no forward declaration needed here to avoid a type conflict.
#define glfwGetProcAddress wasigl_proc_loader
#include "./rcore_desktop_glfw.c"
#undef glfwGetProcAddress

// After the undef, glfwGetProcAddress names the real host import again.
// Re-declare it so the wrapper body below can call it.
extern GLFWglproc glfwGetProcAddress(const char *procname);

// Definition matches the GLFWAPI GLFWglproc declaration glfw3.h produced via macro.
GLFWglproc wasigl_proc_loader(const char *procname) {
    return glfwGetProcAddress(procname);
}
