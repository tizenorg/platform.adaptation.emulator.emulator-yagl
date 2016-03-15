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

#include "yagl_render.h"
#include "yagl_egl_state.h"
#include "yagl_surface.h"
#include "yagl_context.h"
#include "yagl_transport.h"
#include "yagl_state.h"

void yagl_render_invalidate(int throttle)
{
    struct yagl_surface *draw_sfc = yagl_get_draw_surface();
    struct yagl_surface *read_sfc = yagl_get_read_surface();

    if (throttle) {
        struct yagl_context *ctx = yagl_get_context();
        if (ctx) {
            yagl_context_throttle(ctx);
        }
    }

    if (draw_sfc) {
        yagl_surface_invalidate(draw_sfc);
    }

    if (read_sfc && (draw_sfc != read_sfc)) {
        yagl_surface_invalidate(read_sfc);
    }
}

void yagl_render_finish()
{
    struct yagl_surface *draw_sfc = yagl_get_draw_surface();

    if (draw_sfc) {
        draw_sfc->wait_gl(draw_sfc);
    } else {
        /*
         * GL_OES_surfaceless_context: if current context has no draw_sfc
         * we still need to ensure all the offscreen rendering is completed.
         * Transport flush should be sufficient in this case.
         */
        yagl_transport_flush(yagl_get_transport(), NULL);
    }
}
