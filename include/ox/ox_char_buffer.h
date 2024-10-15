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

#ifndef _OX_CHAR_BUFFER_H_
#define _OX_CHAR_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a character buffer.
 * @param cb The character buffer.
 */
static inline void
ox_char_buffer_init (OX_CharBuffer *cb)
{
    ox_vector_init(cb);
}

/**
 * Release a character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer to be released.
 */
static inline void
ox_char_buffer_deinit (OX_Context *ctxt, OX_CharBuffer *cb)
{
    ox_vector_deinit(ctxt, cb);
}

/**
 * Append a character to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param c The character to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_char_buffer_append_char (OX_Context *ctxt, OX_CharBuffer *cb, int c)
{
    return ox_vector_append(ctxt, cb, c);
}

/**
 * Copy a character n times and append them to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param c The character value to be added.
 * @param n Copy times.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_char_buffer_append_char_n (OX_Context *ctxt, OX_CharBuffer *cb, int c, size_t n);

/**
 * Append characters to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param c The characters to be added.
 * @param n Number of characters to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_char_buffer_append_chars (OX_Context *ctxt, OX_CharBuffer *cb, const char *c, size_t n)
{
    OX_Result r;
    size_t len = ox_vector_length(cb);

    if ((r = ox_vector_expand_capacity(ctxt, cb, len + n)) == OX_ERR)
        return r;

    memcpy(&ox_vector_item(cb, len), c, n);

    cb->len += n;

    return OX_OK;
}

/**
 * Append a 0 terminated characters string to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param s The 0 terminaed characters to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_char_buffer_append_char_star (OX_Context *ctxt, OX_CharBuffer *cb, const char *s)
{
    size_t len = strlen(s);

    return ox_char_buffer_append_chars(ctxt, cb, s, len);
}

/**
 * Append a string to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param v The string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_char_buffer_append_string (OX_Context *ctxt, OX_CharBuffer *cb, OX_Value *v);

/**
 * Print to the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param fmt Print format pattern string.
 * @param ... Arguments.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_char_buffer_print (OX_Context *ctxt, OX_CharBuffer *cb, const char *fmt, ...);

/**
 * Print to the character buffer with va_list argument.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param fmt Print format pattern string.
 * @param ap Variable argument list.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_char_buffer_print_v (OX_Context *ctxt, OX_CharBuffer *cb, const char *fmt, va_list ap);

/**
 * Get the 0 termiated characters string from the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @return The 0 terminated characters string.
 * @retval NULL On error.
 */
extern const char*
ox_char_buffer_get_char_star (OX_Context *ctxt, OX_CharBuffer *cb);

/**
 * Get a string from the character buffer.
 * @param ctxt The current running context.
 * @param cb The character buffer.
 * @param[out] s Return the string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_char_buffer_get_string (OX_Context *ctxt, OX_CharBuffer *cb, OX_Value *s);

#ifdef __cplusplus
}
#endif

#endif
