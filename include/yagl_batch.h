#ifndef _YAGL_BATCH_H_
#define _YAGL_BATCH_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_state.h"

#define yagl_batch_put_uint8s(data, count) yagl_batch_put(data, count)
#define yagl_batch_put_int8s(data, count) yagl_batch_put(data, count)
#define yagl_batch_put_uint32s(data, count) yagl_batch_put(data, (count) * sizeof(uint32_t))
#define yagl_batch_put_int32s(data, count) yagl_batch_put(data, (count) * sizeof(int32_t))
#define yagl_batch_put_floats(data, count) yagl_batch_put(data, (count) * sizeof(float))

#endif
