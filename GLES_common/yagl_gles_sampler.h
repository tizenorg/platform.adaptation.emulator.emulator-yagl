#ifndef _YAGL_GLES_SAMPLER_H_
#define _YAGL_GLES_SAMPLER_H_

#include "yagl_types.h"
#include "yagl_object.h"

#define YAGL_NS_SAMPLER 4

struct yagl_gles_sampler
{
    struct yagl_object base;

    yagl_object_name global_name;

    GLenum min_filter;
    GLenum mag_filter;
    GLfloat min_lod;
    GLfloat max_lod;
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum wrap_r;
    GLenum compare_mode;
    GLenum compare_func;
};

struct yagl_gles_sampler *yagl_gles_sampler_create(void);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_sampler_acquire(struct yagl_gles_sampler *sampler);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles_sampler_release(struct yagl_gles_sampler *sampler);

void yagl_gles_sampler_bind(struct yagl_gles_sampler *sampler,
                            GLuint unit);

int yagl_gles_sampler_set_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLint *param);

int yagl_gles_sampler_set_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLfloat *param);

int yagl_gles_sampler_get_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLint *param);

int yagl_gles_sampler_get_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLfloat *param);

#endif
