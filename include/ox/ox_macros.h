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
 * Basic macro definitions.
 */

#ifndef _OX_MACROS_H_
#define _OX_MACROS_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Statement like macro begin.*/
#define OX_STMT_BEGIN do {

/** Statement like macro end.*/
#define OX_STMT_END   } while (0)

/** Get the elements number of the array.*/
#define OX_N_ELEM(a)  (sizeof(a) / sizeof((a)[0]))

/**
 * Get the maximum value of 2 numbers.
 * @param a Number a.
 * @param b Number b.
 * @return The maximum number.
 */
#define OX_MAX(a, b)  ((a) >= (b) ? (a) : (b))

/**
 * Get the minimum value of 2 numbers.
 * @param a Number a.
 * @param b Number b.
 * @return The minimum number.
 */
#define OX_MIN(a, b)  ((a) <= (b) ? (a) : (b))

/**
 * Get the maximum value of 3 numbers.
 * @param a Number a.
 * @param b Number b.
 * @param c Number c.
 * @return The maximum number.
 */
#define OX_MAX_3(a, b, c)\
    ((a) >= (b) ? OX_MAX(a, c) : OX_MAX(b, c))

/**
 * Get the minimum value of 3 numbers.
 * @param a Number a.
 * @param b Number b.
 * @param c Number c.
 * @return The minimum number.
 */
#define OX_MIN_3(a, b, c)\
    ((a) <= (b) ? OX_MIN(a, c) : OX_MIN(b, c))

/**
 * Get the absolute value.
 * @param n The number.
 * @return The absolute value of n.
 */
#define OX_ABS(n)     ((n) >= 0 ? (n) : -(n))

/**
 * Concatenate 2 strings.
 * @param a String a.
 * @param b String b.
 * @return The concatenated string.
 */
#define OX_CONCAT(a, b) a ## b

/**
 * Concatenate 2 strings.
 * a or b will be expanded if it is a macro.
 * @param a String a.
 * @param b String b.
 * @return The concatenated string.
 */
#define OX_CONCAT_2(a, b) OX_CONCAT(a, b)

/**
 * Cast the size_t type value to pointer.
 * @param s The size_t type value.
 * @return The pointer type value.
 */
#define OX_SIZE2PTR(s)  ((void*)(size_t)(s))

/**
 * Cast the pointer value to size_t type.
 * @param p The pointer type value.
 * @return The size_t type value.
 */
#define OX_PTR2SIZE(p)  ((size_t)(p))

/**
 * Get the member offset of a structure.
 * @param s The structure type.
 * @param m The member of the structure s.
 * @return The member's offset.
 */
#define OX_OFFSET_OF(s, m)  OX_PTR2SIZE(&((s*)0)->m)

/**
 * Get the container structure's pointer from the member's pointer.
 * @param p The member m's pointer.
 * @param s The container structure type.
 * @param m The member of the structure s.
 * @return The container structure's pointer.
 */
#define OX_CONTAINER_OF(p, s, m)\
    OX_SIZE2PTR(OX_PTR2SIZE(p) - OX_OFFSET_OF(s, m))

#ifndef OX_TEXT_DOMAIN
#define OX_TEXT_DOMAIN "ox"
#endif

#ifdef _LIBINTL_H
/**
 * Get the local language string.
 * @param s The string.
 * @return The local language string.
 */
#define OX_TEXT(s) dgettext(OX_TEXT_DOMAIN, s)
#else
#define OX_TEXT(s) (s)
#endif

#ifdef __cplusplus
}
#endif

#endif
