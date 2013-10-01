#ifndef _YAGL_EGL_STATE_H_
#define _YAGL_EGL_STATE_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "EGL/egl.h"

struct yagl_context;
struct yagl_surface;

void *yagl_get_gles1_sym(const char *name);
void *yagl_get_gles2_sym(const char *name);

/*
 * Those return per-thread values.
 * @{
 */

EGLint yagl_get_error();

void yagl_set_error(EGLint error);

EGLenum yagl_get_api();

void yagl_set_api(EGLenum api);

struct yagl_context *yagl_get_context();
struct yagl_surface *yagl_get_draw_surface();
struct yagl_surface *yagl_get_read_surface();

int yagl_set_context(struct yagl_context *ctx,
                     struct yagl_surface *draw_sfc,
                     struct yagl_surface *read_sfc);

void yagl_reset_state();

struct yagl_client_interface *yagl_get_client_interface(yagl_client_api client_api);

/*
 * @}
 */

#endif
