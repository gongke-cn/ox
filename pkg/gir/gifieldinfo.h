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
 * GIFieldInfo
 */

/** Field information.*/
typedef struct {
    int              ref;   /**< Reference counter.*/
    OX_GIRepository *repo;  /**< Repository.*/
    OX_Value         cty;   /**< The container's type.*/
    GITypeInfo      *ti;    /**< The type information of the field.*/
    GIFieldInfo     *field; /**< The field.*/
} OX_GIFieldInfo;

/*Scan reference objects in the field information.*/
static void
gifieldinfo_scan (OX_Context *ctxt, void *p)
{
    OX_GIFieldInfo *f = p;

    ox_gc_scan_value(ctxt, &f->repo->v);
    ox_gc_scan_value(ctxt, &f->cty);
}

/*Free the field information.*/
static void
gifieldinfo_free (OX_Context *ctxt, void *p)
{
    OX_GIFieldInfo *f = p;

    f->ref --;
    if (f->ref == 0) {
        gi_base_info_unref(f->field);
        gi_base_info_unref(f->ti);
        OX_DEL(ctxt, f);
    }
}

/*Operation functions of field information.*/
static const OX_PrivateOps
gifieldinfo_ops = {
    gifieldinfo_scan,
    gifieldinfo_free
};

/*Field getter.*/
static OX_Result
gifieldinfo_get_fn (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GIFieldInfo *field = ox_object_get_priv(ctxt, f, &gifieldinfo_ops);
    void *p;
    GIArgument arg;
    OX_GIOwn own = OX_GI_OWN_NOTHING;
    OX_GICtxt gic;
    OX_Result r;

    gictxt_init(&gic, field->repo, NULL, NULL, NULL);

    if (!ox_value_is_cptr(ctxt, thiz, &field->cty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not \"%s\""),
                gitype_get_name(ctxt, &field->cty));
        goto end;
    }

    p = ox_cvalue_get_pointer(ctxt, thiz);

    if (!gi_field_info_get_field(field->field, p, &arg)) {
        r = ox_throw_system_error(ctxt, OX_TEXT("get field \"%s\" failed"),
                gi_base_info_get_name(GI_BASE_INFO(field->field)));
        goto end;
    }

    if ((r = giargument_to_value(ctxt, &gic, &arg, &own, field->ti, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    giargument_free(ctxt, &gic, &arg, own, field->ti);
    return r;
}

/*Field setter.*/
static OX_Result
gifieldinfo_set_fn (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *farg = ox_argument(ctxt, args, argc, 0);
    OX_GIFieldInfo *field = ox_object_get_priv(ctxt, f, &gifieldinfo_ops);
    void *p;
    GIArgument arg;
    OX_GIOwn own = OX_GI_OWN_NOTHING;
    OX_GICtxt gic;
    OX_Result r;

    gictxt_init(&gic, field->repo, NULL, NULL, NULL);

    if (!ox_value_is_cptr(ctxt, thiz, &field->cty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not \"%s\""),
                gitype_get_name(ctxt, &field->cty));
        goto end;
    }

    if ((r = giargument_from_value(ctxt, &gic, &arg, &own, field->ti, farg)) == OX_ERR)
        goto end;

    p = ox_cvalue_get_pointer(ctxt, thiz);

    if (!gi_field_info_set_field(field->field, p, &arg)) {
        r = ox_throw_system_error(ctxt, OX_TEXT("set field \"%s\" failed"),
                gi_base_info_get_name(GI_BASE_INFO(field->field)));
        goto end;
    }

    r = OX_OK;
end:
    giargument_free(ctxt, &gic, &arg, own, field->ti);
    return r;
}

/*Load the field information.*/
static OX_Result
gifieldinfo_load (OX_Context *ctxt, OX_GIRepository *repo, GIFieldInfo *fi, OX_Value *cty, OX_Value *inf)
{
    OX_VS_PUSH_2(ctxt, get, set)
    GIFieldInfoFlags flags = gi_field_info_get_flags(fi);
    const char *ncstr = gi_base_info_get_name(GI_BASE_INFO(fi));
    OX_GIFieldInfo *field = NULL;
    OX_Result r;

    if (!OX_NEW(ctxt, field)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    field->ref = 1;
    field->repo = repo;
    field->field = GI_FIELD_INFO(gi_base_info_ref(fi));
    field->ti = gi_field_info_get_type_info(fi);
    ox_value_copy(ctxt, &field->cty, cty);

    if (flags & GI_FIELD_IS_READABLE) {
        if ((r = ox_native_func_new(ctxt, get, gifieldinfo_get_fn)) == OX_ERR)
            goto end;
        if ((r = ox_object_set_priv(ctxt, get, &gifieldinfo_ops, field)) == OX_ERR)
            goto end;
        field->ref ++;
    }

    if (flags & GI_FIELD_IS_WRITABLE) {
        if ((r = ox_native_func_new(ctxt, set, gifieldinfo_set_fn)) == OX_ERR)
            goto end;
        if ((r = ox_object_set_priv(ctxt, set, &gifieldinfo_ops, field)) == OX_ERR)
            goto end;
        field->ref ++;
    }

    r = ox_object_add_accessor_s(ctxt, inf, ncstr, get, set);
end:
    if (field)
        gifieldinfo_free(ctxt, field);
    OX_VS_POP(ctxt, get)
    return r;
}
