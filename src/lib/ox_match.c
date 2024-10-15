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
 * Match result.
 */

#define OX_LOG_TAG "ox_match"

#include "ox_internal.h"

/*Scan referenced objects in the match result data.*/
static void
match_data_scan (OX_Context *ctxt, void *p)
{
    OX_Match *m = p;

    ox_gc_scan_value(ctxt, &m->s);
    ox_gc_scan_value(ctxt, &m->sub);
    ox_gc_scan_value(ctxt, &m->group_strs);
    ox_gc_scan_value(ctxt, &m->group_slices);
}

/*Free the match result data.*/
static void
match_data_free (OX_Context *ctxt, void *p)
{
    OX_Match *m = p;

    if (m->slices)
        OX_DEL_N(ctxt, m->slices, m->group_num);

    OX_DEL(ctxt, m);
}

/*Operation functions of match result data.*/
static const OX_PrivateOps
match_data_ops = {
    match_data_scan,
    match_data_free
};

/*Get the match data from the value.*/
static OX_Match*
match_data_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Match *m = ox_object_get_priv(ctxt, v, &match_data_ops);

    if (!m)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a regular expression match result"));

    return m;
}

/*Get the match string.*/
static OX_Result
match_get_str (OX_Context *ctxt, OX_Match *m)
{
    OX_Result r;

    if (ox_value_is_null(ctxt, &m->sub)) {
        if ((r = ox_string_substr(ctxt, &m->s, m->start, m->end - m->start, &m->sub)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Operation functions of match result object.*/
static const OX_ObjectOps
match_ops = {
    {
        OX_GCO_MATCH,
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
 * Create a new match object.
 * @param ctxt The current running context.
 * @param[out] mv Return the match result object.
 * @param s The origin string.
 * @param start Start position of match.
 * @param end End position of match.
 * @param n_group Sub group number.
 * @param slices Slices of each group.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_match_new (OX_Context *ctxt, OX_Value *mv, OX_Value *s,
        size_t start, size_t end, int n_group, OX_Slice *slices)
{
    OX_Result r;
    OX_Match *m;

    if ((r = ox_object_new(ctxt, mv, OX_OBJECT(ctxt, Match_inf))) == OX_ERR)
        return r;

    if ((r = ox_object_set_ops(ctxt, mv, &match_ops)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, m))
        return ox_throw_no_mem_error(ctxt);

    m->group_num = n_group;

    if (!OX_NEW_N(ctxt, m->slices, m->group_num)) {
        OX_DEL(ctxt, m);
        return ox_throw_no_mem_error(ctxt);
    }

    ox_value_copy(ctxt, &m->s, s);
    ox_value_set_null(ctxt, &m->sub);
    ox_value_set_null(ctxt, &m->group_strs);
    ox_value_set_null(ctxt, &m->group_slices);

    memcpy(m->slices, slices, sizeof(OX_Slice) * m->group_num);

    m->start = start;
    m->end = end;

    if ((r = ox_object_set_priv(ctxt, mv, &match_data_ops, m)) == OX_ERR) {
        match_data_free(ctxt, m);
        return r;
    }

    return OX_OK;
}

/**
 * Get the match data from the match object.
 * @param ctxt The current running context.
 * @param v The match object value.
 * @return The match data.
 */
OX_Match*
ox_match_get_data (OX_Context *ctxt, OX_Value *v)
{
    return ox_object_get_priv(ctxt, v, &match_data_ops);
}

/*Match.$inf.$to_str*/
static OX_Result
Match_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Match *m;
    OX_Result r;

    if (!(m = match_data_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = match_get_str(ctxt, m)) == OX_ERR)
        return r;

    ox_value_copy(ctxt, rv, &m->sub);
    return OX_OK;
}

/*Match.$inf.start get*/
static OX_Result
Match_inf_start_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Match *m;

    if (!(m = match_data_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, m->start);
    return OX_OK;
}

/*Match.$inf.end get*/
static OX_Result
Match_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Match *m;

    if (!(m = match_data_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, m->end);
    return OX_OK;
}

/*Match.$inf.groups get*/
static OX_Result
Match_inf_groups_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, item)
    OX_Match *m;
    OX_Result r;

    if (!(m = match_data_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (ox_value_is_null(ctxt, &m->group_strs)) {
        size_t i;

        if ((r = ox_array_new(ctxt, &m->group_strs, m->group_num)) == OX_ERR)
            goto end;

        if ((r = match_get_str(ctxt, m)) == OX_ERR)
            goto end;

        if ((r = ox_array_set_item(ctxt, &m->group_strs, 0, &m->sub)) == OX_ERR)
            goto end;

        for (i = 1; i < m->group_num; i ++) {
            OX_Slice *slice = &m->slices[i];

            if ((slice->start == -1) || (slice->end == -1)) {
                ox_value_set_null(ctxt, item);
            } else if ((r = ox_string_substr(ctxt, &m->s, slice->start, slice->end - slice->start, item)) == OX_ERR) {
                goto end;
            }

            if ((r = ox_array_set_item(ctxt, &m->group_strs, i, item)) == OX_ERR)
                goto end;
        }
    }

    ox_value_copy(ctxt, rv, &m->group_strs);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Create a new slice.*/
static OX_Result
match_slice_new (OX_Context *ctxt, OX_Value *slice, size_t start, size_t end)
{
    OX_VS_PUSH(ctxt, item)
    OX_Result r;

    if ((r = ox_array_new(ctxt, slice, 2)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, item, start);
    if ((r = ox_array_set_item(ctxt, slice, 0, item)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, item, end);
    if ((r = ox_array_set_item(ctxt, slice, 1, item)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Match.$inf.slices get*/
static OX_Result
Match_inf_slices_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, item)
    OX_Match *m;
    OX_Result r;

    if (!(m = match_data_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (ox_value_is_null(ctxt, &m->group_slices)) {
        size_t i;

        if ((r = ox_array_new(ctxt, &m->group_slices, m->group_num)) == OX_ERR)
            goto end;

        if ((r = match_slice_new(ctxt, item, m->start, m->end)) == OX_ERR)
            goto end;

        if ((r = ox_array_set_item(ctxt, &m->group_slices, 0, item)) == OX_ERR)
            goto end;

        for (i = 1; i < m->group_num; i ++) {
            OX_Slice *slice = &m->slices[i];

            if ((r = match_slice_new(ctxt, item, slice->start, slice->end)) == OX_ERR)
                goto end;

            if ((r = ox_array_set_item(ctxt, &m->group_slices, i, item)) == OX_ERR)
                goto end;
        }
    }

    ox_value_copy(ctxt, rv, &m->group_slices);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*?
 *? @lib {Match} Match result.
 *?
 *? @class{ Match The match result.
 *? The match result is the result object of "String.$inf.match".
 *?
 *? @roacc start {Number} Start position of the matched substring.
 *? The start position is the first character's index.
 *? @roacc end {Number} End position of the matched substring.
 *? The end position is the last character's index plus one.
 *? @roacc groups {[String]} The matched slices array.
 *? Item 0 of groups is the substring.
 *? In regular expression, the middle part of the parentheses defines a match slice.
 *? Items >= 1 are the substrings of matched slices.
 *? @roacc slices {[Number,Number]} The matched slices array.
 *? The item of slices is an 2 items array, item 0 is the start position of the slice,
 *? item 1 is the end position of the slice.
 *? Item 0 of slices is the start and end position of the matched substring.
 *? Items >=1 are the positions of the matched slices.
 *?
 *? @func $to_str Get the matched substring.
 *? @return {String} The matched substring.
 *?
 *? @class}
 */

/**
 * Initialize the match class.
 * @param ctxt The current running context.
 */
void
ox_match_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, m)

    /*ReMatch.*/
    ox_not_error(ox_named_class_new_s(ctxt, m, OX_OBJECT(ctxt, Match_inf), NULL, "Match"));
    ox_not_error(ox_class_inherit(ctxt, m, OX_OBJECT(ctxt, String)));

    /*ReMatch_inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Match_inf), "$to_str", Match_inf_to_str));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, Match_inf), "start", Match_inf_start_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, Match_inf), "end", Match_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, Match_inf), "groups", Match_inf_groups_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, Match_inf), "slices", Match_inf_slices_get, NULL));

    OX_VS_POP(ctxt, m)
}
