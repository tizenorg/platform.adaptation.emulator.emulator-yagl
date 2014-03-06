#include "yagl_gles3_sync.h"
#include "yagl_egl_fence.h"
#include "yagl_malloc.h"

static void yagl_gles3_sync_destroy(struct yagl_ref *ref)
{
    struct yagl_gles3_sync *sync = (struct yagl_gles3_sync*)ref;

    yagl_egl_fence_release(sync->egl_fence);

    yagl_object_cleanup(&sync->base);

    yagl_free(sync);
}

struct yagl_gles3_sync *yagl_gles3_sync_create(void)
{
    struct yagl_egl_fence *egl_fence;
    struct yagl_gles3_sync *sync;

    egl_fence = yagl_create_egl_fence();

    if (!egl_fence) {
        return NULL;
    }

    sync = yagl_malloc0(sizeof(*sync));

    yagl_object_init(&sync->base, &yagl_gles3_sync_destroy);

    sync->egl_fence = egl_fence;

    return sync;
}

void yagl_gles3_sync_acquire(struct yagl_gles3_sync *sync)
{
    if (sync) {
        yagl_object_acquire(&sync->base);
    }
}

void yagl_gles3_sync_release(struct yagl_gles3_sync *sync)
{
    if (sync) {
        yagl_object_release(&sync->base);
    }
}
