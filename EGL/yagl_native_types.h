#ifndef _YAGL_NATIVE_TYPES_H_
#define _YAGL_NATIVE_TYPES_H_

#include "yagl_export.h"
#include "yagl_types.h"

typedef uintptr_t yagl_os_display;
typedef uintptr_t yagl_os_drawable;
typedef uintptr_t yagl_os_window;
typedef uintptr_t yagl_os_pixmap;

typedef enum
{
    yagl_native_attachment_front,
    yagl_native_attachment_back,
} yagl_native_attachment;

#endif
