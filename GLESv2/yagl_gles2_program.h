#ifndef _YAGL_GLES2_PROGRAM_H_
#define _YAGL_GLES2_PROGRAM_H_

#include "yagl_types.h"
#include "yagl_object.h"
#include "yagl_list.h"
#include "yagl_vector.h"

struct yagl_gles2_shader;

struct yagl_gles2_attrib_variable
{
    int fetched;

    GLchar *name;

    GLint name_size;

    GLenum type;
    GLint size;
};

struct yagl_gles2_uniform_variable
{
    int name_fetched;
    int generic_fetched;
    int extended_fetched;

    /*
     * Common parameters, present when
     * 'name_fetched', 'generic_fetched' or 'extended_fetched' is true.
     * @{
     */
    GLint name_size;
    /*
     * @}
     */

    /*
     * Common parameters, present when
     * 'generic_fetched' or 'extended_fetched' is true.
     * @{
     */
    GLenum type;
    GLint size;
    /*
     * @}
     */

    /*
     * 'name_fetched' or 'generic_fetched'.
     * @{
     */
    GLchar *name;
    /*
     * @}
     */

    /*
     * 'extended_fetched'.
     * @{
     */
    GLint block_index;
    GLint block_offset;
    GLint array_stride;
    GLint matrix_stride;
    GLint is_row_major;
    /*
     * @}
     */
};

struct yagl_gles2_uniform_block
{
    int name_fetched;
    int params_fetched;

    GLuint binding;

    /*
     * Present when 'name_fetched' or 'params_fetched' is true.
     * @{
     */
    GLint name_size;
    /*
     * @}
     */

    /*
     * 'name_fetched'.
     * @{
     */
    GLchar *name;
    /*
     * @}
     */

    /*
     * 'params_fetched'.
     * @{
     */
    GLuint num_active_uniform_indices;
    GLint data_size;
    GLint referenced_by_vertex_shader;
    GLint referenced_by_fragment_shader;
    /*
     * @}
     */

    GLuint *active_uniform_indices;
};

struct yagl_gles2_transform_feedback_varying
{
    /*
     * Valid when struct yagl_gles2_transform_feedback_info::fetched is true.
     * @{
     */

    /*
     * These will be set on link and
     * updated on fetch.
     * @{
     */
    GLchar *name;
    GLint name_size;
    /*
     * @}
     */

    GLenum type;
    GLint size;

    /*
     * @}
     */
};

struct yagl_gles2_transform_feedback_info
{
    int fetched;

    struct yagl_gles2_transform_feedback_varying *varyings;
    GLuint num_varyings;
    GLenum buffer_mode;
    GLint max_varying_bufsize;
};

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

    struct yagl_gles2_attrib_variable *active_attribs;
    GLuint num_active_attribs;
    GLint max_active_attrib_bufsize;

    struct yagl_gles2_uniform_variable *active_uniforms;
    GLuint num_active_uniforms;
    GLint max_active_uniform_bufsize;

    struct yagl_gles2_uniform_block *active_uniform_blocks;
    GLuint num_active_uniform_blocks;
    GLint max_active_uniform_block_bufsize;

    /*
     * Transform feedback info specified via glTransformFeedbackVaryings.
     */
    struct yagl_gles2_transform_feedback_info transform_feedback_info;

    /*
     * Actual transform feedback info after program link.
     */
    struct yagl_gles2_transform_feedback_info linked_transform_feedback_info;

    int linked;

    GLint link_status;
    GLint info_log_length;
};

int yagl_gles2_program_translate_location(struct yagl_gles2_program *program,
                                          GLint location,
                                          uint32_t *global_location);

void yagl_gles2_transform_feedback_info_reset(
    struct yagl_gles2_transform_feedback_info *transform_feedback_info);

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

void yagl_gles2_program_get_active_uniform(struct yagl_gles2_program *program,
                                           GLuint index,
                                           GLsizei bufsize,
                                           GLsizei *length,
                                           GLint *size,
                                           GLenum *type,
                                           GLchar *name);

void yagl_gles2_program_get_active_attrib(struct yagl_gles2_program *program,
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
