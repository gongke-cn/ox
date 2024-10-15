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
 * Regular expression.
 */

#ifndef _OX_RE_H_
#define _OX_RE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is a regular expression.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a regular expression.
 * @retval OX_FALSE The value is not a regular expression.
 */
static inline OX_Bool
ox_value_is_re (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_RE);
}

/**
 * Create a new regular expression.
 * @param ctxt The current running context.
 * @param[out] re Return the new regular expression.
 * @param src The source string of the regular expression.
 * @param flags The flags of the regular expression.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_re_new (OX_Context *ctxt, OX_Value *re, OX_Value *src, OX_ReFlag flags);

/**
 * Create a new regular expression from the input.
 * @param ctxt The current running context.
 * @param[out] re Return the new regular expression.
 * @param input The source input.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_re_from_input (OX_Context *ctxt, OX_Value *re, OX_Input *input);

/**
 * Set the regular expression's flags.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @param flags The flags of the regular expresssion.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_re_set_flags (OX_Context *ctxt, OX_Value *re, OX_ReFlag flags)
{
    OX_Re *rep;

    assert(ctxt && re);
    assert(ox_value_is_re(ctxt, re));

    rep = ox_value_get_gco(ctxt, re);
    rep->flags = flags;

    return OX_OK;
}

/**
 * Get number of groups in the regular expression.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @return Number of the groups.
 */
static inline int
ox_re_get_n_group (OX_Context *ctxt, OX_Value *re)
{
    OX_Re *rep;

    assert(ctxt && re);
    assert(ox_value_is_re(ctxt, re));

    rep = ox_value_get_gco(ctxt, re);
    return rep->group_num;
}

/**
 * Match the string with the regular expression.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @param s The string.
 * @param start Start position.
 * @param flags The match flags will be added in match operation.
 * @param m The match result.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_re_match (OX_Context *ctxt, OX_Value *re, OX_Value *s, size_t start, int flags, OX_Value *m);

/**
 * Disassemble the regular expression.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @param fp Output file.
 */
extern void
ox_re_disassemble (OX_Context *ctxt, OX_Value *re, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
