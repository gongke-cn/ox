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
 * Function.
 */

#define OX_LOG_TAG "ox_function"

#include "ox_internal.h"

/*Scan referenced objects in the function.*/
static void
function_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Function *f = (OX_Function*)gco;

    ox_object_scan(ctxt, gco);

    /*Scan the script.*/
    ox_gc_mark(ctxt, f->sfunc->script);

    /*Scan the referenced frames.*/
    if (f->frames) {
        int i;

        for (i = 0; i < f->sfunc->frame_num; i ++) {
            OX_Frame *ref = f->frames[i];

            if (ref)
                ox_gc_mark(ctxt, ref);
        }
    }
}

/*Free the function.*/
static void
function_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Function *f = (OX_Function*)gco;

    ox_object_deinit(ctxt, &f->o);

    if (f->frames)
        OX_DEL_N(ctxt, f->frames, f->sfunc->frame_num);

    OX_DEL(ctxt, f);
}

/*Call the function.*/
static OX_Result
function_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Function *f = ox_value_get_gco(ctxt, o);
    OX_Frame *frame;
    OX_Result r;

    if (!(frame = ox_frame_push(ctxt, o, f->sfunc->decl_hash.e_num))) {
        r = OX_ERR;
    } else {
        r = ox_function_call(ctxt, o, thiz, args, argc, rv, NULL);

        ox_frame_pop(ctxt);
    }

    return r;
}

/*Operation functions of the function.*/
static const OX_ObjectOps
function_ops = {
    {
        OX_GCO_FUNCTION,
        function_scan,
        function_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    function_call
};

/*Scan referenced objects in the native function.*/
static void
native_func_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_NativeFunc *nf = (OX_NativeFunc*)gco;

    ox_object_scan(ctxt, gco);

    if (nf->script)
        ox_gc_mark(ctxt, nf->script);
}

/*Free the native function.*/
static void
native_func_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_NativeFunc *nf = (OX_NativeFunc*)gco;

    ox_object_deinit(ctxt, &nf->o);

    OX_DEL(ctxt, nf);
}

/*Call the native function.*/
static OX_Result
native_func_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_NativeFunc *nf = ox_value_get_gco(ctxt, o);
    OX_Frame *frame;
    OX_Result r;

    if (!(frame = ox_frame_push(ctxt, o, 0))) {
        r = OX_ERR;
    } else {
        ox_value_set_null(ctxt, rv);

        r = nf->cf(ctxt, o, thiz, args, argc, rv);

        ox_frame_pop(ctxt);
    }

    return r;
}

/*Native function's operation functions.*/
static const OX_ObjectOps
native_func_ops = {
    {
        OX_GCO_NATIVE_FUNC,
        native_func_scan,
        native_func_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    native_func_call
};

/**
 * Create a new native function.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_native_func_new (OX_Context *ctxt, OX_Value *f, OX_CFunc cf)
{
    OX_NativeFunc *nf;

    assert(ctxt && f && cf);

    if (!OX_NEW(ctxt, nf))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &nf->o, OX_OBJECT(ctxt, Function_inf));
    nf->o.gco.ops = (OX_GcObjectOps*)&native_func_ops;
    nf->cf = cf;
    nf->script = ctxt->curr_script;

    ox_value_set_gco(ctxt, f, nf);
    ox_gc_add(ctxt, nf);

    return OX_OK;
}

/**
 * Get the script contains the native function.
 * @param ctxt The current running context.
 * @param f The native function.
 * @param[out] s Return the native script contains the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_native_func_get_script (OX_Context *ctxt, OX_Value *f, OX_Value *s)
{
    OX_NativeFunc *nf;

    assert(ctxt && f);
    assert(ox_value_is_gco(ctxt, f, OX_GCO_NATIVE_FUNC));

    nf = ox_value_get_gco(ctxt, f);

    ox_value_set_gco(ctxt, s, nf->script);

    return OX_OK;
}

/**
 * Create a new native function with scope and name.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @param scope The scope object.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_native_func_new (OX_Context *ctxt, OX_Value *f, OX_CFunc cf,
        OX_Value *scope, OX_Value *name)
{
    OX_Result r;

    if ((r = ox_native_func_new(ctxt, f, cf)) == OX_ERR)
        return r;

    if (scope && !ox_value_is_null(ctxt, scope))
        if ((r = ox_object_set_scope(ctxt, f, scope)) == OX_ERR)
            return r;

    if (name && !ox_value_is_null(ctxt, name))
        if ((r = ox_object_set_name(ctxt, f, name)) == OX_ERR)
            return r;

    return OX_OK;
}

/**
 * Create a new native function with scope and name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @param scope The scope object.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_native_func_new_s (OX_Context *ctxt, OX_Value *f, OX_CFunc cf,
        OX_Value *scope, const char *name)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    assert(name);

    r = ox_string_from_const_char_star(ctxt, nv, name);
    if (r == OX_OK)
        r = ox_named_native_func_new(ctxt, f, cf, scope, nv);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Create a new function.
 * @param ctxt The current running context.
 * @param[out] f Return the new function.
 * @param sfunc The script function use this frame.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_function_new (OX_Context *ctxt, OX_Value *f, OX_ScriptFunc *sfunc)
{
    OX_Function *fp;

    assert(ctxt && f && sfunc);

    if (!OX_NEW(ctxt, fp))
        return ox_throw_no_mem_error(ctxt);

    if (sfunc->frame_num) {
        OX_Frame **pf, *fr;

        if (!OX_NEW_N_0(ctxt, fp->frames, sfunc->frame_num)) {
            OX_DEL(ctxt, fp);
            return ox_throw_no_mem_error(ctxt);
        }

        /*Copy the frames.*/
        fr = ox_frame_get(ctxt);
        assert(fr);

        pf = fp->frames;

        *pf++ = fr;

        if (sfunc->frame_num > 1) {
            OX_Function *bf;
            size_t n = sfunc->frame_num - 1;

            assert(ox_value_get_gco_type(ctxt, &fr->func) == OX_GCO_FUNCTION);

            bf = ox_value_get_gco(ctxt, &fr->func);

            assert(bf->sfunc->frame_num >= n);

            memcpy(pf, bf->frames, sizeof(OX_Frame*) * n);
        }
    } else {
        fp->frames = NULL;
    }

    ox_object_init(ctxt, &fp->o, OX_OBJECT(ctxt, Function_inf));
    fp->o.gco.ops = (OX_GcObjectOps*)&function_ops;
    fp->sfunc = sfunc;

    ox_value_set_gco(ctxt, f, fp);
    ox_gc_add(ctxt, fp);

    return OX_OK;
}

/**
 * Lookup the function's source code's line number.
 * @param ctxt The current running context.
 * @param f The function.
 * @param ip The instruction pointer.
 * @return The line number.
 * @retval -1 Cannot find the line.
 */
int
ox_function_lookup_line (OX_Context *ctxt, OX_Value *f, int ip)
{
    OX_Function *fp;
    OX_ScriptFunc *sf;

    assert(ctxt && f);
    assert(ox_value_is_gco(ctxt, f, OX_GCO_FUNCTION));

    fp = ox_value_get_gco(ctxt, f);
    sf = fp->sfunc;

    return ox_script_func_lookup_line(ctxt, sf, ip);
}

/*Function.is*/
static OX_Result
Function_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_function(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Function.$inf.call*/
static OX_Result
Function_inf_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *c_this = ox_argument(ctxt, args, argc, 0);
    OX_Value *c_args;
    size_t c_argc;

    if (argc > 1) {
        c_args = ox_values_item(ctxt, args, 1);
        c_argc = argc - 1;
    } else {
        c_args = NULL;
        c_argc = 0;
    }

    return ox_call(ctxt, thiz, c_this, c_args, c_argc, rv);
}

/*Function.$inf.$to_str*/
static OX_Result
Function_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, n)
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_char_buffer_append_char_star(ctxt, &cb, "Function")) == OX_ERR)
        goto end;

    if ((r = ox_get_full_name(ctxt, thiz, n)) == OX_ERR)
        goto end;

    if (ox_string_length(ctxt, n)) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, ' ')) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, n)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, n)
    return r;
}

/*Function.$inf.to_c*/
static OX_Result
Function_inf_to_c (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *cty = ox_argument(ctxt, args, argc, 0);
    void *p;
    OX_Result r;

    if (!ox_value_is_function(ctxt, thiz)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a function"));
        goto end;
    }

    if (!ox_value_is_ctype(ctxt, cty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C type"));
        goto end;
    }

    if (ox_ctype_get_kind(ctxt, cty) != OX_CTYPE_FUNC) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C function type"));
        goto end;
    }

    if ((r = ox_function_get_cptr(ctxt, thiz, cty, &p)) == OX_ERR)
        goto end;

    ox_value_copy(ctxt, rv, thiz);
    r = OX_OK;
end:
    return r;
}

/*?
 *? @lib {Function} Function.
 *?
 *? @class{ Function Function class.
 *?
 *? @func call Call the function.
 *? @param this This argument of the functon.
 *? @param ...args The arguments of the function.
 *? @return The result of the function.
 *?
 *? @func $to_str Convert the function to string.
 *? The string's format is "Function NAME".
 *? @return {String} The result string.
 *?
 *? @func to_c Convert an OX function to a C function.
 *? The result C function pointer can be used in external C libraries.
 *? @return {C:void*} The C function's pointer.
 *?
 *? @class}
 */

/**
 * Initialize the function class.
 * @param ctxt The current running context.
 */
void
ox_func_class_init (OX_Context *ctxt)
{
    /*Function.*/
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Function), OX_OBJECT(ctxt, Function_inf), NULL, "Function"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Function", OX_OBJECT(ctxt, Function)));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Function), "is", Function_is));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Function_inf), "call", Function_inf_call));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Function_inf), "$to_str", Function_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Function_inf), "to_c", Function_inf_to_c));
}

/**
 * Call the function class.
 * @param ctxt The current running context.
 * @param o The function class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_function_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_4(ctxt, src, input, ast, script)
    OX_Script *sp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, src)) == OX_ERR)
        goto end;

    if ((r = ox_string_input_new(ctxt, input, src)) == OX_ERR)
        goto end;

    if ((r = ox_parse(ctxt, input, ast, OX_PARSE_FL_RETURN)) == OX_ERR)
        goto end;

    ox_input_close(ctxt, input);

    if ((r = ox_compile(ctxt, input, ast, script, 0)) == OX_ERR)
        goto end;

    if ((r = ox_bc_script_set_base(ctxt, script)) == OX_ERR)
        goto end;

    if ((r = ox_script_init(ctxt, script)) == OX_ERR)
        goto end;

    sp = ox_value_get_gco(ctxt, script);
    ox_value_copy(ctxt, rv, &sp->func);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, src)
    return r;
}
