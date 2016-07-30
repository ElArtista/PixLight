#include "raytracer.h"
#include <string.h>
#include "input.h"
#include "window.h"

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
}

void update(void* userdata, float dt)
{
    (void) dt;
    struct raytracer_context* ctx = userdata;
    /* Process input events */
    window_poll_events(ctx->wnd);
}

void render(void* userdata, float interpolation)
{
    (void) interpolation;
    struct raytracer_context* ctx = userdata;
    /* Show rendered contents from the backbuffer */
    window_swap_buffers(ctx->wnd);
}

void shutdown(struct raytracer_context* ctx)
{
    /* Close window */
    window_destroy(ctx->wnd);
}
