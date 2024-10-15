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
 * Character functions.
 */

#ifndef _OX_CHAR_H_
#define _OX_CHAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the character is a valid ASCII character.
 * @param c Character.
 * @return c is ASCII character or not.
 */
static inline OX_Bool
ox_char_is_ascii (int c)
{
    return (c >= 0) && (c <= 0x7f);
}

/**
 * Check if the character is a printable character.
 * @param c Character.
 * @return c is printable character or not.
 */
static inline OX_Bool
ox_char_is_print (int c)
{
    return (c >= 0x20) && (c < 0x7f);
}

/**
 * Check if the character is a space character.
 * @param c Character.
 * @return c is a space character or not.
 */
static inline OX_Bool
ox_char_is_space (int c)
{
    switch (c) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
        return OX_TRUE;
    default:
        return OX_FALSE;
    }
}

/**
 * Check if the character is a graphic character.
 * @param c Character.
 * @return c is a graphic character or not.
 */
static inline OX_Bool
ox_char_is_graph (int c)
{
    return ox_char_is_print(c) && !ox_char_is_space(c);
}

/**
 * Check if the character is a digital character.
 * @param c Character.
 * @return c is a digital character or not.
 */
static inline OX_Bool
ox_char_is_digit (int c)
{
    return (c >= '0') && (c <= '9');
}

/**
 * Check if the character is a hexadecimal digital character.
 * @param c Character.
 * @return c is a hexadecimal digital character or not.
 */
static inline OX_Bool
ox_char_is_hex (int c)
{
    if (ox_char_is_digit(c))
        return OX_TRUE;

    if ((c >= 'a') && (c <= 'f'))
        return OX_TRUE;

    return (c >= 'A') && (c <= 'F');
}

/**
 * Check if the character is an alphabetic character.
 * @param c Character.
 * @return c is an alphabetic character or not.
 */
static inline OX_Bool
ox_char_is_alpha (int c)
{
    if ((c >= 'a') && (c <= 'z'))
        return OX_TRUE;

    return (c >= 'A') && (c <= 'Z');
}

/**
 * Check if the character is an alphabetic character or a digital character.
 * @param c Character.
 * @retval OX_TRUE c is an alphabetic character or a digital character.
 * @retval OX_FALSE c is not an alphabetic character nor a digital character.
 */
static inline OX_Bool
ox_char_is_alnum (int c)
{
    return ox_char_is_alpha(c) || ox_char_is_digit(c);
}

/**
 * Check if the character is a punctuation character.
 * @param c Character.
 * @return c is a punctuation character or not.
 */
static inline OX_Bool
ox_char_is_punct (int c)
{
    return ox_char_is_print(c) && !ox_char_is_space(c) && !ox_char_is_alnum(c);
}

/**
 * Check if the character is in uppercase.
 * @param c The character.
 * @return c in in uppercase or not.
 */
static inline OX_Bool
ox_char_is_upper (int c)
{
    return (c >= 'A') && (c <= 'Z');
}

/**
 * Check if the character is in lowercase.
 * @param c The character.
 * @return c in in lowercase or not.
 */
static inline OX_Bool
ox_char_is_lower (int c)
{
    return (c >= 'a') && (c <= 'z');
}

/**
 * Convert the lowercase character to uppercase.
 * @param c The character.
 * @return Uppercase character.
 */
static inline int
ox_char_to_upper (int c)
{
    if (ox_char_is_lower(c))
        return c - 'a' + 'A';

    return c;
}

/**
 * Convert the uppercase character to lowercase.
 * @param c The character.
 * @return Lowercase character.
 */
static inline int
ox_char_to_lower (int c)
{
    if (ox_char_is_upper(c))
        return c - 'A' + 'a';

    return c;
}

/**
 * Get the number value of a hexadecimal character.
 * @param c The hexadecimal character.
 * @return The number value.
 */
static inline int
ox_hex_char_to_num (int c)
{
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;

    assert(ox_char_is_digit(c));
    return c - '0';
}

/**
 * Get the hexadecimal character from the number value.
 * @param n The number value.
 * @return The hexadecimal character.
 */
static inline int
ox_hex_char_from_num (int n)
{
    assert((n >= 0) && (n <= 15));

    if (n >= 10)
        return 'a' + n - 10;

    return n + '0';
}

/**
 * Check if the character is a UTF-16 surrogate leading character.
 * @param c Character.
 * @return c is UTF-16 surrogate leading or not.
 */
static inline OX_Bool
ox_char_is_utf16_leading (int c)
{
    return (c >= 0xd800) && (c <= 0xdbff);
}

/**
 * Check if the character is a UTF-16 surrogate trailing character.
 * @param c Character.
 * @return c is UTF-16 surrogate trailing or not.
 */
static inline OX_Bool
ox_char_is_utf16_trailing (int c)
{
    return (c >= 0xdc00) && (c <= 0xdfff);
}

/**
 * Get the unicode from UTF-16 surrogate.
 * @param l Leading character.
 * @param t Trailing character.
 * @return The unicode.
 */
static inline int
ox_uc_from_utf16_surrogate (int l, int t)
{
    return (((l - 0xd800) << 10) | (t - 0xdc00)) + 0x10000;
}

/**
 * Get an unicode from the UTF-8 characters.
 * @param c The characters buffer.
 * @param[in,out] left Left characters in the buffer.
 * @return The unicode.
 * @retval -1 Illegal character.
 */
static inline int
ox_uc_from_utf8 (const char *c, size_t *left)
{
    size_t len = *left;
    int r = -1;

    if (*(uint8_t*)c <= 0x7f) {
        r = *c;
        len --;
    } else if ((*c & 0xe0) == 0xc0) {
        if ((len >= 2) && ((c[1] & 0xc0) == 0x80)) {
            r = ((c[0] & 0x1f) << 6) | (c[1] & 0x3f);
            len -= 2;
        }
    } else if ((*c & 0xf0) == 0xe0) {
        if ((len >= 3)
                && ((c[1] & 0xc0) == 0x80)
                && ((c[2] & 0xc0) == 0x80)) {
            r = ((c[0] & 0xf) << 12)
                    | ((c[1] & 0x3f) << 6)
                    | (c[2] & 0x3f);
            len -= 3;
        }
    } else if ((*c & 0xf8) == 0xf0) {
        if ((len >= 4)
                && ((c[1] & 0xc0) == 0x80)
                && ((c[2] & 0xc0) == 0x80)
                && ((c[3] & 0xc0) == 0x80)) {
            r = ((c[0] & 0x7) << 18)
                    | ((c[1] & 0x3f) << 12)
                    | ((c[2] & 0x3f) << 6)
                    | (c[3] & 0x3f);
            len -= 4;
        }
    }

    *left = len;
    return r;
}

/**
 * Convert the unicode to UTF-8.
 * @param uc The unicode.
 * @param c Character output buffer.
 * @return The characters' number written.
 * @retval -1 Illegal unicode character.
 */
static inline int
ox_uc_to_utf8 (uint32_t uc, char *c)
{
    int r;

    if (uc <= 0x7f) {
        c[0] = uc;
        r = 1;
    } else if (uc <= 0x7ff) {
        c[0] = (uc >> 6) | 0xc0;
        c[1] = (uc & 0x3f) | 0x80;
        r = 2;
    } else if (uc <= 0xffff) {
        c[0] = (uc >> 12) | 0xe0;
        c[1] = ((uc >> 6) & 0x3f) | 0x80;
        c[2] = (uc & 0x3f) | 0x80;
        r = 3;
    } else if (uc <= 0x10ffff) {
        c[0] = (uc >> 18) | 0xf0;
        c[1] = ((uc >> 12) & 0x3f) | 0x80;
        c[2] = ((uc >> 6) & 0x3f) | 0x80;
        c[3] = (uc & 0x3f) | 0x80;
        r = 4;
    } else {
        r = -1;
    }

    return r;
}

/**
 * Get the UTF8 length of the unicode character.
 * @param c The unicode character.
 * @return Length of the UTF8 characters. 
 */
static inline int
ox_uc_utf8_length (int c)
{
    int n;

    if (c <= 0x7f)
        n = 1;
    else if (c <= 0x7ff)
        n = 2;
    else if (c <= 0xffff)
        n = 3;
    else
        n = 4;

    return n;
}

#ifdef __cplusplus
}
#endif

#endif
