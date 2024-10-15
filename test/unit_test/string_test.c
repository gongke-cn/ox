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
 * String test.
 */

#define OX_LOG_TAG "string_test"

#include "test.h"

void
string_test (OX_Context *ctxt)
{
    OX_Value *s1 = ox_value_stack_push(ctxt);
    OX_Value *s2 = ox_value_stack_push(ctxt);
    OX_Value *k = ox_value_stack_push(ctxt);
    OX_Value *v = ox_value_stack_push(ctxt);
    OX_Value *iter = ox_value_stack_push(ctxt);
    size_t i;

    ox_string_from_const_char_star(ctxt, s1, "0123456789");
    TEST(ox_string_length(ctxt, s1) == 10);
    TEST(!strcmp(ox_string_get_char_star(ctxt, s1), "0123456789"));

    ox_string_from_char_star(ctxt, s2, "0123456789");
    TEST(ox_string_length(ctxt, s2) == 10);
    TEST(!strcmp(ox_string_get_char_star(ctxt, s2), "0123456789"));

    TEST(ox_value_get_gco(ctxt, s1) != ox_value_get_gco(ctxt, s2));

    ox_string_singleton(ctxt, s1);
    ox_string_singleton(ctxt, s2);
    TEST(ox_value_get_gco(ctxt, s1) == ox_value_get_gco(ctxt, s2));

    for (i = 0; i < 10; i ++) {
        const char *c;

        ox_value_set_number(ctxt, k, i);
        TEST(ox_get(ctxt, s1, k, v) == OX_OK);
        TEST(ox_string_length(ctxt, v) == 1);

        c = ox_string_get_char_star(ctxt, v);
        TEST(*c == '0' + i);
    }

    TEST(ox_get_s(ctxt, s1, "length", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 10);

    /*Iterator.*/
    TEST(ox_iterator_new(ctxt, iter, s1));
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        char buf[2];

        buf[0] = i + '0';
        buf[1] = 0;

        TEST(ox_iterator_value(ctxt, iter, v) == OX_OK);
        TEST(!strcmp(ox_string_get_char_star(ctxt, v), buf));
        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
        i ++;
    }
    TEST(i == 10);
    TEST(ox_close(ctxt, iter));

    TEST(ox_call_method_s(ctxt, s1, "chars", NULL, 0, iter));
    i = 0;
    while (!ox_iterator_end(ctxt, iter)) {
        TEST(ox_iterator_value(ctxt, iter, v) == OX_OK);
        TEST(ox_value_get_number(ctxt, v) == '0' + i);
        TEST(ox_iterator_next(ctxt, iter) == OX_OK);
        i ++;
    }
    TEST(i == 10);
    TEST(ox_close(ctxt, iter));

    ox_value_stack_pop(ctxt, s1);
}
