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
 * Array.
 */

#ifndef _OX_ARRAY_H_
#define _OX_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is an array.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is an array.
 * @retval OX_FALSE The value is not an array.
 */
static inline OX_Bool
ox_value_is_array (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_ARRAY);
}

/**
 * Get the length of the array.
 * @param ctxt The current running context.
 * @param a The array value.
 * @return The length of the array.
 */
static inline size_t
ox_array_length (OX_Context *ctxt, OX_Value *a)
{
    OX_Array *ap;

    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);

    return ox_vector_length(&ap->items);
}

/**
 * Create a new array.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param len The length of the array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_new (OX_Context *ctxt, OX_Value *a, size_t len);

/**
 * Reset the array's length.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param len The new length of the array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_set_length (OX_Context *ctxt, OX_Value *a, size_t len);

/**
 * Set the array item's value.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The item's index.
 * @param iv The item's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_set_item (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv);

/**
 * Set the array items' values.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The first item's index.
 * @param iv The items' new values.
 * @param n Number of values.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_set_items (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv, size_t n);

/**
 * Append an item to the array's tail.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param iv The new item's value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_append (OX_Context *ctxt, OX_Value *a, OX_Value *iv);

/**
 * Get the array item's value.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The item's index.
 * @param[out] iv Return the item's value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_array_get_item (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv);

#ifdef __cplusplus
}
#endif

#endif
