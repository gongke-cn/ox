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
 * Set object.
 */

#define OX_LOG_TAG "ox_set"

#include "ox_internal.h"

/** Entry of the set.*/
typedef struct {
    OX_HashEntry he; /**< Hash table entry data.*/
    OX_List      ln; /**< List node data.*/
    OX_Value     v;  /**< Value of the entry.*/
} OX_SetEntry;

/** Set's iterator.*/
typedef struct {
    OX_Value     set;  /**< The set contains this iterator.*/
    OX_List      ln;   /**< List node data.*/
    OX_Bool      next; /**< Already get the next entry.*/
    OX_SetEntry *curr; /**< The current entry.*/
} OX_SetIter;

/** Set.*/
typedef struct {
    OX_Object o;      /**< Base object data.*/
    OX_Hash   e_hash; /**< Entries hash table.*/
    OX_List   e_list; /**< Entries list.*/
    OX_List   i_list; /**< Iterator list.*/
} OX_Set;

/*Scan referenced objects in the set.*/
static void
set_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Set *set = (OX_Set*)gco;
    OX_SetEntry *se;

    ox_object_scan(ctxt, gco);

    ox_list_foreach_c(&set->e_list, se, OX_SetEntry, ln) {
        ox_gc_scan_value(ctxt, &se->v);
    }
}

/*Free the set.*/
static void
set_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Set *set = (OX_Set*)gco;
    OX_SetEntry *se, *nse;
    OX_SetIter *si, *nsi;

    ox_list_foreach_safe_c(&set->e_list, se, nse, OX_SetEntry, ln) {
        OX_DEL(ctxt, se);
    }

    ox_list_foreach_safe_c(&set->i_list, si, nsi, OX_SetIter, ln) {
        ox_list_remove(&si->ln);
        ox_list_init(&si->ln);
    }

    ox_hash_deinit(ctxt, &set->e_hash);

    ox_object_deinit(ctxt, &set->o);

    OX_DEL(ctxt, set);
}

/*Get the set data.*/
static OX_Set*
set_data_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Set *set;

    if (!ox_value_is_gco(ctxt, v, OX_GCO_SET)) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a set object"));
        return NULL;
    }

    set = ox_value_get_gco(ctxt, v);

    return set;
}

/*Add an entry to the set.*/
static OX_Result
set_add_entry (OX_Context *ctxt, OX_Value *o, OX_Value *v)
{
    OX_SetEntry *se;
    OX_HashEntry **pe;
    OX_Set *set;
    OX_Result r;

    if (!(set = set_data_get(ctxt, o)))
        return OX_ERR;

    se = ox_hash_lookup_c(ctxt, &set->e_hash, v, &pe, OX_SetEntry, he);

    if (se) {
        r = OX_FALSE;
    } else {
        if (!OX_NEW(ctxt, se))
            return ox_throw_no_mem_error(ctxt);

        ox_value_copy(ctxt, &se->v, v);

        if ((r = ox_hash_insert(ctxt, &set->e_hash, &se->v, &se->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, se);
            return r;
        }

        ox_list_append(&set->e_list, &se->ln);

        r = OX_OK;
    }

    return r;
}

/*Remove an entry of the set.*/
static OX_Result
set_remove_entry (OX_Context *ctxt, OX_Value *o, OX_Value *v)
{
    OX_Set *set;
    OX_SetEntry *se;
    OX_HashEntry **pe;
    OX_Result r;

    if (!(set = set_data_get(ctxt, o)))
        return OX_ERR;

    se = ox_hash_lookup_c(ctxt, &set->e_hash, v, &pe, OX_SetEntry, he);
    if (se) {
        OX_SetIter *si;

        ox_list_foreach_c(&set->i_list, si, OX_SetIter, ln) {
            if (si->curr == se) {
                si->curr = ox_list_next_c(&set->i_list, se, OX_SetEntry, ln);
                si->next = OX_TRUE;
            }
        }

        ox_hash_remove(ctxt, &set->e_hash, &se->v, pe);
        ox_list_remove(&se->ln);

        OX_DEL(ctxt, se);

        r = OX_TRUE;
    } else {
        r = OX_FALSE;
    }

    return r;
}

/*Lookup the set's owned property.*/
static OX_Result
set_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_SetEntry *se;
    OX_Set *set;

    if (!(set = set_data_get(ctxt, o)))
        return OX_ERR;

    se = ox_hash_lookup_c(ctxt, &set->e_hash, k, NULL, OX_SetEntry, he);
    if (se) {
        ox_value_copy(ctxt, v, &se->v);
        return OX_OK;
    }

    ox_value_set_null(ctxt, v);
    return OX_OK;
}

/*Set the property of a set.*/
static OX_Result
set_set (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_Result r;

    r = set_add_entry(ctxt, o, v);
    if (r == OX_FALSE)
        r = OX_OK;

    return r;
}

/*Delete the owned property of the set.*/
static OX_Result
set_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    OX_Result r;

    r = set_remove_entry(ctxt, o, p);
    if (r == OX_FALSE)
        r = OX_OK;

    return r;
}

/*Set's operation functions.*/
static const OX_ObjectOps
set_ops = {
    {
        OX_GCO_SET,
        set_scan,
        set_free
    },
    ox_object_keys,
    set_lookup,
    ox_object_get,
    set_set,
    set_del,
    ox_object_call
};

/*Scan referenced objects in the set iterator.*/
static void
set_iter_scan (OX_Context *ctxt, void *p)
{
    OX_SetIter *si = p;

    ox_gc_scan_value(ctxt, &si->set);
}

/*Free the set iterator.*/
static void
set_iter_free (OX_Context *ctxt, void *p)
{
    OX_SetIter *si = p;

    ox_list_remove(&si->ln);
    OX_DEL(ctxt, si);
}

/*Set iterator's operation functions.*/
static const OX_PrivateOps
set_iter_ops = {
    set_iter_scan,
    set_iter_free
};

/*Get the set iterator data.*/
static OX_SetIter*
set_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_SetIter *si;

    if (!(si = ox_object_get_priv(ctxt, v, &set_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a set iterator"));

    return si;
}

/*Allocate a set object.*/
static OX_Result
set_alloc (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Set *set;

    if (!OX_NEW(ctxt, set))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &set->o, inf);

    set->o.gco.ops = (OX_GcObjectOps*)&set_ops;

    ox_value_hash_init(&set->e_hash);
    ox_list_init(&set->e_list);
    ox_list_init(&set->i_list);

    ox_value_set_gco(ctxt, o, set);
    ox_gc_add(ctxt, set);

    return OX_OK;
}

/*Set.$inf.add*/
static OX_Result
Set_inf_add (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = set_add_entry(ctxt, thiz, v)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, r);
    return OX_OK;
}

/*Set.$inf.has*/
static OX_Result
Set_inf_has (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_SetEntry *se;
    OX_Set *set;
    OX_Bool b;

    if (!(set = set_data_get(ctxt, thiz)))
        return OX_ERR;

    se = ox_hash_lookup_c(ctxt, &set->e_hash, v, NULL, OX_SetEntry, he);

    b = se ? OX_TRUE : OX_FALSE;
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Set.$inf.remove*/
static OX_Result
Set_inf_remove (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = set_remove_entry(ctxt, thiz, v)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, r);
    return OX_OK;
}

/*Set.$inf.$iter*/
static OX_Result
Set_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Set *set;
    OX_SetIter *si;
    OX_Result r;

    if (!(set = set_data_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_object_new(ctxt, rv, OX_OBJECT(ctxt, SetIterator_inf))) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, si))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &si->set, thiz);
    si->curr = ox_list_head_c(&set->e_list, OX_SetEntry, ln);
    si->next = OX_FALSE;

    if ((r = ox_object_set_priv(ctxt, rv, &set_iter_ops, si)) == OX_ERR) {
        set_iter_free(ctxt, si);
        return r;
    }

    ox_list_append(&set->i_list, &si->ln);
    return OX_OK;
}

/*Set.$inf.$to_json*/
static OX_Result
Set_inf_to_json (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Set *set;
    OX_SetEntry *se;
    size_t i;
    OX_Result r;

    if (!(set = set_data_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_array_new(ctxt, rv, set->e_hash.e_num)) == OX_ERR)
        return r;

    i = 0;
    ox_list_foreach_c(&set->e_list, se, OX_SetEntry, ln) {
        if ((r = ox_array_set_item(ctxt, rv, i, &se->v)) == OX_ERR)
            return r;

        i ++;
    }

    return OX_OK;
}

/*Set.$inf.length get*/
static OX_Result
Set_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Set *set;

    if (!(set = set_data_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, set->e_hash.e_num);
    return OX_OK;
}

/*SetIterator.$inf.end get*/
static OX_Result
SetIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SetIter *si;
    OX_Bool b;

    if (!(si = set_iter_get(ctxt, thiz)))
        return OX_ERR;

    b = si->curr ? OX_FALSE : OX_TRUE;
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*SetIterator.$inf.value get*/
static OX_Result
SetIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_SetIter *si;

    if (!(si = set_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (si->curr && !si->next)
        ox_value_copy(ctxt, rv, &si->curr->v);
    else
        ox_value_set_null(ctxt, rv);

    return OX_OK;
}

/*SetIterator.$inf.next*/
static OX_Result
SetIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
     OX_SetIter *si;

    if (!(si = set_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (si->curr) {
        if (si->next) {
            si->next = OX_FALSE;
        } else {
            OX_Set *set = set_data_get(ctxt, &si->set);

            si->curr = ox_list_next_c(&set->e_list, si->curr, OX_SetEntry, ln);
        }
    }

    return OX_OK;
}

/*SetIterator.$inf.close*/
static OX_Result
SetIterator_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
     OX_SetIter *si;

    if (!(si = set_iter_get(ctxt, thiz)))
        return OX_ERR;

    si->curr = NULL;
    ox_list_remove(&si->ln);
    ox_list_init(&si->ln);

    return OX_OK;
}

/*?
 *? @lib {Set} Data set.
 *? Data set is used to store data.
 *? Only one copy of data with the same value will be saved in the data set.
 *?
 *? @class{ Set Data set.
 *? Data set is used to store data.
 *? Only one copy of data with the same value will be saved in the data set.
 *?
 *? @func add Add data to the set.
 *? @param v The value to be added.
 *? @return {Bool} If the value is added successfully, return true.\
 *? Or if the value is already in set, return false.
 *?
 *? @func has Check if the value is already stored in the set.
 *? @param v The value.
 *? @return {Bool} The value is already stored or not.
 *?
 *? @func remove Remove a value from the set.
 *? @param v The value to be removed.
 *? @return {Bool} If the value is removed successfully, return true.\
 *? If the value is not in the set, return false.
 *?
 *? @func $iter Create an iterator to traverse the values in the set.
 *? @return {Iterator[Any]} The iterator used to traverse the values.
 *?
 *? @func $to_json Convert the data set to an array containes all the values in it.
 *? This function is used when invoking JSON.to_str().
 *? @return {Array} The array contains the values in the set.
 *?
 *? @roacc length {Number} The number of values in the set.
 *?
 *? @class}
 */

/**
 * Initialize the set class.
 */
extern void
ox_set_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_3(ctxt, c, inf, iter)

    /*Set.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Set"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Set", c));
    ox_not_error(ox_class_set_alloc_func(ctxt, c, set_alloc));

    /*Set_inf.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "add", Set_inf_add));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "has", Set_inf_has));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "remove", Set_inf_remove));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$iter", Set_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_json", Set_inf_to_json));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "length", Set_inf_length_get, NULL));

    /*SetIterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, SetIterator_inf), c, "Iterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));

    /*SetIterator_inf.*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SetIterator_inf),
            "end", SetIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SetIterator_inf),
            "value", SetIterator_inf_value_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, SetIterator_inf),
            "next", SetIterator_inf_next));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, SetIterator_inf),
            "$close", SetIterator_inf_close));

    OX_VS_POP(ctxt, c)
}
