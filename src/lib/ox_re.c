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
 * Regular expression.
 */

#define OX_LOG_TAG "ox_re"

#include "ox_internal.h"

/** Regular expression pattern.*/
typedef struct {
    OX_List alter_list; /**< Alternatives list.*/
} OX_RePat;

/** Regular expression alternative.*/
typedef struct {
    OX_List ln;         /**< List node data.*/
    OX_List term_list;  /**< Terminals list.*/
} OX_ReAlter;

/** Terminal type.*/
typedef enum {
    OX_RE_TERM_ALL,   /**< Match all characters.*/
    OX_RE_TERM_CHAR,  /**< Match a character.*/
    OX_RE_TERM_LS,    /**< Line start.*/
    OX_RE_TERM_LE,    /**< Line end.*/
    OX_RE_TERM_B,     /**< Blank.*/
    OX_RE_TERM_NB,    /**< Not blank.*/
    OX_RE_TERM_CC,    /**< Character class.*/
    OX_RE_TERM_PAT,   /**< Sub pattern.*/
    OX_RE_TERM_GROUP, /**< Group.*/
    OX_RE_TERM_LA,    /**< Look ahead.*/
    OX_RE_TERM_LAN,   /**< Look ahead not.*/
    OX_RE_TERM_LB,    /**< Look behind.*/
    OX_RE_TERM_LBN,   /**< Look behind not.*/
    OX_RE_TERM_BR     /**< Back reference.*/
} OX_ReTermType;

/** Space characer.*/
#define OX_RE_CHAR_S  -2
/** Character not space.*/
#define OX_RE_CHAR_NS -3
/** Digit character.*/
#define OX_RE_CHAR_D  -4
/** Character not digit.*/
#define OX_RE_CHAR_ND -5
/** Word character.*/
#define OX_RE_CHAR_W  -6
/** Character not word.*/
#define OX_RE_CHAR_NW -7

/** Item of the character class.*/
typedef struct {
    int min; /**< Minimum character.*/
    int max; /**< Maximum character.*/
} OX_ReCharClassItem;

/** Character class.*/
typedef struct {
    OX_Bool rev; /**< Reverse match or not.*/
    OX_VECTOR_TYPE_DECL(OX_ReCharClassItem) items; /**< Items.*/
} OX_ReCharClass;

/** Group.*/
typedef struct {
    OX_RePat pat; /**< Pattern of the group.*/
    int      id;  /**< Group's index.*/
} OX_ReGroup;

/** Regular expression terminal.*/
typedef struct {
    OX_List       ln;   /**< List node data.*/
    OX_ReTermType type; /**< Terminal type.*/
    union {
        int            c;     /**< Character.*/
        int            br;    /**< Back reference index.*/
        OX_ReCharClass cc;    /**< Character class.*/
        OX_RePat       pat;   /**< Sub pattern.*/
        OX_ReGroup     group; /**< Group.*/
    } t;                      /**< Terminal's data.*/
    int           min;        /**< Minimum repeat times.*/
    int           max;        /**< Maximum repeat times.*/
    OX_Bool       greedy;     /**< Greedy mode or not.*/
} OX_ReTerm;

/** Jump label of the regular expression.*/
typedef struct {
    int cp; /**< The label's command pointer.*/
} OX_ReLabel;

/** Next operation.*/
typedef struct OX_ReNextOp_s OX_ReNextOp;

/** Regular expression parser.*/
typedef struct {
    OX_Input     *input;     /**< The input.*/
    int           end_c;     /**< End character.*/
    int           group_num; /**< The number of the group.*/
    OX_Bool       error;     /**< Error flag.*/
    OX_Bool       rev;       /**< Reverse mode.*/
    OX_Bool       enable_cb; /**< Enable character buffer.*/
    OX_RePat      re;        /**< The regular expression pattern.*/
    OX_VECTOR_TYPE_DECL(OX_ReCmd)   cmds;   /**< Commands.*/
    OX_VECTOR_TYPE_DECL(OX_ReLabel) labels; /**< Labels.*/
    OX_CharBuffer cb;        /**< Character buffer.*/
} OX_ReParser;

/** Next operation.*/
struct OX_ReNextOp_s {
    OX_ReNextOp *next;       /**< The next operation.*/
    int          l_mismatch; /**< Mismatch label.*/
    int          l_loop;     /**< Loop label.*/
    OX_ReAlter  *alter;      /**< The alternative.*/
    OX_ReTerm   *term;       /**< The terminal.*/
    int          gid;        /**< The group's index.*/
    int          times;      /**< Repeat times.*/
    void (*fn) (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op); /**< Operation function.*/
};

/** Regular expression's state.*/
typedef struct {
    size_t pos; /**< The current position.*/
    size_t sp;  /**< The substrings' pointer.*/
    size_t cp;  /**< The command pointer.*/
} OX_ReState;

/** Regular expression match context.*/
typedef struct {
    OX_Re  *re;    /**< The regular expression.*/
    int     flags; /**< Flags.*/
    const char *chars; /**< The characters.*/
    size_t  start; /**< The start position.*/
    size_t  pos;   /**< Current position.*/
    size_t  len;   /**< The total length of the string.*/
    int     c;     /**< The current character.*/
    OX_VECTOR_TYPE_DECL(OX_Slice) slice_stack; /**< Slices' stack.*/
    OX_VECTOR_TYPE_DECL(OX_ReState) state_stack; /**< States' stack.*/
} OX_ReCtxt;

/*Release the pattern.*/
static void
pat_deinit (OX_Context *ctxt, OX_RePat *pat);
/*Parse the pattern.*/
static OX_Result
parse_pat (OX_Context *ctxt, OX_ReParser *p, OX_RePat *pat);
/*Generate the terminal's commands.*/
static void
gen_term (OX_Context *ctxt, OX_ReParser *p, OX_ReTerm *t, int times, OX_ReNextOp *next,
        int l_mismatch, int l_loop);
/*Generate the pattern's commands.*/
static void
gen_pat (OX_Context *ctxt, OX_ReParser *p, OX_RePat *pat, OX_ReNextOp *next, int l_mismatch);
/*Build the regular expression.*/
static OX_Result
re_build (OX_Context *ctxt, OX_Value *re, OX_Value *src, OX_ReFlag flags);

/*Scan referenced objects in the regular expression.*/
static void
re_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Re *re = (OX_Re*)gco;

    ox_object_scan(ctxt, &re->o.gco);

    ox_gc_scan_value(ctxt, &re->src);
}

/*Free the regular expression.*/
static void
re_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Re *re = (OX_Re*)gco;

    ox_object_deinit(ctxt, &re->o);

    if (re->cmds)
        OX_DEL_N(ctxt, re->cmds, re->cmd_len);

    OX_DEL(ctxt, re);
}

/** Regular expression's operation functions.*/
static OX_ObjectOps
re_ops = {
    {
        OX_GCO_RE,
        re_scan,
        re_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    ox_object_call
};

/*Allocate regular expression.*/
static OX_Result
re_alloc (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Re *rep;
    
    /*Allocate the regular expression.*/
    if (!OX_NEW(ctxt, rep))
        return ox_throw_no_mem_error(ctxt);

    ox_object_init(ctxt, &rep->o, OX_OBJECT(ctxt, Re_inf));

    rep->o.gco.ops = (OX_GcObjectOps*)&re_ops;
    rep->flags = 0;
    rep->group_num = 0;
    rep->cmd_len = 0;
    rep->cmds = NULL;
    ox_value_set_null(ctxt, &rep->src);

    ox_value_set_gco(ctxt, o, rep);
    ox_gc_add(ctxt, rep);

    return OX_OK;
}

/*Initialize the pattern.*/
static void
pat_init (OX_RePat *pat)
{
    ox_list_init(&pat->alter_list);
}

/*Create a new terminal.*/
static OX_ReTerm*
term_new (OX_Context *ctxt, OX_ReAlter *a)
{
    OX_ReTerm *t;

    if (!OX_NEW(ctxt, t)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    t->type = OX_RE_TERM_ALL;
    t->min = 1;
    t->max = 1;
    t->greedy = OX_TRUE;
    ox_list_append(&a->term_list, &t->ln);

    return t;
}

/*Free the terminal.*/
static void
term_free (OX_Context *ctxt, OX_ReTerm *t)
{
    switch (t->type) {
    case OX_RE_TERM_CC:
        ox_vector_deinit(ctxt, &t->t.cc.items);
        break;
    case OX_RE_TERM_GROUP:
        pat_deinit(ctxt, &t->t.group.pat);
        break;
    case OX_RE_TERM_PAT:
    case OX_RE_TERM_LA:
    case OX_RE_TERM_LAN:
    case OX_RE_TERM_LB:
    case OX_RE_TERM_LBN:
        pat_deinit(ctxt, &t->t.pat);
        break;
    default:
        break;
    }

    OX_DEL(ctxt, t);
}

/*Create a new alternaitve.*/
static OX_ReAlter*
alter_new (OX_Context *ctxt, OX_RePat *pat)
{
    OX_ReAlter *a;

    if (!OX_NEW(ctxt, a)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    ox_list_init(&a->term_list);
    ox_list_append(&pat->alter_list, &a->ln);

    return a;
}

/*Free the alternaitve.*/
static void
alter_free (OX_Context *ctxt, OX_ReAlter *a)
{
    OX_ReTerm *t, *nt;

    ox_list_foreach_safe_c(&a->term_list, t, nt, OX_ReTerm, ln) {
        term_free(ctxt, t);
    }

    OX_DEL(ctxt, a);
}

/*Release the pattern.*/
static void
pat_deinit (OX_Context *ctxt, OX_RePat *pat)
{
    OX_ReAlter *a, *na;

    ox_list_foreach_safe_c(&pat->alter_list, a, na, OX_ReAlter, ln) {
        alter_free(ctxt, a);
    }
}

/*Get a character from the input.*/
static int
get_char (OX_Context *ctxt, OX_ReParser *p)
{
    int c;

    c = ox_input_get_char(ctxt, p->input);
    if (p->enable_cb && (c >= 0)) {
        char buf[4];
        int n;

        n = ox_uc_to_utf8(c, buf);
        assert(n > 0);

        ox_not_error(ox_char_buffer_append_chars(ctxt, &p->cb, buf, n));
    }

    return c;
}

/*Push back a character to the input.*/
static void
unget_char (OX_Context *ctxt, OX_ReParser *p, int c)
{
    if (c >= 0) {
        ox_input_unget_char(ctxt, p->input, c);

        if (p->enable_cb) {
            int n = ox_uc_utf8_length(c);

            assert(p->cb.len >= n);
            p->cb.len -= n;
        }
    }
}

/*Output error prompt message.*/
static void
error (OX_Context *ctxt, OX_ReParser *p, OX_Location *loc, const char *fmt, ...)
{
    OX_Location loc_buf;
    va_list ap;

    if (!loc) {
        loc = &loc_buf;

        ox_input_get_loc(p->input, &loc->first_line, &loc->first_column);
        ox_input_get_loc(p->input, &loc->last_line, &loc->last_column);
    }

    va_start(ap, fmt);
    ox_prompt_v(ctxt, p->input, loc, OX_PROMPT_ERROR, fmt, ap);
    va_end(ap);

    p->error = OX_TRUE;
}

/*Parse number.*/
static int
number (OX_Context *ctxt, OX_ReParser *p)
{
    int c;
    int n;

    c = get_char(ctxt, p);
    if (!ox_char_is_digit(c)) {
        error(ctxt, p, NULL, OX_TEXT("expect a digit character here"));
        return -1;
    }

    n = c - '0';

    while (1) {
        c = get_char(ctxt, p);
        if (!ox_char_is_digit(c)) {
            unget_char(ctxt, p, c);
            break;
        }

        n *= 10;
        n += c - '0';
    }

    return n;
}

/*Hexadecimal escape character.*/
static int
hex_escape (OX_Context *ctxt, OX_ReParser *p)
{
    int c1, c2;

    c1 = get_char(ctxt, p);
    if (!ox_char_is_hex(c1)) {
        error(ctxt, p, NULL, OX_TEXT("expect a hexadecimal character here"));
        return -1;
    }

    c2 = get_char(ctxt, p);
    if (!ox_char_is_hex(c2)) {
        error(ctxt, p, NULL, OX_TEXT("expect a hexadecimal character here"));
        return -1;
    }

    return (ox_hex_char_to_num(c1) << 4) | ox_hex_char_to_num(c2);
}

/*Unicode escape character.*/
static int
uc_escape (OX_Context *ctxt, OX_ReParser *p)
{
    int c, v = 0;
    int i;
    OX_Bool overflow = OX_FALSE;

    c = get_char(ctxt, p);
    if (c == '{') {
        OX_Location loc;
        OX_Bool first = OX_TRUE;

        while (1) {
            c = get_char(ctxt, p);
            if (c == '}')
                break;

            if (first) {
                first = OX_FALSE;
                ox_input_get_loc(p->input, &loc.first_line, &loc.first_column);
            }

            v <<= 4;
            v |= ox_hex_char_to_num(c);

            if (!overflow && (v > 0x10ffff)) {
                ox_input_get_loc(p->input, &loc.last_line, &loc.last_column);
                overflow = OX_TRUE;
                error(ctxt, p, &loc, OX_TEXT("unicode value overflow"));
            }
        }
    } else {
        for (i = 0; i < 4; i ++) {
            c = get_char(ctxt, p);
            if (!ox_char_is_hex(c)) {
                error(ctxt, p, NULL, OX_TEXT("expect a hexadecimal character here"));
                return -1;
            }

            v <<= 4;
            v |= ox_hex_char_to_num(c);
        }

        if (ox_char_is_utf16_leading(v)) {
            char buf[6];
            int pos = 0;

            c = get_char(ctxt, p);
            buf[pos ++] = c;
            if (c == '\\') {
                c = get_char(ctxt, p);
                buf[pos ++] = c;
                if (c == 'u') {
                    c = get_char(ctxt, p);
                    buf[pos ++] = c;
                    if (c != '{') {
                        int t = 0;

                        unget_char(ctxt, p, c);
                        pos --;

                        for (i = 0; i < 4; i ++) {
                            c = get_char(ctxt, p);
                            buf[pos ++] = c;
                            if (!ox_char_is_hex(c)) {
                                error(ctxt, p, NULL, OX_TEXT("expect a hexadecimal character here"));
                                return -1;
                            }

                            t <<= 4;
                            t |= ox_hex_char_to_num(c);
                        }

                        if (ox_char_is_utf16_trailing(t)) {
                            pos = 0;
                            v = ox_uc_from_utf16_surrogate(v, t);
                        }
                    }
                }
            }

            while (pos--)
                unget_char(ctxt, p, buf[pos]);
        }
    }

    return v;
}

/*Parse an escape character.*/
static int
escape_char (OX_Context *ctxt, OX_ReParser *p)
{
    int c;

    c = get_char(ctxt, p);

    switch (c) {
    case 's':
        c = OX_RE_CHAR_S;
        break;
    case 'S':
        c = OX_RE_CHAR_NS;
        break;
    case 'w':
        c = OX_RE_CHAR_W;
        break;
    case 'W':
        c = OX_RE_CHAR_NW;
        break;
    case 'd':
        c = OX_RE_CHAR_D;
        break;
    case 'D':
        c = OX_RE_CHAR_ND;
        break;
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
    case OX_INPUT_END:
        error(ctxt, p, NULL, OX_TEXT("illegal escape character"));
        c = -1;
        break;
    case 'x':
        c = hex_escape(ctxt, p);
        break;
    case 'u':
        c = uc_escape(ctxt, p);
        break;
    case '\n':
        error(ctxt, p, NULL, OX_TEXT("`\n\' cannot be used in regular expression"));
        c = -1;
        break;
    default:
        break;
    }

    return c;
}

/*Parse a character.*/
static int
character (OX_Context *ctxt, OX_ReParser *p)
{
    int c;

    c = get_char(ctxt, p);
    if (c == '\\')
        c = escape_char(ctxt, p);
    else if (c == '\n') {
        error(ctxt, p, NULL, OX_TEXT("`\n\' cannot be used in regular expression"));
        c = -1;
    }

    return c;
}

/*Parse the character class.*/
static OX_Result
char_class (OX_Context *ctxt, OX_ReParser *p, OX_ReTerm *t)
{
    int c;
    OX_Result r;

    t->type = OX_RE_TERM_CC;
    t->t.cc.rev = OX_FALSE;
    ox_vector_init(&t->t.cc.items);

    c = get_char(ctxt, p);
    if (c == '^')
        t->t.cc.rev = OX_TRUE;
    else
        unget_char(ctxt, p, c);

    while (1) {
        OX_ReCharClassItem item;

        c = get_char(ctxt, p);
        if (c == ']')
            break;

        unget_char(ctxt, p, c);

        if ((item.min = character(ctxt, p)) == -1)
            return OX_ERR;

        c = get_char(ctxt, p);
        if (c == '-') {
            if (item.min < 0) {
                error(ctxt, p, NULL, OX_TEXT("character group cannot be used in range"));
                return OX_ERR;
            }

            if ((item.max = character(ctxt, p)) == -1)
                return OX_ERR;

            if (item.max < item.min) {
                int tmp = item.min;

                item.min = item.max;
                item.max = tmp;
            }
        } else {
            unget_char(ctxt, p, c);

            item.max = item.min;
        }

        if ((r = ox_vector_append(ctxt, &t->t.cc.items, item)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Parse the terminal.*/
static OX_Result
parse_term (OX_Context *ctxt, OX_ReParser *p, OX_ReAlter *alter)
{
    OX_ReTerm *t;
    int c;
    OX_Result r;

    if (!(t = term_new(ctxt, alter)))
        return OX_ERR;

    c = get_char(ctxt, p);

    switch (c) {
    case '.':
        t->type = OX_RE_TERM_ALL;
        break;
    case '^':
        t->type = OX_RE_TERM_LS;
        break;
    case '$':
        t->type = OX_RE_TERM_LE;
        break;
    case '\\':
        c = get_char(ctxt, p);
        switch (c) {
        case 'b':
            t->type = OX_RE_TERM_B;
            break;
        case 'B':
            t->type = OX_RE_TERM_NB;
            break;
        default:
            if ((c >= '1') && (c <= '9')) {
                t->type = OX_RE_TERM_BR;
                t->t.br = number(ctxt, p);
            } else {
                t->type = OX_RE_TERM_CHAR;
                unget_char(ctxt, p, c);
                if ((t->t.c = escape_char(ctxt, p)) == -1)
                    return OX_ERR;
            }
            break;
        }
        break;
    case '[':
        if ((r = char_class(ctxt, p, t)) == OX_ERR)
            return r;
        break;
    case '(':
        c = get_char(ctxt, p);
        if (c == '?') {
            c = get_char(ctxt, p);
            if (c == ':') {
                t->type = OX_RE_TERM_PAT;
            } else if (c == '=') {
                t->type = OX_RE_TERM_LA;
            } else if (c == '!') {
                t->type = OX_RE_TERM_LAN;
            } else if (c == '<') {
                c = get_char(ctxt, p);
                if (c == '=') {
                    t->type = OX_RE_TERM_LB;
                } else if (c == '!') {
                    t->type = OX_RE_TERM_LBN;
                } else {
                    error(ctxt, p, NULL, OX_TEXT("expect `=\' or `!\' here"));
                    return OX_ERR;
                }
            } else {
                error(ctxt, p, NULL, OX_TEXT("expect `:\', `=\' or `!\' here"));
                return OX_ERR;
            }

            pat_init(&t->t.pat);

            if ((r = parse_pat(ctxt, p, &t->t.pat)) == OX_ERR)
                return r;

            c = get_char(ctxt, p);
            if (c != ')') {
                error(ctxt, p, NULL, OX_TEXT("expect `)\' here"));
                return OX_ERR;
            }
        } else {
            t->type = OX_RE_TERM_GROUP;
            t->t.group.id = p->group_num ++;

            pat_init(&t->t.group.pat);

            unget_char(ctxt, p, c);
            if ((r = parse_pat(ctxt, p, &t->t.group.pat)) == OX_ERR)
                return r;

            c = get_char(ctxt, p);
            if (c != ')') {
                error(ctxt, p, NULL, OX_TEXT("expect `)\' here"));
                return OX_ERR;
            }
        }
        break;
    case '\n':
        error(ctxt, p, NULL, OX_TEXT("`\n\' cannot be used in regular expression"));
        return OX_ERR;
    default:
        t->type = OX_RE_TERM_CHAR;
        t->t.c = c;
        break;
    }

    if ((t->type != OX_RE_TERM_LS) && (t->type != OX_RE_TERM_LE)) {
        c = get_char(ctxt, p);
        switch (c) {
        case '?':
            t->min = 0;
            t->max = 1;
            break;
        case '*':
            t->min = 0;
            t->max = -1;
            break;
        case '+':
            t->min = 1;
            t->max = -1;
            break;
        case '{':
            c = get_char(ctxt, p);
            if (c != ',') {
                unget_char(ctxt, p, c);
                if ((t->min = number(ctxt, p)) == -1)
                    return OX_ERR;
                c = get_char(ctxt, p);
            } else {
                t->min = 0;
            }

            if (c == ',') {
                c = get_char(ctxt, p);
                if (c == '}') {
                    t->max = -1;
                } else {
                    unget_char(ctxt, p, c);
                    if ((t->max = number(ctxt, p)) == -1)
                        return OX_ERR;
                    c = get_char(ctxt, p);
                }
            } else {
                t->max = t->min;
            }

            if (c != '}') {
                error(ctxt, p, NULL, OX_TEXT("expect `}\' here"));
                return OX_ERR;
            }
            break;
        default:
            unget_char(ctxt, p, c);
            break;
        }

        if (t->min != t->max) {
            c = get_char(ctxt, p);
            if (c == '?')
                t->greedy = OX_FALSE;
            else
                unget_char(ctxt, p, c);
        }
    }

    return OX_OK;
}

/*Parse the alternaitve.*/
static OX_Result
parse_alter (OX_Context *ctxt, OX_ReParser *p, OX_RePat *pat)
{
    OX_ReAlter *a;
    int c;
    OX_Result r;

    if (!(a = alter_new(ctxt, pat)))
        return OX_ERR;

    while (1) {
        c = get_char(ctxt, p);
        if ((c == '|') || (c == OX_INPUT_END) || (c == p->end_c) || (c == ')')) {
            unget_char(ctxt, p, c);
            break;
        }

        unget_char(ctxt, p, c);
        if ((r = parse_term(ctxt, p, a)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Parse the pattern.*/
static OX_Result
parse_pat (OX_Context *ctxt, OX_ReParser *p, OX_RePat *pat)
{
    OX_Result r;

    if ((r = parse_alter(ctxt, p, pat)) == OX_ERR)
        return r;

    while (1) {
        int c = get_char(ctxt, p);

        if (c != '|') {
            unget_char(ctxt, p, c);
            break;
        }

        if ((r = parse_alter(ctxt, p, pat)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Parse the regular expression.*/
static OX_Result
parse_re (OX_Context *ctxt, OX_ReParser *p)
{
    OX_Result r;
    int c;

    if ((r = parse_pat(ctxt, p, &p->re)) == OX_ERR)
        return r;

    c = ox_input_get_char(ctxt, p->input);
    if (c != p->end_c) {
        if (c == OX_INPUT_END) {
            OX_Location loc;

            ox_input_get_loc(p->input, &loc.first_line, &loc.first_column);
            ox_input_get_loc(p->input, &loc.last_line, &loc.last_column);

            loc.first_column ++;
            loc.last_column ++;

            error(ctxt, p, &loc, OX_TEXT("expect `/\' at end of regular expression"));
        } else {
            error(ctxt, p, NULL, OX_TEXT("unexpected character"));
        }
    }

    return OX_OK;
}

/*Get the command pointer.*/
static inline OX_ReCmd*
get_cmd (OX_ReParser *p, int id)
{
    return &ox_vector_item(&p->cmds, id);
}

/*Add a command.*/
static int
add_cmd (OX_Context *ctxt, OX_ReParser *p, OX_ReCmdType type)
{
    int cid = p->cmds.len;
    OX_ReCmd *cmd;

    ox_not_error(ox_vector_expand(ctxt, &p->cmds, cid + 1));

    cmd = get_cmd(p, cid);
    cmd->type = type;
    cmd->l = -1;

    return cid;
}

/*Add a label.*/
static int
add_label (OX_Context *ctxt, OX_ReParser *p)
{
    int lid = p->labels.len;

    ox_not_error(ox_vector_expand(ctxt, &p->labels, lid + 1));

    return lid;
}

/*Set the label's command pointer to current.*/
static void
label_set_cp (OX_Context *ctxt, OX_ReParser *p, int lid)
{
    OX_ReLabel *l;

    l = &ox_vector_item(&p->labels, lid);

    l->cp = p->cmds.len;
}

/*Generate get a character operation.*/
static void
gen_get_char (OX_Context *ctxt, OX_ReParser *p, int l_mismatch)
{
    int cid;
    OX_ReCmd *cmd;

    if (p->rev)
        cid = add_cmd(ctxt, p, OX_RE_CMD_PREV);
    else
        cid = add_cmd(ctxt, p, OX_RE_CMD_NEXT);

    cmd = get_cmd(p, cid);
    cmd->l = l_mismatch;
}

/*Generate match character command.*/
static void
gen_match_char (OX_Context *ctxt, OX_ReParser *p, int c, int l_mismatch)
{
    int cid;
    OX_ReCmd *cmd;
    OX_ReCmdType cty;

    switch (c) {
    case OX_RE_CHAR_S:
        cty = OX_RE_CMD_MATCH_S;
        break;
    case OX_RE_CHAR_NS:
        cty = OX_RE_CMD_MATCH_NS;
        break;
    case OX_RE_CHAR_D:
        cty = OX_RE_CMD_MATCH_D;
        break;
    case OX_RE_CHAR_ND:
        cty = OX_RE_CMD_MATCH_ND;
        break;
    case OX_RE_CHAR_W:
        cty = OX_RE_CMD_MATCH_W;
        break;
    case OX_RE_CHAR_NW:
        cty = OX_RE_CMD_MATCH_NW;
        break;
    default:
        cty = OX_RE_CMD_MATCH_CHAR;
        break;
    }
    cid = add_cmd(ctxt, p, cty);
    cmd = get_cmd(p, cid);
    cmd->c.c = c;
    cmd->l = l_mismatch;
}

/*Generate commands of character class item.*/
static void
gen_cc_item (OX_Context *ctxt, OX_ReParser *p, OX_ReCharClassItem *item, int l_mismatch)
{
    if (item->min == item->max) {
        gen_match_char(ctxt, p, item->min, l_mismatch);
    } else {
        int cid;
        OX_ReCmd *cmd;

        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_RANGE);
        cmd = get_cmd(p, cid);

        cmd->c.r.min = item->min;
        cmd->c.r.max = item->max;
        cmd->l = l_mismatch;
    }
}

/*Generate group end commands.*/
static void
gen_group_end (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op)
{
    int cid;
    OX_ReCmd *cmd;

    cid = add_cmd(ctxt, p, OX_RE_CMD_GROUP_END);
    cmd = get_cmd(p, cid);
    cmd->c.gid = op->gid;

    op->next->fn(ctxt, p, op->next);
}

/*Generate look ahead/behind commands.*/
static void
gen_la_lb (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op)
{
    int cid;
    OX_ReCmd *cmd;
    OX_Bool old_rev = p->rev;

    add_cmd(ctxt, p, OX_RE_CMD_POP_POS);

    p->rev = OX_FALSE;

    if ((op->term->type == OX_RE_TERM_LAN) || (op->term->type == OX_RE_TERM_LBN)) {
        cid = add_cmd(ctxt, p, OX_RE_CMD_JMP);
        cmd = get_cmd(p, cid);
        cmd->l = op->l_mismatch;
    } else {
        op->next->fn(ctxt, p, op->next);
    }

    p->rev = old_rev;
}

/*Generate commands of match operation.*/
static void
gen_match (OX_Context *ctxt, OX_ReParser *p, OX_ReTerm *t, OX_ReNextOp *next, int l_mismatch)
{
    int cid;
    OX_ReCmd *cmd;
    OX_ReNextOp m_next;

    switch (t->type) {
    case OX_RE_TERM_LS:
        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_LS);
        cmd = get_cmd(p, cid);
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_LE:
        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_LE);
        cmd = get_cmd(p, cid);
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_ALL:
        gen_get_char(ctxt, p, l_mismatch);

        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_ALL);
        cmd = get_cmd(p, cid);
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_CHAR:
        gen_get_char(ctxt, p, l_mismatch);
        gen_match_char(ctxt, p, t->t.c, l_mismatch);

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_B:
        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_B);
        cmd = get_cmd(p, cid);
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_NB:
        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_NB);
        cmd = get_cmd(p, cid);
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_BR:
        cid = add_cmd(ctxt, p, OX_RE_CMD_MATCH_BR);
        cmd = get_cmd(p, cid);
        cmd->c.br = t->t.br;
        cmd->l = l_mismatch;

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_CC:
        gen_get_char(ctxt, p, l_mismatch);

        if (t->t.cc.rev) {
            size_t i;
            int l_next;

            for (i = 0; i < t->t.cc.items.len; i ++) {
                OX_ReCharClassItem *item = &ox_vector_item(&t->t.cc.items, i);

                l_next = add_label(ctxt, p);

                gen_cc_item(ctxt, p, item, l_next);
                cid = add_cmd(ctxt, p, OX_RE_CMD_JMP);
                cmd = get_cmd(p, cid);
                cmd->l = l_mismatch;

                label_set_cp(ctxt, p, l_next);
            }
        } else {
            size_t i;
            int l_next, l_end;

            l_end = add_label(ctxt, p);

            for (i = 0; i < t->t.cc.items.len; i ++) {
                OX_ReCharClassItem *item = &ox_vector_item(&t->t.cc.items, i);

                if (i == t->t.cc.items.len - 1)
                    l_next = l_mismatch;
                else
                    l_next = add_label(ctxt, p);

                gen_cc_item(ctxt, p, item, l_next);
                cid = add_cmd(ctxt, p, OX_RE_CMD_JMP);
                cmd = get_cmd(p, cid);
                cmd->l = l_end;

                if (l_next != l_mismatch)
                    label_set_cp(ctxt, p, l_next);
            }

            label_set_cp(ctxt, p, l_end);
        }

        next->fn(ctxt, p, next);
        break;
    case OX_RE_TERM_PAT:
        gen_pat(ctxt, p, &t->t.pat, next, l_mismatch);
        break;
    case OX_RE_TERM_GROUP:
        cid = add_cmd(ctxt, p, OX_RE_CMD_GROUP_START);
        cmd = get_cmd(p, cid);
        cmd->c.gid = t->t.group.id;

        m_next.fn = gen_group_end;
        m_next.next = next;
        m_next.l_mismatch = l_mismatch;
        m_next.gid = t->t.group.id;

        gen_pat(ctxt, p, &t->t.group.pat, &m_next, l_mismatch);
        break;
    case OX_RE_TERM_LA:
    case OX_RE_TERM_LAN:
    case OX_RE_TERM_LB:
    case OX_RE_TERM_LBN: {
        int l_match;

        add_cmd(ctxt, p, OX_RE_CMD_PUSH_POS);

        if ((t->type == OX_RE_TERM_LB) || (t->type == OX_RE_TERM_LBN))
            p->rev = OX_TRUE;

        m_next.fn = gen_la_lb;
        m_next.next = next;
        m_next.l_mismatch = l_mismatch;
        m_next.term = t;

        l_match = add_label(ctxt, p);

        gen_pat(ctxt, p, &t->t.pat, &m_next, l_match);

        if ((t->type == OX_RE_TERM_LB) || (t->type == OX_RE_TERM_LBN))
            p->rev = OX_FALSE;

        label_set_cp(ctxt, p, l_match);
        add_cmd(ctxt, p, OX_RE_CMD_POP_POS);

        if ((t->type == OX_RE_TERM_LAN) || (t->type == OX_RE_TERM_LBN)) {
            next->fn(ctxt, p, next);
        } else {
            cid = add_cmd(ctxt, p, OX_RE_CMD_JMP);
            cmd = get_cmd(p, cid);
            cmd->l = l_mismatch;
        }

        break;
    }
    default:
        assert(0);
    }
}

/*Generate terminal operation.*/
static void
gen_term_op (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op)
{
    gen_term(ctxt, p, op->term, op->times, op->next, op->l_mismatch, op->l_loop);
}

/*Generate the terminal's commands.*/
static void
gen_term (OX_Context *ctxt, OX_ReParser *p, OX_ReTerm *t, int times, OX_ReNextOp *next,
        int l_mismatch, int l_loop)
{
    int cid;
    OX_ReCmd *cmd;
    int l_next, l_old;
    OX_ReNextOp t_next;

    t_next.fn = gen_term_op;
    t_next.next = next;
    t_next.l_mismatch = l_mismatch;
    t_next.times = times + 1;
    t_next.term = t;

    if ((t->min == 1) && (t->max == 1)) {
        gen_match(ctxt, p, t, next, l_mismatch);
    } else if ((times >= 0) && (times < t->min)) {
        gen_match(ctxt, p, t, &t_next, l_mismatch);
    } else if (times == -1) {
        cid = add_cmd(ctxt, p, OX_RE_CMD_JMP);
        cmd = get_cmd(p, cid);
        cmd->l = l_loop;
    } else if (times == t->max) {
        next->fn(ctxt, p, next);
    } else if ((t->max == -1) && t->greedy) {
        t_next.times = -1;
        t_next.l_loop = add_label(ctxt, p);
        label_set_cp(ctxt, p, t_next.l_loop);

        l_next = add_label(ctxt, p);

        cid = add_cmd(ctxt, p, OX_RE_CMD_PUSH);
        cmd = get_cmd(p, cid);
        cmd->l = l_next;

        gen_match(ctxt, p, t, &t_next, l_next);

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_POP);

        l_next = add_label(ctxt, p);
        l_old = next->l_mismatch;
        next->l_mismatch = l_next;

        next->fn(ctxt, p, next);

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_REJECT);

        next->l_mismatch = l_old;
    } else if (t->max == -1) {
        l_old = next->l_mismatch;

        t_next.times = -1;
        t_next.l_loop = add_label(ctxt, p);
        label_set_cp(ctxt, p, t_next.l_loop);

        l_next = add_label(ctxt, p);
        next->l_mismatch = l_next;

        cid = add_cmd(ctxt, p, OX_RE_CMD_PUSH);
        cmd = get_cmd(p, cid);
        cmd->l = l_next;

        next->fn(ctxt, p, next);

        next->l_mismatch = l_old;

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_POP);

        gen_match(ctxt, p, t, &t_next, l_mismatch);
    } else if (t->greedy) {
        l_next = add_label(ctxt, p);

        cid = add_cmd(ctxt, p, OX_RE_CMD_PUSH);
        cmd = get_cmd(p, cid);
        cmd->l = l_next;

        gen_match(ctxt, p, t, &t_next, l_next);

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_POP);

        l_next = add_label(ctxt, p);
        l_old = next->l_mismatch;
        next->l_mismatch = l_next;

        next->fn(ctxt, p, next);

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_REJECT);

        next->l_mismatch = l_old;
    } else {
        int l_old = next->l_mismatch;

        l_next = add_label(ctxt, p);
        next->l_mismatch = l_next;

        cid = add_cmd(ctxt, p, OX_RE_CMD_PUSH);
        cmd = get_cmd(p, cid);
        cmd->l = l_next;

        next->fn(ctxt, p, next);

        next->l_mismatch = l_old;

        label_set_cp(ctxt, p, l_next);
        add_cmd(ctxt, p, OX_RE_CMD_POP);

        gen_match(ctxt, p, t, &t_next, l_mismatch);
    }
}

/*Generate commands for the next terminal.*/
static void
gen_next_term (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op)
{
    OX_ReTerm *nt;
    OX_ReNextOp *next;
    int l_mismatch;
    OX_ReNextOp t_next;

    if (p->rev) {
        nt = ox_list_prev_c(&op->alter->term_list, op->term, OX_ReTerm, ln);
    } else {
        nt = ox_list_next_c(&op->alter->term_list, op->term, OX_ReTerm, ln);
    }

    if (nt) {
        t_next.fn = gen_next_term;
        t_next.next = op->next;
        t_next.l_mismatch = op->l_mismatch;
        t_next.alter = op->alter;
        t_next.term = nt;

        next = &t_next;
    } else {
        next = op->next;
    }

    l_mismatch = op->l_mismatch;

    gen_term(ctxt, p, op->term, 0, next, l_mismatch, -1);
}

/*Generte the alternative's commands.*/
static void
gen_alter (OX_Context *ctxt, OX_ReParser *p, OX_ReAlter *a, OX_ReNextOp *next, int l_mismatch)
{
    if (ox_list_empty(&a->term_list)) {
        next->fn(ctxt, p, next);
    } else {
        OX_ReTerm *t, *nt;
        OX_ReNextOp t_next;

        if (p->rev) {
            t = ox_list_tail_c(&a->term_list, OX_ReTerm, ln);

            if (t->ln.prev == &a->term_list)
                nt = NULL;
            else
                nt = ox_list_prev_c(&a->term_list, t, OX_ReTerm, ln);
        } else {
            t = ox_list_head_c(&a->term_list, OX_ReTerm, ln);

            if (t->ln.next == &a->term_list)
                nt = NULL;
            else
                nt = ox_list_next_c(&a->term_list, t, OX_ReTerm, ln);
        }

        if (nt) {
            t_next.next = next;
            t_next.l_mismatch = l_mismatch;
            t_next.fn = gen_next_term;
            t_next.alter = a;
            t_next.term = nt;

            next = &t_next;
        }

        gen_term(ctxt, p, t, 0, next, l_mismatch, -1);
    }
}

/*Generate the pattern's commands.*/
static void
gen_pat (OX_Context *ctxt, OX_ReParser *p, OX_RePat *pat, OX_ReNextOp *next, int l_mismatch)
{
    if (ox_list_empty(&pat->alter_list)) {
        next->fn(ctxt, p, next);
    } else {
        OX_ReAlter *a;
        int cid;
        OX_ReCmd *cmd;

        ox_list_foreach_c(&pat->alter_list, a, OX_ReAlter, ln) {
            int l;

            if (a->ln.next == &pat->alter_list) {
                l = l_mismatch;
            } else {
                l = add_label(ctxt, p);

                cid = add_cmd(ctxt, p, OX_RE_CMD_PUSH);
                cmd = get_cmd(p, cid);
                cmd->l = l;
            }

            gen_alter(ctxt, p, a, next, l);

            if (l != l_mismatch) {
                label_set_cp(ctxt, p, l);

                add_cmd(ctxt, p, OX_RE_CMD_POP);
            }
        }
    }
}

/*Generate accept command.*/
static void
gen_accept (OX_Context *ctxt, OX_ReParser *p, OX_ReNextOp *op)
{
    int cid;
    OX_ReCmd *cmd;

    cid = add_cmd(ctxt, p, OX_RE_CMD_GROUP_END);
    cmd = get_cmd(p, cid);
    cmd->c.gid = 0;

    add_cmd(ctxt, p, OX_RE_CMD_ACCEPT);
}

/*Generate the regular expression's commands.*/
static OX_Result
gen_re (OX_Context *ctxt, OX_ReParser *p, OX_Value *re)
{
    int cid;
    int lid;
    OX_ReCmd *cmd;
    OX_ReNextOp next;
    OX_Re *rep;
    size_t i, cmd_len;
    int *cp_map;
    OX_Result r;

    /*Generate the commands.*/
    cid = add_cmd(ctxt, p, OX_RE_CMD_GROUP_START);
    cmd = get_cmd(p, cid);
    cmd->c.gid = 0;

    lid = add_label(ctxt, p);

    next.fn = gen_accept;

    gen_pat(ctxt, p, &p->re, &next, lid);

    label_set_cp(ctxt, p, lid);
    add_cmd(ctxt, p, OX_RE_CMD_REJECT);

    /*Set the label's command pointer.*/
    if (!OX_NEW_N(ctxt, cp_map, p->cmds.len))
        return ox_throw_no_mem_error(ctxt);

    cmd_len = 0;
    for (i = 0; i < p->cmds.len; i ++) {
        cmd = get_cmd(p, i);
        cp_map[i] = cmd_len;

        if (cmd->l != -1) {
            OX_ReLabel *l = &ox_vector_item(&p->labels, cmd->l);

            if (l->cp == i + 1) {
                cmd->type = -1;
            } else {
                cmd_len ++;
            }
        } else {
            cmd_len ++;
        }
    }

    for (i = 0; i < p->cmds.len; i ++) {
        cmd = get_cmd(p, i);

        if ((cmd->type != -1) && (cmd->l != -1)) {
            OX_ReLabel *l = &ox_vector_item(&p->labels, cmd->l);

            cmd->l = cp_map[l->cp];
        }
    }

    OX_DEL_N(ctxt, cp_map, p->cmds.len);

    /*Allocate the regular expression.*/
    rep = ox_value_get_gco(ctxt, re);

    rep->cmd_len = cmd_len;
    rep->group_num = p->group_num;

    if (!OX_NEW_N(ctxt, rep->cmds, rep->cmd_len))
        return ox_throw_no_mem_error(ctxt);

    cmd = rep->cmds;
    for (i = 0; i < p->cmds.len; i ++) {
        OX_ReCmd *src = get_cmd(p, i);

        if (src->type != -1) {
            *cmd = *src;
            cmd ++;
        }
    }

    /*Store the source.*/
    if (!p->enable_cb) {
        OX_StringInput *si = (OX_StringInput*)p->input;

        ox_value_copy(ctxt, &rep->src, &si->s);
    } else {
        if ((r = ox_char_buffer_get_string(ctxt, &p->cb, &rep->src)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Command name table.*/
static const char*
re_cmd_names[] = {
    "next",
    "prev",
    "match_all",
    "match_char",
    "match_range",
    "match_ls",
    "match_le",
    "match_s",
    "match_ns",
    "match_d",
    "match_nd",
    "match_w",
    "match_nw",
    "match_b",
    "match_nb",
    "match_br",
    "group_start",
    "group_end",
    "push",
    "pop",
    "push_pos",
    "pop_pos",
    "jmp",
    "accept",
    "reject"
};

/*Output a character.*/
static void
output_char (FILE *fp, uint32_t c)
{
    fprintf(fp, "\'");

    if (ox_char_is_graph(c) || (c == ' '))
        fprintf(fp, "%c", c);
    else if (c == '\n')
        fprintf(fp, "\\n");
    else if (c == '\t')
        fprintf(fp, "\\t");
    else if (c == '\r')
        fprintf(fp, "\\r");
    else if (c == '\v')
        fprintf(fp, "\\v");
    else if (c == '\f')
        fprintf(fp, "\\f");
    else if (c == '\a')
        fprintf(fp, "\\a");
    else if (c == '\b')
        fprintf(fp, "\\b");
    else if (c == '\'')
        fprintf(fp, "\\\'");
    else if (c == '\\')
        fprintf(fp, "\\\\");
    else if (c <= 0x7f)
        fprintf(fp, "\\x%02x", c);
    else
        fprintf(fp, "\\u{%x}", c);

    fprintf(fp, "\'");
}

/**
 * Disassemble the regular expression.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @param fp Output file.
 */
void
ox_re_disassemble (OX_Context *ctxt, OX_Value *re, FILE *fp)
{
    OX_Re *rep;
    size_t i;

    assert(ctxt && re && fp);
    assert(ox_value_is_re(ctxt, re));

    rep = ox_value_get_gco(ctxt, re);

    fprintf(fp, "source: %s\n", ox_string_get_char_star(ctxt, &rep->src));
    fprintf(fp, "n_group: %d\n", rep->group_num);
    fprintf(fp, "flags: ");
    if (rep->flags & OX_RE_FL_IGNORE_CASE)
        fprintf(fp, "i");
    if (rep->flags & OX_RE_FL_MULTILINE)
        fprintf(fp, "u");
    if (rep->flags & OX_RE_FL_DOT_ALL)
        fprintf(fp, "a");
    if (rep->flags & OX_RE_FL_UNICODE)
        fprintf(fp, "u");
    if (rep->flags & OX_RE_FL_PERFECT)
        fprintf(fp, "p");
    fprintf(fp, "\n");

    for (i = 0; i < rep->cmd_len; i ++) {
        OX_ReCmd *cmd = &rep->cmds[i];

        fprintf(fp, "%05"PRIdPTR": %-12s ", i, re_cmd_names[cmd->type]);

        switch (cmd->type) {
        case OX_RE_CMD_PREV:
        case OX_RE_CMD_NEXT:
        case OX_RE_CMD_MATCH_ALL:
        case OX_RE_CMD_MATCH_B:
        case OX_RE_CMD_MATCH_NB:
        case OX_RE_CMD_MATCH_S:
        case OX_RE_CMD_MATCH_NS:
        case OX_RE_CMD_MATCH_D:
        case OX_RE_CMD_MATCH_ND:
        case OX_RE_CMD_MATCH_W:
        case OX_RE_CMD_MATCH_NW:
        case OX_RE_CMD_MATCH_LS:
        case OX_RE_CMD_MATCH_LE:
        case OX_RE_CMD_JMP:
        case OX_RE_CMD_PUSH:
            fprintf(fp, "%d", cmd->l);
            break;
        case OX_RE_CMD_MATCH_CHAR:
            output_char(fp, cmd->c.c);
            fprintf(fp, " %d", cmd->l);
            break;
        case OX_RE_CMD_MATCH_RANGE:
            output_char(fp, cmd->c.r.min);
            fprintf(fp, " ");
            output_char(fp, cmd->c.r.max);
            fprintf(fp, " %d", cmd->l);
            break;
        case OX_RE_CMD_MATCH_BR:
            fprintf(fp, "%d %d", cmd->c.br, cmd->l);
            break;
        case OX_RE_CMD_GROUP_START:
        case OX_RE_CMD_GROUP_END:
            fprintf(fp, "%d", cmd->c.gid);
            break;
        default:
            break;
        }

        fprintf(fp, "\n");
    }
}

/*Create a regular expression from input.*/
static OX_Result
re_from_input (OX_Context *ctxt, OX_Value *re, OX_Input *input, OX_Bool enable_cb, int end_c)
{
    OX_ReParser rp;
    OX_Result r;

    rp.input = input;
    rp.end_c = end_c;
    rp.group_num = 1;
    rp.error = OX_FALSE;
    rp.rev = OX_FALSE;
    rp.enable_cb = enable_cb;
    pat_init(&rp.re);
    ox_vector_init(&rp.cmds);
    ox_vector_init(&rp.labels);
    ox_char_buffer_init(&rp.cb);

    r = parse_re(ctxt, &rp);
    if (rp.error)
        r = OX_ERR;

    if (r == OX_OK)
        r = gen_re(ctxt, &rp, re);

    pat_deinit(ctxt, &rp.re);
    ox_vector_deinit(ctxt, &rp.cmds);
    ox_vector_deinit(ctxt, &rp.labels);
    ox_char_buffer_deinit(ctxt, &rp.cb);

    return r;
}

/*Build the regular expression.*/
static OX_Result
re_build (OX_Context *ctxt, OX_Value *re, OX_Value *src, OX_ReFlag flags)
{
    OX_VS_PUSH(ctxt, input)
    OX_Input *ip = NULL;
    OX_Result r;

    assert(ctxt && re && src);
    assert(ox_value_is_string(ctxt, src));

    if ((r = ox_string_input_new(ctxt, input, src)) == OX_ERR)
        goto end;

    ip = ox_value_get_gco(ctxt, input);
    if ((r = re_from_input(ctxt, re, ip, OX_FALSE, OX_INPUT_END)) == OX_ERR)
        goto end;

    if ((r = ox_re_set_flags(ctxt, re, flags)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (ip)
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, input)
    return r;
}

/**
 * Create a new regular expression.
 * @param ctxt The current running context.
 * @param[out] re Return the new regular expression.
 * @param src The source string of the regular expression.
 * @param flags The flags of the regular expression.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_re_new (OX_Context *ctxt, OX_Value *re, OX_Value *src, OX_ReFlag flags)
{
    OX_Result r;

    assert(ctxt && re && src);

    if ((r = re_alloc(ctxt, re, OX_OBJECT(ctxt, Re_inf))) == OX_ERR)
        return r;

    return re_build(ctxt, re, src, flags);
}

/**
 * Create a new regular expression from the input.
 * @param ctxt The current running context.
 * @param[out] re Return the new regular expression.
 * @param input The source input.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_re_from_input (OX_Context *ctxt, OX_Value *re, OX_Input *input)
{
    OX_Result r;

    if ((r = re_alloc(ctxt, re, OX_OBJECT(ctxt, Re_inf))) == OX_ERR)
        return r;

    return re_from_input(ctxt, re, input, OX_TRUE, '/');
}

/*Initialize the regular expression match context.*/
static void
re_ctxt_init (OX_Context *ctxt, OX_ReCtxt *rc, OX_Value *re, OX_Value *s, size_t pos, int flags)
{
    rc->re = ox_value_get_gco(ctxt, re);
    rc->flags = rc->re->flags | flags;
    rc->chars = ox_string_get_char_star(ctxt, s);
    rc->len = ox_string_length(ctxt, s);
    rc->start = pos;
    rc->c = -1;

    ox_vector_init(&rc->slice_stack);
    ox_vector_init(&rc->state_stack);
}

/*Release the regular expression match context.*/
static void
re_ctxt_deinit (OX_Context *ctxt, OX_ReCtxt *rc)
{
    ox_vector_deinit(ctxt, &rc->slice_stack);
    ox_vector_deinit(ctxt, &rc->state_stack);
}

/*Get the next character from the string.*/
static inline OX_Result
next_char (OX_Context *ctxt, OX_ReCtxt *rc)
{
    OX_Result r;

    if (rc->pos >= rc->len)
        return OX_FALSE;

    if (rc->flags & OX_RE_FL_UNICODE) {
        size_t left = rc->len - rc->pos;

        rc->c = ox_uc_from_utf8(rc->chars + rc->pos, &left);
        if (rc->c == -1)
            return ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));

        rc->pos = rc->len - left;
        r = OX_OK;
    } else {
        rc->c = rc->chars[rc->pos ++];
        r = OX_OK;
    }

    return r;
}

/*Get the previous character from the string.*/
static inline OX_Result
prev_char (OX_Context *ctxt, OX_ReCtxt *rc)
{
    OX_Result r;

    if (rc->pos == 0)
        return OX_FALSE;

    if (rc->flags & OX_RE_FL_UNICODE) {
        size_t left = 0;

        while (1) {
            int c;

            rc->pos --;
            left ++;
            c = rc->chars[rc->pos];
            if ((c & 0xc0) != 0x80)
                break;

            if (rc->pos == 0)
                break;
        }

        rc->c = ox_uc_from_utf8(rc->chars + rc->pos, &left);
        if ((rc->c == -1) || (left != 0))
            return ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));

        r = OX_OK;
    } else {
        rc->c = rc->chars[-- rc->pos];
        r = OX_OK;
    }

    return r;
}

/*Match all character.*/
static inline OX_Bool
match_all (OX_ReCtxt *rc)
{
    OX_Bool r;

    if (rc->flags & OX_RE_FL_DOT_ALL) {
        r = OX_TRUE;
    } else {
        r = (rc->c != '\n');
    }

    return r;
}

/*Match the character.*/
static inline OX_Bool
match_char (OX_ReCtxt *rc, int c)
{
    OX_Bool r;

    if (rc->flags & OX_RE_FL_IGNORE_CASE) {
        r = (ox_char_to_upper(rc->c) == ox_char_to_upper(c));
    } else {
        r = (rc->c == c);
    }

    return r;
}

/*Match the character range.*/
static inline OX_Bool
match_range (OX_ReCtxt *rc, int min, int max)
{
    OX_Bool r;

    if ((rc->flags & OX_RE_FL_IGNORE_CASE) && ox_char_is_alpha(rc->c)) {
        int l = ox_char_to_lower(rc->c);

        if ((l >= min) && (l <= max))
            r = OX_TRUE;
        else {
            int u = ox_char_to_upper(rc->c);

            r = (u >= min) && (u <= max);
        }
    } else {
        r = (rc->c >= min) && (rc->c <= max);
    }

    return r;
}

/*Match line start.*/
static inline OX_Bool
match_line_start (OX_ReCtxt *rc)
{
    if (rc->pos == 0)
        return OX_TRUE;

    if (rc->flags & OX_RE_FL_MULTILINE) {
        if ((rc->pos) && (rc->chars[rc->pos - 1] == '\n'))
            return OX_TRUE;
    }

    return OX_FALSE;
}

/*Match line end.*/
static inline OX_Bool
match_line_end (OX_ReCtxt *rc)
{
    if (rc->pos == rc->len)
        return OX_TRUE;

    if (rc->flags & OX_RE_FL_MULTILINE) {
        if ((rc->pos < rc->len) && (rc->chars[rc->pos] == '\n'))
            return OX_TRUE;
    }

    return OX_FALSE;
}

/*Check if the character is a word character.*/
static inline OX_Bool
char_is_word (int c)
{
    return ox_char_is_alnum(c) || (c == '_');
}

/*Match the blank position.*/
static inline OX_Bool
match_blank (OX_ReCtxt *rc)
{
    int p, n;

    if (rc->pos)
        p = rc->chars[rc->pos - 1];
    else
        p = -1;

    if (rc->pos >= rc->len)
        n = -1;
    else
        n = rc->chars[rc->pos];

    return char_is_word(p) != char_is_word(n);
}

/*Back reference match.*/
static inline OX_Bool
match_back_ref (OX_ReCtxt *rc, int id)
{
    OX_ReState *rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len - 1);
    OX_Slice *slice = &ox_vector_item(&rc->slice_stack, rs->sp + id);
    size_t l1, l2;

    if ((slice->start == -1) || (slice->end == -1))
        return OX_TRUE;

    l1 = slice->end - slice->start;
    l2 = rc->len - rc->pos;

    if (l2 < l1)
        return OX_FALSE;

    if (memcmp(rc->chars + slice->start, rc->chars + rc->pos, l1) == 0) {
        rc->pos += l1;
        return OX_TRUE;
    }

    return OX_FALSE;
}

/*Group start.*/
static inline void
group_start (OX_ReCtxt *rc, int gid)
{
    OX_ReState *rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len - 1);
    OX_Slice *slice = &ox_vector_item(&rc->slice_stack, rs->sp + gid);

    slice->start = rc->pos;
}

/*Group end.*/
static inline void
group_end (OX_ReCtxt *rc, int gid)
{
    OX_ReState *rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len - 1);
    OX_Slice *slice = &ox_vector_item(&rc->slice_stack, rs->sp + gid);

    slice->end = rc->pos;
}

/*Push a new state to the stack.*/
static OX_Result
push_state (OX_Context *ctxt, OX_ReCtxt *rc, int cp)
{
    OX_ReState *rs;
    OX_Slice *sdst;
    size_t sid;
    OX_Result r;

    sid = rc->state_stack.len;

    if ((r = ox_vector_expand(ctxt, &rc->state_stack, sid + 1)) == OX_ERR)
        return r;

    rs = &ox_vector_item(&rc->state_stack, sid);

    rs->pos = rc->pos;
    rs->sp = rc->slice_stack.len;
    rs->cp = cp;

    if ((r = ox_vector_expand(ctxt, &rc->slice_stack, rs->sp + rc->re->group_num)) == OX_ERR)
        return r;

    sdst = &ox_vector_item(&rc->slice_stack, rs->sp);

    if (sid == 0) {
        memset(sdst, 0xff, sizeof(OX_Slice) * rc->re->group_num);
    } else {
        OX_ReState *rs_bot;
        OX_Slice *ssrc;

        rs_bot = rs - 1;
        ssrc = &ox_vector_item(&rc->slice_stack, rs_bot->sp);
        memcpy(sdst, ssrc, sizeof(OX_Slice) * rc->re->group_num);
    }

    return OX_OK;
}

/*Popup the top state from the stack.*/
static OX_Result
pop_state (OX_Context *ctxt, OX_ReCtxt *rc)
{
    OX_ReState *rs;

    assert(rc->state_stack.len >= 2);

    rc->state_stack.len --;

    rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len);

    rc->pos = rs->pos;
    rc->slice_stack.len = rs->sp;

    return OX_OK;
}

/*Push the current position to the stack.*/
static OX_Result
push_pos (OX_Context *ctxt, OX_ReCtxt *rc)
{
    OX_ReState *rs, *rs_bot;
    size_t sid;
    OX_Result r;

    sid = rc->state_stack.len;
    assert(sid > 0);

    if ((r = ox_vector_expand(ctxt, &rc->state_stack, sid + 1)) == OX_ERR)
        return r;

    rs = &ox_vector_item(&rc->state_stack, sid);
    rs_bot = rs - 1;

    rs->pos = rc->pos;
    rs->sp = rs_bot->sp;

    return OX_OK;
}

/*Popup the top position from the stack.*/
static OX_Result
pop_pos (OX_Context *ctxt, OX_ReCtxt *rc)
{
    OX_ReState *rs;

    assert(rc->state_stack.len >= 2);

    rc->state_stack.len --;

    rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len);

    rc->pos = rs->pos;

    return OX_OK;
}

/*Match the string with the regular expression.*/
static OX_Result
re_match (OX_Context *ctxt, OX_ReCtxt *rc)
{
    size_t cp = 0;
    OX_Result r;

    rc->pos = rc->start;
    rc->state_stack.len = 0;
    rc->slice_stack.len = 0;

    if ((r = push_state(ctxt, rc, 0)) == OX_ERR)
        return r;

    while (1) {
        OX_ReCmd *cmd = &rc->re->cmds[cp];

        /*OX_LOG_D(ctxt, "%05d: %d, %s", cp, rc->pos, re_cmd_names[cmd->type]);*/

        switch (cmd->type) {
        case OX_RE_CMD_NEXT:
            if ((r = next_char(ctxt, rc)) == OX_ERR)
                return r;
            if (r)
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_PREV:
            if ((r = prev_char(ctxt, rc)) == OX_ERR)
                return r;
            if (r)
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_ALL:
            if (match_all(rc))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_CHAR:
            if (match_char(rc, cmd->c.c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_RANGE:
            if (match_range(rc, cmd->c.r.min, cmd->c.r.max))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_LS:
            if (match_line_start(rc))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_LE:
            if (match_line_end(rc))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_S:
            if (ox_char_is_space(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_NS:
            if (!ox_char_is_space(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_D:
            if (ox_char_is_digit(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_ND:
            if (!ox_char_is_digit(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_W:
            if (char_is_word(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_NW:
            if (!char_is_word(rc->c))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_B:
            if (match_blank(rc))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_NB:
            if (!match_blank(rc))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_MATCH_BR:
            if (match_back_ref(rc, cmd->c.br))
                cp ++;
            else
                cp = cmd->l;
            break;
        case OX_RE_CMD_GROUP_START:
            group_start(rc, cmd->c.gid);
            cp ++;
            break;
        case OX_RE_CMD_GROUP_END:
            group_end(rc, cmd->c.gid);
            cp ++;
            break;
        case OX_RE_CMD_PUSH:
            if ((r = push_state(ctxt, rc, cmd->l)) == OX_ERR)
                return r;
            cp ++;
            break;
        case OX_RE_CMD_POP:
            if ((r = pop_state(ctxt, rc)) == OX_ERR)
                return r;
            cp ++;
            break;
        case OX_RE_CMD_PUSH_POS:
            if ((r = push_pos(ctxt, rc)) == OX_ERR)
                return r;
            cp ++;
            break;
        case OX_RE_CMD_POP_POS:
            if ((r = pop_pos(ctxt, rc)) == OX_ERR)
                return r;
            cp ++;
            break;
        case OX_RE_CMD_JMP:
            cp = cmd->l;
            break;
        case OX_RE_CMD_ACCEPT:
            if (rc->flags & OX_RE_FL_PERFECT) {
                if (rc->pos != rc->len) {
                    if (rc->state_stack.len > 1) {
                        OX_ReState *rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len - 1);
                        cp = rs->cp;
                        break;
                    } else {
                        return OX_FALSE;
                    }
                }
            }
            return OX_OK;
        case OX_RE_CMD_REJECT:
            if (rc->state_stack.len > 1) {
                OX_ReState *rs = &ox_vector_item(&rc->state_stack, rc->state_stack.len - 1);

                cp = rs->cp;
            } else {
                return OX_FALSE;
            }
            break;
        }
    }
}

/**
 * Match the string with the regular expression.
 * @param ctxt The current running context.
 * @param re The regular expression.
 * @param s The string.
 * @param start Start position.
 * @param flags The match flags will be added in match operation.
 * @param m The match result.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_re_match (OX_Context *ctxt, OX_Value *re, OX_Value *s, size_t start, int flags, OX_Value *m)
{
    OX_ReCtxt rc;
    OX_Result r = OX_FALSE;

    assert(ctxt && re && s && m);
    assert(ox_value_is_re(ctxt, re));
    assert(ox_value_is_string(ctxt, s));

    re_ctxt_init(ctxt, &rc, re, s, start, flags);

    while (rc.start <= rc.len) {
        r = re_match(ctxt, &rc);
        if (r != OX_FALSE)
            break;
        if (rc.flags & OX_RE_FL_PERFECT)
            break;

        rc.start ++;
    }

    if (r == OX_FALSE) {
        ox_value_set_null(ctxt, m);
        r = OX_OK;
    } else if (r == OX_OK) {
        OX_ReState *rs = &ox_vector_item(&rc.state_stack, rc.state_stack.len - 1);
        OX_Slice *slices = &ox_vector_item(&rc.slice_stack, rs->sp);

        r = ox_match_new(ctxt, m, s, rc.start, rc.pos, rc.re->group_num, slices);
    }

    re_ctxt_deinit(ctxt, &rc);
    return r;
}

/*Re.$inf.$init*/
static OX_Result
Re_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *src = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, src_str)
    OX_ReFlag flags_i = 0;
    OX_Result r;

    if (!ox_value_is_re(ctxt, thiz)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a regular expression"));
        goto end;
    }

    if ((r = ox_to_string(ctxt, src, src_str)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, flags)) {
        uint32_t u32;

        if (ox_value_is_string(ctxt, flags)) {
            const char *c = ox_string_get_char_star(ctxt, flags);
            size_t len = ox_string_length(ctxt, flags);

            while (len) {
                switch (*c) {
                case 'i':
                    flags_i |= OX_RE_FL_IGNORE_CASE;
                    break;
                case 'm':
                    flags_i |= OX_RE_FL_MULTILINE;
                    break;
                case 'd':
                    flags_i |= OX_RE_FL_DOT_ALL;
                    break;
                case 'p':
                    flags_i |= OX_RE_FL_PERFECT;
                    break;
                case 'u':
                    flags_i |= OX_RE_FL_UNICODE;
                    break;
                default:
                    break;
                }
                c ++;
                len --;
            }
        } else if ((r = ox_to_uint32(ctxt, flags, &u32)) == OX_ERR) {
            goto end;
        } else {
            flags_i = u32;
        }
    }

    r = re_build(ctxt, thiz, src_str, flags_i);
end:
    OX_VS_POP(ctxt, src_str)
    return r;
}

/*Re.$inf.match*/
static OX_Result
Re_inf_match (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, s)
    size_t pos = 0;
    OX_Result r;

    if (!ox_value_is_re(ctxt, thiz)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a regular expression"));
        goto end;
    }

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, pos_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = ox_string_length(ctxt, s) + p;
            if (p < 0)
                p = 0;
        } else {
            pos = p;
        }

        pos = p;
    }

    r = ox_re_match(ctxt, thiz, s, pos, 0, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Re.$inf.$to_str*/
static OX_Result
Re_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Re *re;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if (!ox_value_is_re(ctxt, thiz)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a regular expression"));
        goto end;
    }

    re = ox_value_get_gco(ctxt, thiz);

    if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_string(ctxt, &cb, &re->src)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
        goto end;

    if (re->flags & OX_RE_FL_IGNORE_CASE) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, 'i')) == OX_ERR)
            goto end;
    }

    if (re->flags & OX_RE_FL_MULTILINE) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, 'm')) == OX_ERR)
            goto end;
    }

    if (re->flags & OX_RE_FL_DOT_ALL) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, 'd')) == OX_ERR)
            goto end;
    }

    if (re->flags & OX_RE_FL_UNICODE) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, 'u')) == OX_ERR)
            goto end;
    }

    if (re->flags & OX_RE_FL_PERFECT) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, 'p')) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

/*?
 *? @lib {Re} Regular expression.
 *?
 *? @class{ Re Regular expression class.
 *?
 *? @bitfield{ Flags The flags of the regular expression.
 *? @item IGNORE_CASE Ignore the case of the characters.
 *? @item MULTILINE Enable multi line mode.
 *? By default, '$' can only match end of the string.
 *? If enable the multi line mode, '$' can match newline.
 *? @item DOT_ALL '.' can match any character include newline.
 *? @item UNICODE Enable unicode mode.
 *? By default, regular expressions process one 8-bit character at a time.
 *? If enable the unicode mode, regular expression process an unicode character at a time.
 *? @item PERFECT Perfect match mode.
 *? If enable perfect match mode, the regular expression must match the entire string.
 *? @bitfield}
 *?
 *? @func $init Initialize a regular expression object.
 *? @param src {String} The source of the regular expression.
 *? @param flags {String} The flags of the regular expression.
 *? Each character describe a flag:
 *? @ul{
 *? @li 'i': Ignore case.
 *? @li 'm': Multi line mode.
 *? @li 'd': Dot all mode.
 *? @li 'p': Perfect match mode.
 *? @li 'u': Unicode mode.
 *? @ul}
 *?
 *? @func match Math a string with the regular expression.
 *? @param s {String} The string to be matched.
 *? @param pos {Number} Start matching position.
 *? If pos < 0, the start position is the string's length + pos.
 *? @return {?Match} The match result.
 *? If the result expression cannot match any substring, return null.
 *?
 *? @func $to_str Convert the regular expression to string.
 *? The string's format is "/SOURCE/FLAGS".
 *? @return {String} The result string.
 *?
 *? @class}
 *?
 */

/**
 * Initialize the regular expression class.
 * @param ctxt The current running context.
 */
void
ox_re_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_2(ctxt, c, v)

    /*Re.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, OX_OBJECT(ctxt, Re_inf), NULL, "Re"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Re", c));
    ox_class_set_alloc_func(ctxt, c, re_alloc);

    ox_value_set_number(ctxt, v, OX_RE_FL_IGNORE_CASE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "IGNORE_CASE", v));
    ox_value_set_number(ctxt, v, OX_RE_FL_MULTILINE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MULTILINE", v));
    ox_value_set_number(ctxt, v, OX_RE_FL_DOT_ALL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "DOT_ALL", v));
    ox_value_set_number(ctxt, v, OX_RE_FL_UNICODE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "UNICODE", v));
    ox_value_set_number(ctxt, v, OX_RE_FL_PERFECT);
    ox_not_error(ox_object_add_const_s(ctxt, c, "PERFECT", v));

    /*Re_inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Re_inf), "$init", Re_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Re_inf), "match", Re_inf_match));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Re_inf), "$to_str", Re_inf_to_str));

    OX_VS_POP(ctxt, c);
}
