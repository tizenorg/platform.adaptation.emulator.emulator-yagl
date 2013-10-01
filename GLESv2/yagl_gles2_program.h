#ifndef _YAGL_GLES2_PROGRAM_H_
#define _YAGL_GLES2_PROGRAM_H_

#include "yagl_types.h"
#include "yagl_object.h"
#include "yagl_list.h"
#include "yagl_vector.h"

struct yagl_gles2_shader;

struct yagl_gles2_program
{
    /*
     * These members must be exactly as in yagl_gles2_shader
     * @{
     */
    struct yagl_object base;

    int is_shader;
    /*
     * @}
     */

    /*
     * Generate uniform locations ourselves or vmexit
     * and ask host.
     */
    int gen_locations;

    yagl_object_name global_name;

    struct yagl_gles2_shader *vertex_shader;

    struct yagl_gles2_shader *fragment_shader;

    union
    {
        struct yagl_list uniform_locations_l;
        struct yagl_vector uniform_locations_v;
    };

    struct yagl_list attrib_locations;

    struct yagl_vector active_uniforms;
    struct yagl_vector active_attribs;

    int linked;
};

struct yagl_gles2_program *yagl_gles2_program_create(int gen_locations);

int yagl_gles2_program_attach_shader(struct yagl_gles2_program *program,
                                     struct yagl_gles2_shader *shader);

int yagl_gles2_program_detach_shader(struct yagl_gles2_program *program,
                                     struct yagl_gles2_shader *shader);

void yagl_gles2_program_link(struct yagl_gles2_program *program);

int yagl_gles2_program_get_uniform_location(struct yagl_gles2_program *program,
                                            const GLchar *name);

int yagl_gles2_program_get_attrib_location(struct yagl_gles2_program *program,
                                           const GLchar *name);

int yagl_gles2_program_get_active_uniform(struct yagl_gles2_program *program,
                                          GLuint index,
                                          GLsizei bufsize,
                                          GLsizei *length,
                                          GLint *size,
                                          GLenum *type,
                                          GLchar *name);

int yagl_gles2_program_get_active_attrib(struct yagl_gles2_program *program,
                                         GLuint index,
                                         GLsizei bufsize,
                                         GLsizei *length,
                                         GLint *size,
                                         GLenum *type,
                                         GLchar *name);

int yagl_gles2_program_get_uniformfv(struct yagl_gles2_program *program,
                                     GLint location,
                                     GLfloat *params);

int yagl_gles2_program_get_uniformiv(struct yagl_gles2_program *program,
                                     GLint location,
                                     GLint *params);

int yagl_gles2_program_uniform1f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x);

int yagl_gles2_program_uniform1fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v);

int yagl_gles2_program_uniform1i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x);

int yagl_gles2_program_uniform1iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v);

int yagl_gles2_program_uniform2f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y);

int yagl_gles2_program_uniform2fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v);

int yagl_gles2_program_uniform2i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y);

int yagl_gles2_program_uniform2iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v);

int yagl_gles2_program_uniform3f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z);

int yagl_gles2_program_uniform3fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v);

int yagl_gles2_program_uniform3i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y,
                                 GLint z);

int yagl_gles2_program_uniform3iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v);

int yagl_gles2_program_uniform4f(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z,
                                 GLfloat w);

int yagl_gles2_program_uniform4fv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLfloat *v);

int yagl_gles2_program_uniform4i(struct yagl_gles2_program *program,
                                 GLint location,
                                 GLint x,
                                 GLint y,
                                 GLint z,
                                 GLint w);

int yagl_gles2_program_uniform4iv(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLsizei count,
                                  const GLint *v);

int yagl_gles2_program_uniform_matrix2fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);

int yagl_gles2_program_uniform_matrix3fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);

int yagl_gles2_program_uniform_matrix4fv(struct yagl_gles2_program *program,
                                         GLint location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value);
/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_program_acquire(struct yagl_gles2_program *program);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
void yagl_gles2_program_release(struct yagl_gles2_program *program);

#endif
