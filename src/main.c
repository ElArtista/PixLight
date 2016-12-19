/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <tinycthread.h>
#include "mainloop.h"
#include "preview.h"
#include "image.h"
#include "raytracer.h"

void APIENTRY gl_debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
    (void) source;
    (void) id;
    (void) severity;
    (void) length;
    (void) user_param;

    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "%s", message);
        exit(1);
    }
}

int raytracer_thrd(void* arg)
{
    struct raytracer_context* rt = (struct raytracer_context*) arg;
    raytracer_run(rt);
    return 0;
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    /* Setup sample rendered bitmap */
    int width = 800;
    int height = 600;
    struct image result;
    image_new(&result, width, height);

    /* Initialize preview */
    struct preview_context ctx;
    memset(&ctx, 0, sizeof(struct preview_context));
    ctx.bitmap = &result;
    preview_init(&ctx);

    /* Setup mainloop parameters */
    struct mainloop_data mld;
    memset(&mld, 0, sizeof(struct mainloop_data));
    mld.max_frameskip = 5;
    mld.updates_per_second = 60;
    mld.update_callback = preview_update;
    mld.render_callback = preview_render;
    mld.userdata = &ctx;
    ctx.should_terminate = &mld.should_terminate;

    /* Setup OpenGL debug handler */
    glDebugMessageCallback(gl_debug_proc, &ctx);

    /* Setup raytracer */
    struct raytracer_context rcctx;
    rcctx.output = &result;
    raytracer_setup(&rcctx);

    /* Launch raytracing thread */
    thrd_t rt;
    thrd_create(&rt, raytracer_thrd, &rcctx);
    thrd_detach(rt);

    /* Run mainloop */
    mainloop(&mld);

    /* De-initialize */
    preview_shutdown(&ctx);
    image_free(&result);

    return 0;
}
