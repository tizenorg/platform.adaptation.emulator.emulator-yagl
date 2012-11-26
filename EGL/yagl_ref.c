#include "yagl_ref.h"
#include "yagl_utils.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

void yagl_ref_init(struct yagl_ref *ref, yagl_ref_destroy_func destroy)
{
    assert(ref);
    assert(destroy);

    memset(ref, 0, sizeof(*ref));

    ref->destroy = destroy;
    yagl_mutex_init(&ref->mutex);
    ref->count = 1;
}

void yagl_ref_cleanup(struct yagl_ref *ref)
{
    assert(ref);
    assert(!ref->count);

    ref->destroy = NULL;
    pthread_mutex_destroy(&ref->mutex);
}

void yagl_ref_acquire(struct yagl_ref *ref)
{
    assert(ref);
    assert(ref->count > 0);

    pthread_mutex_lock(&ref->mutex);
    ++ref->count;
    pthread_mutex_unlock(&ref->mutex);
}

void yagl_ref_release(struct yagl_ref *ref)
{
    int call_destroy = 0;

    assert(ref);
    assert(ref->count > 0);

    pthread_mutex_lock(&ref->mutex);
    call_destroy = (--ref->count == 0);
    pthread_mutex_unlock(&ref->mutex);

    if (call_destroy)
    {
        assert(ref->destroy);
        ref->destroy(ref);
    }
}
