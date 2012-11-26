#ifndef _YAGL_EGL_STATE_H_
#define _YAGL_EGL_STATE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include <EGL/egl.h>

struct yagl_context;

/*
 * Those return per-thread values.
 * @{
 */

EGLint yagl_get_error();

void yagl_set_error(EGLint error);

EGLenum yagl_get_api();

void yagl_set_api(EGLenum api);

struct yagl_context *yagl_get_context();

void yagl_set_context(struct yagl_context *ctx);

void yagl_reset_state();

/*
 * @}
 */

#endif
