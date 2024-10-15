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
 * Value frame.
 */

#define OX_LOG_TAG "ox_frame"

#include "ox_internal.h"

/*Scan referenced objects in the value frame.*/
static void
frame_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Frame *f = (OX_Frame*)gco;

    ox_gc_scan_value(ctxt, &f->func);

    if (f->bot)
        ox_gc_mark(ctxt, f->bot);

    if (f->v)
        ox_gc_scan_values(ctxt, f->v, f->len);

    ox_gc_scan_value(ctxt, &f->thiz);
}

/*Free the value frame.*/
static void
frame_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Frame *f = (OX_Frame*)gco;

    if (f->v)
        OX_DEL_N(ctxt, f->v, f->len);

    OX_DEL(ctxt, f);
}

/*Operation of the value frame.*/
static const OX_GcObjectOps
frame_ops = {
    OX_GCO_FRAME,
    frame_scan,
    frame_free
};

/**
 * Push a new frame to the stack.
 * @param ctxt The current running context.
 * @param func The function use this frame.
 * @param len Frame's value buffer length.
 * @return The new frame.
 */
OX_Frame*
ox_frame_push (OX_Context *ctxt, OX_Value *func, size_t len)
{
    OX_Frame *f;

    assert(ctxt && func);

    if (!OX_NEW(ctxt, f)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    if (len) {
        if (!OX_NEW_N(ctxt, f->v, len)) {
            ox_throw_no_mem_error(ctxt);
            OX_DEL(ctxt, f);
            return NULL;
        }

        ox_values_set_null(ctxt, f->v, len);
    } else {
        f->v = NULL;
    }

    f->gco.ops = &frame_ops;
    f->ip = -1;
    f->bot = ctxt->frames;
    f->len = len;

    ox_value_set_null(ctxt, &f->thiz);
    ox_value_copy(ctxt, &f->func, func);

    ctxt->frames = f;

    ox_gc_add(ctxt, f);

    return f;
}
