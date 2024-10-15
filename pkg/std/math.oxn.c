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
 * Math library.
 */

#define OX_LOG_TAG "math"

#define _GNU_SOURCE
#include <math.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_PI,
    ID_E,
    ID_max,
    ID_min,
    ID_abs,
    ID_sin,
    ID_cos,
    ID_tan,
    ID_asin,
    ID_acos,
    ID_atan,
    ID_pow,
    ID_sqrt,
    ID_cbrt,
    ID_log,
    ID_log2,
    ID_log10,
    ID_exp,
    ID_exp2,
    ID_floor,
    ID_ceil,
    ID_round,
    ID_trunc,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "PI",
    "E",
    "max",
    "min",
    "abs",
    "sin",
    "cos",
    "tan",
    "asin",
    "acos",
    "atan",
    "pow",
    "sqrt",
    "cbrt",
    "log",
    "log2",
    "log10",
    "exp",
    "exp2",
    "floor",
    "ceil",
    "round",
    "trunc",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*max.*/
static OX_Result
Math_max (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    if (argc) {
        size_t i;
        OX_Value *arg;
        OX_Number n, max;
        OX_Result r;

        for (i = 0; i < argc; i ++) {
            arg = ox_values_item(ctxt, args, i);

            if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
                return r;

            if (i)
                max = OX_MAX(max, n);
            else
                max = n;
        }

        ox_value_set_number(ctxt, rv, max);
    }

    return OX_OK;
}

/*min.*/
static OX_Result
Math_min (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    if (argc) {
        size_t i;
        OX_Value *arg;
        OX_Number n, min;
        OX_Result r;

        for (i = 0; i < argc; i ++) {
            arg = ox_values_item(ctxt, args, i);

            if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
                return r;

            if (i)
                min = OX_MIN(min, n);
            else
                min = n;
        }

        ox_value_set_number(ctxt, rv, min);
    }

    return OX_OK;
}

/*abs.*/
static OX_Result
Math_abs (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, fabs(n));
    return OX_OK;
}

/*sin.*/
static OX_Result
Math_sin (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = sin(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*cos.*/
static OX_Result
Math_cos (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = cos(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*tan.*/
static OX_Result
Math_tan (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = tan(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*asin.*/
static OX_Result
Math_asin (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = asin(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*acos.*/
static OX_Result
Math_acos (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = acos(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*atan.*/
static OX_Result
Math_atan (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = atan(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*pow.*/
static OX_Result
Math_pow (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *x_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *y_arg = ox_argument(ctxt, args, argc, 1);
    OX_Number x, y, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, x_arg, &x)) == OX_ERR)
        return r;

    if ((r = ox_to_number(ctxt, y_arg, &y)) == OX_ERR)
        return r;

    rn = pow(x, y);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*sqrt.*/
static OX_Result
Math_sqrt (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = sqrt(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*cbrt.*/
static OX_Result
Math_cbrt (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = cbrt(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*log.*/
static OX_Result
Math_log (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = log(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*log2.*/
static OX_Result
Math_log2 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = log2(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*log10.*/
static OX_Result
Math_log10 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = log10(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*exp.*/
static OX_Result
Math_exp (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = exp(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*exp2.*/
static OX_Result
Math_exp2 (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = exp2(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*floor.*/
static OX_Result
Math_floor (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = floor(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*ceil.*/
static OX_Result
Math_ceil (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = ceil(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*round.*/
static OX_Result
Math_round (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = round(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*trunc.*/
static OX_Result
Math_trunc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n, rn;
    OX_Result r;

    if ((r = ox_to_number(ctxt, n_arg, &n)) == OX_ERR)
        return r;

    rn = trunc(n);

    ox_value_set_number(ctxt, rv, rn);
    return OX_OK;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Mathematical related functions
 *?
 *? @const PI {Number} Ratio of the circumference of a circle to its diameter（π）.
 *? @const E {Number} The value of the base of natural logarith.
 *?
 *? @func max Get the maximum value of the arguments.
 *? @return {Number} The maximum value.
 *?
 *? @func min Get the minimum value of the arguments.
 *? @return {Number} The minimum value.
 *?
 *? @func abs Get the absolute value.
 *? @param x {Number} The input value.
 *? @return {Number} The absolute value of x.
 *?
 *? @func sin Sine function.
 *? @param x {Number} The input number given in radians.
 *? @return {Number} On success, the function returns the sine of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is positive infinity or negative infinity, a domain error occurs, and a NaN is returned.
 *? @ul}
 *?
 *? @func cos Cosine function.
 *? @param x {Number} The input number given in radians.
 *? @return {Number} On success, the function returns the cosine of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is positive infinity or negative infinity, a domain error occurs, and a NaN is returned.
 *? @ul}
 *?
 *? @func tan Tangent function.
 *? @param x {Number} The input number given in radians.
 *? @return {Number} On success, the function returns the tangent of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is positive infinity or negative infinity, a domain error occurs, and a NaN is returned.
 *? @li If the correct result would overflow, a range error occurs, and the functions return HUGE_VAL, HUGE_VALF, or HUGE_VALL, respectively, with the mathematically correct sign.
 *? @ul}
 *?
 *? @func asin Arc sine function.
 *? @param x {Number} The input number.
 *? @return {Number} On success, the function returns the principal value of the arc sine of x in radians; the return value is in the range [-pi/2, pi/2].
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is +0 (-0), +0 (-0) is returned.
 *? @li If x is outside the range [-1, 1], a domain error occurs, and a NaN is returned.
 *? @ul}
 *?
 *? @func acos Arc cosine function.
 *? @param x {Number} The input number.
 *? @return {Number} On success, the function returns the arc cosine of x in radians; the return value is in the range [0, pi].
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is +1, +0 is returned.
 *? @li If x is positive infinity or negative infinity, a domain error occurs, and a NaN is returned.
 *? @li If x is outside the range [-1, 1], a domain error occurs, and a NaN is returned.
 *? @ul}
 *?
 *? @func atan Arc tangent function.
 *? @param x {Number} The input number.
 *? @return {Number} On success, the function returns the principal value of the arc tangent of x in radians; the return value is in the range [-pi/2, pi/2].
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is +0 (-0), +0 (-0) is returned.
 *? @li If x is positive infinity (negative infinity), +pi/2 (-pi/2) is returned.
 *? @ul}
 *?
 *? @func pow Power function.
 *? The function return the value of x raised to the power of y.
 *? @param x Number x.
 *? @param y Number y.
 *? @return {Number} return the value of x to the power of y.
 *? @ul{
 *? @li If the result overflows, a range error occurs, and the functions return HUGE_VAL, HUGE_VALF, or HUGE_VALL, respectively, with the mathematically correct sign.
 *? @li If result underflows, and is not representable, a range error occurs, and 0.0 with the appropriate sign is returned.
 *? @li If x is +0 or -0, and y is an odd integer less than 0, a pole error occurs and HUGE_VAL, HUGE_VALF, or HUGE_VALL, is returned, with the same sign as x.
 *? @li If x is +0 or -0, and y is less than 0 and not an odd integer, a pole error occurs and +HUGE_VAL, +HUGE_VALF, or +HUGE_VALL, is returned.
 *? @li If x is +0 (-0), and y is an odd integer greater than 0, the result is +0 (-0).
 *? @li If x is 0, and y greater than 0 and not an odd integer, the result is +0.
 *? @li If x is -1, and y is positive infinity or negative infinity, the result is 1.0.
 *? @li If x is +1, the result is 1.0 (even if y is a NaN).
 *? @li If y is 0, the result is 1.0 (even if x is a NaN).
 *? @li If x is a finite value less than 0, and y is a finite noninteger, a domain error occurs, and a NaN is returned.
 *? @li If the absolute value of x is less than 1, and y is negative infinity, the result is positive infinity.
 *? @li If the absolute value of x is greater than 1, and y is negative infinity, the result is +0.
 *? @li If the absolute value of x is less than 1, and y is positive infinity, the result is +0.
 *? @li If the absolute value of x is greater than 1, and y is positive infinity, the result is positive infinity.
 *? @li If x is negative infinity, and y is an odd integer less than 0, the result is -0.
 *? @li If x is negative infinity, and y less than 0 and not an odd integer, the result is +0.
 *? @li If x is negative infinity, and y is an odd integer greater than 0, the result is negative infinity.
 *? @li If x is negative infinity, and y greater than 0 and not an odd integer, the result is positive infinity.
 *? @li If x is positive infinity, and y less than 0, the result is +0.
 *? @li If x is positive infinity, and y greater than 0, the result is positive infinity.
 *? @li Except as specified above, if x or y is a NaN, the result is a NaN.
 *? @ul}
 *?
 *? @func sqrt Square root function.
 *? These functions return the nonnegative square root of x.
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the square root of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is +0 (-0), +0 (-0) is returned.
 *? @li If x is positive infinity, positive infinity is returned.
 *? @li If x is less than -0, a domain error occurs, and a NaN is returned.
 *? @ul}
 *?
 *? @func cbrt Cube root function.
 *? These  functions return the (real) cube root of x. This function cannot fail;
 *? every representable real value has a real cube root, and rounding it to a representable value
 *? never causes overflow nor underflow.
 *? @param x {Number} Input number.
 *? @return {Number} return the cube root of x.
 *? If x is +0, -0, positive infinity, negative infinity, or NaN, x is returned.
 *?
 *? @func log Natural logarithmic function
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the natural logarithm of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is 1, the result is +0.
 *? @li If x is positive infinity, positive infinity is returned.
 *? @li If x is zero, then a pole error occurs, and the functions return -HUGE_VAL, -HUGE_VALF, or -HUGE_VALL, respectively.
 *? @li If x is negative (including negative infinity), then a domain error occurs, and a NaN (not a number) is returned.
 *? @ul}
 *?
 *? @func log2 Base-2 logarithmic function.
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the base-2 logarithm of x.
 *? For special cases, including where x is 0, 1, negative, infinity, or NaN.
 *?
 *? @func log10 Base-10 logarithmic function.
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the base-10 logarithm of x.
 *? For special cases, including where x is 0, 1, negative, infinity, or NaN.
 *?
 *? @func exp Base-e exponential function.
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the exponential value of x.
 *? @ul{
 *? @li If x is a NaN, a NaN is returned.
 *? @li If x is positive infinity, positive infinity is returned.
 *? @li If x is negative infinity, +0 is returned.
 *? @li If the result underflows, a range error occurs, and zero is returned.
 *? @li If the result overflows, a range error occurs, and the functions return +HUGE_VAL, +HUGE_VALF, or +HUGE_VALL, respectively.
 *? @ul}
 *?
 *? @func exp2 Base-2 exponential function.
 *? @param x {Number} Input number.
 *? @return {Number} On success, these functions return the base-2 exponential value of x.
 *? For various special cases, including the handling of infinity and NaN, as well as overflows and underflows.
 *?
 *? @func floor Get the largest integral value not greater than argument.
 *? The function returns the largest integral value that is not greater than x.
 *? For example, floor(0.5) is 0.0, and floor(-0.5) is -1.0.
 *? @param x {Number} Input number.
 *? @return {Number} The function returns the floor of x.
 *? If x is integral, +0, -0, NaN, or an infinity, x itself is returned.
 *?
 *? @func ceil Get the smallest integral value not less than argument.
 *? The function returns the smallest integral value that is not less than x.
 *? For example, ceil(0.5) is 1.0, and ceil(-0.5) is 0.0.
 *? @param x {Number} Input number.
 *? @return {Number} The function returns the ceiling of x.
 *? If x is integral, +0, -0, NaN, or infinite, x itself is returned.
 *?
 *? @func round Round to nearest integer, away from zero.
 *? The function rounds x to the nearest integer, but round halfway cases away from zero,
 *? For example, round(0.5) is 1.0, and round(-0.5) is -1.0.
 *? @param x {Number} Input number.
 *? @return {Number} The function returns the rounded integer value.
 *? If x is integral, +0, -0, NaN, or infinite, x itself is returned.
 *?
 *? @func trunc Round to integer, toward zero.
 *? The function rounds x to the nearest integer value that is not larger in magnitude than x.
 *? @param x {Number} Input number.
 *? @return {Number} The function returns the rounded integer value, in floating format.
 *? If x is integral, infinite, or NaN, x itself is returned.
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)

    /*PI.*/
    ox_value_set_number(ctxt, v, M_PI);
    ox_not_error(ox_script_set_value(ctxt, s, ID_PI, v));

    /*E.*/
    ox_value_set_number(ctxt, v, M_E);
    ox_not_error(ox_script_set_value(ctxt, s, ID_E, v));

    /*max.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_max, NULL, "max"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_max, v));

    /*max.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_min, NULL, "min"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_min, v));

    /*abs.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_abs, NULL, "abs"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_abs, v));

    /*sin.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_sin, NULL, "sin"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_sin, v));

    /*cos.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_cos, NULL, "cos"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_cos, v));

    /*tan.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_tan, NULL, "tan"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_tan, v));

    /*asin.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_asin, NULL, "asin"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_asin, v));

    /*acos.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_acos, NULL, "acos"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_acos, v));

    /*atan.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_atan, NULL, "atan"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_atan, v));

    /*pow.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_pow, NULL, "pow"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_pow, v));

    /*sqrt.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_sqrt, NULL, "sqrt"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_sqrt, v));

    /*cbrt.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_cbrt, NULL, "cbrt"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_cbrt, v));

    /*log.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_log, NULL, "log"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_log, v));

    /*log2.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_log2, NULL, "log2"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_log2, v));

    /*log10.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_log10, NULL, "log10"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_log10, v));

    /*exp.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_exp, NULL, "exp"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_exp, v));

    /*exp2.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_exp2, NULL, "exp2"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_exp2, v));

    /*floor.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_floor, NULL, "floor"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_floor, v));

    /*ceil.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_ceil, NULL, "ceil"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_ceil, v));

    /*round.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_round, NULL, "round"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_round, v));

    /*trunc.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, Math_trunc, NULL, "trunc"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_trunc, v));

    OX_VS_POP(ctxt, v)
    return OX_OK;
}

