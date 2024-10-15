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
 * Interface.
 */

#define OX_LOG_TAG "ox_interface"

#include "ox_internal.h"

/*Scan referenced objects in the interface.*/
static void
interface_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Interface *inf = (OX_Interface*)gco;
    size_t i;
    OX_HashEntry *e;

    ox_object_scan(ctxt, gco);

    ox_hash_foreach(&inf->i_hash, i, e) {
        ox_gc_mark(ctxt, e->key);
    }
}

/*Free the interface.*/
static void
interface_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Interface *inf = (OX_Interface*)gco;
    size_t i;
    OX_HashEntry *e, *ne;

    ox_hash_foreach_safe(&inf->i_hash, i, e, ne) {
        OX_DEL(ctxt, e);
    }
    ox_hash_deinit(ctxt, &inf->i_hash);

    ox_object_deinit(ctxt, &inf->o);
    OX_DEL(ctxt, inf);
}

/*Interface's operation functions.*/
static const OX_ObjectOps
interface_ops = {
    {
        OX_GCO_INTERFACE,
        interface_scan,
        interface_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    ox_object_call
};

/**
 * Create a new interface.
 * @param ctxt The current running context.
 * @param[out] inf Return the new interface.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_interface_new (OX_Context *ctxt, OX_Value *inf)
{
    OX_Interface *ip;

    assert(ctxt && inf);

    if (!OX_NEW(ctxt, ip))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &ip->o, NULL);
    ip->o.gco.ops = (OX_GcObjectOps*)&interface_ops;
    ox_size_hash_init(&ip->i_hash);

    ox_value_set_gco(ctxt, inf, ip);
    ox_gc_add(ctxt, ip);

    return OX_OK;
}

/*Add the inherited interface to the hash table.*/
static OX_Result
add_inherit (OX_Context *ctxt, OX_Interface *inf, OX_Interface *iinf)
{
    OX_HashEntry *e, **pe;
    OX_Result r;

    e = ox_hash_lookup(ctxt, &inf->i_hash, iinf, &pe);
    if (e)
        return OX_OK;

    if (!OX_NEW(ctxt, e))
        return ox_throw_no_mem_error(ctxt);

    if ((r = ox_hash_insert(ctxt, &inf->i_hash, iinf, e, pe)) == OX_ERR)
        OX_DEL(ctxt, e);

    return r;
}

/**
 * Inherit properties from the parent interface.
 * @param ctxt The current running context.
 * @param inf The interface.
 * @param pinf The parent interface.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_interface_inherit (OX_Context *ctxt, OX_Value *inf, OX_Value *pinf)
{
    OX_VS_PUSH(ctxt, pn)
    OX_Interface *ip, *iip;
    OX_Property *p;
    size_t i;
    OX_HashEntry *e;
    OX_Result r;

    assert(ctxt && inf && pinf);
    assert(ox_value_is_interface(ctxt, inf));
    assert(ox_value_is_interface(ctxt, pinf));

    ip = ox_value_get_gco(ctxt, inf);
    iip = ox_value_get_gco(ctxt, pinf);

    /*Add properties.*/
    ox_list_foreach_c(&iip->o.p_list, p, OX_Property, ln) {
        ox_value_set_gco(ctxt, pn, p->he.key);

        if (ox_string_equal(ctxt, pn, OX_STRING(ctxt, _class))
                || ox_string_equal(ctxt, pn, OX_STRING(ctxt, _scope)))
            continue;

        switch (p->type) {
        case OX_PROPERTY_CONST:
        case OX_PROPERTY_VAR:
            r = ox_object_add_prop(ctxt, inf, p->type, pn, &p->p.v, NULL);
            break;
        case OX_PROPERTY_ACCESSOR:
            r = ox_object_add_prop(ctxt, inf, p->type, pn, &p->p.a.get, &p->p.a.set);
            break;
        default:
            assert(0);
            break;
        }

        if (r == OX_ERR)
            goto end;
    }

    /*Add the interface.*/
    if ((r = add_inherit(ctxt, ip, iip)) == OX_ERR)
        goto end;

    ox_hash_foreach(&iip->i_hash, i, e) {
        if ((r = add_inherit(ctxt, ip, e->key)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, pn)
    return r;
}

/**
 * Check if the interface has implement the parent interface.
 * @param ctxt The current running context.
 * @param inf The interface.
 * @param pinf The parent interface.
 * @retval OX_TRUE inf has implemented the parent interface.
 * @retval OX_FALSE inf has not implemented the parent interface.
 * @retval OX_ERR On error.
 */
OX_Result
ox_interface_impl (OX_Context *ctxt, OX_Value *inf, OX_Value *pinf)
{
    OX_Interface *pi;
    OX_Interface *ppi;

    assert(ctxt && inf && pinf);
    
    if (!ox_value_is_interface(ctxt, inf))
        return ox_equal(ctxt, inf, pinf);

    pi = ox_value_get_gco(ctxt, inf);
    ppi = ox_value_get_gco(ctxt, pinf);

    if (pi == ppi)
        return OX_TRUE;

    if (ox_hash_lookup(ctxt, &pi->i_hash, ppi, NULL))
        return OX_TRUE;

    return OX_FALSE;
}
