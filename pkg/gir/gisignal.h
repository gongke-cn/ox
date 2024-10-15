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
 * GISignalInfo.
 */

/** GI signal handler.*/
typedef struct {
    gpointer inst; /**< The instance pointer.*/
    gulong   id;   /**< ID of the handler.*/
} OX_GISignalHandler;

/*Free the signal handler.*/
static void
gisignalhandler_free (OX_Context *ctxt, void *p)
{
    OX_GISignalHandler *sh = p;

    OX_DEL(ctxt, sh);
}

/*Operation functions of signal handler.*/
static const OX_PrivateOps
gisignalhandler_ops = {
    NULL,
    gisignalhandler_free
};

/*Get GI signal handler from the value.*/
static OX_GISignalHandler*
gisignalhandler_get (OX_Context *ctxt, OX_Value *v)
{
    OX_GISignalHandler *sh = ox_object_get_priv(ctxt, v, &gisignalhandler_ops);

    if (!sh)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a signal handler"));

    return sh;
}

/*Create a new signal handler object.*/
static OX_Result
gisignalhandler_new (OX_Context *ctxt, OX_GIRepository *repo, OX_Value *v, gpointer inst, gulong id)
{
    OX_Value *inf = ox_script_get_value(ctxt, &repo->script, ID_GSignalHandler_inf);
    OX_GISignalHandler *sh;
    OX_Result r;

    if ((r = ox_object_new(ctxt, v, inf)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, sh))
        return ox_throw_no_mem_error(ctxt);

    sh->inst = inst;
    sh->id = id;

    if ((r = ox_object_set_priv(ctxt, v, &gisignalhandler_ops, sh)) == OX_ERR) {
        gisignalhandler_free(ctxt, sh);
        return r;
    }

    return OX_OK;
}

/*Scan reference*/
static void
gisignalfn_scan (OX_Context *ctxt, void *p)
{
    OX_GIRepository *repo = p;

    ox_gc_scan_value(ctxt, &repo->v);
}

/*Operation functions of signal connect function.*/
static const OX_PrivateOps
gisignalfn_ops = {
    gisignalfn_scan,
    NULL
};

/*Add a signal to the object.*/
static OX_Result
gisignal_add (OX_Context *ctxt, OX_Value *o, GISignalInfo *si)
{
    OX_GIType *ty = gitype_get(ctxt, o);
    OX_GISignal *s;
    OX_HashEntry **pe;
    const char *name = gi_base_info_get_name(GI_BASE_INFO(si));
    OX_Result r;

    s = ox_hash_lookup_c(ctxt, &ty->signal_hash, (void*)name, &pe, OX_GISignal, he);
    if (s) {
        gi_base_info_unref(s->si);
    } else {
        if (!OX_NEW(ctxt, s))
            return ox_throw_no_mem_error(ctxt);

        if ((r = ox_hash_insert(ctxt, &ty->signal_hash, (void*)name, &s->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, s);
            return r;
        }
    }

    s->si = GI_SIGNAL_INFO(gi_base_info_ref(si));

    return OX_OK;
}

/*Inherit the parent's signals.*/
static OX_Result
gisignal_inherit (OX_Context *ctxt, OX_Value *o, OX_Value *po)
{
    OX_GIType *pty = gitype_get(ctxt, po);
    OX_GISignal *s;
    size_t i;
    OX_Result r;

    ox_hash_foreach_c(&pty->signal_hash, i, s, OX_GISignal, he) {
        if ((r = gisignal_add(ctxt, o, s->si)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Connect the signal.*/
static OX_Result
gisignal_connect (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GIInst *inst;
    OX_GIRepository *repo = ox_object_get_priv(ctxt, f, &gisignalfn_ops);
    OX_Value *sn_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *fn = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, sn_str)
    const char *sn;
    OX_GISignal *sig;
    void *p;
    gulong sh_id;
    OX_Result r;

    if (!(inst = giinst_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (!GI_IS_OBJECT_INFO(inst->type->rti)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not an GObject"));
        goto end;
    }

    if ((r = ox_to_string(ctxt, sn_arg, sn_str)) == OX_ERR)
        goto end;
    sn = ox_string_get_char_star(ctxt, sn_str);

    if (!ox_value_is_function(ctxt, fn)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a function"));
        goto end;
    }

    sig = ox_hash_lookup_c(ctxt, &inst->type->signal_hash, (void*)sn, NULL, OX_GISignal, he);
    if (!sig) {
        r = ox_throw_reference_error(ctxt, OX_TEXT("signal \"%s\" is not defined"), sn);
        goto end;
    }

    if ((r = gicallback_from_value(ctxt, repo, fn, GI_CALLABLE_INFO(sig->si), &p)) == OX_ERR)
        goto end;

    sh_id = g_signal_connect(inst->he.key, sn, p, NULL);

    r = gisignalhandler_new(ctxt, repo, rv, inst->he.key, sh_id);
end:
    OX_VS_POP(ctxt, sn_str)
    return r;
}

/*Add signal methods.*/
static OX_Result
gisignal_add_methods (OX_Context *ctxt, OX_GIRepository *repo, OX_Value *c, OX_Value *inf)
{
    OX_VS_PUSH(ctxt, f)
    OX_Result r;

    if ((r = ox_named_native_func_new_s(ctxt, f, gisignal_connect, inf, "$connect")) == OX_ERR)
        goto end;

    if ((r = ox_object_add_const_s(ctxt, inf, "$connect", f)) == OX_ERR)
        goto end;

    if ((r = ox_object_set_priv(ctxt, f, &gisignalfn_ops, repo)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, f)
    return r;
}

/*GSignalHandler.$inf.disconnect.*/
static OX_Result
GSignalHandler_inf_disconnect (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GISignalHandler *sh;

    if (!(sh = gisignalhandler_get(ctxt, thiz)))
        return OX_ERR;

    g_signal_handler_disconnect(sh->inst, sh->id);
    return OX_OK;
}

/*GSignalHandler.$inf.block.*/
static OX_Result
GSignalHandler_inf_block (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GISignalHandler *sh;

    if (!(sh = gisignalhandler_get(ctxt, thiz)))
        return OX_ERR;

    g_signal_handler_block(sh->inst, sh->id);
    return OX_OK;
}

/*GSignalHandler.$inf.unblock.*/
static OX_Result
GSignalHandler_inf_unblock (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GISignalHandler *sh;

    if (!(sh = gisignalhandler_get(ctxt, thiz)))
        return OX_ERR;

    g_signal_handler_unblock(sh->inst, sh->id);
    return OX_OK;
}

/*GSignalHandler.$inf.is_connected get.*/
static OX_Result
GSignalHandler_inf_is_connected_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GISignalHandler *sh;
    gboolean b;

    if (!(sh = gisignalhandler_get(ctxt, thiz)))
        return OX_ERR;

    b = g_signal_handler_is_connected(sh->inst, sh->id);

    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Initialize the signal handler class.*/
static void
gisignalhandler_class_init (OX_Context *ctxt, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "GSinalHandler"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_GSignalHandler_inf, inf));

    /*GSignalHandler.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "disconnect", GSignalHandler_inf_disconnect));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "block", GSignalHandler_inf_block));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "unblock", GSignalHandler_inf_unblock));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "is_connected", GSignalHandler_inf_is_connected_get, NULL));

    OX_VS_POP(ctxt, c)
}