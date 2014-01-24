#include "yagl_onscreen_fence.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_native_display.h"
#include "vigs.h"
#include <string.h>

static int yagl_onscreen_fence_wait(struct yagl_egl_fence *egl_fence)
{
    struct yagl_onscreen_fence *ofence = (struct yagl_onscreen_fence*)egl_fence;
    int ret;

    YAGL_LOG_FUNC_SET(eglClientWaitSyncKHR);

    ret = vigs_drm_fence_wait(ofence->drm_fence);

    if (ret == 0) {
        ofence->signaled = 1;
    } else {
        YAGL_LOG_ERROR("vigs_drm_fence_wait failed: %s",
                       strerror(-ret));
    }

    return (ret == 0);
}

static int yagl_onscreen_fence_signaled(struct yagl_egl_fence *egl_fence)
{
    struct yagl_onscreen_fence *ofence = (struct yagl_onscreen_fence*)egl_fence;
    int ret;

    YAGL_LOG_FUNC_SET(eglGetSyncAttribKHR);

    if (ofence->signaled) {
        return 1;
    }

    ret = vigs_drm_fence_check(ofence->drm_fence);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_fence_check failed: %s",
                       strerror(-ret));
    }

    ofence->signaled = ofence->drm_fence->signaled;

    return ofence->signaled;
}

static void yagl_onscreen_fence_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_fence *fence = (struct yagl_onscreen_fence*)ref;

    vigs_drm_fence_unref(fence->drm_fence);

    yagl_fence_cleanup(&fence->base);

    yagl_free(fence);
}

struct yagl_onscreen_fence
    *yagl_onscreen_fence_create(struct yagl_display *dpy)
{
    struct yagl_onscreen_fence *fence;
    int ret;

    YAGL_LOG_FUNC_SET(eglCreateSyncKHR);

    fence = yagl_malloc0(sizeof(*fence));

    ret = vigs_drm_fence_create(dpy->native_dpy->drm_dev, 0, &fence->drm_fence);

    if (ret != 0) {
        YAGL_LOG_ERROR("vigs_drm_fence_create failed: %s",
                       strerror(-ret));
        goto fail;
    }

    yagl_fence_init(&fence->base,
                    &yagl_onscreen_fence_destroy,
                    dpy,
                    fence->drm_fence->seq);

    fence->base.base.wait = &yagl_onscreen_fence_wait;
    fence->base.base.signaled = &yagl_onscreen_fence_signaled;

    return fence;

fail:
    yagl_free(fence);

    return NULL;
}
