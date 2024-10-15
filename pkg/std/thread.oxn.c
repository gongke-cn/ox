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
 * Thread functions.
 */

#define OX_LOG_TAG "thread"

#include <pthread.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_Thread,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Thread",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Thread data.*/
typedef struct {
    OX_Thread th;   /**< The thread handle.*/
    OX_VM    *vm;   /**< The virtual machine.*/
    OX_Value  fn;   /**< The entry function of the thread.*/
    OX_Value  thiz; /**< This argument of the entry function.*/
    OX_Value *args; /**< Arguments of the entry function.*/
    size_t    argc; /**< Arguments' count of the entry function.*/
    OX_Value  rv;   /**< Return value of the thread.*/
    OX_Result r;    /**< Result of the function.*/
} OX_ThreadData;

/*Thread entry.*/
static void*
thread_entry (void *arg)
{
    OX_Object *o = arg;
    OX_ThreadData *th = o->priv;
    OX_Context *ctxt = ox_context_get(th->vm);
    OX_Value *thv;
    OX_Result r;

    ox_lock(ctxt);
    thv = ox_value_stack_push(ctxt);
    ox_value_set_gco(ctxt, thv, o);

    ox_global_unref(ctxt, thv);

    r = ox_call(ctxt, &th->fn, &th->thiz, th->args, th->argc, &th->rv);
    if (r == OX_ERR)
        ox_catch(ctxt, &th->rv);
    th->r = r;

    ox_value_stack_pop(ctxt, thv);
    ox_unlock(ctxt);

    return NULL;
}

/*Scan referenced objects in the thread data.*/
static void
thread_scan (OX_Context *ctxt, void *p)
{
    OX_ThreadData *th = p;

    ox_gc_scan_value(ctxt, &th->fn);
    ox_gc_scan_value(ctxt, &th->thiz);
    ox_gc_scan_value(ctxt, &th->rv);

    if (th->args)
        ox_gc_scan_values(ctxt, th->args, th->argc);
}

/*Free the thread data.*/
static void
thread_free (OX_Context *ctxt, void *p)
{
    OX_ThreadData *th = p;

    if (th->args)
        OX_DEL_N(ctxt, th->args, th->argc);

    OX_DEL(ctxt, th);
}

/*Thread data's operation functions.*/
static const OX_PrivateOps
thread_ops = {
    thread_scan,
    thread_free
};

/*Get the thread data from the value.*/
static OX_ThreadData*
thread_get (OX_Context *ctxt, OX_Value *v)
{
    OX_ThreadData *th = ox_object_get_priv(ctxt, v, &thread_ops);

    if (!th)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a thread"));

    return th;
}

/*Thread.$inf.$init*/
static OX_Result
Thread_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);
    OX_Value *targ = ox_argument(ctxt, args, argc, 1);
    OX_ThreadData *th;
    OX_Result r;

    if (!OX_NEW(ctxt, th))
        return ox_throw_no_mem_error(ctxt);

    th->vm = ox_vm_get(ctxt);
    ox_value_copy(ctxt, &th->fn, fn);
    ox_value_copy(ctxt, &th->thiz, targ);
    ox_value_set_null(ctxt, &th->rv);
    th->r = 0;

    if (argc > 2) {
        th->argc = argc - 2;

        if (!OX_NEW_N(ctxt, th->args, th->argc)) {
            OX_DEL(ctxt, th);
            return ox_throw_no_mem_error(ctxt);
        }

        ox_values_copy(ctxt, th->args, ox_values_item(ctxt, args, 2), th->argc);
    } else {
        th->args = NULL;
        th->argc = 0;
    }

    r = ox_object_set_priv(ctxt, thiz, &thread_ops, th);
    if (r == OX_ERR) {
        thread_free(ctxt, th);
        return r;
    }

    if ((r = ox_global_ref(ctxt, thiz)) == OX_ERR)
        return r;

    r = ox_thread_create(ctxt, &th->th, thread_entry, ox_value_get_gco(ctxt, thiz));
    if (r == OX_ERR) {
        ox_global_unref(ctxt, thiz);
        thread_free(ctxt, th);
        return r;
    }

    return OX_OK;
}

/*Thread.$inf.detach*/
static OX_Result
Thread_inf_detach (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ThreadData *th;

    if (!(th = thread_get(ctxt, thiz)))
        return OX_ERR;

    return ox_thread_detach(ctxt, &th->th);
}

/*Thread.$inf.join*/
static OX_Result
Thread_inf_join (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ThreadData *th;
    OX_Result r;

    if (!(th = thread_get(ctxt, thiz)))
        return OX_ERR;

    ox_unlock(ctxt);
    r = ox_thread_join(ctxt, &th->th);
    ox_lock(ctxt);
    if (r == OX_ERR)
        return r;

    if (th->r == OX_ERR)
        return ox_throw(ctxt, &th->rv);

    ox_value_copy(ctxt, rv, &th->rv);
    return OX_OK;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Thread.
 *?
 *? @class{ Thread Thread.
 *?
 *? @func $init Initialize a thread object.
 *? @param fn {Function(Any...)=>Any} The thread's entry function.
 *? @param thiz {Any} This argument of the function.
 *? @param args {[Any]} The arguments of the function.
 *? @throw {SystemError} Create the new thread failed.
 *?
 *? @func detach Detach the thread.
 *? If the thread is detached, when the thread is stopped,
 *? the thread's resource is released directy. Other thread do
 *? not need to join it.
 *?
 *? @func join (a,b,c) Wait a joinable thread and release its resource.
 *? Susend current thread and wait the thread until it is stopped.
 *? Then release the thread's resource.
 *? @return The thread's return value.
 *? @throw {Error} If the thread has error, throw the error object.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*Thread.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Thread"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Thread, c));

    /*Thread.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Thread_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "detach", Thread_inf_detach));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "join", Thread_inf_join));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
