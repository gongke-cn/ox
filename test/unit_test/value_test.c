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
 * Value test.
 */

#define OX_LOG_TAG "value_test"

#include "test.h"

void
value_test (OX_Context *ctxt)
{
#define ARRAY_LEN 64

    OX_Value *v = ox_value_stack_push(ctxt);
    OX_Value *v1 = ox_value_stack_push(ctxt);
    OX_Value *va = ox_value_stack_push_n(ctxt, ARRAY_LEN);
    int i;

    TEST(ox_value_is_null(ctxt, v));
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_NULL);

    ox_value_set_bool(ctxt, v, OX_TRUE);
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_BOOL);
    TEST(ox_value_is_bool(ctxt, v));
    TEST(ox_value_get_bool(ctxt, v));

    ox_value_set_bool(ctxt, v, OX_FALSE);
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_BOOL);
    TEST(ox_value_is_bool(ctxt, v));
    TEST(!ox_value_get_bool(ctxt, v));

    ox_value_set_number(ctxt, v, 0);
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_NUMBER);
    TEST(ox_value_is_number(ctxt, v));
    TEST(ox_value_get_number(ctxt, v) == 0);

    ox_value_set_number(ctxt, v, INFINITY);
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_NUMBER);
    TEST(ox_value_is_number(ctxt, v));
    TEST(isinf(ox_value_get_number(ctxt, v)));

    ox_value_set_number(ctxt, v, NAN);
    TEST(ox_value_get_type(ctxt, v) == OX_VALUE_NUMBER);
    TEST(ox_value_is_number(ctxt, v));
    TEST(isnan(ox_value_get_number(ctxt, v)));

    TEST(ox_value_is_null(ctxt, v1));

    ox_value_set_number(ctxt, v, 1234);
    ox_value_copy(ctxt, v1, v);
    TEST(ox_value_is_number(ctxt, v1));
    TEST(ox_value_get_number(ctxt, v1) == 1234);

    for (i = 0; i < ARRAY_LEN; i ++) {
        OX_Value *vp = ox_values_item(ctxt, va, i);

        TEST(ox_value_is_null(ctxt, vp));
    }

    ox_value_set_number(ctxt, v, 0);
    ox_value_set_number(ctxt, v1, 1);

    ox_values_copy(ctxt, va, v, 2);

    TEST(ox_value_is_number(ctxt, va));
    TEST(ox_value_get_number(ctxt, va) == 0);

    TEST(ox_value_is_number(ctxt, ox_values_item(ctxt, va, 1)));
    TEST(ox_value_get_number(ctxt, ox_values_item(ctxt, va, 1)) == 1);

    ox_value_stack_pop(ctxt, v);
}
