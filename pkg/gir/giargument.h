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
 * GIArgument.
 */

/*Free the interface.*/
static void
interface_free (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, GIBaseInfo *bi)
{
    if (GI_IS_STRUCT_INFO(bi) || GI_IS_UNION_INFO(bi)) {
        g_free(arg->v_pointer);
    } else if (GI_IS_OBJECT_INFO(bi)) {
        GIObjectInfoUnrefFunction fn = gi_object_info_get_unref_function_pointer(GI_OBJECT_INFO(bi));

        if (fn)
            fn(arg->v_pointer);
    }
}

/*Convert the GTypeInstance to OX value.*/
static OX_Result
gtypeinst_to_value (OX_Context *ctxt, OX_GICtxt *gic, void *p, OX_GIOwn *pown, GIBaseInfo *bi, OX_Value *v)
{
    GIBaseInfo *inst_bi = NULL;
    OX_Result r;
    GType gty = G_TYPE_FROM_INSTANCE(p);

    inst_bi = gi_repository_find_by_gtype(gic->repo->repo, gty);

    if (inst_bi)
        bi = inst_bi;

    if (!bi) {
        r = ox_throw_type_error(ctxt, OX_TEXT("cannot find the information of GType"));
    } else {
        r = giinst_to_value(ctxt, gic->repo, p, pown, bi, v);
    }

    if (inst_bi)
        gi_base_info_unref(inst_bi);

    return r;
}

/*Convert the interface to OX value.*/
static OX_Result
interface_to_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GIBaseInfo *bi, OX_Value *v)
{
    OX_Result r = OX_FALSE;

    if (GI_IS_CALLBACK_INFO(bi)) {
        r = gicallback_to_value(ctxt, gic->repo, arg->v_pointer, GI_CALLABLE_INFO(bi), v);
    } else if (GI_IS_ENUM_INFO(bi)) {
        ox_value_set_number(ctxt, v, arg->v_int32);
        r = OX_OK;
    } else if (GI_IS_STRUCT_INFO(bi) || GI_IS_UNION_INFO(bi)) {
        r = giinst_to_value(ctxt, gic->repo, arg->v_pointer, pown, bi, v);
    } else {
        r = gtypeinst_to_value(ctxt, gic, arg->v_pointer, pown, bi, v);
    }

    return r;
}

/*Convert the OX value to interface.*/
static OX_Result
interface_from_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GIBaseInfo *bi, OX_Value *v)
{
    OX_Result r;

    if (GI_IS_CALLBACK_INFO(bi)) {
        if (ox_value_is_function(ctxt, v)) {
            if ((r = gicallback_from_value(ctxt, gic->repo, v, GI_CALLABLE_INFO(bi), &arg->v_pointer)) == OX_ERR)
                return r;
        } else {
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a GLib callback"));
        }
    } else if (GI_IS_ENUM_INFO(bi)) {
        int32_t i;

        if ((r = ox_to_int32(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_int32 = i;
    } else {
        OX_GIInst *inst = giinst_get(ctxt, v);

        if (!inst)
            return OX_ERR;

        arg->v_pointer = inst->he.key;
    }

    return OX_OK;
}

/*Free the argument data.*/
static void
giargument_free (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn own, GITypeInfo *ti)
{
    if (own == OX_GI_OWN_NOTHING)
        return;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_UTF8:
    case GI_TYPE_TAG_FILENAME:
        if (arg->v_pointer)
            g_free(arg->v_pointer);
        break;
    case GI_TYPE_TAG_ARRAY:
        if (arg->v_pointer)
            array_free(ctxt, gic, arg->v_pointer, own, ti);
        break;
    case GI_TYPE_TAG_INTERFACE:
        if (arg->v_pointer) {
            GIBaseInfo *bi = gi_type_info_get_interface(ti);
            interface_free(ctxt, gic, arg, bi);
            gi_base_info_unref(bi);
        }
        break;
    case GI_TYPE_TAG_GLIST:
        if (arg->v_pointer)
            glist_free(ctxt, gic, arg->v_pointer, own, ti);
        break;
    case GI_TYPE_TAG_GSLIST:
        if (arg->v_pointer)
            gslist_free(ctxt, gic, arg->v_pointer, own, ti);
        break;
    case GI_TYPE_TAG_GHASH:
        if (arg->v_pointer)
            ghash_free(ctxt, gic, arg->v_pointer, own, ti);
        break;
    case GI_TYPE_TAG_ERROR:
        if (arg->v_pointer)
            g_error_free(arg->v_pointer);
        break;
    default:
        break;
    }
}

/*Convert the argument to OX value.*/
static OX_Result
giargument_to_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_VOID:
        if (!arg->v_pointer) {
            ox_value_set_null(ctxt, v);
        } else {
            if ((r = gtypeinst_to_value(ctxt, gic, arg->v_pointer, pown, NULL, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_BOOLEAN:
        ox_value_set_bool(ctxt, v, arg->v_boolean);
        break;
    case GI_TYPE_TAG_INT8:
        ox_value_set_number(ctxt, v, arg->v_int8);
        break;
    case GI_TYPE_TAG_UINT8:
        ox_value_set_number(ctxt, v, arg->v_uint8);
        break;
    case GI_TYPE_TAG_INT16:
        ox_value_set_number(ctxt, v, arg->v_int16);
        break;
    case GI_TYPE_TAG_UINT16:
        ox_value_set_number(ctxt, v, arg->v_uint16);
        break;
    case GI_TYPE_TAG_INT32:
        ox_value_set_number(ctxt, v, arg->v_int32);
        break;
    case GI_TYPE_TAG_UINT32:
    case GI_TYPE_TAG_UNICHAR:
        ox_value_set_number(ctxt, v, arg->v_uint32);
        break;
    case GI_TYPE_TAG_INT64:
        if ((arg->v_int64 <= OX_MAX_SAFE_INTEGER) && (arg->v_int64 >= OX_MIN_SAFE_INTEGER))
            ox_value_set_number(ctxt, v, arg->v_int64);
        else {
            if ((r = ox_value_set_int64(ctxt, v, arg->v_int64)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_UINT64:
        if (arg->v_uint64 <= OX_MAX_SAFE_INTEGER)
            ox_value_set_number(ctxt, v, arg->v_uint64);
        else {
            if ((r = ox_value_set_uint64(ctxt, v, arg->v_uint64)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_FLOAT:
        ox_value_set_number(ctxt, v, arg->v_float);
        break;
    case GI_TYPE_TAG_DOUBLE:
        ox_value_set_number(ctxt, v, arg->v_double);
        break;
    case GI_TYPE_TAG_GTYPE:
        ox_value_set_number(ctxt, v, arg->v_size);
        break;
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_UTF8:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else if ((r = ox_string_from_char_star(ctxt, v, arg->v_pointer)) == OX_ERR)
            return r;
        break;
    case GI_TYPE_TAG_GLIST:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else if ((r = glist_to_value(ctxt, gic, arg->v_pointer, pown, ti, v)) == OX_ERR)
            return r;
        break;
    case GI_TYPE_TAG_GSLIST:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else if ((r = gslist_to_value(ctxt, gic, arg->v_pointer, pown, ti, v)) == OX_ERR)
            return r;
        break;
    case GI_TYPE_TAG_GHASH:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else if ((r = ghash_to_value(ctxt, gic, arg->v_pointer, pown, ti, v)) == OX_ERR)
            return r;
        break;
    case GI_TYPE_TAG_ERROR:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else {
            if ((r = gerror_to_value(ctxt, arg->v_pointer, v, &gic->repo->script)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_ARRAY:
        if (!arg->v_pointer)
            ox_value_set_null(ctxt, v);
        else {
            if ((r = array_to_value(ctxt, gic, arg->v_pointer, pown, ti, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_INTERFACE:
        if (!arg->v_pointer) {
            ox_value_set_null(ctxt, v);
        } else {
            GIBaseInfo *bi = gi_type_info_get_interface(ti);
            r = interface_to_value(ctxt, gic, arg, pown, bi, v);
            gi_base_info_unref(bi);

            if (r == OX_ERR)
                return r;
        }
        break;
    default:
        assert(0);
        break;
    }

    return OX_OK;
}

/*Convert the OX value to GIArgument.*/
static OX_Result
giargument_from_value (OX_Context *ctxt, OX_GICtxt *gic, GIArgument *arg, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_Result r;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_VOID:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            OX_GIInst *inst = giinst_get(ctxt, v);

            if (!inst)
                return OX_ERR;

            arg->v_pointer = inst->he.key;
        }
        break;
    case GI_TYPE_TAG_BOOLEAN:
        arg->v_boolean = ox_to_bool(ctxt, v);
        break;
    case GI_TYPE_TAG_INT8: {
        int8_t i;

        if ((r = ox_to_int8(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_int8 = i;
        break;
    }
    case GI_TYPE_TAG_UINT8: {
        uint8_t i;

        if ((r = ox_to_uint8(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_uint8 = i;
        break;
    }
    case GI_TYPE_TAG_INT16: {
        int16_t i;

        if ((r = ox_to_int16(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_int16 = i;
        break;
    }
    case GI_TYPE_TAG_UINT16: {
        uint16_t i;

        if ((r = ox_to_uint16(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_uint16 = i;
        break;
    }
    case GI_TYPE_TAG_INT32: {
        int32_t i;

        if ((r = ox_to_int32(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_int32 = i;
        break;
    }
    case GI_TYPE_TAG_UINT32:
    case GI_TYPE_TAG_UNICHAR: {
        uint32_t i;

        if ((r = ox_to_uint32(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_uint32 = i;
        break;
    }
    case GI_TYPE_TAG_INT64: {
        int64_t i;

        if ((r = ox_to_int64(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_int64 = i;
        break;
    }
    case GI_TYPE_TAG_UINT64: {
        uint64_t i;

        if ((r = ox_to_uint64(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_uint64 = i;
        break;
    }
    case GI_TYPE_TAG_FLOAT: {
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        arg->v_float = n;
        break;
    }
    case GI_TYPE_TAG_DOUBLE: {
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        arg->v_double = n;
        break;
    }
    case GI_TYPE_TAG_GTYPE: {
        size_t i = 0;

        if ((r = ox_to_index(ctxt, v, &i)) == OX_ERR)
            return r;

        arg->v_size = i;
        break;
    }
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_UTF8:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else if (ox_value_is_string(ctxt, v)) {
            arg->v_pointer = (void*)ox_string_get_char_star(ctxt, v);
        } else {
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a string"));
        }
        break;
    case GI_TYPE_TAG_GLIST:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            if ((r = glist_from_value(ctxt, gic, (GList**)&arg->v_pointer, pown, ti, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_GSLIST:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            if ((r = gslist_from_value(ctxt, gic, (GSList**)&arg->v_pointer, pown, ti, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_GHASH:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            if ((r = ghash_from_value(ctxt, gic, (GHashTable**)&arg->v_pointer, pown, ti, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_ERROR:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            GError *err;

            if ((r = gerror_from_value(ctxt, &err, v, &gic->repo->script)) == OX_ERR)
                return r;

            arg->v_pointer = err;
        }
        break;
    case GI_TYPE_TAG_ARRAY:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            if ((r = array_from_value(ctxt, gic, (void**)&arg->v_pointer, pown, ti, v)) == OX_ERR)
                return r;
        }
        break;
    case GI_TYPE_TAG_INTERFACE:
        if (ox_value_is_null(ctxt, v)) {
            arg->v_pointer = NULL;
        } else {
            GIBaseInfo *bi = gi_type_info_get_interface(ti);
            r = interface_from_value(ctxt, gic, arg, pown, bi, v);
            gi_base_info_unref(bi);

            if (r == OX_ERR)
                return r;
        }
        break;
    default:
        assert(0);
        break;
    }

    return OX_OK;
}
