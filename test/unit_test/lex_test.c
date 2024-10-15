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
 * Lexical analyzer test.
 */

#define OX_LOG_TAG "lex_test"

#include "test.h"

static void
do_lex_test (OX_Context *ctxt, const char *src, ...)
{
    OX_Value *s = ox_value_stack_push(ctxt);
    OX_Value *input = ox_value_stack_push(ctxt);
    OX_Lex lex;
    OX_Token tok;
    va_list ap;

    test_push_tag("\"%s\"", src);

    ox_string_from_char_star(ctxt, s, src);
    ox_string_input_new(ctxt, input, s);

    ox_lex_init(ctxt, &lex, ox_value_get_gco(ctxt, input));

    tok.v = ox_value_stack_push(ctxt);

    va_start(ap, src);

    while (1) {
        int tt;

        tt = va_arg(ap, int);

        ox_lex_token(ctxt, &lex, &tok, 0);

        if (tt >= OX_KEYWORD_BEGIN)
            TEST(tok.keyword == tt);
        else
            TEST(tok.type == tt);

        switch (tok.type) {
        case OX_TOKEN_BOOL: {
            OX_Bool b = va_arg(ap, int);

            TEST(b == ox_value_get_bool(ctxt, tok.v));
            break;
        }
        case OX_TOKEN_NUMBER: {
            OX_Number n = va_arg(ap, OX_Number);

            TEST(n == ox_value_get_number(ctxt, tok.v));
            break;
        }
        case OX_TOKEN_STRING:
        case OX_TOKEN_STR_HEAD:
        case OX_TOKEN_STR_MID:
        case OX_TOKEN_STR_TAIL: {
            char *s = va_arg(ap, char*);

            TEST(!strcmp(s, ox_string_get_char_star(ctxt, tok.v)));
            break;
        }
        case OX_TOKEN_ID: {
            if (tok.keyword == OX_KEYWORD_NONE) {
                char *s = va_arg(ap, char*);

                TEST(!strcmp(s, ox_string_get_char_star(ctxt, tok.v)));
            }
            break;
        }
        default:
            break;
        }

        if (tok.type == OX_TOKEN_END)
            break;
    }

    va_end(ap);

    ox_lex_deinit(ctxt, &lex);
    ox_value_stack_pop(ctxt, s);

    test_pop_tag();
} 

void
lex_test (OX_Context *ctxt)
{
    do_lex_test(ctxt, "", OX_TOKEN_END);
    do_lex_test(ctxt, " \t\v\f#!comment\n//comment\n/*commant\ncomment*/0", OX_TOKEN_NUMBER, 0., OX_TOKEN_END);
    do_lex_test(ctxt, "null", OX_TOKEN_NULL, OX_TOKEN_END);
    do_lex_test(ctxt, "true", OX_TOKEN_BOOL, OX_TRUE, OX_TOKEN_END);
    do_lex_test(ctxt, "false", OX_TOKEN_BOOL, OX_FALSE, OX_TOKEN_END);
    do_lex_test(ctxt, "0", OX_TOKEN_NUMBER, 0., OX_TOKEN_END);
    do_lex_test(ctxt, "1", OX_TOKEN_NUMBER, 1., OX_TOKEN_END);
    do_lex_test(ctxt, "123456789", OX_TOKEN_NUMBER, 123456789., OX_TOKEN_END);
    do_lex_test(ctxt, "123_456_789", OX_TOKEN_NUMBER, 123456789., OX_TOKEN_END);
    do_lex_test(ctxt, "1.23456789", OX_TOKEN_NUMBER, 1.23456789, OX_TOKEN_END);
    do_lex_test(ctxt, "1.23456789e8", OX_TOKEN_NUMBER, 1.23456789e8, OX_TOKEN_END);
    do_lex_test(ctxt, "1.23456789E8", OX_TOKEN_NUMBER, 1.23456789e8, OX_TOKEN_END);
    do_lex_test(ctxt, "1.23456789E+8", OX_TOKEN_NUMBER, 1.23456789e8, OX_TOKEN_END);
    do_lex_test(ctxt, "1.23456789e-2", OX_TOKEN_NUMBER, 1.23456789e-2, OX_TOKEN_END);
    do_lex_test(ctxt, "0x1234_5678", OX_TOKEN_NUMBER, 305419896., OX_TOKEN_END);
    do_lex_test(ctxt, "0Xabc_def", OX_TOKEN_NUMBER, 11259375., OX_TOKEN_END);
    do_lex_test(ctxt, "0o1234_567", OX_TOKEN_NUMBER, 342391., OX_TOKEN_END);
    do_lex_test(ctxt, "0O1234_567", OX_TOKEN_NUMBER, 342391., OX_TOKEN_END);
    do_lex_test(ctxt, "0b1111_0000", OX_TOKEN_NUMBER, 240., OX_TOKEN_END);
    do_lex_test(ctxt, "0b1010_1010", OX_TOKEN_NUMBER, 170., OX_TOKEN_END);
    do_lex_test(ctxt, "1234.", OX_TOKEN_NUMBER, 1234., '.', OX_TOKEN_END);
    do_lex_test(ctxt, "1234.e", OX_TOKEN_NUMBER, 1234., '.', OX_TOKEN_ID, "e", OX_TOKEN_END);
    do_lex_test(ctxt, "$1a_z9", OX_TOKEN_ID, "$1a_z9", OX_TOKEN_END);
    do_lex_test(ctxt, "a_$z9", OX_TOKEN_ID, "a_$z9", OX_TOKEN_END);
    do_lex_test(ctxt, "_$z9", OX_TOKEN_ID, "_$z9", OX_TOKEN_END);
    do_lex_test(ctxt, "\"\"", OX_TOKEN_STRING, "", OX_TOKEN_END);
    do_lex_test(ctxt, "\"ab01' \"", OX_TOKEN_STRING, "ab01\' ", OX_TOKEN_END);
    do_lex_test(ctxt, "\"\\n\\t\\v\\f\\r\\a\\b\\{\\\"\\\\\"", OX_TOKEN_STRING, "\n\t\v\f\r\a\b{\"\\", OX_TOKEN_END);
    do_lex_test(ctxt, "\"\\x41\\x7b\\x7D\"", OX_TOKEN_STRING, "A{}", OX_TOKEN_END);
    do_lex_test(ctxt, "\"\\u0041\\u{4A}\\u{1f495}\\ud83d\\udc95\"", OX_TOKEN_STRING, "AJ💕💕", OX_TOKEN_END);
    do_lex_test(ctxt, "\"a{1}b{2}c\"",
            OX_TOKEN_STR_HEAD, "a",
            OX_TOKEN_NUMBER, 1.,
            OX_TOKEN_STR_MID, "b",
            OX_TOKEN_NUMBER, 2.,
            OX_TOKEN_STR_TAIL, "c",
            OX_TOKEN_END);
    do_lex_test(ctxt, "<", '<', OX_TOKEN_END);
    do_lex_test(ctxt, "<<", OX_TOKEN_lt_lt, OX_TOKEN_END);
    do_lex_test(ctxt, "<<=", OX_TOKEN_lt_lt_eq, OX_TOKEN_END);
    do_lex_test(ctxt, ".", '.', OX_TOKEN_END);
    do_lex_test(ctxt, "..", '.', '.', OX_TOKEN_END);
    do_lex_test(ctxt, "...", OX_TOKEN_dot_dot_dot, OX_TOKEN_END);
    do_lex_test(ctxt, "...0", OX_TOKEN_dot_dot_dot, OX_TOKEN_NUMBER, 0., OX_TOKEN_END);
}
