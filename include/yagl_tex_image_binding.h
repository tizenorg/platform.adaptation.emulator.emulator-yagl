#ifndef _YAGL_TEX_IMAGE_BINDING_H_
#define _YAGL_TEX_IMAGE_BINDING_H_

#include "yagl_types.h"

struct yagl_tex_image_binding
{
    void (*unbind)(struct yagl_tex_image_binding */*binding*/);

    void *cookie;
};

#endif
