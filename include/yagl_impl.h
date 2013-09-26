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
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET0(func) \
    YAGL_API void func() \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT0(func); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func()); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET1(ret_type, func, a0_type, a0) \
    YAGL_API ret_type func(a0_type a0) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT1(func, a0_type, a0); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET1(func, a0_type, a0) \
    YAGL_API void func(a0_type a0) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT1(func, a0_type, a0); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET2(ret_type, func, a0_type, a1_type, a0, a1) \
    YAGL_API ret_type func(a0_type a0, a1_type a1) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT2(func, a0_type, a1_type, a0, a1); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET2(func, a0_type, a1_type, a0, a1) \
    YAGL_API void func(a0_type a0, a1_type a1) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT2(func, a0_type, a1_type, a0, a1); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET3(ret_type, func, a0_type, a1_type, a2_type, a0, a1, a2) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT3(func, a0_type, a1_type, a2_type, a0, a1, a2); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET3(func, a0_type, a1_type, a2_type, a0, a1, a2) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT3(func, a0_type, a1_type, a2_type, a0, a1, a2); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET4(ret_type, func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT4(func, a0_type, a1_type, a2_type, a3_type, a0, a1, a2, a3); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET5(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3, a4)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT5(func, a0_type, a1_type, a2_type, a3_type, a4_type, a0, a1, a2, a3, a4); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3, a4)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET6(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3, a4, a5)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT6(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a0, a1, a2, a3, a4, a5); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3, a4, a5)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET7(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3, a4, a5, a6)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT7(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a0, a1, a2, a3, a4, a5, a6); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3, a4, a5, a6)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET8(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3, a4, a5, a6, a7)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT8(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a0, a1, a2, a3, a4, a5, a6, a7); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#define YAGL_IMPLEMENT_API_RET9(ret_type, func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
    YAGL_API ret_type func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7, a8_type a8) \
    { \
        ret_type tmp; \
        YAGL_LOG_FUNC_ENTER_SPLIT9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(&tmp, a0, a1, a2, a3, a4, a5, a6, a7, a8)); \
        YAGL_LOG_FUNC_EXIT_SPLIT(ret_type, tmp); \
        return tmp; \
    }

#define YAGL_IMPLEMENT_API_NORET9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
    YAGL_API void func(a0_type a0, a1_type a1, a2_type a2, a3_type a3, a4_type a4, a5_type a5, a6_type a6, a7_type a7, a8_type a8) \
    { \
        YAGL_LOG_FUNC_ENTER_SPLIT9(func, a0_type, a1_type, a2_type, a3_type, a4_type, a5_type, a6_type, a7_type, a8_type, a0, a1, a2, a3, a4, a5, a6, a7, a8); \
        YAGL_HOST_CALL_ASSERT(yagl_host_##func(a0, a1, a2, a3, a4, a5, a6, a7, a8)); \
        YAGL_LOG_FUNC_EXIT(NULL); \
    }

#endif
