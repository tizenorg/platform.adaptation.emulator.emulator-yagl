#ifndef _YAGL_PLATFORM_H_
#define _YAGL_PLATFORM_H_

#if defined(__i386) || defined(_M_IX86)
#define YAGL_LITTLE_ENDIAN
#elif defined(__x86_64) || defined(_M_X64) || defined(_M_IA64)
#define YAGL_LITTLE_ENDIAN
#elif defined(__arm__)
#define YAGL_LITTLE_ENDIAN
#else
#error Unknown architecture
#endif

#if defined(__x86_64) || defined(_M_X64) || defined(_M_IA64) || defined(__LP64__)
#define YAGL_64
#else
#define YAGL_32
#endif

#if !defined(YAGL_64) && !defined(YAGL_32)
#error 32 or 64 bit mode must be set
#endif

#endif
