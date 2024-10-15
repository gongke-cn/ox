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
 * String.
 */

#ifndef _OX_STRING_H_
#define _OX_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is a string.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a string.
 * @retval OX_FALSE The value is not a string.
 */
static inline OX_Bool
ox_value_is_string (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObjectType type = ox_value_get_gco_type(ctxt, v);

    if (type == -1)
        return OX_FALSE;

    return (type & OX_GCO_FL_STRING) ? OX_TRUE : OX_FALSE;
}

/**
 * Get the string's length.
 * @param ctxt The current running context.
 * @param v The string value.
 * @return The length of the string.
 */
static inline size_t
ox_string_length (OX_Context *ctxt, OX_Value *v)
{
    OX_String *s;

    assert(ox_value_is_string(ctxt, v));

    s = ox_value_get_gco(ctxt, v);

    return s->len;
}

/**
 * Get the 0 terminated characters from a string.
 * @param ctxt The current running context.
 * @param v The string value.
 * @return The 0 terminated characters.
 */
static inline const char*
ox_string_get_char_star (OX_Context *ctxt, OX_Value *v)
{
    OX_String *s;

    assert(ox_value_is_string(ctxt, v));

    s = ox_value_get_gco(ctxt, v);

    return s->chars;
}

/**
 * Create a string from a constant 0 terminated characters buffer.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param cstr The 0 terminated characters buffer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_from_const_char_star (OX_Context *ctxt, OX_Value *v, const char *cstr);

/**
 * Create a string from a 0 terminated characters buffer.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param cstr The 0 terminated characters buffer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_from_char_star (OX_Context *ctxt, OX_Value *v, const char *cstr);

/**
 * Create a string from characters.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param chars The characters buffer.
 * @param len Length of the string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_from_chars (OX_Context *ctxt, OX_Value *v, const char *chars, size_t len);

/**
 * Create a string from a file.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param fn The filename.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_from_file (OX_Context *ctxt, OX_Value *v, const char *fn);

/**
 * Check if 2 strings are equal.
 * @param ctxt The current running context.
 * @param sv1 String 1.
 * @param sv2 String 2.
 * @retval OX_TRUE sv1 == sv2.
 * @retval OX_FALSE sv1 != sv2.
 */
extern OX_Bool
ox_string_equal (OX_Context *ctxt, OX_Value *sv1, OX_Value *sv2);

/**
 * Convert the string to singleton.
 * @param ctxt The current running context.
 * @param s The string.
 * @param[out] ss Return the singleton string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_singleton_inner (OX_Context *ctxt, OX_String *s, OX_String **ss);

/**
 * Convert the string to singleton.
 * @param ctxt The current running context.
 * @param v The string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_string_singleton (OX_Context *ctxt, OX_Value *v)
{
    OX_String *s, *ss;
    OX_GcObjectType type;
    OX_Result r;

    assert(ox_value_is_string(ctxt, v));

    s = ox_value_get_gco(ctxt, v);
    type = ox_value_get_gco_type(ctxt, v);

    if (type == OX_GCO_SINGLETON_STRING)
        return OX_OK;

    if ((r = ox_string_singleton_inner(ctxt, s, &ss)) == OX_ERR)
        return r;

    if (ss != s)
        ox_value_set_gco(ctxt, v, ss);
        
    return OX_OK;
}

/**
 * Concatenate 2 strings.
 * @param ctxt The current running context.
 * @param s1 String 1.
 * @param s2 String 2.
 * @param[out] sr Return the result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_concat (OX_Context *ctxt, OX_Value *s1, OX_Value *s2, OX_Value *sr);

/**
 * Compare 2 strings.
 * @param ctxt The current running context.
 * @param s1 String 1.
 * @param s2 String 2.
 * @retval 0 s1 == s2.
 * @retval <0 s1 < s2.
 * @retval >0 s1 > s2.
 */
extern int
ox_string_compare (OX_Context *ctxt, OX_Value *s1, OX_Value *s2);

/**
 * Get the substring.
 * @param ctxt The current running context.
 * @param s The origin string.
 * @param start The start position of the substring.
 * @param len The length of the substring.
 * @param[out] sr Return the result substring.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_substr (OX_Context *ctxt, OX_Value *s, size_t start, size_t len, OX_Value *sr);

/**
 * Trim the prefixed and postfixed spaces in the string.
 * @param ctxt The current running context.
 * @param s The origin string.
 * @param trim Trim mode.
 * @param[out] sr Return the result substring.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_trim (OX_Context *ctxt, OX_Value *s, OX_StringTrim trim, OX_Value *sr);

/**
 * Match the string with a pattern.
 * @param ctxt The current running context.
 * @param s The string.
 * @param m A regular expression or a substring.
 * @param pos Match start position.
 * @param[out] mr Return the match result.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_match (OX_Context *ctxt, OX_Value *s, OX_Value *m, size_t pos, OX_Value *mr);

/**
 * Replace the substrings in the string.
 * @param ctxt The current running context.
 * @param s The string.
 * @param m A regular expression or a substring.
 * @param rep Replace pattern string of replace function.
 * @param pos Replace start position.
 * @param once Only replace once.
 * @param[out] rs Return the result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_replace (OX_Context *ctxt, OX_Value *s, OX_Value *m, OX_Value *rep,
        size_t pos, OX_Bool once, OX_Value *rs);

#ifdef __cplusplus
}
#endif

#endif
