#ifndef _YAGL_MALLOC_H_
#define _YAGL_MALLOC_H_

#include "yagl_export.h"
#include "yagl_types.h"

YAGL_API void *yagl_malloc(size_t size);

YAGL_API void *yagl_malloc0(size_t size);

YAGL_API void *yagl_realloc(void *ptr, size_t size);

YAGL_API void yagl_free(void* ptr);

#endif
