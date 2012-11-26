#ifndef _YAGL_STATE_H_
#define _YAGL_STATE_H_

#include "yagl_export.h"
#include "yagl_types.h"

/*
 * Get a pointer to marshal memory. Marshal memory is used
 * to marshal functions, its arguments and return value.
 */
YAGL_API uint8_t *yagl_batch_get_marshal();

/*
 * Once you've written your marshaling data call this function,
 * it'll update marshal pointer and call into host
 * whenever marshal buffer is full.
 */
YAGL_API int yagl_batch_update_marshal(uint8_t *marshal);

/*
 * Writes 'data' to batch data memory, the returned pointer
 * should be passed right into host call like this:
 * yagl_host_glBufferData(target, size, yagl_batch_put(data, size), usage);
 * If 'data' is too big then it's not written to batch memory and
 * a pointer to 'data' itself gets passed to host, marshal buffer gets
 * flushed.
 */
YAGL_API const void *yagl_batch_put(const void *data, int len);

/*
 * Causes a forced marshal buffer flush. This is required whenever you call
 * a function that returns something via pointers, since it's not safe to
 * assume that such pointers will be valid after function return.
 */
YAGL_API int yagl_batch_sync();

#endif
