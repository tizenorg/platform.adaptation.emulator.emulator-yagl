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

#include "GLES/gl.h"
#include "yagl_export.h"
#include "yagl_client_interface.h"
#include "yagl_gles1_context.h"
#include "yagl_gles_image.h"
#include "yagl_gles_texture.h"

static struct yagl_client_context *yagl_gles1_create_ctx(struct yagl_client_interface *iface,
                                                         yagl_client_api client_api,
                                                         struct yagl_sharegroup *sg)
{
    return yagl_gles1_context_create(sg);
}

static struct yagl_client_image
    *yagl_gles1_create_image(struct yagl_client_interface *iface,
                             yagl_object_name tex_global_name)
{
    return &yagl_gles_image_create(tex_global_name)->base;
}

static void yagl_gles1_release_tex_image(struct yagl_client_interface *iface,
                                         void *cookie)
{
    struct yagl_gles_texture *texture = cookie;

    yagl_gles_texture_release_tex_image(texture);
}

YAGL_API struct yagl_client_interface yagl_gles1_interface =
{
    .create_ctx = &yagl_gles1_create_ctx,
    .create_image = &yagl_gles1_create_image,
    .release_tex_image = &yagl_gles1_release_tex_image
};
