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
 * Vector test.
 */

#define OX_LOG_TAG "vector_test"

#include "test.h"

#define ITEM_NUM 1024

void
vector_test (OX_Context *ctxt)
{
    OX_VECTOR_TYPE_DECL(int) vec = OX_VECTOR_INIT;
    int i;

    TEST(ox_vector_length(&vec) == 0);

    for (i = 0; i < ITEM_NUM; i ++) {
        TEST(ox_vector_append(ctxt, &vec, i) == OX_OK);
        TEST(ox_vector_length(&vec) == i + 1);
    }

    for (i = 0; i < ITEM_NUM; i ++) {
        TEST(ox_vector_item(&vec, i) == i);
    }

    ox_vector_deinit(ctxt, &vec);
}
