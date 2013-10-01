#ifndef _YAGL_STATE_H_
#define _YAGL_STATE_H_

#include "yagl_export.h"
#include "yagl_types.h"

struct yagl_transport;

YAGL_API struct yagl_transport *yagl_get_transport();

YAGL_API uint8_t *yagl_get_tmp_buffer(uint32_t size);

YAGL_API yagl_object_name yagl_get_global_name();

#endif
