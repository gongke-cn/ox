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
 * GIStructInfo.
 */

/*Convert the structure information to value.*/
static OX_Result
gistructinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIStructInfo *si, OX_Value *c)
{
    OX_VS_PUSH_2(ctxt, inf, v)
    size_t size = gi_struct_info_get_size(si);
    unsigned int i, n;
    OX_Result r;

    if ((r = ox_ctype_struct(ctxt, c, inf, size)) == OX_ERR)
        goto end;

    if ((r = gitype_add(ctxt, repo, c, GI_REGISTERED_TYPE_INFO(si))) == OX_ERR)
        goto end;

    /*Load fields.*/
    n = gi_struct_info_get_n_fields(si);
    for (i = 0; i < n; i ++) {
        GIFieldInfo *fi = gi_struct_info_get_field(si, i);

        r = gifieldinfo_load(ctxt, repo, fi, c, inf);

        gi_base_info_unref(fi);

        if (r == OX_ERR)
            goto end;
    }

    /*Load methods.*/
    n = gi_struct_info_get_n_methods(si);
    for (i = 0; i < n; i ++) {
        GIFunctionInfo *fi = gi_struct_info_get_method(si, i);
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
