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

#define OX_LOG_TAG "ox_gc"

#include "ox_internal.h"

/*GC object flags mask.*/
#define OX_GC_FL_MASK (OX_GC_FL_MARKED|OX_GC_FL_SCANNED)

/*Get the next object in the list.*/
static inline OX_GcObject*
gco_next (OX_GcObject *gco)
{
    return OX_SIZE2PTR(gco->next_flags & ~OX_GC_FL_MASK);
}

/*Scan the root objects.*/
static void
gc_scan_root (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_Context *c;
    size_t i;
    OX_GlobalRef *ref;

    /*Scan global reference hash table.*/
    ox_hash_foreach_c(&vm->global_ref_hash, i, ref, OX_GlobalRef, he) {
        ox_gc_mark(ctxt, ref->he.key);
    }

    ox_list_foreach_c(&vm->ctxt_list, c, OX_Context, ln) {
        /*Scan the value stack.*/
        ox_gc_scan_values(ctxt, c->bot_v_stack.items, c->bot_v_stack.len);

        /*Scan the frame stack.*/
        if (c->frames)
            ox_gc_mark(ctxt, c->frames);

        if (c->error_frames)
            ox_gc_mark(ctxt, c->error_frames);

        if (c->main_frames)
            ox_gc_mark(ctxt, c->main_frames);

        ox_gc_scan_value(ctxt, &c->error);

        /*Scan the status stack.*/
        ox_gc_scan_stack(ctxt, &c->bot_s_stack);
    }

    /*Scan strings.*/
    ox_gc_scan_values(ctxt, vm->strings, OX_STR_ID_MAX);

    /*Scan objects.*/
    ox_gc_scan_values(ctxt, vm->objects, OX_OBJ_ID_MAX);

    /*Scan the scripts.*/
    ox_gc_scan_script_hash(ctxt);

    /*Scan the packages.*/
    ox_gc_scan_package(ctxt);
}

/*Scan the objects.*/
static void
gc_scan_objects (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    while (1) {
        OX_GcObject *o;

        while (ox_vector_length(&vm->gc_marked_stack)) {
            vm->gc_marked_stack.len --;
            o = ox_vector_item(&vm->gc_marked_stack, vm->gc_marked_stack.len);

            o->next_flags |= OX_GC_FL_SCANNED;

            if (o->ops->scan) {
                o->ops->scan(ctxt, o);
            }
        }

        if (!vm->gc_marked_full)
            break;

        vm->gc_marked_full = OX_FALSE;
        vm->gc_scan_cnt ++;

        if (vm->gc_scan_cnt > 5) {
            size_t cap = ox_vector_capacity(&vm->gc_marked_stack);

            OX_LOG_D(ctxt, "expand GC marked stack to %"PRIdPTR"B", cap * 2);
            ox_not_error(ox_vector_set_capacity(ctxt, &vm->gc_marked_stack, cap * 2));

            vm->gc_scan_cnt = 0;
        }

        for (o = vm->gco_list; o; o = gco_next(o)) {
            if ((o->next_flags & OX_GC_FL_MASK) == OX_GC_FL_MARKED) {
                o->next_flags |= OX_GC_FL_SCANNED;

                if (o->ops->scan) {
                    o->ops->scan(ctxt, o);
                }
            }
        }
    }
}

/*Sweep th unused objects.*/
static void
gc_sweep (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_GcObject **po = &vm->gco_list;
    OX_GcObject *o;

    while ((o = *po)) {
        if (o->next_flags & OX_GC_FL_MARKED) {
            o->next_flags = OX_PTR2SIZE(gco_next(o));

            po = (OX_GcObject**)&o->next_flags;
        } else {
            *po = gco_next(o);

            if (o->ops->free)
                o->ops->free(ctxt, o);
        }
    }
}

/**
 * Add the GC managed object to the garbage collecter.
 * @param ctxt The current running context.
 * @param ptr The GC managed object's pointer.
 */
void
ox_gc_add (OX_Context *ctxt, void *ptr)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_GcObject *gco = ptr;

    gco->next_flags = OX_PTR2SIZE(vm->gco_list);
    vm->gco_list = gco;

    if ((vm->mem_allocted >= vm->gc_start_size)
            && (vm->mem_allocted * 3 > vm->gc_last_size * 4)) {
        ox_gc_run(ctxt);
    }
}

/**
 * Run the garbage collecter.
 * @param ctxt The current running context.
 */
void
ox_gc_run (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
#if OX_LOG_LEVEL <= OX_LOG_LEVEL_DEBUG
    size_t size = vm->mem_allocted;
#endif

    OX_LOG_D(ctxt, "gc start, allocated: %"PRIdPTR"B", vm->mem_allocted);

    vm->gc_marked_full = OX_FALSE;
    vm->gc_scan_cnt = 0;

    gc_scan_root(ctxt);

    gc_scan_objects(ctxt);

    gc_sweep(ctxt);

    vm->gc_last_size = vm->mem_allocted;

    OX_LOG_D(ctxt, "gc end, collect %"PRIdPTR"B", size - vm->mem_allocted);
}

/**
 * Mark the GC managed object as used.
 * This function must be invoked in garbage collection process.
 * @param ctxt The current running context.
 * @param gco The GC managed object.
 */
void
ox_gc_mark_inner (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_VM *vm = ox_vm_get(ctxt);
    size_t left;

    gco->next_flags |= OX_GC_FL_MARKED;

    left = ox_vector_space(&vm->gc_marked_stack);
    if (left) {
        size_t len = ox_vector_length(&vm->gc_marked_stack);

        ox_vector_item(&vm->gc_marked_stack, len) = gco;
        vm->gc_marked_stack.len ++;
    } else {
        vm->gc_marked_full = OX_TRUE;
    }
}

/**
 * Initialize the garbage collecter.
 * @param ctxt The running context.
 */
void
ox_gc_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    vm->gc_start_size = 64 * 1024;
    vm->gc_last_size = 0;
    vm->gco_list = NULL;

    /*Initialize the marked object stack.*/
    ox_vector_init(&vm->gc_marked_stack);
    ox_not_error(ox_vector_set_capacity(ctxt, &vm->gc_marked_stack, 64));
}

/**
 * Release the garbage collecter.
 * @param ctxt The running context.
 */
void
ox_gc_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_GcObject *o, *no;

    /*Free all the objects.*/
    for (o = vm->gco_list; o; o = no) {
        no = gco_next(o);

        if (o->ops->free)
            o->ops->free(ctxt, o);
    }

    /*Free the marked object stack.*/
    ox_vector_deinit(ctxt, &vm->gc_marked_stack);
}
