#ifndef _YAGL_DRI3_H_
#define _YAGL_DRI3_H_

#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#include <xcb/sync.h>
#include "yagl_native_drawable.h"

enum yagl_dri3_buffer_type {
    yagl_dri3_buffer_back = 0,
    yagl_dri3_buffer_front = 1
};

struct vigs_drm_surface;

struct yagl_dri3_buffer {
    /* Pixmap ID */
    uint32_t pixmap;

    /* Buffer type: back / front */
    enum yagl_dri3_buffer_type buffer_type;

    /* Backing storage */
    struct vigs_drm_surface *sfc;

    /* Synchronization between the client and X server is done using an
     * xshmfence that is mapped into an X server SyncFence. This lets the
     * client check whether the X server is done using a buffer with a simple
     * xshmfence call, rather than going to read X events from the wire.
     *
     * However, we can only wait for one xshmfence to be triggered at a time,
     * so we need to know *which* buffer is going to be idle next. We do that
     * by waiting for a PresentIdleNotify event. When that event arrives, the
     * 'busy' flag gets cleared and the client knows that the fence has been
     * triggered, and that the wait call will not block.
     */

    /* XID of X SyncFence object */
    uint32_t sync_fence;

    /* Pointer to xshmfence object */
    struct xshmfence *shm_fence;

    /* Set on swap, cleared on IdleNotify */
    int busy;

    /* We allocated the pixmap ID, free on destroy */
    int own_pixmap;

    uint32_t pitch;
    uint32_t width, height;
    uint64_t last_swap;
};

#define DRI3_MAX_BACK 4
#define DRI3_BACK_ID(i) (i)
#define DRI3_FRONT_ID (DRI3_MAX_BACK)
#define DRI3_NUM_BUFFERS (1 + DRI3_MAX_BACK)

struct yagl_dri3_drawable {
    struct yagl_native_drawable base;

    int width, height, depth;
    int swap_interval;
    uint8_t have_back;
    uint8_t own_drawable;
    uint8_t is_pixmap;
    uint8_t flipping;

    /* Present extension capabilities */
    uint32_t present_capabilities;

    /* SBC numbers are tracked by using the serial numbers
     * in the present request and complete events */
    uint64_t send_sbc;
    uint64_t recv_sbc;

    /* Last received UST/MSC values for pixmap present complete */
    uint64_t ust, msc;

    /* Last received UST/MSC values from present notify msc event */
    uint64_t notify_ust, notify_msc;

    /* Serial numbers for tracking wait_for_msc events */
    uint32_t send_msc_serial;
    uint32_t recv_msc_serial;

    struct yagl_dri3_buffer *buffers[DRI3_NUM_BUFFERS];
    int cur_back;
    int num_back;

    xcb_present_event_t eid;
    xcb_gcontext_t gc;
    xcb_special_event_t *special_event;
};

int yagl_dri3_display_init(Display *x_dpy, char **dri_device);

struct yagl_native_drawable *yagl_dri3_drawable_create(struct yagl_native_display *dpy,
                                                       yagl_os_drawable os_drawable,
                                                       int own_drawable,
                                                       int is_pixmap);

GC yagl_dri3_drawable_get_gc(struct yagl_native_drawable *drawable);

#endif /* _YAGL_DRI3_H_ */
