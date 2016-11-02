#include "raytracer.h"
#include <linalgb.h>
#include <tinycthread.h>

void raytracer_setup(struct raytracer_context* rc)
{
    int height = rc->output->height;
    int width = rc->output->width;
    unsigned char* data = rc->output->data;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char* pix = data + (i * width + j) * 3;
            pix[0] = i * 255 / height;
            pix[1] = j * 255 / width;
            pix[2] = i * j * 255 / (width * height);
        }
    }
}

struct ray
{
    vec3 origin;
    vec3 direction;
};

static vec3 ray_point_at_parameter(struct ray r, float t)
{
    return vec3_add(r.origin, vec3_mul(r.direction, t));
}

static vec3 raytracer_color(struct ray r)
{
    vec3 unit_direction = vec3_normalize(r.direction);
    float t = 0.5f * (unit_direction.y + 1.0f);
    return vec3_lerp(vec3_one(), vec3_new(0.5f, 0.7f, 1.0f), t);
}

void raytracer_run(struct raytracer_context* rc)
{
    /* Image info aliases */
    int height = rc->output->height;
    int width = rc->output->width;
    unsigned char* data = rc->output->data;

    /* Viewport setup */
    vec3 lower_left_corner = vec3_new(-2.0f, -1.0f, -1.0f);
    vec3 horizontal = vec3_new(4.0f, 0.0f, 0.0f);
    vec3 vertical = vec3_new(0.0f, 2.0f, 0.0f);
    vec3 origin = vec3_zero();

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            float u = (float) i / height;
            float v = (float) j / width;

            struct ray r;
            r.origin = origin;
            r.direction = vec3_add(
                lower_left_corner,
                vec3_add(
                    vec3_mul(horizontal, u),
                    vec3_mul(vertical, v)
                )
            );

            vec3 col = raytracer_color(r);

            unsigned char* pix = data + (i * width + j) * 3;
            pix[0] = col.x * 255.0f;
            pix[1] = col.y * 255.0f;
            pix[2] = col.z * 255.0f;
        }

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 1000 * 5000;
        thrd_sleep(&ts, 0);
    }
}
