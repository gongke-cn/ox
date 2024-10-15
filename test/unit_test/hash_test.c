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
 * Hash table test.
 */

#define OX_LOG_TAG "hash_test"

#include "test.h"

typedef struct {
    OX_HashEntry he;
    int v;
} Entry;

#define ENTRY_NUM 1024

static Entry entries[ENTRY_NUM];

static size_t
hash_key (OX_Context *ctxt, void *k)
{
    return OX_PTR2SIZE(k);
}

static OX_Bool
hash_equal (OX_Context *ctxt, void *k1, void *k2)
{
    return k1 == k2;
}

static const OX_HashOps
hash_ops = {
    hash_key,
    hash_equal
};

void
hash_test (OX_Context *ctxt)
{
    OX_Hash hash;
    Entry *e, *t, *re;
    int i, n;

    ox_hash_init(&hash, &hash_ops);
    TEST(ox_hash_size(&hash) == 0);

    /*Add.*/
    for (i = 0; i < ENTRY_NUM; i ++) {
        e = &entries[i];

        TEST(ox_hash_lookup_c(ctxt, &hash, e, NULL, Entry, he) == NULL);

        e->v = i;
        TEST(ox_hash_insert(ctxt, &hash, e, &e->he, NULL) == OX_OK);
        TEST(ox_hash_size(&hash) == i + 1);
    }

    /*Lookup.*/
    for (i = 0; i < ENTRY_NUM; i ++) {
        e = &entries[i];

        re = ox_hash_lookup_c(ctxt, &hash, e, NULL, Entry, he);
        TEST(re == e);
    }

    /*Traverse.*/
    n = 0;
    ox_hash_foreach_c(&hash, i, e, Entry, he) {
        TEST(e->he.key == e);
        n ++;
    }
    TEST(n == ENTRY_NUM);

    /*Remove.*/
    n = 0;
    ox_hash_foreach_safe_c(&hash, i, e, t, Entry, he) {
        TEST(e->he.key == e);
        ox_hash_remove(ctxt, &hash, e, NULL);
        TEST(ox_hash_size(&hash) == ENTRY_NUM - n - 1);
        n ++;
    }
    TEST(n == ENTRY_NUM);

    ox_hash_deinit(ctxt, &hash);
}
