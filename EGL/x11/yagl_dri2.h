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

#ifndef _YAGL_DRI2_H_
#define _YAGL_DRI2_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/dri2tokens.h>
#include <X11/Xmd.h>
#include <X11/Xlibint.h>
#include <xf86drm.h>

typedef struct
{
   unsigned int attachment;
   unsigned int name;
   unsigned int pitch;
   unsigned int cpp;
   unsigned int flags;
} yagl_DRI2Buffer;

Bool yagl_DRI2QueryExtension(Display *display,
                             int *eventBase,
                             int *errorBase);

Bool yagl_DRI2QueryVersion(Display *display,
                           int *major,
                           int *minor);

Bool yagl_DRI2Connect(Display *display,
                      XID window,
                      char **driverName,
                      char **deviceName);

Bool yagl_DRI2Authenticate(Display *display,
                           XID window,
                           drm_magic_t magic);

void yagl_DRI2CreateDrawable(Display *display, XID drawable);

void yagl_DRI2DestroyDrawable(Display *display, XID handle);

yagl_DRI2Buffer *yagl_DRI2GetBuffers(Display *dpy, XID drawable,
                                     int *width, int *height,
                                     unsigned int *attachments, int count,
                                     int *outCount);

yagl_DRI2Buffer *yagl_DRI2GetBuffersWithFormat(Display *dpy,
                                               XID drawable,
                                               int *width, int *height,
                                               unsigned int *attachments,
                                               int count, int *outCount);

void yagl_DRI2CopyRegion(Display *dpy,
                         XID drawable,
                         XserverRegion region,
                         CARD32 dest, CARD32 src);

void yagl_DRI2SwapBuffers(Display *dpy, XID drawable,
                          CARD64 target_msc, CARD64 divisor,
                          CARD64 remainder, CARD64 *count);

Bool yagl_DRI2GetMSC(Display *dpy, XID drawable,
                     CARD64 *ust, CARD64 *msc, CARD64 *sbc);

Bool yagl_DRI2WaitMSC(Display *dpy, XID drawable,
                      CARD64 target_msc, CARD64 divisor,
                      CARD64 remainder, CARD64 *ust, CARD64 *msc, CARD64 *sbc);

Bool yagl_DRI2WaitSBC(Display *dpy, XID drawable,
                      CARD64 target_sbc, CARD64 *ust,
                      CARD64 *msc, CARD64 *sbc);

void yagl_DRI2SwapInterval(Display *dpy, XID drawable, int interval);

#endif
