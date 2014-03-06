#include "GL/gl.h"
#include "yagl_gles_sampler.h"
#include "yagl_malloc.h"
#include "yagl_state.h"
#include "yagl_host_gles_calls.h"

static void yagl_gles_sampler_destroy(struct yagl_ref *ref)
{
    struct yagl_gles_sampler *sampler = (struct yagl_gles_sampler*)ref;

    yagl_host_glDeleteObjects(&sampler->global_name, 1);

    yagl_object_cleanup(&sampler->base);

    yagl_free(sampler);
}

struct yagl_gles_sampler *yagl_gles_sampler_create(void)
{
    struct yagl_gles_sampler *sampler;

    sampler = yagl_malloc0(sizeof(*sampler));

    yagl_object_init(&sampler->base, &yagl_gles_sampler_destroy);

    sampler->global_name = yagl_get_global_name();

    sampler->min_filter = GL_NEAREST_MIPMAP_LINEAR;
    sampler->mag_filter = GL_LINEAR;
    sampler->min_lod = -1000;
    sampler->max_lod = -1000;
    sampler->wrap_s = GL_REPEAT;
    sampler->wrap_t = GL_REPEAT;
    sampler->wrap_r = GL_REPEAT;
    sampler->compare_mode = GL_NONE;
    sampler->compare_func = GL_LEQUAL;

    yagl_host_glGenSamplers(&sampler->global_name, 1);

    return sampler;
}

void yagl_gles_sampler_acquire(struct yagl_gles_sampler *sampler)
{
    if (sampler) {
        yagl_object_acquire(&sampler->base);
    }
}

void yagl_gles_sampler_release(struct yagl_gles_sampler *sampler)
{
    if (sampler) {
        yagl_object_release(&sampler->base);
    }
}

void yagl_gles_sampler_bind(struct yagl_gles_sampler *sampler,
                            GLuint unit)
{
    if (!sampler) {
        yagl_host_glBindSampler(unit, 0);
        return;
    }

    yagl_host_glBindSampler(unit, sampler->global_name);
}

int yagl_gles_sampler_set_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLint *param)
{
    switch (pname) {
    case GL_TEXTURE_MIN_FILTER:
        sampler->min_filter = param[0];
        break;
    case GL_TEXTURE_MAG_FILTER:
        sampler->mag_filter = param[0];
        break;
    case GL_TEXTURE_WRAP_S:
        sampler->wrap_s = param[0];
        break;
    case GL_TEXTURE_WRAP_T:
        sampler->wrap_t = param[0];
        break;
    case GL_TEXTURE_WRAP_R:
        sampler->wrap_r = param[0];
        break;
    case GL_TEXTURE_COMPARE_MODE:
        sampler->compare_mode = param[0];
        break;
    case GL_TEXTURE_COMPARE_FUNC:
        sampler->compare_func = param[0];
        break;
    default:
        return 0;
    }

    yagl_host_glSamplerParameteriv(sampler->global_name, pname, param, 1);

    return 1;
}

int yagl_gles_sampler_set_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, const GLfloat *param)
{
    switch (pname) {
    case GL_TEXTURE_MIN_LOD:
        sampler->min_lod = param[0];
        break;
    case GL_TEXTURE_MAX_LOD:
        sampler->max_lod = param[0];
        break;
    default:
        return 0;
    }

    yagl_host_glSamplerParameterfv(sampler->global_name, pname, param, 1);

    return 1;
}

int yagl_gles_sampler_get_parameteriv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLint *param)
{
    switch (pname) {
    case GL_TEXTURE_MIN_FILTER:
        param[0] = sampler->min_filter;
        break;
    case GL_TEXTURE_MAG_FILTER:
        param[0] = sampler->mag_filter;
        break;
    case GL_TEXTURE_WRAP_S:
        param[0] = sampler->wrap_s;
        break;
    case GL_TEXTURE_WRAP_T:
        param[0] = sampler->wrap_t;
        break;
    case GL_TEXTURE_WRAP_R:
        param[0] = sampler->wrap_r;
        break;
    case GL_TEXTURE_COMPARE_MODE:
        param[0] = sampler->compare_mode;
        break;
    case GL_TEXTURE_COMPARE_FUNC:
        param[0] = sampler->compare_func;
        break;
    default:
        return 0;
    }

    return 1;
}

int yagl_gles_sampler_get_parameterfv(struct yagl_gles_sampler *sampler,
                                      GLenum pname, GLfloat *param)
{
    switch (pname) {
    case GL_TEXTURE_MIN_LOD:
        param[0] = sampler->min_lod;
        break;
    case GL_TEXTURE_MAX_LOD:
        param[0] = sampler->max_lod;
        break;
    default:
        return 0;
    }

    return 1;
}
