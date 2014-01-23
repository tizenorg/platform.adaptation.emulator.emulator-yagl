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

#ifndef _YAGL_TEXCOMPRESS_ETC_TMP_H_
#define _YAGL_TEXCOMPRESS_ETC_TMP_H_

#define MIN2(A, B) ((A)<(B) ? (A) : (B))

struct TAG(etc1_block) {
   uint32_t pixel_indices;
   int flipped;
   const int *modifier_tables[2];
   UINT8_TYPE base_colors[2][3];
};

static UINT8_TYPE
TAG(etc1_base_color_diff_hi)(UINT8_TYPE in)
{
   return (in & 0xf8) | (in >> 5);
}

static UINT8_TYPE
TAG(etc1_base_color_diff_lo)(UINT8_TYPE in)
{
   static const int lookup[8] = { 0, 1, 2, 3, -4, -3, -2, -1 };

   in = (in >> 3) + lookup[in & 0x7];

   return (in << 3) | (in >> 2);
}

static UINT8_TYPE
TAG(etc1_base_color_ind_hi)(UINT8_TYPE in)
{
   return (in & 0xf0) | ((in & 0xf0) >> 4);
}

static UINT8_TYPE
TAG(etc1_base_color_ind_lo)(UINT8_TYPE in)
{
   return ((in & 0xf) << 4) | (in & 0xf);
}

static const int TAG(etc1_modifier_tables)[8][4] = {
   {  2,   8,  -2,   -8},
   {  5,  17,  -5,  -17},
   {  9,  29,  -9,  -29},
   { 13,  42, -13,  -42},
   { 18,  60, -18,  -60},
   { 24,  80, -24,  -80},
   { 33, 106, -33, -106},
   { 47, 183, -47, -183}
};

#endif
