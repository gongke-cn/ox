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
 * Lexical analyzer.
 */

#define OX_LOG_TAG "ox_lex"

#include "ox_internal.h"

/*Get a character from the lexical analyzer.*/
static inline int
get_char (OX_Context *ctxt, OX_Lex *lex)
{
    return ox_input_get_char(ctxt, lex->input);
}

/*Push back a character to the lexical analyzer.*/
static inline void
unget_char (OX_Context *ctxt, OX_Lex *lex, int c)
{
    ox_input_unget_char(ctxt, lex->input, c);
}

/*Clear the text buffer.*/
static inline void
text_clear (OX_Context *ctxt, OX_Lex *lex)
{
    lex->text.len = 0;
}

/*Add a character to the text buffer.*/
static void
text_add_char (OX_Context *ctxt, OX_Lex *lex, int c)
{
    char buf[4];
    int n;

    assert(c >= 0);

    n = ox_uc_to_utf8(c, buf);
    assert(n > 0);

    ox_not_error(ox_char_buffer_append_chars(ctxt, &lex->text, buf, n));
}

/*Add characters to the text buffer.*/
static void
text_add_chars (OX_Context *ctxt, OX_Lex *lex, const char *c, size_t n)
{
    ox_not_error(ox_char_buffer_append_chars(ctxt, &lex->text, c, n));
}

/*Get the 0 termintated string from the text buffer.*/
static const char*
text_get (OX_Context *ctxt, OX_Lex *lex)
{
    return ox_not_null(ox_char_buffer_get_char_star(ctxt, &lex->text));
}

/*Store the first character's location.*/
static inline void
store_first_loc (OX_Lex *lex, OX_Location *loc)
{
    ox_input_get_loc(lex->input, &loc->first_line, &loc->first_column);
}

/*Store the last character's location.*/
static inline void
store_last_loc (OX_Lex *lex, OX_Location *loc)
{
    ox_input_get_loc(lex->input, &loc->last_line, &loc->last_column);
}

/*Show prompt message.*/
static void
prompt_v (OX_Context *ctxt, OX_Lex *lex, OX_Location *loc, OX_PromptType type, const char *fmt, va_list ap)
{
    OX_Location loc_buf;

    if (lex->status & OX_LEX_ST_NO_PROMPT)
        return;

    if (!loc) {
        loc = &loc_buf;

        store_first_loc(lex, loc);
        store_last_loc(lex, loc);
    }

    if (type == OX_PROMPT_ERROR)
        lex->status |= OX_LEX_ST_ERR;

    ox_prompt_v(ctxt, lex->input, loc, type, fmt, ap);
}

/*Show error prompt message.*/
static void
error (OX_Context *ctxt, OX_Lex *lex, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    prompt_v(ctxt, lex, loc, OX_PROMPT_ERROR, fmt, ap);

    va_end(ap);
}

/*Show expect digit character error prompt.*/
static void
expect_digit_error (OX_Context *ctxt, OX_Lex *lex)
{
    error(ctxt, lex, NULL, OX_TEXT("expect a hexadecimal character here"));
}

/*Show expect hex character error prompt.*/
static void
expect_hex_error (OX_Context *ctxt, OX_Lex *lex)
{
    error(ctxt, lex, NULL, OX_TEXT("expect a hexadecimal character here"));
}

/*Get comment line.*/
static void
comment_line (OX_Context *ctxt, OX_Lex *lex)
{
    int c;

    while (1) {
        c = get_char(ctxt, lex);
        if (c == '\n')
            break;
        if (c == OX_INPUT_END)
            break;
    }
}

/*Get comment block.*/
static void
comment_block (OX_Context *ctxt, OX_Lex *lex)
{
    int c, nc;

    while (1) {
        c = get_char(ctxt, lex);
        if (c == '*') {
            nc = get_char(ctxt, lex);
            if (nc == '/')
                break;
            
            unget_char(ctxt, lex, nc);
        } else if (c == OX_INPUT_END) {
            error(ctxt, lex, NULL, OX_TEXT("expect `*/\' at end of comment block"));
            break;
        }
    }
}

/*Get a number token.*/
static void
number_literal (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok)
{
    int c, nc;
    OX_Bool loop;
    OX_Bool is_float = OX_FALSE;
    size_t old_len;
    const char *text;
    int base = 10;

    tok->type = OX_TOKEN_NUMBER;
    ox_value_set_number(ctxt, tok->v, 0);

    c = get_char(ctxt, lex);

    if (c == '0') {
        nc = get_char(ctxt, lex);

        switch (nc) {
        case 'b':
        case 'B':
            base = 2;
            break;
        case 'o':
        case 'O':
            base = 8;
            break;
        case 'x':
        case 'X':
            base = 16;
            break;
        default:
            unget_char(ctxt, lex, nc);
            unget_char(ctxt, lex, c);
        }
    } else {
        unget_char(ctxt, lex, c);
    }

    text_clear(ctxt, lex);

    loop = OX_TRUE;
    old_len = lex->text.len;
    do {
        c = get_char(ctxt, lex);

        if (c == '_')
            continue;

        switch (base) {
        case 2:
            if ((c != '0') && (c != '1'))
                loop = OX_FALSE;
            break;
        case 8:
            if ((c < '0') || (c > '7'))
                loop = OX_FALSE;
            break;
        case 10:
            if (!ox_char_is_digit(c))
                loop = OX_FALSE;
            break;
        case 16:
            if (!ox_char_is_hex(c))
                loop = OX_FALSE;
            break;
        default:
            assert(0);
        }

        if (loop)
            text_add_char(ctxt, lex, c);
    } while (loop);

    if (lex->text.len == old_len) {
        if (base == 16)
            expect_hex_error(ctxt, lex);
        else
            expect_digit_error(ctxt, lex);
        return;
    }

    if (base == 10) {
        if (c == '.') {
            nc = get_char(ctxt, lex);
            if (!ox_char_is_digit(nc)) {
                unget_char(ctxt, lex, nc);
                goto text_ok;
            } else {
                unget_char(ctxt, lex, nc);
            }

            text_add_char(ctxt, lex, c);

            while (1) {
                c = get_char(ctxt, lex);

                if (c == '_')
                    continue;

                if (!ox_char_is_digit(c))
                    break;

                text_add_char(ctxt, lex, c);
            }

            is_float = OX_TRUE;
        }

        if ((c == 'e') || (c == 'E')) {
            text_add_char(ctxt, lex, c);

            c = get_char(ctxt, lex);
            if ((c == '+') || (c == '-')) {
                text_add_char(ctxt, lex, c);
            } else {
                unget_char(ctxt, lex, c);
            }

            old_len = lex->text.len;
            while (1) {
                c = get_char(ctxt, lex);

                if (c == '_')
                    continue;

                if (!ox_char_is_digit(c))
                    break;

                text_add_char(ctxt, lex, c);
            }

            if (lex->text.len == old_len) {
                expect_digit_error(ctxt, lex);
                return;
            }

            is_float = OX_TRUE;
        }
    }
text_ok:
    unget_char(ctxt, lex, c);

    text = text_get(ctxt, lex);

    if (is_float) {
        double d;

        d = strtod(text, NULL);
        if ((errno = EOVERFLOW) && (d == HUGE_VAL)) {
            store_last_loc(lex, &tok->loc);
            error(ctxt, lex, &tok->loc, OX_TEXT("number value overflow"));
            return;
        }

        ox_value_set_number(ctxt, tok->v, d);
    } else {
        long long i;

        i = strtoll(text, NULL, base);
        if ((errno == EOVERFLOW) && (i == LLONG_MAX)) {
            store_last_loc(lex, &tok->loc);
            error(ctxt, lex, &tok->loc, OX_TEXT("number value overflow"));
            return;
        }

        ox_value_set_number(ctxt, tok->v, i);
    }
}

/*Get a hexadecimal escape character.*/
static int
escape_x_char (OX_Context *ctxt, OX_Lex *lex)
{
    int i, c, v = 0;

    for (i = 0; i < 2; i ++) {
        c = get_char(ctxt, lex);
        if (!ox_char_is_hex(c)) {
            expect_hex_error(ctxt, lex);
            return -1;
        }

        v <<= 4;
        v |= ox_hex_char_to_num(c);
    }

    return v;
}

/*Get an unicode escape character.*/
static int
escape_u_char (OX_Context *ctxt, OX_Lex *lex)
{
    int i, c, v = 0;

    c = get_char(ctxt, lex);
    if (c == '{') {
        OX_Bool overflow = OX_FALSE;
        OX_Location loc;

        store_first_loc(lex, &loc);

        i = 0;
        while (1) {
            c = get_char(ctxt, lex);
            if (c == '}') {
                if (i == 0)
                    expect_hex_error(ctxt, lex);
                break;
            }
            if (!ox_char_is_hex(c)) {
                expect_hex_error(ctxt, lex);
                break;
            }

            v <<= 4;
            v |= ox_hex_char_to_num(c);
            i ++;

            if (!overflow && (v > 0x10ffff)) {
                overflow = OX_TRUE;
                store_last_loc(lex, &loc);
                error(ctxt, lex, &loc, OX_TEXT("unicode value overflow"));
            }
        }
    } else {
        unget_char(ctxt, lex, c);

        for (i = 0; i < 4; i ++) {
            c = get_char(ctxt, lex);
            if (!ox_char_is_hex(c)) {
                expect_hex_error(ctxt, lex);
                return -1;
            }

            v <<= 4;
            v |= ox_hex_char_to_num(c);
        }

        /*Is UTF-16 surrogate leading?*/
        if (ox_char_is_utf16_leading(v)) {
            char buf[6];
            int len = 0;
            int t;

            /*Try to get UTF-16 surrogate trailing.*/
            c = get_char(ctxt, lex);
            buf[len ++] = c;
            if (c != '\\')
                goto pair_end;

            c = get_char(ctxt, lex);
            buf[len ++] = c;
            if (c != 'u')
                goto pair_end;

            c = get_char(ctxt, lex);
            buf[len ++] = c;
            if (!ox_char_is_hex(c))
                goto pair_end;

            t = ox_hex_char_to_num(c);
            for (i = 0; i < 3; i ++) {
                c = get_char(ctxt, lex);
                buf[len++] = c;
                if (!ox_char_is_hex(c))
                    goto pair_end;

                t <<= 4;
                t |= ox_hex_char_to_num(c);
            }

            if (!ox_char_is_utf16_trailing(t))
                goto pair_end;

            len = 0;
            v = ox_uc_from_utf16_surrogate(v, t);
pair_end:
            while (len) {
                len --;
                unget_char(ctxt, lex, buf[len]);
            }
        }
    }

    return v;
}

/*Get the escape character value.*/
static int
escape_char (OX_Context *ctxt, OX_Lex *lex, int c)
{
    switch (c) {
    case 'n':
        c = '\n';
        break;
    case 'r':
        c = '\r';
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
    case 'a':
        c = '\a';
        break;
    case 'b':
        c = '\b';
        break;
    case 'x':
        c = escape_x_char(ctxt, lex);
        break;
    case 'u':
        c = escape_u_char(ctxt, lex);
        break;
    default:
        if (!ox_char_is_graph(c)) {
            error(ctxt, lex, NULL, OX_TEXT("illegal escape character"));
            c = -1;
        }
        break;
    }

    return c;
}

/*Get a character literal.*/
static void
char_literal (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok)
{
    int c, nc;

    c = get_char(ctxt, lex);

    if (c == '\'') {
        error(ctxt, lex, NULL, OX_TEXT("expect a character data in `\'\' and `\'\'"));
        c = 0;
    } else if (c == '\\') {
        nc = get_char(ctxt, lex);
        c = escape_char(ctxt, lex, nc);
    }

    nc = get_char(ctxt, lex);
    if (nc != '\'') {
        error(ctxt, lex, NULL, OX_TEXT("expect a `\'\' at end of character"));
        c = 0;
    }

    tok->type = OX_TOKEN_NUMBER;
    ox_value_set_number(ctxt, tok->v, c);
}

/*Get a string literal.*/
static void
string_literal (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, int flags)
{
    int c, nc;
    const char *text;
    OX_TokenType type;

    text_clear(ctxt, lex);

    c = get_char(ctxt, lex);
    if (c == '\"')
        type = OX_TOKEN_STRING;
    else
        type = OX_TOKEN_STR_TAIL;

    while (1) {
        c = get_char(ctxt, lex);
        if (c == '\"')
            break;
        if (c == OX_INPUT_END) {
            error(ctxt, lex, NULL, OX_TEXT("expect a `\"\' at end of string"));
            break;
        }
        if (!(flags & OX_LEX_FL_NO_EMBED_EXPR) && (c == '{')) {
            type = (type == OX_TOKEN_STRING) ? OX_TOKEN_STR_HEAD : OX_TOKEN_STR_MID;
            break;
        }

        if (c == '\\') {
            nc = get_char(ctxt, lex);

            if (nc != '\n') {
                c = escape_char(ctxt, lex, nc);
                if (c >= 0)
                    text_add_char(ctxt, lex, c);
            }
        } else {
            text_add_char(ctxt, lex, c);
        }
    }

    text = text_get(ctxt, lex);

    tok->type = type;
    ox_string_from_char_star(ctxt, tok->v, text);

    if (type == OX_TOKEN_STR_HEAD) {
        OX_LexStrState ss;

        ss.type = OX_LEX_STR_DOUBLE_QUOT;
        ss.brace = lex->brace_level;

        ox_not_error(ox_vector_append(ctxt, &lex->str_stack, ss));
    } else if (type == OX_TOKEN_STR_TAIL) {
        assert(lex->str_stack.len);

        lex->str_stack.len --;
    }
}

/*Get a single quotation string literal.*/
static void
single_string_literal (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, OX_Bool is_head, int flags)
{
    int c = -1, nc;
    char *start, *end;

    text_clear(ctxt, lex);

    while (1) {
        c = get_char(ctxt, lex);

        if (c == OX_INPUT_END) {
            error(ctxt, lex, NULL, OX_TEXT("expect `\'\'\' at end of string"));
            break;
        }

        if (c == '\'') {
            nc = get_char(ctxt, lex);
            if (nc == '\'')
                break;

            unget_char(ctxt, lex, nc);
        } else if (!(flags & OX_LEX_FL_NO_EMBED_EXPR) && (c == '{')) {
            nc = get_char(ctxt, lex);
            if (nc == '{')
                break;

            unget_char(ctxt, lex, nc);
        }

        text_add_char(ctxt, lex, c);
    }

    if (is_head && (c == '\''))
        tok->type = OX_TOKEN_STRING;
    else if (is_head)
        tok->type = OX_TOKEN_STR_HEAD;
    else if (c == '\'')
        tok->type = OX_TOKEN_STR_TAIL;
    else
        tok->type = OX_TOKEN_STR_MID;

    start = lex->text.items;
    end = start + lex->text.len;

    if ((tok->type == OX_TOKEN_STRING) || (tok->type == OX_TOKEN_STR_HEAD)) {
        while (1) {
            if (!ox_char_is_space(*start) || (start == end)) {
                start = lex->text.items;
                break;
            } else if (*start == '\n') {
                start ++;
                break;
            }

            start ++;
        }
    }

    if ((tok->type == OX_TOKEN_STRING) || (tok->type == OX_TOKEN_STR_TAIL)) {
        while (1) {
            if ((start == end) || !ox_char_is_space(end[-1])) {
                end = lex->text.items + lex->text.len;
                break;
            } else if (end[-1] == '\n') {
                end --;
                break;
            }

            end --;
        }
    }

    ox_string_from_chars(ctxt, tok->v, start, end - start);

    if (tok->type == OX_TOKEN_STR_HEAD) {
        OX_LexStrState ss;

        ss.type = OX_LEX_STR_SINGLE_QUOT;
        ss.brace = lex->brace_level;

        ox_not_error(ox_vector_append(ctxt, &lex->str_stack, ss));
    } else if (tok->type == OX_TOKEN_STR_TAIL) {
        assert(lex->str_stack.len);

        lex->str_stack.len --;
    }
}

/** Keyword FSM's node.*/
typedef struct {
    int first_edge;     /**< The first edge's index.*/
    int edge_num;       /**< Number of edges of this node.*/
    OX_Keyword keyword; /**< The result keyword.*/
} OX_KeywordNode;

/** Keyword FSM's edge.*/
typedef struct {
    int c;         /**< Character.*/
    int dest_node; /**< Desctination node's index.*/
} OX_KeywordEdge;

#include "ox_keyword_table.h"

/*Check if the character is the start of identifier.*/
static OX_Bool
is_id_start (int c)
{
    return (c == '_') || (c == '$') || ox_char_is_alpha(c);
}

/*Check if the character is the continue of identifier.*/
static OX_Bool
is_id_cont (int c)
{
    return (c == '_') || (c == '$') || ox_char_is_alnum(c);
}

/*Get an identifier.*/
static void
identifier (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, OX_TokenType type)
{
    int c;
    const char *text, *pc;
    const OX_KeywordNode *n;
    int eid, last_eid;

    text_clear(ctxt, lex);

    while (1) {
        c = get_char(ctxt, lex);
        if (!is_id_cont(c)) {
            unget_char(ctxt, lex, c);
            break;
        }

        text_add_char(ctxt, lex, c);
    }

    text = text_get(ctxt, lex);

    /*Lookup the keyword.*/
    pc = text;
    n = keyword_nodes;

    do {
        eid = n->first_edge;
        last_eid = eid + n->edge_num;

        while (eid < last_eid) {
            const OX_KeywordEdge *e = &keyword_edges[eid];

            if (e->c == *pc) {
                n = &keyword_nodes[e->dest_node];
                pc ++;
                break;
            }

            eid ++;
        }
    } while (eid < last_eid);

    if ((*pc == 0) && (n->keyword != OX_KEYWORD_NONE))
        tok->keyword = n->keyword;
    else
        tok->keyword = OX_KEYWORD_NONE;

    if (type == OX_TOKEN_ID) {
        switch (tok->keyword) {
        case OX_KEYWORD_null:
            tok->type = OX_TOKEN_NULL;
            ox_value_set_null(ctxt, tok->v);
            break;
        case OX_KEYWORD_true:
            tok->type = OX_TOKEN_BOOL;
            ox_value_set_bool(ctxt, tok->v, OX_TRUE);
            break;
        case OX_KEYWORD_false:
            tok->type = OX_TOKEN_BOOL;
            ox_value_set_bool(ctxt, tok->v, OX_FALSE);
            break;
        default:
            tok->type = type;
            ox_string_from_char_star(ctxt, tok->v, text);
            break;
        }
    } else {
        tok->type = type;
        ox_string_from_char_star(ctxt, tok->v, text);
    }
}

/** Punctuation FSM's node.*/
typedef struct {
    int first_edge;     /**< The first edge's index.*/
    int edge_num;       /**< Number of edges of this node.*/
    OX_TokenType token; /**< The result token.*/
} OX_PunctNode;

/** Punctuation FSM's edge.*/
typedef struct {
    int c;         /**< Character.*/
    int dest_node; /**< Desctination node's index.*/
} OX_PunctEdge;

#include "ox_punct_table.h"

/*Get a punctuation.*/
static void
punctuation (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok)
{
    int nid;
    int buf[4];
    int len = 0;
    int c;

    c = get_char(ctxt, lex);

    tok->type = c;

    nid = 0;
    while (1) {
        const OX_PunctNode *n = &punct_nodes[nid];
        int eid, last_eid;

        if (n->token != OX_TOKEN_END) {
            len = 0;
            tok->type = n->token;
        }
        
        if (nid)
            buf[len ++] = c;

        eid = n->first_edge;
        last_eid = eid + n->edge_num;

        while (eid < last_eid) {
            const OX_PunctEdge *e = &punct_edges[eid];

            if (e->c == c) {
                nid = e->dest_node;
                c = get_char(ctxt, lex);
                break;
            }

            eid ++;
        }

        if (eid >= last_eid) {
            while (len) {
                len --;
                unget_char(ctxt, lex, buf[len]);
            }

            return;
        }
    }
}

/*String format.*/
static void
string_format (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok)
{
    int flags = 0;
    int width = OX_SOUT_WIDTH_DEFAULT, prec = OX_SOUT_PREC_DEFAULT;
    int fmt = OX_SOUT_FMT_STR;
    int n;
    int c;
    OX_Location loc;

    c = get_char(ctxt, lex);
    if (c == '-') {
        flags |= OX_SOUT_FL_ALIGN_HEAD;
        c = get_char(ctxt, lex);
    }

    if (c == '0') {
        flags |= OX_SOUT_FL_ZERO;
        c = get_char(ctxt, lex);
    }

    if (ox_char_is_digit(c)) {
        n = c - '0';

        store_first_loc(lex, &loc);

        while (1) {
            c = get_char(ctxt, lex);
            if (!ox_char_is_digit(c))
                break;

            n *= 10;
            n += c - '0';
        }

        if (n >= 255) {
            unget_char(ctxt, lex, c);
            store_last_loc(lex, &loc);
            c = get_char(ctxt, lex);
            error(ctxt, lex, &loc, OX_TEXT("string's output width should < 255"));
        } else {
            width = n;
        }
    }

    if (c == '.') {
        c = get_char(ctxt, lex);

        if (ox_char_is_digit(c)) {
            n = c - '0';

            store_first_loc(lex, &loc);

            while (1) {
                c = get_char(ctxt, lex);
                if (!ox_char_is_digit(c))
                    break;

                n *= 10;
                n += c - '0';
            }

            if (n >= 255) {
                unget_char(ctxt, lex, c);
                store_last_loc(lex, &loc);
                c = get_char(ctxt, lex);
                error(ctxt, lex, &loc, OX_TEXT("number's output precision should < 255"));
            } else {
                prec = n;
            }
        }
    }

    switch (c) {
    case 'o':
        fmt = OX_SOUT_FMT_OCT;
        break;
    case 'd':
        fmt = OX_SOUT_FMT_DEC;
        break;
    case 'u':
        fmt = OX_SOUT_FMT_UDEC;
        break;
    case 'x':
        fmt = OX_SOUT_FMT_HEX;
        break;
    case 'f':
        fmt = OX_SOUT_FMT_FLOAT;
        break;
    case 'e':
        fmt = OX_SOUT_FMT_EXP;
        break;
    case 'n':
        fmt = OX_SOUT_FMT_NUMBER;
        break;
    case 's':
        fmt = OX_SOUT_FMT_STR;
        break;
    case 'c':
        fmt = OX_SOUT_FMT_CHAR;
        break;
    default:
        unget_char(ctxt, lex, c);
        break;
    }

    tok->type = OX_TOKEN_NUMBER;
    ox_value_set_number(ctxt, tok->v, OX_SOUT_FLAGS_MAKE(flags, width, prec, fmt));
}

/*Get a regular expression.*/
static void
regular_expr (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok)
{
    OX_Result r;
    int c;
    int flags = 0;

    r = ox_re_from_input(ctxt, tok->v, lex->input);
    if (r == OX_ERR)
        lex->status |= OX_LEX_ST_ERR;

    tok->type = OX_TOKEN_RE;

    while (1) {
        c = get_char(ctxt, lex);
        if (!ox_char_is_alnum(c)) {
            unget_char(ctxt, lex, c);
            break;
        }

        switch (c) {
        case 'i':
            flags |= OX_RE_FL_IGNORE_CASE;
            break;
        case 'm':
            flags |= OX_RE_FL_MULTILINE;
            break;
        case 'd':
            flags |= OX_RE_FL_DOT_ALL;
            break;
        case 'u':
            flags |= OX_RE_FL_UNICODE;
            break;
        case 'p':
            flags |= OX_RE_FL_PERFECT;
            break;
        default:
            break;
        }
    }

    if (flags)
        ox_re_set_flags(ctxt, tok->v, flags);
}

/*Get document data.*/
static void
document (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, OX_Bool oneline)
{
    OX_Bool skip = OX_FALSE;
    int c, nc;

    text_clear(ctxt, lex);
    text_add_chars(ctxt, lex, "   ", 3);
    ox_input_get_loc(lex->input, &tok->loc.first_line, &tok->loc.first_column);
    tok->loc.first_column -= 2;

    if (oneline) {
        while (1) {
            c = get_char(ctxt, lex);

            if (c == OX_INPUT_END)
                break;

            text_add_char(ctxt, lex, c);

            if (c == '\n') {
                OX_Bool end = OX_TRUE;
                size_t len = lex->text.len;
                int line = lex->input->line;

                while (1) {
                    c = get_char(ctxt, lex);
                    if (lex->input->line != line + 1) {
                        unget_char(ctxt, lex, c);
                        break;
                    }
                    if (!ox_char_is_space(c)) {
                        if (c == '/') {
                            int nnc;
                            
                            nc = get_char(ctxt, lex);
                            nnc = get_char(ctxt, lex);

                            if ((nc == '/') && (nnc == '?')) {
                                text_add_chars(ctxt, lex, "   ", 3);
                                end = OX_FALSE;
                            } else {
                                unget_char(ctxt, lex, nnc);
                                unget_char(ctxt, lex, nc);
                                unget_char(ctxt, lex, c);
                            }
                        } else {
                            unget_char(ctxt, lex, c);
                        }

                        break;
                    }

                    text_add_char(ctxt, lex, c);
                }

                if (end) {
                    lex->text.len = len;
                    break;
                }
            }
        }
    } else {
        while (1) {
            c = get_char(ctxt, lex);

            if (c == OX_INPUT_END) {
                error(ctxt, lex, NULL, OX_TEXT("expect `*/\' at end of comment block"));
                break;
            }

            if (c == '*') {
                nc = get_char(ctxt, lex);
                if (nc == '/')
                    break;

                unget_char(ctxt, lex, nc);
            }

            if (skip) {
                if (c == '*') {
                    nc = get_char(ctxt, lex);
                    if (nc != '?') {
                        text_add_char(ctxt, lex, ' ');
                        unget_char(ctxt, lex, nc);
                    } else {
                        text_add_chars(ctxt, lex, "  ", 2);
                    }

                    skip = OX_FALSE;
                } else if (!ox_char_is_space(c)) {
                    text_add_char(ctxt, lex, c);
                    skip = OX_FALSE;
                } else {
                    text_add_char(ctxt, lex, c);
                }
            } else {
                text_add_char(ctxt, lex, c);
            }

            if (c == '\n')
                skip = OX_TRUE;
        }
    }

    ox_input_get_loc(lex->input, &tok->loc.last_line, &tok->loc.last_column);

    tok->type = OX_TOKEN_DOC;
    ox_not_error(ox_string_from_chars(ctxt, tok->v, lex->text.items, lex->text.len));
}

/**
 * Initialize the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer to be initialized.
 * @param input The character input.
 */
void
ox_lex_init (OX_Context *ctxt, OX_Lex *lex, OX_Input *input)
{
    lex->input = input;
    lex->status = 0;
    lex->brace_level = 0;
    lex->doc = NULL;

    ox_char_buffer_init(&lex->text);
    ox_vector_init(&lex->str_stack);
}

/**
 * Release the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer to be released.
 */
void
ox_lex_deinit (OX_Context *ctxt, OX_Lex *lex)
{
    ox_char_buffer_deinit(ctxt, &lex->text);
    ox_vector_deinit(ctxt, &lex->str_stack);
}

/**
 * Get a token from the lexical analyzer.
 * @param ctxt The current running context.
 * @param lex The lexical analyzer.
 * @param flags Analyse flags.
 * @param[out] tok Return the token.
 */
void
ox_lex_token (OX_Context *ctxt, OX_Lex *lex, OX_Token *tok, int flags)
{
    int c, nc;

retry:
    /*Eat up space and comment.*/
    while (1) {
        c = get_char(ctxt, lex);

        if (ox_char_is_space(c))
            continue;
        if ((c == '#') && !(lex->status & OX_LEX_ST_BODY)) {
            nc = get_char(ctxt, lex);

            if (nc == '!') {
                comment_line(ctxt, lex);
            } else {
                unget_char(ctxt, lex, nc);
                break;
            }
        } else if (c == '/') {
            nc = get_char(ctxt, lex);

            if ((nc == '/') || (nc == '*')) {
                int nnc = get_char(ctxt, lex);

                if ((nnc == '?') && (lex->status & OX_LEX_ST_DOC)) {
                    document(ctxt, lex, tok, nc == '/');
                    return;
                } else {
                    unget_char(ctxt, lex, nnc);

                    if (nc == '/') {
                        comment_line(ctxt, lex);
                    } else {
                        comment_block(ctxt, lex);
                    }
                }
            } else {
                unget_char(ctxt, lex, nc);
                break;
            }
        } else {
            break;
        }
    }

    lex->status |= OX_LEX_ST_BODY;

    store_first_loc(lex, &tok->loc);

    if (flags & OX_LEX_FL_STR_FMT) {
        unget_char(ctxt, lex, c);
        string_format(ctxt, lex, tok);
        store_last_loc(lex, &tok->loc);
        return;
    }

    switch (c) {
    case OX_INPUT_END:
        tok->loc.first_column ++;
        tok->type = OX_TOKEN_END;
        break;
    case '\'':
        nc = get_char(ctxt, lex);
        if (nc == '\'') {
            single_string_literal(ctxt, lex, tok, OX_TRUE, flags);
        } else {
            unget_char(ctxt, lex, nc);
            char_literal(ctxt, lex, tok);
        }
        break;
    case '\"':
        unget_char(ctxt, lex, c);
        string_literal(ctxt, lex, tok, flags);
        break;
    case '{':
        lex->brace_level ++;
        tok->type = c;
        break;
    case '}':
        if (lex->str_stack.len) {
            OX_LexStrState *ss = &ox_vector_item(&lex->str_stack, lex->str_stack.len - 1);

            if (ss->brace == lex->brace_level) {
                if (ss->type == OX_LEX_STR_DOUBLE_QUOT) {
                    /*Double quotation string's middle part or tail part.*/
                    unget_char(ctxt, lex, c);
                    string_literal(ctxt, lex, tok, flags);
                    break;
                } else {
                    nc = get_char(ctxt, lex);
                    if (nc == '}') {
                        /*Singal quotation string's middle part or tail part.*/
                        single_string_literal(ctxt, lex, tok, OX_FALSE, flags);
                        break;
                    } else {
                        unget_char(ctxt, lex, nc);
                    }
                }
            }
        }
        
        if (lex->brace_level > 0)
            lex->brace_level --;
        tok->type = c;
        break;
    case '/':
        if (flags & OX_LEX_FL_DIV) {
            unget_char(ctxt, lex, c);
            punctuation(ctxt, lex, tok);
        } else {
            regular_expr(ctxt, lex, tok);
        }
        break;
    case '@':
        nc = get_char(ctxt, lex);
        if (is_id_start(nc)) {
            unget_char(ctxt, lex, nc);
            identifier(ctxt, lex, tok, OX_TOKEN_AT_ID);
        } else {
            unget_char(ctxt, lex, nc);
            tok->type = c;
        }
        break;
    case '#':
        nc = get_char(ctxt, lex);
        if (is_id_start(nc)) {
            unget_char(ctxt, lex, nc);
            identifier(ctxt, lex, tok, OX_TOKEN_HASH_ID);
        } else {
            unget_char(ctxt, lex, nc);
            tok->type = c;
        }
        break;
    default:
        if (ox_char_is_digit(c)) {
            unget_char(ctxt, lex, c);
            number_literal(ctxt, lex, tok);
        } else if ((c == '_') || (c == '$') || ox_char_is_alpha(c)) {
            unget_char(ctxt, lex, c);
            identifier(ctxt, lex, tok, OX_TOKEN_ID);
        } else if (ox_char_is_punct(c)) {
            unget_char(ctxt, lex, c);
            punctuation(ctxt, lex, tok);
        } else {
            error(ctxt, lex, NULL, OX_TEXT("illegal character"));
            goto retry;
        }
    }

    store_last_loc(lex, &tok->loc);
    if (tok->type == OX_TOKEN_END)
        tok->loc.last_column ++;
}

/**
 * Get the token type's name.
 * @param ctxt The current running context.
 * @param cb The output character buffer to store the name.
 * @param type The token type or keyword type. 
 */
void
ox_token_type_get_name (OX_Context *ctxt, OX_CharBuffer *cb, int type)
{
    switch (type) {
    case OX_TOKEN_END:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("END")));
        break;
    case OX_TOKEN_NULL:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, "`null\'"));
        break;
    case OX_TOKEN_BOOL:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("boolean")));
        break;
    case OX_TOKEN_NUMBER:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("number")));
        break;
    case OX_TOKEN_STRING:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("string")));
        break;
    case OX_TOKEN_STR_HEAD:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("string head part")));
        break;
    case OX_TOKEN_STR_MID:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("string middle part")));
        break;
    case OX_TOKEN_STR_TAIL:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("string tail part")));
        break;
    case OX_TOKEN_RE:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("regular expression")));
        break;
    case OX_TOKEN_ID:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("identifier")));
        break;
    case OX_TOKEN_AT_ID:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("@identifier")));
        break;
    case OX_TOKEN_HASH_ID:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("#identifier")));
        break;
    case OX_TOKEN_DOC:
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("document")));
        break;
    case '\n':
        ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, OX_TEXT("LF")));
        break;
    default:
        if (type < 256) {
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '`'));
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, type));
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '\''));
        } else if (type > OX_KEYWORD_BEGIN) {
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '`'));
            ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, keywords[type - OX_KEYWORD_BEGIN - 1]));
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '\''));
        } else if (type > OX_TOKEN_PUNCT_BEGIN) {
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '`'));
            ox_not_error(ox_char_buffer_append_char_star(ctxt, cb, puncts[type - OX_TOKEN_PUNCT_BEGIN - 1]));
            ox_not_error(ox_char_buffer_append_char(ctxt, cb, '\''));
        } else {
            assert(0);
        }
    }
}
