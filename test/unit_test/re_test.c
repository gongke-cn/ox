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
 * Regular expression test.
 */

#define OX_LOG_TAG "re_test"

#include "test.h"

static void
do_re_test (OX_Context *ctxt, const char *csrc, int flags, const char *cstr, int n, ...)
{
    OX_Value *src = ox_value_stack_push(ctxt);
    OX_Value *s = ox_value_stack_push(ctxt);
    OX_Value *re = ox_value_stack_push(ctxt);
    OX_Value *mr = ox_value_stack_push(ctxt);
    OX_Value *groups = ox_value_stack_push(ctxt);
    OX_Value *sub = ox_value_stack_push(ctxt);
    va_list ap;
    OX_Result r;

    test_push_tag("re: %s flags: %d string: \"%s\"", csrc, flags, cstr);

    ox_string_from_const_char_star(ctxt, src, csrc);
    r = ox_re_new(ctxt, re, src, flags);
    TEST(r == OX_OK);

    ox_re_disassemble(ctxt, re, stdout);

    ox_string_from_const_char_star(ctxt, s, cstr);

    r = ox_re_match(ctxt, re, s, 0, 0, mr);
    TEST(r == OX_OK);

    if (n == 0) {
        TEST(ox_value_is_null(ctxt, mr));
    } else {
        int i;

        r = ox_get_s(ctxt, mr, "groups", groups);
        TEST(r == OX_OK);

        TEST(ox_array_length(ctxt, groups) == n);

        va_start(ap, n);

        for (i = 0; i < n; i ++) {
            const char *csub, *cexp;

            ox_array_get_item(ctxt, groups, i, sub);

            csub = ox_string_get_char_star(ctxt, sub);
            cexp = va_arg(ap, const char*);

            TEST(!strcmp(csub, cexp));
        }

        va_end(ap);
    }

    test_pop_tag();

    ox_value_stack_pop(ctxt, src);
}

void
re_test (OX_Context *ctxt)
{
    do_re_test(ctxt, "a", 0, "a", 1, "a");
    do_re_test(ctxt, "a", 0, "A", 0);
    do_re_test(ctxt, "a", OX_RE_FL_IGNORE_CASE, "A", 1, "A");
    do_re_test(ctxt, "a", 0, " a", 1, "a");
    do_re_test(ctxt, "^a", 0, " a", 0);
    do_re_test(ctxt, "a", 0, "a ", 1, "a");
    do_re_test(ctxt, "a$", 0, "a ", 0);
    do_re_test(ctxt, "a?", 0, "", 1, "");
    do_re_test(ctxt, "a?", 0, "a", 1, "a");
    do_re_test(ctxt, "a?", 0, "aa", 1, "a");
    do_re_test(ctxt, "a??", 0, "aa", 1, "");
    do_re_test(ctxt, "a+", 0, "", 0);
    do_re_test(ctxt, "a+", 0, "a", 1, "a");
    do_re_test(ctxt, "a+", 0, "aa", 1, "aa");
    do_re_test(ctxt, "a+", 0, "aaa", 1, "aaa");
    do_re_test(ctxt, "a+?", 0, "aaa", 1, "a");
    do_re_test(ctxt, "a*", 0, "", 1, "");
    do_re_test(ctxt, "a*", 0, "a", 1, "a");
    do_re_test(ctxt, "a*", 0, "aa", 1, "aa");
    do_re_test(ctxt, "a*", 0, "aaa", 1, "aaa");
    do_re_test(ctxt, "a*?", 0, "aaa", 1, "");
    do_re_test(ctxt, "\\s", 0, " ", 1, " ");
    do_re_test(ctxt, "\\s", 0, "\t", 1, "\t");
    do_re_test(ctxt, "\\s", 0, "\v", 1, "\v");
    do_re_test(ctxt, "\\s", 0, "\f", 1, "\f");
    do_re_test(ctxt, "\\s", 0, "a", 0);
    do_re_test(ctxt, "\\s", 0, "0", 0);
    do_re_test(ctxt, "\\S", 0, " ", 0);
    do_re_test(ctxt, "\\S", 0, "\t", 0);
    do_re_test(ctxt, "\\S", 0, "\v", 0);
    do_re_test(ctxt, "\\S", 0, "\f", 0);
    do_re_test(ctxt, "\\S", 0, "a", 1, "a");
    do_re_test(ctxt, "\\S", 0, "0", 1, "0");
    do_re_test(ctxt, "\\d", 0, "0", 1, "0");
    do_re_test(ctxt, "\\d", 0, "9", 1, "9");
    do_re_test(ctxt, "\\d", 0, "a", 0);
    do_re_test(ctxt, "\\d", 0, " ", 0);
    do_re_test(ctxt, "\\D", 0, "0", 0);
    do_re_test(ctxt, "\\D", 0, "9", 0);
    do_re_test(ctxt, "\\D", 0, "a", 1, "a");
    do_re_test(ctxt, "\\D", 0, " ", 1, " ");
    do_re_test(ctxt, "\\w", 0, "0", 1, "0");
    do_re_test(ctxt, "\\w", 0, "9", 1, "9");
    do_re_test(ctxt, "\\w", 0, "a", 1, "a");
    do_re_test(ctxt, "\\w", 0, "z", 1, "z");
    do_re_test(ctxt, "\\w", 0, "A", 1, "A");
    do_re_test(ctxt, "\\w", 0, "Z", 1, "Z");
    do_re_test(ctxt, "\\w", 0, "_", 1, "_");
    do_re_test(ctxt, "\\w", 0, " ", 0);
    do_re_test(ctxt, "\\w", 0, "!", 0);
    do_re_test(ctxt, "\\W", 0, "0", 0);
    do_re_test(ctxt, "\\W", 0, "9", 0);
    do_re_test(ctxt, "\\W", 0, "a", 0);
    do_re_test(ctxt, "\\W", 0, "z", 0);
    do_re_test(ctxt, "\\W", 0, "A", 0);
    do_re_test(ctxt, "\\W", 0, "Z", 0);
    do_re_test(ctxt, "\\W", 0, "_", 0);
    do_re_test(ctxt, "\\W", 0, " ", 1, " ");
    do_re_test(ctxt, "\\W", 0, "!", 1, "!");
    do_re_test(ctxt, "[abc]", 0, "a", 1, "a");
    do_re_test(ctxt, "[abc]", 0, "b", 1, "b");
    do_re_test(ctxt, "[abc]", 0, "c", 1, "c");
    do_re_test(ctxt, "[abc]", 0, "d", 0);
    do_re_test(ctxt, "[abc]", 0, "A", 0);
    do_re_test(ctxt, "[abc]", 0, "B", 0);
    do_re_test(ctxt, "[abc]", 0, "C", 0);
    do_re_test(ctxt, "[abc]", OX_RE_FL_IGNORE_CASE, "A", 1, "A");
    do_re_test(ctxt, "[abc]", OX_RE_FL_IGNORE_CASE, "B", 1, "B");
    do_re_test(ctxt, "[abc]", OX_RE_FL_IGNORE_CASE, "C", 1, "C");
    do_re_test(ctxt, "[abc]", OX_RE_FL_IGNORE_CASE, "D", 0);
    do_re_test(ctxt, "[ABC]", OX_RE_FL_IGNORE_CASE, "a", 1, "a");
    do_re_test(ctxt, "[ABC]", OX_RE_FL_IGNORE_CASE, "b", 1, "b");
    do_re_test(ctxt, "[ABC]", OX_RE_FL_IGNORE_CASE, "c", 1, "c");
    do_re_test(ctxt, "[ABC]", OX_RE_FL_IGNORE_CASE, "d", 0);
    do_re_test(ctxt, "[a-c]", 0, "a", 1, "a");
    do_re_test(ctxt, "[a-c]", 0, "b", 1, "b");
    do_re_test(ctxt, "[a-c]", 0, "c", 1, "c");
    do_re_test(ctxt, "[a-c]", 0, "d", 0);
    do_re_test(ctxt, "[a-c]", 0, "A", 0);
    do_re_test(ctxt, "[a-c]", 0, "B", 0);
    do_re_test(ctxt, "[a-c]", 0, "C", 0);
    do_re_test(ctxt, "[a-c]", OX_RE_FL_IGNORE_CASE, "A", 1, "A");
    do_re_test(ctxt, "[a-c]", OX_RE_FL_IGNORE_CASE, "B", 1, "B");
    do_re_test(ctxt, "[a-c]", OX_RE_FL_IGNORE_CASE, "C", 1, "C");
    do_re_test(ctxt, "[a-c]", OX_RE_FL_IGNORE_CASE, "D", 0);
    do_re_test(ctxt, "[A-C]", OX_RE_FL_IGNORE_CASE, "a", 1, "a");
    do_re_test(ctxt, "[A-C]", OX_RE_FL_IGNORE_CASE, "b", 1, "b");
    do_re_test(ctxt, "[A-C]", OX_RE_FL_IGNORE_CASE, "c", 1, "c");
    do_re_test(ctxt, "[A-C]", OX_RE_FL_IGNORE_CASE, "d", 0);
    do_re_test(ctxt, "[^abc]", 0, "a", 0);
    do_re_test(ctxt, "[^abc]", 0, "b", 0);
    do_re_test(ctxt, "[^abc]", 0, "c", 0);
    do_re_test(ctxt, "[^abc]", 0, "d", 1, "d");
    do_re_test(ctxt, "[^a-c]", 0, "a", 0);
    do_re_test(ctxt, "[^a-c]", 0, "b", 0);
    do_re_test(ctxt, "[^a-c]", 0, "c", 0);
    do_re_test(ctxt, "[^a-c]", 0, "d", 1, "d");
    do_re_test(ctxt, "abc", 0, "abc", 1, "abc");
    do_re_test(ctxt, "abc", 0, "abC", 0);
    do_re_test(ctxt, "abc", OX_RE_FL_IGNORE_CASE, "abC", 1, "abC");
    do_re_test(ctxt, "abc|def", 0, "abc", 1, "abc");
    do_re_test(ctxt, "abc|def", 0, "def", 1, "def");
    do_re_test(ctxt, "abc|def", 0, "abd", 0);
    do_re_test(ctxt, "([a-z]+)([0-9]+)", 0, "abc012", 3, "abc012", "abc", "012");
    do_re_test(ctxt, "abc(?:def)", 0, "abcdef", 1, "abcdef");
    do_re_test(ctxt, "abc(?:def)", 0, "abcdee", 0);
    do_re_test(ctxt, "abc(?=def)", 0, "abcdef", 1, "abc");
    do_re_test(ctxt, "abc(?=def)", 0, "abcdeg", 0);
    do_re_test(ctxt, "abc(?!def)", 0, "abcdef", 0);
    do_re_test(ctxt, "abc(?!def)", 0, "abcdeg", 1, "abc");
    do_re_test(ctxt, "abc(?!def)", 0, "abc", 1, "abc");
    do_re_test(ctxt, "(?<=abc)def", 0, "abcdef", 1, "def");
    do_re_test(ctxt, "(?<=abc)def", 0, "abbdef", 0);
    do_re_test(ctxt, "(?<!abc)def", 0, "abcdef", 0);
    do_re_test(ctxt, "(?<!abc)def", 0, "abbdef", 1, "def");
    do_re_test(ctxt, "(?<!abc)def", 0, "def", 1, "def");
    do_re_test(ctxt, "^abc", 0, "def\nabc", 0);
    do_re_test(ctxt, "^abc", OX_RE_FL_MULTILINE, "def\nabc", 1, "abc");
    do_re_test(ctxt, "abc$", 0, "abc\ndef", 0);
    do_re_test(ctxt, "abc$", OX_RE_FL_MULTILINE, "abc\ndef", 1, "abc");
    do_re_test(ctxt, "abc.", 0, "abc\n", 0);
    do_re_test(ctxt, "abc.", OX_RE_FL_DOT_ALL, "abc\n", 1, "abc\n");
    do_re_test(ctxt, "天地", 0, "天地", 0);
    do_re_test(ctxt, "天地", OX_RE_FL_UNICODE, "天地", 1, "天地");
    do_re_test(ctxt, ".*a", 0, "a", 1, "a");
    do_re_test(ctxt, "\\d+|0[xX]\\d+", OX_RE_FL_PERFECT, "0x1", 1, "0x1");
}
