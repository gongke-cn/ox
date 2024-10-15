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
 * Unit test.
 */

#ifndef _TEST_H_
#define _TEST_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../src/lib/ox_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Push test message tag to the stack.
 * @param fmt Tag format pattern.
 * @param ... Arguments.
 */
extern void
test_push_tag (const char *fmt, ...);

/**
 * Pop up the top tag from the stack.
 */
extern void
test_pop_tag (void);

/**
 * Test the condition.
 * @param cond Condition value. If it is 0, test failed.
 * @param file The file which run the test.
 * @param func
 */
extern void
test (int cond, const char *file, const char *func,
        int line, const char *msg);

/** Test the condition.*/
#define TEST(e) test(e, __FILE__, __FUNCTION__, __LINE__, #e)

#ifdef __cplusplus
}
#endif

#endif
