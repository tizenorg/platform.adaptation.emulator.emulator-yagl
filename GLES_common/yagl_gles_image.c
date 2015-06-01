/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#include "GL/gl.h"
#include "yagl_gles_image.h"
#include "yagl_gles_texture.h"
#include "yagl_client_context.h"
#include "yagl_sharegroup.h"
#include "yagl_malloc.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_image_update(struct yagl_client_image *image,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t bpp,
                                   const void *pixels)
{
    struct yagl_gles_image *gles_image = (struct yagl_gles_image*)image;

    yagl_host_glUpdateOffscreenImageYAGL(gles_image->tex_global_name,
                                         width,
                                         height,
                                         bpp,
                                         pixels,
                                         width * height * bpp);
}

static void yagl_gles_image_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_image *image = (struct yagl_gles_image*)ref;

    if (image->texture_obj) {
        yagl_gles_texture_release(image->texture_obj);
    } else {
        yagl_host_glDeleteObjects(&image->tex_global_name, 1);
    }

    yagl_client_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_gles_image *yagl_gles_image_create(yagl_object_name tex_global_name)
{
    struct yagl_gles_image *image;

    image = yagl_malloc0(sizeof(*image));

    yagl_client_image_init(&image->base, &yagl_gles_image_destroy);

    image->base.update = &yagl_gles_image_update;

    image->tex_global_name = tex_global_name;

    return image;
}

struct yagl_gles_image *yagl_gles_image_wrap_tex(struct yagl_client_context *ctx,
                                                 yagl_object_name tex_local_name)
{
    struct yagl_gles_texture *texture_obj;
    struct yagl_gles_image *image;

    texture_obj = (struct yagl_gles_texture *)yagl_sharegroup_acquire_object(ctx->sg,
                                                                             YAGL_NS_TEXTURE,
                                                                             tex_local_name);

    if (!texture_obj) {
        goto fail;
    }

    image = yagl_malloc0(sizeof(*image));

    yagl_client_image_init(&image->base, &yagl_gles_image_destroy);

    image->base.update = &yagl_gles_image_update;

    image->tex_global_name = texture_obj->global_name;
    image->texture_obj = texture_obj;

    return image;

fail:
    yagl_gles_texture_release(texture_obj);

    return NULL;
}

void yagl_gles_image_acquire(struct yagl_gles_image *image)
{
    if (image) {
        yagl_client_image_acquire(&image->base);
    }
}

void yagl_gles_image_release(struct yagl_gles_image *image)
{
    if (image) {
        yagl_client_image_release(&image->base);
    }
}
