#include "yagl_transport.h"
#include "yagl_malloc.h"
#include <stdio.h>
#include <stdlib.h>

#define YAGL_MAX_RETRIES 100

typedef enum
{
    yagl_call_result_ok = 0xA,    /* Call is ok. */
    yagl_call_result_retry = 0xB, /* Page fault on host, retry is required. */
} yagl_call_result;

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
    uint32_t max_size = 3 * 8 + max_data_size + 8;

    if (max_size > t->max_call_size) {
        uint32_t min_size = 3 * 8 + min_data_size + 8;

        if (!yagl_transport_fit(t, min_size)) {
            yagl_transport_sync(t);
        }

        t->direct = 1;
    } else {
        t->direct = 0;

        if (!yagl_transport_fit(t, max_size)) {
            yagl_transport_sync(t);
            if (!yagl_transport_fit(t, max_size)) {
                t->direct = 1;
            }
        }
    }

    t->ptr_begin = t->ptr;
    t->num_in_args = t->num_in_arrays = 0;

    yagl_transport_put_out_uint32_t(t, api);
    yagl_transport_put_out_uint32_t(t, func_id);
    t->res = (uint32_t*)t->ptr;
    yagl_transport_put_out_uint32_t(t, 0);
}

int yagl_transport_end(struct yagl_transport *t)
{
    uint32_t i;

    *t->res = t->direct;

    if (!t->direct && (t->num_in_args == 0) && (t->num_in_arrays == 0)) {
        return 1;
    }

    if (t->retry_count == 0) {
        yagl_transport_put_out_uint32_t(t, 0);
        t->ops->commit(t->ops_data, 0);
    } else {
        t->ops->commit(t->ops_data, t->ptr_begin - t->buff);
    }

    if (*t->res == t->direct) {
        fprintf(stderr, "Critical error! Bad call!\n");
        exit(1);
        return 0;
    }

    switch (*t->res) {
    case yagl_call_result_ok:
        t->retry_count = 0;
        break;
    case yagl_call_result_retry:
        if (!t->direct) {
            fprintf(stderr,
                    "Critical error! Retry returned by host while not in direct mode!\n");
            exit(1);
        }
        if (++t->retry_count >= YAGL_MAX_RETRIES) {
            fprintf(stderr,
                    "Critical error! Max retry count %u reached!\n",
                    t->retry_count);
            exit(1);
        }
        return 0;
    default:
        fprintf(stderr, "Critical error! Bad call result - %u!\n", *t->res);
        exit(1);
        return 0;
    }

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

    t->ptr = t->buff;

    return 1;
}

void yagl_transport_sync(struct yagl_transport *t)
{
    if (t->ptr == t->buff) {
        return;
    }

    yagl_transport_put_out_uint32_t(t, 0);
    t->ops->commit(t->ops_data, 0);

    t->ptr = t->buff;
}
