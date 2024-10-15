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
 * Error.
 */

#define OX_LOG_TAG "ox_error"

#include "ox_internal.h"

/*Throw an error with message string.*/
static OX_Result
throw_error_s (OX_Context *ctxt, OX_Value *c, OX_Value *s)
{
    OX_VS_PUSH(ctxt, e)
    OX_Result r;

    r = ox_call(ctxt, c, ox_value_null(ctxt), s, 1, e);
    if (r == OX_OK)
        ox_throw(ctxt, e);

    OX_VS_POP(ctxt, e)
    return OX_ERR;
}

/*Throw an error with va_list argument.*/
static OX_Result
throw_error_v (OX_Context *ctxt, OX_Value *c, const char *fmt, va_list ap)
{
    OX_CharBuffer cb;
    OX_Result r;
    OX_VS_PUSH(ctxt, s)

    ox_char_buffer_init(&cb);

    ox_char_buffer_print_v(ctxt, &cb, fmt, ap);

    if ((r = ox_char_buffer_get_string(ctxt, &cb, s)) == OX_OK)
        throw_error_s(ctxt, c, s);

    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, s)
    return OX_ERR;
}

/**
 * Throw an error.
 * @param ctxt The current running context.
 * @param e The error value.
 * @return OX_ERR.
 */
OX_Result
ox_throw (OX_Context *ctxt, OX_Value *e)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_Result r;

    ctxt->error_frames = ctxt->frames;

    ox_value_copy(ctxt, &ctxt->error, e);

    if (vm->dump_throw) {
        OX_VS_PUSH(ctxt, s)

        if ((r = ox_to_string(ctxt, e, s)) == OX_OK) {
            char *col = isatty(2) ? "\033[31;1m" : NULL;

            fprintf(stderr,
                    OX_TEXT("%sthrow error%s: %s\n"),
                    col ? col : "",
                    col ? "\033[0m" : "",
                    ox_string_get_char_star(ctxt, s));
            ox_stack_dump(ctxt, stderr);
        }

        OX_VS_POP(ctxt, s)
    }

    return OX_ERR;
}

/**
 * Catch the error.
 * @param ctxt The current running context.
 * @param[out] e Return the error value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_catch (OX_Context *ctxt, OX_Value *e)
{
    ox_value_copy(ctxt, e, &ctxt->error);
    ox_value_set_null(ctxt, &ctxt->error);

    return OX_OK;
}

/**
 * Throw a system error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_system_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, SystemError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw a no memory error.
 * @param ctxt The current running context.
 * @return OX_ERR.
 */
OX_Result
ox_throw_no_mem_error (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    r = ox_string_from_const_char_star(ctxt, s, OX_TEXT("not enough memory"));
    if (r == OX_OK)
        throw_error_s(ctxt, OX_OBJECT(ctxt, NoMemoryError), s);

    OX_VS_POP(ctxt, s)

    return OX_ERR;
}

/**
 * Throw a null error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_null_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, NullError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw a value range error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_range_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, RangeError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw an access error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_access_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, AccessError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw a type error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_type_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, TypeError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw a syntax error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_syntax_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, SyntaxError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/**
 * Throw a reference error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
OX_Result
ox_throw_reference_error (OX_Context *ctxt, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    throw_error_v(ctxt, OX_OBJECT(ctxt, ReferenceError), fmt, ap);

    va_end(ap);

    return OX_ERR;
}

/*Error_inf.$init*/
static OX_Result
Error_inf_init (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if (!ox_value_is_null(ctxt, arg)) {
        if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
            goto end;

        if ((r = ox_set(ctxt, thiz, OX_STRING(ctxt, message), s)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Error_inf.$to_str*/
static OX_Result
Error_inf_to_str (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CharBuffer cb;
    OX_Result r;
    OX_VS_PUSH_3(ctxt, msg, s, c)

    ox_char_buffer_init(&cb);

    if ((r = ox_instance_of(ctxt, thiz, OX_OBJECT(ctxt, Error))) == OX_ERR)
        goto end;

    if (!r) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not an error"));
        goto end;
    }

    if ((r = ox_type_of(ctxt, thiz, c)) == OX_ERR)
        goto end;

    if ((r = ox_get_full_name(ctxt, c, s)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
        goto end;

    if ((r = ox_get(ctxt, thiz, OX_STRING(ctxt, message), msg)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, msg)) {
        if ((r = ox_to_string(ctxt, msg, s)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char_star(ctxt, &cb, ": ")) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
            goto end;
    }

    if ((r = ox_char_buffer_get_string(ctxt, &cb, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, msg)
    return r;
}

/**
 * Initialize the Error class.
 * @param ctxt The current running context.
 */
void
ox_error_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, inf)

    /*Error.*/
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Error), inf, NULL, "Error"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Error", OX_OBJECT(ctxt, Error)));

    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Error_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_str", Error_inf_to_str));

#define add_error(t)\
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, t##Error), inf, NULL, #t "Error"));\
    ox_not_error(ox_class_inherit(ctxt, OX_OBJECT(ctxt, t##Error), OX_OBJECT(ctxt, Error)));\
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), #t "Error", OX_OBJECT(ctxt, t##Error)));

    add_error(Null)
    add_error(Type)
    add_error(Range)
    add_error(System)
    add_error(Reference)
    add_error(NoMemory)
    add_error(Syntax)
    add_error(Access)

    OX_VS_POP(ctxt, inf)
}
