#include "wayland-drm.h"
#include <wayland-server.h>
#include "wayland-drm-server-protocol.h"
#include "vigs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct wl_drm
{
    struct wl_display *display;
    char *device_name;
    struct wayland_drm_callbacks *callbacks;
    void *user_data;
};

struct wl_drm_buffer
{
    struct wl_resource resource;

    struct wl_drm *drm;

    struct vigs_drm_surface *drm_sfc;
};

static void buffer_destroy(struct wl_resource *resource)
{
    struct wl_drm_buffer *buffer = resource->data;

    vigs_drm_gem_unref(&buffer->drm_sfc->gem);

    free(buffer);
}

static void drm_buffer_destroy(struct wl_client *client,
                               struct wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static struct wl_buffer_interface drm_buffer_interface =
{
    drm_buffer_destroy
};

static void drm_create_buffer(struct wl_client *client,
                              struct wl_resource *resource,
                              uint32_t id, uint32_t name,
                              int32_t width, int32_t height,
                              uint32_t stride, uint32_t format)
{
    struct wl_drm *drm = resource->data;
    struct wl_drm_buffer *buffer;

    switch (format) {
    case WL_DRM_FORMAT_ARGB8888:
    case WL_DRM_FORMAT_XRGB8888:
        break;
    default:
        wl_resource_post_error(resource,
                               WL_DRM_ERROR_INVALID_FORMAT,
                               "invalid format");
        return;
    }

    buffer = malloc(sizeof(*buffer));

    if (!buffer) {
        wl_resource_post_no_memory(resource);
        return;
    }

    memset(buffer, 0, sizeof(*buffer));

    buffer->drm_sfc = drm->callbacks->acquire_buffer(drm->user_data, name);

    if (!buffer->drm_sfc) {
        wl_resource_post_error(resource,
                               WL_DRM_ERROR_INVALID_NAME,
                               "invalid name");
        free(buffer);
        return;
    }

    buffer->drm = drm;

    buffer->resource.object.id = id;
    buffer->resource.object.interface = &wl_buffer_interface;
    buffer->resource.object.implementation = (void(**)(void))&drm_buffer_interface;
    buffer->resource.data = buffer;

    buffer->resource.destroy = buffer_destroy;
    buffer->resource.client = resource->client;

    wl_client_add_resource(resource->client, &buffer->resource);
}

static void drm_create_planar_buffer(struct wl_client *client,
                                     struct wl_resource *resource,
                                     uint32_t id, uint32_t name,
                                     int32_t width, int32_t height,
                                     uint32_t format,
                                     int32_t offset0, int32_t stride0,
                                     int32_t offset1, int32_t stride1,
                                     int32_t offset2, int32_t stride2)
{
    wl_resource_post_error(resource,
                           WL_DRM_ERROR_INVALID_FORMAT,
                           "invalid format");
}

static void drm_authenticate(struct wl_client *client,
                             struct wl_resource *resource,
                             uint32_t id)
{
    struct wl_drm *drm = resource->data;

    if (drm->callbacks->authenticate(drm->user_data, id) < 0) {
        wl_resource_post_error(resource,
                               WL_DRM_ERROR_AUTHENTICATE_FAIL,
                               "authenicate failed");
    } else {
        wl_drm_send_authenticated(resource);
    }
}

static struct wl_drm_interface drm_interface =
{
    drm_authenticate,
    drm_create_buffer,
    drm_create_planar_buffer
};

static void bind_drm(struct wl_client *client,
                     void *data,
                     uint32_t version,
                     uint32_t id)
{
    struct wl_drm *drm = data;
    struct wl_resource *resource;

    resource = wl_resource_create(client,
                                  &wl_drm_interface,
                                  1,
                                  id);

    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &drm_interface, data, NULL);

    wl_drm_send_device(resource, drm->device_name);
    wl_drm_send_format(resource, WL_DRM_FORMAT_ARGB8888);
    wl_drm_send_format(resource, WL_DRM_FORMAT_XRGB8888);
}

struct wl_drm *wayland_drm_create(struct wl_display *display,
                                  char *device_name,
                                  struct wayland_drm_callbacks *callbacks,
                                  void *user_data)
{
    struct wl_drm *drm;

    drm = malloc(sizeof(*drm));

    if (!drm) {
        fprintf(stderr, "wayland-drm: Critical error! Unable to allocate wl_drm!\n");
        exit(1);
        return NULL;
    }

    memset(drm, 0, sizeof(*drm));

    drm->display = display;
    drm->device_name = strdup(device_name);
    drm->callbacks = callbacks;
    drm->user_data = user_data;

    wl_global_create(display, &wl_drm_interface, 1, drm, bind_drm);

    return drm;
}

void wayland_drm_destroy(struct wl_drm *drm)
{
    free(drm->device_name);

    free(drm);
}

struct wl_drm_buffer *wayland_drm_get_buffer(struct wl_resource *resource)
{
    if (!resource) {
        return NULL;
    }

    if (wl_resource_instance_of(resource,
                                &wl_buffer_interface,
                                &drm_buffer_interface)) {
        return wl_resource_get_user_data(resource);
    } else {
        return NULL;
    }
}

struct vigs_drm_surface *wayland_drm_buffer_get_sfc(struct wl_drm_buffer *buffer)
{
    return buffer->drm_sfc;
}
