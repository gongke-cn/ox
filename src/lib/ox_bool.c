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
 * Boolean value.
 */

#define OX_LOG_TAG "ox_bool"

#include "ox_internal.h"

/*Get the owned keys in the boolean value.*/
static OX_Result
bool_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    ox_value_set_null(ctxt, keys);
    return OX_OK;
}

/*Lookup the owned property in the boolean value.*/
static OX_Result
bool_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    ox_value_set_null(ctxt, v);
    return OX_OK;
}

/*Get property value of a boolean value.*/
static OX_Result
bool_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_object_get_t(ctxt, OX_OBJECT(ctxt, Bool_inf), p, v, o);
}

/*Set property value of a boolean value.*/
static OX_Result
bool_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_type_error(ctxt, OX_TEXT("cannot set the property of a boolean value"));
}

/*Delete the owned property of the boolean value.*/
static OX_Result
bool_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    return OX_OK;
}

/*Call the boolean value.*/
static OX_Result
bool_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *r)
{
    ox_value_copy(ctxt, r, o);
    return OX_OK;
}

/*Operation functions of the boolean value.*/
const OX_ObjectOps
ox_bool_ops = {
    {},
    bool_keys,
    bool_lookup,
    bool_get,
    bool_set,
    bool_del,
    bool_call
};


/*Bool.is*/
static OX_Result
Bool_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_bool(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Bool.$inf.$to_str*/
static OX_Result
Bool_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Bool b;

    b = ox_to_bool(ctxt, thiz);

    ox_value_copy(ctxt, rv, b ? OX_STRING(ctxt, true) : OX_STRING(ctxt, false));
    return OX_OK;
}

/*Bool.$inf.$to_num*/
static OX_Result
Bool_inf_to_num (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Bool b;

    b = ox_to_bool(ctxt, thiz);

    ox_value_set_number(ctxt, rv, b ? 1 : 0);
    return OX_OK;
}

/*?
 *? @lib {Bool} Boolean value.
 *?
 *? @class{ Bool Boolean class.
 *?
 *? @func $to_str Convert the boolean value to string.
 *? @return {String} Return "true" if the boolean value is true,\
 *? or return "false" if the boolean value is false.
 *?
 *? @func $to_num Convert the boolean value to number.
 *? @return {Number} Return 1 if the boolean value is true,\
 *? or return 0 if the boolean value is false.
 *?
 *? @class}
 */

/**
 * Initialize the boolean class.
 * @param ctxt The current running context.
 */
void
ox_bool_class_init (OX_Context *ctxt)
{
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Bool), OX_OBJECT(ctxt, Bool_inf), NULL, "Bool"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Bool", OX_OBJECT(ctxt, Bool)));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Bool), "is", Bool_is));

    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Bool_inf), "$to_str", Bool_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Bool_inf), "$to_num", Bool_inf_to_num));
}

/**
 * Call the boolean class.
 * @param ctxt The current running context.
 * @param o The boolean class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_bool_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_to_bool(ctxt, arg);

    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}