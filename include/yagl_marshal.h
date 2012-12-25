#ifndef _YAGL_MARSHAL_H_
#define _YAGL_MARSHAL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_byte_order.h"

#define YAGL_MARSHAL_MAX_ARGS 10

/*
 * Each marshalled value is aligned
 * at 8-byte boundary, 2 values (api_id and func_id)
 * are passed always and function may take up to YAGL_MARSHAL_MAX_ARGS
 * arguments.
 */
#define YAGL_MARSHAL_MAX_REQUEST (8 * (2 + YAGL_MARSHAL_MAX_ARGS))

/*
 * Marshal area is limited to this number of bytes.
 */
#define YAGL_MARSHAL_SIZE 0x8000

typedef enum
{
    yagl_call_result_fail = 0,  /* Call failed, fatal error. */
    yagl_call_result_retry = 1, /* Page fault on host, retry is required. */
    yagl_call_result_ok = 2     /* Call is ok. */
} yagl_call_result;

/*
 * All marshalling/unmarshalling must be done with 8-byte alignment,
 * since this is the maximum alignment possible. This way we can
 * just do assignments without "memcpy" calls and can be sure that
 * the code won't fail on architectures that don't support unaligned
 * memory access.
 */

static __inline int yagl_marshal_skip(uint8_t** buff)
{
    *buff += 8;
    return 0;
}

static __inline void yagl_marshal_put_uint8(uint8_t** buff, uint8_t value)
{
    **buff = value;
    *buff += 8;
}

static __inline uint8_t yagl_marshal_get_uint8(uint8_t** buff)
{
    uint8_t tmp = **buff;
    *buff += 8;
    return tmp;
}

static __inline void yagl_marshal_put_uint32(uint8_t** buff, uint32_t value)
{
    *(uint32_t*)(*buff) = yagl_cpu_to_uint32le(value);
    *buff += 8;
}

static __inline uint32_t yagl_marshal_get_uint32(uint8_t** buff)
{
    uint32_t tmp = yagl_uint32le_to_cpu(*(uint32_t*)*buff);
    *buff += 8;
    return tmp;
}

static __inline void yagl_marshal_put_float(uint8_t** buff, float value)
{
    *(float*)(*buff) = value;
    *buff += 8;
}

static __inline float yagl_marshal_get_float(uint8_t** buff)
{
    float tmp = *(float*)*buff;
    *buff += 8;
    return tmp;
}

static __inline void yagl_marshal_put_ptr(uint8_t** buff, const void* value)
{
#ifdef YAGL_64
#error 64-bit ptr not supported
#else
    *(uint32_t*)(*buff) = yagl_cpu_to_uint32le((uint32_t)value);
    *buff += 8;
#endif
}

static __inline void yagl_marshal_put_host_handle(uint8_t** buff, yagl_host_handle value)
{
    *(uint32_t*)(*buff) = yagl_cpu_to_uint32le(value);
    *buff += 8;
}

static __inline yagl_host_handle yagl_marshal_get_host_handle(uint8_t** buff)
{
    yagl_host_handle tmp = yagl_uint32le_to_cpu(*(uint32_t*)*buff);
    *buff += 8;
    return tmp;
}

static __inline yagl_call_result yagl_marshal_get_call_result(uint8_t** buff)
{
    yagl_call_result tmp = yagl_uint32le_to_cpu(*(uint32_t*)*buff);
    *buff += 8;
    return tmp;
}

#define yagl_marshal_put_int8(buff, value) yagl_marshal_put_uint8(buff, (uint8_t)(value))
#define yagl_marshal_get_int8(buff) ((int8_t)yagl_marshal_get_uint8(buff))
#define yagl_marshal_put_int32(buff, value) yagl_marshal_put_uint32(buff, (uint32_t)(value))
#define yagl_marshal_get_int32(buff) ((int32_t)yagl_marshal_get_uint32(buff))
#define yagl_marshal_put_uint32_t(buff, value) yagl_marshal_put_uint32(buff, value)
#define yagl_marshal_get_uint32_t(buff) yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_int(buff, value) yagl_marshal_put_int32(buff, (value))
#define yagl_marshal_get_int(buff) yagl_marshal_get_int32(buff)
#define yagl_marshal_get_render_type(buff) (yagl_render_type)yagl_marshal_get_uint32(buff)
#define yagl_marshal_put_yagl_winsys_id(buff, value) yagl_marshal_put_uint32(buff, value)

#endif
