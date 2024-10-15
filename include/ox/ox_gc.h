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
 * Garbage collecter.
 */

#ifndef _OX_GC_H_
#define _OX_GC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add the GC managed object to the garbage collecter.
 * @param ctxt The current running context.
 * @param ptr The GC managed object's pointer.
 */
extern void
ox_gc_add (OX_Context *ctxt, void *ptr);

/**
 * Run the garbage collecter.
 * @param ctxt The current running context.
 */
extern void
ox_gc_run (OX_Context *ctxt);

/**
 * Mark the GC managed object as used.
 * This function must be invoked in garbage collection process.
 * @param ctxt The current running context.
 * @param gco The GC managed object.
 */
extern void
ox_gc_mark_inner (OX_Context *ctxt, OX_GcObject *gco);

/**
 * Mark the GC managed object as used.
 * This function must be invoked in garbage collection process.
 * @param ctxt The current running context.
 * @param ptr The GC managed object's pointer.
 */
static inline void
ox_gc_mark (OX_Context *ctxt, void *ptr)
{
    OX_GcObject *gco = ptr;

    if (!(gco->next_flags & OX_GC_FL_MARKED))
        ox_gc_mark_inner(ctxt, gco);
}

/**
 * Scan referenced objects in the value.
 * @param ctxt The current running context.
 * @param v The value to be scanned.
 */
static inline void
ox_gc_scan_value (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    if (ox_value_get_tag(v) == OX_VALUE_TAG_GCO) {
        OX_GcObject *gco = ox_value_pointer_get_gco(v);

        ox_gc_mark(ctxt, gco);
    }
}

/**
 * Scan referenced objects in the value buffer.
 * @param ctxt The current running context.
 * @param v The values to be scanned.
 * @param n Number of the values to be scanned.
 */
static inline void
ox_gc_scan_values (OX_Context *ctxt, OX_Value *v, size_t n)
{
    OX_Value *lv;

    v = ox_value_get_pointer(ctxt, v);
    lv = v + n;

    while (v < lv) {
        if (ox_value_get_tag(v) == OX_VALUE_TAG_GCO) {
            OX_GcObject *gco = ox_value_pointer_get_gco(v);

            ox_gc_mark(ctxt, gco);
        }
        v ++;
    }
}

#ifdef __cplusplus
}
#endif

#endif
