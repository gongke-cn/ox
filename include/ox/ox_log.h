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

#ifndef _OX_LOG_H_
#define _OX_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the log output level.
 * Only the message which level greater than this value can be outputted.
 * @param ctxt The current running context.
 * @param level The log output level.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_log_set_level (OX_Context *ctxt, OX_LogLevel level);

/**
 * Get the current log output level.
 * @param ctxt The current running context.
 * @return The current log level.
 */
extern OX_LogLevel
ox_log_get_level (OX_Context *ctxt);

/**
 * Set the log output fields.
 * @param ctxt The current running context.
 * @param fields The fields can be outputted.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_log_set_fields (OX_Context *ctxt, OX_LogField fields);

/**
 * Get the current enabled log fields.
 * @param ctxt The current running context.
 * @return The current enabled log fields.
 */
extern OX_LogField
ox_log_get_fields (OX_Context *ctxt);

/**
 * Set the log output file.
 * @param ctxt The current running context.
 * @param fp The log file.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_log_set_file (OX_Context *ctxt, FILE *fp);

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
extern void
ox_log (OX_Context *ctxt, OX_LogLevel level, const char *tag,
        const char *file, const char *func, int line,
        const char *fmt, ...);

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
extern void
ox_log_v (OX_Context *ctxt, OX_LogLevel level, const char *tag,
        const char *file, const char *func, int line,
        const char *fmt, va_list ap);


#ifndef OX_LOG_TAG
    #define OX_LOG_TAG "ox_log"
#endif

#ifndef OX_LOG_LEVEL
    #define OX_LOG_LEVEL OX_LOG_LEVEL_ALL
#endif

#define OX_LOG_PARAMS OX_LOG_TAG, __FILE__, __FUNCTION__, __LINE__

#if OX_LOG_LEVEL <= OX_LOG_LEVEL_DEBUG
/**
 * Output debug log message.
 * @param c The current running context.
 * @param f Output format and arguments.
 */
#define OX_LOG_D(c, f...) ox_log(c, OX_LOG_LEVEL_DEBUG, OX_LOG_PARAMS, f)
#else
#define OX_LOG_D(c, f...) ((void)0)
#endif

#if OX_LOG_LEVEL <= OX_LOG_LEVEL_INFO
/**
 * Output information log message.
 * @param c The current running context.
 * @param f Output format and arguments.
 */
#define OX_LOG_I(c, f...) ox_log(c, OX_LOG_LEVEL_INFO, OX_LOG_PARAMS, f)
#else
#define OX_LOG_I(c, f...) ((void)0)
#endif

#if OX_LOG_LEVEL <= OX_LOG_LEVEL_INFO
/**
 * Output warning log message.
 * @param c The current running context.
 * @param f Output format and arguments.
 */
#define OX_LOG_W(c, f...) ox_log(c, OX_LOG_LEVEL_WARNING, OX_LOG_PARAMS, f)
#else
#define OX_LOG_W(c, f...) ((void)0)
#endif

#if OX_LOG_LEVEL <= OX_LOG_LEVEL_ERROR
/**
 * Output error log message.
 * @param c The current running context.
 * @param f Output format and arguments.
 */
#define OX_LOG_E(c, f...) ox_log(c, OX_LOG_LEVEL_ERROR, OX_LOG_PARAMS, f)
#else
#define OX_LOG_E(c, f...) ((void)0)
#endif

#if OX_LOG_LEVEL <= OX_LOG_LEVEL_FATAL
/**
 * Output fatal error log message.
 * @param c The current running context.
 * @param f Output format and arguments.
 */
#define OX_LOG_F(c, f...) ox_log(c, OX_LOG_LEVEL_FATAL, OX_LOG_PARAMS, f)
#else
#define OX_LOG_F(c, f...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
