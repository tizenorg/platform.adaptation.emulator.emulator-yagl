/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#ifndef _YAGL_TRANSPORT_H_
#define _YAGL_TRANSPORT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <string.h>
#include <assert.h>
#include <limits.h>

#define YAGL_TRANSPORT_MAX_IN_ARGS 8
#define YAGL_TRANSPORT_MAX_IN_ARRAYS 8

#define YAGL_TRANSPORT_MAX_IN_DA 8
#define YAGL_TRANSPORT_MAX_OUT_DA 8

struct yagl_egl_fence;

struct yagl_transport_ops
{
    void *(*resize)(void */*ops_data*/, uint32_t /*size*/);

    void (*commit)(void */*ops_data*/, int /*sync*/);

    uint32_t (*flush)(void */*ops_data*/, struct yagl_egl_fence **/*fence*/);
};

struct yagl_transport_in_arg
{
    uint8_t *arg_ptr;
    uint8_t *buff_ptr;
    uint32_t size;
};

struct yagl_transport_in_array
{
    uint8_t *arg_ptr;
    uint8_t *buff_ptr;
    uint32_t el_size;
    int32_t *count;
    int32_t *ret_count;
};

struct yagl_transport_da
{
    void *data;
    int32_t size;
};

struct yagl_transport
{
    /*
     * per-transport.
     * @{
     */

    struct yagl_transport_ops *ops;
    void *ops_data;

    uint32_t max_buff_size;
    uint32_t max_call_size;

    uint8_t *buff;
    uint32_t buff_size;

    uint8_t *ptr;

    /*
     * @}
     */

    /*
     * per-call.
     * @{
     */

    int direct;

    uint32_t num_in_args;
    uint32_t num_in_arrays;

    uint32_t num_in_da;
    uint32_t num_out_da;

    struct yagl_transport_in_arg in_args[YAGL_TRANSPORT_MAX_IN_ARGS];
    struct yagl_transport_in_array in_arrays[YAGL_TRANSPORT_MAX_IN_ARRAYS];

    struct yagl_transport_da in_da[YAGL_TRANSPORT_MAX_IN_DA];
    struct yagl_transport_da out_da[YAGL_TRANSPORT_MAX_OUT_DA];

    /*
     * @}
     */
};

YAGL_API struct yagl_transport
    *yagl_transport_create(struct yagl_transport_ops *ops,
                           void *ops_data,
                           uint32_t max_buff_size,
                           uint32_t max_call_size);

YAGL_API void yagl_transport_destroy(struct yagl_transport *t);

YAGL_API void yagl_transport_begin(struct yagl_transport *t,
                                   yagl_api_id api,
                                   uint32_t func_id,
                                   uint32_t min_data_size,
                                   uint32_t max_data_size);

YAGL_API void yagl_transport_end(struct yagl_transport *t);

YAGL_API void yagl_transport_flush(struct yagl_transport *t,
                                   struct yagl_egl_fence *fence);

/*
 * Waits until all commands for this transport
 * are done on host. The wait is ON HOST, not very nice,
 * but it supposed to be used in offscreen backend only,
 * so we don't care.
 */
YAGL_API void yagl_transport_wait(struct yagl_transport *t);

/*
 * All marshalling/unmarshalling must be done with 8-byte alignment,
 * since this is the maximum alignment possible. This way we can
 * just do assignments without "memcpy" calls and can be sure that
 * the code won't fail on architectures that don't support unaligned
 * memory access.
 */

static __inline void yagl_transport_put_out_uint8_t(struct yagl_transport *t,
                                                    uint8_t value)
{
    *t->ptr = value;
    t->ptr += 8;
}

static __inline void yagl_transport_put_out_uint32_t(struct yagl_transport *t,
                                                     uint32_t value)
{
    *(uint32_t*)t->ptr = value;
    t->ptr += 8;
}

static __inline void yagl_transport_put_out_float(struct yagl_transport *t,
                                                  float value)
{
    *(float*)t->ptr = value;
    t->ptr += 8;
}

static __inline void yagl_transport_put_out_uint64_t(struct yagl_transport *t,
                                                     uint64_t ptr)
{
    *(uint64_t*)t->ptr = ptr;
    t->ptr += 8;
}

static __inline void yagl_transport_put_out_uintptr_t(struct yagl_transport *t,
                                                      uintptr_t ptr)
{
    uint64_t value = 9;
    if (sizeof(uintptr_t) == 4) {
        value = ptr & UINT_MAX;
    } else {
        value = ptr;
    }
    yagl_transport_put_out_uint64_t(t, value);
}

static __inline void yagl_transport_put_in_arg_helper(struct yagl_transport *t,
                                                      void *value,
                                                      uint32_t size)
{
    yagl_transport_put_out_uintptr_t(t, (uintptr_t)value);

    t->in_args[t->num_in_args].arg_ptr = (uint8_t*)value;
    t->in_args[t->num_in_args].buff_ptr = t->ptr;
    t->in_args[t->num_in_args].size = size;

    ++t->num_in_args;
}

static __inline void yagl_transport_put_in_uint8_t(struct yagl_transport *t,
                                                   uint8_t *value)
{
    yagl_transport_put_in_arg_helper(t, value, sizeof(*value));
    if (value) {
        yagl_transport_put_out_uint8_t(t, *value);
    }
}

static __inline void yagl_transport_put_in_uint32_t(struct yagl_transport *t,
                                                    uint32_t *value)
{
    yagl_transport_put_in_arg_helper(t, value, sizeof(*value));
    if (value) {
        yagl_transport_put_out_uint32_t(t, *value);
    }
}

static __inline void yagl_transport_put_in_float(struct yagl_transport *t,
                                                 float *value)
{
    yagl_transport_put_in_arg_helper(t, value, sizeof(*value));
    if (value) {
        yagl_transport_put_out_float(t, *value);
    }
}

static __inline uint32_t yagl_transport_array_size(const void *data,
                                                   int32_t count,
                                                   int32_t el_size)
{
    if (data && (count > 0)) {
        return 2 * 8 + ((count * el_size + 7U) & ~7U);
    } else {
        return 2 * 8;
    }
}

static __inline void yagl_transport_put_out_array(struct yagl_transport *t,
                                                  const void *data,
                                                  int32_t count,
                                                  int32_t el_size)
{
    yagl_transport_put_out_uintptr_t(t, (uintptr_t)data);
    yagl_transport_put_out_uint32_t(t, count);

    if (!data || (count <= 0)) {
        return;
    }

    if (t->direct) {
        t->out_da[t->num_out_da].data = (void*)data;
        t->out_da[t->num_out_da].size = count * el_size;

        ++t->num_out_da;
    } else {
        memcpy(t->ptr, data, count * el_size);
        t->ptr += (count * el_size + 7U) & ~7U;
    }
}

static __inline void yagl_transport_put_in_array(struct yagl_transport *t,
                                                 void *data,
                                                 int32_t maxcount,
                                                 int32_t *ret_count,
                                                 int32_t el_size)
{
    int32_t *count;

    yagl_transport_put_out_uintptr_t(t, (uintptr_t)data);
    count = (int32_t*)t->ptr;
    yagl_transport_put_out_uint32_t(t, maxcount);

    t->in_arrays[t->num_in_arrays].arg_ptr = (uint8_t*)data;
    t->in_arrays[t->num_in_arrays].buff_ptr = t->ptr;
    t->in_arrays[t->num_in_arrays].el_size = el_size;
    t->in_arrays[t->num_in_arrays].count = count;
    t->in_arrays[t->num_in_arrays].ret_count = ret_count;

    ++t->num_in_arrays;

    if (!data || (maxcount <= 0)) {
        return;
    }

    if (t->direct) {
        t->in_da[t->num_in_da].data = data;
        t->in_da[t->num_in_da].size = maxcount * el_size;

        ++t->num_in_da;
    } else {
        t->ptr += (maxcount * el_size + 7U) & ~7U;
    }
}

/*
 * We're forced to fault in memory before calling some of the host YaGL
 * functions. This is needed in order to guarantee that
 * virtual <-> physical mapping will exist when host reads/writes
 * target's virtual memory directly.
 */

static __inline void yagl_transport_probe_read(const void *data, int32_t size)
{
    if (data) {
        int32_t i;
        volatile uint8_t tmp;

        if (size > 0) {
            tmp = ((const uint8_t*)data)[0];
        }

        for (i = 4096 - ((uintptr_t)data & (4096 - 1)); i < size; i += 4096) {
            tmp = ((const uint8_t*)data)[i];
        }

        (void)tmp;
    }
}

static __inline void yagl_transport_probe_write(void *data, int32_t size)
{
    if (data) {
        int32_t i;
        volatile uint8_t tmp;

        if (size > 0) {
            tmp = ((const uint8_t*)data)[0];
            ((uint8_t*)data)[0] = tmp;
        }

        for (i = 4096 - ((uintptr_t)data & (4096 - 1)); i < size; i += 4096) {
            tmp = ((const uint8_t*)data)[i];
            ((uint8_t*)data)[i] = tmp;
        }
    }
}

static __inline void yagl_transport_put_out_yagl_host_handle(struct yagl_transport *t,
                                                             yagl_host_handle value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_yagl_winsys_id(struct yagl_transport *t,
                                                           yagl_winsys_id value)
{
    yagl_transport_put_out_uint32_t(t, value);
}

static __inline void yagl_transport_put_out_va(struct yagl_transport *t,
                                               const void *value)
{
    yagl_transport_put_out_uintptr_t(t, (uintptr_t)value);
}

static __inline void yagl_transport_put_in_yagl_host_handle(struct yagl_transport *t,
                                                            yagl_host_handle *value)
{
    yagl_transport_put_in_uint32_t(t, value);
}

static __inline void yagl_transport_put_in_int(struct yagl_transport *t,
                                               int *value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline void yagl_transport_put_in_va(struct yagl_transport *t,
                                              void **value)
{
    yagl_transport_put_in_uint32_t(t, (uint32_t*)value);
}

static __inline int32_t yagl_transport_string_count(const char *str)
{
    return str ? (strlen(str) + 1) : 0;
}

#endif
