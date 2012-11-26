#include "yagl_bimage.h"
#include "yagl_log.h"
#include "yagl_malloc.h"
#include "yagl_display.h"
#include "yagl_mem.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct yagl_bimage *yagl_bimage_create(struct yagl_display *dpy,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t depth)
{
    struct yagl_bimage *res = NULL;
    XPixmapFormatValues *pfs;
    int i, num_formats;
    Visual visual;
    unsigned int image_bytes = 0;

    YAGL_LOG_FUNC_ENTER(yagl_bimage_create,
                        "width = %u, height = %u, depth = %u",
                        width, height, depth);

    res = yagl_malloc0(sizeof(*res));

    res->width = width;
    res->height = height;
    res->depth = depth;
    res->dpy = dpy;

    pfs = XListPixmapFormats(dpy->x_dpy, &num_formats);

    if (!pfs) {
        YAGL_LOG_ERROR("XListPixmapFormats failed");

        goto fail;
    }

    for (i = 0; i < num_formats; ++i) {
        if (pfs[i].depth == (int)depth) {
            res->bpp = (pfs[i].bits_per_pixel / 8);
            break;
        }
    }

    XFree(pfs);
    pfs = NULL;

    if (i == num_formats) {
        YAGL_LOG_ERROR("No suitable pixmap format found for depth %u", depth);

        goto fail;
    }

    if (dpy->xshm_images_supported) {
        res->x_image = XShmCreateImage(dpy->x_dpy,
                                       &visual,
                                       depth,
                                       ZPixmap,
                                       NULL,
                                       &res->x_shm,
                                       width,
                                       height);
        if (res->x_image) {
            image_bytes = res->x_image->bytes_per_line * res->x_image->height;

            res->x_shm.shmid = shmget(IPC_PRIVATE, image_bytes, IPC_CREAT | 0777);

            if (res->x_shm.shmid != -1) {
                res->x_shm.shmaddr = shmat(res->x_shm.shmid, 0, 0);

                if (res->x_shm.shmaddr != (char*)-1) {
                    if (XShmAttach(dpy->x_dpy, &res->x_shm)) {
                        res->pixels = res->x_image->data = res->x_shm.shmaddr;

                        goto allocated;
                    } else {
                        YAGL_LOG_ERROR("XShmAttach failed");
                    }

                    shmdt(res->x_shm.shmaddr);
                    res->x_shm.shmaddr = (char*)-1;
                } else {
                    YAGL_LOG_ERROR("shmat failed");
                }

                shmctl(res->x_shm.shmid, IPC_RMID, 0);
                res->x_shm.shmid = -1;
            } else {
                YAGL_LOG_ERROR("shmget failed");
            }

            XDestroyImage(res->x_image);
            res->x_image = NULL;
        } else {
            YAGL_LOG_ERROR("XShmCreateImage failed");
        }
    }

allocated:
    if (!dpy->xshm_images_supported || !res->x_image) {
        image_bytes = width * height * res->bpp;

        res->pixels = yagl_malloc(image_bytes);

        res->x_image = yagl_malloc0(sizeof(*res->x_image));

        res->x_image->width = res->width;
        res->x_image->height = res->height;
        res->x_image->xoffset = 0;
        res->x_image->format = ZPixmap;
        res->x_image->data = res->pixels;
        res->x_image->byte_order = LSBFirst;
        res->x_image->bitmap_unit = res->bpp * 8;
        res->x_image->bitmap_pad = 8;
        res->x_image->depth = res->depth;
        res->x_image->bytes_per_line = res->width * res->bpp;
        res->x_image->bits_per_pixel = res->bpp * 8;

        if (!XInitImage(res->x_image)) {
            YAGL_LOG_ERROR("XInitImage failed");

            yagl_free(res->x_image);
            res->x_image = NULL;

            yagl_free(res->pixels);
            res->pixels = NULL;

            goto fail;
        }
    }

    XSync(dpy->x_dpy, 0);

    if (mlock(res->pixels, res->width * res->height * res->bpp) == -1)
    {
        fprintf(stderr, "Critical error! Unable to lock YaGL bimage memory: %s!\n", strerror(errno));
        exit(1);
    }

    /*
     * Probe in immediately.
     */

    yagl_mem_probe_write(res->pixels, res->width * res->height * res->bpp);

    YAGL_LOG_FUNC_EXIT(NULL);

    return res;

fail:
    yagl_free(res);

    YAGL_LOG_FUNC_EXIT(NULL);

    return NULL;
}

void yagl_bimage_destroy(struct yagl_bimage *bi)
{
    if (munlock(bi->pixels, bi->width * bi->height * bi->bpp) == -1)
    {
        fprintf(stderr, "Critical error! Unable to unlock YaGL bimage memory: %s!\n", strerror(errno));
        exit(1);
    }

    if (bi->x_shm.shmid != -1) {
        XShmDetach(bi->dpy->x_dpy, &bi->x_shm);

        shmdt(bi->x_shm.shmaddr);
        bi->x_shm.shmaddr = (char*)-1;

        shmctl(bi->x_shm.shmid, IPC_RMID, 0);
        bi->x_shm.shmid = -1;

        XDestroyImage(bi->x_image);
        bi->x_image = NULL;

        bi->pixels = NULL;
    } else {
        yagl_free(bi->x_image);
        bi->x_image = NULL;

        yagl_free(bi->pixels);
        bi->pixels = NULL;
    }

    yagl_free(bi);
}

void yagl_bimage_draw(struct yagl_bimage *bi, Drawable target, GC gc)
{
    if (bi->x_shm.shmid != -1) {
        XShmPutImage(bi->dpy->x_dpy, target, gc, bi->x_image,
                     0, 0, 0, 0, bi->width, bi->height, 0);
    } else {
        XPutImage(bi->dpy->x_dpy, target, gc, bi->x_image,
                  0, 0, 0, 0, bi->width, bi->height);
    }

    XFlush(bi->dpy->x_dpy);
}
