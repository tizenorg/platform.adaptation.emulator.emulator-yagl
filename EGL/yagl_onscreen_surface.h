#ifndef _YAGL_ONSCREEN_SURFACE_H_
#define _YAGL_ONSCREEN_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"

struct vigs_drm_surface;

struct yagl_onscreen_surface
{
    struct yagl_surface base;

    /*
     * Backing pixmap for PBuffer surfaces. NULL otherwise.
     */
    struct yagl_native_drawable *tmp_pixmap;

    /*
     * For widow surfaces this is yagl_native_attachment_back.
     * For pixmap surfaces this is yagl_native_attachment_front.
     * For pbuffer surfaces this is yagl_native_attachment_front of 'tmp_pixmap'.
     *
     * TODO: For window surfaces we also need to support
     * yagl_native_attachment_front.
     */
    struct vigs_drm_surface *drm_sfc;

    /*
     * Last value of 'base.native_drawable->stamp'.
     */
    uint32_t last_stamp;
};

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_window,
                                         const EGLint *attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         struct yagl_native_drawable *native_pixmap,
                                         const EGLint *attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint *attrib_list);

#endif
