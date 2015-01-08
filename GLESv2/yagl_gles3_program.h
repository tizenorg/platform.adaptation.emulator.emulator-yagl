/*
 * YaGL
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact :
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#ifndef _YAGL_GLES3_PROGRAM_H_
#define _YAGL_GLES3_PROGRAM_H_

#include "yagl_gles2_program.h"

int yagl_gles3_program_get_active_uniformsiv(struct yagl_gles2_program *program,
                                             const GLuint *indices,
                                             int num_indices,
                                             GLenum pname,
                                             GLint *params);

void yagl_gles3_program_get_uniform_indices(struct yagl_gles2_program *program,
                                            const GLchar *const *names,
                                            int num_names,
                                            GLuint *indices);

GLuint yagl_gles3_program_get_uniform_block_index(struct yagl_gles2_program *program,
                                                  const GLchar *block_name);

void yagl_gles3_program_set_uniform_block_binding(struct yagl_gles2_program *program,
                                                  GLuint block_index,
                                                  GLuint block_binding);

void yagl_gles3_program_get_active_uniform_block_name(struct yagl_gles2_program *program,
                                                      GLuint block_index,
                                                      GLsizei bufsize,
                                                      GLsizei *length,
                                                      GLchar *block_name);

int yagl_gles3_program_get_active_uniform_blockiv(struct yagl_gles2_program *program,
                                                  GLuint block_index,
                                                  GLenum pname,
                                                  GLint *params);

void yagl_gles3_program_set_transform_feedback_varyings(struct yagl_gles2_program *program,
                                                        const GLchar *const *varyings,
                                                        GLuint num_varyings,
                                                        GLenum buffer_mode);

void yagl_gles3_program_get_transform_feedback_varying(struct yagl_gles2_program *program,
                                                       GLuint index,
                                                       GLsizei bufsize,
                                                       GLsizei *length,
                                                       GLsizei *size,
                                                       GLenum *type,
                                                       GLchar *name);

int yagl_gles3_program_get_uniformuiv(struct yagl_gles2_program *program,
                                      GLint location,
                                      GLuint *params);

int yagl_gles3_program_uniform1ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0);

int yagl_gles3_program_uniform2ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1);

int yagl_gles3_program_uniform3ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1,
                                  GLuint v2);

int yagl_gles3_program_uniform4ui(struct yagl_gles2_program *program,
                                  GLint location,
                                  GLuint v0,
                                  GLuint v1,
                                  GLuint v2,
                                  GLuint v3);

int yagl_gles3_program_uniform1uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v);

int yagl_gles3_program_uniform2uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v);

int yagl_gles3_program_uniform3uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v);

int yagl_gles3_program_uniform4uiv(struct yagl_gles2_program *program,
                                   GLint location,
                                   GLsizei count,
                                   const GLuint *v);

int yagl_gles3_program_uniform_matrix2x3fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_uniform_matrix2x4fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_uniform_matrix3x2fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_uniform_matrix3x4fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_uniform_matrix4x2fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_uniform_matrix4x3fv(struct yagl_gles2_program *program,
                                           GLint location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value);

int yagl_gles3_program_get_frag_data_location(struct yagl_gles2_program *program,
                                              const GLchar *name);

#endif
