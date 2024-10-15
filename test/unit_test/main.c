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
 * Main function of unit test.
 */

#define OX_LOG_TAG "unit_test"

#include "test.h"

extern void log_test (OX_Context *ctxt);
extern void list_test (OX_Context *ctxt);
extern void mem_test (OX_Context *ctxt);
extern void vector_test (OX_Context *ctxt);
extern void char_buffer_test (OX_Context *ctxt);
extern void hash_test (OX_Context *ctxt);
extern void gc_test (OX_Context *ctxt);
extern void value_test (OX_Context *ctxt);
extern void string_test (OX_Context *ctxt);
extern void array_test (OX_Context *ctxt);
extern void input_test (OX_Context *ctxt);
extern void lex_test (OX_Context *ctxt);
extern void object_test (OX_Context *ctxt);
extern void class_test (OX_Context *ctxt);
extern void re_test (OX_Context *ctxt);

/** Test tag entry.*/
typedef struct TestTag_s TestTag;
/** Test tag entry.*/
struct TestTag_s {
    TestTag *bot; /**< The bottom tag entry.*/
    char    *tag; /**< Tag string.*/
};

/*Successed test cases' number.*/
static int test_ok_num = 0;
/*Failed test cases' number.*/
static int test_failed_num = 0;
/*Test tag stack.*/
static TestTag *tag_stack = NULL;

/**
 * Push test message tag to the stack.
 * @param fmt Tag format pattern.
 * @param ... Arguments.
 */
void
test_push_tag (const char *fmt, ...)
{
    TestTag *tt;
    va_list ap;
    char buf[256];

    tt = malloc(sizeof(TestTag));
    assert(tt);

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    tt->tag = strdup(buf);
    assert(tt->tag);

    tt->bot = tag_stack;;
    tag_stack = tt;
}

/**
 * Pop up the top tag from the stack.
 */
void
test_pop_tag (void)
{
    TestTag *tt = tag_stack;

    assert(tt);

    tag_stack = tt->bot;

    free(tt->tag);
    free(tt);
}

/**
 * Test the condition.
 * @param cond Condition value. If it is 0, test failed.
 * @param file The file which run the test.
 * @param func
 */
void
test (int cond, const char *file, const char *func, int line, const char *msg)
{
    if (cond) {
        test_ok_num ++;
    } else {
        TestTag *tt;

        test_failed_num ++;

        fprintf(stderr, "test failed: \"%s\" %s %d: ", file, func, line);

        for (tt = tag_stack; tt; tt = tt->bot)
            fprintf(stderr, "%s: ", tt->tag);

        fprintf(stderr, "%s\n", msg);
    }
}

int
main (int argc, char **argv)
{
    OX_VM *vm;
    OX_Context *ctxt;

    vm = ox_vm_new();
    ctxt = ox_context_get(vm);

    log_test(ctxt);
    list_test(ctxt);
    mem_test(ctxt);
    vector_test(ctxt);
    char_buffer_test(ctxt);
    hash_test(ctxt);
    gc_test(ctxt);
    value_test(ctxt);
    string_test(ctxt);
    array_test(ctxt);
    input_test(ctxt);
    lex_test(ctxt);
    object_test(ctxt);
    class_test(ctxt);
    re_test(ctxt);

    ox_vm_free(vm);

    printf("test total: %d failed: %d\n",
            test_ok_num + test_failed_num, test_failed_num);

    return test_failed_num ? 1 : 0;
}
