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
 * Array test.
 */

#define OX_LOG_TAG "array_test"

#include "test.h"

#define ITEM_NUM 1024

void
array_test (OX_Context *ctxt)
{
    OX_Value *a = ox_value_stack_push(ctxt);
    OX_Value *item = ox_value_stack_push(ctxt);
    OX_Value *pk = ox_value_stack_push(ctxt);
    OX_Value *pv = ox_value_stack_push(ctxt);
    OX_Value *iter = ox_value_stack_push(ctxt);
    size_t i;

    ox_array_new(ctxt, a, 0);
    TEST(ox_array_length(ctxt, a) == 0);

    for (i = 0; i < ITEM_NUM; i ++) {
        ox_array_get_item(ctxt, a, i, item);
        TEST(ox_value_is_null(ctxt, item));
    }

    ox_array_set_length(ctxt, a, ITEM_NUM);
    TEST(ox_array_length(ctxt, a) == ITEM_NUM);

    TEST(ox_get_s(ctxt, a, "length", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == ITEM_NUM);

    for (i = 0; i < ITEM_NUM; i ++) {
        ox_array_get_item(ctxt, a, i, item);
        TEST(ox_value_is_null(ctxt, item));

        ox_value_set_number(ctxt, item, i);
        ox_array_set_item(ctxt, a, i, item);
    }

    TEST(ox_array_length(ctxt, a) == ITEM_NUM);

    TEST(ox_iterator_new(ctxt, iter, a) == OX_OK);
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        TEST(ox_iterator_value(ctxt, iter, pv) == OX_OK);
        TEST(ox_value_get_number(ctxt, pv) == i);
        i ++;
        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
    }
    TEST(ox_close(ctxt, iter));

    for (i = 0; i < ITEM_NUM; i ++) {
        ox_array_get_item(ctxt, a, i, item);
        TEST(ox_value_get_number(ctxt, item) == i);

        ox_value_set_number(ctxt, pk, i);
        ox_get(ctxt, a, pk, pv);
        TEST(ox_value_get_number(ctxt, pv) == i);

        ox_value_set_number(ctxt, pv, -i);
        ox_set(ctxt, a, pk, pv);
        ox_get(ctxt, a, pk, pv);
        TEST(ox_value_get_number(ctxt, pv) == -i);
    }

    ox_value_set_number(ctxt, pv, ITEM_NUM/2);
    TEST(ox_set_s(ctxt, a, "length", pv) == OX_OK);
    TEST(ox_array_length(ctxt, a) == ITEM_NUM/2);

    //TEST(ox_array_set_length(ctxt, a, 0xffffffff) == OX_ERR);
    //TEST(ox_array_set_item(ctxt, a, 0xffffffff, item) == OX_ERR);

    ox_value_stack_pop(ctxt, a);
}
