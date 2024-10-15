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
 * Number value.
 */

#define OX_LOG_TAG "ox_number"

#include "ox_internal.h"

/*Get the owned keys in the number.*/
static OX_Result
number_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    ox_value_set_null(ctxt, keys);
    return OX_OK;
}

/*Lookup the owned property in the number.*/
static OX_Result
number_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    ox_value_set_null(ctxt, v);
    return OX_OK;
}

/*Get property value of a number value.*/
static OX_Result
number_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_object_get_t(ctxt, OX_OBJECT(ctxt, Number_inf), p, v, o);
}

/*Set property value of a number value.*/
static OX_Result
number_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_type_error(ctxt, OX_TEXT("cannot set the property of a number"));
}

/*Delete the owned property of the number.*/
static OX_Result
number_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    return OX_OK;
}

/*Call the number.*/
static OX_Result
number_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *r)
{
    ox_value_copy(ctxt, r, o);
    return OX_OK;
}

/*Operation functions of the number value.*/
const OX_ObjectOps
ox_number_ops = {
    {},
    number_keys,
    number_lookup,
    number_get,
    number_set,
    number_del,
    number_call
};

/*Convert the integer number to string.*/
static OX_Result
int_to_string (OX_Context *ctxt, long long i, OX_Value *s, int fmt, int flags)
{
    char mode[10];
    char *c = mode;
    int width = OX_SOUT_GET_WIDTH(flags);
    char buf[300];

    if (width == 0xff)
        width = -1;

    *c ++ = '%';
    if (flags & OX_SOUT_FL_ALIGN_HEAD)
        *c ++ = '-';
    if (flags & OX_SOUT_FL_ZERO)
        *c ++ = '0';
    *c ++ = '*';
    *c ++ = 'l';
    *c ++ = 'l';
    *c ++ = fmt;
    *c = 0;

    snprintf(buf, sizeof(buf), mode, width, i);

    return ox_string_from_char_star(ctxt, s, buf);
}

/*Convert the float point number to string.*/
static OX_Result
float_to_string (OX_Context *ctxt, OX_Number n, OX_Value *s, int fmt, int flags)
{
    char mode[8];
    char *c = mode;
    int width = OX_SOUT_GET_WIDTH(flags);
    int prec = OX_SOUT_GET_PREC(flags);
    char buf[300];

    if (width == 0xff)
        width = -1;
    if (prec == 0xff)
        prec = -1;

    *c ++ = '%';
    if (flags & OX_SOUT_FL_ALIGN_HEAD)
        *c ++ = '-';
    if (flags & OX_SOUT_FL_ZERO)
        *c ++ = '0';
    *c ++ = '*';
    *c ++ = '.';
    *c ++ = '*';
    *c ++ = fmt;
    *c = 0;

    snprintf(buf, sizeof(buf), mode, width, prec, n);

    return ox_string_from_char_star(ctxt, s, buf);
}

/**
 * Convert the number to string.
 * @param ctxt The current running context.
 * @param n The number.
 * @param[out] s Return result string.
 * @param flags Convert flags. (OX_SOUT_XXXX)
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_number_to_string (OX_Context *ctxt, OX_Number n, OX_Value *s, int flags)
{
    int fmt = OX_SOUT_GET_FMT(flags);
    long long i;
    OX_Result r;

    switch (fmt) {
    case OX_SOUT_FMT_OCT:
        i = (unsigned long long)n;
        r = int_to_string(ctxt, i, s, 'o', flags);
        break;
    case OX_SOUT_FMT_DEC:
        i = (long long)n;
        r = int_to_string(ctxt, i, s, 'd', flags);
        break;
    case OX_SOUT_FMT_UDEC:
        i = (unsigned long long)n;
        r = int_to_string(ctxt, i, s, 'u', flags);
        break;
    case OX_SOUT_FMT_HEX:
        i = (unsigned long long)n;
        r = int_to_string(ctxt, i, s, 'x', flags);
        break;
    case OX_SOUT_FMT_FLOAT:
        r = float_to_string(ctxt, n, s, 'f', flags);
        break;
    case OX_SOUT_FMT_EXP:
        r = float_to_string(ctxt, n, s, 'e', flags);
        break;
    case OX_SOUT_FMT_NUMBER:
        if (n == (unsigned long long )n)
            r = int_to_string(ctxt, n, s, 'u', flags);
        else if (n == (long long)n)
            r = int_to_string(ctxt, n, s, 'd', flags);
        else
            r = float_to_string(ctxt, n, s, 'g', flags);
        break;
    default:
        assert(0);
    }

    return r;
}

/**
 * Convert the integer number to string.
 * @param ctxt The current running context.
 * @param kind Kind of the integer.
 * @param i The integer number.
 * @param[out] s Return result string.
 * @param flags Convert flags. (OX_SOUT_XXXX)
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_int_to_string (OX_Context *ctxt, OX_CTypeKind kind, int64_t i, OX_Value *s, int flags)
{
    int fmt = OX_SOUT_GET_FMT(flags);
    OX_Result r;

    switch (fmt) {
    case OX_SOUT_FMT_OCT:
        r = int_to_string(ctxt, i, s, 'o', flags);
        break;
    case OX_SOUT_FMT_DEC:
        r = int_to_string(ctxt, i, s, 'd', flags);
        break;
    case OX_SOUT_FMT_UDEC:
        r = int_to_string(ctxt, i, s, 'u', flags);
        break;
    case OX_SOUT_FMT_HEX:
        r = int_to_string(ctxt, i, s, 'x', flags);
        break;
    case OX_SOUT_FMT_FLOAT:
        r = float_to_string(ctxt, (double)i, s, 'f', flags);
        break;
    case OX_SOUT_FMT_EXP:
        r = float_to_string(ctxt, (double)i, s, 'e', flags);
        break;
    case OX_SOUT_FMT_NUMBER:
        if (kind == OX_CTYPE_U64)
            r = int_to_string(ctxt, i, s, 'u', flags);
        else
            r = int_to_string(ctxt, i, s, 'd', flags);
        break;
    default:
        assert(0);
    }

    return r;
}

/*Number.is*/
static OX_Result
Number_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_number(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Number.$inf.$to_str*/
static OX_Result
Number_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *prec = ox_argument(ctxt, args, argc, 0);
    int flags, pr = OX_SOUT_PREC_DEFAULT;
    OX_Number n;
    OX_Result r;

    if (!ox_value_is_null(ctxt, prec)) {
        uint32_t prec_i;

        if ((r = ox_to_uint32(ctxt, prec, &prec_i)) == OX_ERR)
            return r;

        pr = prec_i;
    }

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    flags = OX_SOUT_FLAGS_MAKE(0, OX_SOUT_WIDTH_DEFAULT, pr, OX_SOUT_FMT_NUMBER);

    return ox_number_to_string(ctxt, n, rv, flags);
}

/*Number.$inf.e_str*/
static OX_Result
Number_inf_e_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *prec = ox_argument(ctxt, args, argc, 0);
    int flags, pr = OX_SOUT_PREC_DEFAULT;
    OX_Number n;
    OX_Result r;

    if (!ox_value_is_null(ctxt, prec)) {
        uint32_t prec_i;

        if ((r = ox_to_uint32(ctxt, prec, &prec_i)) == OX_ERR)
            return r;

        pr = prec_i;
    }

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    flags = OX_SOUT_FLAGS_MAKE(0, OX_SOUT_WIDTH_DEFAULT, pr, OX_SOUT_FMT_EXP);

    return ox_number_to_string(ctxt, n, rv, flags);
}

/*Number.$inf.f_str*/
static OX_Result
Number_inf_f_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *prec = ox_argument(ctxt, args, argc, 0);
    int flags, pr = OX_SOUT_PREC_DEFAULT;
    OX_Number n;
    OX_Result r;

    if (!ox_value_is_null(ctxt, prec)) {
        uint32_t prec_i;

        if ((r = ox_to_uint32(ctxt, prec, &prec_i)) == OX_ERR)
            return r;

        pr = prec_i;
    }

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    flags = OX_SOUT_FLAGS_MAKE(0, OX_SOUT_WIDTH_DEFAULT, pr, OX_SOUT_FMT_FLOAT);

    return ox_number_to_string(ctxt, n, rv, flags);
}

/*Number.$inf.i_str*/
static OX_Result
Number_inf_i_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *base = ox_argument(ctxt, args, argc, 0);
    int base_i = 10;
    long long i;
    OX_Number base_n, n;
    OX_Result r;
    char buf[256];
    char *c;
    OX_Bool rev = OX_FALSE;

    if (!ox_value_is_null(ctxt, base)) {
        if ((r = ox_to_number(ctxt, base, &base_n)) == OX_ERR)
            return r;

        base_i = base_n;

        if ((base_i < 2) || (base_i > 36)) {
            return ox_throw_range_error(ctxt, OX_TEXT("\"base\" must be in 2 ~ 36"));
        }
    }

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    i = n;

    if (i < 0) {
        i = -i;
        rev = OX_TRUE;
    }

    c = buf + sizeof(buf);
    *--c = 0;

    while (1) {
        int v = i % base_i;

        if (v < 10)
            *--c = v + '0';
        else
            *--c = v - 10 + 'a';

        i /= base_i;
        if (i == 0)
            break;
    }

    if (rev)
        *--c = '-';

    return ox_string_from_char_star(ctxt, rv, c);
}

/*Number.$inf.floor*/
static OX_Result
Number_inf_floor (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    n = floor(n);
    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}

/*Number.$inf.ceil*/
static OX_Result
Number_inf_ceil (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    n = ceil(n);
    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}

/*Number.$inf.round*/
static OX_Result
Number_inf_round (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    n = round(n);
    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}

/*Number.$inf.trunc*/
static OX_Result
Number_inf_trunc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    n = trunc(n);
    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}

/*Number.$inf.isinf*/
static OX_Result
Number_inf_isinf (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, isinf(n));
    return OX_OK;
}

/*Number.$inf.isnan*/
static OX_Result
Number_inf_isnan (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, rv, isnan(n));
    return OX_OK;
}

/*Number.$inf.to_int8*/
static OX_Result
Number_inf_to_int8 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.i8 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_I8), &cvi);
}

/*Number.$inf.to_int16*/
static OX_Result
Number_inf_to_int16 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.i16 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_I16), &cvi);
}

/*Number.$inf.to_int32*/
static OX_Result
Number_inf_to_int32 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.i32 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_I32), &cvi);
}

/*Number.$inf.to_int64*/
static OX_Result
Number_inf_to_int64 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.i64 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_I64), &cvi);
}

/*Number.$inf.to_uint8*/
static OX_Result
Number_inf_to_uint8 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.u8 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_U8), &cvi);
}

/*Number.$inf.to_uint16*/
static OX_Result
Number_inf_to_uint16 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.u16 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_U16), &cvi);
}

/*Number.$inf.to_uint32*/
static OX_Result
Number_inf_to_uint32 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.u32 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_U32), &cvi);
}

/*Number.$inf.to_uint64*/
static OX_Result
Number_inf_to_uint64 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.u64 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_U64), &cvi);
}

/*Number.$inf.to_float32*/
static OX_Result
Number_inf_to_float32 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.f32 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_F32), &cvi);
}

/*Number.$inf.to_float64*/
static OX_Result
Number_inf_to_float64 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Number n;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
        return r;

    cvi.v.f64 = n;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, rv, ox_ctype_get(ctxt, OX_CTYPE_F64), &cvi);
}

/*?
 *? @lib {Number} Number.
 *?
 *? @class{ Number Number class.
 *?
 *? @const NAN {Number} Not a number value.
 *? @const INFINITY {Number} Infinity value.
 *?
 *? @func $to_str Convert the number to string.
 *? @param prec {Number} Precision.
 *? @return {String} The string.
 *?
 *? @func e_str Convert the number to string.
 *? The string's format is [-]d.ddde±dd. Where there is one digit
 *? (which is nonzero if the argument is nonzero) before the decimal-point
 *? character and the number of digits after it is equal to the precision.
 *? @param prec {Number} =6 Precision.
 *? @return {String} The string.
 *?
 *? @func f_str Convert the number to string.
 *? The string's format is [-]ddd.dddd. Where the number of digits after
 *? the decimal-point character is equal to the precision specification.
 *? @param prec {Number} =6 Precision.
 *? @return {String} The string.
 *?
 *? @func i_str Convert the integer number to string.
 *? The number will be converted to integer
 *? @param base {Number} Base of the number.
 *? The value of base should be an integer and it value should > 2 and < 36.
 *? @return {String} The string.
 *?
 *? @func floor Get the largest integral value not greater than the number.
 *? The function returns the largest integral value that is not greater than the number.
 *? For example, floor(0.5) is 0.0, and floor(-0.5) is -1.0.
 *? @return {Number} The function returns the floor of the number.
 *? If the number is integral, +0, -0, NaN, or an infinity, itself is returned.
 *?
 *? @func ceil Get the smallest integral value not less than the number.
 *? The function returns the smallest integral value that is not less than the number.
 *? For example, ceil(0.5) is 1.0, and ceil(-0.5) is 0.0.
 *? @return {Number} The function returns the ceiling of the number.
 *? If the number is integral, +0, -0, NaN, or infinite, itself is returned.
 *?
 *? @func round Round to nearest integer, away from zero.
 *? The function rounds the number to the nearest integer, but round halfway cases away from zero,
 *? For example, round(0.5) is 1.0, and round(-0.5) is -1.0.
 *? @return {Number} The function returns the rounded integer value.
 *? If the number is integral, +0, -0, NaN, or infinite, itself is returned.
 *?
 *? @func trunc Round to integer, toward zero.
 *? The function rounds the number to the nearest integer value that is not larger in magnitude than the number.
 *? @return {Number} The function returns the rounded integer value, in floating format.
 *? If the number is integral, infinite, or NaN, itself is returned.
 *?
 *? @func isnan Check if the number is NAN (not a number).
 *? @return {Bool} The number is NAN or not.
 *?
 *? @func isinf Check if the number is infinity.
 *? @return {Bool} The number is infinity or not.
 *?
 *? @func to_int8 Convert the number to 8 bits integer value.
 *? @return {C:Int8} The 8 bits integer value.
 *?
 *? @func to_int16 Convert the number to 16 bits integer value.
 *? @return {C:Int16} The 16 bits integer value.
 *?
 *? @func to_int32 Convert the number to 32 bits integer value.
 *? @return {C:Int32} The 8 bits integer value.
 *?
 *? @func to_int64 Convert the number to 64 bits integer value.
 *? @return {C:Int64} The 64 bits integer value.
 *?
 *? @func to_uint8 Convert the number to 8 bits unsigned integer value.
 *? @return {C:UInt8} The 8 bits unsigned integer value.
 *?
 *? @func to_uint16 Convert the number to 16 bits unsigned integer value.
 *? @return {C:UInt16} The 16 bits unsigned integer value.
 *?
 *? @func to_uint32 Convert the number to 32 bits unsigned integer value.
 *? @return {C:UInt8} The 32 bits unsigned integer value.
 *?
 *? @func to_uint64 Convert the number to 64 bits unsigned integer value.
 *? @return {C:UInt64} The 8 bits unsigned integer value.
 *?
 *? @func to_ssize Convert the number to ssize_t type integer value.
 *? @return {C:SSize} The ssize_t type integer value.
 *?
 *? @func to_size Convert the number to size_t type integer value.
 *? @return {C:Size} The size_t type integer value.
 *?
 *? @class}
 */

/**
 * Initialize the number class.
 * @param ctxt The current running context.
 */
void
ox_number_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, v)

    /*Number.*/
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Number), OX_OBJECT(ctxt, Number_inf), NULL, "Number"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Number", OX_OBJECT(ctxt, Number)));
    ox_value_set_number(ctxt, v, NAN);
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Number), "NAN", v));
    ox_value_set_number(ctxt, v, INFINITY);
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Number), "INFINITY", v));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number), "is", Number_is));

    /*Number_inf.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "$to_str", Number_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "e_str", Number_inf_e_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "f_str", Number_inf_f_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "i_str", Number_inf_i_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "floor", Number_inf_floor));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "ceil", Number_inf_ceil));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "round", Number_inf_round));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "trunc", Number_inf_trunc));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "isinf", Number_inf_isinf));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "isnan", Number_inf_isnan));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_int8", Number_inf_to_int8));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_int16", Number_inf_to_int16));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_int32", Number_inf_to_int32));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_int64", Number_inf_to_int64));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_uint8", Number_inf_to_uint8));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_uint16", Number_inf_to_uint16));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_uint32", Number_inf_to_uint32));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_uint64", Number_inf_to_uint64));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_float32", Number_inf_to_float32));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_float64", Number_inf_to_float64));
#if __SIZEOF_POINTER__  == 8
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_ssize", Number_inf_to_int64));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_size", Number_inf_to_uint64));
#elif __SIZEOF_POINTER__  == 4
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_ssize", Number_inf_to_int32));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Number_inf), "to_size", Number_inf_to_uint32));
#else
    #error illegal pointer size
#endif

    OX_VS_POP(ctxt, v)
}

/**
 * Call the number class.
 * @param ctxt The current running context.
 * @param o The number class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_number_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, n);
    return OX_OK;
}