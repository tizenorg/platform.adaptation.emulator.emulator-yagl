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
