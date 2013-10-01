#ifndef _YAGL_CLIENT_CONTEXT_H_
#define _YAGL_CLIENT_CONTEXT_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_sharegroup;
struct yagl_client_image;
struct yagl_tex_image_binding;

struct yagl_client_context
{
    yagl_client_api client_api;

    struct yagl_sharegroup *sg;

    void (*prepare)(struct yagl_client_context */*ctx*/);

    int (*bind_tex_image)(struct yagl_client_context */*ctx*/,
                          struct yagl_client_image */*image*/,
                          struct yagl_tex_image_binding */*binding*/);

    void (*destroy)(struct yagl_client_context */*ctx*/);
};

YAGL_API struct yagl_client_context *yagl_get_client_context(void);

#endif
