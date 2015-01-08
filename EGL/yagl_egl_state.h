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

#ifndef _YAGL_EGL_STATE_H_
#define _YAGL_EGL_STATE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "EGL/egl.h"

struct yagl_context;
struct yagl_surface;

void *yagl_get_gles1_sym(const char *name);
void *yagl_get_gles2_sym(const char *name);

/*
 * Those return per-thread values.
 * @{
 */

EGLint yagl_get_error();

void yagl_set_error(EGLint error);

EGLenum yagl_get_api();

void yagl_set_api(EGLenum api);

struct yagl_context *yagl_get_context();
struct yagl_surface *yagl_get_draw_surface();
struct yagl_surface *yagl_get_read_surface();

int yagl_set_context(struct yagl_context *ctx,
                     struct yagl_surface *draw_sfc,
                     struct yagl_surface *read_sfc);

void yagl_reset_state();

struct yagl_client_interface *yagl_get_client_interface(yagl_client_api client_api);

struct yagl_client_interface *yagl_get_any_client_interface();

/*
 * @}
 */

#endif
