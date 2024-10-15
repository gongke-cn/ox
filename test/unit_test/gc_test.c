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
 * GC test.
 */

#define OX_LOG_TAG "gc_test"

#include "test.h"

typedef struct {
    OX_GcObject gco;
    void *ptr;
    OX_Bool used;
} Object;

static void
object_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    Object *o = (Object*)gco;

    if (o->ptr)
        ox_gc_mark(ctxt, o->ptr);
}

static void
object_free (OX_Context *ctxt, OX_GcObject *gco)
{
    Object *o = (Object*)gco;

    o->used = OX_FALSE;
}

static const OX_GcObjectOps
object_ops = {
    0,
    object_scan,
    object_free
};

#define OBJECT_NUM 1024

static Object objects[OBJECT_NUM];

void
gc_test (OX_Context *ctxt)
{
    Object *o;
    int i;

    for (i = 0; i < OBJECT_NUM; i ++) {
        o = &objects[i];

        o->gco.ops = &object_ops;
        o->used = OX_TRUE;

        if (i < OBJECT_NUM - 1)
            o->ptr = &objects[i + 1];
        else
            o->ptr = NULL;

        ox_gc_add(ctxt, o);
    }

    ox_gc_mark(ctxt, &objects[0]);
    ox_gc_run(ctxt);

    for (i = 0; i < OBJECT_NUM; i ++) {
        o = &objects[i];
        TEST(o->used);
    }

    ox_gc_run(ctxt);

    for (i = 0; i < OBJECT_NUM; i ++) {
        o = &objects[i];
        TEST(!o->used);
    }
}