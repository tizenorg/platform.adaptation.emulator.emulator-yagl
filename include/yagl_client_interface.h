#ifndef _YAGL_CLIENT_INTERFACE_H_
#define _YAGL_CLIENT_INTERFACE_H_

#include "yagl_types.h"

struct yagl_client_context;
struct yagl_client_image;
struct yagl_sharegroup;

struct yagl_client_interface
{
    struct yagl_client_context *(*create_ctx)(struct yagl_client_interface */*iface*/,
                                              struct yagl_sharegroup */*sg*/);

    struct yagl_client_image
        *(*create_image)(struct yagl_client_interface */*iface*/,
                         yagl_object_name /*tex_global_name*/);

    void (*release_tex_image)(struct yagl_client_interface */*iface*/,
                              void */*cookie*/);
};

#endif
