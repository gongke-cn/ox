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
 * Operation.
 */

#ifndef _OX_OPERATION_H_
#define _OX_OPERATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Boolean value operation functions.*/
extern const OX_ObjectOps ox_bool_ops;
/** Number value operation functions.*/
extern const OX_ObjectOps ox_number_ops;

/**
 * Get the value's object operation functions.
 */
static inline const OX_ObjectOps*
ox_value_get_object_ops (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    switch (ox_value_get_tag(v)) {
    case OX_VALUE_TAG_NULL:
        return NULL;
    case OX_VALUE_TAG_BOOL:
        return &ox_bool_ops;
    case OX_VALUE_TAG_GCO: {
        OX_GcObject *gco = ox_value_pointer_get_gco(v);

        assert(gco->ops->type & OX_GCO_FL_OPS);

        return (const OX_ObjectOps*)gco->ops;
    }
    default:
        return &ox_number_ops;
    }
}

/**
 * Lookup the property owned by the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    const OX_ObjectOps *ops;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    return ops->lookup(ctxt, o, p, v);
}

/**
 * Lookup the property owned by the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_lookup_s (OX_Context *ctxt, OX_Value *o, const char *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_lookup(ctxt, o, pv, v);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Get property value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    const OX_ObjectOps *ops;
    OX_Result r;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    r = ops->get(ctxt, o, p, v);
    if (r == OX_FALSE) {
        ox_value_set_null(ctxt, v);
        r = OX_OK;
    }

    return r;
}

/**
 * Get property value with a C string property key.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_get_s (OX_Context *ctxt, OX_Value *o, const char *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_get(ctxt, o, pv, v);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Throw the property is null error.
 * @param ctxt The current running context.
 * @param p The property key.
 * @retval OX_ERR
 */
extern OX_Result
ox_throw_null_property_error (OX_Context *ctxt, OX_Value *p);

/**
 * Get property value.
 * If the property does not exist, throw an error.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_get_throw (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    const OX_ObjectOps *ops;
    OX_Result r;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    r = ops->get(ctxt, o, p, v);
    if (r == OX_FALSE)
        r = ox_throw_null_property_error(ctxt, p);

    return r;
}

/**
 * Get property value with a C string property key.
 * If the property does not exist, throw an error.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_get_throw_s (OX_Context *ctxt, OX_Value *o, const char *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_get_throw(ctxt, o, pv, v);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Set property value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param v The property's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    const OX_ObjectOps *ops;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    return ops->set(ctxt, o, p, v);
}

/**
 * Set property value with a C string property key.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @param v The property's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_set_s (OX_Context *ctxt, OX_Value *o, const char *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_set(ctxt, o, pv, v);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Call the value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param rv The return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    const OX_ObjectOps *ops;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    return ops->call(ctxt, o, thiz, args, argc, rv);
}

/**
 * Delete a property of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property key.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    const OX_ObjectOps *ops;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    return ops->del(ctxt, o, p);
}

/**
 * Get the property keys of the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param[out] keys Return the property keys.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    const OX_ObjectOps *ops;

    if (!(ops = ox_value_get_object_ops(ctxt, o)))
        return ox_throw_null_error(ctxt, OX_TEXT("the value is null"));

    return ops->keys(ctxt, o, keys);
}

/**
 * Convert the value to boolean.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The boolean value.
 */
static inline OX_Bool
ox_to_bool (OX_Context *ctxt, OX_Value *v)
{
    switch (ox_value_get_type(ctxt, v)) {
    case OX_VALUE_NULL:
        return OX_FALSE;
    case OX_VALUE_BOOL:
        return ox_value_get_bool(ctxt, v);
    case OX_VALUE_NUMBER:
        return ox_value_get_number(ctxt, v) != 0;
    case OX_VALUE_GCO:
        if (ox_value_is_string(ctxt, v))
            return ox_string_length(ctxt, v) != 0;
        return OX_TRUE;
    default:
        return OX_FALSE;
    }
}

/**
 * Convert the value to number inner function.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pn Return the number value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_number_inner (OX_Context *ctxt, OX_Value *v, OX_Number *pn);

/**
 * Convert the value to number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pn Return the number value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_number (OX_Context *ctxt, OX_Value *v, OX_Number *pn)
{
    OX_Result r = OX_OK;

    switch (ox_value_get_type(ctxt, v)) {
    case OX_VALUE_NULL:
        *pn = 0;
        break;
    case OX_VALUE_NUMBER:
        *pn = ox_value_get_number(ctxt, v);
        break;
    default:
        r = ox_to_number_inner(ctxt, v, pn);
        break;
    }

    return r;
}

/**
 * Convert the value to string inner function.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] s Return the string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_string_inner (OX_Context *ctxt, OX_Value *v, OX_Value *s);

/**
 * Convert the value to string.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] s Return the string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_string (OX_Context *ctxt, OX_Value *v, OX_Value *s)
{
    OX_Result r;

    if (ox_value_is_string(ctxt, v)) {
        ox_value_copy(ctxt, s, v);
        r = OX_OK;
    } else {
        r = ox_to_string_inner(ctxt, v, s);
    }

    return r;
}

/**
 * Convert the value to size_t.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the size_t value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_size (OX_Context *ctxt, OX_Value *v, size_t *pi);

/**
 * Convert the value to ssize_t.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the ssize_t value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_ssize (OX_Context *ctxt, OX_Value *v, ssize_t *pi);

/**
 * Convert the value to index.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the index value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_index (OX_Context *ctxt, OX_Value *v, size_t *pi)
{
    return ox_to_size(ctxt, v, pi);
}

/**
 * Convert the value to 64 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 64 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_int64 (OX_Context *ctxt, OX_Value *v, int64_t *pi);

/**
 * Convert the value to 64 bits unsigned integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 64 bits unsigned integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_to_uint64 (OX_Context *ctxt, OX_Value *v, uint64_t *pi);

/**
 * Convert the value to 32 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 32 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_int32 (OX_Context *ctxt, OX_Value *v, int32_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < (double)INT32_MIN) || (n > (double)INT32_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to unsigned 32 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the unsigned 32 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_uint32 (OX_Context *ctxt, OX_Value *v, uint32_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < 0) || (n > (double)UINT32_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to 16 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 16 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_int16 (OX_Context *ctxt, OX_Value *v, int16_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < (double)INT16_MIN) || (n > (double)INT16_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to unsigned 16 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the unsigned 16 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_uint16 (OX_Context *ctxt, OX_Value *v, uint16_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < 0) || (n > (double)UINT16_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to 8 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 8 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_int8 (OX_Context *ctxt, OX_Value *v, int8_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < (double)INT8_MIN) || (n > (double)INT8_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to unsigned 8 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the unsigned 16 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_to_uint8 (OX_Context *ctxt, OX_Value *v, uint8_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < 0) || (n > (double)UINT8_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Call the method of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The method property's name.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param rv The return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_call_method (OX_Context *ctxt, OX_Value *o, OX_Value *p,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, f, s)
    OX_Result r;

    r = ox_get(ctxt, o, p, f);
    if (r == OX_OK) {
        if (ox_value_is_null(ctxt, f)) {
            if (ox_to_string(ctxt, p, s) == OX_OK) {
                ox_throw_null_error(ctxt,
                        OX_TEXT("value of property \"%s\" is null"),
                        ox_string_get_char_star(ctxt, s));
            }

            r = OX_ERR;
        } else {
            r = ox_call(ctxt, f, o, args, argc, rv);
        }
    }

    OX_VS_POP(ctxt, f)
    return r;
}

/**
 * Call the method of an object with a C string method name.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The method property's name.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param rv The return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_call_method_s (OX_Context *ctxt, OX_Value *o, const char *p,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_call_method(ctxt, o, pv, args, argc, rv);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Call the method of an object if the method exist.
 * @param o The object value.
 * @param p The method property's name.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param rv The return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_try_call_method (OX_Context *ctxt, OX_Value *o, OX_Value *p,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, f)
    OX_Result r;

    r = ox_get(ctxt, o, p, f);
    if (r == OX_OK) {
        if (!ox_value_is_null(ctxt, f))
            r = ox_call(ctxt, f, o, args, argc, rv);
        else
            ox_value_set_null(ctxt, rv);
    }

    OX_VS_POP(ctxt, f)
    return r;
}

/**
 * Call the method of an object if the method exist with a C string method name.
 * @param o The object value.
 * @param p The method property's name.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param rv The return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_try_call_method_s (OX_Context *ctxt, OX_Value *o, const char *p,
        OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, pv)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, pv, p)) == OX_OK)
        r = ox_try_call_method(ctxt, o, pv, args, argc, rv);

    OX_VS_POP(ctxt, pv)
    return r;
}

/**
 * Check if 2 values are equal.
 * @param ctxt The current running context.
 * @param v1 Value 1.
 * @param v2 Value 2.
 * @retval OX_TRUE v1 == v2.
 * @retval OX_FALSE v1 != v2;
 */
extern OX_Bool
ox_equal (OX_Context *ctxt, OX_Value *v1, OX_Value *v2);

/**
 * Invoke "$close" method to close the value.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_close (OX_Context *ctxt, OX_Value *v);

/**
 * Get the class of the value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param[out] c Return the class of the object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_type_of (OX_Context *ctxt, OX_Value *o, OX_Value *c);

/**
 * Check if the value is an instance of the class.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param c The class value.
 * @retval OX_TRUE o is an instance of c.
 * @retval OX_FALSE o is not an instance of c.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_instance_of (OX_Context *ctxt, OX_Value *o, OX_Value *c);

/**
 * Get the value's full name.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] s Return the full name.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_get_full_name (OX_Context *ctxt, OX_Value *v, OX_Value *s);

#ifdef __cplusplus
}
#endif

#endif
