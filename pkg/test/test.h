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
 * oxngen test.
 */

#ifndef _OXNGEN_TEST_H_
#define _OXNGEN_TEST_H_

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int a;
} InnerStruct;

typedef struct {
    char c_field;
    int  i_field;
    unsigned int  b_field_1: 1;
    unsigned int  b_field_2: 2;
    char cb_field[16];
    InnerStruct s_field;
    struct {
        int b;
    } s_field2;
} Struct;

static int increase (int *p)
{
    int r = *p;

    *p += 1;

    return r;
}

static int variable = 1982;

static int function_v (int n, ...)
{
    va_list ap;
    int i;
    int sum = 0;

    va_start(ap, n);

    for (i = 0; i < n; i ++) {
        int v = va_arg(ap, int);

        sum += v;
    }

    va_end(ap);

    return sum;
}

typedef void (*callback) (int v);

static callback cb_ptr = NULL;

static void register_cb(callback cb)
{
    cb_ptr = cb;
}

static void call_cb ()
{
    if (cb_ptr) {
        cb_ptr(123456789);
    }
}

#ifdef __cplusplus
}
#endif

#endif
