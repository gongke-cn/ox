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
 * String template.
 */

#define OX_LOG_TAG "ox_string_templ"

#include "ox_internal.h"

/**
 * Convert the string template to string.
 * @param ctxt The current running context.
 * @param templ The string template.
 * @param[out] str The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_str_templ_to_str (OX_Context *ctxt, OX_Value *templ, OX_Value *str)
{
    OX_VS_PUSH(ctxt, s)
    size_t len = ox_array_length(ctxt, templ);
    size_t i;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    for (i = 0; i < len; i ++) {
        const char *c;
        size_t sl;

        if ((r = ox_array_get_item(ctxt, templ, i, s)) == OX_ERR)
            goto end;

        if (!ox_value_is_null(ctxt, s)) {
            sl = ox_string_length(ctxt, s);
            c = ox_string_get_char_star(ctxt, s);

            while (sl) {
                if (*c == '~') {
                    r = ox_char_buffer_append_chars(ctxt, &cb, "~0", 2);
                } else if (*c == '$') {
                    r = ox_char_buffer_append_chars(ctxt, &cb, "~1", 2);
                } else {
                    r = ox_char_buffer_append_char(ctxt, &cb, *c);
                }
                if (r == OX_ERR)
                    goto end;

                c ++;
                sl --;
            }
        }

        if (i != len - 1) {
            r = ox_char_buffer_append_char(ctxt, &cb, '$');
            if (r == OX_ERR)
                goto end;
        }
    }

    r = ox_char_buffer_get_string(ctxt, &cb, str);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, s)
    return r;
}

/**
 * Create the string template from a string.
 * @param ctxt The current running context.
 * @param[out] templ The string template.
 * @param str The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_str_templ_from_str (OX_Context *ctxt, OX_Value *templ, OX_Value *str)
{
    OX_VS_PUSH(ctxt, s)
    size_t len;
    const char *b, *c, *e;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_array_new(ctxt, templ, 0)) == OX_ERR)
        goto end;

    b = ox_string_get_char_star(ctxt, str);
    len = ox_string_length(ctxt, str);
    e = b + len;
    c = b;

    while (c < e) {
        if (*c == '~') {
            c ++;
            if (*c == '0') {
                r = ox_char_buffer_append_char(ctxt, &cb, '~');
            } else if (*c == '1') {
                r = ox_char_buffer_append_char(ctxt, &cb, '$');
            } else {
                r = ox_throw_syntax_error(ctxt, OX_TEXT("unexpect character after `~\'"));
            }
        } else if (*c == '$') {
            if ((r = ox_char_buffer_get_string(ctxt, &cb, s)) == OX_ERR)
                goto end;
            if ((r = ox_array_append(ctxt, templ, s)) == OX_ERR)
                goto end;

            cb.len = 0;
            b = c + 1;
        } else {
            r = ox_char_buffer_append_char(ctxt, &cb, *c);
        }

        if (r == OX_ERR)
            goto end;
        c ++;
    }

    if (b < e) {
        if ((r = ox_string_from_chars(ctxt, s, b, e - b)) == OX_ERR)
            goto end;
        if ((r = ox_array_append(ctxt, templ, s)) == OX_ERR)
            goto end;
    } else {
        ox_value_set_null(ctxt, s);
        if ((r = ox_array_append(ctxt, templ, s)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, s)
    return r;
}
