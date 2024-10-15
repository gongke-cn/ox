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
 * GIFunctionInfo.
 */

/*GI function.*/
typedef struct {
    GIFunctionInfo  *fi;   /**< Function information.*/
    OX_GICallable    c;    /**< Callable data.*/
} OX_GIFunction;

/*Scan reference objects in the GI function.*/
static void
gifunction_scan (OX_Context *ctxt, void *p)
{
    OX_GIFunction *f = p;

    gicallable_scan(ctxt, &f->c);
}

/*Free the GI function.*/
static void
gifunction_free (OX_Context *ctxt, void *p)
{
    OX_GIFunction *f = p;

    gicallable_deinit(ctxt, &f->c);
    gi_base_info_unref(f->fi);

    OX_DEL(ctxt, f);
}

/*Operation functions of GI function.*/
static const OX_PrivateOps
gifunction_ops = {
    gifunction_scan,
    gifunction_free
};

/*Get GI function from the value.*/
static OX_GIFunction*
gifunction_get (OX_Context *ctxt, OX_Value *v)
{
    OX_GIFunction *fn = ox_object_get_priv(ctxt, v, &gifunction_ops);

    if (!fn)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a GLib function"));

    return fn;
}

/*Invoke the function.*/
static OX_Result
gifunctioninfo_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GIFunction *fn = gifunction_get(ctxt, f);
    GIArgument in_args[fn->c.n_in_args];
    OX_GIOwn in_owns[fn->c.n_in_args];
    GIArgument out_args[fn->c.n_out_args];
    OX_GIOwn out_owns[fn->c.n_out_args];
    OX_VS_PUSH(ctxt, ev)
    OX_GICallState cs;
    OX_Result r;

#if 0
    GIBaseInfo *ci;
    const char *cn = "";

    ci = gi_base_info_get_container(GI_BASE_INFO(fn->fi));
    if (ci)
        cn = gi_base_info_get_name(ci);

    OX_LOG_D(ctxt, "call %s \"%s\"", cn, gi_base_info_get_name(GI_BASE_INFO(fn->fi)));
#endif

    gicallstate_init(&cs, &fn->c, in_args, out_args, in_owns, out_owns);

    if ((r = gicallstate_input_ox(ctxt, &cs, thiz, args, argc)) == OX_ERR)
        goto end;

    if (!gi_function_info_invoke(fn->fi, in_args, fn->c.n_in_args, out_args, fn->c.n_out_args, &cs.ret_arg, &cs.error)) {
        if (cs.error) {
            gerror_to_value(ctxt, cs.error, ev, &fn->c.repo->script);
            r = ox_throw(ctxt, ev);
        } else {
            r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed"),
                    "gi_function_info_invoke");
        }
        goto end;
    }

    if ((r = gicallstate_output_ox(ctxt, &cs, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    gicallstate_deinit(ctxt, &cs);
    OX_VS_POP(ctxt, ev)
    return r;
}

/*Convert the function information to value.*/
static OX_Result
gifunctioninfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIFunctionInfo *fi, OX_Value *v)
{
    OX_GIFunction *fn = NULL;
    OX_Result r;

    if ((r = ox_native_func_new(ctxt, v, gifunctioninfo_call)) == OX_ERR)
        goto end;

    if (!OX_NEW_0(ctxt, fn)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    fn->fi = GI_FUNCTION_INFO(gi_base_info_ref(fi));

    if ((r = gicallable_init(ctxt, repo, &fn->c, GI_CALLABLE_INFO(fi), OX_CALL_C)) == OX_ERR)
        goto end;

    if ((r = ox_object_set_priv(ctxt, v, &gifunction_ops, fn)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
   if (r == OX_ERR) {
        if (fn)
            gifunction_free(ctxt, fn);
    }
    return r;
}
