#ifndef _YAGL_BATCH_GL_H_
#define _YAGL_BATCH_GL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_batch.h"
#include <string.h>

#define yagl_batch_put_GLints(data, count) yagl_batch_put_int32s(data, count)
#define yagl_batch_put_GLuints(data, count) yagl_batch_put_uint32s(data, count)
#define yagl_batch_put_GLfloats(data, count) yagl_batch_put_floats(data, count)

static __inline const GLchar *yagl_batch_put_GLchars(const GLchar *data)
{
    return data ? yagl_batch_put_int8s(data, (strlen(data) + 1)) : NULL;
}

#endif
