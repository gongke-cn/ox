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
 * Class test.
 */

#define OX_LOG_TAG "class_test"

#include "test.h"

void
class_test (OX_Context *ctxt)
{
    OX_Value *c = ox_value_stack_push(ctxt);
    OX_Value *inf = ox_value_stack_push(ctxt);
    OX_Value *pc = ox_value_stack_push(ctxt);
    OX_Value *pinf = ox_value_stack_push(ctxt);
    OX_Value *o = ox_value_stack_push(ctxt);
    OX_Value *v = ox_value_stack_push(ctxt);

    TEST(ox_class_new(ctxt, pc, pinf) == OX_OK);
    ox_value_set_number(ctxt, v, 1);
    ox_set_s(ctxt, pc, "s_p1", v);
    ox_value_set_number(ctxt, v, 2);
    ox_set_s(ctxt, pinf, "d_p1", v);

    TEST(ox_get_s(ctxt, pc, "s_p1", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 1);

    TEST(ox_get_s(ctxt, pinf, "d_p1", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 2);

    TEST(ox_object_new(ctxt, o, pinf) == OX_OK);
    TEST(ox_get_s(ctxt, o, "d_p1", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 2);

    TEST(ox_class_new(ctxt, c, inf) == OX_OK);
    TEST(ox_class_inherit(ctxt, c, pc) == OX_OK);
    ox_value_set_number(ctxt, v, 3);
    ox_set_s(ctxt, inf, "d_p2", v);

    TEST(ox_object_new(ctxt, o, inf) == OX_OK);
    TEST(ox_get_s(ctxt, o, "d_p1", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 2);
    TEST(ox_get_s(ctxt, o, "d_p2", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 3);

    TEST(ox_object_from_class(ctxt, o, c) == OX_OK);
    TEST(ox_get_s(ctxt, o, "d_p1", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 2);
    TEST(ox_get_s(ctxt, o, "d_p2", v) == OX_OK);
    TEST(ox_value_get_number(ctxt, v) == 3);

    ox_value_stack_pop(ctxt, c);
}
