#ifndef _YAGL_ONSCREEN_SURFACE_H_
#define _YAGL_ONSCREEN_SURFACE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_surface.h"
#include "yagl_dri2.h"

struct yagl_onscreen_surface
{
    struct yagl_surface base;

    /*
     * Backing pixmap for PBuffer surfaces. NULL otherwise.
     */
    Pixmap pbuffer_pixmap;

    /*
     * For widow surfaces this is DRI2BufferBackLeft.
     * For pixmap surfaces this is DRI2BufferFrontLeft.
     * For pbuffer surfaces this is DRI2BufferFrontLeft of 'pbuffer_pixmap'.
     *
     * TODO: For window surfaces we also need to support
     * DRI2BufferFrontLeft.
     */
    yagl_DRI2Buffer *buffer;

    /*
     * 'buffer' dimensions.
     * @{
     */
    uint32_t width;
    uint32_t height;
    /*
     * @}
     */

    /*
     * This gets incremented in DRI2 invalidate handler, compare it to
     * 'last_timestamp' in order to find out if an invalidate event
     * had occurred.
     */
    uint32_t stamp;

    /*
     * Last value of 'stamp'.
     */
    uint32_t last_stamp;
};

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_window(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Window x_win,
                                         const EGLint* attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pixmap(struct yagl_display *dpy,
                                         yagl_host_handle host_config,
                                         Pixmap x_pixmap,
                                         const EGLint* attrib_list);

struct yagl_onscreen_surface
    *yagl_onscreen_surface_create_pbuffer(struct yagl_display *dpy,
                                          yagl_host_handle host_config,
                                          const EGLint* attrib_list);

#endif
