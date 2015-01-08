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

#ifndef _YAGL_IMPL_H_
#define _YAGL_IMPL_H_

#include "yagl_export.h"
#include "yagl_types.h"
#include "yagl_log.h"

/*
 * Some macros to help implement bare passthrough.
 */

#define YAGL_IMPLEMENT_API_RET0(ret_type, func) \
    YAGL_API ret_type func() \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT0(func); \
        tmp = yagl_host_##func(); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET0(func) \
    YAGL_API void func() \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT0(func); \
        yagl_host_##func(); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET1(ret_type, func, a0_type, a0) \
    YAGL_API ret_type func(a0_type a0) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT1(func, a0_type, a0); \
        tmp = yagl_host_##func(a0); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET1(func, a0_type, a0) \
    YAGL_API void func(a0_type a0) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT1(func, a0_type, a0); \
        yagl_host_##func(a0); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET2(ret_type, func, a0_type, a1_type, a0, a1) \
    YAGL_API ret_type func(a0_type a0, a1_type a1) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT2(func, a0_type, a1_type, a0, a1); \
        tmp = yagl_host_##func(a0, a1); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET2(func, a0_type, a1_type, a0, a1) \
    YAGL_API void func(a0_type a0, a1_type a1) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT2(func, a0_type, a1_type, a0, a1); \
        yagl_host_##func(a0, a1); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET3(ret_type, func, a0_type, a1_type, a2_type, a0, a1, a2) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT3(func, a0_type, a1_type, a2_type, a0, a1, a2); \
        tmp = yagl_host_##func(a0, a1, a2); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET3(func, a0_type, a1_type, a2_type, a0, a1, a2) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT3(func, a0_type, a1_type, a2_type, a0, a1, a2); \
        yagl_host_##func(a0, a1, a2); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET4(ret_type, func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3); \
        tmp = yagl_host_##func(a0, a1, a2, a3); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3); \
        yagl_host_##func(a0, a1, a2, a3); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET5(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4); \
        tmp = yagl_host_##func(a0, a1, a2, a3, a4); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4); \
        yagl_host_##func(a0, a1, a2, a3, a4); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET6(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5); \
        tmp = yagl_host_##func(a0, a1, a2, a3, a4, a5); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5); \
        yagl_host_##func(a0, a1, a2, a3, a4, a5); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET7(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6); \
        tmp = yagl_host_##func(a0, a1, a2, a3, a4, a5, a6); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6); \
        yagl_host_##func(a0, a1, a2, a3, a4, a5, a6); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET8(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7); \
        tmp = yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7); \
        yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET9(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7, a8_type a8) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        tmp = yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7, a8_type a8) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#endif
