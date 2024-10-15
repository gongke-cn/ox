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
 * Array.
 */

#define OX_LOG_TAG "ox_array"

#include "ox_internal.h"

/*Scan the referenced objects in the array.*/
static void
array_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Array *a = (OX_Array*)gco;

    ox_object_scan(ctxt, gco);
    ox_gc_scan_values(ctxt, a->items.items, a->items.len);
}

/*Free the array.*/
static void
array_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Array *a = (OX_Array*)gco;

    ox_vector_deinit(ctxt, &a->items);
    ox_object_deinit(ctxt, &a->o);

    OX_DEL(ctxt, a);
}

/*Get the property keys of the array.*/
static OX_Result
array_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    OX_VS_PUSH(ctxt, item)
    OX_Result r;
    size_t len, i;

    len = ox_array_length(ctxt, o);

    if ((r = ox_array_new(ctxt, keys, len)) == OX_ERR)
        goto end;

    for (i = 0; i < len; i ++) {
        ox_value_set_number(ctxt, item, i);
        if ((r = ox_array_set_item(ctxt, keys, i, item)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Lookup owned property of the array.*/
static OX_Result
array_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    if (ox_value_is_number(ctxt, p)) {
        size_t id = 0, len = ox_array_length(ctxt, o);
        OX_Result r;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            return r;

        if (id >= len) {
            ox_value_set_null(ctxt, v);
            return OX_OK;
        } else {
            return ox_array_get_item(ctxt, o, id, v);
        }
    } else {
        return ox_object_lookup(ctxt, o, p, v);
    }
}

/*Get property value of an array.*/
static OX_Result
array_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    if (ox_value_is_number(ctxt, p)) {
        size_t id = 0;
        OX_Result r;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            return r;

        if (id >= ox_array_length(ctxt, o))
            return OX_FALSE;

        return ox_array_get_item(ctxt, o, id, v);
    } else {
        return ox_object_get(ctxt, o, p, v);
    }
}

/*Set property value of an array.*/
static OX_Result
array_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    if (ox_value_is_number(ctxt, p)) {
        size_t id = 0;
        OX_Result r;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            return r;

        return ox_array_set_item(ctxt, o, id, v);
    } else {
        return ox_object_set(ctxt, o, p, v);
    }
}

/*Remove items of the array.*/
static OX_Result
array_remove (OX_Context *ctxt, OX_Value *o, size_t pos, size_t num)
{
    size_t len;

    len = ox_array_length(ctxt, o);

    if (pos < len) {
        size_t left, nlen;
        OX_Array *a;

        if (pos + num <= len) {
            left = len - pos - num;
            nlen = len - num;
        } else if (pos < len) {
            left = 0;
            nlen = pos;
        } else {
            left = 0;
            nlen = len;
        }

        a = ox_value_get_gco(ctxt, o);

        if (left)
            memmove(a->items.items + pos, a->items.items + pos + num, left * sizeof(OX_Value));

        a->items.len = nlen;
    }

    return OX_OK;
}

/*Delete a property of the array.*/
static OX_Result
array_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    if (ox_value_is_number(ctxt, p)) {
        size_t id = 0;
        OX_Result r;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            return r;

        return array_remove(ctxt, o, id, 1);
    } else {
        return ox_object_del(ctxt, o, p);
    }
}

/*Array operation functions.*/
static const OX_ObjectOps
array_ops = {
    {
        OX_GCO_ARRAY,
        array_scan,
        array_free
    },
    array_keys,
    array_lookup,
    array_get,
    array_set,
    array_del,
    ox_object_call
};

/*Allocate a new array.*/
static OX_Result
array_alloc (OX_Context *ctxt, OX_Value *a, OX_Value *inf)
{
    OX_Result r;

    if ((r = ox_array_new(ctxt, a, 0)) == OX_ERR)
        return r;

    return ox_object_set_interface(ctxt, a, inf);
}

/*Reset the array's length.*/
static OX_Result
array_set_length (OX_Context *ctxt, OX_Array *ap, size_t len)
{
    size_t old;
    OX_Result r;

    old = ox_vector_length(&ap->items);
    if ((r = ox_vector_expand(ctxt, &ap->items, len)) == OX_ERR)
        return ox_throw_range_error(ctxt,
                OX_TEXT("array's length (%"PRIdPTR") is too big"),
                len);

    if (len > old)
        ox_values_set_null(ctxt, &ox_vector_item(&ap->items, old), len - old);

    ap->items.len = len;

    return OX_OK;
}

/**
 * Create a new array.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param len The length of the array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_new (OX_Context *ctxt, OX_Value *a, size_t len)
{
    OX_Array *ap;
    OX_Result r;

    assert(ctxt && a);

    if (!OX_NEW(ctxt, ap))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &ap->o, OX_OBJECT(ctxt, Array_inf));
    ox_vector_init(&ap->items);
    ap->o.gco.ops = (OX_GcObjectOps*)&array_ops;

    ox_value_set_gco(ctxt, a, ap);
    ox_gc_add(ctxt, ap);

    if (len)
        r = array_set_length(ctxt, ap, len);
    else
        r = OX_OK;

    return r;
}

/**
 * Reset the array's length.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param len The new length of the array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_set_length (OX_Context *ctxt, OX_Value *a, size_t len)
{
    OX_Array *ap;

    assert(ctxt && a);
    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);

    return array_set_length(ctxt, ap, len);
}

/**
 * Set the array item's value.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The item's index.
 * @param iv The item's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_set_item (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv)
{
    OX_Array *ap;
    size_t len;

    assert(ctxt && a && iv);
    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);
    len = ox_array_length(ctxt, a);

    if (id >= len) {
        OX_Result r;

        if ((r = ox_array_set_length(ctxt, a, id + 1)) == OX_ERR)
            return r;
    }

    ox_value_copy(ctxt, &ox_vector_item(&ap->items, id), iv);
    return OX_OK;
}

/**
 * Set the array items' values.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The first item's index.
 * @param iv The items' new values.
 * @param n Number of values.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_set_items (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv, size_t n)
{
    OX_Array *ap;
    OX_Result r;

    assert(ctxt && a);
    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);

    if ((r = ox_array_set_length(ctxt, a, id + n)) == OX_ERR)
        return r;

    ox_values_copy(ctxt, ap->items.items + id, iv, n);
    return OX_OK;
}

/**
 * Append an item to the array's tail.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param iv The new item's value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_append (OX_Context *ctxt, OX_Value *a, OX_Value *iv)
{
    OX_Array *ap;
    size_t len;
    OX_Result r;

    assert(ctxt && a && iv);
    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);
    len = ox_array_length(ctxt, a);

    if ((r = ox_array_set_length(ctxt, a, len + 1)) == OX_ERR)
        return r;

    ox_value_copy(ctxt, &ox_vector_item(&ap->items, len), iv);
    return OX_OK;
}

/**
 * Get the array item's value.
 * @param ctxt The current running context.
 * @param a The array value.
 * @param id The item's index.
 * @param[out] iv Return the item's value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_array_get_item (OX_Context *ctxt, OX_Value *a, size_t id, OX_Value *iv)
{
    OX_Array *ap;
    size_t len;

    assert(ctxt && a && iv);
    assert(ox_value_is_array(ctxt, a));

    ap = ox_value_get_gco(ctxt, a);
    len = ox_array_length(ctxt, a);

    if (id >= len)
        ox_value_set_null(ctxt, iv);
    else
        ox_value_copy(ctxt, iv, &ox_vector_item(&ap->items, id));

    return OX_OK;
}

/*Check if the value is an array.*/
static OX_Result
check_array (OX_Context *ctxt, OX_Value *a)
{
    if (!ox_value_is_array(ctxt, a))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not an array"));

    return OX_OK;
}

/*Array.$inf.length get*/
static OX_Result
Array_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    size_t len;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    len = ox_array_length(ctxt, thiz);
    ox_value_set_number(ctxt, rv, len);

    return OX_OK;
}

/*Array.$inf.length set*/
static OX_Result
Array_inf_length_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *lenv = ox_argument(ctxt, args, argc, 0);
    size_t len = 0;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    if ((r = ox_to_index(ctxt, lenv, &len)) == OX_ERR)
        return r;

    return ox_array_set_length(ctxt, thiz, len);
}

/*Array.is*/
static OX_Result
Array_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_array(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Array.$inf.$to_str*/
static OX_Result
Array_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *sep_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_3(ctxt, item, sep, s)
    OX_Result r;
    size_t i, len;
    OX_CharBuffer cb;

    ox_char_buffer_init(&cb);

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, sep_arg)) {
        if ((r = ox_to_string(ctxt, sep_arg, sep)) == OX_ERR)
            goto end;
    }

    len = ox_array_length(ctxt, thiz);
    for (i = 0; i < len; i ++) {
        if ((r = ox_array_get_item(ctxt, thiz, i, item)) == OX_ERR)
            goto end;

        if ((r = ox_to_string(ctxt, item, s)) == OX_ERR)
            goto end;

        if (i) {
            if (ox_value_is_null(ctxt, sep)) {
                if ((r = ox_char_buffer_append_char(ctxt, &cb, ',')) == OX_ERR)
                    goto end;
            } else {
                if ((r = ox_char_buffer_append_string(ctxt, &cb, sep)) == OX_ERR)
                    goto end;
            }
        }

        if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, item)
    return r;
}

/*Array.$inf.push*/
static OX_Result
Array_inf_push (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    if (argc) {
        size_t len = ox_array_length(ctxt, thiz);
        size_t i;

        if ((r = ox_array_set_length(ctxt, thiz, len + argc)) == OX_ERR)
            return r;

        for (i = 0; i < argc; i ++) {
            OX_Value *arg = ox_values_item(ctxt, args, i);

            if ((r = ox_array_set_item(ctxt, thiz, len + i, arg)) == OX_ERR)
                return r;
        }
    }

    return OX_OK;
}

/*Array.$inf.pop*/
static OX_Result
Array_inf_pop (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Result r;
    size_t len;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    len = ox_array_length(ctxt, thiz);
    if (len) {
        if ((r = ox_array_get_item(ctxt, thiz, len - 1, rv)) == OX_ERR)
            return r;

        if ((r = ox_array_set_length(ctxt, thiz, len - 1)) == OX_ERR)
            return r;
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/** Array iterator data.*/
typedef struct {
    OX_Value a;  /**< The array.*/
    size_t   id; /**< Current item index.*/
} OX_ArrayIter;

/*Scan referenced objects in the array iterator.*/
static void
array_iter_scan (OX_Context *ctxt, void *ptr)
{
    OX_ArrayIter *ai = ptr;

    ox_gc_scan_value(ctxt, &ai->a);
}

/*Free the array iterator data.*/
static void
array_iter_free (OX_Context *ctxt, void *ptr)
{
    OX_ArrayIter *ai = ptr;

    OX_DEL(ctxt, ai);
}

/*Array iterator's operation functions.*/
static const OX_PrivateOps
array_iter_ops = {
    array_iter_scan,
    array_iter_free
};

/*Create a new array iterator.*/
static OX_Result
array_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *a)
{
    OX_Result r;
    OX_ArrayIter *ai;

    if ((r = check_array(ctxt, a)) == OX_ERR)
        return r;

    if ((r = ox_object_new(ctxt, iter, OX_OBJECT(ctxt, ArrayIterator_inf))) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, ai))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &ai->a, a);
    ai->id = 0;

    if ((r = ox_object_set_priv(ctxt, iter, &array_iter_ops, ai)) == OX_ERR) {
        OX_DEL(ctxt, ai);
        return r;
    }

    return OX_OK;
}

/*Array.$inf.$iter*/
static OX_Result
Array_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return array_iter_new(ctxt, rv, thiz);
}

/*Array.$inf.insert*/
static OX_Result
Array_inf_insert (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 0);
    size_t pos = 0;
    OX_Result r;
    size_t add;
    size_t len, nlen, left;
    OX_Value *src;
    OX_Array *a;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    if (argc >= 2) {
        ssize_t p;

        add = argc - 1;
        a = ox_value_get_gco(ctxt, thiz);

        len = ox_array_length(ctxt, thiz);

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            return r;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        pos = p;

        if (pos <= len) {
            nlen = len + add;
            left = len - pos;
        } else {
            nlen = pos + add;
            left = 0;
        }

        if ((r = ox_array_set_length(ctxt, thiz, nlen)) == OX_ERR)
            return r;

        if (left)
            memmove(a->items.items + pos + add, a->items.items + pos, sizeof(OX_Value) * left);

        src = ox_values_item(ctxt, args, 1);
        ox_values_copy(ctxt, a->items.items + pos, src, add);
    }

    return OX_OK;
}

/*Array.$inf.remove*/
static OX_Result
Array_inf_remove (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *num_arg = ox_argument(ctxt, args, argc, 1);
    size_t pos = 0, num = 1;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        return r;

    if (!ox_value_is_null(ctxt, pos_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            return r;

        if (p < 0) {
            p = ox_array_length(ctxt, thiz) + p;
            if (p < 0)
                p = 0;
        }

        pos = p;
    }

    if (!ox_value_is_null(ctxt, num_arg)) {
        if ((r = ox_to_index(ctxt, num_arg, &num)) == OX_ERR)
            return r;
    }

    return array_remove(ctxt, thiz, pos, num);
}

/*Array.$inf.slice*/
static OX_Result
Array_inf_slice (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *start_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *end_arg = ox_argument(ctxt, args, argc, 1);
    size_t start = 0, end = 0, len, nlen, src, dst;
    OX_VS_PUSH(ctxt, item)
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        goto end;

    len = ox_array_length(ctxt, thiz);

    if (!ox_value_is_null(ctxt, start_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, start_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        start = p;
    }

    if (!ox_value_is_null(ctxt, end_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, end_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        end = p;
    } else {
        end = len;
    }

    if (start >= len)
        start = len;
    if (end >= len)
        end = len;
    if (end < start)
        end = start;

    nlen = end - start;

    if ((r = ox_array_new(ctxt, rv, nlen)) == OX_ERR)
        goto end;

    for (dst = 0, src = start; dst < nlen; src ++, dst ++) {
        if ((r = ox_array_get_item(ctxt, thiz, src, item)) == OX_ERR)
            goto end;
        if ((r = ox_array_set_item(ctxt, rv, dst, item)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Array.$inf.has*/
static OX_Result
Array_inf_has (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *start_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, item)
    size_t len, start = 0, i;
    OX_Bool b = OX_FALSE;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        goto end;

    len = ox_array_length(ctxt, thiz);

    if (argc > 1) {
        ssize_t p;

        if ((ox_to_ssize(ctxt, start_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        start = p;
    }

    for (i = start; i < len; i ++) {
        if ((r = ox_array_get_item(ctxt, thiz, i, item)) == OX_ERR)
            goto end;
        if ((r = ox_equal(ctxt, item, arg))) {
            b = OX_TRUE;
            break;
        }
    }

    ox_value_set_bool(ctxt, rv, b);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Array.$inf.find*/
static OX_Result
Array_inf_find (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *start_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, item)
    size_t len, start = 0, i;
    ssize_t idx = -1;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        goto end;

    len = ox_array_length(ctxt, thiz);

    if (argc > 1) {
        ssize_t p;

        if ((ox_to_ssize(ctxt, start_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        start = p;
    }

    for (i = start; i < len; i ++) {
        if ((r = ox_array_get_item(ctxt, thiz, i, item)) == OX_ERR)
            goto end;
        if ((r = ox_equal(ctxt, item, arg))) {
            idx = i;
            break;
        }
    }

    ox_value_set_number(ctxt, rv, idx);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Sort compare result.*/
enum {
    OX_CMP_LESS,
    OX_CMP_EQUAL,
    OX_CMP_GREATER
};

/*Sort compare parameters.*/
typedef struct {
    OX_Value *fn;   /**< The function.*/
    OX_Value *a;    /**< The array.*/
    OX_Value *args; /**< Arguments of the function.*/
    OX_Value *rv;   /**< Result of the function.*/
    OX_Value *s1;   /**< String 1.*/
    OX_Value *s2;   /**< String 2.*/
    OX_Value *tmp;  /**< Temporary value.*/
} OX_SortParams;

/*Compare function.*/
static int
sort_cmp_func (OX_Context *ctxt, OX_SortParams *p, size_t i, size_t j)
{
    if (i == j) {
        return OX_CMP_EQUAL;
    } else {
        OX_Array *a = ox_value_get_gco(ctxt, p->a);
        OX_Value *v1 = &ox_vector_item(&a->items, i);
        OX_Value *v2 = &ox_vector_item(&a->items, j);
        OX_Result r;

        if (p->fn) {
            OX_Number n;

            ox_value_copy(ctxt, p->args, v1);
            ox_value_copy(ctxt, ox_values_item(ctxt, p->args, 1), v2);

            if ((r = ox_call(ctxt, p->fn, ox_value_null(ctxt), p->args, 2, p->rv)) == OX_ERR)
                return r;

            if ((r = ox_to_number(ctxt, p->rv, &n)) == OX_ERR)
                return r;

            if (n == 0)
                return OX_CMP_EQUAL;
            if (n > 0)
                return OX_CMP_GREATER;

            return OX_CMP_LESS;
        } else {
            if (ox_value_is_string(ctxt, v1) || ox_value_is_string(ctxt, v2)) {
                int ir;

                if ((r = ox_to_string(ctxt, v1, p->s1)) == OX_ERR)
                    return r;

                if ((r = ox_to_string(ctxt, v2, p->s2)) == OX_ERR)
                    return r;

                ir = ox_string_compare(ctxt, p->s1, p->s2);

                if (ir == 0)
                    return OX_CMP_EQUAL;
                if (ir > 0)
                    return OX_CMP_GREATER;

                return OX_CMP_LESS;
            } else {
                OX_Number n1, n2, nr;

                if ((r = ox_to_number(ctxt, v1, &n1)) == OX_ERR)
                    return r;

                if ((r = ox_to_number(ctxt, v2, &n2)) == OX_ERR)
                    return r;

                nr = n1 - n2;

                if (nr == 0)
                    return OX_CMP_EQUAL;
                if (nr > 0)
                    return OX_CMP_GREATER;

                return OX_CMP_LESS;
            }
        }
    }
}

/*Swap the values.*/
static void
swap (OX_Context *ctxt, OX_SortParams *p, size_t i, size_t j)
{
    if (i != j) {
        OX_Array *a = ox_value_get_gco(ctxt, p->a);
        OX_Value *v1 = &ox_vector_item(&a->items, i);
        OX_Value *v2 = &ox_vector_item(&a->items, j);

        ox_value_copy(ctxt, p->tmp, v1);
        ox_value_copy(ctxt, v1, v2);
        ox_value_copy(ctxt, v2, p->tmp);
    }
}

/*Quick sort.*/
static OX_Result
sort (OX_Context *ctxt, OX_SortParams *p, ssize_t low, ssize_t high)
{
    if (low < high) {
        ssize_t i = low, j = high - 1, pivot = high;
        OX_Result r;

        while (1) {
            while (i < high) {
                if ((r = sort_cmp_func(ctxt, p, i, pivot)) == OX_ERR)
                    return r;
                if (r == OX_CMP_GREATER)
                    break;
                i ++;
            }

            while (j >= low) {
                if ((r = sort_cmp_func(ctxt, p, j, pivot)) == OX_ERR)
                    return r;
                if (r == OX_CMP_LESS)
                    break;
                j --;
            }

            if (i <= j) {
                swap(ctxt, p, i, j);
                i ++;
                j --;
            }

            if (i > j)
                break;
        }

        swap(ctxt, p, i, pivot);

        if ((r = sort(ctxt, p, low, i - 1)) == OX_ERR)
            return r;

        if ((r = sort(ctxt, p, i + 1, high)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Array.$inf.sort*/
static OX_Result
Array_inf_sort (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);
    OX_Value *top = ox_value_stack_top(ctxt);
    OX_SortParams p;
    size_t len;
    OX_Result r;

    if ((r = check_array(ctxt, thiz)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, fn)) {
        p.fn = NULL;
        p.s1 = ox_value_stack_push_n(ctxt, 3);
        p.s2 = ox_values_item(ctxt, p.s1, 1);
        p.tmp = ox_values_item(ctxt, p.s1, 2);
    } else {
        p.fn = fn;
        p.args = ox_value_stack_push_n(ctxt, 4);
        p.rv = ox_values_item(ctxt, p.args, 2);
        p.tmp = ox_values_item(ctxt, p.args, 3);
    }

    p.a = thiz;

    len = ox_array_length(ctxt, thiz);

    if ((r = sort(ctxt, &p, 0, len - 1)) == OX_ERR)
        goto end;

    ox_value_copy(ctxt, rv, thiz);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, top)
    return r;
}

/*Check if the value is an array iterator.*/
static OX_ArrayIter*
get_array_iter (OX_Context *ctxt, OX_Value *v)
{
    OX_ArrayIter *ai;

    if (!(ai = ox_object_get_priv(ctxt, v, &array_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not an array iterator"));

    return ai;
}

/*ArrayIteraror.$inf.next*/
static OX_Result
ArrayIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ArrayIter *ai;
    size_t len;

    if (!(ai = get_array_iter(ctxt, thiz)))
        return OX_ERR;

    len = ox_array_length(ctxt, &ai->a);

    if (ai->id < len)
        ai->id ++;

    return OX_OK;
}

/*ArrayIteraror.$inf.end get*/
static OX_Result
ArrayIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ArrayIter *ai;
    size_t len;
    OX_Bool b;

    if (!(ai = get_array_iter(ctxt, thiz)))
        return OX_ERR;

    len = ox_array_length(ctxt, &ai->a);

    b = (ai->id >= len);

    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*ArrayIteraror.$inf.value get*/
static OX_Result
ArrayIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ArrayIter *ai;

    if (!(ai = get_array_iter(ctxt, thiz)))
        return OX_ERR;

    return ox_array_get_item(ctxt, &ai->a, ai->id, rv);
}

/*?
 *? @lib {Array} Array.
 *?
 *? @callback CompareFunc Items compare function.
 *? @param v1 The item 1.
 *? @param v2 The item 2.
 *? @return {Number} Return the compare result.
 *? @ul{
 *? @li 0 v1 equal to v2.
 *? @li <0 v1 less than v2.
 *? @li >0 v1 greater than v2.
 *? @ul}
 *?
 *? @class{ Array Array class.
 *?
 *? @acc length {Number} The items' count in the array.
 *?
 *? @func $to_str Convert the array to a string.
 *? The function convert the array's items to string, and join them with a separator ",".
 *? @return {String} The result string.
 *?
 *? @func push Append items to the end of the array.
 *? The arguments of the function will be appended.
 *? If the array has N items.
 *? Invoke "push" with M aguments.
 *? The new length of array is N + M.
 *? The value of item N + 1 equal to argument 0.
 *? The value of item N + 2 equal to argument 1.
 *? And so on.
 *?
 *? @func pop Popup the last item from the array.
 *? If the array has N items.
 *? After invoking "pop", the new length of array is N - 1.
 *? And the result value is equal to the item N.
 *? @return {Any} The last item's value.
 *? If the array's length is 0, return null.
 *?
 *? @func $iter Create an iterator to traverse the items' values in the array.
 *? The value of iterator is the item's value.
 *? @return {Iterator[Any]} The new iterator used to traverse the items.
 *?
 *? @func insert Insert items to the array.
 *? @param pos {Number} The first item's insert positions.
 *? If pos < 0, the insert position is the array's length + pos.
 *? @param ...items Items' values to the inserted.
 *?
 *? @func remove Remove items from the array.
 *? @param pos {Number} The first item's position to be removed.
 *? If pos < 0, the position is the array's length + pos.
 *? @param num {Number} =1 The number of items to be removed.
 *?
 *? @func slice Use the slice of the items to create a new array.
 *? @param begin {Number} The first item's position.
 *? If begin < 0, the first item position is the array's length + begin.
 *? @param end {Number} =this.length The last item's position + 1.
 *? If end < 0, the position is the array's length + end.
 *? @return {[Any]} The new slice array.
 *?
 *? @func has Check if the array has the item.
 *? @param v The value of the item.
 *? @return {Bool} The array has the item or not.
 *?
 *? @func find Search the item's position in the array.
 *? @param v The value of the item.
 *? @return {Number} The item's position in the array.
 *? If cannot find the item, return -1.
 *?
 *? @func sort Sort the items in the array.
 *? @param fn {?CompareFunc} The compare function.
 *? If fn is null, use the default compare function.
 *? If item1 or item2 is string, use function String.$inf.compare.
 *? Otherwise convert the items to numbers, return the value number1 - number2.
 *?
 *? @class}
 */

/**
 * Initialize the array class.
 * @param ctxt The current running context.
 */
void
ox_array_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_2(ctxt, c, iter)

    /*Array*/
    ox_not_error(ox_named_class_new_s(ctxt, c, OX_OBJECT(ctxt, Array_inf), NULL, "Array"));
    ox_not_error(ox_class_set_alloc_func(ctxt, c, array_alloc));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Array", c));

    ox_not_error(ox_object_add_n_method_s(ctxt, c, "is", Array_is));

    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, Array_inf), "length",
            Array_inf_length_get, Array_inf_length_set));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "$to_str",
            Array_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "push",
            Array_inf_push));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "pop",
            Array_inf_pop));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "$iter",
            Array_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "insert",
            Array_inf_insert));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "remove",
            Array_inf_remove));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "slice",
            Array_inf_slice));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "has",
            Array_inf_has));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "find",
            Array_inf_find));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Array_inf), "sort",
            Array_inf_sort));

    /*Array Iterator*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, ArrayIterator_inf),
            c, "Iterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, ArrayIterator_inf), "next",
            ArrayIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ArrayIterator_inf), "end",
            ArrayIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ArrayIterator_inf), "value",
            ArrayIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, c)
}