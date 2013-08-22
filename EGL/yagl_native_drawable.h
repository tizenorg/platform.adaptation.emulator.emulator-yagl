#ifndef _YAGL_NATIVE_DRAWABLE_H_
#define _YAGL_NATIVE_DRAWABLE_H_

#include "yagl_export.h"
#include "yagl_native_types.h"

struct yagl_native_image;
struct vigs_drm_surface;

struct yagl_native_drawable
{
    struct yagl_native_display *dpy;

    yagl_os_drawable os_drawable;

    /*
     * This gets incremented in drawable invalidate handler.
     */
    uint32_t stamp;

    int (*get_buffer)(struct yagl_native_drawable */*drawable*/,
                      yagl_native_attachment /*attachment*/,
                      uint32_t */*buffer_name*/,
                      struct vigs_drm_surface **/*buffer_sfc*/);

    int (*get_buffer_age)(struct yagl_native_drawable */*drawable*/);

    void (*swap_buffers)(struct yagl_native_drawable */*drawable*/);

    /*
     * 'width' and 'height' are here only because of DRI2.
     * DRI2 requires width and height to be passed to DRI2CopyRegion.
     */
    void (*wait)(struct yagl_native_drawable */*drawable*/,
                 uint32_t /*width*/,
                 uint32_t /*height*/);

    void (*copy_to_pixmap)(struct yagl_native_drawable */*drawable*/,
                           yagl_os_pixmap /*os_pixmap*/,
                           uint32_t /*from_x*/,
                           uint32_t /*from_y*/,
                           uint32_t /*to_x*/,
                           uint32_t /*to_y*/,
                           uint32_t /*width*/,
                           uint32_t /*height*/);

    void (*set_swap_interval)(struct yagl_native_drawable */*drawable*/,
                              int /*interval*/);

    void (*get_geometry)(struct yagl_native_drawable */*drawable*/,
                         uint32_t */*width*/,
                         uint32_t */*height*/,
                         uint32_t */*depth*/);

    struct yagl_native_image *(*get_image)(struct yagl_native_drawable */*drawable*/,
                                           uint32_t /*width*/,
                                           uint32_t /*height*/);

    void (*destroy)(struct yagl_native_drawable */*drawable*/);
};

void yagl_native_drawable_init(struct yagl_native_drawable *drawable,
                               struct yagl_native_display *dpy,
                               yagl_os_drawable os_drawable);

void yagl_native_drawable_cleanup(struct yagl_native_drawable *drawable);

#endif
