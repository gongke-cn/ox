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
 * GIVFuncInfo.
 */

/*GI virtual function.*/
typedef struct {
    GIVFuncInfo   *vfi; /**< Virtual function information.*/
    OX_GICallable  c;   /**< Callable data.*/
} OX_GIVFunc;

/*Scan referenced objects in the virtual function.*/
static void
givfunc_scan (OX_Context *ctxt, void *p)
{
    OX_GIVFunc *vf = p;

    gicallable_scan(ctxt, &vf->c);
}

/*Free the virtual function.*/
static void
givfunc_free (OX_Context *ctxt, void *p)
{
    OX_GIVFunc *vf = p;

    gicallable_deinit(ctxt, &vf->c);

    gi_base_info_unref(vf->vfi);

    OX_DEL(ctxt, vf);
}

/*Operation functions of virtual function.*/
static const OX_PrivateOps
givfunc_ops = {
    givfunc_scan,
    givfunc_free
};

/*Invoke the virtual function.*/
static OX_Result
givfunc_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GIVFunc *fn = ox_object_get_priv(ctxt, f, &givfunc_ops);
    GIArgument in_args[fn->c.n_in_args];
    OX_GIOwn in_owns[fn->c.n_in_args];
    GIArgument out_args[fn->c.n_out_args];
    OX_GIOwn out_owns[fn->c.n_out_args];
    OX_VS_PUSH(ctxt, ev)
    OX_GIInst *inst;
    OX_GICallState cs;
    GType gty;
    OX_Result r;

#if 0
    GIBaseInfo *ci;
    const char *cn = "";

    ci = gi_base_info_get_container(GI_BASE_INFO(fn->vfi));
    if (ci)
        cn = gi_base_info_get_name(ci);

    OX_LOG_D(ctxt, "call %s \"%s\"", cn, gi_base_info_get_name(GI_BASE_INFO(fn->vfi)));
#endif

    if (!(inst = giinst_get(ctxt, thiz))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a GObject instance"));
        goto end;
    }
    gty = G_TYPE_FROM_INSTANCE(inst->he.key);

    gicallstate_init(&cs, &fn->c, in_args, out_args, in_owns, out_owns);

    if ((r = gicallstate_input_ox(ctxt, &cs, thiz, args, argc)) == OX_ERR)
        goto end;

    if (!gi_vfunc_info_invoke(fn->vfi, gty, in_args, fn->c.n_in_args, out_args, fn->c.n_out_args, &cs.ret_arg, &cs.error)) {
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

/*Convert the GVFuncInfo to OX value.*/
static OX_Result
givfuncinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIVFuncInfo *vfi, OX_Value *v)
{
    OX_GIVFunc *vf = NULL;
    OX_Result r;

    if ((r = ox_native_func_new(ctxt, v, givfunc_call)) == OX_ERR)
        goto end;

    if (!OX_NEW_0(ctxt, vf)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    vf->vfi = GI_VFUNC_INFO(gi_base_info_ref(vfi));

    if ((r = gicallable_init(ctxt, repo, &vf->c, GI_CALLABLE_INFO(vfi), OX_CALL_C)) == OX_ERR)
        goto end;

    if ((r = ox_object_set_priv(ctxt, v, &givfunc_ops, vf)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (vf)
            givfunc_free(ctxt, vf);
    }
    return r;
}
