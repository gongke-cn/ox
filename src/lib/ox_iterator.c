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
 * Iterator.
 */

#define OX_LOG_TAG "ox_iterator"

#include "ox_internal.h"

/**
 * Get a new iterator of the value.
 * @param ctxt The current running context.
 * @param[out] iter Return the new iterator.
 * @param v The value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_iterator_new (OX_Context *ctxt, OX_Value *iter, OX_Value *v)
{
    assert(ctxt && iter && v);

    if (ox_value_is_null(ctxt, v)) {
        ox_value_set_null(ctxt, iter);
        return OX_OK;
    }

    return ox_call_method(ctxt, v, OX_STRING(ctxt, _iter), NULL, 0, iter);
}

/**
 * Move the iterator to the next position.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_iterator_next (OX_Context *ctxt, OX_Value *iter)
{
    OX_VS_PUSH(ctxt, rv)
    OX_Result r;

    assert(ctxt && iter);

    r = ox_call_method(ctxt, iter, OX_STRING(ctxt, next), NULL, 0, rv);

    OX_VS_POP(ctxt, rv)
    return r;
}

/**
 * Check if the iterator is end.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @retval OX_TRUE The iterator is end.
 * @retval OX_FALSE The iterator is not end.
 * @retval OX_ERR On error.
 */
OX_Result
ox_iterator_end (OX_Context *ctxt, OX_Value *iter)
{
    OX_VS_PUSH(ctxt, v)
    OX_Result r;

    if (ox_value_is_null(ctxt, iter))
        return OX_TRUE;

    r = ox_get_throw(ctxt, iter, OX_STRING(ctxt, end), v);
    if (r == OX_OK)
        r = ox_to_bool(ctxt, v);

    OX_VS_POP(ctxt, v)
    return r;
}

/**
 * Get the iterator's current value.
 * @param ctxt The current running context.
 * @param iter The iterator.
 * @param[out] v Return the current value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_iterator_value (OX_Context *ctxt, OX_Value *iter, OX_Value *v)
{
    assert(ctxt && iter && v);

    return ox_get_throw(ctxt, iter, OX_STRING(ctxt, value), v);
}

/*Iterator.$inf.$iter*/
static OX_Result
Iterator_inf_iterator (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    ox_value_copy(ctxt, rv, thiz);
    return OX_OK;
}

/*Iterator.$inf.to_array*/
static OX_Result
Iterator_inf_to_array (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, item)
    size_t i = 0;
    OX_Result r;

    if ((r = ox_array_new(ctxt, rv, 0)) == OX_ERR)
        goto end;

    while (1) {
        if ((r = ox_iterator_end(ctxt, thiz)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, thiz, item)) == OX_ERR)
            goto end;

        if ((r = ox_array_set_item(ctxt, rv, i, item)) == OX_ERR)
            goto end;

        i ++;

        if ((r = ox_iterator_next(ctxt, thiz)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    r |= ox_close(ctxt, thiz);
    OX_VS_POP(ctxt, item)
    return r;
}

/** Iterator with a function.*/
typedef struct {
    OX_Value iter; /**< Base iterator.*/
    OX_Value fn;   /**< Map function.*/
} OX_FuncIter;

/*Scan referenced objects in the function iterator.*/
static void
func_iter_scan (OX_Context *ctxt, void *p)
{
    OX_FuncIter *fi = p;

    ox_gc_scan_value(ctxt, &fi->iter);
    ox_gc_scan_value(ctxt, &fi->fn);
}

/*Free the function iterator.*/
static void
func_iter_free (OX_Context *ctxt, void *p)
{
    OX_FuncIter *fi = p;

    OX_DEL(ctxt, fi);
}

/*Function iterator's operation functions.*/
static const OX_PrivateOps
func_iter_ops = {
    func_iter_scan,
    func_iter_free
};

/*Create a new function iterator.*/
static OX_FuncIter*
func_iter_new (OX_Context *ctxt, OX_Value *fi, OX_Value *iter, OX_Value *fn, OX_Value *inf)
{
    OX_Result r;
    OX_FuncIter *fip;

    if ((r = ox_object_new(ctxt, fi, inf)) == OX_ERR)
        return NULL;

    if (!OX_NEW(ctxt, fip)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    ox_value_copy(ctxt, &fip->iter, iter);
    ox_value_copy(ctxt, &fip->fn, fn);

    if ((r = ox_object_set_priv(ctxt, fi, &func_iter_ops, fip)) == OX_ERR) {
        func_iter_free(ctxt, fip);
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    return fip;
}

/*Get the function iterator data.*/
static OX_FuncIter*
func_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_FuncIter *fi = ox_object_get_priv(ctxt, v, &func_iter_ops);

    if (!fi)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not an iterator with function"));

    return fi;
}

/*Move to the next valid position.*/
static OX_Result
select_iter_select (OX_Context *ctxt, OX_FuncIter *fi)
{
    OX_VS_PUSH_2(ctxt, item, rv)
    OX_Result r;

    while (1) {
        if ((r = ox_iterator_end(ctxt, &fi->iter)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, &fi->iter, item)) == OX_ERR)
            goto end;

        if ((r = ox_call(ctxt, &fi->fn, ox_value_null(ctxt), item, 1, rv)) == OX_ERR)
            goto end;

        if (ox_to_bool(ctxt, rv))
            break;

        if ((r = ox_iterator_next(ctxt, &fi->iter)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Iterator.$inf.map*/
static OX_Result
Iterator_inf_map (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);

    if (!func_iter_new(ctxt, rv, thiz, fn, OX_OBJECT(ctxt, MapIterator_inf)))
        return OX_ERR;

    return OX_OK;
}

/*Iterator.$inf.select*/
static OX_Result
Iterator_inf_select (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);
    OX_FuncIter *fi;

    if (!(fi = func_iter_new(ctxt, rv, thiz, fn, OX_OBJECT(ctxt, SelectIterator_inf))))
        return OX_ERR;

    return select_iter_select(ctxt, fi);
}

/*Iterator.$inf.$to_str*/
static OX_Result
Iterator_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *sep_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *head_arg = ox_argument(ctxt, args, argc, 1);
    OX_Value *tail_arg = ox_argument(ctxt, args, argc, 2);
    OX_VS_PUSH_5(ctxt, v, s, sep, head, tail)
    OX_Result r;
    OX_CharBuffer cb;
    OX_Bool first = OX_TRUE;

    ox_char_buffer_init(&cb);

    if ((r = ox_to_string(ctxt, sep_arg, sep)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, head_arg, head)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, tail_arg, tail)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_string(ctxt, &cb, head)) == OX_ERR)
        goto end;

    while (1) {
        if ((r = ox_iterator_end(ctxt, thiz)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, thiz, v)) == OX_ERR)
            goto end;

        if ((r = ox_to_string(ctxt, v, s)) == OX_ERR)
            goto end;

        if (first) {
            first = OX_FALSE;
        } else {
            if ((r = ox_char_buffer_append_string(ctxt, &cb, sep)) == OX_ERR)
                goto end;
        }

        if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
            goto end;

        if ((r = ox_iterator_next(ctxt, thiz)) == OX_ERR)
            goto end;
    }

    if ((r = ox_char_buffer_append_string(ctxt, &cb, tail)) == OX_ERR)
        goto end;

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, v)
    return r;
}

/*FuncIterator.$inf.end get*/
static OX_Result
FuncIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FuncIter *fi;
    OX_Result r;

    if (!(fi = func_iter_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_iterator_end(ctxt, &fi->iter)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, r);
    return OX_OK;
}

/*FuncIterator.$inf.close*/
static OX_Result
FuncIterator_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FuncIter *fi;

    if (!(fi = func_iter_get(ctxt, thiz)))
        return OX_ERR;

    return ox_close(ctxt, &fi->iter);
}

/*MapIterator.$inf.value get*/
static OX_Result
MapIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)
    OX_FuncIter *fi;
    OX_Result r;

    if (!(fi = func_iter_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_iterator_value(ctxt, &fi->iter, v)) == OX_ERR)
        goto end;

    r = ox_call(ctxt, &fi->fn, ox_value_null(ctxt), v, 1, rv);
end:
    OX_VS_POP(ctxt, v)
    return r;
}

/*MapIterator.$inf.next*/
static OX_Result
MapIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)
    OX_FuncIter *fi;
    OX_Result r;

    if (!(fi = func_iter_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    r = ox_iterator_next(ctxt, &fi->iter);
end:
    OX_VS_POP(ctxt, v)
    return r;
}

/*SelectIterator.$inf.value get*/
static OX_Result
SelectIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)
    OX_FuncIter *fi;
    OX_Result r;

    if (!(fi = func_iter_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    r = ox_iterator_value(ctxt, &fi->iter, rv);
end:
    OX_VS_POP(ctxt, v)
    return r;
}

/*SelectIterator.$inf.next*/
static OX_Result
SelectIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FuncIter *fi;
    OX_Result r;

    if (!(fi = func_iter_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_iterator_next(ctxt, &fi->iter)) == OX_ERR)
        return r;

    return select_iter_select(ctxt, fi);
}

/*?
 *? @lib {Iterator} Iterator.
 *? Iterator is used to traverse the elements of an object.
 *? In OX language, you can use "for ... as" statement to traverse the iterator.
 *?
 *? @callback MapFunc Iterator element map function.
 *? @param input {Any} The input element value.
 *? @return {Any} The mapped element value.
 *?
 *? @callback SelectFunc Iterator element select function.
 *? @param e {Any} The element.
 *? @return {Bool} the element is selected or not.
 *? If the function returns true, the element will be added to generated iterator.
 *? If the function returns false, the element is ignored.
 *?
 *? @class{ Iterator Iterator class.
 *?
 *? @func $iter Return this iterator.
 *? @return {Iterator} Return this argument.
 *?
 *? @func to_array Traverse the elements of the iterator and collect them to an array.
 *? @return {Array} The array contains all the elements.
 *?
 *? @func map Map the iterator to another one.
 *? @param fn {MapFunc} Element map function.
 *? @return {Iterator} The iterator used to traverse the mapped elements.
 *?
 *? @func select Create a new iterator with selected elements of this iterator.
 *? @param fn {SelectFunc} Element select function.
 *? @return {Iterator} The iterator with selected elements.
 *?
 *? @func $to_str Traverse the elements of the iterator and join them into a string.
 *? @param sep {?String} If sep is not null, this string is used as separator between each element.
 *? @return {String} The result string.
 *?
 *? @class}
 */

/**
 * Initialize the iterator class.
 * @param ctxt The current running context.
 */
void
ox_iterator_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, c)

    /*Iterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Iterator), OX_OBJECT(ctxt, Iterator_inf), NULL, "Iterator"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Iterator", OX_OBJECT(ctxt, Iterator)));

    /*Iterator.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Iterator_inf), "$iter",
            Iterator_inf_iterator));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Iterator_inf), "to_array",
            Iterator_inf_to_array));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Iterator_inf), "map",
            Iterator_inf_map));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Iterator_inf), "select",
            Iterator_inf_select));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Iterator_inf), "$to_str",
            Iterator_inf_to_str));

    /*MapIterator*/
    ox_not_error(ox_named_class_new_s(ctxt, c, OX_OBJECT(ctxt, MapIterator_inf), NULL, "MapIterator"));
    ox_not_error(ox_class_inherit(ctxt, c, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, MapIterator_inf), "end",
            FuncIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, MapIterator_inf), "value",
            MapIterator_inf_value_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, MapIterator_inf), "next",
            MapIterator_inf_next));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, MapIterator_inf), "$close",
            FuncIterator_inf_close));

    /*SelectIterator*/
    ox_not_error(ox_named_class_new_s(ctxt, c, OX_OBJECT(ctxt, SelectIterator_inf), NULL, "SelectIterator"));
    ox_not_error(ox_class_inherit(ctxt, c, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SelectIterator_inf), "end",
            FuncIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SelectIterator_inf), "value",
            SelectIterator_inf_value_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, SelectIterator_inf), "next",
            SelectIterator_inf_next));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, SelectIterator_inf), "$close",
            FuncIterator_inf_close));

    OX_VS_POP(ctxt, c)
}
