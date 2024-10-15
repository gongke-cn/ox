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
 * Iterator.
 */

#ifndef _OX_ITERATOR_H_
#define _OX_ITERATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get a new iterator of the value.
 * @param ctxt The current running context.
 * @param[out] iter Return the new iterator.
 * @param v The value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_iterator_new (OX_Context *ctxt, OX_Value *iter, OX_Value *v);

/**
 * Move the iterator to the next position.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_iterator_next (OX_Context *ctxt, OX_Value *iter);

/**
 * Check if the iterator is end.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @retval OX_TRUE The iterator is end.
 * @retval OX_FALSE The iterator is not end.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_iterator_end (OX_Context *ctxt, OX_Value *iter);

/**
 * Get the iterator's current value.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @param[out] v Return the current value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_iterator_value (OX_Context *ctxt, OX_Value *iter, OX_Value *v);

#ifdef __cplusplus
}
#endif

#endif
