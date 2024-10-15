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
 * Memory manager test.
 */

#define OX_LOG_TAG "mem_test"

#include "test.h"

typedef struct {
    int a;
    int b;
} Data;

#define ARRAY_LEN 1024

void
mem_test (OX_Context *ctxt)
{
    Data *p;
    int i;

    /*Allocate.*/
    OX_NEW(ctxt, p);
    TEST(p != NULL);

    p->a = 19491009;
    p->b = 18931226;

    OX_DEL(ctxt, p);

    /*Allocate and fill 0.*/
    OX_NEW_0(ctxt, p);

    TEST(p->a == 0);
    TEST(p->b == 0);

    OX_DEL(ctxt, p);

    /*Array.*/
    OX_NEW_N(ctxt, p, ARRAY_LEN);

    for (i = 0; i < ARRAY_LEN; i ++) {
        p[i].a = 19491009;
        p[i].b = 18931226;
    }

    OX_DEL_N(ctxt, p, ARRAY_LEN);

    /*Allocate array and fill 0.*/
    OX_NEW_N_0(ctxt, p, ARRAY_LEN);

    for (i = 0; i < ARRAY_LEN; i ++) {
        TEST(p[i].a == 0);
        TEST(p[i].b == 0);
    }

    OX_DEL_N(ctxt, p, ARRAY_LEN);

    /*Resize.*/
    OX_NEW_N(ctxt, p, ARRAY_LEN/2);
    for (i = 0; i < ARRAY_LEN/2; i ++) {
        p[i].a = 19491009;
        p[i].b = 18931226;
    }

    p = OX_RENEW(ctxt, p, ARRAY_LEN/2, ARRAY_LEN);

    for (i = 0; i < ARRAY_LEN/2; i ++) {
        TEST(p[i].a == 19491009);
        TEST(p[i].b == 18931226);
    }

    for (; i < ARRAY_LEN; i ++) {
        p[i].a = 19491009;
        p[i].b = 18931226;
    }

    OX_DEL_N(ctxt, p, ARRAY_LEN);

    /*Assert.*/
    //ox_not_null(OX_NEW_N(ctxt, p, 0xffffffff));
}
