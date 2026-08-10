/*
 * Nuklear - v1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_GLFW_GL2_H_
#define NK_GLFW_GL2_H_

#include <GLFW/glfw3.h>

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT = 0,
    NK_GLFW3_INSTALL_CALLBACKS
};
NK_API struct nk_context*   nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state);
NK_API void                 nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_glfw3_font_stash_end(void);

NK_API void                 nk_glfw3_new_frame(void);
NK_API void                 nk_glfw3_render(enum nk_anti_aliasing);
NK_API void                 nk_glfw3_shutdown(void);

NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_GLFW_GL2_IMPLEMENTATION

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

struct nk_glfw_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint font_tex;
};

struct nk_glfw_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_glfw_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} glfw;

NK_INTERN void
nk_glfw3_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_glfw3_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_glfw_device *dev = &glfw.ogl;
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)glfw.display_width,(GLsizei)glfw.display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, glfw.width, glfw.height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_glfw_vertex);
        size_t vp = offsetof(struct nk_glfw_vertex, position);
        size_t vt = offsetof(struct nk_glfw_vertex, uv);
        size_t vc = offsetof(struct nk_glfw_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_glfw_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
        config.null = dev->null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&glfw.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &glfw.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * glfw.fb_scale.x),
                (GLint)((glfw.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * glfw.fb_scale.y),
                (GLint)(cmd->clip_rect.w * glfw.fb_scale.x),
                (GLint)(cmd->clip_rect.h * glfw.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&glfw.ctx);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

NK_API void
nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint)
{
    (void)win;
    if (glfw.text_len < NK_GLFW_TEXT_MAX)
        glfw.text[glfw.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    glfw.scroll.x += (float)xoff;
    glfw.scroll.y += (float)yoff;
}

NK_API void
nk_glfw3_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)  {
        double dt = glfwGetTime() - glfw.last_button_click;
        if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
            glfw.is_double_click_down = nk_true;
            glfw.double_click_pos = nk_vec2((float)x, (float)y);
        }
        glfw.last_button_click = glfwGetTime();
    } else glfw.is_double_click_down = nk_false;
}

NK_INTERN void
nk_glfw3_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = glfwGetClipboardString(glfw.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_glfw3_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    NK_MEMCPY(str, text, (size_t)len);
    str[len] = '\0';
    glfwSetClipboardString(glfw.win, str);
    free(str);
}

NK_API struct nk_context*
nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state init_state)
{
    glfw.win = win;
    if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
        glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
        glfwSetCharCallback(win, nk_glfw3_char_callback);
        glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    }
    nk_init_default(&glfw.ctx, 0);
    glfw.ctx.clip.copy = nk_glfw3_clipboard_copy;
    glfw.ctx.clip.paste = nk_glfw3_clipboard_paste;
    glfw.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&glfw.ogl.cmds);

    glfw.is_double_click_down = nk_false;
    glfw.double_click_pos = nk_vec2(0, 0);

    return &glfw.ctx;
}

NK_API void
nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&glfw.atlas);
    nk_font_atlas_begin(&glfw.atlas);
    *atlas = &glfw.atlas;
}

NK_API void
nk_glfw3_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_glfw3_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&glfw.atlas, nk_handle_id((int)glfw.ogl.font_tex), &glfw.ogl.null);
    if (glfw.atlas.default_font)
        nk_style_set_font(&glfw.ctx, &glfw.atlas.default_font->handle);
}

NK_API void
nk_glfw3_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context *ctx = &glfw.ctx;
    struct GLFWwindow *win = glfw.win;

    glfwGetWindowSize(win, &glfw.width, &glfw.height);
    glfwGetFramebufferSize(win, &glfw.display_width, &glfw.display_height);
    glfw.fb_scale.x = (float)glfw.display_width/(float)glfw.width;
    glfw.fb_scale.y = (float)glfw.display_height/(float)glfw.height;

    nk_input_begin(ctx);
    for (i = 0; i < glfw.text_len; ++i)
        nk_input_unicode(ctx, glfw.text[i]);

    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, glfwGetKey(win, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS||
                                    glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        nk_input_key(ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_V) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, glfwGetKey(win, GLFW_KEY_Z) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, glfwGetKey(win, GLFW_KEY_B) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    if (ctx->input.mouse.grabbed) {
        glfwSetCursorPos(glfw.win, (double)ctx->input.mouse.prev.x, (double)ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)glfw.double_click_pos.x, (int)glfw.double_click_pos.y, glfw.is_double_click_down);
    nk_input_scroll(ctx, glfw.scroll);
    nk_input_end(&glfw.ctx);
    glfw.text_len = 0;
    glfw.scroll = nk_vec2(0,0);
}

NK_API
void nk_glfw3_shutdown(void)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    nk_font_atlas_clear(&glfw.atlas);
    nk_free(&glfw.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    NK_MEMSET(&glfw, 0, sizeof(glfw));
}

#endif
