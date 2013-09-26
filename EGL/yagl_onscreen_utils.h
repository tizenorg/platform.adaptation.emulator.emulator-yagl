#ifndef _YAGL_ONSCREEN_UTILS_H_
#define _YAGL_ONSCREEN_UTILS_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_native_types.h"

struct vigs_drm_surface;
struct yagl_native_drawable;

struct vigs_drm_surface
    *yagl_onscreen_buffer_create(struct yagl_native_drawable *native_drawable,
                                 yagl_native_attachment attachment,
                                 struct vigs_drm_surface *check_sfc);

#endif
