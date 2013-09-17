#ifndef _YAGL_TRANSPORT_EGL_H_
#define _YAGL_TRANSPORT_EGL_H_

#include "yagl_transport.h"
#include "EGL/egl.h"

static __inline int32_t yagl_transport_attrib_list_count(const EGLint *value)
{
    int32_t count = 0;

    if (value) {
        EGLint tmp = value[count];
        while (tmp != EGL_NONE) {
            count += 2;
            tmp = value[count];
        }
        ++count;
    }

    return count;
}

static __inline void yagl_transport_put_out_EGLint(struct yagl_transport *t,
                                                   EGLint value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_EGLenum(struct yagl_transport *t,
                                                    EGLenum value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_in_EGLint(struct yagl_transport *t,
                                                  EGLint *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_EGLBoolean(struct yagl_transport *t,
                                                      EGLBoolean *value)
{
    yagl_transport_put_in_uint32_t(t, value);
}

#endif
