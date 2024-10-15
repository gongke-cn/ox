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
 * Input test.
 */

#define OX_LOG_TAG "input_test"

#include "test.h"

#define LINE_NUM 100
#define FILENAME "input.txt"

static void
do_input_test (OX_Context *ctxt, OX_Value *v)
{
    OX_Input *input = ox_value_get_gco(ctxt, v);
    int i = 0, j = 0;
    int c;
    int unget = OX_FALSE;

    while (1) {
        c = ox_input_get_char(ctxt, input);
        if (c == OX_INPUT_END)
            break;
        if (c == '\n') {
            TEST(i == j);
            i ++;
            j = 0;
            unget = OX_FALSE;
        } else {
            TEST(c == j % 26 + 'a');
            j ++;
        }

        if ((j == 8) && !unget) {
            unget = OX_TRUE;

            while (j > 0) {
                ox_input_unget_char(ctxt, input, (j - 1) % 26 + 'a');
                j --;
            }

            while (j < 8) {
                c = ox_input_get_char(ctxt, input);
                TEST(c == j % 26 + 'a');
                j ++;
            }
        }

        if ((j == 0) || (j == 1)) {
            OX_Location loc;

            ox_input_get_loc(input, &loc.first_line, &loc.first_column);
            ox_input_get_loc(input, &loc.last_line, &loc.last_column);

            ox_note(ctxt, input, &loc, j ? "line start" : "line end");
        }
    }

    TEST(i == LINE_NUM);

    ox_input_close(ctxt, v);
}

void
input_test (OX_Context *ctxt)
{
    OX_Value *input = ox_value_stack_push(ctxt);
    OX_Value *s = ox_value_stack_push(ctxt);
    OX_CharBuffer cb;
    const char *cstr;
    int i, j;
    FILE *fp;

    ox_char_buffer_init(&cb);

    for (i = 0; i < LINE_NUM; i ++) {
        for (j = 0; j < i; j ++)
            ox_char_buffer_append_char(ctxt, &cb, j % 26 + 'a');
        ox_char_buffer_append_char(ctxt, &cb, '\n');
    }

    cstr = ox_char_buffer_get_char_star(ctxt, &cb);

    /*String input.*/
    ox_string_from_const_char_star(ctxt, s, cstr);
    ox_string_input_new(ctxt, input, s);

    do_input_test(ctxt, input);

    /*File input.*/
    fp = fopen(FILENAME, "wb");
    assert(fp);
    fputs(cstr, fp);
    fclose(fp);

    ox_file_input_new(ctxt, input, FILENAME);
    do_input_test(ctxt, input);

    unlink(FILENAME);

    ox_char_buffer_deinit(ctxt, &cb);
    ox_value_stack_pop(ctxt, input);
}
