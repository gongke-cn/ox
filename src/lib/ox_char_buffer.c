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
 * Character buffer.
 */

#define OX_LOG_TAG "ox_char_buffer"

#include "ox_internal.h"

/**
 * Copy a character n times and append them to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param c The character value to be added.
 * @param n Copy times.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_char_buffer_append_char_n (OX_Context *ctxt, OX_CharBuffer *cb, int c, size_t n)
{
    OX_Result r;

    if ((r = ox_vector_expand_capacity(ctxt, cb, cb->len + n)) == OX_ERR)
        return r;

    memset(cb->items + cb->len, c, n);
    cb->len += n;

    return OX_OK;
}

/**
 * Append a string to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param v The string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_char_buffer_append_string (OX_Context *ctxt, OX_CharBuffer *cb, OX_Value *v)
{
    size_t len = ox_string_length(ctxt, v);
    const char *cstr = ox_string_get_char_star(ctxt, v);

    return ox_char_buffer_append_chars(ctxt, cb, cstr, len);
}

/**
 * Print to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param fmt Print format pattern string.
 * @param ... Arguments.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_char_buffer_print (OX_Context *ctxt, OX_CharBuffer *cb, const char *fmt, ...)
{
    va_list ap;
    OX_Result r;

    va_start(ap, fmt);

    r = ox_char_buffer_print_v(ctxt, cb, fmt, ap);

    va_end(ap);

    return r;
}

/**
 * Print to the character buffer with va_list argument.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param fmt Print format pattern string.
 * @param ap Variable argument list.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_char_buffer_print_v (OX_Context *ctxt, OX_CharBuffer *cb, const char *fmt, va_list ap)
{
    va_list ap_bak;
    char *pc;
    size_t len, left;
    int n;
    OX_Result r;

    va_copy(ap_bak, ap);

    len = ox_vector_length(cb);
    left = ox_vector_space(cb);

    pc = &ox_vector_item(cb, len);
    n = vsnprintf(pc, left, fmt, ap);

    if (n >= left) {
        if ((r = ox_vector_expand_capacity(ctxt, cb, len + n + 1)) == OX_ERR)
            goto end;

        pc = &ox_vector_item(cb, len);
        n = vsnprintf(pc, n + 1, fmt, ap_bak);
    }

    cb->len += n;
    r = OX_OK;
end:
    va_end(ap_bak);

    return r;
}

/**
 * Get the 0 termiated characters string from the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @return The 0 terminated characters string.
 * @retval NULL On error.
 */
const char*
ox_char_buffer_get_char_star (OX_Context *ctxt, OX_CharBuffer *cb)
{
    size_t len = ox_vector_length(cb);
    char *pc;
    OX_Result r;

    if ((r = ox_vector_expand_capacity(ctxt, cb, len + 1)) == OX_ERR)
        return NULL;

    pc = cb->items;
    pc[len] = 0;

    return pc;
}

/**
 * Get a string from the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param[out] s Return the string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_char_buffer_get_string (OX_Context *ctxt, OX_CharBuffer *cb, OX_Value *s)
{
    assert(ctxt && cb && s);

    return ox_string_from_chars(ctxt, s, cb->items, cb->len);
}
