/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Vasily Ulyanov <v.ulyanov@samsung.com>
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

#include "yagl_onscreen_image_gl_texture_2d.h"
#include "yagl_client_interface.h"
#include "yagl_client_image.h"
#include "yagl_egl_state.h"
#include "yagl_context.h"
#include "yagl_malloc.h"

static void yagl_onscreen_image_gl_texture_2d_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_gl_texture_2d_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image_gl_texture_2d *image = (struct yagl_onscreen_image_gl_texture_2d *)ref;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}


struct yagl_onscreen_image_gl_texture_2d
    *yagl_onscreen_image_gl_texture_2d_create(struct yagl_display *dpy,
                                              struct yagl_context *ctx,
                                              yagl_object_name texture,
                                              struct yagl_client_interface *iface)
{
    struct yagl_client_image *client_image;
    struct yagl_onscreen_image_gl_texture_2d *image;

    image = yagl_malloc0(sizeof(*image));

    client_image = iface->wrap_texture(iface,
                                       ctx->client_ctx,
                                       texture);

    if (!client_image) {
        yagl_set_error(EGL_BAD_PARAMETER);
        goto fail;
    }

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_gl_texture_2d_destroy,
                    dpy,
                    (EGLImageKHR)image,
                    client_image);

    yagl_client_image_release(client_image);

    image->base.update = &yagl_onscreen_image_gl_texture_2d_update;

    return image;

fail:
    yagl_free(image);

    return NULL;
}
