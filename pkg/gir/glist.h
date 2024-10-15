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
 * GList.
 */

/*Free the GList.*/
static void
glist_free (OX_Context *ctxt, OX_GICtxt *gic, GList *list, OX_GIOwn own, GITypeInfo *ti)
{
    if ((own == OX_GI_OWN_EVERYTHING) && gcontainer_elements_need_clean(ti)) {
        OX_GIParams p;

        p.ctxt = ctxt;
        p.gic = gic;
        p.ti0 = gi_type_info_get_param_type(ti, 0);

        g_list_foreach(list, gcontainer_free_item, &p);

        gi_base_info_unref(p.ti0);
    }

    g_list_free(list);
}

/*Convert the GList to OX value.*/
static OX_Result
glist_to_value (OX_Context *ctxt, OX_GICtxt *gic, GList *list, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
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

    g_list_foreach(list, gcontainer_add_item, &p);

    gi_base_info_unref(p.ti0);

    return p.r;
}

/*Convert the OX value to GList.*/
static OX_Result
glist_from_value (OX_Context *ctxt, OX_GICtxt *gic, GList **plist, OX_GIOwn *pown, GITypeInfo *ti, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, iter, item)
    GList *list = NULL;
    OX_GIOwn own = OX_GI_OWN_EVERYTHING;
    GITypeInfo *eti = gi_type_info_get_param_type(ti, 0);
    void *data;
    OX_Result r;

    if ((r = ox_iterator_new(ctxt, iter, v)) == OX_ERR)
        goto end;

    while (1) {
        if ((r = ox_iterator_end(ctxt, iter)) == OX_ERR)
            goto end;
        if (r)
            break;
        if ((r = ox_iterator_value(ctxt, iter, item)) == OX_ERR)
            goto end;

        if ((r = gcontainer_element_from_value(ctxt, gic, &data, eti, item)) == OX_ERR)
            goto end;

        list = g_list_append(list, data);

        if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
            goto end;
    }

    *plist = list;
    *pown = own;
    r = OX_OK;
end:
    if (eti)
        gi_base_info_unref(eti);
    if (r == OX_ERR) {
        if (list)
            glist_free(ctxt, gic, list, own, ti);
    }
    OX_VS_POP(ctxt, iter)
    return r;
}