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
 * Source file parser.
 */

#define OX_LOG_TAG "ox_parser"

#include "ox_internal.h"

/** The parser has error.*/
#define OX_PARSER_ST_ERR       (1 << 0)
/** The parser has cached a token.*/
#define OX_PARSER_ST_CACHED    (1 << 1)
/** Parser do not show promot message.*/
#define OX_PRASER_ST_NO_PROMPT (1 << 2)

/** Return statement can be here.*/
#define OX_PARSER_FL_RETURN   (1 << 0)
/** Break statement can be here.*/
#define OX_PARSER_FL_BREAK    (1 << 1)
/** Continue statement can be here.*/
#define OX_PARSER_FL_CONTINUE (1 << 2)
/** Public symbol.*/
#define OX_PARSER_FL_PUBLIC   (1 << 3)
/** Owned ketyword can be here.*/
#define OX_PARSER_FL_OWNED    (1 << 4)
/** textdomain can be here.*/
#define OX_PARSER_FL_TEXTDOMAIN (1 << 5)

/** Recovery until newline.*/
#define OX_RECOVERY_LF        (1 << 0)
/** Recovery until semicolon.*/
#define OX_RECOVERY_SEMICOLON (1 << 1)
/** Recovery until block end.*/
#define OX_RECOVERY_BLOCK     (1 << 2)
/** Recovery until '}'.*/
#define OX_RECOVERY_RB        (1 << 3)
/** Recovery until ']'.*/
#define OX_RECOVERY_RSB       (1 << 4)
/** Recovery until string part.*/
#define OX_RECOVERY_STR_PART  (1 << 5)
/** Recovery until ','.*/
#define OX_RECOVERY_COMMA     (1 << 6)
/** Recovery until ')'.*/
#define OX_RECOVERY_RP        (1 << 7)

/** "static" can be used.*/
#define OX_CLASS_FL_STATIC    (1 << 0)
/** In static block.*/
#define OX_CLASS_FL_IN_STATIC (1 << 1)

/*Get the current token.*/
#define TOK  ((p->status & OX_PARSER_ST_CACHED) ? &p->last_tok : &p->tok)
/*Get the current token's location.*/
#define LOC  ((p->status & OX_PARSER_ST_CACHED) ? &p->last_tok.loc : &p->tok.loc)
/*Current token is in a new line.*/
#define NEWLINE ((p->tok.loc.first_line != p->last_tok.loc.last_line) || (TOK->type == OX_TOKEN_END))

/** Parser.*/
typedef struct {
    OX_Lex    lex;       /**< Lexical analyzer.*/
    OX_Token  tok;       /**< The current token.*/
    OX_Token  last_tok;  /**< The last token.*/
    int       status;    /**< Status.*/
    int       flags;     /**< Flags.*/
    OX_Value *ast;       /**< The AST value.*/
    OX_Value *block;     /**< The current block value.*/
    OX_Value *func;      /**< The current function.*/
    OX_Value *doc;       /**< Document.*/
} OX_Parser;

/** Priority of the expression.*/
typedef enum {
    OX_PRIO_LOWEST,
    OX_PRIO_COMMA,
    OX_PRIO_ASSI,
    OX_PRIO_LOGIC_OR,
    OX_PRIO_LOGIC_AND,
    OX_PRIO_BIT_OR,
    OX_PRIO_BIT_XOR,
    OX_PRIO_BIT_AND,
    OX_PRIO_EQ,
    OX_PRIO_REL,
    OX_PRIO_SHIFT,
    OX_PRIO_ADD,
    OX_PRIO_MUL,
    OX_PRIO_EXP,
    OX_PRIO_UNARY,
    OX_PRIO_CALL,
    OX_PRIO_HIGHEST
} OX_Priority;

/*Parse an identifier.*/
static OX_Result
identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr);
/*Parse an outer identifier.*/
static OX_Result
outer_identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr);
/*Parse a hash identifier.*/
static OX_Result
hash_identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr);
/*Parse the property name.*/
static OX_Result
prop_name (OX_Context *ctxt, OX_Parser *p, OX_Value *pn);
/*Parse a block.*/
static OX_Result
block (OX_Context *ctxt, OX_Parser *p, OX_Value *blk, OX_BlockContentType ctype);
/* . expression */
static OX_Result
dot_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr);
/*Parse a if expression.*/
static OX_Result
if_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_BlockContentType ctype);
/*Parse a case expression.*/
static OX_Result
case_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *re, OX_BlockContentType ctype);
/*Parse an expression's tail with priority.*/
static OX_Result
expression_tail_prio (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, int prio);
/*Parse an expression with priority.*/
static OX_Result
expression_prio (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, int prio);
/*Parse an expression or function in parentheses.*/
static OX_Result
expr_or_parenthese_func (OX_Context *ctxt, OX_Parser *p, OX_Value *e, OX_Bool *is_func, OX_Priority prio);
/*Parse an expression.*/
static OX_Result
expression (OX_Context *ctxt, OX_Parser *p, OX_Value *expr);
/*Parse arguments.*/
static OX_Result
arguments (OX_Context *ctxt, OX_Parser *p, OX_Value *args);
/*Parse a statement.*/
static OX_Result
statement (OX_Context *ctxt, OX_Parser *p);

/*Initialize the token.*/
static void
token_init (OX_Context *ctxt, OX_Token *tok)
{
    tok->v = ox_value_stack_push(ctxt);
    tok->type = OX_TOKEN_END;
    tok->keyword = OX_KEYWORD_NONE;
    tok->loc.first_line = 1;
    tok->loc.first_column = 0;
    tok->loc.last_line = 1;
    tok->loc.last_column = 0;
}

/*Copy the token.*/
static void
token_copy (OX_Context *ctxt, OX_Token *d, OX_Token *s)
{
    d->type = s->type;
    d->keyword = s->keyword;
    d->loc = s->loc;
    ox_value_copy(ctxt, d->v, s->v);
}

/*Check if the token is an expected keyword.*/
static OX_Bool
is_keyword (OX_Token *tok, OX_Keyword kw)
{
    if (tok->type != OX_TOKEN_ID)
        return OX_FALSE;

    return tok->keyword == kw;
}

/*Initialize the parser.*/
static void
parser_init (OX_Context *ctxt, OX_Parser *p, OX_Input *input, OX_Value *ast)
{
    ox_lex_init(ctxt, &p->lex, input);
    token_init(ctxt, &p->tok);
    token_init(ctxt, &p->last_tok);

    p->ast = ast;
    p->status = 0;
    p->flags = 0;
    p->block = ox_value_stack_push_n(ctxt, 3);
    p->func = ox_values_item(ctxt, p->block, 1);
    p->doc = ox_values_item(ctxt, p->block, 2);
}

/*Release the parser.*/
static void
parser_deinit (OX_Context *ctxt, OX_Parser *p)
{
    ox_lex_deinit(ctxt, &p->lex);
    ox_value_stack_pop(ctxt, p->tok.v);
}

/*Check if the command is a document block command.*/
static OX_Bool
is_doc_blk_cmd (const char *c)
{
    char buf[32];
    char *d = buf;
    size_t len = 0;

    /*Get the command.*/
    c ++;
    while (ox_char_is_alpha(*c) && (len < sizeof(buf) - 1)) {
        *d ++ = *c ++;
        len ++;
    }

    *d = 0;

    if (!strcmp(buf, "package")
            || !strcmp(buf, "module")
            || !strcmp(buf, "lib")
            || !strcmp(buf, "exe")
            || !strcmp(buf, "func")
            || !strcmp(buf, "class")
            || !strcmp(buf, "const")
            || !strcmp(buf, "var")
            || !strcmp(buf, "object")
            || !strcmp(buf, "callback")
            || !strcmp(buf, "otype"))
        return OX_TRUE;

    return OX_FALSE;
}

/*Store the document node.*/
static void
store_doc (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, doc, da)
    const char *c;

    /*Store the document.*/
    c = ox_string_get_char_star(ctxt, TOK->v);
    while (ox_char_is_space(*c))
        c ++;

    ox_ast_new(ctxt, doc, OX_AST_doc);
    ox_ast_set_loc(ctxt, doc, LOC);
    ox_not_error(ox_set(ctxt, doc, OX_STRING(ctxt, value), TOK->v));

    if ((c[0] == '@') && is_doc_blk_cmd(c)) {
        ox_not_error(ox_get(ctxt, p->ast, OX_STRING(ctxt, doc), da));

        if (ox_value_is_null(ctxt, da)) {
            ox_not_error(ox_array_new(ctxt, da, 0));
            ox_not_error(ox_set(ctxt, p->ast, OX_STRING(ctxt, doc), da));
        }

        ox_array_append(ctxt, da, doc);
    } else {
        ox_value_copy(ctxt, p->doc, doc);
    }

    OX_VS_POP(ctxt, doc)
}

/*Get a token from the input with flags.*/
static void
get_token_flags (OX_Context *ctxt, OX_Parser *p, int flags)
{
    if (p->status & OX_PARSER_ST_CACHED) {
        p->status &= ~OX_PARSER_ST_CACHED;
        return;
    }

    /*Store the last token.*/
    token_copy(ctxt, &p->last_tok, &p->tok);

    while (1) {
        /*Get the new token.*/
        ox_lex_token(ctxt, &p->lex, &p->tok, flags);

        if (TOK->type != OX_TOKEN_DOC)
            break;

        store_doc(ctxt, p);
    }

#if 0
    {
        OX_CharBuffer cb;

        ox_char_buffer_init(&cb);
        ox_token_type_get_name(ctxt, &cb, p->tok.type);
        OX_LOG_D(ctxt, "token: %s", ox_char_buffer_get_char_star(ctxt, &cb));
        ox_char_buffer_deinit(ctxt, &cb);
    }
#endif
}

/*Get a token from the input.*/
static inline void
get_token (OX_Context *ctxt, OX_Parser *p)
{
    get_token_flags(ctxt, p, 0);
}

/*Push back a token to the input.*/
static inline void
unget_token (OX_Context *ctxt, OX_Parser *p)
{
    if (p->tok.type == '/') {
        ox_input_unget_char(ctxt, p->lex.input, '/');
        token_copy(ctxt, &p->tok, &p->last_tok);
    } else {
        p->status |= OX_PARSER_ST_CACHED;
    }
}

/*Show prompt message.*/
static void
prompt_v (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, OX_PromptType type,
        const char *fmt, va_list ap)
{
    if (p->status & OX_PRASER_ST_NO_PROMPT)
        return;
        
    if (!loc)
        loc = LOC;

    ox_prompt_v(ctxt, p->lex.input, loc, type, fmt, ap);

    if (type == OX_PROMPT_ERROR)
        p->status |= OX_PARSER_ST_ERR;
}

/*Show error message.*/
static void
error (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    prompt_v(ctxt, p, loc, OX_PROMPT_ERROR, fmt, ap);

    va_end(ap);
}

/*Show warning message.*/
static void
warning (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    prompt_v(ctxt, p, loc, OX_PROMPT_WARNING, fmt, ap);

    va_end(ap);
}

/*Show note message.*/
static void
note (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    prompt_v(ctxt, p, loc, OX_PROMPT_NOTE, fmt, ap);

    va_end(ap);
}

/*Show unexpected token error message.*/
static void
unexpect_token_error_s (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, const char *expect)
{
    OX_CharBuffer cb;
    const char *cstr;

    ox_char_buffer_init(&cb);

    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, OX_TEXT("unexpected ")));

    if ((TOK->type == OX_TOKEN_ID) && (TOK->keyword != OX_KEYWORD_NONE))
        ox_token_type_get_name(ctxt, &cb, TOK->keyword);
    else
        ox_token_type_get_name(ctxt, &cb, TOK->type);

    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, OX_TEXT(", expect ")));
    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, expect));

    ox_not_null(cstr = ox_char_buffer_get_char_star(ctxt, &cb));
    error(ctxt, p, loc, "%s", cstr);

    ox_char_buffer_deinit(ctxt, &cb);
}

/*Show unexpected token error message.*/
static void
unexpect_token_error (OX_Context *ctxt, OX_Parser *p, OX_Location *loc, ...)
{
    OX_CharBuffer cb;
    const char *cstr;
    va_list ap;
    int type, last_type;
    OX_Bool first = OX_TRUE;

    ox_char_buffer_init(&cb);

    va_start(ap, loc);

    last_type = va_arg(ap, int);

    while (1) {
        type = va_arg(ap, int);

        if (type == -1) {
            if (!first)
                ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, OX_TEXT(" or ")));
            ox_token_type_get_name(ctxt, &cb, last_type);
            break;
        } else {
            if (!first)
                ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, OX_TEXT(", ")));
            ox_token_type_get_name(ctxt, &cb, last_type);
        }

        last_type = type;
        first = OX_FALSE;
    }

    va_end(ap);

    ox_not_null(cstr = ox_char_buffer_get_char_star(ctxt, &cb));
    unexpect_token_error_s(ctxt, p, loc, cstr);

    ox_char_buffer_deinit(ctxt, &cb);
}

/*Get a token and check its type with flags.*/
static OX_Result
get_expect_token_flags (OX_Context *ctxt, OX_Parser *p, int type, int flags)
{
    OX_Bool match = OX_TRUE;

    get_token_flags(ctxt, p, flags);

    if (type > OX_KEYWORD_BEGIN) {
        if ((TOK->type != OX_TOKEN_ID) || (TOK->keyword != type))
            match = OX_FALSE;
    } else if (TOK->type != type) {
        match = OX_FALSE;
    }

    if (!match) {
        unexpect_token_error(ctxt, p, NULL, type, -1);
        unget_token(ctxt, p);
        return OX_ERR;
    }

    return OX_OK;
}

/*Get a token and check its type.*/
static inline OX_Result
get_expect_token (OX_Context *ctxt, OX_Parser *p, int type)
{
    return get_expect_token_flags(ctxt, p, type, 0);
}

/*Error recovery.*/
static OX_Result
error_recovery (OX_Context *ctxt, OX_Parser *p, int flags)
{
    int b_level = 0;
    int sb_level = 0;
    int p_level = 0;

    while (1) {
        get_token(ctxt, p);

        switch (TOK->type) {
        case '{':
            b_level ++;
            break;
        case '}':
            b_level --;
            break;
        case '[':
            sb_level ++;
            break;
        case ']':
            sb_level --;
            break;
        case '(':
            p_level ++;
            break;
        case ')':
            p_level --;
            break;
        default:
            break;
        }

        if (TOK->type == OX_TOKEN_END)
            return OX_ERR;

        if (flags & OX_RECOVERY_LF) {
            if (NEWLINE) {
                unget_token(ctxt, p);
                return OX_OK;
            }
        }

        if (flags & OX_RECOVERY_COMMA) {
            if (TOK->type == ',')
                return OX_OK;
        }

        if (flags & OX_RECOVERY_SEMICOLON) {
            if (TOK->type == ';')
                return OX_OK;
        }

        if (flags & OX_RECOVERY_BLOCK) {
            if ((TOK->type == '}') && (b_level == 0))
                return OX_OK;
        }

        if (flags & OX_RECOVERY_RP) {
            if ((TOK->type == ')') && (p_level == -1)) {
                return OX_ERR;
            }
        }

        if (flags & OX_RECOVERY_RSB) {
            if ((TOK->type == ']') && (sb_level == -1)) {
                return OX_ERR;
            }
        }

        if (flags & OX_RECOVERY_RB) {
            if ((TOK->type == '}') && (b_level == -1)) {
                return OX_ERR;
            }
        }

        if (flags & OX_RECOVERY_STR_PART) {
            if (TOK->type == OX_TOKEN_STR_MID)
                return OX_OK;
            if (TOK->type == OX_TOKEN_STR_TAIL)
                return OX_ERR;
        }
    }
}

/*Create a new AST node.*/
static void
ast_new (OX_Context *ctxt, OX_Value *v, OX_AstType type)
{
    ox_ast_new(ctxt, v, type);
}

/*Set the number type property of an AST node.*/
static void
ast_set_number (OX_Context *ctxt, OX_Value *ast, OX_Value *k, OX_Number n)
{
    OX_VS_PUSH(ctxt, v)

    ox_value_set_number(ctxt, v, n);
    ox_not_error(ox_set(ctxt, ast, k, v));

    OX_VS_POP(ctxt, v)
}

/*Get the number type property of an AST node.*/
static OX_Result
ast_get_number (OX_Context *ctxt, OX_Value *ast, OX_Value *k, OX_Number *n)
{
    OX_VS_PUSH(ctxt, v)
    OX_Result r;

    ox_not_error(ox_get_throw(ctxt, ast, k, v));
    if (ox_value_is_number(ctxt, v)) {
        *n = ox_value_get_number(ctxt, v);
        r = OX_OK;
    } else {
        r = OX_ERR;
    }

    OX_VS_POP(ctxt, v)

    return r;
}

/*Get the boolean type property of an AST node.*/
static OX_Bool
ast_get_bool (OX_Context *ctxt, OX_Value *ast, OX_Value *k)
{
    OX_VS_PUSH(ctxt, v)
    OX_Bool b;

    ox_not_error(ox_get(ctxt, ast, k, v));
    b = ox_to_bool(ctxt, v);

    OX_VS_POP(ctxt, v)
    return b;
}

/*Set the boolean type property of an AST node.*/
static void
ast_set_bool (OX_Context *ctxt, OX_Value *ast, OX_Value *k, OX_Bool b)
{
    OX_VS_PUSH(ctxt, v)

    ox_value_set_bool(ctxt, v, b);
    ox_not_error(ox_set(ctxt, ast, k, v));

    OX_VS_POP(ctxt, v)
}

/*Create a new AST node.*/
#define AST_NEW(a, t)        ast_new(ctxt, a, OX_AST_##t)
/*Get AST node's property.*/
#define AST_GET(a, p, v)     ox_not_error(ox_get(ctxt, a, OX_STRING(ctxt, p), v))
/*Set AST node's property.*/
#define AST_SET(a, p, v)     ox_not_error(ox_set(ctxt, a, OX_STRING(ctxt, p), v))
/*Get number type property of an AST node.*/
#define AST_GET_N(a, p, n)   ast_get_number(ctxt, a, OX_STRING(ctxt, p), n)
/*Set number type property of an AST node.*/
#define AST_SET_N(a, p, n)   ast_set_number(ctxt, a, OX_STRING(ctxt, p), n)
/*Get boolean type property of an AST node.*/
#define AST_GET_B(a, p)      ast_get_bool(ctxt, a, OX_STRING(ctxt, p))
/*Set boolean type property of an AST node.*/
#define AST_SET_B(a, p, b)   ast_set_bool(ctxt, a, OX_STRING(ctxt, p), b)
/*Create a new array.*/
#define ARRAY_NEW(v)         ox_not_error(ox_array_new(ctxt, v, 0))
/*Append an item to the array.*/
#define ARRAY_APPEND(v, i)   ox_not_error(ox_array_append(ctxt, v, i))
/*Get the location property.*/
#define GET_LOC(a, l)        ox_ast_get_loc(ctxt, a, l)
/*Set the location property.*/
#define SET_LOC(a, l)        ox_ast_set_loc(ctxt, a, l)

/*Store the first character's location.*/
static void
first_loc (OX_Location *dst, OX_Location *src)
{
    dst->first_line = src->first_line;
    dst->first_column = src->first_column;
}

/*Store the last character's location.*/
static void
last_loc (OX_Location *dst, OX_Location *src)
{
    dst->last_line = src->last_line;
    dst->last_column = src->last_column;
}

/*Create a new function node.*/
static void
func_new (OX_Context *ctxt, OX_Parser *p, OX_Value *func)
{
    AST_NEW(func, func);

    if (!ox_value_is_null(ctxt, p->func))
        AST_SET(func, outer, p->func);
}

/*Add a function node.*/
static void
add_func (OX_Context *ctxt, OX_Parser *p, OX_Value *func)
{
    OX_VS_PUSH(ctxt, funcs)
    size_t id;

    AST_GET(p->ast, funcs, funcs);

    if (ox_value_is_null(ctxt, funcs)) {
        ARRAY_NEW(funcs);
        AST_SET(p->ast, funcs, funcs);
    }

    id = ox_array_length(ctxt, funcs);

    ARRAY_APPEND(funcs, func);

    AST_SET_N(func, id, id);

    OX_VS_POP(ctxt, funcs)
}

/*Add a declaration.*/
static void
add_decl (OX_Context *ctxt, OX_Parser *p, OX_Value *n, OX_Location *loc, OX_DeclType type)
{
    OX_VS_PUSH_2(ctxt, decls, decl)
    int old_type = -1, new_type = -1;
    int old_flags = 0, new_flags = 0;
    OX_Bool ok;

    AST_GET(p->func, decls, decls);
    if (ox_value_is_null(ctxt, decls)) {
        ox_not_error(ox_object_new(ctxt, decls, NULL));
        AST_SET(p->func, decls, decls);
    }

    ox_not_error(ox_get(ctxt, decls, n, decl));
    if (!ox_value_is_null(ctxt, decl)) {
        OX_Number n;

        if (AST_GET_N(decl, decl_type, &n) == OX_OK) {
            int i = n;

            old_type = i & OX_DECL_TYPE_MASK;
            old_flags = i & OX_DECL_AUTO_CLOSE;
        }
    }

    new_type = type & OX_DECL_TYPE_MASK;
    new_flags = type & OX_DECL_AUTO_CLOSE;

    if ((old_type == OX_DECL_CONST) || (old_type == OX_DECL_REF)) {
        ok = OX_FALSE;
    } else if ((old_type != -1) && ((new_type == OX_DECL_CONST) || (new_type == OX_DECL_REF))) {
        ok = OX_FALSE;
    } else if ((old_type == OX_DECL_PARAM) && (new_type == OX_DECL_VAR)) {
        new_type = old_type;
        ok = OX_TRUE;
    } else if ((old_type != -1) && (old_type != new_type)) {
        ok = OX_FALSE;
    } else {
        ok = OX_TRUE;
    }

    if (!ok) {
        OX_Location oloc;

        GET_LOC(decl, &oloc);

        switch (old_type) {
        case OX_DECL_PARAM:
            error(ctxt, p, loc, OX_TEXT("identifier is already declared as a parameter"));
            break;
        case OX_DECL_REF:
            error(ctxt, p, loc, OX_TEXT("identifier is already declared as a reference"));
            break;
        case OX_DECL_CONST:
            error(ctxt, p, loc, OX_TEXT("identifier is already declared as a constant"));
            break;
        case OX_DECL_VAR:
            error(ctxt, p, loc, OX_TEXT("identifier is already declared as a variable"));
            break;
        case OX_DECL_OUTER:
            error(ctxt, p, loc, OX_TEXT("identifier is already declared as an outer variable"));
            break;
        default:
            assert(0);
        }

        note(ctxt, p, &oloc, OX_TEXT("previous declaration is here"));
    } else {
        if (ox_value_is_null(ctxt, decl)) {
            AST_NEW(decl, decl);
            ox_not_error(ox_set(ctxt, decls, n, decl));
        }

        if ((old_type != new_type) || (old_flags != new_flags)) {
            type = new_type | new_flags | old_flags;

            AST_SET_N(decl, decl_type, type);
            SET_LOC(decl, loc);

            if (p->flags & OX_PARSER_FL_PUBLIC)
                AST_SET_B(decl, public, OX_TRUE);
        }
    }

    OX_VS_POP(ctxt, decls)
}

/*Add a declaration from the AST node.*/
static void
add_decl_ast (OX_Context *ctxt, OX_Parser *p, OX_Value *ast, OX_DeclType dty)
{
    OX_VS_PUSH_3(ctxt, n, e, items)
    OX_AstType ast_type = ox_ast_get_type(ctxt, ast);

    if (ast_type == OX_AST_id) {
        OX_Location loc;
        OX_DeclType idty = dty;

        GET_LOC(ast, &loc);
        AST_GET(ast, value, n);

        if (AST_GET_B(ast, outer)) {
            if (idty == OX_DECL_CONST)
                error(ctxt, p, &loc, OX_TEXT("cannot define outer identifier as constant"));

            idty = OX_DECL_OUTER;
        }

        if (AST_GET_B(ast, hash)) {
            idty |= OX_DECL_AUTO_CLOSE;
        }

        add_decl(ctxt, p, n, &loc, idty);
    } else if (ast_type == OX_AST_parenthese) {
        AST_GET(ast, expr, e);
        add_decl_ast(ctxt, p, e, dty);
    } else if (ast_type == OX_AST_comma) {
        size_t len, i;

        AST_GET(ast, items, items);
        len = ox_array_length(ctxt, items);

        for (i = 0; i < len; i ++) {
            ox_array_get_item(ctxt, items, i, e);
            add_decl_ast(ctxt, p, e, dty);
        }
    }

    OX_VS_POP(ctxt, n)
}

/*Duplicate declarations.*/
static void
dup_decls (OX_Context *ctxt, OX_Parser *p, OX_Value *f)
{
    OX_VS_PUSH_5(ctxt, decls, iter, e, n, decl)
    OX_Location loc;
    OX_DeclType dty;
    OX_Number tn;

    AST_GET(f, decls, decls);
    if (!ox_value_is_null(ctxt, decls)) {
        ox_not_error(ox_object_iter_new(ctxt, iter, decls, OX_OBJECT_ITER_KEY_VALUE));
        while (!ox_iterator_end(ctxt, iter)) {
            ox_not_error(ox_iterator_value(ctxt, iter, e));
            ox_not_error(ox_array_get_item(ctxt, e, 0, n));
            ox_not_error(ox_array_get_item(ctxt, e, 1, decl));

            GET_LOC(decl, &loc);
            AST_GET_N(decl, type, &tn);
            dty = tn;

            add_decl(ctxt, p, n, &loc, dty);

            ox_not_error(ox_iterator_next(ctxt, iter));
        }
    }

    OX_VS_POP(ctxt, decls)
}

/*Create the script node.*/
static void
script_new (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, func, blk)

    AST_NEW(p->ast, script);
    func_new(ctxt, p, func);
    AST_NEW(blk, block);
    AST_SET(func, block, blk);

    add_func(ctxt, p, func);

    ox_value_copy(ctxt, p->func, func);
    ox_value_copy(ctxt, p->block, blk);
}

/*Add a statement node to the current block.*/
static void
add_stmt (OX_Context *ctxt, OX_Parser *p, OX_Value *stmt)
{
    OX_VS_PUSH(ctxt, items)

    AST_GET(p->block, items, items);

    if (ox_value_is_null(ctxt, items)) {
        ARRAY_NEW(items);
        AST_SET(p->block, items, items);
    }

    ARRAY_APPEND(items, stmt);

    OX_VS_POP(ctxt, items)
}

/*Check the statement end.*/
static OX_Result
stmt_end (OX_Context *ctxt, OX_Parser *p)
{
    OX_Result r;

    if (TOK->type == '}')
        return OX_OK;

    get_token(ctxt, p);

    if (!NEWLINE
            && (TOK->type != ';')
            && (TOK->type != '}')
            && (TOK->type != '{')
            && (TOK->type != OX_TOKEN_END)) {
        unexpect_token_error(ctxt, p, LOC, ';', '\n', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
    } else {
        unget_token(ctxt, p);
        r = OX_OK;
    }

    return r;
}

/*Parse the string format.*/
static int
string_format (OX_Context *ctxt, OX_Parser *p)
{
    OX_Result r;

    r = get_expect_token_flags(ctxt, p, OX_TOKEN_NUMBER, OX_LEX_FL_STR_FMT);
    assert(r == OX_OK);

    return ox_value_get_number(ctxt, TOK->v);
}

/*Parse multipart string.*/
static OX_Result
multipart_string (OX_Context *ctxt, OX_Parser *p, OX_Value *f, OX_Value *s)
{
    OX_VS_PUSH_4(ctxt, e, templ, item, items)
    OX_Bool local = OX_FALSE;
    OX_Location loc;
    OX_Result r;

    if (f) {
        OX_AstType type;

        type = ox_ast_get_type(ctxt, f);

        if (type == OX_AST_id) {
            const char *cstr;

            AST_GET(f, value, e);

            cstr = ox_string_get_char_star(ctxt, e);

            if (!strcmp(cstr, "L"))
                local = OX_TRUE;
        }
    }

    if (local && (TOK->type == OX_TOKEN_STRING)) {
        AST_NEW(s, value);
        AST_SET(s, value, TOK->v);
        AST_SET_B(s, local, OX_TRUE);
        SET_LOC(s, LOC);
        r = OX_OK;
        goto end;
    }

    AST_NEW(s, string);
    ARRAY_NEW(templ);
    AST_SET(s, templ, templ);
    ARRAY_NEW(items);
    AST_SET(s, items, items);

    if (local)
        AST_SET_B(s, local, OX_TRUE);
    else if (f)
        AST_SET(s, expr, f);

    if (ox_string_length(ctxt, TOK->v))
        ARRAY_APPEND(templ, TOK->v);
    else
        ARRAY_APPEND(templ, ox_value_null(ctxt));

    if (TOK->type == OX_TOKEN_STR_HEAD) {
        while (1) {
            r = expression(ctxt, p, e);

            if (r == OX_OK) {
                get_token(ctxt, p);
                if (TOK->type == '!') {
                    int fmt;

                    fmt = string_format(ctxt, p);

                    AST_NEW(item, format);
                    AST_SET(item, expr, e);
                    AST_SET_N(item, format, fmt);
                    ARRAY_APPEND(items, item);
                } else {
                    ARRAY_APPEND(items, e);
                    unget_token(ctxt, p);
                }

                get_token(ctxt, p);
                if ((TOK->type == OX_TOKEN_STR_MID) || (TOK->type == OX_TOKEN_STR_TAIL)) {
                    if (ox_string_length(ctxt, TOK->v))
                        ARRAY_APPEND(templ, TOK->v);
                    else
                        ARRAY_APPEND(templ, ox_value_null(ctxt));

                    if (TOK->type == OX_TOKEN_STR_TAIL)
                        break;
                } else {
                    unexpect_token_error(ctxt, p, LOC, OX_TOKEN_STR_MID, OX_TOKEN_STR_TAIL, -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_STR_PART)) == OX_ERR)
                    break;
            }
        }
    } else {
        r = OX_OK;
    }

    first_loc(&loc, LOC);

    last_loc(&loc, LOC);
    SET_LOC(s, &loc);
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Check if the expression is a valid condition expression.*/
static OX_Bool
is_cond_expr (OX_Context *ctxt, OX_Value *expr)
{
    OX_AstType type;

    type = ox_ast_get_type(ctxt, expr);
    if (type == OX_AST_assi)
        return OX_FALSE;

    if (type == OX_AST_comma) {
        OX_VS_PUSH_2(ctxt, items, item)
        size_t len;
        OX_Bool b;

        AST_GET(expr, items, items);
        len = ox_array_length(ctxt, items);
        ox_array_get_item(ctxt, items, len - 1, item);

        b = is_cond_expr(ctxt, item);

        OX_VS_POP(ctxt, items)
        return b;
    }

    return OX_TRUE;
}

/*Check if the expression is a left expression.*/
static OX_Result
is_left_expr (OX_Context *ctxt, OX_Value *expr)
{
    OX_VS_PUSH(ctxt, op)
    OX_AstType type;
    OX_Bool b;

    type = ox_ast_get_type(ctxt, expr);

    switch (type) {
    case OX_AST_id:
    case OX_AST_array_pattern:
    case OX_AST_object_pattern:
        b = OX_TRUE;
        break;
    case OX_AST_unary_expr:
        AST_GET(expr, operator, op);

        type = ox_ast_get_type(ctxt, op);
        b = (type == OX_AST_get_value);
        break;
    case OX_AST_binary_expr:
        AST_GET(expr, operator, op);

        type = ox_ast_get_type(ctxt, op);
        b = (type == OX_AST_get) || (type == OX_AST_lookup);
        break;
    case OX_AST_parenthese:
        AST_GET(expr, expr, op);

        b = is_left_expr(ctxt, op);
        break;
    default:
        b = OX_FALSE;
        break;
    }

    OX_VS_POP(ctxt, op)
    return b;
}

/*Get the condition expression.*/
static OX_Result
cond_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_Result r;

    if ((r = expression(ctxt, p, expr)) == OX_ERR)
        return r;

    if (!is_cond_expr(ctxt, expr)) {
        OX_Location loc;

        GET_LOC(expr, &loc);
        warning(ctxt, p, &loc, OX_TEXT("expect `(\' and `)\' around the expression"));
    }

    return OX_OK;
}

/*Check if the expression is a left hand expression.*/
static OX_Result
check_left_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    if (!is_left_expr(ctxt, expr)) {
        OX_Location loc;

        GET_LOC(expr, &loc);
        error(ctxt, p, &loc, OX_TEXT("the expression is not a left value"));
    }

    return OX_OK;
}

/*check if the value is an valid identifier.*/
static OX_Result
check_id (OX_Context *ctxt, OX_Parser *p)
{
    if ((TOK->keyword != OX_KEYWORD_NONE)
            && (TOK->keyword != OX_KEYWORD_as)
            && (TOK->keyword != OX_KEYWORD_elif)
            && (TOK->keyword != OX_KEYWORD_else)
            && (TOK->keyword != OX_KEYWORD_catch)
            && (TOK->keyword != OX_KEYWORD_finally)
            && (TOK->keyword != OX_KEYWORD_enum)
            && (TOK->keyword != OX_KEYWORD_bitfield)
            && (TOK->keyword != OX_KEYWORD_textdomain)) {
        error(ctxt, p, LOC, OX_TEXT("keyword `%s\' cannot be used as identifier"),
                ox_string_get_char_star(ctxt, TOK->v));
        return OX_ERR;
    }

    return OX_OK;
}

/*Parse the array item.*/
static OX_Result
array_item (OX_Context *ctxt, OX_Parser *p, OX_Value *item)
{
    OX_VS_PUSH_2(ctxt, e, doc)
    OX_Result r;

    ox_value_copy(ctxt, doc, p->doc);
    ox_value_set_null(ctxt, p->doc);

    if (TOK->type == ',') {
        AST_NEW(item, skip);
        SET_LOC(item, LOC);
        r = OX_OK;
    } else if (TOK->type == OX_TOKEN_dot_dot_dot) {
        AST_NEW(item, spread);
        SET_LOC(item, LOC);

        r = expression_prio(ctxt, p, e, OX_PRIO_COMMA);
        if (r == OX_OK)
            AST_SET(item, expr, e);
    } else if (is_keyword(TOK, OX_KEYWORD_if)) {
        r = if_expr(ctxt, p, item, OX_BLOCK_CONTENT_ITEM);
    } else if (is_keyword(TOK, OX_KEYWORD_case)) {
        r = case_expr(ctxt, p, item, OX_BLOCK_CONTENT_ITEM);
    } else {
        unget_token(ctxt, p);

        r = expression_prio(ctxt, p, item, OX_PRIO_COMMA);
    }

    if (r == OX_OK) {
        if (!ox_value_is_null(ctxt, doc))
            AST_SET(item, doc, doc);
    }

    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse the array literal.*/
static OX_Result
array_literal (OX_Context *ctxt, OX_Parser *p, OX_Value *a, OX_AstType type)
{
    OX_VS_PUSH_2(ctxt, item, items)
    int old_flags = p->flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    ast_new(ctxt, a, type);
    ARRAY_NEW(items);
    AST_SET(a, items, items);

    p->flags |= OX_PARSER_FL_OWNED;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == ']')
            break;

        r = array_item(ctxt, p, item);

        if (r == OX_OK) {
            ARRAY_APPEND(items, item);

            if (ox_ast_get_type(ctxt, item) != OX_AST_skip) {
                get_token(ctxt, p);

                if (TOK->type == ']')
                    break;

                if (TOK->type == ',') {
                } else if (NEWLINE) {
                    unget_token(ctxt, p);
                } else {
                    unexpect_token_error(ctxt, p, LOC, ',', ']', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RSB)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(a, &loc);

    p->flags = old_flags;
    OX_VS_POP(ctxt, item)
    return OX_OK;
}

/*Enumeration define.*/
static OX_Result
enum_def (OX_Context *ctxt, OX_Parser *p, OX_Value *e)
{
    OX_Result r;
    OX_Location loc;
    OX_VS_PUSH_4(ctxt, items, item, name, doc)

    first_loc(&loc, LOC);

    AST_NEW(e, enum);
    ARRAY_NEW(items);
    AST_SET(e, items, items);

    get_token(ctxt, p);
    if (TOK->type == OX_TOKEN_ID) {
        check_id(ctxt, p);

        AST_NEW(name, id);
        AST_SET(name, value, TOK->v);
        SET_LOC(name, LOC);
        AST_SET(e, name, name);
    } else {
        unget_token(ctxt, p);
    }

    if ((r = get_expect_token(ctxt, p, '{')) == OX_ERR)
        goto end;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        ox_value_copy(ctxt, doc, p->doc);
        ox_value_set_null(ctxt, p->doc);

        if (TOK->type != OX_TOKEN_ID) {
            unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, -1);
            unget_token(ctxt, p);
            r = OX_ERR;
        } else {
            check_id(ctxt, p);

            AST_NEW(item, id);
            AST_SET(item, value, TOK->v);
            SET_LOC(item, LOC);
            ARRAY_APPEND(items, item);

            if (!ox_value_is_null(ctxt, doc))
                AST_SET(item, doc, doc);

            r = OX_OK;
        }

        if (r == OX_OK) {
            get_token(ctxt, p);
            if (TOK->type == ',') {
            } else if (TOK->type == '}') {
                break;
            } else if (NEWLINE) {
                unget_token(ctxt, p);
            } else {
                unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                unget_token(ctxt, p);
                r = OX_ERR;
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(e, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, items)
    return r;
}

/*Bitfield define.*/
static OX_Result
bitfield_def (OX_Context *ctxt, OX_Parser *p, OX_Value *b)
{
    OX_Result r;
    OX_Location loc;
    OX_VS_PUSH_4(ctxt, items, item, name, doc)

    first_loc(&loc, LOC);

    AST_NEW(b, bitfield);
    ARRAY_NEW(items);
    AST_SET(b, items, items);

    get_token(ctxt, p);
    if (TOK->type == OX_TOKEN_ID) {
        check_id(ctxt, p);

        AST_NEW(name, id);
        AST_SET(name, value, TOK->v);
        SET_LOC(name, LOC);
        AST_SET(b, name, name);
    } else {
        unget_token(ctxt, p);
    }

    if ((r = get_expect_token(ctxt, p, '{')) == OX_ERR)
        goto end;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        ox_value_copy(ctxt, doc, p->doc);
        ox_value_set_null(ctxt, p->doc);

        if (TOK->type != OX_TOKEN_ID) {
            unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, -1);
            unget_token(ctxt, p);
            r = OX_ERR;
        } else {
            check_id(ctxt, p);

            AST_NEW(item, id);
            AST_SET(item, value, TOK->v);
            SET_LOC(item, LOC);
            ARRAY_APPEND(items, item);

            if (!ox_value_is_null(ctxt, doc))
                AST_SET(item, doc, doc);

            r = OX_OK;
        }

        if (r == OX_OK) {
            get_token(ctxt, p);
            if (TOK->type == ',') {
            } else if (TOK->type == '}') {
                break;
            } else if (NEWLINE) {
                unget_token(ctxt, p);
            } else {
                unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                unget_token(ctxt, p);
                r = OX_ERR;
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(b, &loc);

    if (ox_array_length(ctxt, items) > 32)
        error(ctxt, p, &loc, OX_TEXT("bitfield's items must <= 32"));

    r = OX_OK;
end:
    OX_VS_POP(ctxt, items)
    return r;
}

/*Parse current property expression.*/
static OX_Result
curr_prop_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *e, OX_Value *pn)
{
    OX_VS_PUSH_2(ctxt, t, v)
    OX_Location tloc;
    OX_AstType aty;

    GET_LOC(pn, &tloc);
    AST_NEW(e, binary_expr);
    SET_LOC(e, &tloc);
    AST_NEW(t, curr_object);
    SET_LOC(t, &tloc);
    AST_SET(e, operand1, t);
    AST_NEW(t, get);
    SET_LOC(t, &tloc);
    AST_SET(e, operator, t);

    aty = ox_ast_get_type(ctxt, pn);

    if (aty == OX_AST_expr_name) {
        AST_GET(pn, expr, t);
        AST_SET(e, operand2, t);
    } else if (aty == OX_AST_id) {
        OX_Location nloc;

        AST_NEW(t, value);
        AST_GET(pn, value, v);
        AST_SET(t, value, v);
        GET_LOC(pn, &nloc);
        SET_LOC(t, &nloc);
        AST_SET(e, operand2, t);
    } else {
        AST_SET(e, operand2, pn);
    }

    OX_VS_POP(ctxt, t)
    return OX_OK;
}

/*Parse the object item.*/
static OX_Result
object_item (OX_Context *ctxt, OX_Parser *p, OX_Value *prop)
{
    OX_VS_PUSH_5(ctxt, pn, e, args, items, doc)
    OX_Result r;

    ox_value_copy(ctxt, doc, p->doc);
    ox_value_set_null(ctxt, p->doc);

    if (TOK->type == OX_TOKEN_dot_dot_dot) {
        AST_NEW(prop, spread);
        SET_LOC(prop, LOC);

        r = expression_prio(ctxt, p, e, OX_PRIO_COMMA);
        if (r == OX_OK)
            AST_SET(prop, expr, e);
    } else if (is_keyword(TOK, OX_KEYWORD_if)) {
        r = if_expr(ctxt, p, prop, OX_BLOCK_CONTENT_PROP);
    } else if (is_keyword(TOK, OX_KEYWORD_case)) {
        r = case_expr(ctxt, p, prop, OX_BLOCK_CONTENT_PROP);
    } else if (is_keyword(TOK, OX_KEYWORD_enum)) {
        r = enum_def(ctxt, p, prop);
    } else if (is_keyword(TOK, OX_KEYWORD_bitfield)) {
        r = bitfield_def(ctxt, p, prop);
    } else if (TOK->type == '(') {
        OX_Bool is_func;
        OX_Location loc;

        first_loc(&loc, LOC);

        unget_token(ctxt, p);

        r = expr_or_parenthese_func(ctxt, p, e, &is_func, OX_PRIO_HIGHEST);
        if (r == OX_OK) {
            last_loc(&loc, LOC);

            AST_NEW(prop, call);
            SET_LOC(prop, &loc);
            AST_SET(prop, expr, e);
            AST_NEW(args, args);
            SET_LOC(args, &loc);
            AST_SET(prop, args, args);
            ARRAY_NEW(items);
            AST_SET(args, items, items);
            AST_NEW(e, curr_object);
            SET_LOC(e, &loc);
            ARRAY_APPEND(items, e);

            r = expression_tail_prio(ctxt, p, prop, OX_PRIO_COMMA);
        }
    } else {
        unget_token(ctxt, p);

        r = prop_name(ctxt, p, pn);

        if (r == OX_OK) {
            get_token(ctxt, p);

            if (TOK->type == ':') {
                AST_NEW(prop, prop);
                AST_SET(prop, name, pn);

                r = expression_prio(ctxt, p, e, OX_PRIO_COMMA);
                if (r == OX_OK) {
                    OX_Location ploc;

                    if (ox_ast_get_type(ctxt, e) == OX_AST_func)
                        AST_SET_B(e, this, OX_TRUE);

                    GET_LOC(pn, &ploc);
                    last_loc(&ploc, LOC);
                    SET_LOC(prop, &ploc);
                    AST_SET(prop, expr, e);
                }
            } else if (TOK->type == '(') {
                OX_Location tloc;

                first_loc(&tloc, LOC);

                r = curr_prop_expr(ctxt, p, e, pn);

                if (r == OX_OK) {
                    AST_NEW(prop, call);
                    AST_SET(prop, expr, e);

                    r = arguments(ctxt, p, args);
                }

                if (r == OX_OK) {
                    last_loc(&tloc, LOC);

                    AST_SET(prop, args, args);
                    SET_LOC(prop, &tloc);

                    r = expression_tail_prio(ctxt, p, prop, OX_PRIO_COMMA);
                }
            } else if (TOK->type == '.') {
                r = curr_prop_expr(ctxt, p, prop, pn);

                if (r == OX_OK)
                    r = dot_expr(ctxt, p, prop);
                if (r == OX_OK) {
                    r = expression_tail_prio(ctxt, p, prop, OX_PRIO_COMMA);
                }
            } else {
                OX_AstType type = ox_ast_get_type(ctxt, pn);

                if (type != OX_AST_id) {
                    unexpect_token_error(ctxt, p, LOC, ':', '(', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                } else {
                    OX_Location ploc;

                    unget_token(ctxt, p);

                    GET_LOC(pn, &ploc);
                    AST_NEW(prop, prop);
                    AST_SET(prop, name, pn);
                    AST_SET(prop, expr, pn);
                    SET_LOC(prop, &ploc);
                }
            }
        }
    }

    if (r == OX_OK) {
        if (!ox_value_is_null(ctxt, doc))
            AST_SET(prop, doc, doc);
    }

    OX_VS_POP(ctxt, pn)
    return r;
}

/*Parse the object literal.*/
static OX_Result
object_literal (OX_Context *ctxt, OX_Parser *p, OX_Value *o, OX_AstType type)
{
    OX_VS_PUSH_2(ctxt, prop, props)
    int old_flags = p->flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    ast_new(ctxt, o, type);
    ARRAY_NEW(props);
    AST_SET(o, props, props);

    p->flags |= OX_PARSER_FL_OWNED;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        r = object_item(ctxt, p, prop);

        if (r == OX_OK) {
            ARRAY_APPEND(props, prop);

            get_token(ctxt, p);

            if (TOK->type == '}')
                break;

            if (TOK->type == ',') {
            } else if (NEWLINE) {
                unget_token(ctxt, p);
            } else {
                unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                unget_token(ctxt, p);
                r = OX_ERR;
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(o, &loc);

    p->flags = old_flags;
    OX_VS_POP(ctxt, prop)
    return OX_OK;
}

/* ( expression ) */
static OX_Result
parenthese_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH(ctxt, e)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(expr, parenthese);

    if ((r = expression(ctxt, p, e)) == OX_ERR)
        return r;

    if ((r = get_expect_token(ctxt, p, ')')) == OX_ERR)
        return r;

    last_loc(&loc, LOC);
    AST_SET(expr, expr, e);
    SET_LOC(expr, &loc);

    OX_VS_POP(ctxt, e)
    return OX_OK;
}

/*Parse an unary expression.*/
static OX_Result
unary_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_AstType type)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(expr, unary_expr);
    ast_new(ctxt, op, type);
    SET_LOC(op, LOC);
    AST_SET(expr, operator, op);

    if ((r = expression_prio(ctxt, p, e, OX_PRIO_UNARY)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    AST_SET(expr, operand1, e);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, op)
    return r;
}

/*Parse a yield expression.*/
static OX_Result
yield_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(expr, unary_expr);
    ast_new(ctxt, op, OX_AST_yield);
    SET_LOC(op, LOC);
    AST_SET(expr, operator, op);

    get_token(ctxt, p);

    if (NEWLINE || (TOK->type == ';')) {
        unget_token(ctxt, p);

        AST_NEW(e, value);
        AST_SET(e, value, ox_value_null(ctxt));
        SET_LOC(e, LOC);
    } else {
        unget_token(ctxt, p);

        if ((r = expression_prio(ctxt, p, e, OX_PRIO_UNARY)) == OX_ERR)
            goto end;
    }

    last_loc(&loc, LOC);
    AST_SET(expr, operand1, e);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, op)
    return r;
}

/*Parse get member expression.*/
static OX_Result
get_member_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Location loc;

    GET_LOC(expr, &loc);

    AST_NEW(e, binary_expr);
    AST_NEW(op, get);
    SET_LOC(op, LOC);
    AST_SET(e, operator, op);
    AST_SET(e, operand1, expr);
    ox_value_copy(ctxt, expr, e);

    AST_NEW(e, value);
    AST_SET(e, value, TOK->v);
    SET_LOC(e, LOC);

    if (TOK->type == OX_TOKEN_HASH_ID)
        AST_SET_B(e, private, OX_TRUE);

    last_loc(&loc, LOC);
    AST_SET(expr, operand2, e);
    SET_LOC(expr, &loc);

    OX_VS_POP(ctxt, op)
    return OX_OK;
}

/* . expression */
static OX_Result
dot_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_4(ctxt, e, call, args, items)
    OX_Result r;

    get_token(ctxt, p);

    if ((TOK->type == OX_TOKEN_ID) || (TOK->type == OX_TOKEN_HASH_ID)) {
        r = get_member_expr(ctxt, p, expr);
    } else if (TOK->type == '(') {
        OX_Bool is_func;
        OX_Location loc;

        unget_token(ctxt, p);
        if ((r = expr_or_parenthese_func(ctxt, p, e, &is_func, OX_PRIO_HIGHEST)) == OX_ERR)
            goto end;

        GET_LOC(e, &loc);

        AST_NEW(call, call);
        SET_LOC(call, &loc);
        AST_SET(call, expr, e);
        AST_NEW(args, args);
        SET_LOC(args, &loc);
        AST_SET(call, args, args);
        ARRAY_NEW(items);
        AST_SET(args, items, items);
        ARRAY_APPEND(items, expr);
        ox_value_copy(ctxt, expr, call);
    } else if (TOK->type == '[') {
        r = array_literal(ctxt, p, e, OX_AST_array_append);
        if (r == OX_OK) {
            AST_SET(e, expr, expr);
            ox_value_copy(ctxt, expr, e);
        }
    } else if (TOK->type == '{') {
        r = object_literal(ctxt, p, e, OX_AST_object_set);
        if (r == OX_OK) {
            AST_SET(e, expr, expr);
            ox_value_copy(ctxt, expr, e);
        }
    } else {
        unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, '[', '{', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
    }
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse lookup expression.*/
static OX_Result
lookup_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Result r;
    OX_Location loc, op_loc;

    GET_LOC(expr, &loc);
    first_loc(&op_loc, LOC);

    AST_NEW(e, binary_expr);
    AST_NEW(op, lookup);
    AST_SET(e, operator, op);
    AST_SET(e, operand1, expr);
    ox_value_copy(ctxt, expr, e);

    if ((r = expression(ctxt, p, e)) == OX_ERR)
        goto end;

    if ((r = get_expect_token(ctxt, p, ']')) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    last_loc(&op_loc, LOC);
    AST_SET(expr, operand2, e);
    SET_LOC(expr, &loc);
    SET_LOC(op, &op_loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, op)
    return r;
}

/*Parse an expression or function in parentheses.*/
static OX_Result
expr_or_parenthese_func (OX_Context *ctxt, OX_Parser *p, OX_Value *e, OX_Bool *is_func, OX_Priority prio)
{
    OX_VS_PUSH_7(ctxt, f, old_func, blk, old_blk, s, op, ast)
    OX_Result r;

    func_new(ctxt, p, f);
    ox_value_copy(ctxt, old_func, p->func);
    ox_value_copy(ctxt, p->func, f);

    if (is_func)
        *is_func = OX_FALSE;

    r = expression_prio(ctxt, p, e, prio);

    if (r == OX_OK) {
        OX_AstType type = ox_ast_get_type(ctxt, e);

        if (type == OX_AST_unary_expr) {
            AST_GET(e, operator, op);
            if (ox_ast_get_type(ctxt, op) == OX_AST_global) {
                AST_GET(e, operand1, ast);
                type = ox_ast_get_type(ctxt, ast);
            }
        }

        if (type == OX_AST_parenthese) {
            OX_Location loc;

            AST_NEW(blk, block);
            ox_value_copy(ctxt, old_blk, p->block);
            ox_value_copy(ctxt, p->block, blk);
            AST_SET(f, block, blk);

            GET_LOC(e, &loc);
            SET_LOC(f, &loc);
            SET_LOC(blk, &loc);
            AST_NEW(s, return);
            SET_LOC(s, &loc);
            AST_SET(s, expr, e);

            add_stmt(ctxt, p, s);
            add_func(ctxt, p, f);

            ox_value_copy(ctxt, e, f);

            ox_value_copy(ctxt, p->block, old_blk);
            ox_value_copy(ctxt, p->func, old_func);

            if (is_func)
                *is_func = OX_TRUE;
        } else {
            ox_value_copy(ctxt, p->func, old_func);
            dup_decls(ctxt, p, f);
        }
    } else {
        ox_value_copy(ctxt, p->func, old_func);
    }

    OX_VS_POP(ctxt, f)
    return r;
}

/*Parse arguments.*/
static OX_Result
arguments (OX_Context *ctxt, OX_Parser *p, OX_Value *args)
{
    OX_VS_PUSH_3(ctxt, e, items, item)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(args, args);
    ARRAY_NEW(items);
    AST_SET(args, items, items);

    get_token(ctxt, p);

    if (TOK->type != ')') {
        unget_token(ctxt, p);

        while (1) {
            get_token(ctxt, p);

            if (TOK->type == OX_TOKEN_dot_dot_dot) {
                AST_NEW(item, spread);
                SET_LOC(item, LOC);

                r = expression_prio(ctxt, p, e, OX_PRIO_COMMA);
                if (r == OX_OK)
                    AST_SET(item, expr, e);
            } else {
                unget_token(ctxt, p);

                r = expr_or_parenthese_func(ctxt, p, item, NULL, OX_PRIO_COMMA);
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                get_token(ctxt, p);
                if (TOK->type == ')')
                    break;
                if (TOK->type != ',') {
                    unexpect_token_error(ctxt, p, LOC, ',', ')', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RP)) == OX_ERR)
                    break;
            }
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(args, &loc);

    OX_VS_POP(ctxt, items)
    return OX_OK;
}

/*Parse call expression.*/
static OX_Result
call_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_2(ctxt, e, args)
    OX_Location loc;
    OX_Result r;

    GET_LOC(expr, &loc);

    AST_NEW(e, call);
    AST_SET(e, expr, expr);
    ox_value_copy(ctxt, expr, e);

    r = arguments(ctxt, p, args);

    if (r == OX_OK) {
        last_loc(&loc, LOC);
        SET_LOC(expr, &loc);
        AST_SET(expr, args, args);
    }

    OX_VS_POP(ctxt, e)
    return r;
}

/*Question expression.*/
static OX_Result
ques_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_Result r;
    OX_Bool loop;

    get_token(ctxt, p);

    switch (TOK->type) {
    case '.':
        r = dot_expr(ctxt, p, expr);
        break;
    case '(':
        r = call_expr(ctxt, p, expr);
        break;
    case '[':
        r = lookup_expr(ctxt, p, expr);
        break;
    default:
        unexpect_token_error(ctxt, p, LOC, '.', '[', '(', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
    }

    if (r == OX_ERR)
        return r;

    AST_SET_B(expr, ques_src, OX_TRUE);

    loop = OX_TRUE;
    do {
        get_token(ctxt, p);

        switch (TOK->type) {
        case '.':
            r = dot_expr(ctxt, p, expr);
            break;
        case '(':
            r = call_expr(ctxt, p, expr);
            break;
        case '[':
            r = lookup_expr(ctxt, p, expr);
            break;
        case '?':
            r = ques_expr(ctxt, p, expr);
            break;
        default:
            unget_token(ctxt, p);
            loop = OX_FALSE;
            break;
        }

        if (r == OX_ERR)
            return r;

        AST_SET_B(expr, ques_src, OX_TRUE);
    } while (loop);

    AST_SET_B(expr, ques_dst, OX_TRUE);

    return OX_OK;
}

/*Parse binary expression.*/
static OX_Result
binary_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_Priority prio, OX_AstType type)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Result r;
    OX_Location loc;

    GET_LOC(expr, &loc);

    AST_NEW(e, binary_expr);
    ast_new(ctxt, op, type);
    SET_LOC(op, LOC);
    AST_SET(e, operator, op);
    AST_SET(e, operand1, expr);
    ox_value_copy(ctxt, expr, e);

    if ((r = expression_prio(ctxt, p, e, prio)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    AST_SET(expr, operand2, e);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, op)
    return r;
}

/*Parse assignment expression.*/
static OX_Result
assi_op_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_AstType type, OX_DeclType dty, OX_Bool get_old)
{
    OX_VS_PUSH_2(ctxt, op, e)
    OX_Result r;
    OX_Location loc;
    OX_AstType aty = ox_ast_get_type(ctxt, expr);

    if (aty == OX_AST_id) {
        add_decl_ast(ctxt, p, expr, dty);

        if (!ox_value_is_null(ctxt, p->doc)) {
            AST_SET(expr, doc, p->doc);
            ox_value_set_null(ctxt, p->doc);
        }
    }

    check_left_expr(ctxt, p, expr);

    GET_LOC(expr, &loc);

    AST_NEW(e, assi);

    ast_new(ctxt, op, type);
    SET_LOC(op, LOC);
    AST_SET(e, operator, op);

    if (get_old) {
        AST_SET_B(e, get, OX_TRUE);
    }

    AST_SET(e, left, expr);
    ox_value_copy(ctxt, expr, e);

    if ((r = expression_prio(ctxt, p, e, OX_PRIO_ASSI)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    AST_SET(expr, right, e);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, op)
    return r;
}

/*Parse assignment expression.*/
static OX_Result
assi_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_DeclType dty)
{
    return assi_op_expr(ctxt, p, expr, OX_AST_none, dty, OX_FALSE);
}

/*Get default expression.*/
static OX_Result
default_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_Result r;

    get_token(ctxt, p);

    if (TOK->type == '=') {
        r = expression_prio(ctxt, p, expr, OX_PRIO_COMMA);
    } else {
        unget_token(ctxt, p);
        ox_value_set_null(ctxt, expr);
        r = OX_OK;
    }

    return r;
}

/*Parse assignment pattern.*/
static OX_Result
assi_pattern (OX_Context *ctxt, OX_Parser *p, OX_Value *pat, OX_DeclType dty)
{
    OX_VS_PUSH_4(ctxt, e, items, item, pn)
    OX_AstType ast_type;
    OX_Result r;
    OX_Location loc;

    get_token(ctxt, p);

    switch (TOK->type) {
    case '[':
        first_loc(&loc, LOC);

        AST_NEW(pat, array_pattern);
        ARRAY_NEW(items);
        AST_SET(pat, items, items);

        while (1) {
            OX_Bool is_left = OX_FALSE;

            get_token(ctxt, p);

            if (TOK->type == ']')
                break;

            if (TOK->type == ',') {
                AST_NEW(item, skip);
                ARRAY_APPEND(items, item);
                continue;
            } else if (TOK->type == OX_TOKEN_dot_dot_dot) {
                AST_NEW(item, rest);
                SET_LOC(item, LOC);

                r = assi_pattern(ctxt, p, e, dty);
                if (r == OX_OK)
                    AST_SET(item, pattern, e);

                is_left = OX_TRUE;
            } else {
                unget_token(ctxt, p);

                AST_NEW(item, item_pattern);

                r = assi_pattern(ctxt, p, e, dty);
                if (r == OX_OK) {
                    AST_SET(item, pattern, e);

                    r = default_expr(ctxt, p, e);
                    if (r == OX_OK)
                        AST_SET(item, expr, e);
                }
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                get_token(ctxt, p);
                if (TOK->type == ']')
                    break;

                if (is_left) {
                    unexpect_token_error(ctxt, p, LOC, ']', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                } else if (TOK->type == ',') {
                } else if (NEWLINE) {
                    unget_token(ctxt, p);
                } else {
                    unexpect_token_error(ctxt, p, LOC, ',', ']', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RSB)) == OX_ERR)
                    break;
            }
        }

        last_loc(&loc, LOC);
        SET_LOC(pat, &loc);
        break;
    case '{':
        first_loc(&loc, LOC);

        AST_NEW(pat, object_pattern);
        ARRAY_NEW(items);
        AST_SET(pat, props, items);

        while (1) {
            OX_Bool is_left = OX_FALSE;

            get_token(ctxt, p);

            if (TOK->type == '}')
                break;

            if (TOK->type == OX_TOKEN_dot_dot_dot) {
                AST_NEW(item, rest);
                SET_LOC(item, LOC);

                r = assi_pattern(ctxt, p, e, dty);
                if (r == OX_OK)
                    AST_SET(item, pattern, e);

                is_left = OX_TRUE;
            } else {
                unget_token(ctxt, p);

                AST_NEW(item, prop_pattern);

                r = prop_name(ctxt, p, pn);

                if (r == OX_OK) {
                    AST_SET(item, name, pn);

                    get_token(ctxt, p);

                    if (TOK->type == ':') {
                        r = assi_pattern(ctxt, p, e, dty);
                        if (r == OX_OK) {
                            AST_SET(item, pattern, e);

                            r = default_expr(ctxt, p, e);
                            if (r == OX_OK)
                                AST_SET(item, expr, e);
                        }
                    } else {
                        OX_AstType type = ox_ast_get_type(ctxt, pn);

                        if (type != OX_AST_id) {
                            unexpect_token_error(ctxt, p, LOC, ':', -1);
                            unget_token(ctxt, p);
                            r = OX_ERR;
                        }
                        
                        if (r == OX_OK) {
                            AST_SET(item, pattern, pn);
                            add_decl_ast(ctxt, p, pn, dty);

                            if (TOK->type == '=') {
                                r = expression_prio(ctxt, p, e, OX_PRIO_COMMA);
                                if (r == OX_OK)
                                    AST_SET(item, expr, e);
                            } else {
                                unget_token(ctxt, p);
                            }
                        }

                        if (r == OX_OK)
                            AST_SET(item, pattern, pn);
                    }
                }
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                get_token(ctxt, p);
                if (TOK->type == '}')
                    break;

                if (is_left) {
                    unexpect_token_error(ctxt, p, LOC, '}', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                } else if (TOK->type == ',') {
                } else if (NEWLINE) {
                    unget_token(ctxt, p);
                } else {
                    unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RB)) == OX_ERR)
                    break;
            }
        }

        last_loc(&loc, LOC);
        SET_LOC(pat, &loc);
        break;
    default:
        unget_token(ctxt, p);
        if ((r = expression_prio(ctxt, p, pat, OX_PRIO_UNARY)) == OX_ERR)
            goto end;

        check_left_expr(ctxt, p, pat);

        ast_type = ox_ast_get_type(ctxt, pat);
        if (ast_type == OX_AST_id)
            add_decl_ast(ctxt, p, pat, dty);
        break;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse reverse assignment expression.*/
static OX_Result
rev_assi_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_DeclType dty)
{
    OX_VS_PUSH(ctxt, e)
    OX_Location loc;
    OX_Result r;

    GET_LOC(expr, &loc);

    AST_NEW(e, rev_assi);
    AST_SET(e, right, expr);
    ox_value_copy(ctxt, expr, e);

    if ((r = assi_pattern(ctxt, p, e, dty)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    SET_LOC(expr, &loc);
    AST_SET(expr, left, e);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse comma expression.*/
static OX_Result
comma_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_VS_PUSH_2(ctxt, e, items)
    OX_Location loc;
    OX_AstType type;
    OX_Result r;

    GET_LOC(expr, &loc);

    type = ox_ast_get_type(ctxt, expr);
    if (type != OX_AST_comma) {
        AST_NEW(e, comma);
        ARRAY_NEW(items);
        AST_SET(e, items, items);
        ARRAY_APPEND(items, expr);
        ox_value_copy(ctxt, expr, e);
    } else {
        AST_GET(expr, items, items);
    }

    if ((r = expression_prio(ctxt, p, e, OX_PRIO_COMMA)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);

    ARRAY_APPEND(items, e);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse the property name.*/
static OX_Result
prop_name (OX_Context *ctxt, OX_Parser *p, OX_Value *pn)
{
    OX_VS_PUSH(ctxt, e)
    OX_Location loc;
    OX_Result r;

    get_token(ctxt, p);

    switch (TOK->type) {
    case OX_TOKEN_ID:
    case OX_TOKEN_HASH_ID:
        AST_NEW(pn, id);
        AST_SET(pn, value, TOK->v);
        SET_LOC(pn, LOC);

        if (TOK->type == OX_TOKEN_HASH_ID)
            AST_SET_B(pn, private, OX_TRUE);
        break;
    case OX_TOKEN_STRING:
    case OX_TOKEN_NUMBER:
        AST_NEW(pn, value);
        AST_SET(pn, value, TOK->v);
        SET_LOC(pn, LOC);
        break;
    case OX_TOKEN_STR_HEAD:
        if ((r = multipart_string(ctxt, p, NULL, pn)) == OX_ERR)
            goto end;
        break;
    case '[':
        if ((r = expression(ctxt, p, e)) == OX_ERR)
            goto end;

        if ((r = get_expect_token(ctxt, p, ']')) == OX_ERR)
            goto end;

        AST_NEW(pn, expr_name);
        AST_SET(pn, expr, e);
        GET_LOC(e, &loc);
        SET_LOC(pn, &loc);
        break;
    default:
        unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, OX_TOKEN_NUMBER, OX_TOKEN_STRING, '[', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse the parameter pattern.*/
static OX_Result
param_pattern (OX_Context *ctxt, OX_Parser *p, OX_Value *pat)
{
    OX_VS_PUSH_5(ctxt, expr, items, item, ipat, pn)
    OX_Location loc;
    OX_Result r;

    get_token(ctxt, p);

    first_loc(&loc, LOC);

    switch (TOK->type) {
    case OX_TOKEN_ID:
        /*Parameter identifier.*/
        AST_NEW(pat, id);
        AST_SET(pat, value, TOK->v);
        SET_LOC(pat, LOC);

        add_decl_ast(ctxt, p, pat, OX_DECL_PARAM);
        break;
    case '[':
        /*Array match pattern.*/
        AST_NEW(pat, array_pattern);
        ARRAY_NEW(items);
        AST_SET(pat, items, items);

        while (1) {
            OX_Bool is_left = OX_FALSE;

            get_token(ctxt, p);

            if (TOK->type == ']')
                break;

            if (TOK->type == ',') {
                AST_NEW(item, skip);
                SET_LOC(item, LOC);
                ARRAY_APPEND(items, item);
                continue;
            }

            if (TOK->type == OX_TOKEN_dot_dot_dot) {
                AST_NEW(item, rest);
                SET_LOC(item, LOC);

                r = param_pattern(ctxt, p, ipat);
                if (r == OX_OK)
                    AST_SET(item, pattern, ipat);

                is_left = OX_TRUE;
            } else {
                unget_token(ctxt, p);

                AST_NEW(item, item_pattern);

                r = param_pattern(ctxt, p, ipat);
                if (r == OX_OK) {
                    AST_SET(item, pattern, ipat);
                    r = default_expr(ctxt, p, expr);
                    if (r == OX_OK)
                        AST_SET(item, expr, expr);
                }
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                get_token(ctxt, p);
                if (TOK->type == ']')
                    break;

                if (is_left) {
                    unexpect_token_error(ctxt, p, LOC, ']', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                } else if (TOK->type == ',') {
                } else if (NEWLINE) {
                    unget_token(ctxt, p);
                } else {
                    unexpect_token_error(ctxt, p, LOC, ',', ']', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RSB)) == OX_ERR)
                    break;
            }
        }
        break;
    case '{':
        /*Object match pattern.*/
        AST_NEW(pat, object_pattern);
        ARRAY_NEW(items);
        AST_SET(pat, props, items);

        while (1) {
            OX_Bool is_left = OX_FALSE;

            get_token(ctxt, p);

            if (TOK->type == '}')
                break;

            if (TOK->type == OX_TOKEN_dot_dot_dot) {
                AST_NEW(item, rest);
                SET_LOC(item, LOC);

                r = param_pattern(ctxt, p, ipat);

                if (r == OX_OK)
                    AST_SET(item, pattern, ipat);

                is_left = OX_TRUE;
            } else {
                unget_token(ctxt, p);

                AST_NEW(item, prop_pattern);

                r = prop_name(ctxt, p, pn);

                if (r == OX_OK) {
                    AST_SET(item, name, pn);

                    get_token(ctxt, p);

                    if (TOK->type == ':') {
                        r = param_pattern(ctxt, p, ipat);

                        if (r == OX_OK) {
                            AST_SET(item, pattern, ipat);
                            add_decl_ast(ctxt, p, ipat, OX_DECL_PARAM);

                            r = default_expr(ctxt, p, expr);
                            if (r == OX_OK)
                                AST_SET(item, expr, expr);
                        }
                    } else {
                        OX_AstType type = ox_ast_get_type(ctxt, pn);

                        if (type != OX_AST_id) {
                            unexpect_token_error(ctxt, p, LOC, ':', -1);
                            unget_token(ctxt, p);
                            r = OX_ERR;
                        }

                        if (r == OX_OK) {
                            AST_SET(item, pattern, pn);
                            add_decl_ast(ctxt, p, pn, OX_DECL_PARAM);

                            if (TOK->type == '=') {
                                r = expression_prio(ctxt, p, expr, OX_PRIO_COMMA);
                                if (r == OX_OK)
                                    AST_SET(item, expr, expr);
                            } else {
                                unget_token(ctxt, p);
                            }
                        }
                    }
                }
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                get_token(ctxt, p);
                if (TOK->type == '}')
                    break;

                if (is_left) {
                    unexpect_token_error(ctxt, p, LOC, '}', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                } else if (TOK->type == ',') {
                } else if (NEWLINE) {
                    unget_token(ctxt, p);
                } else {
                    unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                    unget_token(ctxt, p);
                    r = OX_ERR;
                }
            }

            if (r == OX_ERR) {
                if ((r = error_recovery(ctxt, p, OX_RECOVERY_COMMA|OX_RECOVERY_RB)) == OX_ERR)
                    break;
            }
        }
        break;
    default:
        unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, '[', '{', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
        goto end;
    }

    last_loc(&loc, LOC);
    SET_LOC(pat, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, expr)
    return r;
}

/*Parse a parameter.*/
static OX_Result
parameter (OX_Context *ctxt, OX_Parser *p, OX_Bool *is_left)
{
    OX_VS_PUSH_5(ctxt, param, params, items, pat, e)
    OX_Result r;

    get_token(ctxt, p);

    if (TOK->type == OX_TOKEN_dot_dot_dot) {
        AST_NEW(param, rest);
        SET_LOC(param, LOC);

        if ((r = param_pattern(ctxt, p, pat)) == OX_ERR)
            goto end;

        AST_SET(param, pattern, pat);

        *is_left = OX_TRUE;
    } else {
        unget_token(ctxt, p);
        if ((r = param_pattern(ctxt, p, param)) == OX_ERR)
            goto end;

        if ((r = default_expr(ctxt, p, e)) == OX_ERR)
            goto end;

        AST_SET(param, expr, e);
    }

    AST_GET(p->func, params, params);
    AST_GET(params, items, items);
    if (ox_value_is_null(ctxt, items)) {
        ARRAY_NEW(items);
        AST_SET(params, items, items);
    }

    ARRAY_APPEND(items, param);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, param)
    return r;
}

/*Parse a function declaration.*/
static OX_Result
func_decl (OX_Context *ctxt, OX_Parser *p, OX_Value *fn)
{
    OX_VS_PUSH_6(ctxt, blk, old_func, old_blk, s, e, params)
    int old_flags = p->flags;
    OX_Location loc, ploc;
    OX_Result r;

    first_loc(&loc, LOC);

    func_new(ctxt, p, fn);
    ox_value_copy(ctxt, old_func, p->func);
    ox_value_copy(ctxt, p->func, fn);

    get_token(ctxt, p);

    if (TOK->type == '(') {
        first_loc(&ploc, LOC);

        AST_NEW(params, params);
        AST_SET(fn, params, params);

        get_token(ctxt, p);

        if (TOK->type != ')') {
            unget_token(ctxt, p);

            while (1) {
                OX_Bool is_left = OX_FALSE;

                r = parameter(ctxt, p, &is_left);

                if (r == OX_OK) {
                    get_token(ctxt, p);

                    if (TOK->type == ')')
                        break;

                    if (is_left) {
                        unexpect_token_error(ctxt, p, LOC, ')', -1);
                        unget_token(ctxt, p);
                        r = OX_ERR;
                    } else if (TOK->type == ',') {
                    } else if (NEWLINE) {
                        unget_token(ctxt, p);
                    } else {
                        unexpect_token_error(ctxt, p, LOC, ',', '\n', ')', -1);
                        unget_token(ctxt, p);
                        r = OX_ERR;
                    }
                }

                if (r == OX_ERR) {
                    if ((r = error_recovery(ctxt, p,
                            OX_RECOVERY_COMMA
                            |OX_RECOVERY_RP)) == OX_ERR)
                        goto end;
                }
            }
        }

        last_loc(&ploc, LOC);
        SET_LOC(params, &ploc);
    } else {
        unget_token(ctxt, p);
    }

    p->flags &= ~(OX_PARSER_FL_PUBLIC
            |OX_PARSER_FL_BREAK
            |OX_PARSER_FL_CONTINUE
            |OX_PARSER_FL_TEXTDOMAIN);
    p->flags |= OX_PARSER_FL_RETURN;

    get_token(ctxt, p);

    if (TOK->type == '{') {
        unget_token(ctxt, p);

        if ((r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
            goto end;

        AST_SET(fn, block, blk);
    } else if (TOK->type == OX_TOKEN_eq_gt) {
        OX_Location loc;

        if ((r = expression_prio(ctxt, p, e, OX_PRIO_COMMA)) == OX_ERR)
            goto end;

        AST_NEW(s, return);
        AST_SET(s, expr, e);
        GET_LOC(e, &loc);
        SET_LOC(s, &loc);

        AST_NEW(blk, block);
        ox_value_copy(ctxt, old_blk, p->block);
        ox_value_copy(ctxt, p->block, blk);

        add_stmt(ctxt, p, s);

        ox_value_copy(ctxt, p->block, old_blk);

        AST_SET(fn, block, blk);
    }

    last_loc(&loc, LOC);
    SET_LOC(fn, &loc);

    add_func(ctxt, p, fn);

    r = OX_OK;
end:
    p->flags = old_flags;
    ox_value_copy(ctxt, p->func, old_func);
    OX_VS_POP(ctxt, blk)
    return r;
}

/*Parse the accessor.*/
static OX_Result
accessor (OX_Context *ctxt, OX_Parser *p, OX_Value *a)
{
    OX_VS_PUSH_5(ctxt, fn, blk, params, items, old_func)
    int old_flags = p->flags;
    OX_Location loc;
    OX_Result r;

    /*Getter.*/
    AST_NEW(a, accessor);
    func_new(ctxt, p, fn);
    ox_value_copy(ctxt, old_func, p->func);
    ox_value_copy(ctxt, p->func, fn);

    p->flags &= ~(OX_PARSER_FL_PUBLIC|OX_PARSER_FL_BREAK|OX_PARSER_FL_CONTINUE);
    p->flags |= OX_PARSER_FL_RETURN;
    r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT);
    p->flags = old_flags;
    if (r == OX_ERR)
        goto end;

    GET_LOC(blk, &loc);
    AST_SET(fn, block, blk);
    SET_LOC(fn, &loc);
    AST_SET(a, get, fn);

    add_func(ctxt, p, fn);
    ox_value_copy(ctxt, p->func, old_func);

    /*Setter.*/
    get_token(ctxt, p);

    if (NEWLINE || (TOK->type == ';')) {
        unget_token(ctxt, p);
    } else if (TOK->type == '(') {
        unget_token(ctxt, p);

        if ((r = func_decl(ctxt, p, fn)) == OX_ERR)
            goto end;

        AST_GET(fn, params, params);
        if (!ox_value_is_null(ctxt, params))
            AST_GET(params, items, items);
        if (!ox_value_is_array(ctxt, items) || (ox_array_length(ctxt, items) != 1))
            error(ctxt, p, LOC, OX_TEXT("setter can only have one parameter"));

        AST_SET(a, set, fn);
    } else {
        unget_token(ctxt, p);
    }

    last_loc(&loc, LOC);
    SET_LOC(a, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*Parse the class body.*/
static OX_Result
class_body (OX_Context *ctxt, OX_Parser *p, OX_Value *c, int flags)
{
    OX_VS_PUSH_6(ctxt, pn, expr, props, prop, fn, doc)
    int old_flags = p->flags;
    OX_Result r;

    p->flags &= ~(OX_PARSER_FL_PUBLIC|OX_PARSER_FL_TEXTDOMAIN);

    AST_GET(c, props, props);

    /*Class body.*/
    while (1) {
        OX_Bool is_static = (flags & OX_CLASS_FL_IN_STATIC) ? OX_TRUE : OX_FALSE;
        OX_Bool is_prop = OX_TRUE;

        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        ox_value_copy(ctxt, doc, p->doc);
        ox_value_set_null(ctxt, p->doc);

        if (is_keyword(TOK, OX_KEYWORD_static)) {
            if (!(flags & OX_CLASS_FL_STATIC))
                error(ctxt, p, LOC, OX_TEXT("`static\' cannot be used in here"));

            get_token(ctxt, p);

            if (TOK->type == '{') {
                class_body(ctxt, p, c, OX_CLASS_FL_IN_STATIC);
                AST_GET(c, props, props);
                continue;
            } else {
                unget_token(ctxt, p);
                is_static = OX_TRUE;
            }

            r = OX_OK;
        } else if (is_keyword(TOK, OX_KEYWORD_enum)) {
            r = enum_def(ctxt, p, prop);
            is_prop = OX_FALSE;
        } else if (is_keyword(TOK, OX_KEYWORD_bitfield)) {
            r = bitfield_def(ctxt, p, prop);
            is_prop = OX_FALSE;
        } else {
            unget_token(ctxt, p);
            r = OX_OK;
        }

        if ((r == OX_OK) && is_prop)
            r = prop_name(ctxt, p, pn);

        if ((r == OX_OK) && is_prop) {
            get_token(ctxt, p);

            if (NEWLINE) {
                unget_token(ctxt, p);
                AST_NEW(prop, var);
            } else if (TOK->type == ':') {
                r = expression(ctxt, p, expr);
                if (r == OX_OK) {
                    if (ox_ast_get_type(ctxt, expr) == OX_AST_func)
                        AST_SET_B(expr, this, OX_TRUE);

                    AST_NEW(prop, const);
                    AST_SET(prop, expr, expr);
                }
            } else if (TOK->type == '=') {
                r = expression(ctxt, p, expr);
                if (r == OX_OK) {
                    if (ox_ast_get_type(ctxt, expr) == OX_AST_func)
                        AST_SET_B(expr, this, OX_TRUE);

                    AST_NEW(prop, var);
                    AST_SET(prop, expr, expr);
                }
            } else if (TOK->type == '(') {
                unget_token(ctxt, p);

                r = func_decl(ctxt, p, expr);
                if (r == OX_OK) {
                    AST_SET_B(expr, this, OX_TRUE);
                    AST_NEW(prop, method);
                    AST_SET(prop, expr, expr);
                }
            } else if (TOK->type == '{') {
                unget_token(ctxt, p);

                r = accessor(ctxt, p, prop);
                if (r == OX_OK) {
                    AST_GET(prop, get, fn);
                    AST_SET_B(fn, this, OX_TRUE);

                    AST_GET(prop, set, fn);
                    if (!ox_value_is_null(ctxt, fn))
                        AST_SET_B(fn, this, OX_TRUE);
                }
            } else {
                unget_token(ctxt, p);
                AST_NEW(prop, var);
            }
        }

        if ((r == OX_OK) && is_prop) {
            if (is_static) {
                AST_SET_B(prop, static, OX_TRUE);
            } else {
                OX_AstType type = ox_ast_get_type(ctxt, prop);

                if ((type == OX_AST_var) || (type == OX_AST_const)) {
                    OX_Location nloc;

                    GET_LOC(pn, &nloc);
                    warning(ctxt, p, &nloc, OX_TEXT("field should be used as static property"));
                }
            }

            AST_SET(prop, name, pn);
        }

        if (r == OX_OK) {
            if (ox_value_is_null(ctxt, props)) {
                ARRAY_NEW(props);
                AST_SET(c, props, props);
            }

            ARRAY_APPEND(props, prop);

            if (!ox_value_is_null(ctxt, doc))
                AST_SET(prop, doc, doc);

            get_token(ctxt, p);

            if (TOK->type == '}')
                break;
            if (TOK->type == ';') {
            } else if (NEWLINE) {
                unget_token(ctxt, p);
            } else {
                unexpect_token_error(ctxt, p, LOC, ';', '}', -1);
                unget_token(ctxt, p);
                r = OX_ERR;
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, OX_RECOVERY_SEMICOLON|OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    p->flags = old_flags;
    OX_VS_POP(ctxt, pn)
    return OX_OK;
}

/*Parse a class declaration.*/
static OX_Result
class_decl (OX_Context *ctxt, OX_Parser *p, OX_Value *c)
{
    OX_VS_PUSH_2(ctxt, pc, parents)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(c, class);

    /*Parents.*/
    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '{')
            break;

        unget_token(ctxt, p);

        if ((r = expression_prio(ctxt, p, pc, OX_PRIO_COMMA)) == OX_ERR)
            goto end;

        if (ox_value_is_null(ctxt, parents)) {
            ARRAY_NEW(parents);
            AST_SET(c, parents, parents);
        }

        ARRAY_APPEND(parents, pc);

        get_token(ctxt, p);

        if (TOK->type == '{')
            break;

        if (TOK->type == ',') {
        } else if (NEWLINE) {
            unget_token(ctxt, p);
        } else {
            unexpect_token_error(ctxt, p, LOC, ',', '{', -1);
            unget_token(ctxt, p);
            r = OX_ERR;
            goto end;
        }
    }

    if ((r = class_body(ctxt, p, c, OX_CLASS_FL_STATIC)) == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    SET_LOC(c, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, pc)
    return r;
}

/*Check if the identifier is an dollar argument.*/
static OX_Bool
is_dollar_arg (OX_Context *ctxt, OX_Parser *p, OX_Value *n, int *pid)
{
    const char *s = ox_string_get_char_star(ctxt, n);
    char *e;
    int id;

    if (s[0] != '$')
        return OX_FALSE;

    if (s[1] == 0) {
        *pid = 0;
        return OX_TRUE;
    }

    id = strtol(s + 1, &e, 10);
    if (e[0] != 0)
        return OX_FALSE;

    if (id > 0xffff)
        error(ctxt, p, LOC, OX_TEXT("argument index is too big"));

    *pid = id;
    return OX_TRUE;
}

/* "this". */
static OX_Result
this_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    AST_NEW(expr, this);
    SET_LOC(expr, LOC);

    return OX_OK;
}

/* "argv". */
static OX_Result
argv_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    AST_NEW(expr, argv);
    SET_LOC(expr, LOC);

    return OX_OK;
}

/*Parse an identifier.*/
static OX_Result
identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    int id;

    check_id(ctxt, p);

    if (is_dollar_arg(ctxt, p, TOK->v, &id)) {
        AST_NEW(expr, arg);
        AST_SET_N(expr, id, id);
        SET_LOC(expr, LOC);
    } else {
        AST_NEW(expr, id);
        AST_SET(expr, value, TOK->v);
        SET_LOC(expr, LOC);
    }

    return OX_OK;
}

/*Parse an outer identifier.*/
static OX_Result
outer_identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    check_id(ctxt, p);

    AST_NEW(expr, id);
    AST_SET(expr, value, TOK->v);
    AST_SET_B(expr, outer, OX_TRUE);
    SET_LOC(expr, LOC);

    return OX_OK;
}

/*Parse a hash identifier.*/
static OX_Result
hash_identifier (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    check_id(ctxt, p);

    AST_NEW(expr, id);
    AST_SET(expr, value, TOK->v);
    AST_SET_B(expr, hash, OX_TRUE);
    SET_LOC(expr, LOC);

    return OX_OK;
}

/*Parse an expression's tail with priority.*/
static OX_Result
expression_tail_prio (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, int prio)
{
    OX_VS_PUSH(ctxt, t)
    OX_Result r = OX_OK;

    while (1) {
        get_token_flags(ctxt, p, OX_LEX_FL_DIV);

        if (NEWLINE) {
            unget_token(ctxt, p);
            goto end;
        }

        switch (TOK->type) {
        case OX_TOKEN_STRING:
        case OX_TOKEN_STR_HEAD:
            if ((r = multipart_string(ctxt, p, expr, t)) == OX_ERR)
                goto end;
            ox_value_copy(ctxt, expr, t);
            break;
        case '.':
            if ((r = dot_expr(ctxt, p, expr)) == OX_ERR)
                goto end;
            break;
        case '[':
            if ((r = lookup_expr(ctxt, p, expr)) == OX_ERR)
                goto end;
            break;
        case '(':
            if ((r = call_expr(ctxt, p, expr)) == OX_ERR)
                goto end;
            break;
        case '?':
            if ((r = ques_expr(ctxt, p, expr)) == OX_ERR)
                goto end;
            break;

#define LEFT_OP(t, pr)\
        case t:\
            if (prio >= pr) {\
                unget_token(ctxt, p);\
                goto end;\
            }

#define RIGHT_OP(t, pr)\
        case t:\
            if (prio > pr) {\
                unget_token(ctxt, p);\
                goto end;\
            }

#define BINARY_OP(t, pr, n)\
        LEFT_OP(t, pr)\
        if ((r = binary_expr(ctxt, p, expr, pr, n)) == OX_ERR)\
            goto end;\
        break;

        BINARY_OP(OX_TOKEN_star_star, OX_PRIO_EXP, OX_AST_exp)
        BINARY_OP('*', OX_PRIO_MUL, OX_AST_mul)
        BINARY_OP('/', OX_PRIO_MUL, OX_AST_div)
        BINARY_OP('%', OX_PRIO_MUL, OX_AST_mod)
        BINARY_OP('+', OX_PRIO_ADD, OX_AST_add)
        BINARY_OP('-', OX_PRIO_ADD, OX_AST_sub)
        BINARY_OP('~', OX_PRIO_ADD, OX_AST_match)
        BINARY_OP(OX_TOKEN_lt_lt, OX_PRIO_SHIFT, OX_AST_shl)
        BINARY_OP(OX_TOKEN_gt_gt, OX_PRIO_SHIFT, OX_AST_shr)
        BINARY_OP(OX_TOKEN_gt_gt_gt, OX_PRIO_SHIFT, OX_AST_ushr)
        BINARY_OP('<', OX_PRIO_REL, OX_AST_lt)
        BINARY_OP('>', OX_PRIO_REL, OX_AST_gt)
        BINARY_OP(OX_TOKEN_lt_eq, OX_PRIO_REL, OX_AST_le)
        BINARY_OP(OX_TOKEN_gt_eq, OX_PRIO_REL, OX_AST_ge)
        BINARY_OP(OX_TOKEN_eq_eq, OX_PRIO_EQ, OX_AST_eq)
        BINARY_OP(OX_TOKEN_bang_eq, OX_PRIO_EQ, OX_AST_ne)
        BINARY_OP('&', OX_PRIO_BIT_AND, OX_AST_bit_and)
        BINARY_OP('^', OX_PRIO_BIT_XOR, OX_AST_bit_xor)
        BINARY_OP('|', OX_PRIO_BIT_OR, OX_AST_bit_or)
        BINARY_OP(OX_TOKEN_amp_amp, OX_PRIO_LOGIC_AND, OX_AST_logic_and)
        BINARY_OP(OX_TOKEN_pipe_pipe, OX_PRIO_LOGIC_OR, OX_AST_logic_or)

#define ASSI_OP(t, n, o)\
        RIGHT_OP(t, OX_PRIO_ASSI)\
        if ((r = assi_op_expr(ctxt, p, expr, n, OX_DECL_VAR, o)) == OX_ERR)\
            goto end;\
        break;

        RIGHT_OP('=', OX_PRIO_ASSI)
            if ((r = assi_expr(ctxt, p, expr, OX_DECL_VAR)) == OX_ERR)
                goto end;
            break;
        RIGHT_OP(':', OX_PRIO_ASSI)
            if ((r = assi_expr(ctxt, p, expr, OX_DECL_CONST)) == OX_ERR)
                goto end;
            break;
        RIGHT_OP(OX_TOKEN_eq_gt, OX_PRIO_ASSI)
            if ((r = rev_assi_expr(ctxt, p, expr, OX_DECL_VAR)) == OX_ERR)
                goto end;
            break;
        RIGHT_OP(OX_TOKEN_colon_gt, OX_PRIO_ASSI)
            if ((r = rev_assi_expr(ctxt, p, expr, OX_DECL_CONST)) == OX_ERR)
                goto end;
            break;
        ASSI_OP(OX_TOKEN_star_star_eq, OX_AST_exp, OX_FALSE)
        ASSI_OP(OX_TOKEN_plus_eq, OX_AST_add, OX_FALSE)
        ASSI_OP(OX_TOKEN_minus_eq, OX_AST_sub, OX_FALSE)
        ASSI_OP(OX_TOKEN_tilde_eq, OX_AST_match, OX_FALSE)
        ASSI_OP(OX_TOKEN_star_eq, OX_AST_mul, OX_FALSE)
        ASSI_OP(OX_TOKEN_slash_eq, OX_AST_div, OX_FALSE)
        ASSI_OP(OX_TOKEN_percent_eq, OX_AST_mod, OX_FALSE)
        ASSI_OP(OX_TOKEN_lt_lt_eq, OX_AST_shl, OX_FALSE)
        ASSI_OP(OX_TOKEN_gt_gt_eq, OX_AST_shr, OX_FALSE)
        ASSI_OP(OX_TOKEN_gt_gt_gt_eq, OX_AST_ushr, OX_FALSE)
        ASSI_OP(OX_TOKEN_amp_eq, OX_AST_bit_and, OX_FALSE)
        ASSI_OP(OX_TOKEN_caret_eq, OX_AST_bit_xor, OX_FALSE)
        ASSI_OP(OX_TOKEN_pipe_eq, OX_AST_bit_or, OX_FALSE)
        ASSI_OP(OX_TOKEN_amp_amp_eq, OX_AST_logic_and, OX_FALSE)
        ASSI_OP(OX_TOKEN_pipe_pipe_eq, OX_AST_logic_or, OX_FALSE)

        ASSI_OP(OX_TOKEN_dot_star_star_eq, OX_AST_exp, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_plus_eq, OX_AST_add, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_minus_eq, OX_AST_sub, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_tilde_eq, OX_AST_match, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_star_eq, OX_AST_mul, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_slash_eq, OX_AST_div, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_percent_eq, OX_AST_mod, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_lt_lt_eq, OX_AST_shl, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_gt_gt_eq, OX_AST_shr, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_gt_gt_gt_eq, OX_AST_ushr, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_amp_eq, OX_AST_bit_and, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_caret_eq, OX_AST_bit_xor, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_pipe_eq, OX_AST_bit_or, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_amp_amp_eq, OX_AST_logic_and, OX_TRUE)
        ASSI_OP(OX_TOKEN_dot_pipe_pipe_eq, OX_AST_logic_or, OX_TRUE)

        LEFT_OP(',', OX_PRIO_COMMA)
            if ((r = comma_expr(ctxt, p, expr)) == OX_ERR)
                goto end;
            break;
        case OX_TOKEN_ID:
            switch (TOK->keyword) {
            BINARY_OP(OX_KEYWORD_instof, OX_PRIO_REL, OX_AST_instof)
            default:
                unget_token(ctxt, p);
                goto end;
            }
            break;
        default:
            unget_token(ctxt, p);
            goto end;
        }
    }
end:
    OX_VS_POP(ctxt, t)
    return r;
}

/*Parse an expression with priority.*/
static OX_Result
expression_prio (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, int prio)
{
    OX_Result r;

    get_token(ctxt, p);

    switch (TOK->type) {
    case OX_TOKEN_NULL:
    case OX_TOKEN_BOOL:
    case OX_TOKEN_NUMBER:
    case OX_TOKEN_STRING:
    case OX_TOKEN_RE:
        AST_NEW(expr, value);
        AST_SET(expr, value, TOK->v);
        SET_LOC(expr, LOC);
        break;
    case OX_TOKEN_STR_HEAD:
        if ((r = multipart_string(ctxt, p, NULL, expr)) == OX_ERR)
            return r;
        break;
    case '[':
        if ((r = array_literal(ctxt, p, expr, OX_AST_array)) == OX_ERR)
            return r;
        break;
    case '{':
        if ((r = object_literal(ctxt, p, expr, OX_AST_object)) == OX_ERR)
            return r;
        break;
    case '(':
        if ((r = parenthese_expr(ctxt, p, expr)) == OX_ERR)
            return r;
        break;
    case '+':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_plus)) == OX_ERR)
            return r;
        break;
    case '-':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_minus)) == OX_ERR)
            return r;
        break;
    case '~':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_bit_rev)) == OX_ERR)
            return r;
        break;
    case '!':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_logic_not)) == OX_ERR)
            return r;
        break;
    case '&':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_get_ptr)) == OX_ERR)
            return r;
        break;
    case '*':
        if ((r = unary_expr(ctxt, p, expr, OX_AST_get_value)) == OX_ERR)
            return r;
        break;
    case OX_TOKEN_AT_ID:
        if ((r = outer_identifier(ctxt, p, expr)) == OX_ERR)
            return r;
        break;
    case OX_TOKEN_HASH_ID:
        if ((r = hash_identifier(ctxt, p, expr)) == OX_ERR)
            return r;
        break;
    case OX_TOKEN_ID:
        switch (TOK->keyword) {
        case OX_KEYWORD_func:
            if ((r = func_decl(ctxt, p, expr)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_class:
            if ((r = class_decl(ctxt, p, expr)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_if:
            if ((r = if_expr(ctxt, p, expr, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_case:
            if ((r = case_expr(ctxt, p, expr, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_this:
            if ((r = this_expr(ctxt, p, expr)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_argv:
            if ((r = argv_expr(ctxt, p, expr)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_typeof:
            if ((r = unary_expr(ctxt, p, expr, OX_AST_typeof)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_global:
            if ((r = unary_expr(ctxt, p, expr, OX_AST_global)) == OX_ERR)
                return r;
            break;
        case OX_KEYWORD_yield:
            if ((r = yield_expr(ctxt, p, expr)) == OX_ERR)
                return r;
            break;
        default:
            if ((p->flags & OX_PARSER_FL_OWNED) && is_keyword(TOK, OX_KEYWORD_owned)) {
                if ((r = unary_expr(ctxt, p, expr, OX_AST_owned))== OX_ERR)
                    return r;
            } else if ((r = identifier(ctxt, p, expr)) == OX_ERR) {
                return r;
            }
            break;
        }
        break;
    default:
        unexpect_token_error_s(ctxt, p, LOC, OX_TEXT("an expression"));
        unget_token(ctxt, p);
        return OX_ERR;
    }

    if (prio == OX_PRIO_HIGHEST)
        return OX_OK;

    return expression_tail_prio(ctxt, p, expr, prio);
}

/*Parse an expression.*/
static OX_Result
expression (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    return expression_prio(ctxt, p, expr, OX_PRIO_LOWEST);
}

/*Parse a block.*/
static OX_Result
block (OX_Context *ctxt, OX_Parser *p, OX_Value *blk, OX_BlockContentType ctype)
{
    OX_VS_PUSH_3(ctxt, old_blk, items, item)
    int old_flags = p->flags;
    int eflags;
    OX_Location loc;
    OX_Result r;

    AST_NEW(blk, block);
    ox_value_copy(ctxt, old_blk, p->block);

    if (ctype == OX_BLOCK_CONTENT_STMT) {
        ox_value_copy(ctxt, p->block, blk);
        eflags = OX_RECOVERY_LF|OX_RECOVERY_SEMICOLON|OX_RECOVERY_BLOCK;
    } else {
        ARRAY_NEW(items);

        if (ctype == OX_BLOCK_CONTENT_ITEM)
            AST_SET(blk, items, items);
        else
            AST_SET(blk, props, items);
        eflags = OX_RECOVERY_LF|OX_RECOVERY_COMMA|OX_RECOVERY_BLOCK;
    }

    p->flags &= ~(OX_PARSER_FL_PUBLIC|OX_PARSER_FL_TEXTDOMAIN);

    if ((r = get_expect_token(ctxt, p, '{')) == OX_ERR)
        goto end;

    first_loc(&loc, LOC);

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;
        if (TOK->type == OX_TOKEN_END) {
            unexpect_token_error(ctxt, p, LOC, '}', -1);
            break;
        }

        if (ctype == OX_BLOCK_CONTENT_STMT) {
            unget_token(ctxt, p);

            r = statement(ctxt, p);
        } else {
            if (ctype == OX_BLOCK_CONTENT_ITEM) {
                r = array_item(ctxt, p, item);
            } else {
                r = object_item(ctxt, p, item);
            }

            if (r == OX_OK) {
                ARRAY_APPEND(items, item);

                if (ox_ast_get_type(ctxt, item) != OX_AST_skip) {
                    get_token(ctxt, p);

                    if (TOK->type == '}')
                        break;

                    if (TOK->type == ',') {
                    } else if (NEWLINE) {
                        unget_token(ctxt, p);
                    } else {
                        unexpect_token_error(ctxt, p, LOC, ',', '}', -1);
                        unget_token(ctxt, p);
                        r = OX_ERR;
                    }
                }
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p, eflags)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);

    SET_LOC(blk, &loc);

    r = OX_OK;
end:
    p->flags = old_flags;
    ox_value_copy(ctxt, p->block, old_blk);
    OX_VS_POP(ctxt, old_blk)
    return r;
}

/*Parse a if expression.*/
static OX_Result
if_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *expr, OX_BlockContentType ctype)
{
    OX_VS_PUSH_4(ctxt, items, item, cexpr, cblk)
    OX_Location loc, iloc;
    OX_Result r;

    AST_NEW(expr, if);
    ARRAY_NEW(items);
    AST_SET(expr, items, items);

    first_loc(&loc, LOC);
    iloc = *LOC;

    while (1) {
        if ((r = cond_expr(ctxt, p, cexpr)) == OX_ERR)
            goto end;

        if ((r = block(ctxt, p, cblk, ctype)) == OX_ERR)
            goto end;

        AST_NEW(item, expr_block);
        SET_LOC(item, &iloc);
        AST_SET(item, expr, cexpr);
        AST_SET(item, block, cblk);
        ARRAY_APPEND(items, item);

        get_token(ctxt, p);
        if (is_keyword(TOK, OX_KEYWORD_else)) {
            if ((r = block(ctxt, p, cblk, ctype)) == OX_ERR)
                goto end;

            AST_SET(expr, else, cblk);
            break;
        } else if (!is_keyword(TOK, OX_KEYWORD_elif)) {
            unget_token(ctxt, p);
            break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(expr, &loc);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, items)
    return r;
}

/*Parse a if statement.*/
static OX_Result
if_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH(ctxt, expr)
    OX_Result r;

    if ((r = if_expr(ctxt, p, expr, OX_BLOCK_CONTENT_STMT)) == OX_OK)
        add_stmt(ctxt, p, expr);

    OX_VS_POP(ctxt, expr)
    return r;
}

/*Parse a do while statement.*/
static OX_Result
do_while_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_3(ctxt, stmt, blk, expr)
    int old_flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    old_flags = p->flags;
    p->flags &= ~OX_PARSER_FL_PUBLIC;
    p->flags |= OX_PARSER_FL_BREAK|OX_PARSER_FL_CONTINUE;
    r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT);
    p->flags = old_flags;
    if (r == OX_ERR)
        goto end;

    if ((r = get_expect_token(ctxt, p, OX_KEYWORD_while)) == OX_ERR)
        goto end;

    if ((r = cond_expr(ctxt, p, expr)) == OX_ERR)
        goto end;

    stmt_end(ctxt, p);

    last_loc(&loc, LOC);

    AST_NEW(stmt, do_while);
    AST_SET(stmt, expr, expr);
    AST_SET(stmt, block, blk);
    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a while statement.*/
static OX_Result
while_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_3(ctxt, stmt, blk, expr)
    int old_flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    if ((r = cond_expr(ctxt, p, expr)) == OX_ERR)
        goto end;

    old_flags = p->flags;
    p->flags &= ~OX_PARSER_FL_PUBLIC;
    p->flags |= OX_PARSER_FL_BREAK|OX_PARSER_FL_CONTINUE;
    r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT);
    p->flags = old_flags;
    if (r == OX_ERR)
        goto end;

    last_loc(&loc, LOC);

    AST_NEW(stmt, while);
    AST_SET(stmt, expr, expr);
    AST_SET(stmt, block, blk);
    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a for statement.*/
static OX_Result
for_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_4(ctxt, stmt, expr, blk, op)
    int old_flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    get_token(ctxt, p);
    if (TOK->type != ';') {
        unget_token(ctxt, p);
        if ((r = expression(ctxt, p, expr)) == OX_ERR)
            goto end;
    } else {
        unget_token(ctxt, p);
    }

    get_token(ctxt, p);
    if (is_keyword(TOK, OX_KEYWORD_as)) {
        AST_NEW(stmt, for_as);
        AST_NEW(op, none);
        AST_SET(stmt, operator, op);
        SET_LOC(op, LOC);
        AST_SET(stmt, right, expr);

        get_token(ctxt, p);

        if ((TOK->type == '[') || (TOK->type == '{')) {
            unget_token(ctxt, p);
            r = assi_pattern(ctxt, p, expr, OX_DECL_VAR);
        } else {
            unget_token(ctxt, p);
            r = expression(ctxt, p, expr);
            if (r == OX_OK) {
                check_left_expr(ctxt, p, expr);
                add_decl_ast(ctxt, p, expr, OX_DECL_VAR);
            }
        }

        if (r == OX_ERR)
            goto end;

        AST_SET(stmt, left, expr);
    } else if (TOK->type == ';') {
        AST_NEW(stmt, for);

        if (!ox_value_is_null(ctxt, expr))
            AST_SET(stmt, init, expr);

        get_token(ctxt, p);

        if (TOK->type != ';') {
            unget_token(ctxt, p);

            if ((r = cond_expr(ctxt, p, expr)) == OX_ERR)
                goto end;

            AST_SET(stmt, cond, expr);

            if ((r = get_expect_token(ctxt, p, ';')) == OX_ERR)
                goto end;
        }

        get_token(ctxt, p);
        if (TOK->type != '{') {
            unget_token(ctxt, p);

            if ((r = expression(ctxt, p, expr)) == OX_ERR)
                goto end;

            AST_SET(stmt, step, expr);
        } else {
            unget_token(ctxt, p);
        }
    } else {
        unexpect_token_error(ctxt, p, LOC, ';', OX_KEYWORD_as, -1);
        unget_token(ctxt, p);
        r = OX_ERR;
        goto end;
    }

    old_flags = p->flags;
    p->flags &= ~OX_PARSER_FL_PUBLIC;
    p->flags |= OX_PARSER_FL_BREAK|OX_PARSER_FL_CONTINUE;

    r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT);
    p->flags = old_flags;
    if (r == OX_ERR)
        goto end;

    last_loc(&loc, LOC);
    SET_LOC(stmt, &loc);
    AST_SET(stmt, block, blk);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a schedule statement.*/
static OX_Result
sched_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, stmt, blk)
    int old_flags;
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    AST_NEW(stmt, sched);

    get_token(ctxt, p);
    if (TOK->type == '{') {
        unget_token(ctxt, p);

        old_flags = p->flags;
        p->flags &= ~OX_PARSER_FL_PUBLIC;
        r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT);
        p->flags = old_flags;
        if (r == OX_ERR)
            goto end;

        AST_SET(stmt, block, blk);
    } else {
        unget_token(ctxt, p);
    }

    last_loc(&loc, LOC);

    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a case condition.*/
static OX_Result
case_cond (OX_Context *ctxt, OX_Parser *p, OX_Value *cond, OX_Location *aloc)
{
    OX_VS_PUSH(ctxt, e)
    OX_Result r;

    get_token(ctxt, p);

    if (TOK->type == '*') {
        if (aloc->first_line) {
            error(ctxt, p, LOC, OX_TEXT("`*\' is already used"));
            note(ctxt, p, aloc, OX_TEXT("previous `*\' is here"));
        } else {
            *aloc = *LOC;
        }

        AST_NEW(cond, all);
        SET_LOC(cond, LOC);
    } else {
        OX_Bool is_func;

        unget_token(ctxt, p);

        if ((r = expr_or_parenthese_func(ctxt, p, e, &is_func, OX_PRIO_COMMA)) == OX_ERR)
            goto end;

        if (is_func) {
            AST_NEW(cond, case_func);
            AST_SET(cond, expr, e);
        } else {
            ox_value_copy(ctxt, cond, e);
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, e)
    return r;
}

/*Parse case conditions.*/
static OX_Result
case_conds (OX_Context *ctxt, OX_Parser *p, OX_Value *conds, OX_Location *aloc)
{
    OX_VS_PUSH(ctxt, cond)
    OX_Result r;

    ARRAY_NEW(conds);

    if ((r = case_cond(ctxt, p, cond, aloc)) == OX_ERR)
        goto end;

    ARRAY_APPEND(conds, cond);

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '{') {
            unget_token(ctxt, p);
            break;
        }

        if (TOK->type == ',') {
        } else if (NEWLINE) {
            unget_token(ctxt, p);
        } else {
            unexpect_token_error(ctxt, p, LOC, ',', '{', '\n', -1);
            unget_token(ctxt, p);
            r = OX_ERR;
            goto end;
        }

        if ((r = case_cond(ctxt, p, cond, aloc)) == OX_ERR)
            goto end;

        ARRAY_APPEND(conds, cond);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, cond)
    return r;
}

/*Parse a case expression.*/
static OX_Result
case_expr (OX_Context *ctxt, OX_Parser *p, OX_Value *re, OX_BlockContentType ctype)
{
    OX_VS_PUSH_5(ctxt, expr, items, item, conds, blk)
    int old_flags = p->flags;
    OX_Location loc, all_loc;
    OX_Result r;

    all_loc.first_line = 0;

    first_loc(&loc, LOC);

    AST_NEW(re, case);

    if ((r = expression(ctxt, p, expr)) == OX_ERR)
        goto end;

    AST_SET(re, expr, expr);
    ARRAY_NEW(items);
    AST_SET(re, items, items);

    if ((r = get_expect_token(ctxt, p, '{')) == OX_ERR)
        goto end;

    p->flags &= ~OX_PARSER_FL_PUBLIC;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        unget_token(ctxt, p);
        r = case_conds(ctxt, p, conds, &all_loc);

        if (r == OX_OK)
            r = block(ctxt, p, blk, ctype);

        if (r == OX_OK) {
            AST_NEW(item, exprs_block);
            AST_SET(item, exprs, conds);
            AST_SET(item, block, blk);
            ARRAY_APPEND(items, item);
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p,
                    OX_RECOVERY_COMMA
                    |OX_RECOVERY_BLOCK
                    |OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(re, &loc);

    r = OX_OK;
end:
    p->flags = old_flags;
    OX_VS_POP(ctxt, expr)
    return r;
}

/*Parse a case statement.*/
static OX_Result
case_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH(ctxt, stmt)
    OX_Result r;

    r = case_expr(ctxt, p, stmt, OX_BLOCK_CONTENT_STMT);
    if (r == OX_OK)
        add_stmt(ctxt, p, stmt);

    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a try statement.*/
static OX_Result
try_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_4(ctxt, stmt, blk, expr, ent)
    int old_flags = p->flags;
    OX_Location loc;
    OX_Result r;

    p->flags &= ~OX_PARSER_FL_PUBLIC;

    first_loc(&loc, LOC);

    AST_NEW(stmt, try);

    if ((r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
        goto end;

    AST_SET(stmt, block, blk);

    get_token(ctxt, p);
    if (is_keyword(TOK, OX_KEYWORD_catch)) {
        AST_NEW(ent, expr_block);
        SET_LOC(ent, LOC);

        get_token(ctxt, p);

        if ((TOK->type == '[') || (TOK->type == '{')) {
            unget_token(ctxt, p);
            r = assi_pattern(ctxt, p, expr, OX_DECL_VAR);
        } else {
            unget_token(ctxt, p);
            r = expression(ctxt, p, expr);
            if (r == OX_OK) {
                check_left_expr(ctxt, p, expr);
                add_decl_ast(ctxt, p, expr, OX_DECL_VAR);
            }
        }
        if (r == OX_ERR)
            goto end;

        if ((r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
            goto end;

        AST_SET(ent, expr, expr);
        AST_SET(ent, block, blk);
        AST_SET(stmt, catch, ent);

        get_token(ctxt, p);
    }

    if (is_keyword(TOK, OX_KEYWORD_finally)) {
        if ((r = block(ctxt, p, blk, OX_BLOCK_CONTENT_STMT)) == OX_ERR)
            goto end;

        AST_SET(stmt, finally, blk);
    } else {
        unget_token(ctxt, p);
    }

    last_loc(&loc, LOC);
    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    p->flags = old_flags;
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a return statement.*/
static OX_Result
return_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, stmt, expr)
    OX_Location loc;
    OX_Result r;

    if (!(p->flags & OX_PARSER_FL_RETURN))
        error(ctxt, p, LOC, OX_TEXT("`return\' cannot be used here"));

    first_loc(&loc, LOC);

    AST_NEW(stmt, return);

    get_token(ctxt, p);

    if (NEWLINE || (TOK->type == ';')) {
        unget_token(ctxt, p);
    } else {
        unget_token(ctxt, p);

        if ((r = expression(ctxt, p, expr)) == OX_ERR)
            goto end;

        AST_SET(stmt, expr, expr);
    }

    stmt_end(ctxt, p);

    last_loc(&loc, LOC);
    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a throw statement.*/
static OX_Result
throw_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, stmt, expr)
    OX_Location loc;
    OX_Result r;

    first_loc(&loc, LOC);

    if ((r = expression(ctxt, p, expr)) == OX_ERR)
        goto end;

    stmt_end(ctxt, p);

    last_loc(&loc, LOC);

    AST_NEW(stmt, throw);
    AST_SET(stmt, expr, expr);
    SET_LOC(stmt, &loc);
    add_stmt(ctxt, p, stmt);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, stmt)
    return r;
}

/*Parse a break statement.*/
static OX_Result
break_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH(ctxt, stmt)

    if (!(p->flags & OX_PARSER_FL_BREAK))
        error(ctxt, p, LOC, OX_TEXT("`break\' cannot be used here"));

    stmt_end(ctxt, p);

    AST_NEW(stmt, break);
    SET_LOC(stmt, LOC);
    add_stmt(ctxt, p, stmt);

    OX_VS_POP(ctxt, stmt)
    return OX_OK;
}

/*Parse a continue statement.*/
static OX_Result
continue_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH(ctxt, stmt)

    if (!(p->flags & OX_PARSER_FL_CONTINUE))
        error(ctxt, p, LOC, OX_TEXT("`continue\' cannot be used here"));

    stmt_end(ctxt, p);

    AST_NEW(stmt, continue);
    SET_LOC(stmt, LOC);
    add_stmt(ctxt, p, stmt);

    OX_VS_POP(ctxt, stmt)
    return OX_OK;
}

/*Parse a textdomain statement.*/
static OX_Result
textdomain_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH(ctxt, td)
    OX_Result r;

    /*Get text domain.*/
    if ((r = get_expect_token(ctxt, p, OX_TOKEN_STRING)) == OX_ERR)
        goto end;

    AST_GET(p->ast, textdomain, td);
    if (!ox_value_is_null(ctxt, td)) {
        OX_Location oloc;

        GET_LOC(td, &oloc);

        error(ctxt, p, LOC, OX_TEXT("text domain is already defined"));
        note(ctxt, p, &oloc, OX_TEXT("previous text domain definition is here"));

        r = OX_OK;
        goto end;
    }

    AST_NEW(td, value);
    AST_SET(td, value, TOK->v);
    SET_LOC(td, LOC);
    AST_SET(p->ast, textdomain, td);

    get_token(ctxt, p);

    if (NEWLINE || (TOK->type == ';')) {
        unget_token(ctxt, p);
    } else {
        /*Get the path of the message files.*/
        unget_token(ctxt, p);

        if ((r = get_expect_token(ctxt, p, OX_TOKEN_STRING)) == OX_ERR)
            goto end;

        AST_SET(td, path, TOK->v);
    }

    stmt_end(ctxt, p);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, td)
    return r;
}

/*Parse an expression statement.*/
static OX_Result
expr_stmt (OX_Context *ctxt, OX_Parser *p, OX_Value *expr)
{
    OX_Result r;

    if ((r = expression(ctxt, p, expr)) == OX_OK) {
        add_stmt(ctxt, p, expr);
        stmt_end(ctxt, p);

        add_decl_ast(ctxt, p, expr, OX_DECL_VAR);
    }

    return r;
}

/*Parse a statement.*/
static OX_Result
statement (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, expr, doc)
    OX_Result r;

    ox_value_copy(ctxt, doc, p->doc);
    ox_value_set_null(ctxt, p->doc);

    get_token(ctxt, p);

    switch (TOK->type) {
    case OX_TOKEN_NULL:
    case OX_TOKEN_BOOL:
    case OX_TOKEN_NUMBER:
    case OX_TOKEN_STRING:
    case OX_TOKEN_STR_HEAD:
    case OX_TOKEN_RE:
    case OX_TOKEN_AT_ID:
    case OX_TOKEN_HASH_ID:
    case '[':
    case '{':
    case '(':
    case '+':
    case '-':
    case '!':
    case '~':
    case '?':
    case '*':
    case '&':
        unget_token(ctxt, p);
        r = expr_stmt(ctxt, p, expr);
        break;
    case ';':
        r = OX_OK;
        break;
    case OX_TOKEN_ID:
        switch (TOK->keyword) {
        case OX_KEYWORD_if:
            r = if_stmt(ctxt, p);
            break;
        case OX_KEYWORD_do:
            r = do_while_stmt(ctxt, p);
            break;
        case OX_KEYWORD_while:
            r = while_stmt(ctxt, p);
            break;
        case OX_KEYWORD_for:
            r = for_stmt(ctxt, p);
            break;
        case OX_KEYWORD_sched:
            r = sched_stmt(ctxt, p);
            break;
        case OX_KEYWORD_case:
            r = case_stmt(ctxt, p);
            break;
        case OX_KEYWORD_try:
            r = try_stmt(ctxt, p);
            break;
        case OX_KEYWORD_return:
            r = return_stmt(ctxt, p);
            break;
        case OX_KEYWORD_throw:
            r = throw_stmt(ctxt, p);
            break;
        case OX_KEYWORD_break:
            r = break_stmt(ctxt, p);
            break;
        case OX_KEYWORD_continue:
            r = continue_stmt(ctxt, p);
            break;
        case OX_KEYWORD_typeof:
        case OX_KEYWORD_global:
        case OX_KEYWORD_yield:
            unget_token(ctxt, p);
            r = expr_stmt(ctxt, p, expr);
            break;
        case OX_KEYWORD_textdomain:
            if (p->flags & OX_PARSER_FL_TEXTDOMAIN) {
                r = textdomain_stmt(ctxt, p);
                break;
            }
        default:
            unget_token(ctxt, p);
            r = expr_stmt(ctxt, p, expr);
            break;
        }
        break;
    default:
        unexpect_token_error_s(ctxt, p, LOC, OX_TEXT("a statement"));
        r = OX_ERR;
        break;
    }

    if ((r == OX_OK)
            && !ox_value_is_null(ctxt, expr)
            && !ox_value_is_null(ctxt, doc)) {
        AST_SET(expr, doc, doc);
    }

    OX_VS_POP(ctxt, expr)
    return r;
}

/*Add a reference item to the script.*/
static OX_Result
add_ref (OX_Context *ctxt, OX_Parser *p, OX_Value *file, OX_Value *orig, OX_Value *name)
{
    OX_VS_PUSH_6(ctxt, refs, fn, ent, items, item, n)

    AST_GET(p->ast, refs, refs);
    if (ox_value_is_null(ctxt, refs)) {
        ox_not_error(ox_object_new(ctxt, refs, NULL));
        AST_SET(p->ast, refs, refs);
    }

    AST_GET(file, value, fn);

    ox_not_error(ox_get(ctxt, refs, fn, ent));
    if (ox_value_is_null(ctxt, ent)) {
        AST_NEW(ent, ref);
        AST_SET(ent, file, file);
        ox_not_error(ox_set(ctxt, refs, fn, ent));
    }

    if (orig && !ox_value_is_null(ctxt, orig)) {
        AST_GET(ent, items, items);
        if (ox_value_is_null(ctxt, items)) {
            ARRAY_NEW(items);
            AST_SET(ent, items, items);
        }

        AST_NEW(item, ref_item);
        AST_SET(item, orig, orig);

        if (name && !ox_value_is_null(ctxt, name)) {
            OX_Location loc;

            GET_LOC(name, &loc);
            AST_GET(name, value, n);
            AST_SET(item, name, name);
            add_decl(ctxt, p, n, &loc, OX_DECL_REF);
        }

        if (p->flags & OX_PARSER_FL_PUBLIC)
            AST_SET_B(item, public, OX_TRUE);

        ARRAY_APPEND(items, item);
    }

    OX_VS_POP(ctxt, refs)
    return OX_OK;
}

/*Parse an reference item.*/
static OX_Result
ref_item (OX_Context *ctxt, OX_Parser *p, OX_Value *file)
{
    OX_VS_PUSH_2(ctxt, name, orig)
    OX_Bool all = OX_FALSE;
    OX_Result r;

    get_token(ctxt, p);
    if (TOK->type == '*') {
        AST_NEW(orig, all);
        SET_LOC(orig, LOC);
        all = OX_TRUE;
    } else if (TOK->type == OX_TOKEN_ID) {
        AST_NEW(orig, value);
        AST_SET(orig, value, TOK->v);
        SET_LOC(orig, LOC);
    } else {
        unexpect_token_error(ctxt, p, LOC, OX_TOKEN_ID, '*', -1);
        unget_token(ctxt, p);
        r = OX_ERR;
        goto end;
    }

    get_token(ctxt, p);
    if (NEWLINE || !is_keyword(TOK, OX_KEYWORD_as)) {
        unget_token(ctxt, p);

        if (!all)
            ox_value_copy(ctxt, name, orig);
    } else {
        if ((r = get_expect_token(ctxt, p, OX_TOKEN_ID)) == OX_ERR)
            goto end;

        AST_NEW(name, value);
        AST_SET(name, value, TOK->v);
        SET_LOC(name, LOC);
    }

    add_ref(ctxt, p, file, orig, name);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, name)
    return r;
}

/*Parse an reference block.*/
static OX_Result
ref_block (OX_Context *ctxt, OX_Parser *p, OX_Value *file)
{
    OX_Result r;
    OX_Bool has_item = OX_FALSE;

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == '}')
            break;

        unget_token(ctxt, p);

        has_item = OX_TRUE;

        r = ref_item(ctxt, p, file);

        if (r == OX_OK) {
            get_token(ctxt, p);
            if (TOK->type == '}')
                break;
            if (TOK->type == ',') {
            } else if (NEWLINE) {
                unget_token(ctxt, p);
            } else {
                unexpect_token_error(ctxt, p, LOC, ',', '}', '\n', -1);
                unget_token(ctxt, p);
                r = OX_ERR;
            }
        }

        if (r == OX_ERR) {
            if ((r = error_recovery(ctxt, p,
                    OX_RECOVERY_COMMA
                    |OX_RECOVERY_RB)) == OX_ERR)
                break;
        }
    }

    if (!has_item)
        add_ref(ctxt, p, file, NULL, NULL);

    return OX_OK;
}

/*Parse "ref" statement.*/
static OX_Result
ref_stmt (OX_Context *ctxt, OX_Parser *p)
{
    OX_VS_PUSH_2(ctxt, file, name)
    OX_Result r;

    if ((r = get_expect_token(ctxt, p, OX_TOKEN_STRING)) == OX_ERR)
        goto end;

    AST_NEW(file, value);
    AST_SET(file, value, TOK->v);
    SET_LOC(file, LOC);

    get_token(ctxt, p);

    if (NEWLINE || (TOK->type == ';')) {
        unget_token(ctxt, p);
        AST_NEW(name, value);
        AST_SET(name, value, OX_STRING(ctxt, star));
        SET_LOC(name, LOC);
        add_ref(ctxt, p, file, name, NULL);
    } else if (TOK->type == '{') {
        if ((r = ref_block(ctxt, p, file)) == OX_ERR)
            goto end;
    } else if ((TOK->type == '*') || (TOK->type == OX_TOKEN_ID)) {
        unget_token(ctxt, p);
        if ((r = ref_item(ctxt, p, file)) == OX_ERR)
            goto end;
    } else {
        unexpect_token_error(ctxt, p, LOC, ';', '{', '*', OX_TOKEN_ID, -1);
        unget_token(ctxt, p);
        r = OX_ERR;
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, file)
    return r;
}

/*Parse script item.*/
static OX_Result
script_item (OX_Context *ctxt, OX_Parser *p)
{
    OX_Result r;

    get_token(ctxt, p);

    if (is_keyword(TOK, OX_KEYWORD_public)) {
        int old_flags = p->flags;

        p->flags |= OX_PARSER_FL_PUBLIC;

        get_token(ctxt, p);

        if (is_keyword(TOK, OX_KEYWORD_ref)) {
            r = ref_stmt(ctxt, p);
        } else {
            unget_token(ctxt, p);
            r = statement(ctxt, p);
        }

        p->flags = old_flags;
    } else if (is_keyword(TOK, OX_KEYWORD_ref)) {
        r = ref_stmt(ctxt, p);
    } else {
        unget_token(ctxt, p);

        r = statement(ctxt, p);
    }

    return r;
}

/*Parse a script.*/
static void
parse_script (OX_Context *ctxt, OX_Parser *p)
{
    OX_Result r;
    OX_Location loc;

    loc.first_line = 1;
    loc.first_column = 1;

    script_new(ctxt, p);

    while (1) {
        get_token(ctxt, p);

        if (TOK->type == OX_TOKEN_END)
            break;

        unget_token(ctxt, p);
        if ((r = script_item(ctxt, p)) == OX_ERR) {
            if ((r = error_recovery(ctxt, p,
                    OX_RECOVERY_LF
                    |OX_RECOVERY_SEMICOLON
                    |OX_RECOVERY_BLOCK)) == OX_ERR)
                break;
        }
    }

    last_loc(&loc, LOC);
    SET_LOC(p->func, &loc);
}

/**
 * Parse a script to abstract syntax tree.
 * @param ctxt The current running context.
 * @param input The input value.
 * @param[out] ast Return the result AST value.
 * @param flags Parse flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_parse (OX_Context *ctxt, OX_Value *input, OX_Value *ast, int flags)
{
    OX_Parser p;
    OX_Input *inp;

    assert(ctxt && input && ast);
    assert(ox_value_is_input(ctxt, input));

    inp = ox_value_get_gco(ctxt, input);

    parser_init(ctxt, &p, inp, ast);

    if (flags & OX_PARSE_FL_RETURN)
        p.flags |= OX_PARSER_FL_RETURN;
    
    if (flags & OX_PARSE_FL_NO_PROMPT) {
        p.status |= OX_PRASER_ST_NO_PROMPT;
        p.lex.status |= OX_LEX_ST_NO_PROMPT;
    }

    if (flags & OX_PARSE_FL_DOC)
        p.lex.status |= OX_LEX_ST_DOC;

    p.flags |= OX_PARSER_FL_TEXTDOMAIN;

    parse_script(ctxt, &p);

    parser_deinit(ctxt, &p);

    if (ox_input_error(inp)
            || (p.lex.status & OX_LEX_ST_ERR)
            || (p.status & OX_PARSER_ST_ERR)) {
        return ox_throw_syntax_error(ctxt,
                OX_TEXT("error occurred while parsing \"%s\""),
                inp->name);
    }

    return OX_OK;
}