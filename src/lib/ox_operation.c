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
 * Operations.
 */

#define OX_LOG_TAG "ox_operation"

#include "ox_internal.h"

/**
 * Convert the value to number inner function.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pn Return the number value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_to_number_inner (OX_Context *ctxt, OX_Value *v, OX_Number *pn)
{
    OX_VS_PUSH_2(ctxt, n, f)
    OX_Result r;

    if ((r = ox_get(ctxt, v, OX_STRING(ctxt, _to_num), f)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, f)) {
        if ((r = ox_call(ctxt, f, v, NULL, 0, n)) == OX_ERR)
            goto end;

        if (!ox_value_is_number(ctxt, n)) {
            r = ox_throw_type_error(ctxt, OX_TEXT("result of \"$to_num\" must be a number"));
            goto end;
        }

        *pn = ox_value_get_number(ctxt, n);
    } else {
        *pn = NAN;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, n)
    return r;
}

/*Default to string function.*/
static OX_Result
to_string_default (OX_Context *ctxt, OX_Value *v, OX_Value *s)
{
    OX_VS_PUSH(ctxt, n)
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_char_buffer_append_char_star(ctxt, &cb, "Object")) == OX_ERR)
        goto end;

    if ((r = ox_get_full_name(ctxt, v, n)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, n) && ox_string_length(ctxt, n)) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, ':')) == OX_ERR)
            goto end;
        if ((r = ox_char_buffer_append_string(ctxt, &cb, n)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, s);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, n)
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
OX_Result
ox_to_string_inner (OX_Context *ctxt, OX_Value *v, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, f, t)
    OX_Result r;

    if (ox_value_is_null(ctxt, v)) {
        ox_value_copy(ctxt, s, OX_STRING(ctxt, empty));
    } else {
        if ((r = ox_get(ctxt, v, OX_STRING(ctxt, _to_str), f)) == OX_ERR)
            goto end;

        if (!ox_value_is_null(ctxt, f)) {
            if ((r = ox_call(ctxt, f, v, NULL, 0, s)) == OX_ERR)
                goto end;

            if (!ox_value_is_string(ctxt, s)) {
                r = ox_throw_type_error(ctxt, OX_TEXT("result of \"$to_str\" must be a string"));
                goto end;
            }
        } else {
            if ((r = ox_get(ctxt, v, OX_STRING(ctxt, _to_num), f)) == OX_ERR)
                goto end;

            if (!ox_value_is_null(ctxt, f)) {
                if ((r = ox_call(ctxt, f, v, NULL, 0, t)) == OX_ERR)
                    goto end;

                if (!ox_value_is_number(ctxt, t)) {
                    r = ox_throw_type_error(ctxt, OX_TEXT("result of \"$to_num\" must be a number"));
                    goto end;
                }

                if ((r = ox_to_string(ctxt, t, s)) == OX_ERR)
                    goto end;
            } else {
                if ((r = to_string_default(ctxt, v, s)) == OX_ERR)
                    goto end;
            }
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, f)
    return r;
}

/**
 * Convert the value to string with expected format.
 * @param ctxt The current running context.add_cmd
 * @param v The value.
 * @param[out] s Return the string.
 * @param flags Format flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_to_string_format (OX_Context *ctxt, OX_Value *v, OX_Value *s, int flags)
{
    OX_CharBuffer cb;
    int fmt, width, kind;
    OX_Result r;

    ox_char_buffer_init(&cb);

    fmt = OX_SOUT_GET_FMT(flags);
    width = OX_SOUT_GET_WIDTH(flags);

    if (fmt == OX_SOUT_FMT_STR) {
        /*String.*/
        size_t len;

        if ((r = ox_to_string(ctxt, v, s)) == OX_ERR)
            goto end;

        if ((width != OX_SOUT_WIDTH_DEFAULT) && ((len = ox_string_length(ctxt, s)) < width)) {
            int plen = width - len;

            if (flags & OX_SOUT_FL_ALIGN_HEAD) {
                if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
                    goto end;
                if ((r = ox_char_buffer_append_char_n(ctxt, &cb, ' ', plen)) == OX_ERR)
                    goto end;
            } else {
                if ((r = ox_char_buffer_append_char_n(ctxt, &cb, ' ', plen)) == OX_ERR)
                    goto end;
                if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
                    goto end;
            }

            if ((r = ox_char_buffer_get_string(ctxt, &cb, s)) == OX_ERR)
                goto end;
        }
    } else if (fmt == OX_SOUT_FMT_CHAR) {
        /*Character.*/
        char buf[5];
        uint32_t uc = 0;
        int len;

        if ((r = ox_to_uint32(ctxt, v, &uc)) == OX_ERR)
            goto end;

        len = ox_uc_to_utf8(uc, buf);
        if (len == -1) {
            r = ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));
            goto end;
        }

        if ((width != OX_SOUT_WIDTH_DEFAULT) && (len < width)) {
            int plen = width - len;

            if (flags & OX_SOUT_FL_ALIGN_HEAD) {
                if ((r = ox_char_buffer_append_chars(ctxt, &cb, buf, len)) == OX_ERR)
                    goto end;
                if ((r = ox_char_buffer_append_char_n(ctxt, &cb, ' ', plen)) == OX_ERR)
                    goto end;
            } else {
                if ((r = ox_char_buffer_append_char_n(ctxt, &cb, ' ', plen)) == OX_ERR)
                    goto end;
                if ((r = ox_char_buffer_append_chars(ctxt, &cb, buf, len)) == OX_ERR)
                    goto end;
            }

            if ((r = ox_char_buffer_get_string(ctxt, &cb, s)) == OX_ERR)
                goto end;
        } else {
            if ((r = ox_string_from_chars(ctxt, s, buf, len)) == OX_ERR)
                goto end;
        }
    } else if ((kind = ox_value_is_int64(ctxt, v))) {
        int64_t i = ox_value_get_int64(ctxt, v);

        if ((r = ox_int_to_string(ctxt, kind, i, s, flags)) == OX_ERR)
            goto end;
    } else {
        /*Number.*/
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            goto end;

        if ((r = ox_number_to_string(ctxt, n, s, flags)) == OX_ERR)
            goto end;
    }
end:
    ox_char_buffer_deinit(ctxt, &cb);
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
OX_Result
ox_to_size (OX_Context *ctxt, OX_Value *v, size_t *pi)
{
    OX_Number n;
    OX_Result r;

#if __SIZEOF_POINTER__ == 8
    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_uint64(ctxt, v);
        return OX_OK;
    }
#endif /*__SIZEOF_POINTER__ == 8*/

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < 0) || (n > (double)SIZE_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to ssize_t.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the ssize_t value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_to_ssize (OX_Context *ctxt, OX_Value *v, ssize_t *pi)
{
    OX_Number n;
    OX_Result r;

#if __SIZEOF_POINTER__ == 8
    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_int64(ctxt, v);
        return OX_OK;
    }
#endif /*__SIZEOF_POINTER__ == 8*/

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

#if __SIZEOF_POINTER__ == 8
    if ((n < (double)INT64_MIN) || (n > (double)INT64_MAX))
#elif __SIZEOF_POINTER__  == 4
    if ((n < (double)INT32_MIN) || (n > (double)INT32_MAX))
#else
    #error illegal pointer size
#endif
    {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/**
 * Convert the value to 64 bits integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 64 bits integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_to_int64 (OX_Context *ctxt, OX_Value *v, int64_t *pi)
{
    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_int64(ctxt, v);
    } else {
        OX_Number n;
        OX_Result r;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        if ((n < (double)INT64_MIN) || (n > (double)INT64_MAX))
            return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));

        *pi = n;
    }

    return OX_OK;
}

/**
 * Convert the value to 64 bits unsigned integer number.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] pi Return the 64 bits unsigned integer number.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_to_uint64 (OX_Context *ctxt, OX_Value *v, uint64_t *pi)
{
    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_uint64(ctxt, v);
    } else {
        OX_Number n;
        OX_Result r;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        if ((n < 0) || (n > (double)UINT64_MAX))
            return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));

        *pi = n;
    }

    return OX_OK;
}

/*Check if a string equal to a C string.*/
static OX_Bool
string_equal_to_c_string (OX_Context *ctxt, OX_Value *s, OX_Value *cs)
{
    OX_Value *st = ox_value_stack_push(ctxt);
    OX_CValue *cv = ox_value_get_gco(ctxt, cs);
    const char *p;
    OX_Bool b;

    p = cv->v.p;
    if (!p) {
        b = OX_FALSE;
    } else {
        if (ox_string_from_char_star(ctxt, st, p) == OX_ERR)
            b = OX_FALSE;
        else
            b = ox_equal(ctxt, s, st);
    }

    ox_value_stack_pop(ctxt, st);
    return b;
}

/*Equal check for strings.*/
static OX_Result
string_equal (OX_Context *ctxt, OX_Value *v1, OX_Value *v2)
{
    if (ox_value_is_string(ctxt, v1)) {
        if (ox_value_is_string(ctxt, v2))
            return ox_string_equal(ctxt, v1, v2);

        if (ox_value_is_c_string(ctxt, v2))
            return string_equal_to_c_string(ctxt, v1, v2);
    } else if (ox_value_is_string(ctxt, v2)) {
        if (ox_value_is_c_string(ctxt, v1))
            return string_equal_to_c_string(ctxt, v2, v1);
    }

    return OX_ERR;
}

/*Check if a number equal to a C number.*/
static OX_Bool
number_equal_to_c_number (OX_Context *ctxt, OX_Value *v1, OX_Value *v2)
{
    OX_Number n1, n2;
    OX_Result r;

    n1 = ox_value_get_number(ctxt, v1);

    r = ox_to_number(ctxt, v2, &n2);
    if (r == OX_ERR)
        return OX_FALSE;

    return n1 == n2;
}

/*Check if 2 C numbers are equal.*/
static OX_Result
c_number_equal (OX_Context *ctxt, OX_CTypeKind k, OX_Value *v1, OX_Value *v2)
{
    OX_Result r;

    switch (k) {
    case OX_CTYPE_I64: {
        int64_t i1, i2;

        if ((r = ox_to_int64(ctxt, v1, &i1)) == OX_ERR)
            return OX_FALSE;

        if ((r = ox_to_int64(ctxt, v2, &i2)) == OX_ERR)
            return OX_FALSE;

        return i1 == i2;
    }
    case OX_CTYPE_U64: {
        uint64_t i1, i2;

        if ((r = ox_to_uint64(ctxt, v1, &i1)) == OX_ERR)
            return OX_FALSE;

        if ((r = ox_to_uint64(ctxt, v2, &i2)) == OX_ERR)
            return OX_FALSE;

        return i1 == i2;
    }
    default: {
        OX_Number n1, n2;

        if ((r = ox_to_number(ctxt, v1, &n1)) == OX_ERR)
            return OX_FALSE;

        if ((r = ox_to_number(ctxt, v2, &n2)) == OX_ERR)
            return OX_FALSE;

        return n1 == n2;
    }
    }
}

/*Equal check for numbers.*/
static OX_Result
number_equal (OX_Context *ctxt, OX_Value *v1, OX_Value *v2)
{
    OX_CTypeKind k1, k2;

    if (ox_value_is_number(ctxt, v1)) {
        if (ox_value_is_number(ctxt, v2))
            return ox_value_get_number(ctxt, v1) == ox_value_get_number(ctxt, v2);

        if (ox_value_is_c_number(ctxt, v2))
            return number_equal_to_c_number(ctxt, v1, v2);
    } else if (ox_value_is_number(ctxt, v2)) {
        if (ox_value_is_c_number(ctxt, v1))
            return number_equal_to_c_number(ctxt, v2, v1);
    } else if ((k1 = ox_value_is_c_number(ctxt, v1))
            && (k2 = ox_value_is_c_number(ctxt, v2))) {
        return c_number_equal(ctxt, OX_MAX(k1, k2), v1, v2);
    }

    return -1;
}

/**
 * Check if 2 values are equal.
 * @param ctxt The current running context.
 * @param v1 Value 1.
 * @param v2 Value 2.
 * @retval OX_TRUE v1 == v2.
 * @retval OX_FALSE v1 != v2;
 */
OX_Bool
ox_equal (OX_Context *ctxt, OX_Value *v1, OX_Value *v2)
{
    OX_ValueType t1, t2;
    OX_Result r;
    OX_Bool b;

    assert(ctxt && v1 && v2);

    if ((r = string_equal(ctxt, v1, v2)) != OX_ERR)
        return r;

    if ((r = number_equal(ctxt, v1, v2)) != OX_ERR)
        return r;

    t1 = ox_value_get_type(ctxt, v1);
    t2 = ox_value_get_type(ctxt, v2);

    if (t1 != t2)
        return OX_FALSE;

    switch (t1) {
    case OX_VALUE_NULL:
        b = OX_TRUE;
        break;
    case OX_VALUE_BOOL:
        b = (ox_value_get_bool(ctxt, v1) == ox_value_get_bool(ctxt, v2));
        break;
    case OX_VALUE_GCO:
        if (ox_value_is_cvalue(ctxt, v1) && ox_value_is_cvalue(ctxt, v2)) {
            OX_CValue *cv1 = ox_value_get_gco(ctxt, v1);
            OX_CValue *cv2 = ox_value_get_gco(ctxt, v2);

            if (cv1 == cv2) {
                b = OX_TRUE;
            } else {
                OX_Value *t1, *t2;

                t1 = ox_cvalue_get_ctype(ctxt, v1);
                t2 = ox_cvalue_get_ctype(ctxt, v2);

                if (!ox_ctype_equal(ctxt, t1, t2)) {
                    b = OX_FALSE;
                } else {
                    assert(ox_ctype_get_kind(ctxt, t1) & OX_CTYPE_FL_PTR);

                    b = ox_cvalue_get_pointer(ctxt, v1) == ox_cvalue_get_pointer(ctxt, v2);
                }
            }
        } else {
            b = (ox_value_get_gco(ctxt, v1) == ox_value_get_gco(ctxt, v2));
        }
        break;
    default:
        assert(0);
        b = OX_FALSE;
        break;
    }

    return b;
}

/**
 * Invoke "$close" method to close the value.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_close (OX_Context *ctxt, OX_Value *v)
{
    OX_VS_PUSH(ctxt, rv)
    OX_Result r;

    assert(ctxt && v);

    if (ox_value_is_null(ctxt, v))
        r = OX_OK;
    else
        r = ox_try_call_method(ctxt, v, OX_STRING(ctxt, _close), NULL, 0, rv);

    OX_VS_POP(ctxt, rv)
    return r;
}

/*Check if the C value is an instance of the C type.*/
static OX_Result
cvalue_instance_of (OX_Context *ctxt, OX_Value *v, OX_Value *et)
{
    OX_Value *t;

    t = ox_cvalue_get_ctype(ctxt, v);

    if (ox_value_is_struct_cptr(ctxt, v)) {
        OX_CPtrType *pt = ox_value_get_gco(ctxt, t);

        t = &pt->vty;
    }

    return ox_equal(ctxt, t, et);
}

/*Check if the object is an instance of the class.*/
static OX_Result
object_instance_of (OX_Context *ctxt, OX_Value *o, OX_Value *c)
{
    OX_VS_PUSH_2(ctxt, inf, cinf)
    OX_Result r;

    if ((r = ox_object_get_interface(ctxt, o, inf)) == OX_ERR)
        goto end;

    if ((r = ox_get(ctxt, c, OX_STRING(ctxt, _inf), cinf)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, inf) || ox_value_is_null(ctxt, cinf))
        r = OX_FALSE;
    else
        r = ox_interface_impl(ctxt, inf, cinf);
end:
    OX_VS_POP(ctxt, cinf)
    return r;
}

/**
 * Get the class of the value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param[out] c Return the class of the object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_type_of (OX_Context *ctxt, OX_Value *o, OX_Value *c)
{
    OX_Result r = OX_OK;

    switch (ox_value_get_type(ctxt, o)) {
    case OX_VALUE_NULL:
        ox_value_set_null(ctxt, c);
        break;
    case OX_VALUE_BOOL:
        ox_value_copy(ctxt, c, OX_OBJECT(ctxt, Bool));
        break;
    case OX_VALUE_NUMBER:
        ox_value_copy(ctxt, c, OX_OBJECT(ctxt, Number));
        break;
    default:
        if (ox_value_is_string(ctxt, o)) {
            ox_value_copy(ctxt, c, OX_OBJECT(ctxt, String));
        } else if (ox_value_is_struct_cptr(ctxt, o)) {
            OX_Value *t = ox_cvalue_get_ctype(ctxt, o);
            OX_CPtrType *pt = ox_value_get_gco(ctxt, t);

            ox_value_copy(ctxt, c, &pt->vty);
        } else if (ox_value_is_cvalue(ctxt, o)) {
            OX_Value *t = ox_cvalue_get_ctype(ctxt, o);

            ox_value_copy(ctxt, c, t);
        } else if (ox_value_is_object(ctxt, o)) {
            OX_Object *op = ox_value_get_gco(ctxt, o);

            if (ox_value_is_null(ctxt, &op->inf))
                ox_value_set_null(ctxt, OX_OBJECT(ctxt, Object));
            else
                r = ox_get(ctxt, &op->inf, OX_STRING(ctxt, _class), c);
        } else {
            ox_value_set_null(ctxt, c);
        }
        break;
    }

    return r;
}

/**
 * Check if the value is an instance of the class.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param c The class value.
 * @retval OX_TRUE o is an instance of c.
 * @retval OX_FALSE o is not an instance of c.
 * @retval OX_ERR On error.
 */
OX_Result
ox_instance_of (OX_Context *ctxt, OX_Value *o, OX_Value *c)
{
    if (!ox_value_is_object(ctxt, c))
        return OX_FALSE;

    switch (ox_value_get_type(ctxt, o)) {
    case OX_VALUE_NULL:
        return OX_FALSE;
    case OX_VALUE_BOOL:
        return ox_equal(ctxt, c, OX_OBJECT(ctxt, Bool));
    case OX_VALUE_NUMBER:
        return ox_equal(ctxt, c, OX_OBJECT(ctxt, Number));
    default:
        if (ox_value_is_string(ctxt, o))
            return ox_equal(ctxt, c, OX_OBJECT(ctxt, String));
        if (ox_value_is_cvalue(ctxt, o))
            return cvalue_instance_of(ctxt, o, c);
        if (ox_value_is_object(ctxt, o))
            return object_instance_of(ctxt, o, c);
    }

    return OX_FALSE;
}

/** Get full name stack entry.*/
typedef struct OX_GetFullNameStack_s OX_GetFullNameStack;
/** Get full name stack entry.*/
struct OX_GetFullNameStack_s {
    OX_Value *o; /**< The current object.*/
    OX_GetFullNameStack *bot; /**< The bottom stack entry.*/
};

/*Get the value's full name.*/
static OX_Result
get_full_name (OX_Context *ctxt, OX_CharBuffer *cb, OX_Value *v, OX_GetFullNameStack *stack)
{
    OX_VS_PUSH_3(ctxt, scope, name, str)
    OX_GetFullNameStack *se, cse;
    OX_Result r;

    for (se = stack; se; se = se->bot) {
        if (ox_equal(ctxt, se->o, v)) {
            r = ox_throw_reference_error(ctxt, OX_TEXT("circular reference"));
            goto end;
        }
    }

    cse.bot = stack;
    cse.o = v;

    if ((r = ox_lookup(ctxt, v, OX_STRING(ctxt, _scope), scope)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, scope)) {
        if ((r = get_full_name(ctxt, cb, scope, &cse)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char(ctxt, cb, '.')) == OX_ERR)
            goto end;
    }

    if ((r = ox_lookup(ctxt, v, OX_STRING(ctxt, _name), name)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, name, str)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_string(ctxt, cb, str)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, scope)
    return r;
}

/**
 * Get the value's full name.
 * @param ctxt The current running context.
 * @param v The value.
 * @param[out] s Return the full name.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_get_full_name (OX_Context *ctxt, OX_Value *v, OX_Value *s)
{
    OX_CharBuffer cb;
    OX_Result r;

    assert(ctxt && v && s);

    ox_char_buffer_init(&cb);

    if ((r = get_full_name(ctxt, &cb, v, NULL)) == OX_ERR)
        goto end;

    r = ox_char_buffer_get_string(ctxt, &cb, s);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

/**
 * Throw the property is null error.
 * @param ctxt The current running context.
 * @param p The property key.
 * @retval OX_ERR
 */
OX_Result
ox_throw_null_property_error (OX_Context *ctxt, OX_Value *p)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    r = ox_to_string(ctxt, p, s);
    if (r == OX_OK)
        r = ox_throw_null_error(ctxt,
                OX_TEXT("the property \"%s\" does not exist"),
                ox_string_get_char_star(ctxt, s));

    OX_VS_POP(ctxt, s)

    return OX_ERR;
}
