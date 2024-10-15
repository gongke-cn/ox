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
 * Get text from C file.
 */

#define OX_LOG_TAG "c_parser"

#include "gettext.h"

/*Declaration index.*/
enum {
    ID_c_parse,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "c_parse",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Token type.*/
typedef enum {
    OX_GT_TOKEN_END = -1,   /**< Input end.*/
    OX_GT_TOKEN_ID  = 256,  /**< Identifier.*/
    OX_GT_TOKEN_STRING      /**< String.*/
} OX_GtTokenType;

/** Parse context.*/
typedef struct {
    OX_CharBuffer  text;     /**< Text buffer.*/
    OX_CharBuffer  sb;       /**< String buffer.*/
    OX_Value      *cb;       /**< Callback function.*/
    OX_Input      *input;    /**< The input.*/
    OX_Value      *v;        /**< The value of the token.*/
    int            line;     /**< The first character's line number of current token.*/
    OX_GtTokenType tok_type; /**< The current token type.*/
} OX_GtParseCtxt;

/*Add a string to the set.*/
static OX_Result
add_string (OX_Context *ctxt, OX_Value *cb, OX_Value *s, int l)
{
    OX_VS_PUSH_3(ctxt, msg, line, rv)
    OX_Result r;

    ox_value_copy(ctxt, msg, s);
    ox_value_set_number(ctxt, line, l);

    r = ox_call(ctxt, cb, ox_value_null(ctxt), msg, 2, rv);

    OX_VS_POP(ctxt, msg)
    return r;
}

/*Get a token from the input.*/
static OX_GtTokenType
get_token (OX_Context *ctxt, OX_GtParseCtxt *pc)
{
    int c;

    if (pc->tok_type != OX_GT_TOKEN_END) {
        OX_GtTokenType tt = pc->tok_type;

        pc->tok_type = OX_GT_TOKEN_END;

        return tt;
    }

    while (1) {
        c = ox_input_get_char(ctxt, pc->input);
        if (c == OX_INPUT_END)
            return OX_GT_TOKEN_END;

        if (c == '/') {
            int nc = ox_input_get_char(ctxt, pc->input);

            if (nc == '/') {
                while (1) {
                    c = ox_input_get_char(ctxt, pc->input);
                    if ((c == OX_INPUT_END) || (c == '\n'))
                        break;
                }
            } else if (nc == '*') {
                while (1) {
                    c = ox_input_get_char(ctxt, pc->input);
                    if (c == OX_INPUT_END)
                        break;
                    if (c == '*') {
                        nc = ox_input_get_char(ctxt, pc->input);
                        if (nc == '/')
                            break;

                        ox_input_unget_char(ctxt, pc->input, nc);
                    }
                }
            } else {
                ox_input_unget_char(ctxt, pc->input, nc);
                break;
            }
        } else if (!ox_char_is_space(c)) {
            break;
        }
    }

    pc->line = pc->input->line;

    if (c == '\"') {
        pc->text.len = 0;

        while (1) {
            c = ox_input_get_char(ctxt, pc->input);
            if ((c == '\"') || (c == '\n') || (c == OX_INPUT_END))
                break;

            if (c == '\\') {
                int nc = ox_input_get_char(ctxt, pc->input);
                int i;

                switch (nc) {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'x':
                    c = 0;
                    for (i = 0; i < 2; i ++) {
                        nc = ox_input_get_char(ctxt, pc->input);
                        if (!ox_char_is_hex(nc)) {
                            ox_input_unget_char(ctxt, pc->input, nc);
                            c = -1;
                            break;
                        }

                        c <<= 4;
                        c |= ox_hex_char_to_num(nc);
                    }
                    break;
                case 'u':
                    c = 0;
                    for (i = 0; i < 4; i ++) {
                        nc = ox_input_get_char(ctxt, pc->input);
                        if (!ox_char_is_hex(nc)) {
                            ox_input_unget_char(ctxt, pc->input, nc);
                            c = -1;
                            break;
                        }

                        c <<= 4;
                        c |= ox_hex_char_to_num(nc);
                    }
                    break;
                default:
                    if ((nc >= '0') && (nc <= '7')) {
                        int c2, c3;

                        c2 = ox_input_get_char(ctxt, pc->input);
                        c3 = ox_input_get_char(ctxt, pc->input);

                        if ((c2 >= '0') && (c2 <= '7') && (c3 >= '0') && (c3 <= '7')) {
                            c = ((nc - '0') << 6) | ((c2 - '0') << 3) | (c3 - '0');
                        } else {
                            ox_input_unget_char(ctxt, pc->input, c3);
                            ox_input_unget_char(ctxt, pc->input, c2);

                            if (nc == '0')
                                c = 0;
                            else
                                c = nc;
                        }
                    } else {
                        c = nc;
                    }
                    break;
                }
            }

            if (c != -1)
                ox_not_error(ox_char_buffer_append_char(ctxt, &pc->text, c));
        }

        ox_not_error(ox_string_from_chars(ctxt, pc->v, pc->text.items, pc->text.len));
        return OX_GT_TOKEN_STRING;
    } else if (ox_char_is_alpha(c) || (c == '_')) {
        pc->text.len = 0;

        ox_not_error(ox_char_buffer_append_char(ctxt, &pc->text, c));

        while (1) {
            c = ox_input_get_char(ctxt, pc->input);
            if (!ox_char_is_alnum(c) && (c != '_')) {
                ox_input_unget_char(ctxt, pc->input, c);
                break;
            }

            ox_not_error(ox_char_buffer_append_char(ctxt, &pc->text, c));
        }

        ox_not_error(ox_string_from_chars(ctxt, pc->v, pc->text.items, pc->text.len));
        return OX_GT_TOKEN_ID;
    }

    return c;
}

/*Push back a token.*/
static void
unget_token (OX_Context *ctxt, OX_GtParseCtxt *pc, OX_GtTokenType tt)
{
    pc->tok_type = tt;
}

/*Parse the C file.*/
static OX_Result
parse (OX_Context *ctxt, OX_GtParseCtxt *pc)
{
    OX_GtTokenType tt;
    int line;

    while (1) {
        tt = get_token(ctxt, pc);
        if (tt == OX_GT_TOKEN_END)
            break;

        if (tt == OX_GT_TOKEN_ID) {
            const char *cstr = ox_string_get_char_star(ctxt, pc->v);
            if (!strcmp(cstr, "_") || !strcmp(cstr, "OX_TEXT")) {
                tt = get_token(ctxt, pc);
                if (tt != '(') {
                    unget_token(ctxt, pc, tt);
                    continue;
                }

                pc->sb.len = 0;

                tt = get_token(ctxt, pc);
                if (tt != OX_GT_TOKEN_STRING) {
                    unget_token(ctxt, pc, tt);
                    continue;
                }

                line = pc->line;

                ox_not_error(ox_char_buffer_append_string(ctxt, &pc->sb, pc->v));

                while (1) {
                    tt = get_token(ctxt, pc);
                    if (tt != OX_GT_TOKEN_STRING)
                        break;

                    ox_not_error(ox_char_buffer_append_string(ctxt, &pc->sb, pc->v));
                }

                if (tt != ')') {
                    unget_token(ctxt, pc, tt);
                    continue;
                }

                ox_not_error(ox_char_buffer_get_string(ctxt, &pc->sb, pc->v));
                ox_not_error(add_string(ctxt, pc->cb, pc->v, line));
            }
        }
    }

    return OX_OK;
}

/*c_parse*/
static OX_Result
gettext_c_parse (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *cb = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, fn_str, input)
    const char *fn;
    OX_GtParseCtxt pc;
    OX_Result r;

    if ((r = ox_to_string(ctxt, fn_arg, fn_str)) == OX_ERR)
        goto end;

    fn = ox_string_get_char_star(ctxt, fn_str);

    if ((r = ox_file_input_new(ctxt, input, fn)) == OX_ERR)
        goto end;

    pc.v = ox_value_stack_push(ctxt);
    pc.input = ox_value_get_gco(ctxt, input);
    pc.cb = cb;
    pc.tok_type = OX_GT_TOKEN_END;

    ox_char_buffer_init(&pc.text);
    ox_char_buffer_init(&pc.sb);

    r = parse(ctxt, &pc);

    ox_char_buffer_deinit(ctxt, &pc.text);
    ox_char_buffer_deinit(ctxt, &pc.sb);

    r = OX_OK;
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, fn_str)
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
    OX_VS_PUSH(ctxt, v)

    ox_not_error(ox_named_native_func_new_s(ctxt, v, gettext_c_parse, NULL, "c_parse"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_c_parse, v));

    OX_VS_POP(ctxt, v)
    return OX_OK;
}