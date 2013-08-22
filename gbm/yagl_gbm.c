#include "yagl_export.h"
#include "yagl_gbm.h"
#include "vigs.h"
#ifdef YAGL_PLATFORM_WAYLAND
#include "wayland-drm.h"
#endif
#include <gbm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct yagl_gbm_bo
{
    struct gbm_bo base;

    uint32_t format;
    union gbm_bo_handle handle;

    void *user_data;

    void (*destroy_user_data)(struct gbm_bo */*bo*/,
                              void */*user_data*/);
};

struct yagl_gbm_surface
{
    struct gbm_surface base;

    struct
    {
        struct gbm_bo *bo;
        int locked;
    } color_buffers[3], *back, *front;
};

static void* gbm_malloc0(size_t size)
{
    void *tmp = malloc(size);
    if (!tmp) {
        fprintf(stderr,
                "GBM: Critical error! Unable to allocate %u bytes!\n",
                (unsigned int)size);
        exit(1);
        return 0;
    }
    memset(tmp, 0, size);
    return tmp;
}

static void gbm_free(void *ptr)
{
    free(ptr);
}

static struct vigs_drm_surface
    *yagl_gbm_surface_acquire_back(struct gbm_surface *sfc)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)sfc;
    int i;

    if (!yagl_sfc->back) {
        for (i = 0;
             i < sizeof(yagl_sfc->color_buffers)/sizeof(yagl_sfc->color_buffers[0]);
             ++i) {
            if (!yagl_sfc->color_buffers[i].locked) {
                yagl_sfc->back = &yagl_sfc->color_buffers[i];
                break;
            }
        }
    }

    if (!yagl_sfc->back) {
        return NULL;
    }

    if (!yagl_sfc->back->bo) {
        yagl_sfc->back->bo = gbm_bo_create(sfc->gbm, sfc->width,
                                           sfc->height, sfc->format,
                                           sfc->flags);
    }

    if (!yagl_sfc->back->bo) {
        return NULL;
    }

    vigs_drm_gem_ref(&yagl_sfc->back->bo->drm_sfc->gem);

    return yagl_sfc->back->bo->drm_sfc;
}

static void yagl_gbm_surface_swap_buffers(struct gbm_surface *sfc)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)sfc;
    yagl_sfc->front = yagl_sfc->back;
    yagl_sfc->back = NULL;
}

YAGL_API int gbm_device_get_fd(struct gbm_device *gbm)
{
    return gbm->drm_dev->fd;
}

YAGL_API const char *gbm_device_get_backend_name(struct gbm_device *gbm)
{
    return "gbm_yagl.so";
}

YAGL_API int gbm_device_is_format_supported(struct gbm_device *gbm,
                                            uint32_t format,
                                            uint32_t usage)
{
    switch (format) {
    case GBM_BO_FORMAT_XRGB8888:
    case GBM_FORMAT_XRGB8888:
    case GBM_BO_FORMAT_ARGB8888:
    case GBM_FORMAT_ARGB8888:
        return 1;
    default:
        return 0;
    }
}

YAGL_API void gbm_device_destroy(struct gbm_device *gbm)
{
    vigs_drm_device_destroy(gbm->drm_dev);
    gbm_free(gbm);
}

YAGL_API struct gbm_device *gbm_create_device(int fd)
{
    struct vigs_drm_device *drm_dev;
    struct gbm_device *gbm;
    int ret;

    ret = vigs_drm_device_create(fd, &drm_dev);

    if (ret != 0) {
        fprintf(stderr,
                "GBM: Critical error! vigs_drm_device_create failed: %s\n",
                strerror(-ret));
        return NULL;
    }

    gbm = gbm_malloc0(sizeof(*gbm));

    gbm->dummy = &gbm_create_device;
    gbm->drm_dev = drm_dev;

    return gbm;
}

YAGL_API struct gbm_bo *gbm_bo_create(struct gbm_device *gbm,
                                      uint32_t width, uint32_t height,
                                      uint32_t format, uint32_t flags)
{
    vigs_drm_surface_format sfc_format;
    uint32_t depth;
    struct yagl_gbm_bo *bo;
    struct vigs_drm_surface *sfc;
    int ret;

    if ((flags & GBM_BO_USE_CURSOR_64X64) != 0) {
        return NULL;
    }

    switch (format) {
    case GBM_BO_FORMAT_XRGB8888:
    case GBM_FORMAT_XRGB8888:
        sfc_format = vigs_drm_surface_bgrx8888;
        depth = 24;
        break;
    case GBM_BO_FORMAT_ARGB8888:
    case GBM_FORMAT_ARGB8888:
        sfc_format = vigs_drm_surface_bgra8888;
        depth = 32;
        break;
    default:
        fprintf(stderr, "GBM: Bad format = %u\n", format);
        return NULL;
    }

    ret = vigs_drm_surface_create(gbm->drm_dev,
                                  width,
                                  height,
                                  (width * 4),
                                  sfc_format,
                                  &sfc);

    if (ret != 0) {
        fprintf(stderr,
                "GBM: Unable to create DRM surface(%ux%u, fmt = %u): %s\n",
                width, height, sfc_format,
                strerror(-ret));
        return NULL;
    }

    bo = gbm_malloc0(sizeof(*bo));

    bo->base.gbm = gbm;
    bo->base.drm_sfc = sfc;
    bo->base.depth = depth;

    bo->format = format;
    bo->handle.u32 = sfc->gem.handle;

    return &bo->base;
}

YAGL_API struct gbm_bo *gbm_bo_import(struct gbm_device *gbm, uint32_t type,
                                      void *buffer, uint32_t usage)
{
#ifdef YAGL_PLATFORM_WAYLAND
    struct wl_drm_buffer *drm_buffer;
#endif
    struct vigs_drm_surface *drm_sfc;
    struct yagl_gbm_bo *bo;

    switch (type) {
#ifdef YAGL_PLATFORM_WAYLAND
    case GBM_BO_IMPORT_WL_BUFFER:
        drm_buffer = wayland_drm_get_buffer((struct wl_resource*)buffer);

        if (!drm_buffer) {
            return NULL;
        }

        drm_sfc = wayland_drm_buffer_get_sfc(drm_buffer);

        vigs_drm_gem_ref(&drm_sfc->gem);

        break;
#endif
    default:
        fprintf(stderr, "GBM: bo_import bad type = %u\n", type);
        return NULL;
    }

    bo = gbm_malloc0(sizeof(*bo));

    bo->base.gbm = gbm;
    bo->base.drm_sfc = drm_sfc;

    bo->handle.u32 = drm_sfc->gem.handle;

    switch (drm_sfc->format) {
    case vigs_drm_surface_bgrx8888:
        bo->format = GBM_FORMAT_XRGB8888;
        bo->base.depth = 24;
        break;
    case vigs_drm_surface_bgra8888:
        bo->format = GBM_FORMAT_ARGB8888;
        bo->base.depth = 32;
        break;
    default:
        fprintf(stderr, "GBM: bo_import, bad format = %u\n", drm_sfc->format);
        return NULL;
    }

    return &bo->base;
}

YAGL_API uint32_t gbm_bo_get_width(struct gbm_bo *bo)
{
    return bo->drm_sfc->width;
}

YAGL_API uint32_t gbm_bo_get_height(struct gbm_bo *bo)
{
    return bo->drm_sfc->height;
}

YAGL_API uint32_t gbm_bo_get_stride(struct gbm_bo *bo)
{
    return bo->drm_sfc->stride;
}

YAGL_API uint32_t gbm_bo_get_format(struct gbm_bo *bo)
{
    struct yagl_gbm_bo *yagl_bo = (struct yagl_gbm_bo*)bo;
    return yagl_bo->format;
}

YAGL_API struct gbm_device *gbm_bo_get_device(struct gbm_bo *bo)
{
    return bo->gbm;
}

YAGL_API union gbm_bo_handle gbm_bo_get_handle(struct gbm_bo *bo)
{
    struct yagl_gbm_bo *yagl_bo = (struct yagl_gbm_bo*)bo;
    return yagl_bo->handle;
}

YAGL_API int gbm_bo_write(struct gbm_bo *bo, const void *buf, size_t count)
{
    int ret = vigs_drm_gem_map(&bo->drm_sfc->gem, 1);

    if (ret != 0) {
        fprintf(stderr,
                "GBM: Unable to map DRM surface(id = %u): %s\n",
                bo->drm_sfc->id,
                strerror(-ret));
        return -1;
    }

    ret = vigs_drm_surface_start_access(bo->drm_sfc, VIGS_DRM_SAF_WRITE);

    if (ret != 0) {
        fprintf(stderr,
                "GBM: Unable to start DRM surface access(id = %u): %s\n",
                bo->drm_sfc->id,
                strerror(-ret));
        return -1;
    }

    memcpy(bo->drm_sfc->gem.vaddr, buf, count);

    ret = vigs_drm_surface_end_access(bo->drm_sfc, 1);

    if (ret != 0) {
        fprintf(stderr,
                "GBM: Unable to end DRM surface access(id = %u): %s\n",
                bo->drm_sfc->id,
                strerror(-ret));
        return -1;
    }

    return 0;
}

YAGL_API void gbm_bo_set_user_data(struct gbm_bo *bo,
                                   void *data,
                                   void (*destroy_user_data)(struct gbm_bo*, void*))
{
    struct yagl_gbm_bo *yagl_bo = (struct yagl_gbm_bo*)bo;

    yagl_bo->user_data = data;
    yagl_bo->destroy_user_data = destroy_user_data;
}

YAGL_API void *gbm_bo_get_user_data(struct gbm_bo *bo)
{
    struct yagl_gbm_bo *yagl_bo = (struct yagl_gbm_bo*)bo;
    return yagl_bo->user_data;
}

YAGL_API void gbm_bo_destroy(struct gbm_bo *bo)
{
    struct yagl_gbm_bo *yagl_bo = (struct yagl_gbm_bo*)bo;

    if (yagl_bo->destroy_user_data) {
        yagl_bo->destroy_user_data(bo, yagl_bo->user_data);
    }

    vigs_drm_gem_unref(&bo->drm_sfc->gem);

    gbm_free(yagl_bo);
}

YAGL_API struct gbm_surface *gbm_surface_create(struct gbm_device *gbm,
                                                uint32_t width, uint32_t height,
                                                uint32_t format, uint32_t flags)
{
    uint32_t depth;
    struct yagl_gbm_surface *sfc;

    switch (format) {
    case GBM_BO_FORMAT_XRGB8888:
    case GBM_FORMAT_XRGB8888:
        depth = 24;
        break;
    case GBM_BO_FORMAT_ARGB8888:
    case GBM_FORMAT_ARGB8888:
        depth = 32;
        break;
    default:
        fprintf(stderr, "GBM: Bad format = %u\n", format);
        return NULL;
    }

    sfc = gbm_malloc0(sizeof(*sfc));

    sfc->base.gbm = gbm;
    sfc->base.width = width;
    sfc->base.depth = depth;
    sfc->base.height = height;
    sfc->base.format = format;
    sfc->base.flags = flags;

    sfc->base.acquire_back = &yagl_gbm_surface_acquire_back;
    sfc->base.swap_buffers = &yagl_gbm_surface_swap_buffers;

    return &sfc->base;
}

YAGL_API struct gbm_bo *gbm_surface_lock_front_buffer(struct gbm_surface *surface)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)surface;
    struct gbm_bo *bo;

    if (!yagl_sfc->front) {
        return NULL;
    }

    bo = yagl_sfc->front->bo;

    yagl_sfc->front->locked = 1;
    yagl_sfc->front = NULL;

    return bo;
}

YAGL_API void gbm_surface_release_buffer(struct gbm_surface *surface,
                                         struct gbm_bo *bo)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)surface;
    int i;

    for (i = 0;
         i < sizeof(yagl_sfc->color_buffers)/sizeof(yagl_sfc->color_buffers[0]);
         ++i) {
        if (yagl_sfc->color_buffers[i].bo == bo) {
            yagl_sfc->color_buffers[i].locked = 0;
        }
    }
}

YAGL_API int gbm_surface_has_free_buffers(struct gbm_surface *surface)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)surface;
    int i;

    for (i = 0;
         i < sizeof(yagl_sfc->color_buffers)/sizeof(yagl_sfc->color_buffers[0]);
         ++i) {
        if (!yagl_sfc->color_buffers[i].locked) {
            return 1;
        }
    }

    return 0;
}

YAGL_API void gbm_surface_destroy(struct gbm_surface *surface)
{
    struct yagl_gbm_surface *yagl_sfc = (struct yagl_gbm_surface*)surface;
    int i;

    for (i = 0;
         i < sizeof(yagl_sfc->color_buffers)/sizeof(yagl_sfc->color_buffers[0]);
         ++i) {
        if (yagl_sfc->color_buffers[i].bo) {
            gbm_bo_destroy(yagl_sfc->color_buffers[i].bo);
        }
    }

    gbm_free(yagl_sfc);
}
