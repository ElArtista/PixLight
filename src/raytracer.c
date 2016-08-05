#include "raytracer.h"
#include <string.h>
#include <stdlib.h>
#include "input.h"
#include "window.h"
#include <glad/glad.h>

static void on_key(struct window* wnd, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    struct raytracer_context* ctx = window_get_userdata(wnd);
    if (action == 0 && key == KEY_ESCAPE)
        *(ctx->should_terminate) = 1;
}

void init(struct raytracer_context* ctx)
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

    /* Allocate space for result bitmap */
    ctx->bitmap.width = width;
    ctx->bitmap.height = height;
    ctx->bitmap.data = malloc(width * height * 3);
    memset(ctx->bitmap.data, 0, width * height * 3);

    /* Sample */
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char* pix = ctx->bitmap.data + (i * width + j) * 3;
            pix[0] = i * 255 / height;
            pix[1] = j * 255 / width;
            pix[2] = i * j * 255 / (width * height);
        }
    }
}

void update(void* userdata, float dt)
{
    (void) dt;
    struct raytracer_context* ctx = userdata;
    /* Process input events */
    window_poll_events(ctx->wnd);
}

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

static void render_quad()
{
    GLfloat quadVert[] =
    {
       -1.0f,  1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    GLuint quadVao;
    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);

    GLuint quadVbo;
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), &quadVert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &quadVbo);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &quadVao);
}

void render(void* userdata, float interpolation)
{
    (void) interpolation;
    struct raytracer_context* ctx = userdata;

    /* Clear */
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Create texture in GPU */
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ctx->bitmap.width, ctx->bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, ctx->bitmap.data);

    /* Create and setup preview shader */
    GLuint sh = build_preview_shader();
    glUseProgram(sh);
    glUniform1i(glGetUniformLocation(sh, "tex"), 0);
    glUniform2i(glGetUniformLocation(sh, "gScreenSize"), ctx->bitmap.width, ctx->bitmap.height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    /* Render quad */
    render_quad();

    /* Free GPU resources */
    glUseProgram(0);
    glDeleteProgram(sh);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &tex);

    /* Show rendered contents from the backbuffer */
    window_swap_buffers(ctx->wnd);
}

void shutdown(struct raytracer_context* ctx)
{
    /* Close window */
    window_destroy(ctx->wnd);
}
