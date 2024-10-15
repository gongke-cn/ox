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
 * GI type.
 */

/** GI Signal.*/
typedef struct {
    OX_HashEntry  he; /**< Hash table entry data.*/
    GISignalInfo *si; /**< Signal information.*/
} OX_GISignal;

/** GI type.*/
typedef struct {
    OX_HashEntry          he;   /**< Hash table entry data.*/
    GIRegisteredTypeInfo *rti;  /**< The type's information.*/
    OX_Value              v;    /**< Value of the type.*/
    OX_Hash               signal_hash; /**< Signal hash table.*/
    OX_GIRepository      *repo; /**< The repository.*/
} OX_GIType;

/*Scan referenced objects in the GI type.*/
static void
gitype_scan (OX_Context *ctxt, void *p)
{
    OX_GIType *ty = p;

    ox_gc_scan_value(ctxt, &ty->repo->v);
}

/*Free the GI type data.*/
static void
gitype_free (OX_Context *ctxt, void *p)
{
    OX_GIType *ty = p;
    size_t i;
    OX_GISignal *s, *ns;

    /*Free signals.*/
    ox_hash_foreach_safe_c(&ty->signal_hash, i, s, ns, OX_GISignal, he) {
        gi_base_info_unref(s->si);
        OX_DEL(ctxt, s);
    }
    ox_hash_deinit(ctxt, &ty->signal_hash);

    /*Free the type.*/
    gi_base_info_unref(ty->rti);

    OX_DEL(ctxt, ty);
}

/*GI type's operation functions.*/
static const OX_PrivateOps
gitype_ops = {
    gitype_scan,
    gitype_free
};

/*Get the GIType from the value.*/
static OX_GIType*
gitype_get (OX_Context *ctxt, OX_Value *v)
{
    OX_GIType *ty = ox_object_get_priv(ctxt, v, &gitype_ops);

    if (!ty)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a GI type"));

    return ty;
}

/*Add a type.*/
static OX_Result
gitype_add (OX_Context *ctxt, OX_GIRepository *repo, OX_Value *v, GIRegisteredTypeInfo *rti)
{
    OX_GIType *ty;
    GType gty;
    OX_Result r;

    if (!OX_NEW(ctxt, ty)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ty->repo = repo;
    ty->rti = GI_REGISTERED_TYPE_INFO(gi_base_info_ref(rti));
    ox_value_copy(ctxt, &ty->v, v);
    ox_char_star_hash_init(&ty->signal_hash);

    if ((r = ox_object_set_priv(ctxt, v, &gitype_ops, ty)) == OX_ERR) {
        gitype_free(ctxt, ty);
        goto end;
    }

    gty = gi_registered_type_info_get_g_type(rti);

    r = ox_hash_insert(ctxt, &repo->type_hash, OX_SIZE2PTR(gty), &ty->he, NULL);
    assert(r == OX_OK);
end:
    return r;
}

/*Get the type's information.*/
static GIRegisteredTypeInfo*
gitype_get_info (OX_Context *ctxt, OX_Value *v)
{
    OX_GIType *ty = gitype_get(ctxt, v);

    assert(ty);

    return ty->rti;
}

/*Get the type's name*/
static const char*
gitype_get_name (OX_Context *ctxt, OX_Value *v)
{
    GIRegisteredTypeInfo *rti = gitype_get_info(ctxt, v);

    return gi_base_info_get_name(GI_BASE_INFO(rti));
}

/*Check if the type is a big number.*/
static OX_Bool
gitype_is_big_number (GITypeInfo *ti)
{
    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_INT64:
    case GI_TYPE_TAG_UINT64:
    case GI_TYPE_TAG_FLOAT:
    case GI_TYPE_TAG_DOUBLE:
        return OX_TRUE;
    default:
        return OX_FALSE;
    }
}

/*Check if the type is struct or union.*/
static OX_Bool
gitype_is_struct_union (GITypeInfo *ti)
{
    GIBaseInfo *bi;
    OX_Bool b = OX_FALSE;

    if (gi_type_info_get_tag(ti) != GI_TYPE_TAG_INTERFACE)
        return OX_FALSE;

    bi = gi_type_info_get_interface(ti);
    if (GI_IS_STRUCT_INFO(bi) || GI_IS_UNION_INFO(bi))
        b = OX_TRUE;
    gi_base_info_unref(bi);

    return b;
}

/*Check if the type is struct or union.*/
static OX_Bool
gitype_is_callback (GITypeInfo *ti)
{
    GIBaseInfo *bi;
    OX_Bool b = OX_FALSE;

    if (gi_type_info_get_tag(ti) != GI_TYPE_TAG_INTERFACE)
        return OX_FALSE;

    bi = gi_type_info_get_interface(ti);
    if (GI_IS_CALLBACK_INFO(bi))
        b = OX_TRUE;
    gi_base_info_unref(bi);

    return b;
}

/*Check if the type is a reference.*/
static OX_Bool
gitype_is_ref (GITypeInfo *ti)
{
    GIBaseInfo *bi;
    OX_Bool b = OX_FALSE;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_GHASH:
        b = OX_TRUE;
    case GI_TYPE_TAG_ARRAY:
        switch (gi_type_info_get_array_type(ti)) {
        case GI_ARRAY_TYPE_ARRAY:
        case GI_ARRAY_TYPE_PTR_ARRAY:
        case GI_ARRAY_TYPE_BYTE_ARRAY:
            b = OX_TRUE;
        default:
            break;
        }
        break;
    case GI_TYPE_TAG_INTERFACE:
        bi = gi_type_info_get_interface(ti);
        if (GI_IS_OBJECT_INFO(bi))
            b = OX_TRUE;
        gi_base_info_unref(bi);
        break;
    default:
        break;
    }

    return b;
}

/*Increase the type's reference counter.*/
static void*
gitype_ref (GITypeInfo *ti, void *p)
{
    GIBaseInfo *bi;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_GHASH:
        g_hash_table_ref(p);
        break;
    case GI_TYPE_TAG_ARRAY:
        switch (gi_type_info_get_array_type(ti)) {
        case GI_ARRAY_TYPE_ARRAY:
            g_array_ref(p);
            break;
        case GI_ARRAY_TYPE_PTR_ARRAY:
            g_ptr_array_ref(p);
            break;
        case GI_ARRAY_TYPE_BYTE_ARRAY:
            g_byte_array_ref(p);
            break;
        default:
            break;
        }
        break;
    case GI_TYPE_TAG_INTERFACE:
        bi = gi_type_info_get_interface(ti);
        if (GI_IS_OBJECT_INFO(bi)) {
            GIObjectInfoRefFunction fn;

            fn = gi_object_info_get_ref_function_pointer(GI_OBJECT_INFO(bi));
            if (fn)
                fn(p);
        }
        gi_base_info_unref(bi);
        break;
    default:
        break;
    }

    return p;
}

#if 0
/*Decrease the type's reference counter.*/
static void
gitype_unref (GITypeInfo *ti, void *p)
{
    GIBaseInfo *bi;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_GHASH:
        g_hash_table_unref(p);
        break;
    case GI_TYPE_TAG_ARRAY:
        switch (gi_type_info_get_array_type(ti)) {
        case GI_ARRAY_TYPE_ARRAY:
            g_array_unref(p);
            break;
        case GI_ARRAY_TYPE_PTR_ARRAY:
            g_ptr_array_unref(p);
            break;
        case GI_ARRAY_TYPE_BYTE_ARRAY:
            g_byte_array_unref(p);
            break;
        default:
            break;
        }
        break;
    case GI_TYPE_TAG_INTERFACE:
        bi = gi_type_info_get_interface(ti);
        if (GI_IS_OBJECT_INFO(bi)) {
            GIObjectInfoUnrefFunction fn;

            fn = gi_object_info_get_unref_function_pointer(GI_OBJECT_INFO(bi));
            if (fn)
                fn(p);
        }
        gi_base_info_unref(bi);
        break;
    default:
        break;
    }
}
#endif

/*Check if the type is used as pointer element in container.*/
static OX_Bool
gitype_is_ptr_element (GITypeInfo *ti)
{
    if (gitype_is_big_number(ti))
        return OX_TRUE;
    else if (gi_type_info_is_pointer(ti) && !gitype_is_callback(ti))
        return OX_TRUE;
    else
        return OX_FALSE;
}

/*Get the type's size.*/
static size_t
gitype_get_size (OX_Context *ctxt, OX_GICtxt *gic, GITypeInfo *ti)
{
    size_t s = SIZE_MAX;

    switch (gi_type_info_get_tag(ti)) {
    case GI_TYPE_TAG_BOOLEAN:
    case GI_TYPE_TAG_INT8:
    case GI_TYPE_TAG_UINT8:
        s = 1;
        break;
    case GI_TYPE_TAG_INT16:
    case GI_TYPE_TAG_UINT16:
        s = 2;
        break;
    case GI_TYPE_TAG_INT32:
    case GI_TYPE_TAG_UINT32:
    case GI_TYPE_TAG_UNICHAR:
        s = 4;
        break;
    case GI_TYPE_TAG_INT64:
    case GI_TYPE_TAG_UINT64:
        s = 8;
        break;
    case GI_TYPE_TAG_FLOAT:
        s = sizeof(float);
        break;
    case GI_TYPE_TAG_DOUBLE:
        s = sizeof(double);
        break;
    case GI_TYPE_TAG_GTYPE:
        s = sizeof(GType);
        break;
    case GI_TYPE_TAG_UTF8:
    case GI_TYPE_TAG_FILENAME:
    case GI_TYPE_TAG_GLIST:
    case GI_TYPE_TAG_GSLIST:
    case GI_TYPE_TAG_GHASH:
    case GI_TYPE_TAG_ERROR:
        s = sizeof(void*);
        break;
    case GI_TYPE_TAG_ARRAY:
        switch (gi_type_info_get_array_type(ti)) {
        case GI_ARRAY_TYPE_C: {
            size_t esize = gcontainer_get_element_size(ctxt, gic, ti);
            size_t len = array_get_length(ctxt, gic, ti);

            if ((esize == SIZE_MAX) || (len == SIZE_MAX))
                return SIZE_MAX;

            s = esize * len;
            break;
        }
        case GI_ARRAY_TYPE_ARRAY:
        case GI_ARRAY_TYPE_PTR_ARRAY:
        case GI_ARRAY_TYPE_BYTE_ARRAY:
            s = sizeof(void*);
            break;
        }
        break;
    case GI_TYPE_TAG_INTERFACE: {
        GIBaseInfo *bi = gi_type_info_get_interface(ti);

        if (GI_IS_STRUCT_INFO(bi)) {
            s = gi_struct_info_get_size(GI_STRUCT_INFO(bi));
        } else if (GI_IS_UNION_INFO(bi)) {
            s = gi_union_info_get_size(GI_UNION_INFO(bi));
        } else {
            s = sizeof(void*);
        }

        gi_base_info_unref(bi);
        break;
    }
    default:
        break;
    }

    return s;
}
