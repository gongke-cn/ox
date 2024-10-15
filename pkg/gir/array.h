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
 * Array.
 */

/*Get the length of the array.*/
static size_t
array_get_length (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti)
{
    size_t len;
    unsigned int idx;

    if (gi_type_info_get_array_fixed_size(ti, &len))
        return len;

    if (gi_type_info_get_array_length_index(ti, &idx)) {
        OX_GIArgType *atype;
        GIArgument *arg;
        GITypeInfo *ti;

        if (!gic->atypes)
            return SIZE_MAX;


        atype = &gic->atypes[idx];
        ti = atype->ti;
        arg = (atype->dir != GI_DIRECTION_OUT)
                ? &gic->in_args[atype->in_arg_idx]
                : &gic->out_args[atype->out_arg_idx];

        arg = &gic->in_args[atype->in_arg_idx];

        switch (gi_type_info_get_tag(ti)) {
        case GI_TYPE_TAG_INT8:
            return arg->v_int8;
        case GI_TYPE_TAG_UINT8:
            return arg->v_uint8;
        case GI_TYPE_TAG_INT16:
            return arg->v_int16;
        case GI_TYPE_TAG_UINT16:
            return arg->v_uint16;
        case GI_TYPE_TAG_INT32:
            return arg->v_int32;
        case GI_TYPE_TAG_UINT32:
            return arg->v_uint32;
        case GI_TYPE_TAG_INT64:
            return arg->v_int64;
        case GI_TYPE_TAG_UINT64:
            return arg->v_uint64;
        case GI_TYPE_TAG_GTYPE:
            return arg->v_size;
        case GI_TYPE_TAG_FLOAT:
            return arg->v_float;
        case GI_TYPE_TAG_DOUBLE:
            return arg->v_double;
        default:
            break;
        }
    }

    return SIZE_MAX;
}

/*Check if the array's items should be clean.*/
static OX_Bool
array_elements_need_clean (GITypeInfo *ti)
{
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    OX_Bool b = gi_type_info_is_pointer(eti)
            && !gitype_is_struct_union(eti)
            && !gitype_is_callback(eti);

    gi_base_info_unref(eti);

    return b;
}

/*Free items in the CArray.*/
static void
carray_free_items (OX_Context *ctxt, OX_GICtxt *gic, void *a, GITypeInfo *ti, size_t len)
{
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);

    if (len == SIZE_MAX)
        len = array_get_length(ctxt, gic, ti);

    if ((esize != SIZE_MAX) && (len != SIZE_MAX)) {
        char *p = a;
        size_t i;

        for (i = 0; i < len; p += esize, i ++) {
            GIArgument arg;

            arg.v_pointer = *(void**)p;

            giargument_free(ctxt, gic, &arg, OX_GI_OWN_EVERYTHING, eti);
        }
    }

    gi_base_info_unref(eti);
}

/*Free items in the GArray.*/
static void
garray_free_items (OX_Context *ctxt, OX_GICtxt *gic, GArray *a, GITypeInfo *ti)
{
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    char *p = a->data;
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    size_t len = a->len;
    size_t i;

    for (i = 0; i < len; p += esize, i ++) {
        GIArgument arg;

        arg.v_pointer = *(void**)p;

        giargument_free(ctxt, gic, &arg, OX_GI_OWN_EVERYTHING, eti);
    }

    gi_base_info_unref(eti);
}

/*Free the array.*/
static void
array_free (OX_Context *ctxt, OX_GICtxt *gic, void *a, OX_GIOwn own, GITypeInfo *ti)
{
    switch (gi_type_info_get_array_type(ti)) {
    case GI_ARRAY_TYPE_C:
        if ((own == OX_GI_OWN_EVERYTHING) && array_elements_need_clean(ti))
            carray_free_items(ctxt, gic, a, ti, SIZE_MAX);
        g_free(a);
        break;
    case GI_ARRAY_TYPE_ARRAY:
        if ((own == OX_GI_OWN_EVERYTHING) && array_elements_need_clean(ti))
            garray_free_items(ctxt, gic, a, ti);
        g_array_unref(a);
        break;
    case GI_ARRAY_TYPE_PTR_ARRAY:
        if ((own == OX_GI_OWN_EVERYTHING) && gcontainer_elements_need_clean(ti)) {
            OX_GIParams p;

            p.ctxt = ctxt;
            p.gic = gic;
            p.ti0 = gi_type_info_get_param_type(ti, 0);

            g_ptr_array_foreach(a, gcontainer_free_item, &p);

            gi_base_info_unref(p.ti0);
        }
        g_ptr_array_unref(a);
        break;
    case GI_ARRAY_TYPE_BYTE_ARRAY:
        g_byte_array_unref(a);
        break;
    }
}

/*Add item to the OX array.*/
static OX_Result
array_item_to_value (OX_Context *ctxt, OX_GICtxt *gic, void *p, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH(ctxt, item)
    GIArgument arg, *parg;
    OX_GIOwn own = OX_GI_OWN_NOTHING;
    OX_Result r;

    if (gitype_is_struct_union(ti)) {
        arg.v_pointer = p;
        parg = &arg;
    } else if (gi_type_info_is_pointer(ti)) {
        arg.v_pointer = *(void**)p;
        parg = &arg;
    } else {
        parg = p;
    }

    if ((r = giargument_to_value(ctxt, gic, parg, &own, ti, item)) == OX_ERR)
        goto end;

    if ((r = ox_array_append(ctxt, v, item)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Convert C array to OX value.*/
static OX_Result
carray_to_value (OX_Context *ctxt, OX_GICtxt *gic, void *a, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    size_t len = array_get_length(ctxt, gic, ti);
    size_t i;
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    char *p = a;
    OX_Result r;

    if ((esize == SIZE_MAX) || (len == SIZE_MAX)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("array's size is unknown"));
        goto end;
    }

    if ((r = ox_array_new(ctxt, v, 0)) == OX_ERR)
        goto end;

    for (i = 0; i < len; i ++, p += esize) {
        if ((r = array_item_to_value(ctxt, gic, p, eti, v)) == OX_ERR)
            goto end;
    }
    
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    return r;
}

/*Convert GArray to OX value.*/
static OX_Result
garray_to_value (OX_Context *ctxt, OX_GICtxt *gic, GArray *a, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    size_t esize = g_array_get_element_size(a);
    size_t len = a->len, i;
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    char *p = a->data;
    OX_Result r;

    if ((r = ox_array_new(ctxt, v, 0)) == OX_ERR)
        goto end;

    for (i = 0; i < len; i ++, p += esize) {
        if ((r = array_item_to_value(ctxt, gic, p, eti, v)) == OX_ERR)
            goto end;
    }
    
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    return r;
}

/*Convert the GPtrArray to OX value.*/
static OX_Result
gptrarray_to_value (OX_Context *ctxt, OX_GICtxt *gic, GPtrArray *a, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_GIParams p;
    OX_Result r;

    if ((r = ox_array_new(ctxt, v, 0)) == OX_ERR)
        return r;

    p.ctxt = ctxt;
    p.gic = gic;
    p.ti0 = gi_type_info_get_param_type(ti, 0);
    p.v = v;
    p.r = OX_OK;

    g_ptr_array_foreach(a, gcontainer_add_item, &p);

    gi_base_info_unref(p.ti0);

    return p.r;
}

/*Convert the GByteArray to OX value.*/
static OX_Result
gbytearray_to_value (OX_Context *ctxt, OX_GICtxt *gic, GByteArray *a, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pty)
    OX_Value *ty;
    size_t len = a->len;
    uint8_t *b = NULL;
    OX_CValueInfo cvi;
    OX_Result r;

    ty = ox_ctype_get(ctxt, OX_CTYPE_U8);

    if ((r = ox_ctype_pointer(ctxt, pty, ty, len)) == OX_ERR)
        goto end;

    if (!OX_NEW_N(ctxt, b, len)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    cvi.v.p = b;
    cvi.base = NULL;
    cvi.own = OX_CPTR_EXTERNAL;

    if ((r = ox_cvalue_new(ctxt, v, pty, &cvi)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (b)
            OX_DEL_N(ctxt, b, len);
    }
    OX_VS_POP(ctxt, pty)
    return r;
}

/*Convert the array to OX value.*/
static OX_Result
array_to_value (OX_Context *ctxt, OX_GICtxt *gic, void *a, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r = OX_OK;

    switch (gi_type_info_get_array_type(ti)) {
    case GI_ARRAY_TYPE_C:
        r = carray_to_value(ctxt, gic, a, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_ARRAY:
        r = garray_to_value(ctxt, gic, a, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_PTR_ARRAY:
        r = gptrarray_to_value(ctxt, gic, a, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_BYTE_ARRAY:
        r = gbytearray_to_value(ctxt, gic, a, pown, ti, v);
        break;
    default:
        assert(0);
    }

    return r;
}

/*Convert the OX value to C array.*/
static OX_Result
carray_from_value (OX_Context *ctxt, OX_GICtxt *gic, void **pa, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, item, ty)
    OX_Result r;
    void *p = NULL;
    size_t alloc = 0;
    size_t set_len = 0;
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    gboolean zterm = gi_type_info_is_zero_terminated(ti);

    if (esize == SIZE_MAX) {
        r = ox_throw_range_error(ctxt, OX_TEXT("element's size is unknown"));
        goto end;
    }

    if (ox_value_is_array(ctxt, v)) {
        size_t len = ox_array_length(ctxt, v);
        size_t elen = array_get_length(ctxt, gic, ti);
        size_t i;
        uint8_t *dst;

        if (elen != SIZE_MAX) {
            if (len < elen) {
                r = ox_throw_range_error(ctxt, OX_TEXT("array's length is less than the expected length"));
                goto end;
            }

            len = elen;
        }

        if (!len) {
            p = NULL;
        } else {
            alloc = esize * len;

            if (zterm)
                alloc += esize;

            if (!OX_NEW_N_0(ctxt, p, alloc)) {
                r = ox_throw_no_mem_error(ctxt);
                goto end;
            }

            for (i = 0, dst = p; i < len; i ++, set_len ++, dst += esize) {
                GIArgument arg;
                OX_GIOwn own = OX_GI_OWN_NOTHING;

                if ((r = ox_array_get_item(ctxt, v, i, item)) == OX_ERR)
                    goto end;

                if ((r = giargument_from_value(ctxt, gic, &arg, &own, eti, item)) == OX_ERR)
                    goto end;

                if (gitype_is_struct_union(eti)) {
                    memcpy(dst, arg.v_pointer, esize);
                } else if (gitype_is_ref(eti)) {
                    *(void**)dst = gitype_ref(eti, arg.v_pointer);
                } else {
                    memcpy(dst, &arg, esize);
                }

                giargument_free(ctxt, gic, &arg, own, eti);
            }
        }

        *pa = p;
        *pown = OX_GI_OWN_EVERYTHING;
    } else if (ox_value_is_cvalue(ctxt, v)) {
        OX_Value *cty = ox_cvalue_get_ctype(ctxt, v);
        OX_Value *ity;
        size_t elen = array_get_length(ctxt, gic, ti);
        OX_Bool match = OX_FALSE;
        OX_CArrayInfo cai;

        if (!(ity = ox_ctype_get_value_type(ctxt, cty)))
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C pointer"));

        switch (gi_type_info_get_tag(eti)) {
        case GI_TYPE_TAG_INT8:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_I8);
            break;
        case GI_TYPE_TAG_UINT8:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_U8);
            break;
        case GI_TYPE_TAG_INT16:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_I16);
            break;
        case GI_TYPE_TAG_UINT16:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_U16);
            break;
        case GI_TYPE_TAG_INT32:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_I32);
            break;
        case GI_TYPE_TAG_UINT32:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_U32);
            break;
        case GI_TYPE_TAG_INT64:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_I64);
            break;
        case GI_TYPE_TAG_UINT64:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_U64);
            break;
        case GI_TYPE_TAG_FLOAT:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_F32);
            break;
        case GI_TYPE_TAG_DOUBLE:
            match = (ox_ctype_get_kind(ctxt, ity) == OX_CTYPE_F64);
            break;
        case GI_TYPE_TAG_INTERFACE: {
            GIBaseInfo *bi = gi_type_info_get_interface(eti);

            if (GI_IS_STRUCT_INFO(bi) || GI_IS_UNION_INFO(bi)) {
                r = gibaseinfo_to_value(ctxt, gic->repo, bi, ty);
                if (r == OX_OK)
                    match = ox_equal(ctxt, ty, ity);
            } else {
                r = OX_OK;
            }

            gi_base_info_unref(bi);

            if (r == OX_ERR)
                goto end;
            break;
        }
        default:
            break;
        }

        if (!match)
            return ox_throw_type_error(ctxt, OX_TEXT("the array's item type mismatch"));

        if ((r = ox_ctype_get_array_info(ctxt, cty, &cai)) == OX_ERR)
            goto end;

        if ((cai.len != -1) && (elen != SIZE_MAX) && (cai.len < elen)) {
            r = ox_throw_range_error(ctxt, OX_TEXT("array's length is less than the expected length"));
            goto end;
        }

        *pa = ox_cvalue_get_pointer(ctxt, v);
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to C array"));
        goto end;
    }
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    if (r == OX_ERR) {
        if (p) {
            carray_free_items(ctxt, gic, p, ti, set_len);
            g_free(p);
        }
    }
    OX_VS_POP(ctxt, item)
    return r;
}

/*Convert the OX value to GArray.*/
static OX_Result
garray_from_value (OX_Context *ctxt, OX_GICtxt *gic, GArray **pa, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH(ctxt, item)
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    gboolean zterm = gi_type_info_is_zero_terminated(ti);
    GArray *a = NULL;
    OX_Result r;

    if (esize == SIZE_MAX) {
        r = ox_throw_range_error(ctxt, OX_TEXT("element's size is unknown"));
        goto end;
    }

    if (!(a = g_array_new(zterm, FALSE, esize))) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (ox_value_is_array(ctxt, v)) {
        size_t len = ox_array_length(ctxt, v);
        size_t elen = array_get_length(ctxt, gic, ti);
        size_t i;

        if (elen != SIZE_MAX) {
            if (len < elen) {
                r = ox_throw_range_error(ctxt, OX_TEXT("array's length is less than the expected length"));
                goto end;
            }

            len = elen;
        }

        for (i = 0; i < len; i ++) {
            GIArgument arg;
            OX_GIOwn own = OX_GI_OWN_NOTHING;
            void *p;

            if ((r = ox_array_get_item(ctxt, v, i, item)) == OX_ERR)
                goto end;

            if ((r = giargument_from_value(ctxt, gic, &arg, &own, eti, item)) == OX_ERR)
                goto end;

            if (gitype_is_struct_union(eti)) {
                p = arg.v_pointer;
            } else if (gitype_is_ref(eti)) {
                gitype_ref(eti, arg.v_pointer);
                p = &arg;
            } else {
                p = &arg;
            }

            g_array_append_vals(a, p, 1);

            giargument_free(ctxt, gic, &arg, own, eti);
        }

        *pa = a;
        *pown = OX_GI_OWN_EVERYTHING;
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to GArray"));
        goto end;
    }
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    if (r == OX_ERR) {
        if (a)
            array_free(ctxt, gic, a, OX_GI_OWN_EVERYTHING, ti);
    }
    OX_VS_POP(ctxt, item)
    return r;
}

/*Convert the OX value to GPtrArray.*/
static OX_Result
gptrarray_from_value (OX_Context *ctxt, OX_GICtxt *gic, GPtrArray **pa, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH(ctxt, item)
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
    GPtrArray *a = NULL;
    OX_Result r;

    if (esize == SIZE_MAX) {
        r = ox_throw_range_error(ctxt, OX_TEXT("element's size is unknown"));
        goto end;
    }

    if (!(a = g_ptr_array_new())) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (ox_value_is_array(ctxt, v)) {
        size_t len = ox_array_length(ctxt, v);
        size_t elen = array_get_length(ctxt, gic, ti);
        size_t i;

        if (elen != SIZE_MAX) {
            if (len < elen) {
                r = ox_throw_range_error(ctxt, OX_TEXT("array's length is less than the expected length"));
                goto end;
            }

            len = elen;
        }

        for (i = 0; i < len; i ++) {
            void *p;

            if ((r = ox_array_get_item(ctxt, v, i, item)) == OX_ERR)
                goto end;

            if ((r = gcontainer_element_from_value(ctxt, gic, &p, eti, item)) == OX_ERR)
                goto end;

            g_ptr_array_add(a, p);
        }

        *pa = a;
        *pown = OX_GI_OWN_EVERYTHING;
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to GPtrArray"));
        goto end;
    }
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    if (r == OX_ERR) {
        if (a)
            array_free(ctxt, gic, a, OX_GI_OWN_EVERYTHING, ti);
    }
    OX_VS_POP(ctxt, item)
    return r;
}

/*Convert the OX value to GByteArray.*/
static OX_Result
gbytearray_from_value (OX_Context *ctxt, OX_GICtxt *gic, GByteArray **pa, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    GByteArray *a;
    OX_Result r;
    size_t elen = array_get_length(ctxt, gic, ti);

    if (!(a = g_byte_array_new())) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (ox_value_is_string(ctxt, v)) {
        size_t len = ox_string_length(ctxt, v);
        const void *p;

        if (elen != SIZE_MAX) {
            if (len < elen) {
                r = ox_throw_range_error(ctxt, OX_TEXT("string's length is less than the expected length"));
                goto end;
            }

            len = elen;
        }

        p = ox_string_get_char_star(ctxt, v);

        g_byte_array_append(a, p, len);
    } else if (ox_value_is_ctype(ctxt, v)) {
        OX_Value *ty = ox_cvalue_get_ctype(ctxt, v);
        OX_CArrayInfo cai;
        size_t len;
        void *p;

        if ((r = ox_ctype_get_array_info(ctxt, ty, &cai)) == OX_ERR)
            goto end;

        if ((cai.isize == -1) || (cai.len == -1)) {
            r = ox_throw_type_error(ctxt, OX_TEXT("array's size is unknown"));
            goto end;
        }

        len = cai.isize * cai.len;
        if (elen != SIZE_MAX) {
            if (len < elen) {
                r = ox_throw_range_error(ctxt, OX_TEXT("string's length is less than the expected length"));
                goto end;
            }

            len = elen;
        }

        p = ox_value_get_pointer(ctxt, v);

        g_byte_array_append(a, p, len);
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to GByteArray"));
        goto end;
    }

    *pa = a;
    *pown = OX_GI_OWN_EVERYTHING;
    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (a)
            g_byte_array_unref(a);
    }
    return r;
}

/*Convert the OX value to array.*/
static OX_Result
array_from_value (OX_Context *ctxt, OX_GICtxt *gic, void **pa, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r = OX_OK;

    switch (gi_type_info_get_array_type(ti)) {
    case GI_ARRAY_TYPE_C:
        r = carray_from_value(ctxt, gic, pa, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_ARRAY:
        r = garray_from_value(ctxt, gic, (GArray**)pa, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_PTR_ARRAY:
        r = gptrarray_from_value(ctxt, gic, (GPtrArray**)pa, pown, ti, v);
        break;
    case GI_ARRAY_TYPE_BYTE_ARRAY:
        r = gbytearray_from_value(ctxt, gic, (GByteArray**)pa, pown, ti, v);
        break;
    default:
        assert(0);
    }

    return r;
}
