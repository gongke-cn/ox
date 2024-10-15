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
 * Value.
 */

#define OX_LOG_TAG "ox_value"

#include "ox_internal.h"

/**
 * Add a value variable to the current value stack.
 * @param ctxt The current running context.
 * @return The new value in the stack.
 */
OX_Value*
ox_value_stack_push (OX_Context *ctxt)
{
    OX_ValueBuffer *vb = ox_get_value_stack(ctxt);
    size_t idx = ox_vector_length(vb);
    OX_Value *v;

    ox_not_error(ox_vector_expand(ctxt, vb, idx + 1));

    v = &ox_vector_item(vb, idx);

    ox_value_set_null(ctxt, v);

    return OX_VALUE_IDX2PTR(idx);
}

/**
 * Add value variables to the current value stack.
 * @param ctxt The current running context.
 * @param n Number of values to be added.
 * @return The first value in the stack.
 */
OX_Value*
ox_value_stack_push_n (OX_Context *ctxt, size_t n)
{
    OX_ValueBuffer *vb = ox_get_value_stack(ctxt);
    size_t idx = ox_vector_length(vb);
    OX_Value *v;

    ox_not_error(ox_vector_expand(ctxt, vb, idx + n));

    v = &ox_vector_item(vb, idx);

    ox_values_set_null(ctxt, v, n);

    return OX_VALUE_IDX2PTR(idx);
}
