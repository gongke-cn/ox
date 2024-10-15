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
 * Vector function.
 */

#ifndef _OX_VECTOR_H_
#define _OX_VECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializer of the vector.
 */
#define OX_VECTOR_INIT {NULL, 0, 0}

/**
 * Initialize a vector.
 * @param v The vector to be initialized.
 */
#define ox_vector_init(v)\
    OX_STMT_BEGIN\
        (v)->items = NULL;\
        (v)->len = 0;\
        (v)->cap = 0;\
    OX_STMT_END

/**
 * Release a vector.
 * @param c The current running context.
 * @param v The vector to be released.
 */
#define ox_vector_deinit(c, v)\
    OX_STMT_BEGIN\
        if ((v)->items)\
            OX_DEL_N(c, (v)->items, (v)->cap);\
    OX_STMT_END

/**
 * Get the length of the vector.
 * @param v The vector.
 * @return The length of the vector.
 */
#define ox_vector_length(v) ((v)->len)

/**
 * Get the capacity of the vector.
 * @param v The vector.
 * @return The capacity of the vector.
 */
#define ox_vector_capacity(v) ((v)->cap)

/**
 * Get the space left in the vector.
 * @param v The vector.
 * @return The space of the vector.
 */
#define ox_vector_space(v) ((v)->cap - (v)->len)

/**
 * Get the item of the vector.
 * @param v The vector.
 * @param i The item's index.
 * @return The item of the vector.
 */
#define ox_vector_item(v, i) ((v)->items[i])

/** Generic vector.*/
typedef OX_VECTOR_TYPE_DECL(uint8_t) OX_GenVector;

/**
 * Set the generic vector's capacity.
 * @param ctxt The current running context.
 * @param gv The generic vector.
 * @param cap The new capacity if the old capcity less than it.
 * @param es The element size of the vector.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_gen_vector_set_capacity (OX_Context *ctxt, OX_GenVector *gv, size_t cap, size_t es)
{
    if (gv->cap < cap) {
        uint8_t *nitems = ox_realloc(ctxt, gv->items, gv->cap * es, cap *es);

        if (!nitems)
            return OX_ERR;

        gv->items = nitems;
        gv->cap = cap;
    }

    return OX_OK;
}

/**
 * Set the vector's capacity.
 * @param c The current running context.
 * @param v The vector.
 * @param cap The new capacity if the old capcity less than it.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_vector_set_capacity(c, v, cap)\
    ox_gen_vector_set_capacity(c, (OX_GenVector*)(v), cap, sizeof((v)->items[0]))

/**
 * Expand the generic vector's capacity.
 * @param ctxt The current running context.
 * @param gv The generic vector.
 * @param cap The new capacity must greater or equal to it.
 * @param es The element size of the vector.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_gen_vector_expand_capacity (OX_Context *ctxt, OX_GenVector *gv, size_t cap, size_t es)
{
    if (gv->cap < cap) {
        uint8_t *nitems;

        cap = OX_MAX(gv->cap * 2, cap);

        nitems = ox_realloc(ctxt, gv->items, gv->cap * es, cap *es);

        if (!nitems)
            return OX_ERR;

        gv->items = nitems;
        gv->cap = cap;
    }

    return OX_OK;
}

/**
 * Expand the vector's capacity.
 * @param c The current running context.
 * @param v The vector.
 * @param cap The new capacity must greater or equal to it.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_vector_expand_capacity(c, v, cap)\
    ox_gen_vector_expand_capacity(c, (OX_GenVector*)(v), cap, sizeof((v)->items[0]))

/**
 * Expand the generic vector's size.
 * @param ctxt The current running context.
 * @param gv The generic vector.
 * @param size The new size of the vector if the old size less than it.
 * @param es The element size of the vector.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_gen_vector_expand (OX_Context *ctxt, OX_GenVector *gv, size_t size, size_t es)
{
    if (gv->len < size) {
        OX_Result r;

        if ((r = ox_gen_vector_expand_capacity(ctxt, gv, size, es)) == OX_ERR)
            return r;

        gv->len = size;
    }

    return OX_OK;
}

/**
 * Expand the vector's size.
 * @param c The current running context.
 * @param v The vector.
 * @param size The new size of the vector if the old size less than it.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_vector_expand(c, v, s)\
    ox_gen_vector_expand(c, (OX_GenVector*)(v), s, sizeof((v)->items[0]))

/**
 * Append an item to the vector's tail.
 * @param c The current running context.
 * @param v The vector.
 * @param i The item to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_vector_append(c, v, i)\
    (ox_vector_expand_capacity(c, v, (v)->len + 1) == OX_OK\
            ? ((v)->items[(v)->len ++] = (i), OX_OK)\
            : OX_ERR)

#ifdef __cplusplus
}
#endif

#endif
