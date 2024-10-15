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
 * GIPropertyInfo.
 */

/*Add a property information.*/
static OX_Result
gipropertyinfo_add (OX_Context *ctxt, OX_GIRepository *repo, GIPropertyInfo *pi, OX_Value *inf)
{
    OX_VS_PUSH_2(ctxt, get, set)
    GIFunctionInfo *fi;
    GParamFlags flags;
    const char *name;
    OX_Result r;

    name = gi_base_info_get_name(GI_BASE_INFO(pi));
    flags = gi_property_info_get_flags(pi);

    if (flags & G_PARAM_READABLE) {
        fi = gi_property_info_get_getter(pi);

        if (fi) {
            r = gifunctioninfo_to_value(ctxt, repo, fi, get);

            gi_base_info_unref(fi);

            if (r == OX_ERR)
                goto end;
        }
    }

    if (flags & G_PARAM_WRITABLE) {
        fi = gi_property_info_get_setter(pi);

        if (fi) {
            r = gifunctioninfo_to_value(ctxt, repo, fi, set);

            gi_base_info_unref(fi);

            if (r == OX_ERR)
                goto end;
        }
    }

    if (!ox_value_is_null(ctxt, get) || !ox_value_is_null(ctxt, set)) {
        if ((r = ox_object_add_accessor_s(ctxt, inf, name, get, set)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, get)
    return r;
}
