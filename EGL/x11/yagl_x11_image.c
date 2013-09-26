#include "yagl_x11_image.h"
#include "yagl_x11_display.h"
#include "yagl_x11_drawable.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void yagl_x11_image_draw_internal(struct yagl_x11_image *image,
                                         Drawable target,
                                         GC x_gc)
{
    Display *x_dpy = YAGL_X11_DPY(image->base.dpy->os_dpy);

    if (image->x_shm.shmid != -1) {
        XShmPutImage(x_dpy, target, x_gc, image->x_image,
                     0, 0, 0, 0, image->base.width, image->base.height, 0);
    } else {
        XPutImage(x_dpy, target, x_gc, image->x_image,
                  0, 0, 0, 0, image->base.width, image->base.height);
    }

    XFlush(x_dpy);
}

static void yagl_x11_image_draw(struct yagl_native_image *image,
                                struct yagl_native_drawable *drawable)
{
    struct yagl_x11_image *x11_image = (struct yagl_x11_image*)image;
    struct yagl_x11_drawable *x11_drawable = (struct yagl_x11_drawable*)drawable;
    Drawable x_drawable = YAGL_X11_DRAWABLE(drawable->os_drawable);
    GC x_gc = yagl_x11_drawable_get_gc(x11_drawable);

    if (x_gc) {
        yagl_x11_image_draw_internal(x11_image, x_drawable, x_gc);
    }
}

static void yagl_x11_image_draw_to_pixmap(struct yagl_native_image *image,
                                          yagl_os_pixmap os_pixmap)
{
    struct yagl_x11_image *x11_image = (struct yagl_x11_image*)image;
    Display *x_dpy = YAGL_X11_DPY(image->dpy->os_dpy);
    Pixmap x_pixmap = YAGL_X11_DRAWABLE(os_pixmap);
    GC x_gc = XCreateGC(x_dpy, x_pixmap, 0, NULL);

    if (x_gc) {
        yagl_x11_image_draw_internal(x11_image, x_pixmap, x_gc);
        XFreeGC(x_dpy, x_gc);
    }
}

static void yagl_x11_image_destroy(struct yagl_native_image *image)
{
    struct yagl_x11_image *x11_image = (struct yagl_x11_image*)image;

    if (x11_image->is_wrapped) {
        XDestroyImage(x11_image->x_image);
        x11_image->x_image = NULL;
    } else {
        Display *x_dpy = YAGL_X11_DPY(image->dpy->os_dpy);

        if (x11_image->x_shm.shmid != -1) {
            XShmDetach(x_dpy, &x11_image->x_shm);

            shmdt(x11_image->x_shm.shmaddr);
            x11_image->x_shm.shmaddr = (char*)-1;

            shmctl(x11_image->x_shm.shmid, IPC_RMID, 0);
            x11_image->x_shm.shmid = -1;

            XDestroyImage(x11_image->x_image);
            x11_image->x_image = NULL;
        } else {
            yagl_free(x11_image->x_image);
            x11_image->x_image = NULL;

            yagl_free(image->pixels);
        }
    }

    yagl_native_image_cleanup(image);

    yagl_free(x11_image);
}

struct yagl_native_image
    *yagl_x11_image_create(struct yagl_native_display *dpy,
                           uint32_t width,
                           uint32_t height,
                           uint32_t depth)
{
    struct yagl_x11_display *x11_dpy = (struct yagl_x11_display*)dpy;
    Display *x_dpy = YAGL_X11_DPY(dpy->os_dpy);
    struct yagl_x11_image *image;
    XPixmapFormatValues *pfs;
    int i, num_formats;
    Visual visual;
    unsigned int image_bytes = 0;
    void *pixels = NULL;
    uint32_t bpp = 0;

    YAGL_LOG_FUNC_ENTER(yagl_x11_image_create,
                        "width = %u, height = %u, depth = %u",
                        width, height, depth);

    image = yagl_malloc0(sizeof(*image));

    pfs = XListPixmapFormats(x_dpy, &num_formats);

    if (!pfs) {
        YAGL_LOG_ERROR("XListPixmapFormats failed");

        goto fail;
    }

    for (i = 0; i < num_formats; ++i) {
        if (pfs[i].depth == (int)depth) {
            bpp = (pfs[i].bits_per_pixel / 8);
            break;
        }
    }

    XFree(pfs);
    pfs = NULL;

    if (i == num_formats) {
        YAGL_LOG_ERROR("No suitable pixmap format found for depth %u", depth);

        goto fail;
    }

    if (x11_dpy->xshm_images_supported) {
        image->x_image = XShmCreateImage(x_dpy,
                                         &visual,
                                         depth,
                                         ZPixmap,
                                         NULL,
                                         &image->x_shm,
                                         width,
                                         height);
        if (image->x_image) {
            image_bytes = image->x_image->bytes_per_line * image->x_image->height;

            image->x_shm.shmid = shmget(IPC_PRIVATE,
                                        image_bytes,
                                        IPC_CREAT | 0777);

            if (image->x_shm.shmid != -1) {
                image->x_shm.shmaddr = shmat(image->x_shm.shmid, 0, 0);

                if (image->x_shm.shmaddr != (char*)-1) {
                    if (XShmAttach(x_dpy, &image->x_shm)) {
                        pixels = image->x_image->data = image->x_shm.shmaddr;

                        goto allocated;
                    } else {
                        YAGL_LOG_ERROR("XShmAttach failed");
                    }

                    shmdt(image->x_shm.shmaddr);
                    image->x_shm.shmaddr = (char*)-1;
                } else {
                    YAGL_LOG_ERROR("shmat failed");
                }

                shmctl(image->x_shm.shmid, IPC_RMID, 0);
                image->x_shm.shmid = -1;
            } else {
                YAGL_LOG_ERROR("shmget failed");
            }

            XDestroyImage(image->x_image);
            image->x_image = NULL;
        } else {
            YAGL_LOG_ERROR("XShmCreateImage failed");
        }
    }

allocated:
    if (!image->x_image) {
        image->x_shm.shmid = -1;

        image_bytes = width * height * bpp;

        pixels = yagl_malloc(image_bytes);

        image->x_image = yagl_malloc0(sizeof(*image->x_image));

        image->x_image->width = width;
        image->x_image->height = height;
        image->x_image->xoffset = 0;
        image->x_image->format = ZPixmap;
        image->x_image->data = pixels;
        image->x_image->byte_order = LSBFirst;
        image->x_image->bitmap_unit = bpp * 8;
        image->x_image->bitmap_pad = 8;
        image->x_image->depth = depth;
        image->x_image->bytes_per_line = width * bpp;
        image->x_image->bits_per_pixel = bpp * 8;

        if (!XInitImage(image->x_image)) {
            YAGL_LOG_ERROR("XInitImage failed");

            yagl_free(image->x_image);
            image->x_image = NULL;

            yagl_free(pixels);
            pixels = NULL;

            goto fail;
        }
    }

    XSync(x_dpy, 0);

    yagl_native_image_init(&image->base,
                           dpy,
                           width,
                           height,
                           depth,
                           bpp,
                           pixels);

    image->base.draw = &yagl_x11_image_draw;
    image->base.draw_to_pixmap = &yagl_x11_image_draw_to_pixmap;
    image->base.destroy = &yagl_x11_image_destroy;

    image->is_wrapped = 0;

    YAGL_LOG_FUNC_EXIT(NULL);

    return &image->base;

fail:
    yagl_free(image);

    YAGL_LOG_FUNC_EXIT(NULL);

    return NULL;
}

struct yagl_native_image
    *yagl_x11_image_wrap(struct yagl_native_display *dpy,
                         XImage *x_image)
{
    struct yagl_x11_image *image;

    image = yagl_malloc0(sizeof(*image));

    yagl_native_image_init(&image->base,
                           dpy,
                           x_image->width,
                           x_image->height,
                           x_image->depth,
                           (x_image->bits_per_pixel / 8),
                           x_image->data);

    image->base.draw = &yagl_x11_image_draw;
    image->base.draw_to_pixmap = &yagl_x11_image_draw_to_pixmap;
    image->base.destroy = &yagl_x11_image_destroy;

    image->x_image = x_image;
    image->x_shm.shmid = -1;
    image->is_wrapped = 1;

    return &image->base;
}
