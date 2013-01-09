#ifndef _YAGL_MEM_H_
#define _YAGL_MEM_H_

#include "yagl_export.h"
#include "yagl_types.h"

/*
 * We're forced to fault in memory before calling some of the host YaGL
 * functions. This is needed in order to guarantee that
 * virtual <-> physical mapping will exist when host reads/writes
 * target's virtual memory directly.
 */

static __inline void yagl_mem_probe_read_uint8(const uint8_t *value)
{
    volatile uint8_t tmp;
    if (value) {
        tmp = *value;
    }
}

static __inline void yagl_mem_probe_write_uint8(uint8_t *value)
{
    volatile uint8_t tmp;
    if (value) {
        tmp = *value;
        *value = tmp;
    }
}

static __inline void yagl_mem_probe_read_uint32(const uint32_t *value)
{
    volatile uint32_t tmp;
    if (value) {
        tmp = *value;
    }
}

static __inline void yagl_mem_probe_write_uint32(uint32_t *value)
{
    volatile uint32_t tmp;
    if (value) {
        tmp = *value;
        *value = tmp;
    }
}

static __inline void yagl_mem_probe_read_float(const float *value)
{
    volatile float tmp;
    if (value) {
        tmp = *value;
    }
}

static __inline void yagl_mem_probe_write_float(float *value)
{
    volatile float tmp;
    if (value) {
        tmp = *value;
        *value = tmp;
    }
}

static __inline void yagl_mem_probe_read_ptr(const void **value)
{
    const void *volatile tmp;
    if (value) {
        tmp = *value;
    }
}

static __inline void yagl_mem_probe_write_ptr(void **value)
{
    void *volatile tmp;
    if (value) {
        tmp = *value;
        *value = tmp;
    }
}

static __inline void yagl_mem_probe_read(const void *data, int len)
{
    if (data) {
        int i;
        volatile uint8_t tmp;

        /*
         * TODO: Replace 4096 by a single sysconf(_SC_PAGE_SIZE)
         */

        if (len > 0) {
            tmp = ((const uint8_t*)data)[0];
        }

        for (i = 4096 - ((uintptr_t)data & (4096 - 1)); i < len; i += 4096) {
            tmp = ((const uint8_t*)data)[i];
        }
    }
}

static __inline void yagl_mem_probe_write(void *data, int len)
{
    if (data) {
        int i;
        volatile uint8_t tmp;

        /*
         * TODO: Replace 4096 by a single sysconf(_SC_PAGE_SIZE)
         */

        if (len > 0) {
            tmp = ((const uint8_t*)data)[0];
            ((uint8_t*)data)[0] = tmp;
        }

        for (i = 4096 - ((uintptr_t)data & (4096 - 1)); i < len; i += 4096) {
            tmp = ((const uint8_t*)data)[i];
            ((uint8_t*)data)[i] = tmp;
        }
    }
}

static __inline void yagl_mem_probe_read_int8(const int8_t *val_p)
{
    yagl_mem_probe_read_uint8((const uint8_t *)val_p);
}

static __inline void yagl_mem_probe_write_int8(int8_t *val_p)
{
    yagl_mem_probe_write_uint8((uint8_t *)val_p);
}

static __inline void yagl_mem_probe_read_int32(const int32_t *val_p)
{
    yagl_mem_probe_read_uint32((const uint32_t *)val_p);
}

static __inline void yagl_mem_probe_write_int32(int32_t *val_p)
{
    yagl_mem_probe_write_uint32((uint32_t *)val_p);
}

#define YAGL_HOST_CALL_ASSERT(res) \
        if (!res) { \
            fprintf(stderr, \
                    "Critical error! Call at %s:%d failed!\n", \
                    __FUNCTION__, \
                    __LINE__); \
            exit(1); \
        }

#endif
