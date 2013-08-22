#ifndef _YAGL_GBM_H_
#define _YAGL_GBM_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <gbm.h>

struct vigs_drm_device;
struct vigs_drm_surface;

struct gbm_device
{
    /*
     * Hack to make gbm_device detectable by its first element.
     */
    struct gbm_device *(*dummy)(int);

    struct vigs_drm_device *drm_dev;
};

struct gbm_bo
{
    struct gbm_device *gbm;

    struct vigs_drm_surface *drm_sfc;

    uint32_t depth;
};

struct gbm_surface
{
    struct gbm_device *gbm;

    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t format;
    uint32_t flags;

    struct vigs_drm_surface *(*acquire_back)(struct gbm_surface */*sfc*/);

    void (*swap_buffers)(struct gbm_surface */*sfc*/);

    int (*get_buffer_age)(struct gbm_surface */*sfc*/);
};

#endif
