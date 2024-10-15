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
 * Proxy object.
 */

#define OX_LOG_TAG "ox_proxy"

#include "ox_internal.h"

/** Proxy object.*/
typedef struct {
    OX_Object o;     /**< Base object data.*/
    OX_Value  templ; /**< Template object.*/
    OX_Value  thiz;  /**< This value.*/
} OX_Proxy;

/*Scan referenced objects in the proxy.*/
static void
proxy_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Proxy *p = (OX_Proxy*)gco;

    ox_object_scan(ctxt, gco);
    ox_gc_scan_value(ctxt, &p->templ);
    ox_gc_scan_value(ctxt, &p->thiz);
}

/*Free the proxy.*/
static void
proxy_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Proxy *p = (OX_Proxy*)gco;

    ox_object_deinit(ctxt, &p->o);

    OX_DEL(ctxt, p);
}

/*Get the property keys of the proxy object.*/
static OX_Result
proxy_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH(ctxt, fn)
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, keys), fn)) == OX_ERR)
        goto end;

    r = ox_call(ctxt, fn, &proxy->thiz, NULL, 0, keys);
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*Lookup the object owned the property.*/
static OX_Result
proxy_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH(ctxt, fn)
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, has), fn)) == OX_ERR)
        goto end;

    r = ox_call(ctxt, fn, &proxy->thiz, p, 1, v);
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*Get property value of a proxy object.*/
static OX_Result
proxy_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH(ctxt, fn)
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, get), fn)) == OX_ERR)
        goto end;

    r = ox_call(ctxt, fn, &proxy->thiz, p, 1, v);
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*Set property value of a proxy object.*/
static OX_Result
proxy_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_Value *fn = ox_value_stack_push_n(ctxt, 4);
    OX_Value *args = ox_values_item(ctxt, fn, 1);
    OX_Value *rv = ox_values_item(ctxt, fn, 3);
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, set), fn)) == OX_ERR)
        goto end;

    ox_value_copy(ctxt, args, p);
    ox_value_copy(ctxt, ox_values_item(ctxt, args, 1), v);

    r = ox_call(ctxt, fn, &proxy->thiz, args, 2, rv);
end:
    ox_value_stack_pop(ctxt, fn);
    return r;
}

/*Delete a property of the proxy object.*/
static OX_Result
proxy_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH_2(ctxt, fn, rv)
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, del), fn)) == OX_ERR)
        goto end;

    r = ox_call(ctxt, fn, &proxy->thiz, p, 1, rv);
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*Call the proxy object.*/
static OX_Result
proxy_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Proxy *proxy = ox_value_get_gco(ctxt, o);
    OX_Value *fn = ox_value_stack_push_n(ctxt, argc + 2);
    OX_Value *argv = ox_values_item(ctxt, fn, 1);
    OX_Result r;

    if ((r = ox_get_throw(ctxt, &proxy->templ, OX_STRING(ctxt, call), fn)) == OX_ERR)
        goto end;

    ox_value_copy(ctxt, argv, thiz);
    if (argc) {
        ox_values_copy(ctxt, ox_values_item(ctxt, argv, 1), args, argc);
    }

    r = ox_call(ctxt, fn, &proxy->templ, argv, argc + 1, rv);
end:
    ox_value_stack_pop(ctxt, fn);
    return r;
}

/*Proxy's operation functions.*/
static const OX_ObjectOps
proxy_ops = {
    {
        OX_GCO_PROXY,
        proxy_scan,
        proxy_free
    },
    proxy_keys,
    proxy_lookup,
    proxy_get,
    proxy_set,
    proxy_del,
    proxy_call
};

/*Proxy*/
static OX_Result
Proxy_create (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *templ = ox_argument(ctxt, args, argc, 0);
    OX_Value *tv = ox_argument(ctxt, args, argc, 1);
    OX_Proxy *p;

    if (!OX_NEW(ctxt, p))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &p->o, NULL);

    ox_value_set_null(ctxt, &p->templ);
    ox_value_set_null(ctxt, &p->thiz);

    p->o.gco.ops = (OX_GcObjectOps*)&proxy_ops;

    ox_value_set_gco(ctxt, rv, p);
    ox_gc_add(ctxt, p);

    ox_value_copy(ctxt, &p->templ, templ);
    ox_value_copy(ctxt, &p->thiz, tv);
    return OX_OK;
}

/**
 * Initialize the proxy classes.
 * @param ctxt The current running context.
 */
void
ox_proxy_class_init (OX_Context *ctxt)
{
    /*Proxy*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Global), "Proxy", Proxy_create));
}