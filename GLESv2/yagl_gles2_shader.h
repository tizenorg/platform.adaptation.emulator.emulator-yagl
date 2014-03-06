#ifndef _YAGL_GLES2_SHADER_H_
#define _YAGL_GLES2_SHADER_H_

#include "yagl_types.h"
#include "yagl_object.h"

/*
 * Programs and shaders share the same namespace,
 * pretty clumsy!
 */
#define YAGL_NS_SHADER_PROGRAM 3

struct yagl_gles2_shader
{
    /*
     * These members must be exactly as in yagl_gles2_program
     * @{
     */
    struct yagl_object base;

    int is_shader;
    /*
     * @}
     */

    yagl_object_name global_name;

    GLenum type;

    GLchar *source;
};

struct yagl_gles2_shader *yagl_gles2_shader_create(GLenum type);

/*
 * Takes ownership of 'source'.
 */
void yagl_gles2_shader_source(struct yagl_gles2_shader *shader,
                              GLchar *source,
                              const GLchar *patched_source,
                              int patched_len);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_shader_acquire(struct yagl_gles2_shader *shader);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_shader_release(struct yagl_gles2_shader *shader);

#endif
