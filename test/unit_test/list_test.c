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
 * List test.
 */

#define OX_LOG_TAG "list_test"

#include "test.h"

#define NODE_NUM 1024

static OX_List nodes[NODE_NUM];

typedef struct {
    OX_List ln;
    int v;
} Container;

static Container cnodes[NODE_NUM];

void
list_test (OX_Context *ctxt)
{
    OX_LIST_DECL(list);
    OX_LIST_DECL(list2);
    OX_List *n, *t;
    Container *cn, *ct;
    int i;

    TEST(ox_list_empty(&list));
    TEST(ox_list_head(&list) == NULL);
    TEST(ox_list_tail(&list) == NULL);

    /*Append nodes.*/
    for (i = 0; i < NODE_NUM; i ++) {
        OX_List *n = &nodes[i];

        ox_list_append(&list, n);
        TEST(!ox_list_empty(&list));
        TEST(ox_list_head(&list) == nodes);
        TEST(ox_list_tail(&list) == n);
    }

    i = 0;
    ox_list_foreach(&list, n) {
        TEST(n == &nodes[i]);
        i ++;
    }
    TEST(i == NODE_NUM);

    i = 0;
    ox_list_foreach_r(&list, n) {
        TEST(n == &nodes[NODE_NUM - i - 1]);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Remove*/
    i = 0;
    ox_list_foreach_safe(&list, n, t) {
        TEST(n == &nodes[i]);
        ox_list_remove(n);
        if (i < NODE_NUM - 1)
            TEST(ox_list_head(&list) == &nodes[i + 1]);
        else
            TEST(ox_list_head(&list) == NULL);
        i ++;
    }
    TEST(i == NODE_NUM);
    TEST(ox_list_empty(&list));

    /*Prepend nodes.*/
    ox_list_init(&list);
    TEST(ox_list_empty(&list));

    for (i = 0; i < NODE_NUM; i ++) {
        OX_List *n = &nodes[i];

        ox_list_prepend(&list, n);
        TEST(!ox_list_empty(&list));
        TEST(ox_list_head(&list) == n);
        TEST(ox_list_tail(&list) == nodes);
    }

    i = 0;
    ox_list_foreach(&list, n) {
        TEST(n == &nodes[NODE_NUM - i - 1]);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Remove reversely.*/
    i = 0;
    ox_list_foreach_safe_r(&list, n, t) {
        TEST(n == &nodes[i]);
        ox_list_remove(n);
        if (i < NODE_NUM - 1)
            TEST(ox_list_tail(&list) == &nodes[i + 1]);
        else
            TEST(ox_list_tail(&list) == NULL);
        i ++;
    }
    TEST(i == NODE_NUM);
    TEST(ox_list_empty(&list));

    /*Add container nodes.*/
    ox_list_init(&list);

    for (i = 0; i < NODE_NUM; i ++) {
        Container *c = &cnodes[i];

        c->v = i;
        ox_list_append(&list, &c->ln);
    }

    /*Traverse container.*/
    i = 0;
    ox_list_foreach_c(&list, cn, Container, ln) {
        TEST(cn->v == i);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Traverse container reversely.*/
    i = 0;
    ox_list_foreach_c_r(&list, cn, Container, ln) {
        TEST(cn->v == NODE_NUM - i - 1);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Container macros.*/
    cn = ox_list_head_c(&list, Container, ln);
    i = 0;
    while (cn) {
        TEST(cn->v == i);
        cn = ox_list_next_c(&list, cn, Container, ln);
        i ++;
    }
    TEST(i == NODE_NUM);

    cn = ox_list_tail_c(&list, Container, ln);
    i = 0;
    while (cn) {
        TEST(cn->v == NODE_NUM - i - 1);
        cn = ox_list_prev_c(&list, cn, Container, ln);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Remove container.*/
    i = 0;
    ox_list_foreach_safe_c(&list, cn, ct, Container, ln) {
        TEST(cn->v == i);
        ox_list_remove(&cn->ln);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Add container nodes.*/
    for (i = 0; i < NODE_NUM; i ++) {
        Container *c = &cnodes[i];

        c->v = i;
        ox_list_append(&list, &c->ln);
    }

    /*Remove container reversely.*/
    i = 0;
    ox_list_foreach_safe_c_r(&list, cn, ct, Container, ln) {
        TEST(cn->v == NODE_NUM - i - 1);
        ox_list_remove(&cn->ln);
        i ++;
    }
    TEST(i == NODE_NUM);

    /*Join.*/
    ox_list_init(&list);

    for (i = 0; i < NODE_NUM / 2; i ++) {
        Container *c = &cnodes[i];

        c->v = i;
        ox_list_append(&list, &c->ln);
    }

    for (; i < NODE_NUM; i ++) {
        Container *c = &cnodes[i];

        c->v = i;
        ox_list_append(&list2, &c->ln);
    }

    ox_list_join(&list, &list2);

    i = 0;
    ox_list_foreach_c(&list, cn, Container, ln) {
        TEST(cn->v == i);
        i ++;
    }
    TEST(i == NODE_NUM);
}