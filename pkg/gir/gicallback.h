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
 * GICallbackInfo.
 */

/** GI callback.*/
typedef struct {
    OX_HashEntry    he;      /**< Hash table entry.*/
    OX_Value        v;       /**< The value of the callback.*/
    OX_VM          *vm;      /**< The virtual machine.*/
    OX_GICallable   c;       /**< Callable data.*/
    ffi_cif         cif;     /**< FFI CIF data.*/
    ffi_type      **atypes;  /**< Argument types.*/
    ffi_closure    *closure; /**< Closure.*/
    GICallableInfo *ci;      /**< Callable information.*/
} OX_GICallback;

/*Scan referenced objects in the callback.*/
static void
gicallback_scan (OX_Context *ctxt, void *p)
{
    OX_GICallback *cb = p;

    gicallable_scan(ctxt, &cb->c);
}

/*Free the callback.*/
static void
gicallback_free (OX_Context *ctxt, void *p)
{
    OX_GICallback *cb = p;

    //OX_LOG_D(ctxt, "callback free %p", cb);

    if (cb->he.key)
        ox_hash_remove(ctxt, &cb->c.repo->cb_hash, cb->he.key, NULL);

    if (cb->atypes) {
        int n_args = cb->c.n_args;

        if (cb->c.inst_bi)
            n_args ++;

        OX_DEL_N(ctxt, cb->atypes, n_args);
    }

    if (cb->closure)
        ffi_closure_free(cb->closure);

    gicallable_deinit(ctxt, &cb->c);

    gi_base_info_unref(cb->ci);

    OX_DEL(ctxt, cb);
}

/*Operation functions of GI callback.*/
static const OX_PrivateOps
gicallback_ops = {
    gicallback_scan,
    gicallback_free
};

/*Call the callback OX function.*/
static void
gicallback_call (ffi_cif *cif, void *rvalue, void **avalues, void *userdata)
{
    OX_GICallback *cb = userdata;
    OX_Context *ctxt = ox_context_get(cb->vm);
    OX_Value *argsv, *rv, *ev, *sv;

    ox_lock(ctxt);
    {
        GIArgument in_args[cb->c.n_in_args];
        GIArgument out_args[cb->c.n_out_args];
        OX_GIOwn in_owns[cb->c.n_in_args];
        OX_GIOwn out_owns[cb->c.n_out_args];
        OX_GICallState cs;
        OX_Result r;

        OX_LOG_D(ctxt, "callback \"%s\"", gi_base_info_get_name(GI_BASE_INFO(cb->ci)));
        OX_LOG_D(ctxt, "callback ptr: %p", cb);

        gicallstate_init(&cs, &cb->c, in_args, out_args, in_owns, out_owns);

        argsv = ox_value_stack_push_n(ctxt, cb->c.n_args + 3);
        rv = ox_values_item(ctxt, argsv, cb->c.n_args);
        ev = ox_values_item(ctxt, argsv, cb->c.n_args + 1);
        sv = ox_values_item(ctxt, argsv, cb->c.n_args + 2);

        if ((r = gicallstate_input_c(ctxt, &cs, avalues, argsv)) == OX_ERR)
            goto end;

        r = ox_call(ctxt, &cb->v, ox_value_null(ctxt), argsv, cb->c.n_in_args, rv);
        if (r == OX_OK) {
            if ((r = gicallstate_output_c(ctxt, &cs, avalues, rvalue, rv)) == OX_ERR)
                goto end;
        }

end:
        if (r == OX_ERR) {
            ox_catch(ctxt, ev);

            if (!ox_value_is_null(ctxt, ev)) {
                if (ox_to_string(ctxt, ev, sv) == OX_OK) {
                    char *col = isatty(2) ? "\033[31;1m" : NULL;

                    fprintf(stderr,
                            OX_TEXT("%suncaught error%s: %s\n"),
                            col ? col : "",
                            col ? "\033[0m" : "",
                            ox_string_get_char_star(ctxt, sv));
                    ox_stack_dump(ctxt, stderr);
                }
            }
        }

        gicallstate_deinit(ctxt, &cs);
        ox_value_stack_pop(ctxt, argsv);
    }
    ox_unlock(ctxt);
}

/*Create a new callback.*/
static OX_Result
gicallback_from_value (OX_Context *ctxt, OX_GIRepository *repo, OX_Value *v, GICallableInfo *ci, void **pp)
{
    OX_GICallback *cb = NULL;
    ffi_type *rtype = NULL;
    ffi_status status;
    int n_args;
    void *p;
    OX_Result r;

    cb = ox_object_get_priv(ctxt, v, &gicallback_ops);
    if (cb) {
        *pp = cb->he.key;
        r = OX_OK;
        goto end;
    }

    if (!OX_NEW_0(ctxt, cb)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    cb->ci = GI_CALLABLE_INFO(gi_base_info_ref(ci));
    cb->vm = ox_vm_get(ctxt);
    ox_value_copy(ctxt, &cb->v, v);

    if ((r = gicallable_init(ctxt, repo, &cb->c, ci, C_CALL_OX)) == OX_ERR)
        goto end;

    n_args = cb->c.n_args;

    if (cb->c.inst_bi)
        n_args ++;

    if (n_args) {
        ffi_type **ftype;
        int i;

        if (!OX_NEW_N_0(ctxt, cb->atypes, n_args)) {
            r = ox_throw_no_mem_error(ctxt);
            goto end;
        }

        ftype = cb->atypes;

        if (cb->c.inst_bi) {
            *ftype = &ffi_type_pointer;
            ftype ++;
        }

        for (i = 0; i < cb->c.n_args; i ++) {
            OX_GIArgType *atype = &cb->c.atypes[i];

            *ftype = gi_type_info_get_ffi_type(atype->ti);
            ftype ++;
        }
    }

    if (cb->c.ret_ti)
        rtype = gi_type_info_get_ffi_type(cb->c.ret_ti);
    else
        rtype = &ffi_type_void;

    status = ffi_prep_cif(&cb->cif, FFI_DEFAULT_ABI, n_args, rtype, cb->atypes);
    if (status != FFI_OK) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed"),
                "ffi_prep_cif");
        goto end;
    }

    if (!(cb->closure = ffi_closure_alloc(sizeof(ffi_closure), &p))) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed"),
                "ffi_closure_alloc");
        goto end;
    }

    status = ffi_prep_closure_loc(cb->closure, &cb->cif, gicallback_call, cb, NULL);
    if (status != FFI_OK) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed"),
                "ffi_prep_closure_loc");
        goto end;
    }

    if ((r = ox_object_set_priv(ctxt, v, &gicallback_ops, cb)) == OX_ERR)
        goto end;

    if ((r = ox_hash_insert(ctxt, &repo->cb_hash, p, &cb->he, NULL)) == OX_ERR)
        goto end;

    *pp = p;
    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (cb)
            gicallback_free(ctxt, cb);
    }
    return r;
}

/*Convert callback to OX value.*/
static OX_Result
gicallback_to_value (OX_Context *ctxt, OX_GIRepository *repo, void *p, GICallableInfo *ci, OX_Value *v)
{
    OX_GICallback *cb;
    OX_Result r;

    cb = ox_hash_lookup_c(ctxt, &repo->cb_hash, p, NULL, OX_GICallback, he);
    if (!cb) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a GLib callback"));
        goto end;
    }

    ox_value_copy(ctxt, v, &cb->v);
    r = OX_OK;
end:
    return r;
}