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
 * XML parser.
 */

#define OX_LOG_TAG "xml"

#include <ox_internal.h>

/*Declaration index.*/
enum {
    ID_XmlInput,
    ID_XmlInput_inf,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "XmlInput",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Entity input.*/
typedef struct OX_EntityInput_s OX_EntityInput;
/** Entity input.*/
struct OX_EntityInput_s {
    OX_EntityInput *bot;    /**< The bottom entity input.*/
    OX_Input       *input;  /**< The input.*/
    OX_Value        inputv; /**< The input value.*/
};

/** XML input.*/
typedef struct {
    OX_Input       *input;     /**< The input.*/
    OX_Value        inputv;    /**< The input value.*/
    OX_Value        entity_cb; /**< Entity callback.*/
    OX_Location     text_loc;  /**< The text location.*/
    OX_CharBuffer   text;      /**< Text buffer.*/
    OX_EntityInput *entities;  /**< The entity input stack.*/
    OX_Bool         resolve_pe;/**< Resolve the parsed entities.*/
} OX_XmlInput;

/**Input error.*/
#define XML_INPUT_ERR (-2)

/*Scan reference objects in XML input.*/
static void
xml_input_scan (OX_Context *ctxt, void *ptr)
{
    OX_XmlInput *xi = ptr;
    OX_EntityInput *ei;

    for (ei = xi->entities; ei; ei = ei->bot)
        ox_gc_scan_value(ctxt, &ei->inputv);

    ox_gc_scan_value(ctxt, &xi->inputv);
    ox_gc_scan_value(ctxt, &xi->entity_cb);
}

/*Free the entity input.*/
static void
entity_input_free (OX_Context *ctxt, OX_EntityInput *ei)
{
    OX_DEL(ctxt, ei);
}

/*Free the XML input.*/
static void
xml_input_free (OX_Context *ctxt, void *ptr)
{
    OX_XmlInput *xi = ptr;
    OX_EntityInput *ei, *nei;

    for (ei = xi->entities; ei; ei = nei) {
        nei = ei->bot;
        entity_input_free(ctxt, ei);
    }

    ox_char_buffer_deinit(ctxt, &xi->text);

    OX_DEL(ctxt, xi);
}

/*Operation functions of XML input.*/
static const OX_PrivateOps
xml_input_ops = {
    xml_input_scan,
    xml_input_free
};

/*Create a new XML input.*/
static OX_XmlInput*
xml_input_new (OX_Context *ctxt, OX_Value *v, OX_Value *f)
{
    OX_XmlInput *xi;
    OX_Result r;

    if ((r = ox_object_new(ctxt, v, ox_script_get_value(ctxt, f, ID_XmlInput_inf))) == OX_ERR)
        return NULL;

    if (!OX_NEW(ctxt, xi)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    xi->input = NULL;
    xi->entities = NULL;
    xi->resolve_pe = OX_FALSE;

    ox_value_set_null(ctxt, &xi->inputv);
    ox_value_set_null(ctxt, &xi->entity_cb);

    memset(&xi->text_loc, 0, sizeof(xi->text_loc));

    ox_char_buffer_init(&xi->text);

    if ((r = ox_object_set_priv(ctxt, v, &xml_input_ops, xi)) == OX_ERR) {
        xml_input_free(ctxt, xi);
        return NULL;
    }

    return xi;
}

/*Check if the character is space.*/
static int
is_space (int c)
{
    switch (c) {
    case 0x20:
    case 0x9:
    case 0xd:
    case 0xa:
        return OX_TRUE;
    default:
        return OX_FALSE;
    }
}

/*Check if the character is a name start character.*/
static int
is_name_start (int c)
{
    if ((c == ':')
            || (c == '_')
            || ((c >= 'A') && (c <= 'Z'))
            || ((c >= 'a') && (c <= 'z'))
            || ((c >= 0xc0) && (c <= 0xd6))
            || ((c >= 0xd8) && (c <= 0xf6))
            || ((c >= 0xf8) && (c <= 0x2ff))
            || ((c >= 0x370) && (c <= 0x37d))
            || ((c >= 0x37f) && (c <= 0x1fff))
            || ((c >= 0x200c) && (c <= 0x200d))
            || ((c >= 0x2070) && (c <= 0x218f))
            || ((c >= 0x2c00) && (c <= 0x2fef))
            || ((c >= 0x3001) && (c <= 0xd7ff))
            || ((c >= 0xf900) && (c <= 0xfdcf))
            || ((c >= 0xfdf0) && (c <= 0xfffd))
            || ((c >= 0x10000) && (c <= 0xeffff)))
        return OX_TRUE;

    return OX_FALSE;
}

/*Check if the character is a name character.*/
static int
is_name (int c)
{
    if (is_name_start(c))
        return OX_TRUE;

    if ((c == '-')
            || (c == '.')
            || ((c >= '0') && (c <= '9'))
            || (c == 0xb7)
            || ((c >= 0x300) && (c <= 0x36f))
            || ((c >= 0x203f) && (c <= 0x2040)))
        return OX_TRUE;

    return OX_FALSE;
}

/*Get a character from the XML input.*/
static int
xml_input_get_char_inner (OX_Context *ctxt, OX_XmlInput *xi)
{
    OX_EntityInput *ei;
    OX_Input *input;
    int c;

    while (1) {
        if (xi->entities)
            input = xi->entities->input;
        else
            input = xi->input;

        c = ox_input_get_char(ctxt, input);
        if (c == OX_INPUT_END) {
            if (!xi->entities)
                break;

            ei = xi->entities;
            xi->entities = ei->bot;
            entity_input_free(ctxt, ei);
        } else {
            break;
        }
    }

    return c;
}

/*Push back a character to the XML input.*/
static void
xml_input_unget_char (OX_Context *ctxt, OX_XmlInput *xi, int c)
{
    if (c >= 0) {
        OX_Input *input;

        if (xi->entities)
            input = xi->entities->input;
        else
            input = xi->input;

        ox_input_unget_char(ctxt, input, c);
    }
}

/*Append a character to the buffer.*/
static OX_Result
cb_append (OX_Context *ctxt, OX_CharBuffer *cb, int c)
{
    char chars[8];
    int len;

    len = ox_uc_to_utf8(c, chars);

    return ox_char_buffer_append_chars(ctxt, cb, chars, len);
}

/*Get parsed entity.*/
static OX_Result
xml_input_get_parsed_entity (OX_Context *ctxt, OX_XmlInput *xi, int c)
{
    size_t start = xi->text.len, end;
    OX_VS_PUSH_3(ctxt, name, ev, input)
    OX_EntityInput *ei = NULL;
    OX_Result r;

    /*Get the entity.*/
    if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
        goto end;

    while (1) {
        c = xml_input_get_char_inner(ctxt, xi);
        if (!is_name(c)) {
            if (c != ';') {
                r = ox_throw_null_error(ctxt, OX_TEXT("expect `%s\' here"), ";");
                goto end;
            }
            break;
        }

        if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
            goto end;
    }

    end = xi->text.len;
    if ((r = ox_string_from_chars(ctxt, name, xi->text.items + start, end - start)) == OX_ERR)
        goto end;

    /*Resolve the entity.*/
    if ((r = ox_call(ctxt, &xi->entity_cb, ox_value_null(ctxt), name, 1, ev)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, ev)) {
        r = ox_throw_null_error(ctxt, OX_TEXT("cannot resolve entity \"%s\""),
                ox_string_get_char_star(ctxt, name));
        goto end;
    }

    /*Push a new entitiy input.*/
    if (!OX_NEW(ctxt, ei)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if ((r = ox_string_input_new(ctxt, input, ev)) == OX_ERR) {
        OX_DEL(ctxt, ei);
        goto end;
    }

    ei->input = ox_value_get_gco(ctxt, input);
    ei->bot = xi->entities;
    ox_value_copy(ctxt, &ei->inputv, input);
    xi->entities = ei;

    r = OX_OK;
end:
    xi->text.len = start;
    OX_VS_POP(ctxt, name)
    return r;
}

/*Get a character from the XML input.*/
static int
xml_input_get_char (OX_Context *ctxt, OX_XmlInput *xi)
{
    int c;
    OX_Result r;

    if (xi->resolve_pe) {
        while (1) {
            c = xml_input_get_char_inner(ctxt, xi);
            if (c == '%') {
                c = xml_input_get_char_inner(ctxt, xi);
                if (is_name_start(c)) {
                    if ((r = xml_input_get_parsed_entity(ctxt, xi, c)) == OX_ERR) {
                        c = XML_INPUT_ERR;
                        break;
                    }
                } else {
                    xml_input_unget_char(ctxt, xi, c);
                    c = '%';
                    break;
                }
            } else {
                break;
            }
        }
    } else {
        c = xml_input_get_char_inner(ctxt, xi);
    }

    return c;
}

/*Get current location of the XML input.*/
static void
xml_input_get_loc (OX_XmlInput *xi, int *line, int *col)
{
    OX_Input *input;

    if (xi->entities)
        input = xi->entities->input;
    else
        input = xi->input;

    ox_input_get_loc(input, line, col);
}

/*Get the current location.*/
static void
get_loc (OX_XmlInput *xi, OX_Location *loc)
{
    xml_input_get_loc(xi, &loc->first_line, &loc->first_column);
    xml_input_get_loc(xi, &loc->last_line, &loc->last_column);
}

/*Create a new location object.*/
static OX_Result
new_loc (OX_Context *ctxt, OX_Value *v, OX_XmlInput *xi, OX_Location *loc)
{
    OX_VS_PUSH(ctxt, n)
    OX_Input *input;
    OX_Result r;

    if ((r = ox_object_new(ctxt, v, NULL)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, n, loc->first_line);
    if ((r = ox_set(ctxt, v, OX_STRING(ctxt, first_line), n)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, n, loc->first_column);
    if ((r = ox_set(ctxt, v, OX_STRING(ctxt, first_column), n)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, n, loc->last_line);
    if ((r = ox_set(ctxt, v, OX_STRING(ctxt, last_line), n)) == OX_ERR)
        goto end;

    ox_value_set_number(ctxt, n, loc->last_column);
    if ((r = ox_set(ctxt, v, OX_STRING(ctxt, last_column), n)) == OX_ERR)
        goto end;

    if (xi->entities)
        input = xi->entities->input;
    else
        input = xi->input;

    if ((r = ox_string_from_char_star(ctxt, n, input->name)) == OX_ERR)
        goto end;
    if ((r = ox_set(ctxt, v, OX_STRING(ctxt, file), n)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, n)
    return r;
}

/*XmlInput.from_file*/
static OX_Result
XmlInput_from_file (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *cb_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, fn)
    const char *fn_cstr;
    OX_XmlInput *xi;
    OX_Result r;

    if ((r = ox_to_string(ctxt, fn_arg, fn)) == OX_ERR)
        goto end;

    if (!(xi = xml_input_new(ctxt, rv, f))) {
        r = OX_ERR;
        goto end;
    }

    fn_cstr = ox_string_get_char_star(ctxt, fn);
    if ((r = ox_file_input_new(ctxt, &xi->inputv, fn_cstr)) == OX_ERR)
        goto end;

    xi->input = ox_value_get_gco(ctxt, &xi->inputv);
    ox_value_copy(ctxt, &xi->entity_cb, cb_arg);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*XmlInput.from_str*/
static OX_Result
XmlInput_from_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *cb_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, s)
    OX_XmlInput *xi;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    if (!(xi = xml_input_new(ctxt, rv, f))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_string_input_new(ctxt, &xi->inputv, s)) == OX_ERR)
        goto end;

    xi->input = ox_value_get_gco(ctxt, &xi->inputv);
    ox_value_copy(ctxt, &xi->entity_cb, cb_arg);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*XmlInput.is_nm_token*/
static OX_Result
XmlInput_is_nm_token (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *c, *end;
    size_t len;
    OX_Bool b;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    c = ox_string_get_char_star(ctxt, s);
    len = ox_string_length(ctxt, s);

    if (len == 0) {
        b = OX_FALSE;
    } else {
        b = OX_TRUE;
        end = c + len;
        while (c < end) {
            if (!is_name(*c)) {
                b = OX_FALSE;
                break;
            }

            c ++;
        }
    }

    ox_value_set_bool(ctxt, rv, b);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*XmlInput.$inf.eatup_space*/
static OX_Result
XmlInput_inf_eatup_space (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR)
            return OX_ERR;
        if (!is_space(c)) {
            xml_input_unget_char(ctxt, xi, c);
            break;
        }
    }

    return OX_OK;
}

/*XmlInput.$inf.get_char*/
static OX_Result
XmlInput_inf_get_char (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *inner_arg = ox_argument(ctxt, args, argc, 0);
    OX_Bool inner = ox_to_bool(ctxt, inner_arg);
    OX_XmlInput *xi;
    int c;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    if (inner) {
        c = xml_input_get_char_inner(ctxt, xi);
    } else {
        c = xml_input_get_char(ctxt, xi);
    }

    if (c == XML_INPUT_ERR)
        return OX_ERR;

    ox_value_set_number(ctxt, rv, c);
    return OX_OK;
}

/*XmlInput.$inf.get_char_ns*/
static OX_Result
XmlInput_inf_get_char_ns (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *inner_arg = ox_argument(ctxt, args, argc, 0);
    OX_Bool inner = ox_to_bool(ctxt, inner_arg);
    OX_XmlInput *xi;
    int c;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    while (1) {
        if (inner) {
            c = xml_input_get_char_inner(ctxt, xi);
        } else {
            c = xml_input_get_char(ctxt, xi);
        }

        if (c == XML_INPUT_ERR)
            return OX_ERR;

        if (!is_space(c))
            break;
    }

    ox_value_set_number(ctxt, rv, c);
    return OX_OK;
}

/*XmlInput.$inf.unget_char*/
static OX_Result
XmlInput_inf_unget_char (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *c_arg = ox_argument(ctxt, args, argc, 0);
    OX_XmlInput *xi;
    OX_Result r;
    int32_t c;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    if ((r = ox_to_int32(ctxt, c_arg, &c)) == OX_ERR)
        return r;

    xml_input_unget_char(ctxt, xi, c);

    return OX_OK;
}

/*XmlInput.$inf.get_loc*/
static OX_Result
XmlInput_inf_get_loc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    OX_Location loc;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    get_loc(xi, &loc);

    if ((r = new_loc(ctxt, rv, xi, &loc)) == OX_ERR)
        return r;

    return OX_OK;
}

/*XmlInput.$inf.get_text_loc*/
static OX_Result
XmlInput_inf_get_text_loc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops)))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));

    if ((r = new_loc(ctxt, rv, xi, &xi->text_loc)) == OX_ERR)
        return r;

    return OX_OK;
}

/*XmlInput.$inf.get_name*/
static OX_Result
XmlInput_inf_get_name (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    xi->text.len = 0;
    if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
        r = OX_ERR;
        goto end;
    }
    if (!is_name_start(c)) {
        get_loc(xi, &xi->text_loc);
        r = ox_throw_syntax_error(ctxt, OX_TEXT("expect a name start character here"));
        goto end;
    }

    if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
        goto end;

    xml_input_get_loc(xi, &xi->text_loc.first_line, &xi->text_loc.first_column);

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }
        if (!is_name(c)) {
            xml_input_unget_char(ctxt, xi, c);
            xml_input_get_loc(xi, &xi->text_loc.last_line, &xi->text_loc.last_column);
            break;
        }

        if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &xi->text, rv);
end:
    return r;
}

/*XmlInput.$inf.get_nm_token*/
static OX_Result
XmlInput_inf_get_nm_token (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    xi->text.len = 0;
    if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
        r = OX_ERR;
        goto end;
    }
    if (!is_name(c)) {
        get_loc(xi, &xi->text_loc);
        r = ox_throw_syntax_error(ctxt, OX_TEXT("expect a name character here"));
        goto end;
    }

    if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
        goto end;

    xml_input_get_loc(xi, &xi->text_loc.first_line, &xi->text_loc.first_column);

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }
        if (!is_name(c)) {
            xml_input_unget_char(ctxt, xi, c);
            xml_input_get_loc(xi, &xi->text_loc.last_line, &xi->text_loc.last_column);
            break;
        }

        if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &xi->text, rv);
end:
    return r;
}

/*Get reference.*/
static OX_Result
get_ref (OX_Context *ctxt, OX_XmlInput *xi)
{
    int c;
    size_t s;
    OX_Result r;
    OX_Location loc;
    const char *text;
    enum {
        TYPE_HEX,
        TYPE_DEC,
        TYPE_NAME
    } type;
    OX_VS_PUSH_3(ctxt, iv, ov, sv)

    s = xi->text.len;

    xml_input_get_loc(xi, &loc.first_line, &loc.first_column);

    if ((c = xml_input_get_char(ctxt, xi)) == OX_ERR) {
        r = OX_ERR;
        goto end;
    }

    if (c == '#') {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }
        if (c == 'x') {
            while (1) {
                if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
                    r = OX_ERR;
                    goto end;
                }

                if (!ox_char_is_hex(c))
                    break;

                if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                    goto end;
            }

            type = TYPE_HEX;
        } else {
            xml_input_unget_char(ctxt, xi, c);

            while (1) {
                if ((c = xml_input_get_char(ctxt, xi)) == OX_ERR) {
                    r = OX_ERR;
                    goto end;
                }

                if (!ox_char_is_digit(c))
                    break;

                if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                    goto end;
            }

            type = TYPE_DEC;
        }
    } else {
        if (!is_name_start(c)) {
            get_loc(xi, &xi->text_loc);
            r = ox_throw_syntax_error(ctxt, OX_TEXT("expect a name start character here"));
            goto end;
        }

        if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
            goto end;

        while (1) {
            if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
                r = OX_ERR;
                goto end;
            }

            if (!is_name(c))
                break;

            if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                goto end;
        }

        type = TYPE_NAME;
    }

    if (c != ';') {
        get_loc(xi, &xi->text_loc);
        r = ox_throw_syntax_error(ctxt, OX_TEXT("expect `%s\' here"), ";");
        goto end;
    }

    xml_input_get_loc(xi, &loc.last_line, &loc.last_column);

    if (!(text = ox_char_buffer_get_char_star(ctxt, &xi->text))) {
        r = OX_ERR;
        goto end;
    }
    text += s;

    if (type == TYPE_NAME) {
        if ((r = ox_string_from_char_star(ctxt, iv, text)) == OX_ERR)
            goto end;

        xi->text.len = s;

        if ((r = ox_call(ctxt, &xi->entity_cb, ox_value_null(ctxt), iv, 1, ov)) == OX_ERR)
            goto end;

        if (ox_value_is_null(ctxt, ov)) {
            get_loc(xi, &xi->text_loc);
            xi->text_loc.first_column -= ox_string_length(ctxt, iv) + 1;

            r = ox_throw_syntax_error(ctxt, OX_TEXT("cannot resolve entity \"%s\""), text);
            goto end;
        } else {
            if ((r = ox_to_string(ctxt, ov, sv)) == OX_ERR)
                goto end;
            if ((r = ox_char_buffer_append_string(ctxt, &xi->text, sv)) == OX_ERR)
                goto end;
        }
    } else {
        int base = (type == TYPE_HEX) ? 16 : 10;
        long l;

        l = strtol(text, NULL, base);
        if (l > 0x10ffff) {
            xi->text_loc = loc;
            r = ox_throw_range_error(ctxt, OX_TEXT("illegal unicode"));
            goto end;
        }

        xi->text.len = s;

        if ((r = cb_append(ctxt, &xi->text, l)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, iv)
    return r;
}

/*XmlInput.$inf.get_str*/
static OX_Result
XmlInput_inf_get_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c, sc;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    if ((sc = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
        r = OX_ERR;
        goto end;
    }

    if ((sc != '\'') && (sc != '\"')) {
        get_loc(xi, &xi->text_loc);
        r = ox_throw_syntax_error(ctxt, OX_TEXT("expect `%s\' or `%s\' here"), "\'", "\"");
        goto end;
    }

    xi->text.len = 0;
    xml_input_get_loc(xi, &xi->text_loc.first_line, &xi->text_loc.first_column);

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }

        if (c == sc) {
            xml_input_get_loc(xi, &xi->text_loc.last_line, &xi->text_loc.last_column);
            break;
        } else if (c == OX_INPUT_END) {
            get_loc(xi, &xi->text_loc);
            r = ox_throw_syntax_error(ctxt, OX_TEXT("expect `%c\' at end of string"), sc);
            goto end;
        } else if (c == '&') {
            if ((r = get_ref(ctxt, xi)) == OX_ERR)
                goto end;
        } else {
            if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                goto end;
        }
    }

    r = ox_char_buffer_get_string(ctxt, &xi->text, rv);
end:
    return r;
}

/*XmlInput.$inf.get_chars*/
static OX_Result
XmlInput_inf_get_chars (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    xi->text.len = 0;

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }

        if ((c == '<') || (c == OX_INPUT_END)) {
            xml_input_unget_char(ctxt, xi, c);
            break;
        } else if (c == '&') {
            if ((r = get_ref(ctxt, xi)) == OX_ERR)
                goto end;
        } else {
            if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                goto end;
        }
    }

    if (xi->text.len == 0) {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
    } else {
        r = ox_char_buffer_get_string(ctxt, &xi->text, rv);
    }
end:
    return r;
}

/*XmlInput.$inf.get_pi_data*/
static OX_Result
XmlInput_inf_get_pi_data (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    int c;
    OX_Bool empty = OX_TRUE;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    xi->text.len = 0;

    while (1) {
        if ((c = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
            r = OX_ERR;
            goto end;
        }

        if (c == '?') {
            int nc;

            if ((nc = xml_input_get_char(ctxt, xi)) == XML_INPUT_ERR) {
                r = OX_ERR;
                goto end;
            }

            if (nc == '>')
                break;

            xml_input_unget_char(ctxt, xi, nc);
        } else if (c == OX_INPUT_END) {
            r = ox_throw_type_error(ctxt, OX_TEXT("expect `%s\' here"), "?>");
            goto end;
        }

        if (empty) {
            if (!is_space(c))
                empty = OX_FALSE;
        }

        if (!empty) {
            if ((r = cb_append(ctxt, &xi->text, c)) == OX_ERR)
                goto end;
        }
    }

    r = ox_char_buffer_get_string(ctxt, &xi->text, rv);
end:
    return r;
}

/*XmlInput.$inf.eatup_chars*/
static OX_Result
XmlInput_inf_eatup_chars (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *end_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, end)
    const char *ecstr;
    size_t elen;
    OX_XmlInput *xi;
    int c;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    if ((r = ox_to_string(ctxt, end_arg, end)) == OX_ERR)
        goto end;

    ecstr = ox_string_get_char_star(ctxt, end);
    elen = ox_string_length(ctxt, end);

    while (1) {
        int n;

        for (n = 0; n < elen; n ++) {
            if ((c = xml_input_get_char_inner(ctxt, xi)) == XML_INPUT_ERR) {
                r = OX_ERR;
                goto end;
            }

            if (c != ecstr[n])
                break;
        }

        if (n == elen)
            break;

        while (n > 1) {
            xml_input_unget_char(ctxt, xi, ecstr[n - 1]);
            n --;
        }

        if (c == OX_INPUT_END) {
            r = ox_throw_type_error(ctxt, OX_TEXT("expect `%s\' here"), ecstr);
            goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, end)
    return r;
}

/*XmlInput.$inf.set_resolve_pe*/
static OX_Result
XmlInput_inf_set_resolve_pe (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *resolve_arg = ox_argument(ctxt, args, argc, 0);
    OX_XmlInput *xi;
    OX_Bool b;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    b = ox_to_bool(ctxt, resolve_arg);
    xi->resolve_pe = b;

    r = OX_OK;
end:
    return r;
}

/*XmlInput.$inf.$close*/
static OX_Result
XmlInput_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_XmlInput *xi;
    OX_EntityInput *ei;
    OX_Result r;

    if (!(xi = ox_object_get_priv(ctxt, thiz, &xml_input_ops))) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a XML input"));
        goto end;
    }

    while (xi->entities) {
        ei = xi->entities;
        xi->entities = ei->bot;

        ox_input_close(ctxt, &ei->inputv);
        entity_input_free(ctxt, ei);
    }

    ox_input_close(ctxt, &xi->inputv);
    r = OX_OK;
end:
    return r;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*XML.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "XmlInput"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_XmlInput, c));
    ox_not_error(ox_script_set_value(ctxt, s, ID_XmlInput_inf, inf));

    /*static methods.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "from_file", XmlInput_from_file));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "from_str", XmlInput_from_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "is_nm_token", XmlInput_is_nm_token));

    /*Methods.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "eatup_space", XmlInput_inf_eatup_space));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_char", XmlInput_inf_get_char));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_char_ns", XmlInput_inf_get_char_ns));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "unget_char", XmlInput_inf_unget_char));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_loc", XmlInput_inf_get_loc));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_text_loc", XmlInput_inf_get_text_loc));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_name", XmlInput_inf_get_name));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_nm_token", XmlInput_inf_get_nm_token));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_str", XmlInput_inf_get_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_chars", XmlInput_inf_get_chars));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_pi_data", XmlInput_inf_get_pi_data));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "eatup_chars", XmlInput_inf_eatup_chars));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "set_resolve_pe", XmlInput_inf_set_resolve_pe));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$close", XmlInput_inf_close));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}