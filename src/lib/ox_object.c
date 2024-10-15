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
 * Object.
 */

#define OX_LOG_TAG "ox_object"

#include "ox_internal.h"

/** Object iterator data.*/
typedef struct {
    OX_ObjectIterType type; /**< The iterator type.*/
    OX_Value          keys; /**< The keys of the object.*/
    OX_Value          o;    /**< The object.*/
    size_t            id;   /**< Current key's index.*/
} OX_ObjectIter;

/*Scan referenced objects in the property.*/
static void
prop_scan (OX_Context *ctxt, OX_Property *p)
{
    ox_gc_mark(ctxt, p->he.key);

    switch (p->type) {
    case OX_PROPERTY_CONST:
    case OX_PROPERTY_VAR:
        ox_gc_scan_value(ctxt, &p->p.v);
        break;
    case OX_PROPERTY_ACCESSOR:
        ox_gc_scan_value(ctxt, &p->p.a.get);
        ox_gc_scan_value(ctxt, &p->p.a.set);
        break;
    }
}

/*Free the property.*/
static void
prop_free (OX_Context *ctxt, OX_Property *p)
{
    OX_DEL(ctxt, p);
}

/**
 * Initialize an object.
 * @param ctxt The current running context.
 * @param o The object to be initialized.
 * @param inf The interface of the object.
 */
void
ox_object_init (OX_Context *ctxt, OX_Object *o, OX_Value *inf)
{
    ox_list_init(&o->p_list);
    ox_size_hash_init(&o->p_hash);

    o->priv_ops = NULL;
    o->priv = NULL;

    if (inf)
        ox_value_copy(ctxt, &o->inf, inf);
    else
        ox_value_set_null(ctxt, &o->inf);
}

/**
 * Release the object.
 * @param ctxt The current running context.
 * @param o The object to released.
 */
void
ox_object_deinit (OX_Context *ctxt, OX_Object *o)
{
    OX_Property *p, *np;

    ox_list_foreach_safe_c(&o->p_list, p, np, OX_Property, ln) {
        prop_free(ctxt, p);
    }

    ox_hash_deinit(ctxt, &o->p_hash);

    if (o->priv) {
        if (o->priv_ops && o->priv_ops->free)
            o->priv_ops->free(ctxt, o->priv);
    }
}

/**
 * Scan referenced objects in the object.
 * @param ctxt The current running context.
 * @param gco The object to be scanned.
 */
void
ox_object_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Object *o = (OX_Object*)gco;
    OX_Property *p;

    ox_gc_scan_value(ctxt, &o->inf);

    ox_list_foreach_c(&o->p_list, p, OX_Property, ln) {
        prop_scan(ctxt, p);
    }

    if (o->priv) {
        if (o->priv_ops && o->priv_ops->scan)
            o->priv_ops->scan(ctxt, o->priv);
    }
}

/**
 * Scan referenced objects in the object.
 * @param ctxt The current running context.
 * @param gco The object to be scanned.
 */
void
ox_object_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Object *o = (OX_Object*)gco;

    ox_object_deinit(ctxt, o);

    OX_DEL(ctxt, o);
}

/*Get the property's value.*/
static OX_Result
prop_get (OX_Context *ctxt, OX_Value *thiz, OX_Property *prop, OX_Value *v)
{
    OX_Result r;

    switch (prop->type) {
    case OX_PROPERTY_CONST:
    case OX_PROPERTY_VAR:
        ox_value_copy(ctxt, v, &prop->p.v);
        r = OX_OK;
        break;
    case OX_PROPERTY_ACCESSOR:
        r = ox_call(ctxt, &prop->p.a.get, thiz, NULL, 0, v);
        break;
    default:
        assert(0);
    }
    
    return r;
}

/**
 * Lookup the object owned the property.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Object *op;
    OX_Result r;

    assert(ox_value_is_object(ctxt, o));

    op = ox_value_get_gco(ctxt, o);

    if (ox_value_is_string(ctxt, p)) {
        if ((r = ox_string_singleton(ctxt, p)) == OX_OK) {
            OX_String *s = ox_value_get_gco(ctxt, p);
            OX_Property *prop;

            prop = ox_hash_lookup_c(ctxt, &op->p_hash, s, NULL, OX_Property, he);
            if (prop)
                return prop_get(ctxt, o, prop, v);
        }
    }

    ox_value_set_null(ctxt, v);
    return OX_OK;
}

/**
 * Get the property value of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_object_get_t(ctxt, o, p, v, o);
}

/**
 * Get the property value of an object with this argument.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @param thiz This argument.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_get_t (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Value *thiz)
{
    OX_Object *op;
    OX_Result r;

    assert(ox_value_is_object(ctxt, o));

    op = ox_value_get_gco(ctxt, o);

    if (ox_value_is_string(ctxt, p)) {
        if ((r = ox_string_singleton(ctxt, p)) == OX_OK) {
            OX_String *s = ox_value_get_gco(ctxt, p);
            OX_Property *prop;

            prop = ox_hash_lookup_c(ctxt, &op->p_hash, s, NULL, OX_Property, he);
            if (!prop && !ox_value_is_null(ctxt, &op->inf)) {
                op = ox_value_get_gco(ctxt, &op->inf);
                prop = ox_hash_lookup_c(ctxt, &op->p_hash, s, NULL, OX_Property, he);
            }

            if (prop) {
                r = prop_get(ctxt, thiz, prop, v);
            } else {
                r = OX_FALSE;
            }
        }
    } else {
        r = OX_FALSE;
    }

    return r;
}

/**
 * Set the property value of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param v The property's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_object_set_t(ctxt, o, p, v, o);
}

/*Throw access error.*/
static OX_Result
access_error (OX_Context *ctxt, OX_Value *p)
{
    return ox_throw_access_error(ctxt, OX_TEXT("property \"%s\" cannot be set"),
            ox_string_get_char_star(ctxt, p));
}

/**
 * Set the property value of an object with this argument.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param v The property's new value.
 * @param thiz This argument.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_t (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Value *thiz)
{
    OX_VS_PUSH(ctxt, rv)
    OX_Result r;

    assert(ox_value_is_object(ctxt, o));

    if (ox_value_is_string(ctxt, p)) {
        if ((r = ox_string_singleton(ctxt, p)) == OX_OK) {
            OX_Object *op = ox_value_get_gco(ctxt, o);
            OX_HashEntry **pe;
            OX_String *s = ox_value_get_gco(ctxt, p);
            OX_Property *prop;

            prop = ox_hash_lookup_c(ctxt, &op->p_hash, s, &pe, OX_Property, he);
            if (!prop && !ox_value_is_null(ctxt, &op->inf)) {
                OX_Object *iop = ox_value_get_gco(ctxt, &op->inf);

                prop = ox_hash_lookup_c(ctxt, &iop->p_hash, s, NULL, OX_Property, he);
            }

            if (prop) {
                switch (prop->type) {
                case OX_PROPERTY_CONST:
                    r = access_error(ctxt, p);
                    break;
                case OX_PROPERTY_VAR:
                    ox_value_copy(ctxt, &prop->p.v, v);
                    r = OX_OK;
                    break;
                case OX_PROPERTY_ACCESSOR:
                    if (ox_value_is_null(ctxt, &prop->p.a.set))
                        r = access_error(ctxt, p);
                    else
                        r = ox_call(ctxt, &prop->p.a.set, thiz, v, 1, rv);
                    break;
                default:
                    assert(0);
                }
            } else {
                if (!OX_NEW(ctxt, prop)) {
                    r = ox_throw_no_mem_error(ctxt);
                } else {
                    prop->type = OX_PROPERTY_VAR;
                    ox_value_copy(ctxt, &prop->p.v, v);
                    ox_list_append(&op->p_list, &prop->ln);
                    if ((r = ox_hash_insert(ctxt, &op->p_hash, s, &prop->he, pe)) == OX_ERR) {
                        OX_DEL(ctxt, prop);
                    }
                }
            }
        }
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("object's property must be a string"));
    }

    OX_VS_POP(ctxt, rv)
    return r;
}

/**
 * Call the object value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Count of the arguments.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args,
        size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, f)
    OX_Result r;

    r = ox_get(ctxt, o, OX_STRING(ctxt, _call), f);
    if (r == OX_OK) {
        if (ox_value_is_null(ctxt, f))
            ox_value_copy(ctxt, rv, o);
        else
            r = ox_call(ctxt, f, thiz, args, argc, rv);
    }

    OX_VS_POP(ctxt, f)
    return r;
}

/*Object's operation functions.*/
static const OX_ObjectOps
object_ops = {
    {
        OX_GCO_OBJECT,
        ox_object_scan,
        ox_object_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    ox_object_call
};

/**
 * Create a new object.
 * @param ctxt The current running context.
 * @param[out] o Return the new object.
 * @param inf The interface of the object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_new (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Object *op;

    assert(ctxt && o);

    if (!OX_NEW(ctxt, op))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, op, inf);
    op->gco.ops = (OX_GcObjectOps*)&object_ops;

    ox_value_set_gco(ctxt, o, op);
    ox_gc_add(ctxt, op);
    return OX_OK;
}

/**
 * Create a new object from a class.
 * @param ctxt The current running context.
 * @param[out] o Return the new object.
 * @param c The class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_from_class (OX_Context *ctxt, OX_Value *o, OX_Value *c)
{
    OX_VS_PUSH(ctxt, inf)
    OX_Result r;

    if (c) {
        if ((r = ox_get(ctxt, c, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
            goto end;
    }

    r = ox_object_new(ctxt, o, inf);
end:
    OX_VS_POP(ctxt, inf)
    return r;
}

/**
 * Set the object's private data.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param ops The private data's operation functions.
 * @param data The private data.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_priv (OX_Context *ctxt, OX_Value *o, const OX_PrivateOps *ops, void *data)
{
    OX_Object *op;

    assert(ctxt && o && ops);
    assert(ox_value_is_object(ctxt, o));

    op = ox_value_get_gco(ctxt, o);

    if (op->priv_ops && (op->priv_ops != ops))
        return ox_throw_type_error(ctxt, OX_TEXT("private data's type mismatch"));

    if (op->priv) {
        if (op->priv_ops && op->priv_ops->free)
            op->priv_ops->free(ctxt, op->priv);
    }

    op->priv_ops = ops;
    op->priv = data;
    return OX_OK;
}

/**
 * Get the object's private data.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param ops The private data's operation functions.
 * @return The private data.
 * @retval NULL The private data is not set.
 */
void*
ox_object_get_priv (OX_Context *ctxt, OX_Value *o, const OX_PrivateOps *ops)
{
    OX_Object *op;

    assert(ctxt && o && ops);

    if (!ox_value_is_object(ctxt, o))
        return NULL;

    op = ox_value_get_gco(ctxt, o);

    if (op->priv_ops != ops)
        return NULL;

    return op->priv;
}

/**
 * Set the object's scope.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param scope The scope object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_scope (OX_Context *ctxt, OX_Value *o, OX_Value *scope)
{
    assert(ctxt && o && scope);
    assert(ox_value_is_object(ctxt, o));
    assert(ox_value_is_object(ctxt, scope));

    return ox_object_add_const(ctxt, o, OX_STRING(ctxt, _scope), scope);
}

/**
 * Set the object's name.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_name (OX_Context *ctxt, OX_Value *o, OX_Value *name)
{
    assert(ctxt && o && name);
    assert(ox_value_is_object(ctxt, o));
    assert(ox_value_is_string(ctxt, name));

    return ox_object_add_const(ctxt, o, OX_STRING(ctxt, _name), name);
}

/**
 * Set the object's name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_name_s (OX_Context *ctxt, OX_Value *o, const char *name)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, nv, name)) == OX_OK)
        r = ox_object_set_name(ctxt, o, nv);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Add a property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param type The property's type.
 * @param name The name of the property.
 * @param v1 Value 1 of the property.
 * @param v2 Value 2 of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_prop (OX_Context *ctxt, OX_Value *o, OX_PropertyType type,
        OX_Value *name, OX_Value *v1, OX_Value *v2)
{
    OX_Object *op;
    OX_HashEntry **pe;
    OX_String *s;
    OX_Property *p;
    OX_Result r;
    OX_Bool need_reset = OX_FALSE;

    assert(ctxt && o && name);
    assert(ox_value_is_object(ctxt, o));
    assert(ox_value_is_string(ctxt, name));

    if ((r = ox_string_singleton(ctxt, name)) == OX_ERR)
        return r;

    op = ox_value_get_gco(ctxt, o);

    s = ox_value_get_gco(ctxt, name);
    p = ox_hash_lookup_c(ctxt, &op->p_hash, s, &pe, OX_Property, he);
    if (!p) {
        if (!OX_NEW(ctxt, p))
            return ox_throw_no_mem_error(ctxt);

        if ((r = ox_hash_insert(ctxt, &op->p_hash, s, &p->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, p);
            return r;
        }
        ox_list_append(&op->p_list, &p->ln);

        need_reset = OX_TRUE;
    } else if (type != p->type) {
        need_reset = OX_TRUE;
    }

    if (need_reset) {
        p->type = type;

        if (type == OX_PROPERTY_ACCESSOR) {
            ox_value_set_null(ctxt, &p->p.a.get);
            ox_value_set_null(ctxt, &p->p.a.set);
        } else {
            ox_value_set_null(ctxt, &p->p.v);
        }
    }

    switch (type) {
    case OX_PROPERTY_CONST:
    case OX_PROPERTY_VAR:
        if (v1)
            ox_value_copy(ctxt, &p->p.v, v1);
        break;
    case OX_PROPERTY_ACCESSOR:
        if (v1)
            ox_value_copy(ctxt, &p->p.a.get, v1);

        if (v2)
            ox_value_copy(ctxt, &p->p.a.set, v2);
        break;
    default:
        assert(0);
    }

    return OX_OK;
}

/**
 * Add a property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param type The property's type.
 * @param name The name of the property.
 * @param v1 Value 1 of the property.
 * @param v2 Value 2 of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_prop_s (OX_Context *ctxt, OX_Value *o, OX_PropertyType type,
        const char *name, OX_Value *v1, OX_Value *v2)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, nv, name)) == OX_OK)
        r = ox_object_add_prop(ctxt, o, type, nv, v1, v2);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Add a native method property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param cf The C function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_n_method (OX_Context *ctxt, OX_Value *o,
        OX_Value *name, OX_CFunc cf)
{
    OX_VS_PUSH(ctxt, f)
    OX_Result r;

    if ((r = ox_named_native_func_new(ctxt, f, cf, o, name)) == OX_ERR)
        goto end;

    r = ox_object_add_prop(ctxt, o, OX_PROPERTY_CONST, name, f, NULL);
end:
    OX_VS_POP(ctxt, f)
    return r;
}

/**
 * Add a native method property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param cf The C function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_n_method_s (OX_Context *ctxt, OX_Value *o,
        const char *name, OX_CFunc cf)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, nv, name)) == OX_OK)
        r = ox_object_add_n_method(ctxt, o, nv, cf);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Add a native accessor property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param get The C getter function's pointer.
 * @param set The C setter function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_n_accessor (OX_Context *ctxt, OX_Value *o,
        OX_Value *name, OX_CFunc get, OX_CFunc set)
{
    OX_VS_PUSH_3(ctxt, fn, gf, sf)
    OX_CharBuffer cb;
    size_t len;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_char_buffer_append_string(ctxt, &cb, name)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_char(ctxt, &cb, ':')) == OX_ERR)
        goto end;

    len = cb.len;

    if (get) {
        if ((r = ox_char_buffer_append_char_star(ctxt, &cb, "get")) == OX_ERR)
            goto end;
        if ((r = ox_char_buffer_get_string(ctxt, &cb, fn)) == OX_ERR)
            goto end;
        if ((r = ox_named_native_func_new(ctxt, gf, get, o, fn)) == OX_ERR)
            goto end;
    }

    if (set) {
        cb.len = len;
        if ((r = ox_char_buffer_append_char_star(ctxt, &cb, "set")) == OX_ERR)
            goto end;
        if ((r = ox_char_buffer_get_string(ctxt, &cb, fn)) == OX_ERR)
            goto end;
        if ((r = ox_named_native_func_new(ctxt, sf, set, o, fn)) == OX_ERR)
            goto end;
    }

    r = ox_object_add_prop(ctxt, o, OX_PROPERTY_ACCESSOR, name, gf, sf);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, fn)
    return r;
}

/**
 * Add a native accessor property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param get The C getter function's pointer.
 * @param set The C setter function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_add_n_accessor_s (OX_Context *ctxt, OX_Value *o,
        const char *name, OX_CFunc get, OX_CFunc set)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, nv, name)) == OX_OK) {
        r = ox_object_add_n_accessor(ctxt, o, nv, get, set);
    }

    OX_VS_POP(ctxt, nv)
    return r;
}

/*Scan referenced objects in the object iterator.*/
static void
object_iter_scan (OX_Context *ctxt, void *ptr)
{
    OX_ObjectIter *oi = ptr;

    ox_gc_scan_value(ctxt, &oi->keys);
    ox_gc_scan_value(ctxt, &oi->o);
}

/*Free the object iterator.*/
static void
object_iter_free (OX_Context *ctxt, void *ptr)
{
    OX_ObjectIter *oi = ptr;

    OX_DEL(ctxt, oi);
}

/*Object iterator data's operation functions.*/
static const OX_PrivateOps
object_iter_ops = {
    object_iter_scan,
    object_iter_free
};

/*Get the object iterator data.*/
static OX_ObjectIter*
get_object_iter (OX_Context *ctxt, OX_Value *v)
{
    OX_ObjectIter *oi;

    if (!(oi = ox_object_get_priv(ctxt, v, &object_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not an object iterator"));

    return oi;
}

/**
 * Create a new object iterator.
 * @param ctxt The current running context.
 * @param[out] iter Return the new iterator,
 * @param o The object value.
 * @param type The iterator's type.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *o, OX_ObjectIterType type)
{
    OX_VS_PUSH(ctxt, keys)
    OX_ObjectIter *oi;
    OX_Result r;

    assert(ctxt && iter && o);
    assert(ox_value_is_object(ctxt, o));

    if ((r = ox_keys(ctxt, o, keys)) == OX_ERR)
        goto end;

    if (!ox_value_is_array(ctxt, keys)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("keys must be an array"));
        goto end;
    }

    if ((r = ox_object_new(ctxt, iter, OX_OBJECT(ctxt, ObjectIterator_inf))) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, oi)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_copy(ctxt, &oi->o, o);
    ox_value_copy(ctxt, &oi->keys, keys);
    oi->type = type;
    oi->id = 0;

    if ((r = ox_object_set_priv(ctxt, iter, &object_iter_ops, oi)) == OX_ERR) {
        object_iter_free(ctxt, oi);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, keys);
    return r;
}

/**
 * Delete a property of the object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param p The property name.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    OX_Object *op;
    OX_String *s;
    OX_HashEntry **pe;
    OX_Property *prop;
    OX_Result r;

    op = ox_value_get_gco(ctxt, o);

    if ((r = ox_string_singleton(ctxt, p)) == OX_ERR)
        return r;

    s = ox_value_get_gco(ctxt, p);
    prop = ox_hash_lookup_c(ctxt, &op->p_hash, s, &pe, OX_Property, he);
    if (prop) {
        ox_hash_remove(ctxt, &op->p_hash, s, pe);
        ox_list_remove(&prop->ln);
        prop_free(ctxt, prop);
    }

    return OX_OK;
}

/**
 * Get the keys of the object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param[out] keys Return the property keys array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    OX_VS_PUSH(ctxt, key)
    OX_Object *op;
    OX_Result r;

    if ((r = ox_get(ctxt, o, OX_STRING(ctxt, _keys), keys)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, keys)) {
        op = ox_value_get_gco(ctxt, o);
        OX_Property *p;

        if ((r = ox_array_new(ctxt, keys, 0)) == OX_ERR)
            goto end;

        ox_list_foreach_c(&op->p_list, p, OX_Property, ln) {
            OX_String *s = p->he.key;

            if (s->chars[0] == '#')
                continue;

            ox_value_set_gco(ctxt, key, p->he.key);
            if ((r = ox_array_append(ctxt, keys, key)) == OX_ERR)
                goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, key)
    return r;
}

/**
 * Set the keys of an object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param keys The keys array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_object_set_keys (OX_Context *ctxt, OX_Value *o, const char **keys)
{
    OX_VS_PUSH_2(ctxt, ka, k)
    OX_Result r;

    assert(ctxt && o);
    assert(ox_value_is_object(ctxt, o));

    if ((r = ox_array_new(ctxt, ka, 0)) == OX_ERR)
        goto end;

    if (keys) {
        const char **key = keys;

        while (*key) {
            if ((r = ox_string_from_const_char_star(ctxt, k, *key)) == OX_ERR)
                goto end;
            if ((r = ox_string_singleton(ctxt, k)) == OX_ERR)
                goto end;
            if ((r = ox_array_append(ctxt, ka, k)) == OX_ERR)
                goto end;

            key ++;
        }
    }

    if ((r = ox_set(ctxt, o, OX_STRING(ctxt, _keys), ka)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, ka)
    return r;
}

/*Check if the value is an object.*/
static OX_Result
check_object (OX_Context *ctxt, OX_Value *o)
{
    if (!ox_value_is_object(ctxt, o))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not an object"));

    return OX_OK;
}

/*Check if the value is an object or null.*/
static OX_Result
check_object_null (OX_Context *ctxt, OX_Value *o)
{
    if (!ox_value_is_object(ctxt, o) && !ox_value_is_null(ctxt, o))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not an object or null"));

    return OX_OK;
}

/*Object.entries*/
static OX_Result
object_entries (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = check_object_null(ctxt, o)) == OX_ERR)
        return r;

    if (ox_value_is_null(ctxt, o)) {
        ox_value_set_null(ctxt, rv);
        return OX_OK;
    }

    return ox_object_iter_new(ctxt, rv, o, OX_OBJECT_ITER_KEY_VALUE);
}

/*Object.keys*/
static OX_Result
object_keys (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = check_object_null(ctxt, o)) == OX_ERR)
        return r;

    if (ox_value_is_null(ctxt, o)) {
        ox_value_set_null(ctxt, rv);
        return OX_OK;
    }

    return ox_object_iter_new(ctxt, rv, o, OX_OBJECT_ITER_KEY);
}

/*Object.values*/
static OX_Result
object_values (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = check_object_null(ctxt, o)) == OX_ERR)
        return r;

    if (ox_value_is_null(ctxt, o)) {
        ox_value_set_null(ctxt, rv);
        return OX_OK;
    }

    return ox_object_iter_new(ctxt, rv, o, OX_OBJECT_ITER_VALUE);
}

/*Object.lookup*/
static OX_Result
object_lookup (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Result r;

    if ((r = check_object_null(ctxt, o)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, o)) {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
        goto end;
    }

    r = ox_lookup(ctxt, o, p, rv);
end:
    return r;
}

/*Object.get*/
static OX_Result
object_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Result r;

    if ((r = check_object_null(ctxt, o)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, o)) {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
        goto end;
    }

    r = ox_get(ctxt, o, p, rv);
end:
    return r;
}

/*Object.set*/
static OX_Result
object_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Value *pv = ox_argument(ctxt, args, argc, 2);
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    r = ox_set(ctxt, o, p, pv);
end:
    return r;
}

/*Object.add_const*/
static OX_Result
object_add_const (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Value *v = ox_argument(ctxt, args, argc, 2);
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, p, s)) == OX_ERR)
        goto end;

    r = ox_object_add_const(ctxt, o, s, v);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Object.add_var*/
static OX_Result
object_add_var (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Value *v = ox_argument(ctxt, args, argc, 2);
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, p, s)) == OX_ERR)
        goto end;

    r = ox_object_add_var(ctxt, o, s, v);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Object.add_accessor*/
static OX_Result
object_add_accessor (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Value *get = ox_argument(ctxt, args, argc, 2);
    OX_Value *set = ox_argument(ctxt, args, argc, 3);
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, p, s)) == OX_ERR)
        goto end;

    r = ox_object_add_accessor(ctxt, o, s, get, set);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Object.del_prop*/
static OX_Result
object_del_prop (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *p = ox_argument(ctxt, args, argc, 1);
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    r = ox_del(ctxt, o, p);
end:
    return r;
}

/*Object.get_name.*/
static OX_Result
object_get_name (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);

    return ox_get_full_name(ctxt, o, rv);
}

/*Object.unref.*/
static OX_Result
object_unref (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if (ox_value_is_gco(ctxt, o, -1))
        r = ox_global_unref(ctxt, o);
    else
        r = OX_OK;

    return r;
}

/*Object.is.*/
static OX_Result
object_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_object(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Object.set_inf.*/
static OX_Result
object_set_inf (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *o = ox_argument(ctxt, args, argc, 0);
    OX_Value *inf = ox_argument(ctxt, args, argc, 1);
    OX_Object *op;
    OX_Result r;

    if ((r = check_object(ctxt, o)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, inf) && !ox_value_is_object(ctxt, inf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is neither an object nor null"));
        goto end;
    }

    op = ox_value_get_gco(ctxt, o);
    ox_value_copy(ctxt, &op->inf, inf);

    r = OX_OK;
end:
    return r;
}

/*ObjectIterator.$inf.next*/
static OX_Result
ObjectIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ObjectIter *oi;
    size_t len;

    if (!(oi = get_object_iter(ctxt, thiz)))
        return OX_ERR;

    len = ox_array_length(ctxt, &oi->keys);

    if (oi->id < len)
        oi->id ++;

    return OX_OK;
}

/*ObjectIterator.$inf.end get*/
static OX_Result
ObjectIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ObjectIter *oi;
    size_t len;
    OX_Bool b;

    if (!(oi = get_object_iter(ctxt, thiz))) {
        return OX_ERR;
    }

    len = ox_array_length(ctxt, &oi->keys);
    b = (oi->id >= len);

    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*ObjectIterator.$inf.value get*/
static OX_Result
ObjectIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, key, val)
    OX_ObjectIter *oi;
    size_t len;
    OX_Result r;

    if (!(oi = get_object_iter(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    len = ox_array_length(ctxt, &oi->keys);

    if (oi->id < len) {
        if ((r = ox_array_get_item(ctxt, &oi->keys, oi->id, key)) == OX_ERR)
            goto end;

        switch (oi->type) {
        case OX_OBJECT_ITER_KEY:
            ox_value_copy(ctxt, rv, key);
            break;
        case OX_OBJECT_ITER_VALUE:
            if ((r = ox_get(ctxt, &oi->o, key, rv)) == OX_ERR)
                goto end;
            break;
        case OX_OBJECT_ITER_KEY_VALUE:
            if ((r = ox_array_new(ctxt, rv, 2)) == OX_ERR)
                goto end;
            if ((r = ox_array_set_item(ctxt, rv, 0, key)) == OX_ERR)
                goto end;
            if ((r = ox_get(ctxt, &oi->o, key, val)) == OX_ERR)
                goto end;
            if ((r = ox_array_set_item(ctxt, rv, 1, val)) == OX_ERR)
                goto end;
            break;
        }
    } else {
        ox_value_set_null(ctxt, rv);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, key);
    return r;
}

/*?
 *? @lib {Object} Generic object.
 *?
 *? @class{ Object Class of the generic object.
 *?
 *? @sfunc entries Create an iterator the traverse the properties of the object.
 *? The value of iterator is a 2 items array.
 *? Item 0 is the name of the property.
 *? Item 1 is the value of the property.
 *? @return {Iterator[[String,Any]]} The iterator used to traverse the properties.
 *?
 *? @sfunc keys Create an iterator the traverse the properties' names of the object.
 *? The value of iterator is the name of the property.
 *? @param o {Object} The object.
 *? @return {Iterator[String]} The iterator used to traverse the properties' names.
 *?
 *? @sfunc values Create an iterator the traverse the properties' values of the object.
 *? The value of iterator is the value of the property.
 *? @param o {Object} The object.
 *? @return {Iterator[Any]} The iterator used to traverse the properties' values.
 *?
 *? @sfunc lookup Lookup the object owned property.
 *? @param o {Object} The object.
 *? @param name {String} The property's name.
 *? @return {?Any} The property's value.
 *? If the object has not the property, return null.
 *?
 *? @sfunc get Get the property value of the object.
 *? @param o {Object} The object.
 *? @param name {String} The property's name.
 *? @return {?Any} The property's value.
 *? If the object has not the property, return null.
 *?
 *? @sfunc set Set the property value of the object.
 *? @param o {Object} The object.
 *? @param name {String} The property's name.
 *? @param v The property's new value.
 *?
 *? @sfunc add_const Add a constant property to the object.
 *? @param o {Object} The object.
 *? @param name {String} The name of the property.
 *? @param v {Any} The value of the property.
 *?
 *? @sfunc add_var Add a vriable property to the object.
 *? @param o {Object} The object.
 *? @param name {String} The name of the property.
 *? @param v {Any} The value of the property.
 *?
 *? @sfunc add_accessor Add an accessor property to the object.
 *? @param o {Object} The object.
 *? @param name {String} The name of the property.
 *? @param get {Function} The getter function of the accessor.
 *? @param set {Function} The setter function of the accessor.
 *?
 *? @sfunc del_prop Delete a property from the object.
 *? @param o {Object} The object.
 *? @param name {String} The name of the property.
 *?
 *? @sfunc get_name Get the full name of a class or a function object.
 *? @param o {Object} The object.
 *? @return {String} The full name of the object.
 *?
 *? @sfunc unref Decrease the global referece counter of an object.
 *? Object is managed by GC. When there is no internal reference, the object will be released by GC.
 *? Sometimes, for the security of external C library access, OX language can use "global"
 *? operator to increase the global reference counter of the object.
 *? When an object's global reference counter > 0, it will not be released.
 *? "unref" function is used to decrease the global reference counter when the external C library
 *? no longer require access the object.
 *? @param o {Object} The object.
 *?
 *? @sfunc is Check if the value is an object.
 *? @param v The value.
 *? @return {Bool} The value is an object or not.
 *?
 *? @sfunc set_inf Set the object's interface.
 *? @param o {Object} The object.
 *? @param inf {Object} The new interface object. 
 *?
 *? @class}
 */

/**
 * Initialize the object class.
 * @param ctxt The current running context.
 */
void
ox_object_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, iter)

    /*Object*/
    ox_not_error(ox_object_new(ctxt, OX_OBJECT(ctxt, Object), NULL));
    ox_not_error(ox_object_set_name_s(ctxt, OX_OBJECT(ctxt, Object), "Object"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Object", OX_OBJECT(ctxt, Object)));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "entries", object_entries));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "keys", object_keys));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "values", object_values));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "lookup", object_lookup));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "get", object_get));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "set", object_set));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "add_const", object_add_const));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "add_var", object_add_var));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "add_accessor", object_add_accessor));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "del_prop", object_del_prop));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "get_name", object_get_name));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "unref", object_unref));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "is", object_is));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Object), "set_inf", object_set_inf));

    /*ObjectIterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, ObjectIterator_inf),
            OX_OBJECT(ctxt, Object), "Iterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, ObjectIterator_inf), "next",
            ObjectIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ObjectIterator_inf), "end",
            ObjectIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ObjectIterator_inf), "value",
            ObjectIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, iter)
}
