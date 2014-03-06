#include "yagl_log.h"
#include "yagl_utils.h"
#include "yagl_malloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/time.h>

static char *my_strndup(const char *str, size_t len)
{
    size_t n;
    char *dst;

    n = strlen(str);
    if (len < n) {
        n = len;
    }

    dst = yagl_malloc(n + 1);

    memcpy(dst, str, n);
    dst[n] = '\0';

    return dst;
}

static const char* g_log_level_to_str[yagl_log_level_max + 1] =
{
    "OFF",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG",
    "TRACE"
};

static struct
{
    const char* datatype;
    const char* format;
} g_datatype_to_format[] =
{
    { "EGLboolean", "%u" },
    { "EGLenum", "%u" },
    { "EGLint", "%d" },
    { "EGLConfig", "%p" },
    { "EGLContext", "%p" },
    { "EGLDisplay", "%p" },
    { "EGLSurface", "%p" },
    { "EGLClientBuffer", "%p" },
    { "EGLNativeDisplayType", "%p" },
    { "yagl_host_handle", "%u" },
    { "int", "%d" },
    { "GLenum", "%u" },
    { "GLuint", "%u" },
    { "GLbitfield", "%u" },
    { "GLclampf", "%f" },
    { "GLfloat", "%f" },
    { "GLint", "%d" },
    { "GLsizei", "%d" },
    { "GLclampx", "%d" },
    { "GLfixed", "%d" },
    { "GLboolean", "%" PRIu8 },
    { "GLchar", "%" PRIi8 },
    { "GLintptr", "%ld" },
    { "GLsizeiptr", "%ld" },
    { "GLeglImageOES", "%p" },
    { "GLsync", "%p" }
};

static pthread_once_t g_log_init = PTHREAD_ONCE_INIT;
static yagl_log_level g_log_level = yagl_log_level_off;
static char** g_log_facilities_match = NULL;
static char** g_log_facilities_no_match = NULL;
static int g_log_func_trace = 0;
static pthread_mutex_t g_log_mutex;

static const char* yagl_log_datatype_to_format(const char* type)
{
    unsigned int i;

    if (strchr(type, '*'))
    {
        return "%p";
    }

    for (i = 0; i < sizeof(g_datatype_to_format)/sizeof(g_datatype_to_format[0]); ++i)
    {
        if (strcmp(g_datatype_to_format[i].datatype, type) == 0)
        {
            return g_datatype_to_format[i].format;
        }
    }

    return NULL;
}

static void yagl_log_print_current_time(void)
{
    char buff[128];
    struct tm tm;
    struct timeval tv = { 0, 0 };
    time_t ti;

    gettimeofday(&tv, NULL);

    ti = tv.tv_sec;

    localtime_r(&ti, &tm);
    strftime(buff, sizeof(buff),
             "%H:%M:%S", &tm);
    fprintf(stderr, "%s", buff);
}

/*
 * Simple asterisk pattern matcher.
 */
static int yagl_log_match(const char* str, const char* expr)
{
    while (*str && *expr)
    {
        /*
         * Swallow '**'.
         */

        while ((*expr == '*') &&
               (*(expr + 1) == '*'))
        {
            ++expr;
        }

        if (*expr == '*')
        {
            if (!*(expr + 1))
            {
                /*
                 * Last '*' always matches.
                 */

                return 1;
            }

            /*
             * Search for symbol after the asterisk.
             */

            while (*str && (*str != *(expr + 1)))
            {
                ++str;
            }

            if (!*str)
            {
                /*
                 * Reached the end, didn't find symbol following asterisk,
                 * no match.
                 */

                return 0;
            }

            /*
             * Jump to the symbol after the one that's after asterisk.
             */

            ++str;
            expr += 2;
        }
        else
        {
            /*
             * No asterisk, exact match.
             */

            if (*str != *expr)
            {
                return 0;
            }

            ++str;
            ++expr;
        }
    }

    /*
     * Remaining '*' always match.
     */

    while (*expr == '*')
    {
        ++expr;
    }

    return (*str == 0) && (*expr == 0);
}

static void yagl_log_init_once(void)
{
    char* level_str = getenv("YAGL_DEBUG");
    int level = level_str ? atoi(level_str) : yagl_log_level_off;
    char* facilities;
    char* func_trace;

    yagl_mutex_init(&g_log_mutex);

    if (level < 0)
    {
        g_log_level = yagl_log_level_off;
    }
    else if (level > yagl_log_level_max)
    {
        g_log_level = (yagl_log_level)yagl_log_level_max;
    }
    else
    {
        g_log_level = (yagl_log_level)level;
    }

    facilities = getenv("YAGL_DEBUG_FACILITIES");

    if (facilities)
    {
        char* tmp_facilities = facilities;
        int i = 0, num_match = 0, num_no_match = 0;

        while (1)
        {
            char* tmp = strchr(tmp_facilities, ',');

            if (!tmp)
            {
                break;
            }

            if (tmp - tmp_facilities > 0)
            {
                ++i;
            }

            tmp_facilities = tmp + 1;
        }

        if (strlen(tmp_facilities) > 0)
        {
            ++i;
        }

        g_log_facilities_match = yagl_malloc0((i + 1) * sizeof(char*));
        g_log_facilities_no_match = yagl_malloc0((i + 1) * sizeof(char*));

        tmp_facilities = facilities;

        while (1)
        {
            char* tmp = strchr(tmp_facilities, ',');

            if (!tmp)
            {
                break;
            }

            if (tmp - tmp_facilities > 0)
            {
                if (*tmp_facilities == '^')
                {
                    if ((tmp - tmp_facilities - 1) > 0)
                    {
                        g_log_facilities_no_match[num_no_match] = my_strndup(tmp_facilities + 1, tmp - tmp_facilities - 1);
                        ++num_no_match;
                    }
                }
                else
                {
                    g_log_facilities_match[num_match] = my_strndup(tmp_facilities, tmp - tmp_facilities);
                    ++num_match;
                }
            }

            tmp_facilities = tmp + 1;
        }

        if (strlen(tmp_facilities) > 0)
        {
            if (*tmp_facilities == '^')
            {
                if ((strlen(tmp_facilities) - 1) > 0)
                {
                    g_log_facilities_no_match[num_no_match] = strdup(tmp_facilities + 1);
                    ++num_no_match;
                }
            }
            else
            {
                g_log_facilities_match[num_match] = strdup(tmp_facilities);
                ++num_match;
            }
        }

        g_log_facilities_no_match[num_no_match] = NULL;
        g_log_facilities_match[num_match] = NULL;

        if (!num_no_match)
        {
            yagl_free(g_log_facilities_no_match);
            g_log_facilities_no_match = NULL;
        }

        if (!num_match)
        {
            yagl_free(g_log_facilities_match);
            g_log_facilities_match = NULL;
        }
    }

    func_trace = getenv("YAGL_DEBUG_FUNC_TRACE");

    g_log_func_trace = func_trace ? (atoi(func_trace) > 0) : 0;
}

void yagl_log_init(void)
{
    pthread_once(&g_log_init, yagl_log_init_once);
}

void yagl_log_event(yagl_log_level log_level,
                    pid_t process_id,
                    pid_t thread_id,
                    const char* facility,
                    int line,
                    const char* format, ...)
{
    va_list args;

    yagl_log_init();

    pthread_mutex_lock(&g_log_mutex);

    yagl_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u/%u] %s:%d",
            g_log_level_to_str[log_level],
            process_id,
            thread_id,
            facility,
            line);
    if (format)
    {
        va_start(args, format);
        fprintf(stderr, " - ");
        vfprintf(stderr, format, args);
        va_end(args);
    }
    fprintf(stderr, "\n");

    pthread_mutex_unlock(&g_log_mutex);
}

void yagl_log_func_enter(pid_t process_id,
                         pid_t thread_id,
                         const char* func,
                         int line,
                         const char* format, ...)
{
    va_list args;

    yagl_log_init();

    pthread_mutex_lock(&g_log_mutex);

    yagl_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u/%u] {{{ %s(",
            g_log_level_to_str[yagl_log_level_trace],
            process_id,
            thread_id,
            func);
    if (format)
    {
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
    fprintf(stderr, "):%d\n", line);

    pthread_mutex_unlock(&g_log_mutex);
}

void yagl_log_func_exit(pid_t process_id,
                        pid_t thread_id,
                        const char* func,
                        int line,
                        const char* format, ...)
{
    va_list args;

    yagl_log_init();

    pthread_mutex_lock(&g_log_mutex);

    yagl_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u/%u] }}} %s:%d",
            g_log_level_to_str[yagl_log_level_trace],
            process_id,
            thread_id,
            func,
            line);
    if (format)
    {
        va_start(args, format);
        fprintf(stderr, " = ");
        vfprintf(stderr, format, args);
        va_end(args);
    }
    fprintf(stderr, "\n");

    pthread_mutex_unlock(&g_log_mutex);
}

void yagl_log_func_enter_split(pid_t process_id,
                               pid_t thread_id,
                               const char* func,
                               int line,
                               int num_args, ...)
{
    char format[1025] = { '\0' };
    va_list args;

    yagl_log_init();

    pthread_mutex_lock(&g_log_mutex);

    yagl_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u/%u] {{{ %s(",
            g_log_level_to_str[yagl_log_level_trace],
            process_id,
            thread_id,
            func);

    if (num_args > 0)
    {
        int i, skip = 0;

        va_start(args, num_args);

        for (i = 0; i < num_args;)
        {
            const char* arg_format = yagl_log_datatype_to_format(va_arg(args, const char*));
            const char* arg_name;

            if (!arg_format)
            {
                skip = 1;
                break;
            }

            arg_name = va_arg(args, const char*);

            strcat(format, arg_name);
            strcat(format, " = ");
            strcat(format, arg_format);

            ++i;

            if (i < num_args)
            {
                strcat(format, ", ");
            }
        }

        if (skip)
        {
            fprintf(stderr, "...");
        }
        else
        {
            vfprintf(stderr, format, args);
        }

        va_end(args);
    }

    fprintf(stderr, "):%d\n", line);

    pthread_mutex_unlock(&g_log_mutex);
}

void yagl_log_func_exit_split(pid_t process_id,
                              pid_t thread_id,
                              const char* func,
                              int line,
                              const char* datatype, ...)
{
    va_list args;

    yagl_log_init();

    pthread_mutex_lock(&g_log_mutex);

    yagl_log_print_current_time();
    fprintf(stderr,
            " %-5s [%u/%u] }}} %s:%d",
            g_log_level_to_str[yagl_log_level_trace],
            process_id,
            thread_id,
            func,
            line);

    if (datatype)
    {
        const char* format = yagl_log_datatype_to_format(datatype);
        if (format)
        {
            fprintf(stderr, " = ");
            va_start(args, datatype);
            vfprintf(stderr, format, args);
            va_end(args);
        }
    }

    fprintf(stderr, "\n");

    pthread_mutex_unlock(&g_log_mutex);
}

int yagl_log_is_enabled_for_level(yagl_log_level log_level)
{
    yagl_log_init();

    return log_level <= g_log_level;
}

int yagl_log_is_enabled_for_facility(const char* facility)
{
    int i;
    int res = 0;

    yagl_log_init();

    if (g_log_facilities_match)
    {
        for (i = 0; g_log_facilities_match[i]; ++i)
        {
            if (yagl_log_match(facility, g_log_facilities_match[i]))
            {
                res = 1;
                break;
            }
        }
    }
    else
    {
        res = 1;
    }

    if (res && g_log_facilities_no_match)
    {
        for (i = 0; g_log_facilities_no_match[i]; ++i)
        {
            if (yagl_log_match(facility, g_log_facilities_no_match[i]))
            {
                res = 0;
                break;
            }
        }
    }

    return res;
}

int yagl_log_is_enabled_for_func_tracing(void)
{
    yagl_log_init();

    return g_log_func_trace;
}
