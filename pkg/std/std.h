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
 * Throw system error.
 */

#ifndef _ERROR_H_
#define _ERROR_H_

#include <errno.h>
#include <string.h>

#define OX_TEXT_DOMAIN "std"
#include <ox_internal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Throw system error.
 * @param ctxt The current running context.
 * @param fn The invoked function name.
 * @param en The error number.
 * @return OX_ERR.
 */
static inline OX_Result
std_system_error_no (OX_Context *ctxt, const char *fn, int en)
{
    return ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
            fn, strerror(en));
}

/**
 * Throw system error.
 * @param ctxt The current running context.
 * @param fn The invoked function name.
 * @return OX_ERR.
 */
static inline OX_Result
std_system_error (OX_Context *ctxt, const char *fn)
{
    return std_system_error_no(ctxt, fn, errno);
}

/**
 * Throw system error with variable arguments.
 * @param ctxt The current running context.
 * @param fn The invoked function name.
 * @param en The error number.
 * @param ... Arguments.
 * @return OX_ERR.
 */
static inline OX_Result
std_system_error_v (OX_Context *ctxt, const char *fn, int en, const char *fmt, ...)
{
    va_list ap;
    OX_CharBuffer cb;
    const char *cstr;
    OX_Result r;

    ox_char_buffer_init(&cb);

    va_start(ap, fmt);
    r = ox_char_buffer_print_v(ctxt, &cb, fmt, ap);
    va_end(ap);

    if (r == OX_OK) {
        cstr = ox_char_buffer_get_char_star(ctxt, &cb);
        if (cstr)
            ox_throw_system_error(ctxt, OX_TEXT("\"%s\" (%s) failed: %s"),
                    fn, cstr, strerror(en));
    }

    ox_char_buffer_deinit(ctxt, &cb);
    return OX_ERR;
}

#ifdef __cplusplus
}
#endif

#endif
