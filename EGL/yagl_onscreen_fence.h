#ifndef _YAGL_ONSCREEN_FENCE_H_
#define _YAGL_ONSCREEN_FENCE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_fence.h"

struct vigs_drm_fence;

struct yagl_onscreen_fence
{
    struct yagl_fence base;

    struct vigs_drm_fence *drm_fence;

    int signaled;
};

struct yagl_onscreen_fence
    *yagl_onscreen_fence_create(struct yagl_display *dpy);

#endif
