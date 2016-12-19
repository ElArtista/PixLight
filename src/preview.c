#include "preview.h"
#include <string.h>
#include <stdlib.h>
#include <gfxwnd/input.h>
#include <gfxwnd/window.h>
#include <glad/glad.h>

static const char* preview_vshdr = " \
#version 330 core\n \
layout (location = 0) in vec3 position;\n \
\n\
void main() \n\
{ \n\
    gl_Position = vec4(position, 1.0f); \n\
}";

static const char* preview_fshdr = " \
#version 330 core \n\
out vec4 color; \n\
\n\
uniform ivec2 gScreenSize; \n\
uniform sampler2D tex; \n\
\
void main() \n\
{ \n\
    vec2 UVCoords = gl_FragCoord.xy / gScreenSize; \n\
    color = vec4(texture(tex, UVCoords).rgb, 1.0); \n\
}";

static const GLfloat quad_vert[] = {
   -1.0f,  1.0f, 0.0f,
   -1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f
};

struct preview_internal_state {
    GLuint shdr;
    GLuint quad_vao;
    GLuint quad_vbo;
    GLuint tex;
};

static GLuint build_preview_shader()
{
    GLuint vid = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vid, 1, &preview_vshdr, 0);
    glCompileShader(vid);

    GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fid, 1, &preview_fshdr, 0);
    glCompileShader(fid);

    GLuint pid = glCreateProgram();
    glAttachShader(pid, vid);
    glAttachShader(pid, fid);
    glLinkProgram(pid);

    glDeleteShader(vid);
    glDeleteShader(fid);
    return pid;
}

static void on_key(struct window* wnd, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    struct preview_context* ctx = window_get_userdata(wnd);
    if (action == 0 && key == KEY_ESCAPE)
        *(ctx->should_terminate) = 1;
}

void preview_init(struct preview_context* ctx)
{
    /* Create window */
    const char* title = "PixLight";
    int width = 800, height = 600, mode = 0;
    ctx->wnd = window_create(title, width, height, mode);

    /* Assosiate context to be accessed from callback functions */
    window_set_userdata(ctx->wnd, ctx);

    /* Set event callbacks */
    struct window_callbacks wnd_callbacks;
    memset(&wnd_callbacks, 0, sizeof(struct window_callbacks));
    wnd_callbacks.key_cb = on_key;
    window_set_callbacks(ctx->wnd, &wnd_callbacks);

    /* Setup shader */
    struct preview_internal_state* is = malloc(sizeof(struct preview_internal_state));
    is->shdr = build_preview_shader();
    /* Setup quad geometry */
    glGenVertexArrays(1, &is->quad_vao);
    glBindVertexArray(is->quad_vao);
    glGenBuffers(1, &is->quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, is->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vert), &quad_vert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    /* Setup GPU preview texture */
    glGenTextures(1, &is->tex);
    glBindTexture(GL_TEXTURE_2D, is->tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->bitmap->width, ctx->bitmap->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ctx->bitmap->data);
    /* Set internal state */
    ctx->_internal_state = is;
}

void preview_update(void* userdata, float dt)
{
    (void) dt;
    struct preview_context* ctx = userdata;
    /* Process input events */
    window_update(ctx->wnd);
}

void preview_render(void* userdata, float interpolation)
{
    (void) interpolation;
    struct preview_context* ctx = userdata;

    /* Clear */
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Setup shader */
    struct preview_internal_state* is = ctx->_internal_state;
    glUseProgram(is->shdr);
    glUniform1i(glGetUniformLocation(is->shdr, "tex"), 0);
    glUniform2i(glGetUniformLocation(is->shdr, "gScreenSize"), ctx->bitmap->width, ctx->bitmap->height);

    /* Update GPU texture */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, is->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->bitmap->width, ctx->bitmap->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ctx->bitmap->data);

    /* Render quad */
    glBindVertexArray(is->quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    /* Show rendered contents from the backbuffer */
    window_swap_buffers(ctx->wnd);
}

void preview_shutdown(struct preview_context* ctx)
{
    struct preview_internal_state* is = ctx->_internal_state;
    /* Free GPU texture */
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &is->tex);
    /* Free quad geometry data */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &is->quad_vbo);
    glDeleteVertexArrays(1, &is->quad_vao);
    /* Free shader */
    glUseProgram(0);
    glDeleteProgram(is->shdr);

    /* Free internal state */
    free(is);
    ctx->_internal_state = 0;

    /* Close window */
    window_destroy(ctx->wnd);
}
