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
 * GIEnumInfo
 */

/*Convert the enumeration information to value.*/
static OX_Result
gienuminfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIEnumInfo *ei, OX_Value *c)
{
    OX_VS_PUSH_2(ctxt, inf, v)
    unsigned int n, i;
    OX_Result r;

    if ((r = ox_class_new(ctxt, c, inf)) == OX_ERR)
        goto end;

    /*Load values.*/
    n = gi_enum_info_get_n_values(ei);
    for (i = 0; i < n; i++) {
        GIValueInfo *vi = gi_enum_info_get_value(ei, i);
        int64_t i = gi_value_info_get_value(vi);
        const char *name = gi_base_info_get_name(GI_BASE_INFO(vi));

        ox_value_set_number(ctxt, v, i);
        r = ox_object_add_const_s(ctxt, c, name, v);

        gi_base_info_unref(vi);

        if (r == OX_ERR)
            goto end;
    }

    /*Load methods.*/
    n = gi_enum_info_get_n_methods(ei);
    for (i = 0; i < n; i ++) {
        GIFunctionInfo *fi = gi_enum_info_get_method(ei, i);
        OX_Value *o;

        if (gi_function_info_get_flags(fi) & GI_FUNCTION_IS_CONSTRUCTOR)
            o = c;
        else
            o = inf;

        r = gibaseinfo_to_const_prop(ctxt, repo, GI_BASE_INFO(fi), o, v);

        gi_base_info_unref(fi);

        if (r == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, inf)
    return r;
}
