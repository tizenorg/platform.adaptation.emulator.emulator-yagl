#ifndef _YAGL_ONSCREEN_BUFFER_H_
#define _YAGL_ONSCREEN_BUFFER_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_native_types.h"

struct vigs_drm_surface;
struct yagl_onscreen_display;
struct yagl_native_drawable;

struct yagl_onscreen_buffer
{
    uint32_t name;

    struct vigs_drm_surface *drm_sfc;
};

struct yagl_onscreen_buffer
    *yagl_onscreen_buffer_create(struct yagl_onscreen_display *dpy,
                                 struct yagl_native_drawable *native_drawable,
                                 yagl_native_attachment attachment,
                                 uint32_t check_name);

void yagl_onscreen_buffer_destroy(struct yagl_onscreen_buffer *buffer);

#endif
