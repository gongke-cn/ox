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
 * Dictionary.
 */

#define OX_LOG_TAG "ox_dict"

#include "ox_internal.h"

/** Entry of the dictionary.*/
typedef struct {
    OX_HashEntry he; /**< Hash table entry data.*/
    OX_List      ln; /**< List node data.*/
    OX_Value     k;  /**< Key of the entry.*/
    OX_Value     v;  /**< Value of the entry.*/
} OX_DictEntry;

/** Dictionary's iterator.*/
typedef struct {
    OX_ObjectIterType type; /**< Iterator type.*/
    OX_Value      dict;  /**< The dictionary contains this iterator.*/
    OX_List       ln;    /**< List node data.*/
    OX_Bool       next;  /**< Already get the next entry.*/
    OX_DictEntry *curr;  /**< The current entry.*/
} OX_DictIter;

/** Dictionary.*/
typedef struct {
    OX_Object o;      /**< Base object data.*/
    OX_Hash   e_hash; /**< Entries hash table.*/
    OX_List   e_list; /**< Entries list.*/
    OX_List   i_list; /**< Iterator list.*/
} OX_Dict;

/*Scan referenced objects in the dictionary.*/
static void
dict_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Dict *dict = (OX_Dict*)gco;
    OX_DictEntry *de;

    ox_object_scan(ctxt, gco);

    ox_list_foreach_c(&dict->e_list, de, OX_DictEntry, ln) {
        ox_gc_scan_value(ctxt, &de->k);
        ox_gc_scan_value(ctxt, &de->v);
    }
}

/*Free the dictionary.*/
static void
dict_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Dict *dict = (OX_Dict*)gco;
    OX_DictEntry *de, *nde;
    OX_DictIter *di, *ndi;

    ox_list_foreach_safe_c(&dict->e_list, de, nde, OX_DictEntry, ln) {
        OX_DEL(ctxt, de);
    }

    ox_list_foreach_safe_c(&dict->i_list, di, ndi, OX_DictIter, ln) {
        ox_list_remove(&di->ln);
        ox_list_init(&di->ln);
    }

    ox_hash_deinit(ctxt, &dict->e_hash);

    ox_object_deinit(ctxt, &dict->o);

    OX_DEL(ctxt, dict);
}

/*Get the dictionary data.*/
static OX_Dict*
dict_data_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Dict *dict;

    if (!ox_value_is_gco(ctxt, v, OX_GCO_DICT)) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a dictionary object"));
        return NULL;
    }

    dict = ox_value_get_gco(ctxt, v);

    return dict;
}

/*Add an entry to the dictionary.*/
static OX_Result
dict_add_entry (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_DictEntry *de;
    OX_HashEntry **pe;
    OX_Dict *dict;
    OX_Result r;

    if (!(dict = dict_data_get(ctxt, o)))
        return OX_ERR;

    de = ox_hash_lookup_c(ctxt, &dict->e_hash, k, &pe, OX_DictEntry, he);

    if (de) {
        ox_value_copy(ctxt, &de->v, v);
    } else {
        if (!OX_NEW(ctxt, de))
            return ox_throw_no_mem_error(ctxt);

        ox_value_copy(ctxt, &de->k, k);
        ox_value_copy(ctxt, &de->v, v);

        if ((r = ox_hash_insert(ctxt, &dict->e_hash, &de->k, &de->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, de);
            return r;
        }

        ox_list_append(&dict->e_list, &de->ln);
    }

    return OX_OK;
}

/*Remove an entry from the dictionary.*/
static OX_Result
dict_remove_entry (OX_Context *ctxt, OX_Value *o, OX_Value *k)
{
    OX_DictEntry *de;
    OX_HashEntry **pe;
    OX_Dict *dict;
    OX_Result r;

    if (!(dict = dict_data_get(ctxt, o)))
        return OX_ERR;

    de = ox_hash_lookup_c(ctxt, &dict->e_hash, k, &pe, OX_DictEntry, he);
    if (de) {
        OX_DictIter *di;

        ox_list_foreach_c(&dict->i_list, di, OX_DictIter, ln) {
            if (di->curr == de) {
                di->curr = ox_list_next_c(&dict->i_list, de, OX_DictEntry, ln);
                di->next = OX_TRUE;
            }
        }

        ox_hash_remove(ctxt, &dict->e_hash, &de->k, pe);
        ox_list_remove(&de->ln);

        OX_DEL(ctxt, de);

        r = OX_TRUE;
    } else {
        r = OX_FALSE;
    }

    return r;
}

/*Lookup the entry in the dictionary.*/
static OX_Result
dict_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_DictEntry *de;
    OX_Dict *dict;

    if (!(dict = dict_data_get(ctxt, o)))
        return OX_ERR;

    de = ox_hash_lookup_c(ctxt, &dict->e_hash, k, NULL, OX_DictEntry, he);
    if (!de) {
        ox_value_set_null(ctxt, v);
        return OX_OK;
    }

    ox_value_copy(ctxt, v, &de->v);
    return OX_OK;
}

/*Set the property of a dictionary.*/
static OX_Result
dict_set (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    return dict_add_entry(ctxt, o, k, v);
}

/*Remove an property of a dictionary.*/
static OX_Result
dict_del (OX_Context *ctxt, OX_Value *o, OX_Value *k)
{
    OX_Result r;

    r = dict_remove_entry(ctxt, o, k);
    if (r == OX_FALSE)
        r = OX_OK;

    return r;
}

/*Dictionary's operation functions.*/
static const OX_ObjectOps
dict_ops = {
    {
        OX_GCO_DICT,
        dict_scan,
        dict_free
    },
    ox_object_keys,
    dict_lookup,
    ox_object_get,
    dict_set,
    dict_del,
    ox_object_call
};

/*Scan referenced objects in the dictionary iterator.*/
static void
dict_iter_scan (OX_Context *ctxt, void *p)
{
    OX_DictIter *di = p;

    ox_gc_scan_value(ctxt, &di->dict);
}

/*Free the dictionary iterator.*/
static void
dict_iter_free (OX_Context *ctxt, void *p)
{
    OX_DictIter *di = p;

    ox_list_remove(&di->ln);
    OX_DEL(ctxt, di);
}

/*Dictionary iterator's operation functions.*/
static const OX_PrivateOps
dict_iter_ops = {
    dict_iter_scan,
    dict_iter_free
};

/*Get the dictionary iterator data.*/
static OX_DictIter*
dict_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_DictIter *di;

    if (!(di = ox_object_get_priv(ctxt, v, &dict_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a dictionary iterator"));

    return di;
}

/*Allocate a dictionary object.*/
static OX_Result
dict_alloc (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Dict *dict;

    if (!OX_NEW(ctxt, dict))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &dict->o, inf);

    dict->o.gco.ops = (OX_GcObjectOps*)&dict_ops;

    ox_value_hash_init(&dict->e_hash);
    ox_list_init(&dict->e_list);
    ox_list_init(&dict->i_list);

    ox_value_set_gco(ctxt, o, dict);
    ox_gc_add(ctxt, dict);

    return OX_OK;
}

/**
 * Create a new dictionary.
 * @param ctxt The current running context.
 * @param[out] dict Return the new dictionary.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_dict_new (OX_Context *ctxt, OX_Value *dict)
{
    return dict_alloc(ctxt, dict, OX_OBJECT(ctxt, Dict_inf));
}

/**
 * Add a new entry to the dictionary.
 * @param ctxt The current running context.
 * @param dict The dictionary.
 * @param k The key of the new entry.
 * @param v The value of the new entry.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_dict_add (OX_Context *ctxt, OX_Value *dict, OX_Value *k, OX_Value *v)
{
    return dict_add_entry(ctxt, dict, k, v);
}

/*Create a new dictionary iterator.*/
static OX_Result
dict_iter_new (OX_Context *ctxt, OX_Value *v, OX_Value *dictv, OX_ObjectIterType type)
{
    OX_Dict *dict;
    OX_DictIter *di;
    OX_Result r;

    if (!(dict = dict_data_get(ctxt, dictv)))
        return OX_ERR;

    if ((r = ox_object_new(ctxt, v, OX_OBJECT(ctxt, DictIterator_inf))) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, di))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &di->dict, dictv);
    di->curr = ox_list_head_c(&dict->e_list, OX_DictEntry, ln);
    di->next = OX_FALSE;
    di->type = type;

    if ((r = ox_object_set_priv(ctxt, v, &dict_iter_ops, di)) == OX_ERR) {
        dict_iter_free(ctxt, di);
        return r;
    }

    ox_list_append(&dict->i_list, &di->ln);
    return OX_OK;
}

/*Dict.$inf.add*/
static OX_Result
Dict_inf_add (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *k = ox_argument(ctxt, args, argc, 0);
    OX_Value *v = ox_argument(ctxt, args, argc, 1);

    return dict_add_entry(ctxt, thiz, k, v);
}

/*Dict.$inf.get*/
static OX_Result
Dict_inf_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *k = ox_argument(ctxt, args, argc, 0);
    OX_DictEntry *de;
    OX_Dict *dict;

    if (!(dict = dict_data_get(ctxt, thiz)))
        return OX_ERR;

    de = ox_hash_lookup_c(ctxt, &dict->e_hash, k, NULL, OX_DictEntry, he);
    if (de) {
        ox_value_copy(ctxt, rv, &de->v);
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/*Dict.$inf.remove*/
static OX_Result
Dict_inf_remove (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *k = ox_argument(ctxt, args, argc, 0);
    OX_Result r;

    if ((r = dict_remove_entry(ctxt, thiz, k)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, r);
    return OX_OK;
}

/*Dict.$inf.keys*/
static OX_Result
Dict_inf_keys (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return dict_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY);
}

/*Dict.$inf.values*/
static OX_Result
Dict_inf_values (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return dict_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_VALUE);
}

/*Dict.$inf.entries*/
static OX_Result
Dict_inf_entries (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return dict_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY_VALUE);
}

/*Dict.$inf.$to_json*/
static OX_Result
Dict_inf_to_json (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    OX_DictEntry *de;
    OX_Dict *dict;
    OX_Result r;

    if (!(dict = dict_data_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_object_new(ctxt, rv, NULL)) == OX_ERR)
        goto end;

    ox_list_foreach_c(&dict->e_list, de, OX_DictEntry, ln) {
        if ((r = ox_to_string(ctxt, &de->k, s)) == OX_ERR)
            goto end;

        if ((r = ox_set(ctxt, rv, s, &de->v)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Dict.$inf.length get*/
static OX_Result
Dict_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Dict *dict;

    if (!(dict = dict_data_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, dict->e_hash.e_num);
    return OX_OK;
}

/*DictIterator.$inf.end get*/
static OX_Result
DictIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_DictIter *di;
    OX_Bool b;

    if (!(di = dict_iter_get(ctxt, thiz)))
        return OX_ERR;

    b = di->curr ? OX_FALSE : OX_TRUE;
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*DictIterator.$inf.value get*/
static OX_Result
DictIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_DictIter *di;
    OX_Result r;

    if (!(di = dict_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (di->curr && !di->next) {
        switch (di->type) {
        case OX_OBJECT_ITER_KEY:
            ox_value_copy(ctxt, rv, &di->curr->k);
            break;
        case OX_OBJECT_ITER_VALUE:
            ox_value_copy(ctxt, rv, &di->curr->v);
            break;
        case OX_OBJECT_ITER_KEY_VALUE:
            if ((r = ox_array_new(ctxt, rv, 2)) == OX_ERR)
                return r;

            if ((r = ox_array_set_item(ctxt, rv, 0, &di->curr->k)) == OX_ERR)
                return r;

            if ((r = ox_array_set_item(ctxt, rv, 1, &di->curr->v)) == OX_ERR)
                return r;
            break;
        default:
            assert(0);
        }
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/*DictIterator.$inf.next*/
static OX_Result
DictIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
     OX_DictIter *di;

    if (!(di = dict_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (di->curr) {
        if (di->next) {
            di->next = OX_FALSE;
        } else {
            OX_Dict *dict = dict_data_get(ctxt, &di->dict);

            di->curr = ox_list_next_c(&dict->e_list, di->curr, OX_DictEntry, ln);
        }
    }

    return OX_OK;
}

/*DictIterator.$inf.close*/
static OX_Result
DictIterator_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
     OX_DictIter *di;

    if (!(di = dict_iter_get(ctxt, thiz)))
        return OX_ERR;

    di->curr = NULL;
    ox_list_remove(&di->ln);
    ox_list_init(&di->ln);

    return OX_OK;
}

/*?
 *? @lib {Dict} Dictionary.
 *? Dictionary is a data set to store values.
 *? Each value has a key with it.
 *? User can use key to lookup the value.
 *?
 *? @class{ Dict Dictionary class.
 *?
 *? @func add Add a new entry to the dictionary.
 *? If the dictionary already has an entry with the same key, change its value.
 *? @param k The key of the entry.
 *? @param v The value of the entry.
 *?
 *? @func get Lookup the entry's value use its key.
 *? @param k The key of the entry.
 *? @return The entry's value.
 *? If the dictionary has not the entry with the key, return false.
 *?
 *? @func remove Remove the entry from the dictionary.
 *? @param k The key of the entry.
 *? @return {Bool} Return true in success.\
 *? If cannot find the entry with the key, return false.
 *?
 *? @func $iter Create a new iterator to traverse the entries in the dictionary.
 *? The iterator's value is a 2 items array.
 *? The first item is the entry's key.
 *? The second item is the entry's value.
 *? @return {Iterator[[Any,Any]]} The iterator to trverse the entries.
 *?
 *? @func entries Create a new iterator to traverse the entries in the dictionary.
 *? The iterator's value is a 2 items array.
 *? The first item is the entry's key.
 *? The second item is the entry's value.
 *? @return {Iterator[[Any,Any]]} The iterator to trverse the entries.
 *?
 *? @func keys Create a new iterator to traverse the entries' keys in the dictionary.
 *? The iterator's value is the entry's key.
 *? @return {Iterator[Any]} The iterator to trverse the entries' keys.
 *?
 *? @func values Create a new iterator to traverse the entries' values in the dictionary.
 *? The iterator's value is the entry's value.
 *? @return {Iterator[Any]} The iterator to trverse the entries' values
 *?
 *? @func $to_json Help function for "JSON.to_str()".
 *? Convert the dictionary to an object.
 *? Each entry is converted to a property.
 *? The property's key is the entry's key.
 *? The property's value is the entry's value.
 *? @return {Object} The result object.
 *?
 *? @roacc length {Number} The entries' count in the dictionary..
 *?
 *? @class}
 */

/**
 * Initialize the dictionary class.
 */
extern void
ox_dict_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_3(ctxt, c, inf, iter)

    /*Dict.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Dict"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Dict", c));
    ox_not_error(ox_class_set_alloc_func(ctxt, c, dict_alloc));
    ox_value_copy(ctxt, OX_OBJECT(ctxt, Dict_inf), inf);

    /*Dict_inf.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "add", Dict_inf_add));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get", Dict_inf_get));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "remove", Dict_inf_remove));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$iter", Dict_inf_entries));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "keys", Dict_inf_keys));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "values", Dict_inf_values));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "entries", Dict_inf_entries));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_json", Dict_inf_to_json));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "length", Dict_inf_length_get, NULL));

    /*DictIterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, DictIterator_inf), c, "Iterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));

    /*DictIterator_inf.*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, DictIterator_inf),
            "end", DictIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, DictIterator_inf),
            "value", DictIterator_inf_value_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, DictIterator_inf),
            "next", DictIterator_inf_next));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, DictIterator_inf),
            "$close", DictIterator_inf_close));

    OX_VS_POP(ctxt, c)
}
