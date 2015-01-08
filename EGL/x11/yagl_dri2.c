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

#include "yagl_dri2.h"
#include "yagl_display.h"
#include "yagl_surface.h"
#include "yagl_log.h"
#include "yagl_native_drawable.h"
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <X11/extensions/dri2proto.h>

/*
 * Allow the build to work with an older versions of dri2proto.h and
 * dri2tokens.h.
 */
#if DRI2_MINOR < 1
#undef DRI2_MINOR
#define DRI2_MINOR 1
#define X_DRI2GetBuffersWithFormat 7
#endif

static char dri2ExtensionName[] = DRI2_NAME;
static XExtensionInfo *dri2Info;
static XEXT_GENERATE_CLOSE_DISPLAY(DRI2CloseDisplay, dri2Info)

static Bool DRI2WireToEvent(Display *x_dpy, XEvent *event, xEvent *wire);
static Status DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire);
static int DRI2Error(Display *display, xError *err,
                     XExtCodes *codes, int *ret_code);

static XExtensionHooks dri2ExtensionHooks =
{
    NULL,                   /* create_gc */
    NULL,                   /* copy_gc */
    NULL,                   /* flush_gc */
    NULL,                   /* free_gc */
    NULL,                   /* create_font */
    NULL,                   /* free_font */
    DRI2CloseDisplay,       /* close_display */
    DRI2WireToEvent,        /* wire_to_event */
    DRI2EventToWire,        /* event_to_wire */
    DRI2Error,              /* error */
    NULL,                   /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY(DRI2FindDisplay,
                                  dri2Info,
                                  dri2ExtensionName,
                                  &dri2ExtensionHooks,
                                  0, NULL)

static Bool DRI2WireToEvent(Display *x_dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = DRI2FindDisplay(x_dpy);
    xDRI2InvalidateBuffers *awire;
    struct yagl_display *dpy;
    struct yagl_surface *sfc;

    YAGL_LOG_FUNC_SET(DRI2WireToEvent);

    XextCheckExtension(x_dpy, info, dri2ExtensionName, False);

    switch ((wire->u.u.type & 0x7f) - info->codes->first_event) {
#ifdef X_DRI2SwapBuffers
    case DRI2_BufferSwapComplete:
        return False;
#endif
#ifdef DRI2_InvalidateBuffers
    case DRI2_InvalidateBuffers:
        awire = (xDRI2InvalidateBuffers*)wire;
        dpy = yagl_display_get_os((yagl_os_display)x_dpy);

        if (!dpy) {
            YAGL_LOG_ERROR("EGL display not found for X display 0x%X", x_dpy);
            return False;
        }

        sfc = yagl_display_surface_acquire(dpy, (EGLSurface)awire->drawable);

        if (sfc) {
            if (sfc->type == EGL_WINDOW_BIT) {
                ++sfc->native_drawable->stamp;
                YAGL_LOG_TRACE("EGL surface 0x%X invalidated, stamp = %u",
                               awire->drawable,
                               sfc->native_drawable->stamp);
            } else {
                YAGL_LOG_ERROR("EGL surface 0x%X is not a window surface",
                               awire->drawable);
            }
        } else {
            YAGL_LOG_ERROR("EGL surface not found for X drawable 0x%X",
                           awire->drawable);
        }

        yagl_surface_release(sfc);

        return False;
#endif
    default:
        break;
    }

    return False;
}

/*
 * We don't actually support this.  It doesn't make sense for clients to
 * send each other DRI2 events.
 */
static Status DRI2EventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    switch (event->type) {
    default:
        /* client doesn't support server event */
        break;
    }

    return Success;
}

static int DRI2Error(Display *display, xError *err, XExtCodes *codes,
                     int *ret_code)
{
    if ((err->majorCode == codes->major_opcode) &&
        (err->errorCode == BadDrawable) &&
        (err->minorCode == X_DRI2CopyRegion)) {
        return True;
    }

    /*
     * If the X drawable was destroyed before the GLX drawable, the
     * DRI2 drawble will be gone by the time we call
     * DRI2DestroyDrawable.  So just ignore BadDrawable here.
     */
    if ((err->majorCode == codes->major_opcode) &&
        (err->errorCode == BadDrawable) &&
        (err->minorCode == X_DRI2DestroyDrawable)) {
        return True;
    }

    /*
     * If the server is non-local DRI2Connect will raise BadRequest.
     * Swallow this so that DRI2Connect can signal this in its return code
     */
    if ((err->majorCode == codes->major_opcode) &&
        (err->minorCode == X_DRI2Connect) &&
        (err->errorCode == BadRequest)) {
        *ret_code = False;
        return True;
    }

    return False;
}

Bool yagl_DRI2QueryExtension(Display *dpy, int *eventBase, int *errorBase)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);

    if (XextHasExtension(info)) {
        *eventBase = info->codes->first_event;
        *errorBase = info->codes->first_error;
        return True;
    }

    return False;
}

Bool yagl_DRI2QueryVersion(Display *dpy, int *major, int *minor)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2QueryVersionReply rep;
    xDRI2QueryVersionReq *req;
    int i, nevents;

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2QueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2QueryVersion;
    req->majorVersion = DRI2_MAJOR;
    req->minorVersion = DRI2_MINOR;
    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    *major = rep.majorVersion;
    *minor = rep.minorVersion;
    UnlockDisplay(dpy);
    SyncHandle();

    switch (rep.minorVersion) {
    case 1:
        nevents = 0;
        break;
    case 2:
        nevents = 1;
        break;
    case 3:
    default:
        nevents = 2;
        break;
    }

    for (i = 0; i < nevents; i++) {
        XESetWireToEvent(dpy, info->codes->first_event + i, DRI2WireToEvent);
        XESetEventToWire(dpy, info->codes->first_event + i, DRI2EventToWire);
    }

    return True;
}

Bool yagl_DRI2Connect(Display *dpy, XID window,
                      char **driverName, char **deviceName)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2ConnectReply rep;
    xDRI2ConnectReq *req;
#ifdef DRI2DriverPrimeShift
    char *prime;
#endif

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2Connect, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2Connect;
    req->window = window;

    req->driverType = DRI2DriverDRI;
#ifdef DRI2DriverPrimeShift
    prime = getenv("DRI_PRIME");
    if (prime) {
        uint32_t primeid;
        errno = 0;
        primeid = strtoul(prime, NULL, 0);
        if (errno == 0) {
            req->driverType |= ((primeid & DRI2DriverPrimeMask) << DRI2DriverPrimeShift);
        }
    }
#endif

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    if (rep.driverNameLength == 0 && rep.deviceNameLength == 0) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    *driverName = Xmalloc(rep.driverNameLength + 1);
    if (*driverName == NULL) {
        _XEatData(dpy,
                ((rep.driverNameLength + 3) & ~3) +
                ((rep.deviceNameLength + 3) & ~3));
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    _XReadPad(dpy, *driverName, rep.driverNameLength);
    (*driverName)[rep.driverNameLength] = '\0';

    *deviceName = Xmalloc(rep.deviceNameLength + 1);
    if (*deviceName == NULL) {
        Xfree(*driverName);
        _XEatData(dpy, ((rep.deviceNameLength + 3) & ~3));
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    _XReadPad(dpy, *deviceName, rep.deviceNameLength);
    (*deviceName)[rep.deviceNameLength] = '\0';

    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}

Bool yagl_DRI2Authenticate(Display *dpy, XID window, drm_magic_t magic)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2AuthenticateReq *req;
    xDRI2AuthenticateReply rep;

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2Authenticate, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2Authenticate;
    req->window = window;
    req->magic = magic;

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return rep.authenticated;
}

void yagl_DRI2CreateDrawable(Display *dpy, XID drawable)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2CreateDrawableReq *req;

    XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

    LockDisplay(dpy);
    GetReq(DRI2CreateDrawable, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2CreateDrawable;
    req->drawable = drawable;
    UnlockDisplay(dpy);
    SyncHandle();
}

void yagl_DRI2DestroyDrawable(Display *dpy, XID drawable)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2DestroyDrawableReq *req;

    XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

    XSync(dpy, False);

    LockDisplay(dpy);
    GetReq(DRI2DestroyDrawable, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2DestroyDrawable;
    req->drawable = drawable;
    UnlockDisplay(dpy);
    SyncHandle();
}

yagl_DRI2Buffer *yagl_DRI2GetBuffers(Display *dpy, XID drawable,
                                     int *width, int *height,
                                     unsigned int *attachments, int count,
                                     int *outCount)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2GetBuffersReply rep;
    xDRI2GetBuffersReq *req;
    yagl_DRI2Buffer *buffers;
    xDRI2Buffer repBuffer;
    CARD32 *p;
    int i;

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReqExtra(DRI2GetBuffers, count * 4, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2GetBuffers;
    req->drawable = drawable;
    req->count = count;
    p = (CARD32*)&req[1];
    for (i = 0; i < count; i++) {
        p[i] = attachments[i];
    }

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return NULL;
    }

    *width = rep.width;
    *height = rep.height;
    *outCount = rep.count;

    buffers = Xmalloc(rep.count * sizeof buffers[0]);
    if (buffers == NULL) {
        _XEatData(dpy, rep.count * sizeof repBuffer);
        UnlockDisplay(dpy);
        SyncHandle();
        return NULL;
    }

    for (i = 0; i < (int)rep.count; i++) {
        _XReadPad(dpy, (char*)&repBuffer, sizeof repBuffer);
        buffers[i].attachment = repBuffer.attachment;
        buffers[i].name = repBuffer.name;
        buffers[i].pitch = repBuffer.pitch;
        buffers[i].cpp = repBuffer.cpp;
        buffers[i].flags = repBuffer.flags;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return buffers;
}


yagl_DRI2Buffer
    *yagl_DRI2GetBuffersWithFormat(Display *dpy, XID drawable,
                                   int *width, int *height,
                                   unsigned int *attachments, int count,
                                   int *outCount)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2GetBuffersReply rep;
    xDRI2GetBuffersReq *req;
    yagl_DRI2Buffer *buffers;
    xDRI2Buffer repBuffer;
    CARD32 *p;
    int i;

    XextCheckExtension(dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReqExtra(DRI2GetBuffers, count * (4 * 2), req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2GetBuffersWithFormat;
    req->drawable = drawable;
    req->count = count;
    p = (CARD32*)&req[1];
    for (i = 0; i < (count * 2); i++) {
        p[i] = attachments[i];
    }

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return NULL;
    }

    *width = rep.width;
    *height = rep.height;
    *outCount = rep.count;

    buffers = Xmalloc(rep.count * sizeof buffers[0]);
    if (buffers == NULL) {
        _XEatData(dpy, rep.count * sizeof repBuffer);
        UnlockDisplay(dpy);
        SyncHandle();
        return NULL;
    }

    for (i = 0; i < (int)rep.count; i++) {
        _XReadPad(dpy, (char *) &repBuffer, sizeof repBuffer);
        buffers[i].attachment = repBuffer.attachment;
        buffers[i].name = repBuffer.name;
        buffers[i].pitch = repBuffer.pitch;
        buffers[i].cpp = repBuffer.cpp;
        buffers[i].flags = repBuffer.flags;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    return buffers;
}

void yagl_DRI2CopyRegion(Display *dpy, XID drawable, XserverRegion region,
                         CARD32 dest, CARD32 src)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2CopyRegionReq *req;
    xDRI2CopyRegionReply rep;

    XextSimpleCheckExtension(dpy, info, dri2ExtensionName);

    LockDisplay(dpy);
    GetReq(DRI2CopyRegion, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2CopyRegion;
    req->drawable = drawable;
    req->region = region;
    req->dest = dest;
    req->src = src;

    _XReply(dpy, (xReply *) & rep, 0, xFalse);

    UnlockDisplay(dpy);
    SyncHandle();
}

#ifdef X_DRI2SwapBuffers
static void load_swap_req(xDRI2SwapBuffersReq *req,
                          CARD64 target, CARD64 divisor,
                          CARD64 remainder)
{
    req->target_msc_hi = target >> 32;
    req->target_msc_lo = target & 0xffffffff;
    req->divisor_hi = divisor >> 32;
    req->divisor_lo = divisor & 0xffffffff;
    req->remainder_hi = remainder >> 32;
    req->remainder_lo = remainder & 0xffffffff;
}

static CARD64 vals_to_card64(CARD32 lo, CARD32 hi)
{
    return (CARD64)hi << 32 | lo;
}

void yagl_DRI2SwapBuffers(Display *dpy, XID drawable, CARD64 target_msc,
                          CARD64 divisor, CARD64 remainder, CARD64 *count)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2SwapBuffersReq *req;
    xDRI2SwapBuffersReply rep;

    XextSimpleCheckExtension (dpy, info, dri2ExtensionName);

    LockDisplay(dpy);
    GetReq(DRI2SwapBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2SwapBuffers;
    req->drawable = drawable;
    load_swap_req(req, target_msc, divisor, remainder);

    _XReply(dpy, (xReply*)&rep, 0, xFalse);

    *count = vals_to_card64(rep.swap_lo, rep.swap_hi);

    UnlockDisplay(dpy);
    SyncHandle();
}
#endif

#ifdef X_DRI2GetMSC
Bool yagl_DRI2GetMSC(Display *dpy, XID drawable, CARD64 *ust, CARD64 *msc,
                     CARD64 *sbc)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2GetMSCReq *req;
    xDRI2MSCReply rep;

    XextCheckExtension (dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2GetMSC, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2GetMSC;
    req->drawable = drawable;

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    *ust = vals_to_card64(rep.ust_lo, rep.ust_hi);
    *msc = vals_to_card64(rep.msc_lo, rep.msc_hi);
    *sbc = vals_to_card64(rep.sbc_lo, rep.sbc_hi);

    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}
#endif

#ifdef X_DRI2WaitMSC
static void load_msc_req(xDRI2WaitMSCReq *req, CARD64 target, CARD64 divisor,
                         CARD64 remainder)
{
    req->target_msc_hi = target >> 32;
    req->target_msc_lo = target & 0xffffffff;
    req->divisor_hi = divisor >> 32;
    req->divisor_lo = divisor & 0xffffffff;
    req->remainder_hi = remainder >> 32;
    req->remainder_lo = remainder & 0xffffffff;
}

Bool yagl_DRI2WaitMSC(Display *dpy, XID drawable,
                      CARD64 target_msc, CARD64 divisor,
                      CARD64 remainder, CARD64 *ust, CARD64 *msc, CARD64 *sbc)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2WaitMSCReq *req;
    xDRI2MSCReply rep;

    XextCheckExtension (dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2WaitMSC, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2WaitMSC;
    req->drawable = drawable;
    load_msc_req(req, target_msc, divisor, remainder);

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    *ust = ((CARD64)rep.ust_hi << 32) | (CARD64)rep.ust_lo;
    *msc = ((CARD64)rep.msc_hi << 32) | (CARD64)rep.msc_lo;
    *sbc = ((CARD64)rep.sbc_hi << 32) | (CARD64)rep.sbc_lo;

    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}
#endif

#ifdef X_DRI2WaitSBC
static void load_sbc_req(xDRI2WaitSBCReq *req, CARD64 target)
{
    req->target_sbc_hi = target >> 32;
    req->target_sbc_lo = target & 0xffffffff;
}

Bool yagl_DRI2WaitSBC(Display *dpy, XID drawable, CARD64 target_sbc, CARD64 *ust,
                      CARD64 *msc, CARD64 *sbc)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2WaitSBCReq *req;
    xDRI2MSCReply rep;

    XextCheckExtension (dpy, info, dri2ExtensionName, False);

    LockDisplay(dpy);
    GetReq(DRI2WaitSBC, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2WaitSBC;
    req->drawable = drawable;
    load_sbc_req(req, target_sbc);

    if (!_XReply(dpy, (xReply*)&rep, 0, xFalse)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }

    *ust = ((CARD64)rep.ust_hi << 32) | rep.ust_lo;
    *msc = ((CARD64)rep.msc_hi << 32) | rep.msc_lo;
    *sbc = ((CARD64)rep.sbc_hi << 32) | rep.sbc_lo;

    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}
#endif

#ifdef X_DRI2SwapInterval
void yagl_DRI2SwapInterval(Display *dpy, XID drawable, int interval)
{
    XExtDisplayInfo *info = DRI2FindDisplay(dpy);
    xDRI2SwapIntervalReq *req;

    XextSimpleCheckExtension (dpy, info, dri2ExtensionName);

    LockDisplay(dpy);
    GetReq(DRI2SwapInterval, req);
    req->reqType = info->codes->major_opcode;
    req->dri2ReqType = X_DRI2SwapInterval;
    req->drawable = drawable;
    req->interval = interval;
    UnlockDisplay(dpy);
    SyncHandle();
}
#endif
