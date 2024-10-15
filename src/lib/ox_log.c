/******************************************************************************
 *                                 OX Language                                *
 *                                                                            *
 * Copyright 2024 Gong Ke                                                     *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the              *
 * "Software"), to deal in the Software without restriction, including        *
 * without limitation the rights to use, copy, modify, merge, publish,        *
 * distribute, sublicense, and/or sell copies of the Software, and to permit  *
 * persons to whom the Software is furnished to do so, subject to the         *
 * following conditions:                                                      *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included    *
 * in all copies or substantial portions of the Software.                     *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  *
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 ******************************************************************************/

/**
 * @file
 * Log output functions.
 */

#define OX_LOG_TAG "ox_log"

#include "ox_internal.h"

/**
 * Set the log output level.
 * Only the message which level greater than this value can be outputted.
 * @param ctxt The current running context.
 * @param level The log output level.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_log_set_level (OX_Context *ctxt, OX_LogLevel level)
{
    OX_VM *vm;

    assert(ctxt);
    assert((level >= OX_LOG_LEVEL_ALL) && (level <= OX_LOG_LEVEL_NONE));

    vm = ox_vm_get(ctxt);
    vm->log_level = level;

    return OX_OK;
}

/**
 * Get the current log output level.
 * @param ctxt The current running context.
 * @return The current log level.
 */
OX_LogLevel
ox_log_get_level (OX_Context *ctxt)
{
    OX_VM *vm;

    assert(ctxt);

    vm = ox_vm_get(ctxt);

    return vm->log_level;
}

/**
 * Set the log output fields.
 * @param ctxt The current running context.
 * @param fields The fields can be outputted.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_log_set_fields (OX_Context *ctxt, OX_LogField fields)
{
    OX_VM *vm;

    assert(ctxt);

    vm = ox_vm_get(ctxt);
    vm->log_fields = fields;

    return OX_OK;
}

/**
 * Get the current enabled log fields.
 * @param ctxt The current running context.
 * @return The current enabled log fields.
 */
OX_LogField
ox_log_get_fields (OX_Context *ctxt)
{
    OX_VM *vm;

    assert(ctxt);

    vm = ox_vm_get(ctxt);

    return vm->log_fields;
}

/**
 * Set the log output file.
 * @param ctxt The current running context.
 * @param fp The log file.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_log_set_file (OX_Context *ctxt, FILE *fp)
{
    OX_VM *vm;

    assert(ctxt && fp);

    vm = ox_vm_get(ctxt);
    vm->log_file = fp;

    return OX_OK;
}

/**
 * Output log message.
 * @param ctxt The current running context.
 * @param level The level of the message.
 * @param tag The tag of the message.
 * @param file The name of the file which generate the message.
 * @param func The name of the function which generate the message.
 * @param line The line number where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
void
ox_log (OX_Context *ctxt, OX_LogLevel level, const char *tag,
        const char *file, const char *func, int line,
        const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    ox_log_v(ctxt, level, tag, file, func, line, fmt, ap);

    va_end(ap);
}

/**
 * Output log message with va_list argument.
 * @param ctxt The current running context.
 * @param level The level of the message.
 * @param tag The tag of the message.
 * @param file The name of the file which generate the message.
 * @param func The name of the function which generate the message.
 * @param line The line number where generate the message.
 * @param fmt Output format pattern.
 * @param ap Variable argument list.
 */
void
ox_log_v (OX_Context *ctxt, OX_LogLevel level, const char *tag,
        const char *file, const char *func, int line,
        const char *fmt, va_list ap)
{
    OX_VM *vm;
    char lc;

    assert(ctxt);

    vm = ox_vm_get(ctxt);

    if (vm->log_level > level)
        return;

    if (vm->log_fields & OX_LOG_FIELD_LEVEL) {
        char *col;

        switch (level) {
        case OX_LOG_LEVEL_DEBUG:
            lc = 'D';
            col = "\033[36;1m";
            break;
        case OX_LOG_LEVEL_INFO:
            lc = 'I';
            col = NULL;
            break;
        case OX_LOG_LEVEL_WARNING:
            lc = 'W';
            col = "\033[35;1m";
            break;
        case OX_LOG_LEVEL_ERROR:
            lc = 'E';
            col = "\033[31;1m";
            break;
        case OX_LOG_LEVEL_FATAL:
            lc = 'F';
            col = "\033[33;1m";
            break;
        default:
            assert(0);
        }

#ifdef OX_SUPPORT_COLOR_TTY
        if (!isatty(fileno(vm->log_file)))
#endif /*OX_SUPPORT_COLOR_TTY*/
            col = NULL;

        fprintf(vm->log_file, "%s%c%s|",
                col ? col : "",
                lc,
                col ? "\033[0m" : "");
    }

    if (vm->log_fields & (OX_LOG_FIELD_DATE|OX_LOG_FIELD_TIME)) {
        time_t t;
        struct tm *date;

        time(&t);
        date = localtime(&t);

        if (vm->log_fields & OX_LOG_FIELD_DATE) {
            fprintf(vm->log_file, "%04d-%02d-%02d",
                    date->tm_year + 1900, date->tm_mon + 1, date->tm_mday);

            if (vm->log_fields & (OX_LOG_FIELD_TIME|OX_LOG_FIELD_MSEC))
                fprintf(vm->log_file, " ");
        }

        if (vm->log_fields & OX_LOG_FIELD_TIME) {
            fprintf(vm->log_file, "%02d:%02d:%02d",
                    date->tm_hour, date->tm_min, date->tm_sec);

            if (vm->log_fields & OX_LOG_FIELD_MSEC)
                fprintf(vm->log_file, ".");
        }

        if (!(vm->log_fields & OX_LOG_FIELD_MSEC))
            fprintf(vm->log_file, "|");
    }

    if (vm->log_fields & OX_LOG_FIELD_MSEC) {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        fprintf(vm->log_file, "%03ld|", tv.tv_usec / 1000);
    }

    if (vm->log_fields & OX_LOG_FIELD_THREAD) {
        int tid = ox_thread_id();

        fprintf(vm->log_file, "%d|", (int)tid);
    }

    if (vm->log_fields & OX_LOG_FIELD_TAG) {
        if (tag)
            fprintf(vm->log_file, "%s|", tag);
    }

    if (vm->log_fields & OX_LOG_FIELD_FILE) {
        if (file)
            fprintf(vm->log_file, "\"%s\"|", file);
    }

    if (vm->log_fields & OX_LOG_FIELD_FUNC) {
        if (func)
            fprintf(vm->log_file, "%s|", func);
    }

    if (vm->log_fields & OX_LOG_FIELD_LINE) {
        if (line > 0)
            fprintf(vm->log_file, "%d|", line);
    }

    vfprintf(vm->log_file, fmt, ap);
    fprintf(vm->log_file, "\n");
}

/**
 * Initialize the log data in the running context.
 * @param ctxt The running context.
 */
void
ox_log_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    char *v = getenv("OX_LOG_LEVEL");
    OX_LogLevel level = OX_LOG_LEVEL_NONE;

    if (v) {
        if (!strcasecmp(v, "a"))
            level = OX_LOG_LEVEL_ALL;
        else if (!strcasecmp(v, "d"))
            level = OX_LOG_LEVEL_DEBUG;
        else if (!strcasecmp(v, "i"))
            level = OX_LOG_LEVEL_INFO;
        else if (!strcasecmp(v, "w"))
            level = OX_LOG_LEVEL_WARNING;
        else if (!strcasecmp(v, "e"))
            level = OX_LOG_LEVEL_ERROR;
        else if (!strcasecmp(v, "f"))
            level = OX_LOG_LEVEL_FATAL;
        else if (!strcasecmp(v, "n"))
            level = OX_LOG_LEVEL_NONE;
    }

    vm->log_level = level;
    vm->log_fields = OX_LOG_FIELD_ALL;
    vm->log_file = stderr;
}

/**
 * Release the log data in the running context.
 * @param ctxt The running context.
 */
void
ox_log_deinit (OX_Context *ctxt)
{
}
