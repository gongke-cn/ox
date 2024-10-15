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
 * Memory allocate and free functions.
 */

#ifndef _OX_MEM_H_
#define _OX_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Resize a member buffer.
 * @param ctxt The current running context.
 * @param optr The old buffer's pointer.
 * @param osize The old buffer's size.
 * @param nsize The new buffer's size.
 * @return The new buffer's pointer.
 * @retval NULL Cannot allocate the new buffer if nsize is not 0.
 */
extern void*
ox_realloc (OX_Context *ctxt, void *optr, size_t osize, size_t nsize);

/**
 * Allocate a new buffer.
 * @param ctxt The current running context.
 * @param size The size of the new buffer.
 * @return The new buffer's pointer.
 * @retval NULL Cannot allocate the new buffer.
 */
static inline void*
ox_alloc (OX_Context *ctxt, size_t size)
{
    return ox_realloc(ctxt, NULL, 0, size);
}

/**
 * Allocate a new buffer and fill it with 0.
 * @param ctxt The current running context.
 * @param size The size of the new buffer.
 * @return The new buffer's pointer.
 * @retval NULL Cannot allocate the new buffer.
 */
static inline void*
ox_alloc_0 (OX_Context *ctxt, size_t size)
{
    void *nptr = ox_alloc(ctxt, size);

    if (nptr)
        memset(nptr, 0, size);

    return nptr;
}

/**
 * Free an unused buffer.
 * @param ctxt The current running context.
 * @param ptr The buffer's pointer.
 * @param size The size of the buffer.
 */
static inline void
ox_free (OX_Context *ctxt, void *ptr, size_t size)
{
    ox_realloc(ctxt, ptr, size, 0);
}

/**
 * Allocate and duplicate a 0 terminated string.
 * @param ctxt The current running context.
 * @param s Origin string.
 * @return The new string.
 * @retval NULL Cannot allocate the string.
 */
static inline void*
ox_strdup (OX_Context *ctxt, const char *s)
{
    size_t len;
    char *ns;

    if (!s)
        return NULL;

    len = strlen(s);

    if (!(ns = ox_alloc(ctxt, len + 1)))
        return NULL;

    memcpy(ns, s, len);
    ns[len] = 0;
    return ns;
}

/**
 * Allocate and duplicate a string.
 * @param ctxt The current running context.
 * @param s Origin string.
 * @param n Length of the string.
 * @return The new string.
 * @retval NULL Cannot allocate the string.
 */
static inline void*
ox_strndup (OX_Context *ctxt, const char *s, size_t n)
{
    char *ns;

    if (!s)
        return NULL;

    if (!(ns = ox_alloc(ctxt, n + 1)))
        return NULL;

    memcpy(ns, s, n);
    ns[n] = 0;
    return ns;
}

/**
 * Free a 0 terminated string.
 * @param ctxt The current running context.
 * @param s The string to be freed.
 */
static inline void
ox_strfree (OX_Context *ctxt, char *s)
{
    if (s) {
        size_t len = strlen(s);

        ox_free(ctxt, s, len + 1);
    }
}

/**
 * Allocate a new buffer for a data type.
 * @param c The current running context.
 * @param p The pointer of the data type to store the result buffer.
 */
#define OX_NEW(c, p)\
    ((p) = ox_alloc(c, sizeof(*(p))))

/**
 * Allocate a new buffer for a data type and fill it with 0.
 * @param c The current running context.
 * @param p The pointer of the data type to store the result buffer.
 */
#define OX_NEW_0(c, p)\
    ((p) = ox_alloc_0(c, sizeof(*(p))))

/**
 * Allocate a new buffer for a data array.
 * @param c The current running context.
 * @param p The pointer of the data type to store the result array.
 * @param n Number of the elements in the array.
 */
#define OX_NEW_N(c, p, n)\
    ((p) = ox_alloc(c, sizeof(*(p)) * (n)))

/**
 * Allocate a new buffer for a data array and fill it with 0.
 * @param c The current running context.
 * @param p The pointer of the data type to store the result array.
 * @param n Number of the elements in the array.
 */
#define OX_NEW_N_0(c, p, n)\
    ((p) = ox_alloc_0(c, sizeof(*(p)) * (n)))

/**
 * Free a data type pointer.
 * @param c The current running context.
 * @param p The data type pointer to be freed.
 */
#define OX_DEL(c, p)\
    ox_free(c, p, sizeof(*(p)))

/**
 * Free a data type array.
 * @param c The current running context.
 * @param p The data type array to be freed.
 * @param n Number of elements in the array.
 */
#define OX_DEL_N(c, p, n)\
    ox_free(c, p, sizeof(*(p)) * (n))

/**
 * Resize a data type array.
 * @param c The current running context.
 * @param p The data type array.
 * @param o Old elements number in the array.
 * @param n New elements number in the array.
 * @retval The new array pointer.
 */
#define OX_RENEW(c, p, o, n)\
    ox_realloc(c, p, sizeof(*(p)) * (o), sizeof(*(p)) * (n))

#ifdef __cplusplus
}
#endif

#endif
