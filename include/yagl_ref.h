#ifndef _YAGL_REF_H_
#define _YAGL_REF_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <pthread.h>

struct yagl_ref;

typedef void (*yagl_ref_destroy_func)(struct yagl_ref */*ref*/);

struct yagl_ref
{
    yagl_ref_destroy_func destroy;

    pthread_mutex_t mutex;
    volatile uint32_t count;
};

/*
 * Initializes ref count to 1.
 */
YAGL_API void yagl_ref_init(struct yagl_ref *ref, yagl_ref_destroy_func destroy);

YAGL_API void yagl_ref_cleanup(struct yagl_ref *ref);

/*
 * Increments ref count.
 */
YAGL_API void yagl_ref_acquire(struct yagl_ref *ref);

/*
 * Decrements ref count and releases when 0.
 */
YAGL_API void yagl_ref_release(struct yagl_ref *ref);

#endif
