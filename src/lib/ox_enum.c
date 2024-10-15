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
 * Enumeration.
 */

#define OX_LOG_TAG "ox_enum"

#include "ox_internal.h"

/** Item of the enumeration.*/
typedef struct {
    OX_HashEntry he;   /**< Hash table entry.*/
    OX_List      ln;   /**< List node data.*/
    OX_Value     name; /**< Name of the item.*/
} OX_EnumItem;

/** Enumeration data.*/
typedef struct {
    OX_Hash item_hash; /**< Item hash table.*/
    OX_List item_list; /**< Item list.*/
} OX_Enum;

/*Scan referenced objects in the enumeration data.*/
static void
enum_data_scan (OX_Context *ctxt, void *p)
{
    OX_Enum *e = p;
    OX_EnumItem *ei;

    ox_list_foreach_c(&e->item_list, ei, OX_EnumItem, ln) {
        ox_gc_scan_value(ctxt, &ei->name);
    }
}

/*Free the enumeration data.*/
static void
enum_data_free (OX_Context *ctxt, void *p)
{
    OX_Enum *e = p;
    OX_EnumItem *ei, *nei;

    ox_list_foreach_safe_c(&e->item_list, ei, nei, OX_EnumItem, ln) {
        OX_DEL(ctxt, ei);
    }

    ox_hash_deinit(ctxt, &e->item_hash);

    OX_DEL(ctxt, e);
}

/*Operation functions of enumeration data.*/
static OX_PrivateOps
enum_data_ops = {
    enum_data_scan,
    enum_data_free
};

/*Get the enumeration data.*/
static OX_Enum*
enum_data_get (OX_Context *ctxt, OX_Value *e)
{
    OX_Enum *ep;

    if (!(ep = ox_object_get_priv(ctxt, e, &enum_data_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not an enumeration"));

    return ep;
}

/*Get the property keys of the enumeration.*/
static OX_Result
enum_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    OX_VS_PUSH(ctxt, key)
    OX_Enum *e;
    OX_EnumItem *ei;
    OX_Result r;

    if (!(e = enum_data_get(ctxt, o))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_array_new(ctxt, keys, 0)) == OX_ERR)
        goto end;

    ox_list_foreach_c(&e->item_list, ei, OX_EnumItem, ln) {
        int32_t k = OX_PTR2SIZE(ei->he.key);

        ox_value_set_number(ctxt, key, k);

        if ((r = ox_array_append(ctxt, keys, key)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, key)
    return r;
}

/*Get property of en enumeration.*/
static OX_Result
enum_get_ex (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Bool lookup)
{
    if (ox_value_is_number(ctxt, p)) {
        OX_Enum *e;
        OX_EnumItem *ei;
        int32_t i;
        OX_Result r;

        if (!(e = enum_data_get(ctxt, o)))
            return OX_ERR;

        if ((r = ox_to_int32(ctxt, p, &i)) == OX_ERR)
            return r;

        ei = ox_hash_lookup_c(ctxt, &e->item_hash, OX_SIZE2PTR(i), NULL, OX_EnumItem, he);
        if (!ei)
            return OX_FALSE;

        ox_value_copy(ctxt, v, &ei->name);
        return OX_OK;
    } else if (lookup) {
        return ox_object_lookup(ctxt, o, p, v);
    } else {
        return ox_object_get(ctxt, o, p, v);
    }
}

/*Lookup the property owned by the enumeration.*/
static OX_Result
enum_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return enum_get_ex(ctxt, o, p, v, OX_TRUE);
}

/*Get property of en enumeration.*/
static OX_Result
enum_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return enum_get_ex(ctxt, o, p, v, OX_FALSE);
}

/*Set property of en enumeration.*/
static OX_Result
enum_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_access_error(ctxt, OX_TEXT("cannot set property of enumeration"));
}

/*Get property of a bitfield.*/
static OX_Result
bitfield_get_ex (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Bool lookup)
{
    if (ox_value_is_number(ctxt, p)) {
        OX_Enum *e;
        OX_EnumItem *ei;
        int32_t i;
        OX_Result r;

        if (!(e = enum_data_get(ctxt, o)))
            return OX_ERR;

        if ((r = ox_to_int32(ctxt, p, &i)) == OX_ERR)
            return r;

        if ((r = ox_array_new(ctxt, v, 0)) == OX_ERR)
            return r;

        ox_list_foreach_c(&e->item_list, ei, OX_EnumItem, ln) {
            int32_t k = OX_PTR2SIZE(ei->he.key);

            if ((i & k) == k) {
                if ((r = ox_array_append(ctxt, v, &ei->name)) == OX_ERR)
                    return r;
            }
        }

        return OX_OK;
    } else if (lookup) {
        return ox_object_lookup(ctxt, o, p, v);
    } else {
        return ox_object_get(ctxt, o, p, v);
    }
}

/*Lookup the owned property by the bitfield.*/
static OX_Result
bitfield_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return bitfield_get_ex(ctxt, o, p, v, OX_TRUE);
}

/*Get property of a bitfield.*/
static OX_Result
bitfield_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return bitfield_get_ex(ctxt, o, p, v, OX_FALSE);
}

/*Set property of a bitfield.*/
static OX_Result
bitfield_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_access_error(ctxt, OX_TEXT("cannot set property of bitfield"));
}

/*Enumeration operation functions.*/
static const OX_ObjectOps
enum_ops = {
    {
        OX_GCO_ENUM,
        ox_object_scan,
        ox_object_free
    },
    enum_keys,
    enum_lookup,
    enum_get,
    enum_set,
    ox_object_del,
    ox_object_call
};

/*Bitfield operation functions.*/
static const OX_ObjectOps
bitfield_ops = {
    {
        OX_GCO_ENUM,
        ox_object_scan,
        ox_object_free
    },
    enum_keys,
    bitfield_lookup,
    bitfield_get,
    bitfield_set,
    ox_object_del,
    ox_object_call
};

/**
 * Create a new enumeration.
 * @param ctxt The current running context.
 * @param[out] e Return the enumeration object.
 * @param type The enumeration's type.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_enum_new (OX_Context *ctxt, OX_Value *e, OX_EnumType type)
{
    OX_Enum *ep;
    OX_Result r;
    const OX_ObjectOps *ops;

    assert(ctxt && e);

    if ((r = ox_object_new(ctxt, e, OX_OBJECT(ctxt, Enum_inf))) == OX_ERR)
        return r;

    ops = (type == OX_ENUM_ENUM) ? &enum_ops : &bitfield_ops;
    if ((r = ox_object_set_ops(ctxt, e, ops)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, ep))
        return ox_throw_no_mem_error(ctxt);

    ox_size_hash_init(&ep->item_hash);
    ox_list_init(&ep->item_list);

    if ((r = ox_object_set_priv(ctxt, e, &enum_data_ops, ep)) == OX_ERR) {
        enum_data_free(ctxt, ep);
        return r;
    }

    return OX_OK;
}

/**
 * Add an enumeration item.
 * @param ctxt The current running context.
 * @param e The enumeration object.
 * @param c The container class.
 * @param name The name of the enumeration.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_enum_add_item (OX_Context *ctxt, OX_Value *e, OX_Value *c, OX_Value *name, int v)
{
    OX_Enum *ep;
    OX_EnumItem *ei;
    OX_HashEntry **pe;
    OX_Result r;
    OX_Value *nv;

    assert(ctxt && e && c && name);
    assert(ox_value_is_object(ctxt, c));
    assert(ox_value_is_string(ctxt, name));
    assert(ox_value_is_gco(ctxt, e, OX_GCO_ENUM));

    if (!(ep = enum_data_get(ctxt, e)))
        return OX_ERR;

    ei = ox_hash_lookup_c(ctxt, &ep->item_hash, OX_SIZE2PTR(v), &pe, OX_EnumItem, he);
    if (ei) {
        OX_LOG_D(ctxt, "enumeration item %d is already defined as %s",
                v, ox_string_get_char_star(ctxt, &ei->name));
        return OX_OK;
    }

    if (!OX_NEW(ctxt, ei))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &ei->name, name);

    if ((r = ox_hash_insert(ctxt, &ep->item_hash, OX_SIZE2PTR(v), &ei->he, pe)) == OX_ERR) {
        OX_DEL(ctxt, ei);
        return r;
    }

    ox_list_append(&ep->item_list, &ei->ln);

    nv = ox_value_stack_push(ctxt);
    ox_value_set_number(ctxt, nv, v);
    r = ox_object_add_const(ctxt, c, name, nv);
    ox_value_stack_pop(ctxt, nv);

    return r;
}

/**
 * Add an enumeration item.
 * The name is a null terminated characters string.
 * @param ctxt The current running context.
 * @param e The enumeration object.
 * @param c The container class.
 * @param name The name of the enumeration.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_enum_add_item_s (OX_Context *ctxt, OX_Value *e, OX_Value *c, const char *name, int v)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    r = ox_string_from_const_char_star(ctxt, nv, name);
    if (r == OX_OK)
        r = ox_enum_add_item(ctxt, e, c, nv, v);

    OX_VS_POP(ctxt, nv)
    return r;
}

/** Enumeration iterator data.*/
typedef struct {
    OX_Value          e;    /**< The enumeration.*/
    OX_EnumItem      *ei;   /**< The current enumeration item.*/
    OX_ObjectIterType type; /**< The iterator type.*/
} OX_EnumIter;

/*Scan referenced objects in the enumeration iterator.*/
static void
enum_iter_scan (OX_Context *ctxt, void *p)
{
    OX_EnumIter *iter = p;

    ox_gc_scan_value(ctxt, &iter->e);
}

/*Free the enumeration iterator.*/
static void
enum_iter_free (OX_Context *ctxt, void *p)
{
    OX_EnumIter *iter = p;

    OX_DEL(ctxt, iter);
}

/*Enumeration iterator's operation functions.*/
static const OX_PrivateOps
enum_iter_ops = {
    enum_iter_scan,
    enum_iter_free
};

/*Create a new enumeration iterator.*/
static OX_Result
enum_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *e, OX_ObjectIterType type)
{
    OX_Enum *ep;
    OX_EnumIter *ei;
    OX_Result r;

    if (!(ep = enum_data_get(ctxt, e)))
        return OX_ERR;

    if ((r = ox_object_new(ctxt, iter, OX_OBJECT(ctxt, EnumIterator_inf))) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, ei))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &ei->e, e);
    ei->type = type;
    ei->ei = ox_list_head_c(&ep->item_list, OX_EnumItem, ln);

    if ((r = ox_object_set_priv(ctxt, iter, &enum_iter_ops, ei)) == OX_ERR) {
        enum_iter_free(ctxt, ei);
        return r;
    }

    return OX_OK;
}

/*Get the enumeration iterator data.*/
static OX_EnumIter*
enum_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_EnumIter *iter;

    if (!(iter = ox_object_get_priv(ctxt, v, &enum_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not an enumeration iterator"));

    return iter;
}

/*Enum.$inf.$iter*/
static OX_Result
Enum_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return enum_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY_VALUE);
}

/*Enum.$inf.entries*/
static OX_Result
Enum_inf_entries (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return enum_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY_VALUE);
}

/*Enum.$inf.keys*/
static OX_Result
Enum_inf_keys (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return enum_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY);
}

/*Enum.$inf.values*/
static OX_Result
Enum_inf_values (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return enum_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_VALUE);
}

/*EnumIterator.$inf.next*/
static OX_Result
EnumIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Enum *e;
    OX_EnumIter *iter;

    if (!(iter = enum_iter_get(ctxt, thiz)))
        return OX_ERR;

    e = enum_data_get(ctxt, &iter->e);

    if (iter->ei)
        iter->ei = ox_list_next_c(&e->item_list, iter->ei, OX_EnumItem, ln);

    return OX_OK;
}

/*EnumIterator.$inf.end get*/
static OX_Result
EnumIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_EnumIter *iter;

    if (!(iter = enum_iter_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_set_bool(ctxt, rv, iter->ei ? OX_FALSE : OX_TRUE);
    return OX_OK;
}

/*EnumIterator.$inf.value get*/
static OX_Result
EnumIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_EnumIter *iter;
    int i;
    OX_Result r;

    if (!(iter = enum_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (iter->ei) {
        switch (iter->type) {
        case OX_OBJECT_ITER_KEY:
            i = OX_PTR2SIZE(iter->ei->he.key);
            ox_value_set_number(ctxt, rv, i);
            break;
        case OX_OBJECT_ITER_VALUE:
            ox_value_copy(ctxt, rv, &iter->ei->name);
            break;
        case OX_OBJECT_ITER_KEY_VALUE: {
            OX_Value *v;

            if ((r = ox_array_new(ctxt, rv, 2)) == OX_ERR)
                return r;

            v = ox_value_stack_push(ctxt);

            i = OX_PTR2SIZE(iter->ei->he.key);
            ox_value_set_number(ctxt, v, i);
            r = ox_array_set_item(ctxt, rv, 0, v);
            if (r == OX_OK)
                r = ox_array_set_item(ctxt, rv, 1, &iter->ei->name);

            ox_value_stack_pop(ctxt, v);

            if (r == OX_ERR)
                return r;
            break;
        }
        default:
            assert(0);
        }
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/*?
 *? @lib {Enum} Enumeration and bitfield.
 *?
 *? @class{ Enum Enumeration and bitfiled class.
 *?
 *? @func $iter Create an iterator to traverse the items in the enumeration or bitfield.
 *? The value of iterator is a 2 items array.
 *? Item 0 is the item value of the enumeration.
 *? Item 1 is the name of the item.
 *? @return {Iterator[[Number,String]]} The iterator used to traverse the items.
 *?
 *? @func entries Create an iterator to traverse the items in the enumeration or bitfield.
 *? The value of iterator is a 2 items array.
 *? Item 0 is the item value of the enumeration.
 *? Item 1 is the name of the item.
 *? @return {Iterator[[Number,String]]} The iterator used to traverse the items.
 *?
 *? @func keys Create an iterator to traverse the items' values in the enumeration or bitfield.
 *? The value of iterator is the item's value.
 *? @return {Iterator[Number]} The iterator used to traverse the item's value.
 *?
 *? @func values Create an iterator to traverse the items' names in the enumeration or bitfield.
 *? The value of iterator is the item's name.
 *? @return {Iterator[Number]} The iterator used to traverse the item's name.
 *?
 *? @class}
 */

/**
 * Initialize the enumeration object.
 * @param ctxt The current running context.
 */
void
ox_enum_object_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, iter)

    /*Enum_inf.*/
    ox_not_error(ox_interface_new(ctxt, OX_OBJECT(ctxt, Enum_inf)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Enum_inf),
            "$iter", Enum_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Enum_inf),
            "entries", Enum_inf_entries));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Enum_inf),
            "keys", Enum_inf_keys));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Enum_inf),
            "values", Enum_inf_values));

    /*EnumIterator.*/
    ox_not_error(ox_class_new(ctxt, iter, OX_OBJECT(ctxt, EnumIterator_inf)));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, EnumIterator_inf),
            "next", EnumIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, EnumIterator_inf),
            "end", EnumIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, EnumIterator_inf),
            "value", EnumIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, iter)
}
