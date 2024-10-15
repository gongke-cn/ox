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
 * GIObjectInfo.
 */

/*Convert the object information to value.*/
static OX_Result
giobjectinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIObjectInfo *oi, OX_Value *c)
{
    OX_VS_PUSH_4(ctxt, inf, v, pc, ic)
    GIObjectInfo *poi;
    OX_Result r;
    unsigned int i, n;

    OX_LOG_D(ctxt, "load object \"%s\"", gi_base_info_get_name(GI_BASE_INFO(oi)));

    if ((r = ox_class_new(ctxt, c, inf)) == OX_ERR)
        goto end;

    if ((r = gitype_add(ctxt, repo, c, GI_REGISTERED_TYPE_INFO(oi))) == OX_ERR)
        goto end;

    /*Parent.*/
    poi = gi_object_info_get_parent(oi);
    if (poi) {
        r = gibaseinfo_to_value(ctxt, repo, GI_BASE_INFO(poi), pc);
        if (r == OX_OK)
            r = ox_class_inherit(ctxt, c, pc);
        if (r == OX_OK)
            r = gisignal_inherit(ctxt, c, pc);

        gi_base_info_unref(poi);

        if (r == OX_ERR)
            goto end;
    }

    /*Interfaces.*/
    n = gi_object_info_get_n_interfaces(oi);
    for (i = 0; i < n; i ++) {
        GIInterfaceInfo *ii = gi_object_info_get_interface(oi, i);

        if ((r = gibaseinfo_to_value(ctxt, repo, GI_BASE_INFO(ii), ic)) == OX_OK)
            r = ox_class_inherit(ctxt, c, ic);
        if (r == OX_OK)
            r = gisignal_inherit(ctxt, c, ic);

        gi_base_info_unref(ii);

        if (r == OX_ERR)
            goto end;
    }

    /*Constants.*/
    n = gi_object_info_get_n_constants(oi);
    for (i = 0; i < n; i ++) {
        GIConstantInfo *ci = gi_object_info_get_constant(oi, i);

        r = gibaseinfo_to_const_prop(ctxt, repo, GI_BASE_INFO(ci), c, v);

        gi_base_info_unref(ci);

        if (r == OX_ERR)
            goto end;
    }

    /*Load methods.*/
    n = gi_object_info_get_n_methods(oi);
    for (i = 0; i < n; i ++) {
        GIFunctionInfo *fi = gi_object_info_get_method(oi, i);
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
    n = gi_object_info_get_n_signals(oi);
    for (i = 0; i < n; i ++) {
        GISignalInfo *si = gi_object_info_get_signal(oi, i);

        r = gisignal_add(ctxt, c, si);

        gi_base_info_unref(si);

        if (r == OX_ERR)
            goto end;
    }

    /*Add signal methods.*/
    if ((r = gisignal_add_methods(ctxt, repo, c, inf)) == OX_ERR)
        goto end;

    /*Properties.*/
    n = gi_object_info_get_n_properties(oi);
    for (i = 0; i < n; i ++) {
        GIPropertyInfo *pi = gi_object_info_get_property(oi, i);

        r = gipropertyinfo_add(ctxt, repo, pi, inf);

        gi_base_info_unref(pi);

        if (r == OX_ERR)
            goto end;
    }

    /*Virtual fnuctions.*/
    n = gi_object_info_get_n_vfuncs(oi);
    for (i = 0; i < n; i ++) {
        GIVFuncInfo *vi = gi_object_info_get_vfunc(oi, i);

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
