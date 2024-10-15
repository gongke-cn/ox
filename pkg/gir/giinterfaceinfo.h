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
 * GIInterfaceInfo.
 */

/*Convert the interface information to value.*/
static OX_Result
giinterfaceinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIInterfaceInfo *ii, OX_Value *c)
{
    OX_VS_PUSH_2(ctxt, inf, v)
    OX_Result r;
    unsigned int i, n;

    OX_LOG_D(ctxt, "load interface \"%s\"", gi_base_info_get_name(GI_BASE_INFO(ii)));

    if ((r = ox_class_new(ctxt, c, inf)) == OX_ERR)
        goto end;

    if ((r = gitype_add(ctxt, repo, c, GI_REGISTERED_TYPE_INFO(ii))) == OX_ERR)
        goto end;

    /*Constants.*/
    n = gi_interface_info_get_n_constants(ii);
    for (i = 0; i < n; i ++) {
        GIConstantInfo *ci = gi_interface_info_get_constant(ii, i);

        r = gibaseinfo_to_const_prop(ctxt, repo, GI_BASE_INFO(ci), c, v);

        gi_base_info_unref(ci);

        if (r == OX_ERR)
            goto end;
    }

    /*Load methods.*/
    n = gi_interface_info_get_n_methods(ii);
    for (i = 0; i < n; i ++) {
        GIFunctionInfo *fi = gi_interface_info_get_method(ii, i);
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

    /*Signal.*/
    n = gi_interface_info_get_n_signals(ii);
    for (i = 0; i < n; i ++) {
        GISignalInfo *si = gi_interface_info_get_signal(ii, i);

        r = gisignal_add(ctxt, c, si);

        gi_base_info_unref(si);

        if (r == OX_ERR)
            goto end;
    }

    /*Properties.*/
    n = gi_interface_info_get_n_properties(ii);
    for (i = 0; i < n; i ++) {
        GIPropertyInfo *pi = gi_interface_info_get_property(ii, i);

        r = gipropertyinfo_add(ctxt, repo, pi, inf);

        gi_base_info_unref(pi);

        if (r == OX_ERR)
            goto end;
    }

    /*Virtual fnuctions.*/
    n = gi_interface_info_get_n_vfuncs(ii);
    for (i = 0; i < n; i ++) {
        GIVFuncInfo *vi = gi_interface_info_get_vfunc(ii, i);

        r = gibaseinfo_to_const_prop(ctxt, repo, GI_BASE_INFO(vi), inf, v);

        gi_base_info_unref(vi);

        if (r == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, inf)
    return r;
}
