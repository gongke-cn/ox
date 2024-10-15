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
 * GError.
 */

/*Convert the GError to value.*/
static OX_Result
gerror_to_value (OX_Context *ctxt, GError *err, OX_Value *ev, OX_Value *s)
{
    OX_VS_PUSH(ctxt, v)
    OX_Value *inf = ox_script_get_value(ctxt, s, ID_GError_inf);
    OX_Value *str;
    OX_Result r;

    if ((r = ox_object_new(ctxt, ev, inf)) == OX_ERR)
        goto end;

    /*domain*/
    if ((r = ox_string_from_char_star(ctxt, v, g_quark_to_string(err->domain))) == OX_ERR)
        goto end;
    str = ox_script_get_value(ctxt, s, ID_STR_domain);
    if ((r = ox_object_add_const(ctxt, ev, str, v)) == OX_ERR)
        goto end;

    /*code*/
    ox_value_set_number(ctxt, v, err->code);
    str = ox_script_get_value(ctxt, s, ID_STR_code);
    if ((r = ox_object_add_const(ctxt, ev, str, v)) == OX_ERR)
        goto end;

    /*message*/
    if ((r = ox_string_from_char_star(ctxt, v, err->message)) == OX_ERR)
        goto end;
    str = ox_script_get_value(ctxt, s, ID_STR_message);
    if ((r = ox_object_add_const(ctxt, ev, str, v)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, v)
    return r;
}

/*Convert the OX value to gerror*/
static OX_Result
gerror_from_value (OX_Context *ctxt, GError **perr, OX_Value *ev, OX_Value *s)
{
    OX_VS_PUSH_3(ctxt, v, domainv, msgv)
    OX_Value *str;
    GQuark domain;
    int32_t code;
    const char *msg;
    OX_Result r;

    /*domain.*/
    str = ox_script_get_value(ctxt, s, ID_STR_domain);
    if ((r = ox_get(ctxt, ev, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_string(ctxt, v, domainv)) == OX_ERR)
        goto end;
    domain = g_quark_from_string(ox_string_get_char_star(ctxt, domainv));

    /*code.*/
    str = ox_script_get_value(ctxt, s, ID_STR_code);
    if ((r = ox_get(ctxt, ev, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_int32(ctxt, v, &code)) == OX_ERR)
        goto end;

    /*message.*/
    str = ox_script_get_value(ctxt, s, ID_STR_message);
    if ((r = ox_get(ctxt, ev, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_string(ctxt, v, msgv)) == OX_ERR)
        goto end;
    msg = ox_string_get_char_star(ctxt, msgv);

    *perr = g_error_new(domain, code, "%s", msg);
end:
    OX_VS_POP(ctxt, v)
    return r;
}

/*GError.$inf.$init*/
static OX_Result
GError_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *domain_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *code_arg = ox_argument(ctxt, args, argc, 1);
    OX_Value *msg_arg = ox_argument(ctxt, args, argc, 2);
    OX_VS_PUSH_3(ctxt, domainv, msgv, codev)
    OX_Value *str;
    int32_t code;
    OX_Result r;

    /*domain.*/
    if ((r = ox_to_string(ctxt, domain_arg, domainv)) == OX_ERR)
        goto end;
    str = ox_script_get_value(ctxt, f, ID_STR_domain);
    if ((r = ox_object_add_const(ctxt, thiz, str, domainv)) == OX_ERR)
        goto end;

    /*code.*/
    if ((r = ox_to_int32(ctxt, code_arg, &code)) == OX_ERR)
        goto end;
    ox_value_set_number(ctxt, codev, code);
    str = ox_script_get_value(ctxt, f, ID_STR_code);
    if ((r = ox_object_add_const(ctxt, thiz, str, codev)) == OX_ERR)
        goto end;

    /*message.*/
    if ((r = ox_to_string(ctxt, msg_arg, msgv)) == OX_ERR)
        goto end;
    str = ox_script_get_value(ctxt, f, ID_STR_message);
    if ((r = ox_object_add_const(ctxt, thiz, str, msgv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, domainv)
    return r;
}

/*GError.$inf.$to_str*/
static OX_Result
GError_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, v, s)
    OX_Value *str;
    int32_t code;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    /*domain.*/
    str = ox_script_get_value(ctxt, f, ID_STR_domain);
    if ((r = ox_get(ctxt, thiz, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_string(ctxt, v, s)) == OX_ERR)
        goto end;
    if ((r = ox_char_buffer_append_char_star(ctxt, &cb, ox_string_get_char_star(ctxt, s))) == OX_ERR)
        goto end;

    /*code.*/
    str = ox_script_get_value(ctxt, f, ID_STR_code);
    if ((r = ox_get(ctxt, thiz, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_int32(ctxt, v, &code)) == OX_ERR)
        goto end;
    if ((r = ox_char_buffer_print(ctxt, &cb, ": %d: ", code)) == OX_ERR)
        goto end;

    /*message.*/
    str = ox_script_get_value(ctxt, f, ID_STR_message);
    if ((r = ox_get(ctxt, thiz, str, v)) == OX_ERR)
        goto end;
    if ((r = ox_to_string(ctxt, v, s)) == OX_ERR)
        goto end;
    if ((r = ox_char_buffer_append_char_star(ctxt, &cb, ox_string_get_char_star(ctxt, s))) == OX_ERR)
        goto end;

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, v)
    return r;
}

/*Initialize the GError class.*/
static void
gerror_class_init (OX_Context *ctxt, OX_Value *s)
{
    OX_VS_PUSH_3(ctxt, c, inf, str)

    /*strings.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, str, "domain"));
    ox_not_error(ox_string_singleton(ctxt, str));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_domain, str));
    ox_not_error(ox_string_from_const_char_star(ctxt, str, "code"));
    ox_not_error(ox_string_singleton(ctxt, str));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_code, str));
    ox_not_error(ox_string_from_const_char_star(ctxt, str, "message"));
    ox_not_error(ox_string_singleton(ctxt, str));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_message, str));

    /*GError*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "GError"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_GError, c));
    ox_not_error(ox_script_set_value(ctxt, s, ID_GError_inf, inf));

    /*GError.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", GError_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_str", GError_inf_to_str));

    OX_VS_POP(ctxt, c)
}
