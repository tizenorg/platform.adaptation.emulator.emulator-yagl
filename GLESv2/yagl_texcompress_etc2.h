/*
 * Copyright (C) 2011 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _YAGL_TEXCOMPRESS_ETC2_H_
#define _YAGL_TEXCOMPRESS_ETC2_H_

#include "yagl_types.h"

void yagl_texcompress_etc2_unpack_r11(uint8_t *dst_row,
                                      unsigned dst_stride,
                                      const uint8_t *src_row,
                                      unsigned src_stride,
                                      unsigned width,
                                      unsigned height);

void yagl_texcompress_etc2_unpack_signed_r11(uint8_t *dst_row,
                                             unsigned dst_stride,
                                             const uint8_t *src_row,
                                             unsigned src_stride,
                                             unsigned width,
                                             unsigned height);

void yagl_texcompress_etc2_unpack_rg11(uint8_t *dst_row,
                                       unsigned dst_stride,
                                       const uint8_t *src_row,
                                       unsigned src_stride,
                                       unsigned width,
                                       unsigned height);

void yagl_texcompress_etc2_unpack_signed_rg11(uint8_t *dst_row,
                                              unsigned dst_stride,
                                              const uint8_t *src_row,
                                              unsigned src_stride,
                                              unsigned width,
                                              unsigned height);

void yagl_texcompress_etc2_unpack_rgb8(uint8_t *dst_row,
                                       unsigned dst_stride,
                                       const uint8_t *src_row,
                                       unsigned src_stride,
                                       unsigned width,
                                       unsigned height);

void yagl_texcompress_etc2_unpack_srgb8(uint8_t *dst_row,
                                        unsigned dst_stride,
                                        const uint8_t *src_row,
                                        unsigned src_stride,
                                        unsigned width,
                                        unsigned height);

void yagl_texcompress_etc2_unpack_rgb8_punchthrough_alpha1(uint8_t *dst_row,
                                                           unsigned dst_stride,
                                                           const uint8_t *src_row,
                                                           unsigned src_stride,
                                                           unsigned width,
                                                           unsigned height);

void yagl_texcompress_etc2_unpack_srgb8_punchthrough_alpha1(uint8_t *dst_row,
                                                            unsigned dst_stride,
                                                            const uint8_t *src_row,
                                                            unsigned src_stride,
                                                            unsigned width,
                                                            unsigned height);

void yagl_texcompress_etc2_unpack_rgba8(uint8_t *dst_row,
                                        unsigned dst_stride,
                                        const uint8_t *src_row,
                                        unsigned src_stride,
                                        unsigned width,
                                        unsigned height);

void yagl_texcompress_etc2_unpack_srgb8_alpha8(uint8_t *dst_row,
                                               unsigned dst_stride,
                                               const uint8_t *src_row,
                                               unsigned src_stride,
                                               unsigned width,
                                               unsigned height);

#endif
