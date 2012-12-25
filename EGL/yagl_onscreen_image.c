#include "yagl_onscreen_image.h"
#include "yagl_log.h"
#include "yagl_malloc.h"

static void yagl_onscreen_image_update(struct yagl_image *image)
{
}

static void yagl_onscreen_image_destroy(struct yagl_ref *ref)
{
    struct yagl_onscreen_image *image = (struct yagl_onscreen_image*)ref;

    yagl_image_cleanup(&image->base);

    yagl_free(image);
}

struct yagl_onscreen_image
    *yagl_onscreen_image_create(struct yagl_display *dpy,
                                Pixmap x_pixmap,
                                yagl_host_handle host_image)
{
    struct yagl_onscreen_image *image;

    image = yagl_malloc0(sizeof(*image));

    yagl_image_init(&image->base,
                    &yagl_onscreen_image_destroy,
                    host_image,
                    dpy,
                    x_pixmap);

    image->base.update = &yagl_onscreen_image_update;

    return image;
}
