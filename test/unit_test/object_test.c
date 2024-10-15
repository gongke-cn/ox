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
 * Object test.
 */

#define OX_LOG_TAG "object_test"

#include "test.h"

static OX_Result
cfunc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Number n = ox_value_get_number(ctxt, v);

    ox_value_set_number(ctxt, rv, -n);
    return OX_OK;
}

static OX_Number rec_n = 1;

static OX_Result
get_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    ox_value_set_number(ctxt, rv, rec_n * 10);
    return OX_OK;
}

static OX_Result
set_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Number n = ox_value_get_number(ctxt, v);

    rec_n = n;
    return OX_OK;
}

#define PROP_NUM 1024

void
object_test (OX_Context *ctxt)
{
    OX_Value *o = ox_value_stack_push(ctxt);
    OX_Value *inf = ox_value_stack_push(ctxt);
    OX_Value *k = ox_value_stack_push(ctxt);
    OX_Value *pv = ox_value_stack_push(ctxt);
    OX_Value *tv = ox_value_stack_push(ctxt);
    OX_Value *iter = ox_value_stack_push(ctxt);
    size_t i;

    TEST(ox_interface_new(ctxt, inf) == OX_OK);
    ox_value_set_number(ctxt, pv, 0);
    ox_object_add_var_s(ctxt, inf, "v1", pv);
    ox_value_set_number(ctxt, pv, 1);
    ox_object_add_const_s(ctxt, inf, "c1", pv);

    TEST(ox_object_new(ctxt, o, inf) == OX_OK);
    TEST(ox_get_s(ctxt, o, "v1", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 0);
    TEST(ox_get_s(ctxt, o, "c1", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 1);

    ox_value_set_number(ctxt, pv, 2);
    TEST(ox_set_s(ctxt, o, "v1", pv) == OX_OK);
    TEST(ox_get_s(ctxt, o, "v1", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 2);

    TEST(ox_set_s(ctxt, o, "c1", pv) == OX_ERR);

    ox_value_set_number(ctxt, pv, 3);
    TEST(ox_set_s(ctxt, o, "v2", pv) == OX_OK);
    TEST(ox_get_s(ctxt, o, "v2", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 3);

    ox_value_set_number(ctxt, pv, 4);
    TEST(ox_set_s(ctxt, o, "v2", pv) == OX_OK);
    TEST(ox_get_s(ctxt, o, "v2", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 4);

    /*Method.*/
    TEST(ox_object_add_n_method_s(ctxt, o, "f1", cfunc) == OX_OK);
    ox_value_set_number(ctxt, pv, 1234);
    TEST(ox_call_method_s(ctxt, o, "f1", pv, 1, tv) == OX_OK);
    TEST(ox_value_get_number(ctxt, tv) == -1234);

    TEST(ox_get_s(ctxt, o, "f1", pv) == OX_OK);
    TEST(ox_get_s(ctxt, pv, "$name", k));
    TEST(!strcmp(ox_string_get_char_star(ctxt, k), "f1"));

    TEST(ox_get_s(ctxt, pv, "$scope", tv) == OX_OK);
    TEST(ox_equal(ctxt, tv, o));

    /*Accessor.*/
    TEST(ox_object_add_n_accessor_s(ctxt, o, "a1", get_func, set_func) == OX_OK);

    TEST(ox_get_s(ctxt, o, "a1", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 10);

    ox_value_set_number(ctxt, pv, 2);
    TEST(ox_set_s(ctxt, o, "a1", pv) == OX_OK);

    TEST(ox_get_s(ctxt, o, "a1", pv) == OX_OK);
    TEST(ox_value_get_number(ctxt, pv) == 20);

    /*Iterator.*/
    ox_object_new(ctxt, o, NULL);
    for (i = 0; i < PROP_NUM; i ++) {
        char name[64];

        snprintf(name, sizeof(name), "p%d", (int)i);
        ox_string_from_char_star(ctxt, k, name);
        ox_value_set_number(ctxt, pv, i);

        ox_set(ctxt, o, k, pv);
    }

    TEST(ox_object_iter_new(ctxt, iter, o, OX_OBJECT_ITER_KEY) == OX_OK);
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        char buf[64];

        snprintf(buf, sizeof(buf), "p%d", (int)i);
        TEST(ox_iterator_value(ctxt, iter, pv) == OX_OK);
        TEST(!strcmp(buf, ox_string_get_char_star(ctxt, pv)));
        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
        i ++;
    }
    TEST(i == PROP_NUM);
    TEST(ox_close(ctxt, iter) == OX_OK);

    TEST(ox_object_iter_new(ctxt, iter, o, OX_OBJECT_ITER_VALUE) == OX_OK);
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        TEST(ox_iterator_value(ctxt, iter, pv) == OX_OK);
        TEST(ox_value_get_number(ctxt, pv) == i);
        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
        i ++;
    }
    TEST(i == PROP_NUM);
    TEST(ox_close(ctxt, iter) == OX_OK);

    TEST(ox_object_iter_new(ctxt, iter, o, OX_OBJECT_ITER_KEY_VALUE) == OX_OK);
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        char buf[64];

        snprintf(buf, sizeof(buf), "p%d", (int)i);

        TEST(ox_iterator_value(ctxt, iter, pv) == OX_OK);
        
        ox_array_get_item(ctxt, pv, 0, tv);
        TEST(!strcmp(buf, ox_string_get_char_star(ctxt, tv)));

        ox_array_get_item(ctxt, pv, 1, tv);
        TEST(ox_value_get_number(ctxt, tv) == i);

        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
        i ++;
    }
    TEST(i == PROP_NUM);
    TEST(ox_close(ctxt, iter) == OX_OK);

    ox_value_stack_pop(ctxt, o);
}
