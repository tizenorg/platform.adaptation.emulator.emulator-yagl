#include "yagl_transport.h"
#include "yagl_malloc.h"
#include "yagl_log.h"
#include <stdio.h>
#include <stdlib.h>

#define YAGL_MAX_RETRIES 100

typedef enum
{
    yagl_call_result_ok = 0xA,    /* Call is ok. */
    yagl_call_result_retry = 0xB, /* Page fault on host, retry is required. */
} yagl_call_result;

static __inline void yagl_transport_ptr_reset(struct yagl_transport *t)
{
    /*
     * 4 element header.
     */
    t->ptr = t->buff + (4 * 8);

    t->direct = 0;

    t->num_in_args = t->num_in_arrays = 0;
    t->num_in_da = t->num_out_da = 0;
}

static __inline uint32_t yagl_transport_batch_size(struct yagl_transport *t)
{
    return t->ptr - (t->buff + (4 * 8));
}

static __inline void yagl_transport_update_header(struct yagl_transport *t,
                                                  uint32_t fence_seq,
                                                  uint32_t batch_size)
{
    *(uint32_t*)(t->buff + 8 * 0) = 0; /* res */
    *(uint32_t*)(t->buff + 8 * 1) = fence_seq;
    *(uint32_t*)(t->buff + 8 * 2) = batch_size;
    *(uint32_t*)(t->buff + 8 * 3) = t->num_out_da;
};

static __inline int yagl_transport_check_call_result(struct yagl_transport *t,
                                                     uint32_t* retry_count)
{
    uint32_t res = *(uint32_t*)t->buff;
    *(uint32_t*)t->buff = 0;

    switch (res) {
    case yagl_call_result_ok:
        *retry_count = 0;
        return 1;
    case yagl_call_result_retry:
        if (!t->direct) {
            fprintf(stderr,
                    "Critical error! Retry returned by host while not in direct mode!\n");
            exit(1);
        }
        if (++*retry_count >= YAGL_MAX_RETRIES) {
            fprintf(stderr,
                    "Critical error! Max retry count %u reached!\n",
                    *retry_count);
            exit(1);
        }
        return 0;
    default:
        fprintf(stderr, "Critical error! Bad call result - %u!\n", res);
        exit(1);
        return 0;
    }
}

static int yagl_transport_resize(struct yagl_transport *t, uint32_t size)
{
    void *buff;
    uint32_t offset;

    size = (size + 4095U) & ~4095U;

    buff = t->ops->resize(t->ops_data, size);

    if (!buff) {
        return 0;
    }

    offset = t->ptr - t->buff;

    t->buff = buff;
    t->buff_size = size;

    t->ptr = t->buff + offset;

    return 1;
}

static int yagl_transport_fit(struct yagl_transport *t, uint32_t size)
{
    uint32_t new_size;

    if ((t->ptr + size) <= (t->buff + t->buff_size)) {
        return 1;
    }

    new_size = t->ptr - t->buff + size;

    if (new_size <= t->max_buff_size) {
        return yagl_transport_resize(t, new_size);
    }

    return 0;
}

struct yagl_transport *yagl_transport_create(struct yagl_transport_ops *ops,
                                             void *ops_data,
                                             uint32_t max_buff_size,
                                             uint32_t max_call_size)
{
    struct yagl_transport *t;

    t = yagl_malloc0(sizeof(*t));

    t->ops = ops;
    t->ops_data = ops_data;

    t->max_buff_size = max_buff_size;
    t->max_call_size = max_call_size;

    t->max_buff_size = (t->max_buff_size + 4095U) & ~4095U;

    if (t->max_buff_size < 4096) {
        t->max_buff_size = 4096;
    }

    if (t->max_call_size > t->max_buff_size) {
        t->max_call_size = t->max_buff_size;
    }

    yagl_transport_ptr_reset(t);

    if (!yagl_transport_resize(t, 4096)) {
        yagl_free(t);
        return NULL;
    }

    return t;
}

void yagl_transport_destroy(struct yagl_transport *t)
{
    yagl_free(t);
}

void yagl_transport_begin(struct yagl_transport *t,
                          yagl_api_id api,
                          uint32_t func_id,
                          uint32_t min_data_size,
                          uint32_t max_data_size)
{
    /*
     * 3 element func header + data + reserve for out DAs.
     */

    uint32_t max_size = (3 * 8) + max_data_size + (8 * 2 * YAGL_TRANSPORT_MAX_OUT_DA);

    if (max_size > t->max_call_size) {
        uint32_t min_size = 3 * 8 + min_data_size + (8 * 2 * YAGL_TRANSPORT_MAX_OUT_DA);

        if (!yagl_transport_fit(t, min_size)) {
            yagl_transport_flush(t, NULL);
        }

        t->direct = 1;
    } else {
        t->direct = 0;

        if (!yagl_transport_fit(t, max_size)) {
            yagl_transport_flush(t, NULL);
            if (!yagl_transport_fit(t, max_size)) {
                t->direct = 1;
            }
        }
    }

    yagl_transport_put_out_uint32_t(t, api);
    yagl_transport_put_out_uint32_t(t, func_id);
    yagl_transport_put_out_uint32_t(t, t->direct);
}

void yagl_transport_end(struct yagl_transport *t)
{
    uint32_t i, retry_count = 0;
    void *fence = NULL;
    int have_ret = (t->num_in_args != 0) || (t->num_in_arrays != 0);

    if (!have_ret) {
        if (!t->direct) {
            /*
             * No return values and not direct, entirely async.
             * very fast path.
             */
            return;
        }

        /*
         * No return values, but is direct, needs commit.
         * fast path.
         */

        yagl_transport_update_header(t, t->ops->flush(t->ops_data, NULL),
                                     yagl_transport_batch_size(t));

        for (i = 0; i < t->num_out_da; ++i) {
            yagl_transport_put_out_va(t, t->out_da[i].data);
            yagl_transport_put_out_uint32_t(t, t->out_da[i].size);
        }

        do {
            for (i = 0; i < t->num_out_da; ++i) {
                yagl_transport_probe_read(t->out_da[i].data, t->out_da[i].size);
            }

            t->ops->commit(t->ops_data, 0);
        } while (!yagl_transport_check_call_result(t, &retry_count));

        yagl_transport_ptr_reset(t);

        return;
    }

    /*
     * Have return values, sync.
     * slow path.
     */

    yagl_transport_update_header(t, t->ops->flush(t->ops_data, &fence),
                                 yagl_transport_batch_size(t));

    for (i = 0; i < t->num_out_da; ++i) {
        yagl_transport_put_out_va(t, t->out_da[i].data);
        yagl_transport_put_out_uint32_t(t, t->out_da[i].size);
    }

    do {
        for (i = 0; i < t->num_out_da; ++i) {
            yagl_transport_probe_read(t->out_da[i].data, t->out_da[i].size);
        }

        t->ops->commit(t->ops_data, 0);
    } while (!yagl_transport_check_call_result(t, &retry_count));

    t->ops->fence_wait(t->ops_data, fence);

    do {
        for (i = 0; i < t->num_in_da; ++i) {
            yagl_transport_probe_write(t->in_da[i].data, t->in_da[i].size);
        }

        t->ops->commit(t->ops_data, 1);
    } while (!yagl_transport_check_call_result(t, &retry_count));

    for (i = 0; i < t->num_in_args; ++i) {
        if (t->in_args[i].arg_ptr) {
            memcpy(t->in_args[i].arg_ptr, t->in_args[i].buff_ptr, t->in_args[i].size);
        }
    }

    for (i = 0; i < t->num_in_arrays; ++i) {
        if (!t->direct &&
            t->in_arrays[i].arg_ptr &&
            ((*t->in_arrays[i].count) > 0)) {
            memcpy(t->in_arrays[i].arg_ptr,
                   t->in_arrays[i].buff_ptr,
                   t->in_arrays[i].el_size * (*t->in_arrays[i].count));
        }
        if (t->in_arrays[i].ret_count) {
            *t->in_arrays[i].ret_count = *t->in_arrays[i].count;
        }
    }

    yagl_transport_ptr_reset(t);
}

void yagl_transport_flush(struct yagl_transport *t,
                          void *fence)
{
    uint32_t batch_size = yagl_transport_batch_size(t);

    if ((batch_size == 0) && !fence) {
        return;
    }

    if (fence) {
        yagl_transport_update_header(t,
            t->ops->flush(t->ops_data, &fence), batch_size);
    } else {
        yagl_transport_update_header(t,
            t->ops->flush(t->ops_data, NULL), batch_size);
    }

    t->ops->commit(t->ops_data, 0);

    yagl_transport_ptr_reset(t);
}

void yagl_transport_wait(struct yagl_transport *t)
{
    t->ops->commit(t->ops_data, 1);
}
