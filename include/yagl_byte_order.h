#ifndef _YAGL_BYTE_ORDER_H_
#define _YAGL_BYTE_ORDER_H_

#include "yagl_export.h"
#include "yagl_types.h"

static __inline uint16_t yagl_swap_uint16(uint16_t value)
{
    return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
}

static __inline uint32_t yagl_swap_uint32(uint32_t value)
{
    return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) |
           ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}

static __inline uint64_t yagl_swap_uint64(uint64_t value)
{
    uint32_t hi = (uint32_t)(value >> 32);
    uint32_t lo = (uint32_t)(value & 0xFFFFFFFF);
    return (uint64_t)yagl_swap_uint32(hi) | ((uint64_t)yagl_swap_uint32(lo) << 32);
}

#define YAGL_DEFINE_CPUTO_NOOP(type, endian) \
    static __inline type##_t yagl_cpu_to_##type##endian(type##_t value) \
    { \
        return value; \
    }

#define YAGL_DEFINE_CPUTO_SWAP(type, endian) \
    static __inline type##_t yagl_cpu_to_##type##endian(type##_t value) \
    { \
        return yagl_swap_##type(value); \
    }

#define YAGL_DEFINE_TOCPU_NOOP(type, endian) \
    static __inline type##_t yagl_##type##endian##_to_cpu(type##_t value) \
    { \
        return value; \
    }

#define YAGL_DEFINE_TOCPU_SWAP(type, endian) \
    static __inline type##_t yagl_##type##endian##_to_cpu(type##_t value) \
    { \
        return yagl_swap_##type(value); \
    }

#ifdef YAGL_LITTLE_ENDIAN
#define YAGL_DEFINE_CPUTO_LE(type) YAGL_DEFINE_CPUTO_NOOP(type, le)
#define YAGL_DEFINE_CPUTO_BE(type) YAGL_DEFINE_CPUTO_SWAP(type, be)
#define YAGL_DEFINE_TOCPU_LE(type) YAGL_DEFINE_TOCPU_NOOP(type, le)
#define YAGL_DEFINE_TOCPU_BE(type) YAGL_DEFINE_TOCPU_SWAP(type, be)
#else
#define YAGL_DEFINE_CPUTO_LE(type) YAGL_DEFINE_CPUTO_SWAP(type, le)
#define YAGL_DEFINE_CPUTO_BE(type) YAGL_DEFINE_CPUTO_NOOP(type, be)
#define YAGL_DEFINE_TOCPU_LE(type) YAGL_DEFINE_TOCPU_SWAP(type, le)
#define YAGL_DEFINE_TOCPU_BE(type) YAGL_DEFINE_TOCPU_NOOP(type, be)
#endif

YAGL_DEFINE_CPUTO_LE(uint16)
YAGL_DEFINE_CPUTO_LE(uint32)
YAGL_DEFINE_CPUTO_LE(uint64)
YAGL_DEFINE_CPUTO_BE(uint16)
YAGL_DEFINE_CPUTO_BE(uint32)
YAGL_DEFINE_CPUTO_BE(uint64)
YAGL_DEFINE_TOCPU_LE(uint16)
YAGL_DEFINE_TOCPU_LE(uint32)
YAGL_DEFINE_TOCPU_LE(uint64)
YAGL_DEFINE_TOCPU_BE(uint16)
YAGL_DEFINE_TOCPU_BE(uint32)
YAGL_DEFINE_TOCPU_BE(uint64)

#endif
