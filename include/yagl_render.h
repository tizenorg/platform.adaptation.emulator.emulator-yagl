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

#ifndef _YAGL_RENDER_H_
#define _YAGL_RENDER_H_

#include "yagl_export.h"
#include "yagl_types.h"

/*
 * From http://www.x.org/wiki/DRI2:
 * "When and how does a client detect that a new front buffer has
 * been attached? The Gallium Design frowns on the SAREA timestamp mechanism
 * because it greatly complicates the DRI driver implementation, but only
 * checking on glXSwapBuffers() isn't sufficient. Consider an application that
 * (unlike, say, glxgears) doesn't render as many frames per second as
 * possible, but uses OpenGL as a way to redraw it's interface in response
 * to X events (mouse clicks, window resizing). When resizing the window of
 * such an application in a composited environment, the front buffer is
 * reallocated and attached and the application is sent an expose event.
 * The application will re-layout the interface and re-render it, however,
 * the DRI driver won't update its back buffers until glXSwapBuffer() is called
 * at which point it's too late. This application will always be a frame behind.
 *
 * This is somewhat of an implementation problem for the 3d client and/or
 * gallium architecture design -- it looks like there will be times when it is
 * necessary to check window dimensions apart from swapbuffers, and in the case
 * of the above scenario the application will give us a very good hint by
 * adjusting the viewport parameters. More generally we can say we want to
 * check at two places -- SwapBuffers, and immediately before the very first
 * piece of rendering after swapbuffers. This will catch the application-redraw
 * case, and won't hurt performance as long as the mechanism for
 * checking window size continues to be fast."
 *
 * This function is intended to be called "immediately before the very first
 * piece of rendering after swapbuffers".
 */
YAGL_API void yagl_render_invalidate(int throttle);


/*
 * This must be called when client API finished rendering part of
 * the geometry and now it's guaranteed that by that point the rendering
 * results will be visible to direct rendering clients and X.Org. Callers
 * of this should probably include glFinish, eglWaitClient, eglWaitGL, etc.
 */
YAGL_API void yagl_render_finish();

#endif
