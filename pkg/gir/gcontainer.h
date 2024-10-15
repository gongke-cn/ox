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
 * GLib container.
 */

/*Get the element size of a container.*/
static size_t
gcontainer_get_element_size (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti)
{
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    size_t size;

    size = gitype_get_size(ctxt, gic, eti);

    gi_base_info_unref(eti);

    return size;
}

/*Check if the container's items should be clean.*/
static OX_Bool
gcontainer_elements_need_clean (GITypeInfo *ti)
{
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    OX_Bool b;

    b = gitype_is_ptr_element(eti);

    gi_base_info_unref(eti);

    return b;
}

/*Free the item in the container.*/
static void
gcontainer_item_free (OX_Context *ctxt, OX_GICtxt *gic, void *data, GITypeInfo *ti)
{
    if (gitype_is_big_number(ti)) {
        if (data)
            g_free(data);
    } else {
        GIArgument arg;

        arg.v_pointer = data;

        giargument_free(ctxt, gic, &arg, OX_GI_OWN_EVERYTHING, ti);
    }
}

/*Free the item in the container.*/
static void
gcontainer_free_item (void *data, void *userdata)
{
    OX_GIParams *p = userdata;
    OX_Context *ctxt = p->ctxt;
    OX_GICtxt *gic = p->gic;
    GITypeInfo *ti = p->ti0;

    gcontainer_item_free(ctxt, gic, data, ti);
}

/*Convert the element of container to value.*/
static OX_Result
gcontainer_element_to_value (OX_Context *ctxt, OX_GICtxt *gic, void *data, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r = OX_OK;
    GIArgument arg;
    OX_GIOwn own = OX_GI_OWN_NOTHING;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_INT64: {
        int64_t i = *(int64_t*)data;

        if ((i <= OX_MAX_SAFE_INTEGER) && (i >= OX_MIN_SAFE_INTEGER))
            ox_value_set_number(ctxt, v, i);
        else if ((r = ox_value_set_int64(ctxt, v, i)) == OX_ERR)
            goto end;
        break;
    }
    case GI_TYPE_TAG_UINT64: {
        uint64_t i = *(uint64_t*)data;

        if (i <= OX_MAX_SAFE_INTEGER)
            ox_value_set_number(ctxt, v, i);
        else if ((r = ox_value_set_uint64(ctxt, v, i)) == OX_ERR)
            goto end;
        break;
    }
    case GI_TYPE_TAG_FLOAT: {
        float f = *(float*)data;

        ox_value_set_number(ctxt, v, f);
        break;
    }
    case GI_TYPE_TAG_DOUBLE: {
        double d = *(double*)data;

        ox_value_set_number(ctxt, v, d);
        break;
    }
    default:
        arg.v_pointer = data;

        if ((r = giargument_to_value(ctxt, gic, &arg, &own, ti, v)) == OX_ERR)
            goto end;
        break;
    }
end:
    return r;
}

/*Add item to the array.*/
static void
gcontainer_add_item (void *data, void *userdata)
{
    OX_GIParams *p = userdata;
    OX_Context *ctxt = p->ctxt;
    OX_GICtxt *gic = p->gic;
    GITypeInfo *ti = p->ti0;
    OX_Value *a = p->v;
    OX_VS_PUSH(ctxt, v)
    OX_Result r;

    if ((r = gcontainer_element_to_value(ctxt, gic, data, ti, v)) == OX_ERR)
        goto end;

    r = ox_array_append(ctxt, a, v);
end:
    OX_VS_POP(ctxt, v)
    p->r |= r;
}

/*Convert OX value to container's item.*/
static OX_Result
gcontainer_element_from_value (OX_Context *ctxt, OX_GICtxt *gic, void **pdata, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r;
    GIArgument arg;
    void *data;
    OX_GIOwn own = OX_GI_OWN_NOTHING;

    if ((r = giargument_from_value(ctxt, gic, &arg, &own, ti, v)) == OX_ERR)
        return r;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_INT64:
        data = g_malloc(sizeof(int64_t));
        *(int64_t*)data = arg.v_int64;
        break;
    case GI_TYPE_TAG_UINT64:
        data = g_malloc(sizeof(uint64_t));
        *(uint64_t*)data = arg.v_uint64;
        break;
    case GI_TYPE_TAG_FLOAT:
        data = g_malloc(sizeof(float));
        *(float*)data = arg.v_float;
        break;
    case GI_TYPE_TAG_DOUBLE:
        data = g_malloc(sizeof(double));
        *(double*)data = arg.v_double;
        break;
    default:
        data = arg.v_pointer;
        break;
    }

    *pdata = data;
    return OX_OK;
}
