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

#include "yagl_client_image.h"
#include "yagl_egl_state.h"
#include "yagl_display.h"
#include "yagl_context.h"
#include "yagl_image.h"

void yagl_client_image_init(struct yagl_client_image *image,
                            yagl_ref_destroy_func destroy_func)
{
    yagl_object_init(&image->base, destroy_func);
}

void yagl_client_image_cleanup(struct yagl_client_image *image)
{
    yagl_object_cleanup(&image->base);
}

void yagl_client_image_acquire(struct yagl_client_image *image)
{
    if (image) {
        yagl_object_acquire(&image->base);
    }
}

void yagl_client_image_release(struct yagl_client_image *image)
{
    if (image) {
        yagl_object_release(&image->base);
    }
}

struct yagl_client_image *yagl_acquire_client_image(yagl_host_handle handle)
{
    struct yagl_context *ctx = yagl_get_context();
    struct yagl_image *image;
    struct yagl_client_image *client_image;

    if (!ctx) {
        return NULL;
    }

    image = yagl_display_image_acquire(ctx->dpy, (EGLImageKHR)handle);

    if (!image) {
        return NULL;
    }

    image->update(image);

    client_image = image->client_image;

    yagl_client_image_acquire(client_image);
    yagl_image_release(image);

    return client_image;
}
