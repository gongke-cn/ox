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
 * Fiber.
 */

#define OX_LOG_TAG "fiber"

#include "ox_internal.h"

/*Declaration index.*/
enum {
    ID_Fiber,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Fiber",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*Scan referenced objects in a fiber.*/
static void
fiber_scan (OX_Context *ctxt, void *p)
{
    OX_Fiber *fiber = p;

    ox_gc_scan_value(ctxt, &fiber->func);
    ox_gc_scan_value(ctxt, &fiber->rv);

    if (fiber->args)
        ox_gc_scan_values(ctxt, fiber->args, fiber->argc);

    if (fiber->rsr.frame)
        ox_gc_mark(ctxt, fiber->rsr.frame);

    ox_gc_scan_values(ctxt, fiber->v_stack.items, fiber->v_stack.len);
    ox_gc_scan_stack(ctxt, &fiber->s_stack);
}

/*Free a fiber.*/
static void
fiber_free (OX_Context *ctxt, void *p)
{
    OX_Fiber *fiber = p;
    size_t i;

    /*Free the arguments.*/
    if (fiber->args)
        OX_DEL_N(ctxt, fiber->args, fiber->argc);

    /*Clear the stack.*/
    for (i = 0; i < fiber->s_stack.len; i ++) {
        OX_Stack *se = &ox_vector_item(&fiber->s_stack, i);

        ox_stack_deinit(ctxt, se);
    }

    ox_vector_deinit(ctxt, &fiber->v_stack);
    ox_vector_deinit(ctxt, &fiber->s_stack);

    OX_DEL(ctxt, fiber);
}

/*Fiber's operation functions.*/
static const OX_PrivateOps
fiber_ops = {
    fiber_scan,
    fiber_free
};

/*Fiber.$inf.$init*/
static OX_Result
Fiber_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *func = ox_argument(ctxt, args, argc, 0);
    OX_Value *targ = ox_argument(ctxt, args, argc, 1);
    OX_Function *fp;
    OX_Frame *old_frame, *frame;
    OX_Fiber *fiber;
    OX_Result r;

    if (ox_value_get_gco_type(ctxt, func) != OX_GCO_FUNCTION) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a function"));
        goto end;
    }

    /*Create a new fiber.*/
    if (!OX_NEW(ctxt, fiber)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    fiber->state = OX_FIBER_STATE_INIT;
    fiber->yr = NULL;
    fiber->rsr.frame = NULL;

    ox_value_copy(ctxt, &fiber->func, func);

    if (argc > 2) {
        fiber->argc = argc - 2;

        if (!OX_NEW_N(ctxt, fiber->args, fiber->argc)) {
            OX_DEL(ctxt, fiber);
            r = ox_throw_no_mem_error(ctxt);
            goto end;
        }

        ox_values_copy(ctxt, fiber->args, ox_values_item(ctxt, args, 2), fiber->argc);
    } else {
        fiber->args = NULL;
        fiber->argc = 0;
    }

    ox_value_set_null(ctxt, &fiber->rv);
    ox_vector_init(&fiber->v_stack);
    ox_vector_init(&fiber->s_stack);

    if ((r = ox_object_set_priv(ctxt, thiz, &fiber_ops, fiber)) == OX_ERR) {
        fiber_free(ctxt, fiber);
        goto end;
    }

    /*Create the function's frame.*/
    fp = ox_value_get_gco(ctxt, &fiber->func);

    old_frame = ctxt->frames;

    frame = ox_frame_push(ctxt, &fiber->func, fp->sfunc->decl_hash.e_num);
    if (!frame)
        goto end;

    frame->ip = 0;
    fiber->rsr.frame = frame;
    ctxt->frames = old_frame;
    ox_value_copy(ctxt, &frame->thiz, targ);

    /*Allocate register and argv buffer.*/
    if ((r = ox_vector_expand(ctxt, &fiber->v_stack, fp->sfunc->reg_num + 1)) == OX_ERR)
        goto end;

    ox_values_set_null(ctxt, fiber->v_stack.items, fp->sfunc->reg_num + 1);

    /*Call the function.*/
    fiber->state = OX_FIBER_STATE_RUN;
    fiber->rsr.vp = 0;
    fiber->rsr.sp = 0;
    fiber->rsr.args = fiber->args;
    fiber->rsr.argc = fiber->argc;
    fiber->rsr.rv = &fiber->rv;

    r = ox_function_call(ctxt, NULL, NULL, NULL, 0, NULL, fiber);
end:
    return r;
}

/*Fiber.$inf.end get*/
static OX_Result
Fiber_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Fiber *fiber;
    OX_Bool b;

    if (!(fiber = ox_object_get_priv(ctxt, thiz, &fiber_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a fiber"));

    b = (fiber->state == OX_FIBER_STATE_END) || (fiber->state == OX_FIBER_STATE_ERROR);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Fiber.$inf.value get*/
static OX_Result
Fiber_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Fiber *fiber;

    if (!(fiber = ox_object_get_priv(ctxt, thiz, &fiber_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a fiber"));

    ox_value_copy(ctxt, rv, &fiber->rv);
    return OX_OK;
}

/*Fiber.$inf.next*/
static OX_Result
Fiber_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Fiber *fiber;
    OX_Result r;

    if (!(fiber = ox_object_get_priv(ctxt, thiz, &fiber_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a fiber"));

    switch (fiber->state) {
    case OX_FIBER_STATE_RUN:
        if (fiber->yr) {
            size_t rid = OX_VALUE_PTR2IDX(fiber->yr);

            ox_value_copy(ctxt, &ox_vector_item(&fiber->v_stack, rid), arg);
        }

        fiber->rsr.frame->ip += 3;

        r = ox_function_call(ctxt, NULL, NULL, NULL, 0, NULL, fiber);
        break;
    case OX_FIBER_STATE_END:
        r = OX_OK;
        break;
    case OX_FIBER_STATE_ERROR:
        r = ox_throw(ctxt, &fiber->rv);
        break;
    default:
        assert(0);
    }

    return r;
}

/*?
 *? @lib Fiber.
 *?
 *? @class{ Fiber Fiber.
 *? @inherit {Iterator}
 *?
 *? @func $init Initialize a fiber object.
 *? @param fn {Function(Any)=>Any} The fiber's entry function.
 *? @param arg The argument of the entry function.
 *? @throw {SystemError} Create the new thread failed.
 *?
 *? @roacc end {Bool} If the fiber is end.
 *? @roacc value {Any} The fiber's result value.
 *?
 *? @func next Run the fiber until end or yield.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*Fiber.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Fiber"));
    ox_not_error(ox_class_inherit(ctxt, c, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Fiber, c));

    /*Fiber.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Fiber_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "next", Fiber_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "end", Fiber_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "value", Fiber_inf_value_get, NULL));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
