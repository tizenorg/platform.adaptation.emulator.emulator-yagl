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

#ifndef _YAGL_OBJECT_H_
#define _YAGL_OBJECT_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_ref.h"

/*
 * This represents OpenGL client object such as buffer, texture, etc.
 * An object can have a "local name", i.e. name that is visible to an
 * OpenGL user, that name is unique only within a particular namespace
 * inside a particular context (or sharegroup to be more precise).
 * Object implementation will typically also have one or more
 * "global name"s, which are unique per-process, these names are
 * used for target <-> host interactions, they're not visible to
 * OpenGL user, they're implementation details.
 * It's possible to have an object without a local name, such object
 * won't be visible to OpenGL user, but can be used internally.
 *
 * In OpenGL one can often spot the following scenario:
 * N = glGenXXX(...) - creates object, returns its name N
 * glBindXXX(N, ...) - binds name N
 * glDeleteXXX(N, ...) - frees name N
 * glGetIntegerv(GL_XXX_BINDING, ...) - must return N though it's free
 * Thus, local name for an object must be available even after object
 * is removed from a namespace.
 */

struct yagl_object
{
    struct yagl_ref ref;

    yagl_object_name local_name;
};

/*
 * For implementations.
 * @{
 */

YAGL_API void yagl_object_init(struct yagl_object *obj,
                               yagl_ref_destroy_func destroy);
YAGL_API void yagl_object_cleanup(struct yagl_object *obj);

/*
 * @}
 */

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_object_acquire(struct yagl_object *obj);

/*
 * Passing NULL won't hurt, this is for convenience.
 */
YAGL_API void yagl_object_release(struct yagl_object *obj);

#endif
