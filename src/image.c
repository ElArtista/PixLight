#include "image.h"
#include <stdlib.h>
#include <string.h>

void image_new(struct image* im, int width, int height)
{
    im->width = width;
    im->height = height;
    im->data = malloc(width * height * 3);
    memset(im->data, 0, width * height * 3);
}

void image_free(struct image* im)
{
    free(im->data);
    memset(im, 0, sizeof(struct image));
}
