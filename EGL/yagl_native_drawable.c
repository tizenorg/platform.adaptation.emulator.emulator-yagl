#include "yagl_native_drawable.h"

void yagl_native_drawable_init(struct yagl_native_drawable *drawable,
                               struct yagl_native_display *dpy,
                               yagl_os_drawable os_drawable)
{
    drawable->dpy = dpy;
    drawable->os_drawable = os_drawable;
    drawable->stamp = 0;
}

void yagl_native_drawable_cleanup(struct yagl_native_drawable *drawable)
{
}
