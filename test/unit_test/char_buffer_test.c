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
 * Character buffer test.
 */

#define OX_LOG_TAG "char_buffer_test"

#include "test.h"

void
char_buffer_test (OX_Context *ctxt)
{
    OX_CharBuffer cb;
    const char *s;
    int i;

    ox_char_buffer_init(&cb);

    for (i = 0; i < 26; i ++) {
        TEST(ox_char_buffer_append_char(ctxt, &cb, 'a' + i) == OX_OK);
    }

    for (i = 0; i < 10; i ++) {
        char buf[2];

        buf[0] = '0' + i;
        buf[1] = 0;

        TEST(ox_char_buffer_append_char_star(ctxt, &cb, buf) == OX_OK);
    }

    for (i = 0; i < 10; i ++) {
        TEST(ox_char_buffer_print(ctxt, &cb, "%d", i) == OX_OK);
    }

    s = ox_char_buffer_get_char_star(ctxt, &cb);
    TEST(!strcmp(s, "abcdefghijklmnopqrstuvwxyz01234567890123456789"));

    ox_char_buffer_deinit(ctxt, &cb);
}
