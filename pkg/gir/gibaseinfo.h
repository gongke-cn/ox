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
 * GIBaseInfo.
 */

/*Convert the registered type information to value.*/
static OX_Result
girttypeinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIRegisteredTypeInfo *rti, OX_Value *v)
{
    OX_Result r = OX_FALSE;
    GType gty = gi_registered_type_info_get_g_type(rti);
    OX_GIType *ty;

    ty = ox_hash_lookup_c(ctxt, &repo->type_hash, OX_SIZE2PTR(gty), NULL, OX_GIType, he);
    if (ty) {
        ox_value_copy(ctxt, v, &ty->v);
        r = OX_OK;
    } else if (GI_IS_STRUCT_INFO(rti)) {
        r = gistructinfo_to_value(ctxt, repo, GI_STRUCT_INFO(rti), v);
    } else if (GI_IS_UNION_INFO(rti)) {
        r = giunioninfo_to_value(ctxt, repo, GI_UNION_INFO(rti), v);
    } else if (GI_IS_INTERFACE_INFO(rti)) {
        r = giinterfaceinfo_to_value(ctxt, repo, GI_INTERFACE_INFO(rti), v);
    } else if (GI_IS_OBJECT_INFO(rti)) {
        r = giobjectinfo_to_value(ctxt, repo, GI_OBJECT_INFO(rti), v);
    }

    return r;
}

/*Convert the base information to value.*/
static OX_Result
gibaseinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIBaseInfo *bi, OX_Value *v)
{
    OX_Result r = OX_FALSE;

    if (GI_IS_ENUM_INFO(bi)) {
        r = gienuminfo_to_value(ctxt, repo, GI_ENUM_INFO(bi), v);
    } else if (GI_IS_CONSTANT_INFO(bi)) {
        r = giconstantinfo_to_value(ctxt, repo, GI_CONSTANT_INFO(bi), v);
    } else if (GI_IS_FUNCTION_INFO(bi)) {
        r = gifunctioninfo_to_value(ctxt, repo, GI_FUNCTION_INFO(bi), v);
    } else if (GI_IS_REGISTERED_TYPE_INFO(bi)) {
        r = girttypeinfo_to_value(ctxt, repo, GI_REGISTERED_TYPE_INFO(bi), v);
    } else if (GI_IS_VFUNC_INFO(bi)) {
        r = givfuncinfo_to_value(ctxt, repo, GI_VFUNC_INFO(bi), v);
    }

    return r;
}

/*Convert the base information to the object's constance property.*/
static OX_Result
gibaseinfo_to_const_prop (OX_Context *ctxt, OX_GIRepository *repo, GIBaseInfo *bi, OX_Value *o, OX_Value *v)
{
    OX_VS_PUSH(ctxt, name)
    OX_Result r;

    r = gibaseinfo_to_value(ctxt, repo, bi, v);
    if (r == OX_OK) {
        const char *ncstr = gi_base_info_get_name(bi);

        if ((r = ox_string_from_char_star(ctxt, name, ncstr)) == OX_ERR)
            goto end;

        if ((r = ox_object_add_const(ctxt, o, name, v)) == OX_ERR)
            goto end;

        if (ox_value_is_object(ctxt, v)) {
            if ((r = ox_object_set_scope(ctxt, v, o)) == OX_ERR)
                goto end;

            if ((r = ox_object_set_name(ctxt, v, name)) == OX_ERR)
                goto end;
        }
    }
end:
    OX_VS_POP(ctxt, name)
    return r;
}