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
 * JSON functtions.
 */

#define OX_LOG_TAG "ox_json"

#include "ox_internal.h"

/** JSON loader context.*/
typedef struct {
    OX_Lex    lex;   /**< The lexical analyzer.*/
    OX_Token  token; /**< The current token.*/
    OX_Bool   unget; /**< Unget token flag.*/
    OX_Bool   sched; /**< Need schedule when read.*/
} JsonLoadCtxt;

/** JSON store object stack.*/
typedef struct JsonStoreStack_s JsonStoreStack;
/** JSON store object stack.*/
struct JsonStoreStack_s {
    JsonStoreStack *bot; /**< The bottom stack entry.*/
    OX_Object      *o;   /**< The object.*/
};

/** JSON store context.*/
typedef struct {
    JsonStoreStack *stack;  /**< The object stack.*/
    OX_Value       *indent; /**< Indent string.*/
    OX_CharBuffer   text;   /**< Text buffer.*/
    int             depth;  /**< Depth of the stack.*/
    OX_Value       *filter; /**< Filter function.*/
    OX_Value       *map;    /**< Map function.*/
} JsonStoreCtxt;

/*Load a value from input.*/
static OX_Result
json_load_value (OX_Context *ctxt, JsonLoadCtxt *jlc, OX_Value *v);
/*Store a value.*/
static OX_Result
json_store_value (OX_Context *ctxt, JsonStoreCtxt *jsc, OX_Value *args);

/*Output error message.*/
static OX_Result
error (OX_Context *ctxt, JsonLoadCtxt *jlc, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    if (!loc)
        loc = &jlc->token.loc;

    va_start(ap, fmt);
    ox_prompt_v(ctxt, jlc->lex.input, loc, OX_PROMPT_ERROR, fmt, ap);
    va_end(ap);

    return OX_ERR;
}

/*Get a token from the input.*/
static void
get_token (OX_Context *ctxt, JsonLoadCtxt *jlc)
{
    if (jlc->unget) {
        jlc->unget = OX_FALSE;
        return;
    }

    if (jlc->sched)
        ox_suspend(ctxt);

    ox_lex_token(ctxt, &jlc->lex, &jlc->token, OX_LEX_FL_NO_EMBED_EXPR);

    if (jlc->sched)
        ox_resume(ctxt);
}

/*Unget a token.*/
static void
unget_token (OX_Context *ctxt, JsonLoadCtxt *jlc)
{
    jlc->unget = OX_TRUE;
}

/*Load an array from input.*/
static OX_Result
json_load_array (OX_Context *ctxt, JsonLoadCtxt *jlc, OX_Value *v)
{
    OX_VS_PUSH(ctxt, item)
    OX_Result r;

    if ((r = ox_array_new(ctxt, v, 0)) == OX_ERR)
        goto end;

    while (1) {
        get_token(ctxt, jlc);

        if (jlc->token.type == OX_INPUT_END) {
            r = error(ctxt, jlc, NULL, OX_TEXT("unteminated array"));
            goto end;
        }
        if (jlc->token.type == ']')
            break;

        unget_token(ctxt, jlc);

        if ((r = json_load_value(ctxt, jlc, item)) == OX_ERR)
            goto end;

        if ((r = ox_array_append(ctxt, v, item)) == OX_ERR)
            goto end;

        get_token(ctxt, jlc);
        if (jlc->token.type == ']')
            break;
        if (jlc->token.type != ',') {
            unget_token(ctxt, jlc);
            /*r = error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect `,\' or `]\' here"));
            goto end;*/
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*Load an object from the input.*/
static OX_Result
json_load_object (OX_Context *ctxt, JsonLoadCtxt *jlc, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, pn, pv)
    OX_Result r;

    if ((r = ox_object_new(ctxt, v, NULL)) == OX_ERR)
        goto end;

    while (1) {
        get_token(ctxt, jlc);

        if (jlc->token.type == OX_INPUT_END) {
            r = error(ctxt, jlc, NULL, OX_TEXT("unteminated object"));
            goto end;
        }
        if (jlc->token.type == '}')
            break;

        if ((jlc->token.type == OX_TOKEN_STRING) || (jlc->token.type == OX_TOKEN_ID)) {
            ox_value_copy(ctxt, pn, jlc->token.v);
        } else {
            r = error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect a property name here"));
            goto end;
        }

        get_token(ctxt, jlc);
        if (jlc->token.type != ':') {
            r = error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect `:\' here"));
            goto end;
        }

        if ((r = json_load_value(ctxt, jlc, pv)) == OX_ERR)
            goto end;

        if ((r = ox_object_add_prop(ctxt, v, OX_PROPERTY_VAR, pn, pv, NULL)) == OX_ERR)
            goto end;

        get_token(ctxt, jlc);
        if (jlc->token.type == '}')
            break;
        if (jlc->token.type != ',') {
            unget_token(ctxt, jlc);
            /*r = error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect `,\' or `}\' here"));
            goto end;*/
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, pn)
    return r;
}

/*Load a value from input.*/
static OX_Result
json_load_value (OX_Context *ctxt, JsonLoadCtxt *jlc, OX_Value *v)
{
    OX_Number n;
    const char *cstr;
    OX_Result r;

    get_token(ctxt, jlc);

    switch (jlc->token.type) {
    case OX_TOKEN_NULL:
    case OX_TOKEN_BOOL:
    case OX_TOKEN_NUMBER:
    case OX_TOKEN_STRING:
        ox_value_copy(ctxt, v, jlc->token.v);
        break;
    case '+':
        get_token(ctxt, jlc);
        if (jlc->token.type != OX_TOKEN_NUMBER) {
            return error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect a number here"));
        }

        ox_value_copy(ctxt, v, jlc->token.v);
        break;
    case '-':
        get_token(ctxt, jlc);
        if (jlc->token.type != OX_TOKEN_NUMBER)
            return error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect a number here"));

        n = ox_value_get_number(ctxt, jlc->token.v);
        if (isnan(n))
            return error(ctxt, jlc, NULL, OX_TEXT("unexpected NaN after `-\'"));

        ox_value_set_number(ctxt, v, -n);
        break;
    case '[':
        if ((r = json_load_array(ctxt, jlc, v)) == OX_ERR)
            return r;
        break;
    case '{':
        if ((r = json_load_object(ctxt, jlc, v)) == OX_ERR)
            return r;
        break;
    case OX_TOKEN_ID:
        cstr = ox_string_get_char_star(ctxt, jlc->token.v);

        if (!strcmp(cstr, "NaN")) {
            jlc->token.type = OX_TOKEN_NUMBER;
            ox_value_set_number(ctxt, jlc->token.v, NAN);
            break;
        } else if (!strcmp(cstr, "Infinity")) {
            jlc->token.type = OX_TOKEN_NUMBER;
            ox_value_set_number(ctxt, jlc->token.v, INFINITY);
            break;
        }
    default:
        return error(ctxt, jlc, NULL, OX_TEXT("unexpected token, expect a value here"));
    }

    return OX_OK;
}

/*Store a string.*/
static OX_Result
json_store_string (OX_Context *ctxt, JsonStoreCtxt *jsc, OX_Value *v)
{
    const char *cstr, *pc;
    size_t len, left;
    OX_Result r;

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '\"')) == OX_ERR)
        return r;

    cstr = ox_string_get_char_star(ctxt, v);
    len = ox_string_length(ctxt, v);

    pc = cstr;
    left = len;
    while (left) {
        int c = *pc;

        switch (c) {
        case '\n':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\n", 2);
            break;
        case '\r':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\r", 2);
            break;
        case '\t':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\t", 2);
            break;
        case '\v':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\v", 2);
            break;
        case '\f':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\f", 2);
            break;
        case '\a':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\a", 2);
            break;
        case '\b':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\b", 2);
            break;
        case '\"':
            r = ox_char_buffer_append_chars(ctxt, &jsc->text, "\\\"", 2);
            break;
        default:
            r = ox_char_buffer_append_char(ctxt, &jsc->text, c);
            break;
        }

        if (r == OX_ERR)
            return r;

        pc ++;
        left --;
    }

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '\"')) == OX_ERR)
        return r;

    return OX_OK;
}

/*Store new line.*/
static OX_Result
json_store_lf (OX_Context *ctxt, JsonStoreCtxt *jsc)
{
    OX_Result r;

    if (!jsc->indent)
        return OX_OK;

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '\n')) == OX_ERR)
        return r;

    return OX_OK;
}

/*Store indent.*/
static OX_Result
json_store_indent (OX_Context *ctxt, JsonStoreCtxt *jsc)
{
    size_t i;
    OX_Result r;

    if (!jsc->indent)
        return OX_OK;

    for (i = 0; i < jsc->depth; i ++) {
        if ((r = ox_char_buffer_append_string(ctxt, &jsc->text, jsc->indent)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Store an array.*/
static OX_Result
json_store_array (OX_Context *ctxt, JsonStoreCtxt *jsc, OX_Value *v)
{
    OX_VS_PUSH_4(ctxt, root, key, item, rv)
    size_t len = ox_array_length(ctxt, v);
    OX_Object *o = ox_value_get_gco(ctxt, v);
    JsonStoreStack stack, *ps;
    size_t i;
    OX_Result r;

    for (ps = jsc->stack; ps; ps = ps->bot) {
        if (ps->o == o) {
            /*if ((r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "null")) == OX_ERR)
                goto end;
            r = OX_OK;*/
            r = ox_throw_reference_error(ctxt, OX_TEXT("circular reference in JSON"));
            goto end;
        }
    }

    stack.bot = jsc->stack;
    stack.o = o;
    jsc->stack = &stack;

    ox_value_copy(ctxt, root, v);

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '[')) == OX_ERR)
        goto end;

    if (len) {
        if ((r = json_store_lf(ctxt, jsc)) == OX_ERR)
            goto end;

        jsc->depth ++;

        for (i = 0; i < len; i ++) {
            if ((r = ox_array_get_item(ctxt, v, i, item)) == OX_ERR)
                goto end;

            if (jsc->filter) {
                ox_value_set_number(ctxt, key, i);

                if ((r = ox_call(ctxt, jsc->filter, ox_value_null(ctxt), root, 3, rv)) == OX_ERR)
                    goto end;

                if (!ox_to_bool(ctxt, rv))
                    continue;
            }

            if ((r = json_store_indent(ctxt, jsc)) == OX_ERR)
                goto end;

            ox_value_set_number(ctxt, key, i);

            if ((r = json_store_value(ctxt, jsc, root)) == OX_ERR)
                goto end;

            if (i != len - 1) {
                if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, ',')) == OX_ERR)
                    goto end;
            }

            if ((r = json_store_lf(ctxt, jsc)) == OX_ERR)
                goto end;
        }

        jsc->depth --;
        if ((r = json_store_indent(ctxt, jsc)) == OX_ERR)
            goto end;
    }

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, ']')) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    jsc->stack = stack.bot;
    OX_VS_POP(ctxt, root)
    return r;
}

/*Store an object.*/
static OX_Result
json_store_object (OX_Context *ctxt, JsonStoreCtxt *jsc, OX_Value *v)
{
    OX_VS_PUSH_6(ctxt, iter, e, root, pn, pv, rv)
    OX_Bool first = OX_TRUE;
    OX_Object *o = ox_value_get_gco(ctxt, v);
    JsonStoreStack stack, *ps;
    OX_Result r;

    for (ps = jsc->stack; ps; ps = ps->bot) {
        if (ps->o == o) {
            /*if ((r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "null")) == OX_ERR)
                goto end;
            r = OX_OK;*/
            r = ox_throw_reference_error(ctxt, OX_TEXT("circular reference in JSON"));
            goto end;
        }
    }

    stack.bot = jsc->stack;
    stack.o = o;
    jsc->stack = &stack;

    ox_value_copy(ctxt, root, v);

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '{')) == OX_ERR)
        goto end;

    if ((r = ox_object_iter_new(ctxt, iter, v, OX_OBJECT_ITER_KEY_VALUE)) == OX_ERR)
        goto end;

    while (1) {
        if ((r = ox_iterator_end(ctxt, iter)) == OX_ERR)
            goto end;
        if (r) {
            if (!first) {
                if ((r = json_store_lf(ctxt, jsc)) == OX_ERR)
                    goto end;
                jsc->depth --;
                if ((r = json_store_indent(ctxt, jsc)) == OX_ERR)
                    goto end;
            }
            break;
        }

        if ((r = ox_iterator_value(ctxt, iter, e)) == OX_ERR)
            goto end;

        if ((r = ox_array_get_item(ctxt, e, 0, pn)) == OX_ERR)
            goto end;

        if ((r = ox_array_get_item(ctxt, e, 1, pv)) == OX_ERR)
            goto end;

        if (jsc->filter) {
            if ((r = ox_call(ctxt, jsc->filter, ox_value_null(ctxt), root, 3, rv)) == OX_ERR)
                goto end;
            if (!ox_to_bool(ctxt, rv)) {
                if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
                    goto end;
                continue;
            }
        }

        if (first) {
            first = OX_FALSE;

            if ((r = json_store_lf(ctxt, jsc)) == OX_ERR)
                goto end;

            jsc->depth ++;
        } else {
            if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, ',')) == OX_ERR)
                goto end;
            if ((r = json_store_lf(ctxt, jsc)) == OX_ERR)
                goto end;
        }

        if ((r = json_store_indent(ctxt, jsc)) == OX_ERR)
            goto end;

        if ((r = json_store_string(ctxt, jsc, pn)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, ':')) == OX_ERR)
            goto end;

        if (jsc->indent) {
            if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, ' ')) == OX_ERR)
                goto end;
        }

        if ((r = json_store_value(ctxt, jsc, root)) == OX_ERR)
            goto end;

        if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
            goto end;
    }

    if ((r = ox_char_buffer_append_char(ctxt, &jsc->text, '}')) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    jsc->stack = stack.bot;
    if (!ox_value_is_null(ctxt, iter))
        ox_close(ctxt, iter);
    OX_VS_POP(ctxt, iter)
    return r;
}

/*Store a value.*/
static OX_Result
json_store_value (OX_Context *ctxt, JsonStoreCtxt *jsc, OX_Value *args)
{
    OX_Result r;
    OX_VS_PUSH_3(ctxt, f, t, s)
    OX_Value *v = ox_values_item(ctxt, args, 2);

    if (ox_value_is_null(ctxt, v)) {
        r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "null");
        goto end;
    }

    if ((r = ox_get(ctxt, v, OX_STRING(ctxt, _to_json), f)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, f)) {
        if ((r = ox_call(ctxt, f, v, NULL, 0, t)) == OX_ERR)
            goto end;
        ox_value_copy(ctxt, v, t);
    }

    if (jsc->map) {
        if ((r = ox_call(ctxt, jsc->map, ox_value_null(ctxt), args, 3, t)) == OX_ERR)
            goto end;
        ox_value_copy(ctxt, v, t);
    }

    if (ox_value_is_bool(ctxt, v)) {
        r = ox_char_buffer_append_char_star(ctxt, &jsc->text,
                ox_value_get_bool(ctxt, v) ? "true" : "false");
    } else if (ox_value_is_number(ctxt, v) || ox_value_is_c_number(ctxt, v)) {
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            goto end;

        if (isinf(n)) {
            if (n < 0)
                r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "-Infinity");
            else
                r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "Infinity");
        } else if (isnan(n)) {
            r = ox_char_buffer_append_char_star(ctxt, &jsc->text, "NaN");
        } else {
            r = ox_char_buffer_print(ctxt, &jsc->text, "%.21g", n);
        }
    } else if (ox_value_is_string(ctxt, v) || ox_value_is_c_string(ctxt, v)) {
        if ((r = ox_to_string(ctxt, v, s)) == OX_ERR)
            goto end;

        r = json_store_string(ctxt, jsc, s);
    } else if (ox_value_is_array(ctxt, v)) {
        r = json_store_array(ctxt, jsc, v);
    } else if (ox_value_is_object(ctxt, v)) {
        r = json_store_object(ctxt, jsc, v);
    } else {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be stored as JSON"));
    }
end:
    OX_VS_POP(ctxt, f)
    return r;
}

/**
 * Parse the JSON input.
 * @param ctxt The current running context.
 * @param inputv The JSON input.
 * @param sched Schedule when read from input.
 * @param[out] v Return the result value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_json_parse (OX_Context *ctxt, OX_Value *input, OX_Bool sched, OX_Value *v)
{
    OX_Input *ip;
    JsonLoadCtxt jlc;
    OX_Result r;

    assert(ctxt && input && v);
    assert(ox_value_is_input(ctxt, input));

    ip = ox_value_get_gco(ctxt, input);

    ox_lex_init(ctxt, &jlc.lex, ip);
    jlc.token.v = ox_value_stack_push(ctxt);
    jlc.token.type = OX_TOKEN_END;
    jlc.unget = OX_FALSE;
    jlc.sched = sched;

    r = json_load_value(ctxt, &jlc, v);

    ox_value_stack_pop(ctxt, jlc.token.v);
    ox_lex_deinit(ctxt, &jlc.lex);
    ox_input_close(ctxt, input);

    if (r == OX_OK) {
        if (ox_input_error(ip) || (jlc.lex.status & OX_LEX_ST_ERR))
            r = OX_ERR;
    }

    if (r == OX_ERR)
        ox_throw_syntax_error(ctxt, OX_TEXT("parse JSON failed"));

    return r;
}

/**
 * Convert the value to JSON string.
 * @param ctxt The current running context.
 * @param v The value.
 * @param indent Indent string.
 * @param filter The filter function.
 * @param map The map function.
 * @param[out] str Return the JSON string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_json_to_str (OX_Context *ctxt, OX_Value *v, OX_Value *indent, OX_Value *filter,
        OX_Value *map, OX_Value *str)
{
    OX_Value *args = ox_value_stack_push_n(ctxt, 3);
    JsonStoreCtxt jsc;
    OX_Result r;

    assert(ctxt && v && str);

    jsc.stack = NULL;
    jsc.depth = 0;
    ox_char_buffer_init(&jsc.text);

    if (indent && !ox_value_is_null(ctxt, indent))
        jsc.indent = indent;
    else
        jsc.indent = NULL;

    if (filter && !ox_value_is_null(ctxt, filter)) {
        jsc.filter = filter;
    } else {
        jsc.filter = NULL;
    }

    if (map && !ox_value_is_null(ctxt, map)) {
        jsc.map = map;
    } else {
        jsc.map = NULL;
    }

    ox_value_copy(ctxt, ox_values_item(ctxt, args, 2), v);

    if ((r = json_store_value(ctxt, &jsc, args)) == OX_ERR)
        goto end;

    r = ox_char_buffer_get_string(ctxt, &jsc.text, str);
end:
    ox_char_buffer_deinit(ctxt, &jsc.text);
    ox_value_stack_pop(ctxt, args);
    return r;
}
