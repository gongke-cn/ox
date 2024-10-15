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
 * Compile functions.
 */

#define OX_LOG_TAG "ox_compile"

#include "ox_internal.h"

/** Compile error.*/
#define OX_COMPILE_FL_ERROR (1 << 10)

/** Compiler value entry.*/
typedef struct {
    OX_HashEntry he; /**< Hash table entry data.*/
    OX_Value     v;  /**< The value.*/
    size_t       id; /**< The value's index.*/
} OX_CompValue;

/** Compiler jump label.*/
typedef struct {
    int off;         /**< Offset of the label.*/
    int stack_level; /**< The stack level of the label.*/
} OX_CompLabel;

/** Compiler register.*/
typedef struct {
    int id;  /**< Register.*/
    int off; /**< The offset where reference this register.*/
} OX_CompRegister;

/** Compiler data.*/
typedef struct {
    OX_Input      *input;   /**< The input.*/
    OX_BcScript   *s;       /**< The result script.*/
    OX_ScriptFunc *sf;      /**< Current script function.*/
    OX_Value      *f;       /**< Current function's AST.*/
    OX_Hash        cv_hash; /**< Constant value hash table.*/
    OX_Hash        pp_hash; /**< Private property name hash table.*/
    OX_Hash        lt_hash; /**< Localized text string hash table.*/
    OX_Hash        ltt_hash;/**< Localized string template hash table.*/
    OX_VECTOR_TYPE_DECL(OX_CompLabel)    labels; /**< Labels array.*/
    OX_VECTOR_TYPE_DECL(OX_Command)      cmds;   /**< Commands array.*/
    OX_VECTOR_TYPE_DECL(OX_CompRegister) regs;   /**< Registers array.*/
    OX_VECTOR_TYPE_DECL(uint8_t)         bc;     /**< Byte code buffer.*/
    OX_VECTOR_TYPE_DECL(OX_ScriptLoc)    ltab;   /**< Location table.*/
    int            break_label;    /**< Break label.*/
    int            continue_label; /**< Continue label.*/
    int            ques_label;     /**< Question null check label.*/
    int            ques_r;         /**< Question null check result register.*/
    int            reg_lifetimes[256]; /**< Lifetimes of the registers.*/
    int            flags;   /**< Flags.*/
    int            bot_frame_num;  /**< The bottom frame number.*/
    int            this_r;  /**< This register.*/
    int            owned_num;      /**< Number of owned objects.*/
    int            stack_level;    /**< The current stack level.*/
    OX_Location    loc;     /**< The current location.*/
} OX_Compiler;

/** Declaration's scope.*/
typedef enum {
    OX_DECL_SCOPE_PUBLIC, /**< Public declaration.*/
    OX_DECL_SCOPE_LOCAL,  /**< Local declaration.*/
    OX_DECL_SCOPE_TEMP    /**< Temporary declaration.*/
} OX_DeclScope;

/** Assignment left expression type.*/
typedef enum {
    OX_ASSI_LEFT_GLOBAL, /**< Global declaration.*/
    OX_ASSI_LEFT_DECL,   /**< Declaration.*/
    OX_ASSI_LEFT_PROP,   /**< Property.*/
    OX_ASSI_LEFT_ARRAY,  /**< Array pattern.*/
    OX_ASSI_LEFT_OBJECT, /**< Object pattern.*/
    OX_ASSI_LEFT_VALUE   /**< Pointed value.*/
} OX_AssiLeftType;

/** Assignment left expression.*/
typedef struct {
    OX_AssiLeftType type;  /**< Expression type.*/
    OX_Value       *ast;   /**< AST node.*/
    int             cv_id; /**< Constant value index.*/
    int             br_id; /**< Base value register's index.*/
    int             pr_id; /**< Property key register's index.*/
    OX_ScriptDecl  *decl;  /**< The declaration.*/
    int             depth; /**< Declaration depth.*/
    OX_Location     loc;   /**< Location.*/
} OX_AssiLeft;

/** Null check question data.*/
typedef struct {
    OX_Bool is_src; /**< Is a question source.*/
    OX_Bool is_dst; /**< Is a question destination.*/
    int     old_r;  /**< Old register.*/
    int     old_l;  /**< Old label.*/
} OX_QuesData;

/*Convert the property name to commands.*/
static void
prop_name_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *pn, int rr);
/*Register value to commands.*/
static void
left_ast_assi (OX_Context *ctxt, OX_Compiler *c, OX_Value *left, int rr, OX_Location *loc);
/*Convert the expression to commands.*/
static void
expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er);
/*Convert the block to commands.*/
static void
block_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *blk, int rr, OX_BlockContentType ctype, int or);
/*Convert if statment to commands.*/
static void
if_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s, int rr, OX_BlockContentType ctype, int or);
/*Convert case statment to commands.*/
static void
case_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s, int rr, OX_BlockContentType ctype, int or);

/*Initialize the compiler.*/
static void
compiler_init (OX_Context *ctxt, OX_Compiler *c, OX_Input *input, int flags)
{
    c->loc.first_line = 0;
    c->loc.first_column = 0;
    c->loc.last_line = 0;
    c->loc.last_column = 0;
    c->input = input;
    c->s = NULL;
    c->sf = NULL;
    c->f = NULL;
    c->flags = flags;
    c->bot_frame_num = 0;
    c->this_r = -1;
    c->owned_num = 0;

    ox_value_hash_init(&c->cv_hash);
    ox_value_hash_init(&c->pp_hash);
    ox_value_hash_init(&c->lt_hash);
    ox_value_hash_init(&c->ltt_hash);
    ox_vector_init(&c->labels);
    ox_vector_init(&c->cmds);
    ox_vector_init(&c->regs);
    ox_vector_init(&c->bc);
    ox_vector_init(&c->ltab);

    if (flags & OX_COMPILE_FL_CURR) {
        OX_Frame *fr = ox_frame_get(ctxt);

        if (fr) {
            OX_Function *f = ox_value_get_gco(ctxt, &fr->func);

            c->bot_frame_num += f->sfunc->frame_num + 1;
        }
    }
}

/*Release the compiler.*/
static void
compiler_deinit (OX_Context *ctxt, OX_Compiler *c)
{
    OX_CompValue *v, *nv;
    size_t i;

    ox_hash_foreach_safe_c(&c->cv_hash, i, v, nv, OX_CompValue, he) {
        OX_DEL(ctxt, v);
    }

    ox_hash_foreach_safe_c(&c->pp_hash, i, v, nv, OX_CompValue, he) {
        OX_DEL(ctxt, v);
    }

    ox_hash_foreach_safe_c(&c->lt_hash, i, v, nv, OX_CompValue, he) {
        OX_DEL(ctxt, v);
    }

    ox_hash_foreach_safe_c(&c->ltt_hash, i, v, nv, OX_CompValue, he) {
        OX_DEL(ctxt, v);
    }

    ox_hash_deinit(ctxt, &c->cv_hash);
    ox_hash_deinit(ctxt, &c->pp_hash);
    ox_hash_deinit(ctxt, &c->lt_hash);
    ox_hash_deinit(ctxt, &c->ltt_hash);
    ox_vector_deinit(ctxt, &c->labels);
    ox_vector_deinit(ctxt, &c->cmds);
    ox_vector_deinit(ctxt, &c->regs);
    ox_vector_deinit(ctxt, &c->bc);
    ox_vector_deinit(ctxt, &c->ltab);
}

/*Output error prompt message.*/
static void
error (OX_Context *ctxt, OX_Compiler *c, OX_Location *loc, const char *fmt, ...)
{
    va_list ap;

    c->flags |= OX_COMPILE_FL_ERROR;

    va_start(ap, fmt);
    ox_prompt_v(ctxt, c->input, loc, OX_PROMPT_ERROR, fmt, ap);
    va_end(ap);
}

/*Set the current location.*/
static void
set_loc (OX_Compiler *c, OX_Location *loc)
{
    c->loc = *loc;
}

/*Set the current location as the start position of loc.*/
static void
set_start_loc (OX_Compiler *c, OX_Location *loc, int w)
{
    c->loc.first_line = loc->first_line;
    c->loc.first_column = loc->first_column;
    c->loc.last_line = loc->first_line;
    c->loc.last_column = loc->first_column + w - 1;
}

/*Set the current location as the end position of loc.*/
static void
set_end_loc (OX_Compiler *c, OX_Location *loc, int w)
{
    c->loc.first_line = loc->last_line;
    c->loc.first_column = loc->last_column;
    c->loc.last_line = loc->last_line;
    c->loc.last_column = loc->last_column - w + 1;
}

/*Get the location of the AST node.*/
#define GET_LOC(n, l)  ox_ast_get_loc(ctxt, n, l)

/*Get the AST node's property.*/
static void
ast_get (OX_Context *ctxt, OX_Value *ast, OX_Value *p, OX_Value *v)
{
    ox_not_error(ox_get(ctxt, ast, p, v));
}

/*Get the AST node's property.*/
#define AST_GET(n, p, v) ast_get(ctxt, n, OX_STRING(ctxt, p), v)

/*Get the AST node's number type property.*/
static OX_Number
ast_get_number (OX_Context *ctxt, OX_Value *ast, OX_Value *p)
{
    OX_VS_PUSH(ctxt, v)
    OX_Number n;

    ast_get(ctxt, ast, p, v);

    n = ox_value_get_number(ctxt, v);

    OX_VS_POP(ctxt, v)
    return n;
}

/*Get the AST node's number type property.*/
#define AST_GET_N(n, p) ast_get_number(ctxt, n, OX_STRING(ctxt, p))

/*Get the AST node's boolean type property.*/
static OX_Number
ast_get_bool (OX_Context *ctxt, OX_Value *ast, OX_Value *p)
{
    OX_VS_PUSH(ctxt, v)
    OX_Bool b;

    ast_get(ctxt, ast, p, v);

    b = ox_to_bool(ctxt, v);

    OX_VS_POP(ctxt, v)
    return b;
}

/*Get the AST node's boolean type property.*/
#define AST_GET_B(n, p) ast_get_bool(ctxt, n, OX_STRING(ctxt, p))

/*Check if the AST node is a declaration.*/
static OX_Bool
is_decl (OX_Context *ctxt, OX_Value *ast)
{
    OX_AstType aty;

    aty = ox_ast_get_type(ctxt, ast);

    switch (aty) {
    case OX_AST_func:
    case OX_AST_class:
        return OX_TRUE;
    default:
        return OX_FALSE;
    }
}

/*Add a constant value.*/
static int
add_cv (OX_Context *ctxt, OX_Compiler *c, OX_Value *v)
{
    OX_HashEntry **pe;
    OX_CompValue *ent;

    ent = ox_hash_lookup_c(ctxt, &c->cv_hash, v, &pe, OX_CompValue, he);
    if (!ent) {
        ox_not_null(OX_NEW(ctxt, ent));

        ent->id = c->cv_hash.e_num;
        ox_value_copy(ctxt, &ent->v, v);

        ox_not_error(ox_hash_insert(ctxt, &c->cv_hash, &ent->v, &ent->he, pe));
    }

    return ent->id;
}

/*Add a private property name.*/
static int
add_pp (OX_Context *ctxt, OX_Compiler *c, OX_Value *v)
{
    OX_HashEntry **pe;
    OX_CompValue *ent;

    ent = ox_hash_lookup_c(ctxt, &c->pp_hash, v, &pe, OX_CompValue, he);
    if (!ent) {
        ox_not_null(OX_NEW(ctxt, ent));

        ent->id = c->pp_hash.e_num;
        ox_value_copy(ctxt, &ent->v, v);

        ox_not_error(ox_hash_insert(ctxt, &c->pp_hash, &ent->v, &ent->he, pe));
    }

    return ent->id;
}

/*Add a localized text string.*/
static int
add_lt (OX_Context *ctxt, OX_Compiler *c, OX_Value *v)
{
    OX_HashEntry **pe;
    OX_CompValue *ent;

    ent = ox_hash_lookup_c(ctxt, &c->lt_hash, v, &pe, OX_CompValue, he);
    if (!ent) {
        ox_not_null(OX_NEW(ctxt, ent));

        ent->id = c->lt_hash.e_num;
        ox_value_copy(ctxt, &ent->v, v);

        ox_not_error(ox_hash_insert(ctxt, &c->lt_hash, &ent->v, &ent->he, pe));
    }

    return ent->id;
}

/*Add a localized string template.*/
static int
add_ltt (OX_Context *ctxt, OX_Compiler *c, OX_Value *v)
{
    OX_VS_PUSH(ctxt, s)
    OX_HashEntry **pe;
    OX_CompValue *ent;

    ox_not_error(ox_str_templ_to_str(ctxt, v, s));
    ox_not_error(ox_set(ctxt, v, OX_STRING(ctxt, value), s));

    ent = ox_hash_lookup_c(ctxt, &c->ltt_hash, s, &pe, OX_CompValue, he);
    if (!ent) {
        ox_not_null(OX_NEW(ctxt, ent));

        ent->id = c->ltt_hash.e_num;
        ox_value_copy(ctxt, &ent->v, s);

        ox_not_error(ox_hash_insert(ctxt, &c->ltt_hash, &ent->v, &ent->he, pe));
    }

    OX_VS_POP(ctxt, s)
    return ent->id;
}

/*Allocate a new label with stack level offset.*/
static int
add_label_l (OX_Context *ctxt, OX_Compiler *c, int sl)
{
    int id = c->labels.len;
    OX_CompLabel *l;

    ox_not_error(ox_vector_expand(ctxt, &c->labels, id + 1));

    l = &ox_vector_item(&c->labels, id);
    l->off = -1;
    l->stack_level = c->stack_level + sl;

    return id;
}

/*Allocate a new label.*/
static int
add_label (OX_Context *ctxt, OX_Compiler *c)
{
    return add_label_l(ctxt, c, 0);
}

/*Add a new command.*/
static int
add_cmd (OX_Context *ctxt, OX_Compiler *c, OX_ByteCode bc)
{
    int id = c->cmds.len;
    OX_Command *cmd;

    ox_not_error(ox_vector_expand(ctxt, &c->cmds, id + 1));

    cmd = &ox_vector_item(&c->cmds, id);
    cmd->bc = bc;
    cmd->g.loc = c->loc;

    return id;
}

/*Allocate a new register.*/
static int
add_reg (OX_Context *ctxt, OX_Compiler *c)
{
    int id = c->regs.len;
    OX_CompRegister *r;

    ox_not_error(ox_vector_expand(ctxt, &c->regs, id + 1));

    r = &ox_vector_item(&c->regs, id);
    r->id = -1;
    r->off = -1;

    return id;
}

/*Add declarations.*/
static void
add_decls (OX_Context *ctxt, OX_Compiler *c)
{
    OX_VS_PUSH_5(ctxt, decls, iter, e, name, decl)
    OX_BcScript *s = c->s;
    OX_ScriptFunc *sf = c->sf;
    OX_Value *f = c->f;

    AST_GET(f, decls, decls);

    if (!ox_value_is_null(ctxt, decls)) {
        ox_not_error(ox_object_iter_new(ctxt, iter, decls, OX_OBJECT_ITER_KEY_VALUE));
        while (!ox_iterator_end(ctxt, iter)) {
            OX_DeclType type;
            OX_Bool pub;
            int id;

            ox_not_error(ox_iterator_value(ctxt, iter, e));
            ox_not_error(ox_array_get_item(ctxt, e, 0, name));
            ox_not_error(ox_array_get_item(ctxt, e, 1, decl));
            ox_not_error(ox_iterator_next(ctxt, iter));

            type = AST_GET_N(decl, decl_type);
            if (type == OX_DECL_OUTER)
                continue;

            pub = AST_GET_B(decl, public);

            /*Add declaration entry.*/
            id = ox_script_func_add_decl(ctxt, sf, type, name);
            assert(id != -1);

            /*Add public entry.*/
            if (pub) {
                id = ox_script_add_public(ctxt, &s->script, name, id);
                assert(id != -1);
            }
        }
    }

    OX_VS_POP(ctxt, decls)
}

/*Allocate register.*/
static int
alloc_reg (OX_Context *ctxt, OX_Compiler *c, int start, int end)
{
    int i;

    for (i = 0; i < OX_N_ELEM(c->reg_lifetimes); i ++) {
        int *pr = c->reg_lifetimes + i;

        if (*pr < start) {
            *pr = end;
            c->sf->reg_num = OX_MAX(c->sf->reg_num, i + 1);
            return i;
        }
    }

    ox_throw_range_error(ctxt, OX_TEXT("too many registers used"));
    return -1;
}

/*Dump a value.*/
static void
dump_value (OX_Context *ctxt, OX_Value *v, FILE *fp)
{
    switch (ox_value_get_type(ctxt, v)) {
    case OX_VALUE_NUMBER:
        fprintf(fp, "%g", ox_value_get_number(ctxt, v));
        break;
    default:
        if (ox_value_is_re(ctxt, v)) {
            OX_VS_PUSH(ctxt, s)

            ox_not_error(ox_to_string(ctxt, v, s));
            fprintf(fp, "%s", ox_string_get_char_star(ctxt, s));

            OX_VS_POP(ctxt, s)
        } else if (ox_value_is_string(ctxt, v)) {
            const char *pc = ox_string_get_char_star(ctxt, v);
            const char *cstr;
            OX_CharBuffer cb;
            size_t len = 0;

            ox_char_buffer_init(&cb);

            while (*pc) {
                if (len > 16) {
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "..."));
                    break;
                }

                switch (*pc) {
                case '\n':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\n"));
                    break;
                case '\r':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\r"));
                    break;
                case '\t':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\t"));
                    break;
                case '\v':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\v"));
                    break;
                case '\a':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\a"));
                    break;
                case '\b':
                   ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\b")); 
                    break;
                case '\"':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\\""));
                    break;
                case '\\':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\\\"));
                    break;
                case '{':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\{"));
                    break;
                case '}':
                    ox_not_error(ox_char_buffer_append_char_star(ctxt, &cb, "\\}"));
                    break;
                default:
                    if (ox_char_is_graph(*pc) || (*pc == ' '))
                        ox_not_error(ox_char_buffer_append_char(ctxt, &cb, *pc));
                    else
                        ox_not_error(ox_char_buffer_print(ctxt, &cb, "\\x%02x", *pc));
                    break;
                }

                pc ++;
                len ++;
            }

            ox_not_null(cstr = ox_char_buffer_get_char_star(ctxt, &cb));
            fprintf(fp, "\"%s\"", cstr);
            ox_char_buffer_deinit(ctxt, &cb);
        }
        break;
    }
}

/*Dump the constant value.*/
static void
dump_const (OX_Context *ctxt, OX_BcScript *s, int id, FILE *fp)
{
    OX_Value *c = &s->cvs[id];

    dump_value(ctxt, c, fp);
}

/*Dump the private property.*/
static void
dump_private (OX_Context *ctxt, OX_BcScript *s, int id, FILE *fp)
{
    OX_Value *v = &s->pps[id];
    const char *c = ox_string_get_char_star(ctxt, v);
    const char *p = strchr(c, '@');
    OX_CharBuffer cb;

    ox_char_buffer_init(&cb);

    ox_not_error(ox_char_buffer_append_chars(ctxt, &cb, c, p - c));

    ox_not_null(c = ox_char_buffer_get_char_star(ctxt, &cb));

    fprintf(fp, "\"%s\"", c);

    ox_char_buffer_deinit(ctxt, &cb);
}

/*Dump the local text value.*/
static void
dump_local_text (OX_Context *ctxt, OX_BcScript *s, int id, FILE *fp)
{
    OX_Value *v = &s->ts[id];

    dump_value(ctxt, v, fp);
}

/*Dump the local text template.*/
static void
dump_local_templ (OX_Context *ctxt, OX_BcScript *s, int id, FILE *fp)
{
    OX_Value *v = &s->tts[id];

    dump_value(ctxt, v, fp);
}

#include "ox_command.h"

/*Check the expression is not null.*/
static void
check_not_null (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_AstType aty = ox_ast_get_type(ctxt, e);
    OX_VS_PUSH_3(ctxt, v, op, p)
    int cid, pid;
    OX_Location loc;

    if (aty == OX_AST_id) {
        GET_LOC(e, &loc);
        set_loc(c, &loc);

        AST_GET(e, value, v);
        cid = add_cv(ctxt, c, v);
        cmd_name_nn(ctxt, c, cid, er);
    } else if (aty == OX_AST_binary_expr) {
        OX_AstType type;

        AST_GET(e, operator, op);

        type = ox_ast_get_type(ctxt, op);

        if ((type == OX_AST_get) || (type == OX_AST_lookup)) {
            AST_GET(e, operand2, p);

            AST_GET(p, value, v);

            if (AST_GET_B(p, private)) {
                pid = add_pp(ctxt, c, v);
                cmd_pprop_nn(ctxt, c, pid, er);
            } else {
                cid = add_cv(ctxt, c, v);
                cmd_prop_nn(ctxt, c, cid, er);
            }
        }
    }

    OX_VS_POP(ctxt, v)
}

/*Argument to commands.*/
static void
arg_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    int id = AST_GET_N(e, id);
    OX_Location loc;

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    cmd_get_a(ctxt, c, id, er);
}

/*Current object to commands.*/
static void
curr_object_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_Location loc;

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    cmd_curr(ctxt, c, er);
}

/*this argument to commands.*/
static void
this_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_3(ctxt, f, bot, id)
    int depth = 0;
    OX_Bool found = OX_FALSE;
    OX_Location loc;

    if (c->this_r != -1) {
        cmd_dup(ctxt, c, c->this_r, er);
        return;
    }

    ox_value_copy(ctxt, f, c->f);

    while (1) {
        AST_GET(f, id, id);

        if (!ox_value_is_null(ctxt, id)) {
            if (AST_GET_B(f, this)) {
                found = OX_TRUE;
                break;
            }

            depth ++;
        }

        AST_GET(f, outer, bot);

        if (ox_value_is_null(ctxt, bot))
            break;

        ox_value_copy(ctxt, f, bot);
    }

    if (!found && (c->flags & OX_COMPILE_FL_CURR)) {
        /*Lookup in the running function.*/
        OX_Frame *frame = ox_frame_get(ctxt);
        OX_Function *bf;

        if (frame) {
            depth ++;

            bf = ox_value_get_gco(ctxt, &frame->func);
            if (bf->sfunc->flags & OX_SCRIPT_FUNC_FL_THIS)
                found = OX_TRUE;

            if (!found) {
                OX_Frame **cf = bf->frames;
                OX_Frame **ef = cf + bf->sfunc->frame_num;

                while (cf < ef) {
                    frame = *cf;
                    depth ++;
                    
                    bf = ox_value_get_gco(ctxt, &frame->func);
                    if (bf->sfunc->flags & OX_SCRIPT_FUNC_FL_THIS) {
                        found = OX_TRUE;
                        break;
                    }

                    cf ++;
                }
            }
        }
    }
    
    GET_LOC(e, &loc);

    if (!found) {
        error(ctxt, c, &loc, OX_TEXT("the function has not this argument"));
    } else if (depth) {
        set_loc(c, &loc);
        cmd_this_b(ctxt, c, depth - 1, er);
    } else {
        set_loc(c, &loc);
        cmd_this(ctxt, c, er);
    }

    OX_VS_POP(ctxt, f)
}

/*"argv" to commands.*/
static void
argv_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_Location loc;

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    cmd_argv(ctxt, c, er);
}

/*Convert function to commands.*/
static void
func_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    int fid;
    OX_Location loc;

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    fid = AST_GET_N(e, id);
    cmd_f_new(ctxt, c, fid, er);
}

/*Convert the enumeration to commands.*/
static void
enum_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int cr)
{
    OX_VS_PUSH_4(ctxt, name, v, items, item)

    AST_GET(e, name, name);
    if (!ox_value_is_null(ctxt, name)) {
        int cid;

        AST_GET(name, value, v);
        cid = add_cv(ctxt, c, v);

        cmd_e_start_n(ctxt, c, cid, cr);
    } else {
        cmd_e_start(ctxt, c, cr);
    }

    AST_GET(e, items, items);
    if (!ox_value_is_null(ctxt, items)) {
        size_t len, i;

        len = ox_array_length(ctxt, items);
        for (i = 0; i < len; i ++) {
            int cid;

            ox_not_error(ox_array_get_item(ctxt, items, i, item));

            AST_GET(item, value, v);
            cid = add_cv(ctxt, c, v);
            cmd_e_item(ctxt, c, cid);
        }
    }

    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, name)
}

/*Convert the bitfield to commands.*/
static void
bitfield_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *b, int cr)
{
    OX_VS_PUSH_4(ctxt, name, v, items, item)

    AST_GET(b, name, name);
    if (!ox_value_is_null(ctxt, name)) {
        int cid;

        AST_GET(name, value, v);
        cid = add_cv(ctxt, c, v);

        cmd_b_start_n(ctxt, c, cid, cr);
    } else {
        cmd_b_start(ctxt, c, cr);
    }

    AST_GET(b, items, items);
    if (!ox_value_is_null(ctxt, items)) {
        size_t len, i;

        len = ox_array_length(ctxt, items);
        for (i = 0; i < len; i ++) {
            int cid;

            ox_not_error(ox_array_get_item(ctxt, items, i, item));

            AST_GET(item, value, v);
            cid = add_cv(ctxt, c, v);
            cmd_b_item(ctxt, c, cid);
        }
    }

    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, name)
}

/*Convert the class's properies to commands.*/
static void
class_props_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int cr, int ir)
{
    OX_VS_PUSH_6(ctxt, props, prop, pn, pe, getv, setv)
    size_t i, len;
    OX_Location loc, floc;

    AST_GET(e, props, props);
    if (!ox_value_is_null(ctxt, props)) {
        len = ox_array_length(ctxt, props);
        for (i = 0; i < len; i ++) {
            OX_AstType aty;
            OX_Bool is_static;
            int vr, fr, getr, setr;
            int sr = -1, pr = -1;
            int fid;

            ox_not_error(ox_array_get_item(ctxt, props, i, prop));
            aty = ox_ast_get_type(ctxt, prop);

            if ((aty != OX_AST_enum) && (aty != OX_AST_bitfield)) {
                is_static = AST_GET_B(prop, static);
                sr = is_static ? cr : ir;

                AST_GET(prop, name, pn);
                pr = add_reg(ctxt, c);
                prop_name_to_cmds(ctxt, c, pn, pr);

                GET_LOC(pn, &loc);
            }

            switch (aty) {
            case OX_AST_enum:
                enum_to_cmds(ctxt, c, prop, cr);
                break;
            case OX_AST_bitfield:
                bitfield_to_cmds(ctxt, c, prop, cr);
                break;
            case OX_AST_var:
                AST_GET(prop, expr, pe);
                vr = add_reg(ctxt, c);

                if (ox_value_is_null(ctxt, pe)) {
                    set_loc(c, &loc);
                    cmd_load_null(ctxt, c, vr);
                } else {
                    expr_to_cmds(ctxt, c, pe, vr);
                    set_loc(c, &loc);
                }

                cmd_c_var(ctxt, c, sr, pr, vr);

                if (!ox_value_is_null(ctxt, pe) && is_decl(ctxt, pe)) {
                    cmd_set_name(ctxt, c, vr, pr);
                    cmd_set_scope(ctxt, c, vr, sr);
                }
                break;
            case OX_AST_const:
                AST_GET(prop, expr, pe);
                vr = add_reg(ctxt, c);
                expr_to_cmds(ctxt, c, pe, vr);

                set_loc(c, &loc);
                cmd_c_const(ctxt, c, sr, pr, vr);

                if (is_decl(ctxt, pe)) {
                    cmd_set_name(ctxt, c, vr, pr);
                    cmd_set_scope(ctxt, c, vr, sr);
                }
                break;
            case OX_AST_method:
                AST_GET(prop, expr, pe);
                fid = AST_GET_N(pe, id);
                fr = add_reg(ctxt, c);

                set_loc(c, &loc);
                cmd_f_new(ctxt, c, fid, fr);
                cmd_c_const(ctxt, c, sr, pr, fr);
                cmd_set_name(ctxt, c, fr, pr);
                cmd_set_scope(ctxt, c, fr, sr);
                break;
            case OX_AST_accessor:
                AST_GET(prop, get, getv);
                AST_GET(prop, set, setv);

                fid = AST_GET_N(getv, id);
                getr = add_reg(ctxt, c);
                GET_LOC(getv, &floc);
                set_loc(c, &floc);
                cmd_f_new(ctxt, c, fid, getr);

                if (ox_value_is_null(ctxt, setv)) {
                    set_loc(c, &loc);
                    cmd_c_ro_acce(ctxt, c, sr, pr, getr);
                    cmd_set_name_g(ctxt, c, getr, pr);
                    cmd_set_scope(ctxt, c, getr, sr);
                } else {
                    fid = AST_GET_N(setv, id);
                    setr = add_reg(ctxt, c);
                    GET_LOC(setv, &floc);
                    set_loc(c, &floc);
                    cmd_f_new(ctxt, c, fid, setr);
                    set_loc(c, &loc);
                    cmd_c_acce(ctxt, c, sr, pr, getr, setr);
                    cmd_set_name_g(ctxt, c, getr, pr);
                    cmd_set_scope(ctxt, c, getr, sr);
                    cmd_set_name_s(ctxt, c, setr, pr);
                    cmd_set_scope(ctxt, c, setr, sr);
                }
                break;
            default:
                assert(0);
            }
        }
    }

    OX_VS_POP(ctxt, props)
}

/*Convert class to commands.*/
static void
class_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int cr)
{
    OX_VS_PUSH_2(ctxt, parents, parent)
    size_t i, len;
    int ir, old_this_r;
    OX_Location loc, ploc;

    ir = add_reg(ctxt, c);

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 5);

    cmd_c_new(ctxt, c, cr, ir);

    AST_GET(e, parents, parents);
    if (!ox_value_is_null(ctxt, parents)) {
        len = ox_array_length(ctxt, parents);
        for (i = 0; i < len; i ++) {
            int pr = add_reg(ctxt, c);

            ox_not_error(ox_array_get_item(ctxt, parents, i, parent));
            expr_to_cmds(ctxt, c, parent, pr);

            GET_LOC(parent, &ploc);
            set_loc(c, &ploc);
            cmd_c_parent(ctxt, c, cr, pr);
        }
    }

    old_this_r = c->this_r;
    c->this_r = cr;

    class_props_to_cmds(ctxt, c, e, cr, ir);

    c->this_r = old_this_r;

    OX_VS_POP(ctxt, parents)
}

/*Convert value to commands.*/
static void
value_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, v)
    OX_Location loc;

    AST_GET(e, value, v);

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    if (ox_value_is_null(ctxt, v)) {
        cmd_load_null(ctxt, c, er);
    } else if (ox_value_is_bool(ctxt, v)) {
        if (ox_value_get_bool(ctxt, v))
            cmd_load_true(ctxt, c, er);
        else
            cmd_load_false(ctxt, c, er);
    } else if (AST_GET_B(e, private)) {
        int id = add_pp(ctxt, c, v);

        cmd_get_pp(ctxt, c, id, er);
    } else if (AST_GET_B(e, local)) {
        int id = add_lt(ctxt, c, v);

        cmd_get_lt(ctxt, c, id, er);
    } else {
        int id = add_cv(ctxt, c, v);

        cmd_get_cv(ctxt, c, id, er);
    }

    OX_VS_POP(ctxt, v)
}

/*Lookup the declaration*/
static OX_ScriptDecl*
decl_lookup (OX_Context *ctxt, OX_Compiler *c, OX_Value *k, int *pdepth)
{
    OX_VS_PUSH_3(ctxt, func, bot, id)
    OX_ScriptFunc *sf = c->sf;
    OX_ScriptDecl *sd;
    int depth = 0;
    size_t fid;
    OX_String *s;

    ox_not_error(ox_string_singleton(ctxt, k));
    s = ox_value_get_gco(ctxt, k);
    ox_value_copy(ctxt, func, c->f);

    while (1) {
        AST_GET(func, id, id);
        if (!ox_value_is_null(ctxt, id)) {
            sd = ox_hash_lookup_c(ctxt, &sf->decl_hash, s, NULL, OX_ScriptDecl, he);
            if (sd)
                break;

            depth ++;
        }

        AST_GET(func, outer, bot);
        if (ox_value_is_null(ctxt, bot))
            break;

        ox_value_copy(ctxt, func, bot);
        fid = AST_GET_N(func, id);
        sf = &c->s->sfuncs[fid];
    }

    if (!sd && (c->flags & OX_COMPILE_FL_CURR)) {
        /*Lookup in the running function.*/
        OX_Frame *frame = ox_frame_get(ctxt);
        OX_Function *bf;

        if (frame) {
            bf = ox_value_get_gco(ctxt, &frame->func);
            sf = bf->sfunc;
            sd = ox_hash_lookup_c(ctxt, &sf->decl_hash, s, NULL, OX_ScriptDecl, he);

            if (!sd) {
                OX_Frame **cf = bf->frames;
                OX_Frame **ef = cf + sf->frame_num;

                while (cf < ef) {
                    frame = *cf;
                    depth ++;

                    bf = ox_value_get_gco(ctxt, &frame->func);
                    sf = bf->sfunc;
                    sd = ox_hash_lookup_c(ctxt, &sf->decl_hash, s, NULL, OX_ScriptDecl, he);
                    if (sd)
                        break;

                    cf ++;
                }
            }
        }
    }

    OX_VS_POP(ctxt, func)

    if (pdepth)
        *pdepth = depth;

    return sd;
}

/*Convert identifier reference to commands.*/
static void
id_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, id)
    OX_ScriptDecl *sd;
    int depth = 0;
    OX_Location loc;

    AST_GET(e, value, id);

    GET_LOC(e, &loc);
    set_loc(c, &loc);

    sd = decl_lookup(ctxt, c, id, &depth);

    if (sd) {
        if (depth) {
            cmd_get_t_b(ctxt, c, depth - 1, sd->id, er);
        } else {
            cmd_get_t(ctxt, c, sd->id, er);
        }
    } else {
        int cid = add_cv(ctxt, c, id);

        cmd_get_n(ctxt, c, cid, er);
    }

    OX_VS_POP(ctxt, id)
}

/*Convert multipart string to commands.*/
static void
string_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_6(ctxt, items, item, expr, fmt, func, templ)
    int tid, tr;
    size_t i, len;
    OX_Location loc, iloc;

    AST_GET(e, items, items);

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 1);

    AST_GET(e, templ, templ);

    tr = add_reg(ctxt, c);

    if (AST_GET_B(e, local)) {
        tid = add_ltt(ctxt, c, templ);
        cmd_get_ltt(ctxt, c, tid, tr);
    } else {
        tid = add_cv(ctxt, c, templ);
        cmd_get_cv(ctxt, c, tid, tr);
    }

    AST_GET(e, expr, func);

    if (!ox_value_is_null(ctxt, func)) {
        int fr = add_reg(ctxt, c);

        expr_to_cmds(ctxt, c, func, fr);
        check_not_null (ctxt, c, func, fr);
        cmd_str_start_t(ctxt, c, tr, fr);
    } else {
        cmd_str_start(ctxt, c, tr);
    }

    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        int tr = add_reg(ctxt, c);
        int cid = -1;

        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        if (ox_ast_get_type(ctxt, item) == OX_AST_format) {
            AST_GET(item, expr, expr);
            AST_GET(item, format, fmt);

            cid = add_cv(ctxt, c, fmt);
        } else {
            ox_value_copy(ctxt, expr, item);
        }

        expr_to_cmds(ctxt, c, expr, tr);

        GET_LOC(expr, &iloc);
        set_loc(c, &iloc);

        if (cid != -1)
            cmd_str_item_f(ctxt, c, cid, tr);
        else
            cmd_str_item(ctxt, c, tr);
    }

    set_end_loc(c, &loc, 1);
    cmd_str_end(ctxt, c, er);

    OX_VS_POP(ctxt, items)
}

/*Convert ( expression ) to commands.*/
static void
parenthese_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, v)

    AST_GET(e, expr, v);
    expr_to_cmds(ctxt, c, v, er);

    OX_VS_POP(ctxt, v)
}

/*Convert the property name to commands.*/
static void
prop_name_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *pn, int rr)
{
    OX_VS_PUSH_2(ctxt, e, v)
    OX_AstType aty = ox_ast_get_type(ctxt, pn);
    int cv_id, pv_id;
    OX_Location loc;

    switch (aty) {
    case OX_AST_id:
    case OX_AST_value:
        GET_LOC(pn, &loc);
        set_loc(c, &loc);

        AST_GET(pn, value, v);

        if (AST_GET_B(pn, private)) {
            pv_id = add_pp(ctxt, c, v);
            cmd_get_pp(ctxt, c, pv_id, rr);
        } else {
            cv_id = add_cv(ctxt, c, v);
            cmd_get_cv(ctxt, c, cv_id, rr);
        }
        break;
    case OX_AST_expr_name:
        AST_GET(pn, expr, e);
        expr_to_cmds(ctxt, c, e, rr);
        break;
    case OX_AST_string:
        string_to_cmds(ctxt, c, pn, rr);
        break;
    default:
        assert(0);
    }

    OX_VS_POP(ctxt, e)
}

/*Convert array items to commands.*/
static void
array_items_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *items)
{
    OX_VS_PUSH_2(ctxt, item, ie)
    size_t i, len;
    int tr;
    OX_Location iloc;

    len = ox_array_length(ctxt, items);

    if (len) {
        for (i = 0; i < len; i ++) {
            OX_AstType aty;

            ox_not_error(ox_array_get_item(ctxt, items, i, item));

            aty = ox_ast_get_type(ctxt, item);
            switch (aty) {
            case OX_AST_skip:
                GET_LOC(item, &iloc);
                set_loc(c, &iloc);
                cmd_a_next(ctxt, c);
                break;
            case OX_AST_spread:
                tr = add_reg(ctxt, c);
                AST_GET(item, expr, ie);
                expr_to_cmds(ctxt, c, ie, tr);

                GET_LOC(item, &iloc);
                set_loc(c, &iloc);
                cmd_a_spread(ctxt, c, tr);
                break;
            case OX_AST_if:
                if_to_cmds(ctxt, c, item, -1, OX_BLOCK_CONTENT_ITEM, -1);
                break;
            case OX_AST_case:
                case_to_cmds(ctxt, c, item, -1, OX_BLOCK_CONTENT_ITEM, -1);
                break;
            default:
                tr = add_reg(ctxt, c);
                expr_to_cmds(ctxt, c, item, tr);

                GET_LOC(item, &iloc);
                set_loc(c, &iloc);
                cmd_a_item(ctxt, c, tr);
                break;
            }
        }
    }

    OX_VS_POP(ctxt, item)
}

/*Convert array literal to commands.*/
static void
array_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, items)
    OX_Location loc;

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 1);
    cmd_a_new(ctxt, c, er);
    cmd_a_start(ctxt, c, er);

    AST_GET(e, items, items);
    array_items_to_cmds(ctxt, c, items);

    set_end_loc(c, &loc, 1);
    cmd_a_end(ctxt, c);

    OX_VS_POP(ctxt, items)
}

/*Convert array append to commands.*/
static void
array_append_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_2(ctxt, a, items)
    OX_Location loc;

    AST_GET(e, expr, a);
    expr_to_cmds(ctxt, c, a, er);

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 1);
    cmd_a_start(ctxt, c, er);

    AST_GET(e, items, items);
    array_items_to_cmds(ctxt, c, items);

    set_end_loc(c, &loc, 1);
    cmd_a_end(ctxt, c);

    OX_VS_POP(ctxt, a)
}

/*Convert object properties to commands.*/
static void
object_props_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *items, int er)
{
    OX_VS_PUSH_3(ctxt, item, pn, pv)
    size_t i, len;
    int tr, pr;
    OX_Location iloc;

    len = ox_array_length(ctxt, items);

    if (len) {
        for (i = 0; i < len; i ++) {
            OX_AstType aty;

            ox_not_error(ox_array_get_item(ctxt, items, i, item));

            aty = ox_ast_get_type(ctxt, item);
            switch (aty) {
            case OX_AST_spread:
                tr = add_reg(ctxt, c);
                AST_GET(item, expr, pv);
                expr_to_cmds(ctxt, c, pv, tr);

                GET_LOC(item, &iloc);
                set_loc(c, &iloc);
                cmd_o_spread(ctxt, c, tr);
                break;
            case OX_AST_if:
                if_to_cmds(ctxt, c, item, -1, OX_BLOCK_CONTENT_PROP, er);
                break;
            case OX_AST_case:
                case_to_cmds(ctxt, c, item, -1, OX_BLOCK_CONTENT_PROP, er);
                break;
            case OX_AST_enum:
                enum_to_cmds(ctxt, c, item, er);
                break;
            case OX_AST_bitfield:
                bitfield_to_cmds(ctxt, c, item, er);
                break;
            case OX_AST_prop:
                tr = add_reg(ctxt, c);
                pr = add_reg(ctxt, c);
                AST_GET(item, name, pn);
                AST_GET(item, expr, pv);
                prop_name_to_cmds(ctxt, c, pn, pr);
                expr_to_cmds(ctxt, c, pv, tr);

                GET_LOC(item, &iloc);
                set_loc(c, &iloc);
                cmd_o_prop(ctxt, c, pr, tr);
                break;
            default:
                tr = add_reg(ctxt, c);
                expr_to_cmds(ctxt, c, item, tr);
                break;
            }
        }
    }

    OX_VS_POP(ctxt, item)
}

/*Convert object literal to commands.*/
static void
object_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, items)
    OX_Location loc;

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 1);
    cmd_o_new(ctxt, c, er);
    cmd_o_start(ctxt, c, er);

    AST_GET(e, props, items);

    object_props_to_cmds(ctxt, c, items, er);

    set_end_loc(c, &loc, 1);
    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, items)
}

/*Convert object set to commands.*/
static void
object_set_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH(ctxt, items)
    OX_Location loc;

    OX_VS_PUSH(ctxt, o)

    AST_GET(e, expr, o);
    expr_to_cmds(ctxt, c, o, er);

    GET_LOC(e, &loc);
    set_start_loc(c, &loc, 1);
    cmd_o_start(ctxt, c, er);

    AST_GET(e, props, items);

    object_props_to_cmds(ctxt, c, items, er);

    set_end_loc(c, &loc, 1);
    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, o)
}

/*Convert unary expression to commands.*/
static void
unary_expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_2(ctxt, op, v)
    int tr;
    OX_ByteCode bc;
    OX_Location loc;

    AST_GET(e, operator, op);
    switch (ox_ast_get_type(ctxt, op)) {
    case OX_AST_plus:
        bc = OX_BC_to_num;
        break;
    case OX_AST_minus:
        bc = OX_BC_neg;
        break;
    case OX_AST_bit_rev:
        bc = OX_BC_rev;
        break;
    case OX_AST_logic_not:
        bc = OX_BC_not;
        break;
    case OX_AST_typeof:
        bc = OX_BC_typeof;
        break;
    case OX_AST_global:
        bc = OX_BC_global;
        break;
    case OX_AST_get_ptr:
        bc = OX_BC_get_ptr;
        break;
    case OX_AST_get_value:
        bc = OX_BC_get_value;
        break;
    case OX_AST_owned:
        bc = OX_BC_owned;
        break;
    case OX_AST_yield:
        bc = OX_BC_yield;
        break;
    default:
        assert(0);
    }

    if ((bc == OX_BC_global) || (bc == OX_BC_owned)) {
        tr = er;
    } else {
        tr = add_reg(ctxt, c);
    }

    AST_GET(e, operand1, v);
    expr_to_cmds(ctxt, c, v, tr);

    GET_LOC(op, &loc);
    set_loc(c, &loc);

    if (bc == OX_BC_global) {
        cmd_global(ctxt, c, tr);
    } else if (bc == OX_BC_owned) {
        char buf[32];
        int cid;
        OX_VS_PUSH(ctxt, s)

        snprintf(buf, sizeof(buf), "owned%d", c->owned_num ++);
        ox_not_error(ox_string_from_char_star(ctxt, s, buf));
        ox_not_error(ox_string_singleton(ctxt, s));

        cid = add_cv(ctxt, c, s);

        cmd_owned(ctxt, c, cid, tr);
    } else {
        cmd_model_sd(ctxt, c, bc, tr, er);
    }

    OX_VS_POP(ctxt, op)
}

/*Convert relative expression to commands.*/
static void
rel_expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, int bc, OX_Value *e, int er, int *rr)
{
    int pbc = -1;
    OX_VS_PUSH_3(ctxt, op, v1, v2)
    int r1 = add_reg(ctxt, c);
    int r2 = add_reg(ctxt, c);
    OX_Location loc;

    AST_GET(e, operand1, v1);
    if (ox_ast_get_type(ctxt, v1) == OX_AST_binary_expr) {
        AST_GET(v1, operator, op);
        switch (ox_ast_get_type(ctxt, op)) {
        case OX_AST_lt:
            pbc = OX_BC_lt;
            break;
        case OX_AST_gt:
            pbc = OX_BC_gt;
            break;
        case OX_AST_le:
            pbc = OX_BC_le;
            break;
        case OX_AST_ge:
            pbc = OX_BC_ge;
            break;
        default:
            break;
        }
    }

    AST_GET(e, operand1, v1);
    AST_GET(e, operand2, v2);

    if (pbc == -1) {
        expr_to_cmds(ctxt, c, v1, r1);
        expr_to_cmds(ctxt, c, v2, r2);
        cmd_model_ssd(ctxt, c, bc, r1, r2, er);
    } else {
        int l = add_label(ctxt, c);

        AST_GET(e, operator, op);
        GET_LOC(op, &loc);

        rel_expr_to_cmds(ctxt, c, pbc, v1, er, &r1);

        set_loc(c, &loc);
        cmd_jf(ctxt, c, er, l);

        expr_to_cmds(ctxt, c, v2, r2);
        
        set_loc(c, &loc);
        cmd_model_ssd(ctxt, c, bc, r1, r2, er);
        cmd_stub(ctxt, c, l);

        if (rr)
            *rr = r2;
    }

    OX_VS_POP(ctxt, op)
}

/*Initialize the question data.*/
static void
ques_data_init (OX_QuesData *qd)
{
    qd->is_src = OX_FALSE;
    qd->is_dst = OX_FALSE;
    qd->old_r = -1;
    qd->old_l = -1;
}

/*Check the AST node's question setting.*/
static void
ques_data_check (OX_Context *ctxt, OX_Compiler *c, OX_QuesData *qd, OX_Value *e, int er)
{
    qd->is_src = AST_GET_B(e, ques_src);
    qd->is_dst = AST_GET_B(e, ques_dst);

    if (qd->is_dst) {
        qd->old_r = c->ques_r;
        qd->old_l = c->ques_label;
        c->ques_r = er;
        c->ques_label = add_label(ctxt, c);
    }
}

/*Generate the question source's code.*/
static void
ques_src_cmds (OX_Context *ctxt, OX_Compiler *c, OX_QuesData *qd, OX_Location *loc, int r)
{
    if (qd->is_src) {
        int l = add_label(ctxt, c);

        assert((c->ques_label != -1) && (c->ques_r != -1));

        set_loc(c, loc);
        cmd_jnn(ctxt, c, r, l);
        cmd_load_null(ctxt, c, c->ques_r);
        cmd_jmp(ctxt, c, c->ques_label);
        cmd_stub(ctxt, c, l);
    }
}

/*Generate the question destination's end code.*/
static void
ques_dest_end (OX_Context *ctxt, OX_Compiler *c, OX_QuesData *qd)
{
    if (qd->is_dst) {
        cmd_stub(ctxt, c, c->ques_label);
        c->ques_r = qd->old_r;
        c->ques_label = qd->old_l;
    }
}

/*Convert binary expression to commands.*/
static void
binary_expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_3(ctxt, op, v, sv)
    int le;
    OX_AstType aty;
    OX_Bool is_rel = OX_FALSE;
    OX_Bool is_logic = OX_FALSE;
    OX_ByteCode bc;
    OX_Location loc;

    AST_GET(e, operator, op);
    GET_LOC(op, &loc);

    switch ((aty = ox_ast_get_type(ctxt, op))) {
    case OX_AST_exp:
        bc = OX_BC_exp;
        break;
    case OX_AST_mul:
        bc = OX_BC_mul;
        break;
    case OX_AST_div:
        bc = OX_BC_div;
        break;
    case OX_AST_mod:
        bc = OX_BC_mod;
        break;
    case OX_AST_add:
        bc = OX_BC_add;
        break;
    case OX_AST_sub:
        bc = OX_BC_sub;
        break;
    case OX_AST_match:
        bc = OX_BC_match;
        break;
    case OX_AST_shl:
        bc = OX_BC_shl;
        break;
    case OX_AST_shr:
        bc = OX_BC_shr;
        break;
    case OX_AST_ushr:
        bc = OX_BC_ushr;
        break;
    case OX_AST_lt:
        bc = OX_BC_lt;
        is_rel = OX_TRUE;
        break;
    case OX_AST_gt:
        bc = OX_BC_gt;
        is_rel = OX_TRUE;
        break;
    case OX_AST_le:
        bc = OX_BC_le;
        is_rel = OX_TRUE;
        break;
    case OX_AST_ge:
        bc = OX_BC_ge;
        is_rel = OX_TRUE;
        break;
    case OX_AST_instof:
        bc = OX_BC_instof;
        break;
    case OX_AST_eq:
        bc = OX_BC_eq;
        break;
    case OX_AST_ne:
        bc = OX_BC_ne;
        break;
    case OX_AST_bit_and:
        bc = OX_BC_and;
        break;
    case OX_AST_bit_xor:
        bc = OX_BC_xor;
        break;
    case OX_AST_bit_or:
        bc = OX_BC_or;
        break;
    case OX_AST_get:
        bc = OX_BC_get_p;
        break;
    case OX_AST_lookup:
        bc = OX_BC_lookup_p;
        break;
    case OX_AST_logic_and:
        bc = OX_BC_jf;
        is_logic = OX_TRUE;
        break;
    case OX_AST_logic_or:
        bc = OX_BC_jt;
        is_logic = OX_TRUE;
        break;
    default:
        assert(0);
    }

    if (is_rel) {
        rel_expr_to_cmds(ctxt, c, bc, e, er, NULL);
    } else if (is_logic) {
        AST_GET(e, operand1, v);
        expr_to_cmds(ctxt, c, v, er);

        le = add_label(ctxt, c);
        set_loc(c, &loc);
        cmd_model_sl(ctxt, c, bc, er, le);

        AST_GET(e, operand2, v);
        expr_to_cmds(ctxt, c, v, er);

        cmd_stub(ctxt, c, le);
    } else {
        int r1 = add_reg(ctxt, c);
        int r2 = add_reg(ctxt, c);
        OX_QuesData qd;

        ques_data_init(&qd);

        if ((aty == OX_AST_get) || (aty == OX_AST_lookup))
            ques_data_check(ctxt, c, &qd, e, er);

        AST_GET(e, operand1, v);
        expr_to_cmds(ctxt, c, v, r1);

        if ((aty == OX_AST_get) || (aty == OX_AST_lookup)) {
            if (qd.is_src) {
                ques_src_cmds(ctxt, c, &qd, &loc, r1);
            } else {
                check_not_null(ctxt, c, v, r1);
            }
        }

        AST_GET(e, operand2, v);
        expr_to_cmds(ctxt, c, v, r2);

        if ((aty == OX_AST_get) || (aty == OX_AST_lookup)) {
            if (ox_ast_get_type(ctxt, v) == OX_AST_value) {
                AST_GET(v, value, sv);

                if (ox_value_is_string(ctxt, sv) && !AST_GET_B(v, private))
                    ox_not_error(ox_string_singleton(ctxt, sv));
            }
        }

        set_loc(c, &loc);
        cmd_model_ssd(ctxt, c, bc, r1, r2, er);

        ques_dest_end(ctxt, c, &qd);
    }

    OX_VS_POP(ctxt, op)
}

/*Convert the expression to left expression.*/
static void
expr_to_assi_left (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, OX_AssiLeft *al)
{
    OX_VS_PUSH_6(ctxt, ce, te, k, base, prop, op)
    OX_ScriptDecl *sd;
    int depth;
    OX_AstType aty;

    ox_value_copy(ctxt, ce, e);
    aty = ox_ast_get_type(ctxt, ce);
    while (aty == OX_AST_parenthese) {
        AST_GET(ce, expr, te);
        ox_value_copy(ctxt, ce, te);
        aty = ox_ast_get_type(ctxt, ce);
    }

    GET_LOC(e, &al->loc);

    al->ast = ce;

    switch (aty) {
    case OX_AST_array_pattern:
        al->type = OX_ASSI_LEFT_ARRAY;
        break;
    case OX_AST_object_pattern:
        al->type = OX_ASSI_LEFT_OBJECT;
        break;
    case OX_AST_id:
        AST_GET(ce, value, k);
        sd = decl_lookup(ctxt, c, k, &depth);
        if (sd) {
            al->type = OX_ASSI_LEFT_DECL;
            al->decl = sd;
            al->depth = depth;
        } else {
            al->type = OX_ASSI_LEFT_GLOBAL;
            al->cv_id = add_cv(ctxt, c, k);
        }
        break;
    case OX_AST_unary_expr:
        al->type = OX_ASSI_LEFT_VALUE;
        al->br_id = add_reg(ctxt, c);

        AST_GET(ce, operator, op);
        assert(ox_ast_get_type(ctxt, op) == OX_AST_get_value);

        AST_GET(ce, operand1, base);

        expr_to_cmds(ctxt, c, base, al->br_id);
        break;
    case OX_AST_binary_expr:
        al->type = OX_ASSI_LEFT_PROP;
        al->br_id = add_reg(ctxt, c);
        al->pr_id = add_reg(ctxt, c);

        AST_GET(ce, operator, op);
        assert((ox_ast_get_type(ctxt, op) == OX_AST_get)
                || (ox_ast_get_type(ctxt, op) == OX_AST_lookup));

        AST_GET(ce, operand1, base);
        AST_GET(ce, operand2, prop);

        expr_to_cmds(ctxt, c, base, al->br_id);

        check_not_null(ctxt, c, base, al->br_id);

        expr_to_cmds(ctxt, c, prop, al->pr_id);
        break;
    default:
        assert(0);
    }
    
    OX_VS_POP(ctxt, te)
}

/*Get value of left expression.*/
static void
assi_left_value (OX_Context *ctxt, OX_Compiler *c, OX_AssiLeft *al, int rr)
{
    OX_Location loc;

    GET_LOC(al->ast, &loc);
    set_loc(c, &loc);

    switch (al->type) {
    case OX_ASSI_LEFT_DECL:
        if (al->depth)
            cmd_get_t_b(ctxt, c, al->depth - 1, al->decl->id, rr);
        else
            cmd_get_t(ctxt, c, al->decl->id, rr);
        break;
    case OX_ASSI_LEFT_GLOBAL:
        cmd_get_n(ctxt, c, al->cv_id, rr);
        break;
    case OX_ASSI_LEFT_PROP:
        cmd_get_p(ctxt, c, al->br_id, al->pr_id, rr);
        break;
    case OX_ASSI_LEFT_VALUE:
        cmd_get_value(ctxt, c, al->br_id, rr);
        break;
    default:
        assert(0);
    }
}

/*Array pattern assignment.*/
static void
array_pattern_assi (OX_Context *ctxt, OX_Compiler *c, OX_Value *ast, int rr)
{
    OX_VS_PUSH_4(ctxt, items, item, pat, defv)
    size_t i, len;
    int tr;
    OX_Location loc, iloc;

    GET_LOC(ast, &loc);
    set_start_loc(c, &loc, 1);

    cmd_apat_start(ctxt, c, rr);

    AST_GET(ast, items, items);
    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        OX_AstType aty;

        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        aty = ox_ast_get_type(ctxt, item);
        switch (aty) {
        case OX_AST_skip:
            GET_LOC(item, &iloc);
            set_loc(c, &iloc);
            cmd_apat_next(ctxt, c);
            break;
        case OX_AST_rest:
            tr = add_reg(ctxt, c);
            AST_GET(item, pattern, pat);
            GET_LOC(item, &iloc);
            set_loc(c, &iloc);
            cmd_apat_rest(ctxt, c, tr);
            left_ast_assi(ctxt, c, pat, tr, &iloc);
            break;
        case OX_AST_item_pattern:
            tr = add_reg(ctxt, c);
            AST_GET(item, pattern, pat);
            AST_GET(item, expr, defv);
            GET_LOC(pat, &iloc);
            set_loc(c, &iloc);
            cmd_apat_get(ctxt, c, tr);

            if (!ox_value_is_null(ctxt, defv)) {
                int l = add_label(ctxt, c);
                OX_Location vloc;

                GET_LOC(defv, &vloc);
                set_loc(c, &vloc);
                cmd_jnn(ctxt, c, tr, l);
                expr_to_cmds(ctxt, c, defv, tr);
                cmd_stub(ctxt, c, l);
            }

            left_ast_assi(ctxt, c, pat, tr, &iloc);
            break;
        default:
            assert(0);
        }
    }

    set_end_loc(c, &loc, 1);
    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, items)
}

/*Object pattern assignment.*/
static void
object_pattern_assi (OX_Context *ctxt, OX_Compiler *c, OX_Value *ast, int rr)
{
    OX_VS_PUSH_5(ctxt, items, item, pat, key, defv)
    size_t i, len;
    int tr, kr;
    OX_Location loc, iloc;

    GET_LOC(ast, &loc);
    set_start_loc(c, &loc, 1);

    cmd_opat_start(ctxt, c, rr);

    AST_GET(ast, props, items);
    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        OX_AstType aty;

        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        aty = ox_ast_get_type(ctxt, item);
        switch (aty) {
        case OX_AST_rest:
            tr = add_reg(ctxt, c);
            AST_GET(item, pattern, pat);

            GET_LOC(item, &iloc);
            set_loc(c, &iloc);
            cmd_opat_rest(ctxt, c, tr);

            left_ast_assi(ctxt, c, pat, tr, &iloc);
            break;
        case OX_AST_prop_pattern:
            tr = add_reg(ctxt, c);
            kr = add_reg(ctxt, c);

            AST_GET(item, pattern, pat);tr = add_reg(ctxt, c);
            AST_GET(item, name, key);
            AST_GET(item, expr, defv);

            prop_name_to_cmds(ctxt, c, key, kr);
            GET_LOC(key, &iloc);
            set_loc(c, &iloc);
            cmd_opat_get(ctxt, c, kr, tr);

            if (!ox_value_is_null(ctxt, defv)) {
                int l = add_label(ctxt, c);
                OX_Location vloc;

                GET_LOC(defv, &vloc);
                set_loc(c, &vloc);
                cmd_jnn(ctxt, c, tr, l);
                expr_to_cmds(ctxt, c, defv, tr);
                cmd_stub(ctxt, c, l);
            }

            left_ast_assi(ctxt, c, pat, tr, &iloc);
            break;
        default:
            assert(0);
        }
    }

    set_end_loc(c, &loc, 1);
    cmd_s_pop(ctxt, c);

    OX_VS_POP(ctxt, items)
}

/*Assign value in register to left expression.*/
static void
assi_left_assi (OX_Context *ctxt, OX_Compiler *c, OX_AssiLeft *al, int rr, OX_Location *loc)
{
    switch (al->type) {
    case OX_ASSI_LEFT_ARRAY:
        array_pattern_assi(ctxt, c, al->ast, rr);
        break;
    case OX_ASSI_LEFT_OBJECT:
        object_pattern_assi(ctxt, c, al->ast, rr);
        break;
    case OX_ASSI_LEFT_DECL:
        if (al->depth && ((al->decl->type == OX_DECL_CONST) || (al->decl->type == OX_DECL_REF))) {
            error(ctxt, c, &al->loc, OX_TEXT("constant cannot be reset"));
        } else {
            set_loc(c, loc);

            if (al->decl->type & OX_DECL_AUTO_CLOSE) {
                if (al->depth) {
                    cmd_set_t_b_ac(ctxt, c, al->depth - 1, al->decl->id, rr);
                } else {
                    cmd_set_t_ac(ctxt, c, al->decl->id, rr);
                }
            } else {
                if (al->depth)
                    cmd_set_t_b(ctxt, c, al->depth - 1, al->decl->id, rr);
                else
                    cmd_set_t(ctxt, c, al->decl->id, rr);
            }
        }
        break;
    case OX_ASSI_LEFT_GLOBAL: {
        OX_Location loc;
        OX_VS_PUSH(ctxt, n)

        AST_GET(al->ast, value, n);

        GET_LOC(al->ast, &loc);
        error(ctxt, c, &loc, OX_TEXT("\"%s\" is not defined"),
                ox_string_get_char_star(ctxt, n));

        OX_VS_POP(ctxt, n)
        break;
    }
    case OX_ASSI_LEFT_PROP:
        set_loc(c, loc);
        cmd_set_p(ctxt, c, al->br_id, al->pr_id, rr);
        break;
    case OX_ASSI_LEFT_VALUE:
        set_loc(c, loc);
        cmd_set_value(ctxt, c, al->br_id, rr);
        break;
    default:
        assert(0);
    }
}

/*Convert assignment expression to commands.*/
static void
assi_expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_4(ctxt, left, right, op, n)
    OX_AstType aty = ox_ast_get_type(ctxt, e);
    OX_Bool is_assi = OX_FALSE;
    OX_AssiLeft al;
    OX_Location loc;

    AST_GET(e, left, left);
    AST_GET(e, right, right);

    if (aty == OX_AST_rev_assi) {
        expr_to_cmds(ctxt, c, right, er);
        expr_to_assi_left(ctxt, c, left, &al);

        GET_LOC(e, &loc);
        assi_left_assi(ctxt, c, &al, er, &loc);
        is_assi = OX_TRUE;
    } else {
        OX_ByteCode bc = -1;
        OX_Bool is_logic = OX_FALSE;
        OX_Bool get_old = AST_GET_B(e, get);

        AST_GET(e, operator, op);
        aty = ox_ast_get_type(ctxt, op);

        GET_LOC(op, &loc);

        switch (aty) {
        case OX_AST_none:
            is_assi = OX_TRUE;
            break;
        case OX_AST_add:
            bc = OX_BC_add;
            break;
        case OX_AST_sub:
            bc = OX_BC_sub;
            break;
        case OX_AST_match:
            bc = OX_BC_match;
            break;
        case OX_AST_mul:
            bc = OX_BC_mul;
            break;
        case OX_AST_div:
            bc = OX_BC_div;
            break;
        case OX_AST_mod:
            bc = OX_BC_mod;
            break;
        case OX_AST_exp:
            bc = OX_BC_exp;
            break;
        case OX_AST_shl:
            bc = OX_BC_shl;
            break;
        case OX_AST_shr:
            bc = OX_BC_shr;
            break;
        case OX_AST_ushr:
            bc = OX_BC_ushr;
            break;
        case OX_AST_bit_and:
            bc = OX_BC_and;
            break;
        case OX_AST_bit_xor:
            bc = OX_BC_xor;
            break;
        case OX_AST_bit_or:
            bc = OX_BC_or;
            break;
        case OX_AST_logic_and:
            is_logic = OX_TRUE;
            bc = OX_BC_jf;
            break;
        case OX_AST_logic_or:
            is_logic = OX_TRUE;
            bc = OX_BC_jt;
            break;
        default:
            assert(0);
        }

        expr_to_assi_left(ctxt, c, left, &al);

        if (bc == -1) {
            expr_to_cmds(ctxt, c, right, er);
            assi_left_assi(ctxt, c, &al, er, &loc);
        } else if (is_logic) {
            int le = add_label(ctxt, c);
            int lr, rr;

            if (get_old) {
                lr = er;
                rr = add_reg(ctxt, c);
            } else {
                lr = add_reg(ctxt, c);
                rr = er;
            }

            assi_left_value(ctxt, c, &al, lr);
            set_loc(c, &loc);
            cmd_model_sl(ctxt, c, bc, lr, le);
            expr_to_cmds(ctxt, c, right, rr);
            assi_left_assi(ctxt, c, &al, rr, &loc);
            cmd_stub(ctxt, c, le);
        } else {
            int rr = add_reg(ctxt, c);
            int lr, ar;

            if (get_old) {
                lr = er;
                ar = add_reg(ctxt, c);
            } else {
                lr = add_reg(ctxt, c);
                ar = er;
            }

            assi_left_value(ctxt, c, &al, lr);
            expr_to_cmds(ctxt, c, right, rr);
            set_loc(c, &loc);
            cmd_model_ssd(ctxt, c, bc, lr, rr, ar);
            assi_left_assi(ctxt, c, &al, ar, &loc);
        }
    }

    /*Set the name of function/class/enumeration.*/
    if (is_assi
            && ((al.type == OX_ASSI_LEFT_DECL) || (al.type == OX_ASSI_LEFT_GLOBAL))
            && is_decl(ctxt, right)) {
        int nr = add_reg(ctxt, c);
        int cv_id;

        AST_GET(left, value, n);
        
        cv_id = add_cv(ctxt, c, n);
        set_loc(c, &loc);
        cmd_get_cv(ctxt, c, cv_id, nr);
        cmd_set_name(ctxt, c, er, nr);
    }

    OX_VS_POP(ctxt, left)
}

/*Call expression to commands.*/
static void
call_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_8(ctxt, f, op, args, items, arg, ae, base, prop)
    int tr = -1, fr;
    size_t i, len;
    OX_AstType aty;
    OX_Location loc;
    OX_QuesData qd;

    ques_data_init(&qd);
    ques_data_check(ctxt, c, &qd, e, er);

    AST_GET(e, expr, f);

    fr = add_reg(ctxt, c);

    aty = ox_ast_get_type(ctxt, f);
    if (aty == OX_AST_binary_expr) {
        AST_GET(f, operator, op);
        aty = ox_ast_get_type(ctxt, op);
        if ((aty == OX_AST_get) || (aty == OX_AST_lookup)) {
            int pr = add_reg(ctxt, c);
            OX_QuesData fqd;
            OX_Location floc;

            ques_data_init(&fqd);
            ques_data_check(ctxt, c, &fqd, f, fr);

            tr = add_reg(ctxt, c);

            AST_GET(f, operand1, base);
            AST_GET(f, operand2, prop);
            GET_LOC(f, &floc);

            expr_to_cmds(ctxt, c, base, tr);

            if (fqd.is_src)
                ques_src_cmds(ctxt, c, &fqd, &floc, tr);
            else
                check_not_null(ctxt, c, base, tr);

            expr_to_cmds(ctxt, c, prop, pr);

            GET_LOC(op, &loc);
            set_loc(c, &loc);

            if (aty == OX_AST_lookup)
                cmd_lookup_p(ctxt, c, tr, pr, fr);
            else
                cmd_get_p(ctxt, c, tr, pr, fr);

            ques_dest_end(ctxt, c, &fqd);
        }
    }

    AST_GET(e, args, args);
    GET_LOC(args, &loc);

    if (tr == -1) {
        tr = add_reg(ctxt, c);

        expr_to_cmds(ctxt, c, f, fr);
        set_start_loc(c, &loc, 1);
        cmd_load_null(ctxt, c, tr);
    } else {
        set_start_loc(c, &loc, 1);
    }

    if (qd.is_src)
        ques_src_cmds(ctxt, c, &qd, &loc, fr);
    else
        check_not_null(ctxt, c, f, fr);

    cmd_call_start(ctxt, c, fr, tr);

    AST_GET(args, items, items);
    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        int ar;
        OX_Location iloc;

        ox_not_error(ox_array_get_item(ctxt, items, i, arg));

        aty = ox_ast_get_type(ctxt, arg);
        switch (aty) {
        case OX_AST_spread:
            ar = add_reg(ctxt, c);
            AST_GET(arg, expr, ae);
            expr_to_cmds(ctxt, c, ae, ar);

            GET_LOC(arg, &iloc);
            set_loc(c, &iloc);
            cmd_arg_spread(ctxt, c, ar);
            break;
        default:
            ar = add_reg(ctxt, c);
            expr_to_cmds(ctxt, c, arg, ar);

            GET_LOC(arg, &iloc);
            set_loc(c, &iloc);
            cmd_arg(ctxt, c, ar);
            break;
        }
    }

    set_end_loc(c, &loc, 1);
    cmd_call_end(ctxt, c, er);
    ques_dest_end(ctxt, c, &qd);

    OX_VS_POP(ctxt, f)
}

/*Convert comma expression to commands.*/
static void
comma_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_VS_PUSH_2(ctxt, items, item)
    size_t i, len;

    AST_GET(e, items, items);

    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        int tr = (i == len - 1) ? er : add_reg(ctxt, c);

        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        expr_to_cmds(ctxt, c, item, tr);
    }

    OX_VS_POP(ctxt, items)
}

/*Convert the expression to commands.*/
static void
expr_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *e, int er)
{
    OX_AstType aty = ox_ast_get_type(ctxt, e);

    switch (aty) {
    case OX_AST_arg:
        arg_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_curr_object:
        curr_object_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_this:
        this_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_argv:
        argv_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_func:
        func_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_class:
        class_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_value:
        value_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_id:
        id_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_string:
        string_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_parenthese:
        parenthese_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_array:
        array_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_object:
        object_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_array_append:
        array_append_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_object_set:
        object_set_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_unary_expr:
        unary_expr_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_binary_expr:
        binary_expr_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_assi:
    case OX_AST_rev_assi:
        assi_expr_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_call:
        call_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_comma:
        comma_to_cmds(ctxt, c, e, er);
        break;
    case OX_AST_if:
        if_to_cmds(ctxt, c, e, er, OX_BLOCK_CONTENT_STMT, -1);
        break;
    case OX_AST_case:
        case_to_cmds(ctxt, c, e, er, OX_BLOCK_CONTENT_STMT, -1);
        break;
    default:
        break;
    }
}

/*Register value to commands.*/
static void
left_ast_assi (OX_Context *ctxt, OX_Compiler *c, OX_Value *left, int rr, OX_Location *loc)
{
    OX_AssiLeft al;

    expr_to_assi_left(ctxt, c, left, &al);

    assi_left_assi(ctxt, c, &al, rr, loc);
}

/*Convert if statment to commands.*/
static void
if_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s, int rr, OX_BlockContentType ctype, int or)
{
    OX_VS_PUSH_4(ctxt, items, item, expr, blk)
    size_t len, i;
    int endl = add_label(ctxt, c);
    OX_Location loc;

    AST_GET(s, items, items);
    len = ox_array_length(ctxt, items);
    for (i = 0; i < len; i ++) {
        int er, l;

        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        AST_GET(item, expr, expr);
        er = add_reg(ctxt, c);
        expr_to_cmds(ctxt, c, expr, er);

        l = add_label(ctxt, c);
        GET_LOC(item, &loc);
        set_loc(c, &loc);
        cmd_jf(ctxt, c, er, l);
        AST_GET(item, block, blk);
        block_to_cmds(ctxt, c, blk, rr, ctype, or);
        GET_LOC(blk, &loc);
        set_end_loc(c, &loc, 1);
        cmd_jmp(ctxt, c, endl);
        cmd_stub(ctxt, c, l);
    }

    AST_GET(s, else, blk);
    if (!ox_value_is_null(ctxt, blk)) {
        block_to_cmds(ctxt, c, blk, rr, ctype, or);
    } else if (rr != -1) {
        GET_LOC(s, &loc);
        set_end_loc(c, &loc, 1);
        cmd_load_null(ctxt, c, rr);
    }
    cmd_stub(ctxt, c, endl);

    OX_VS_POP(ctxt, items)
}

/*Convert case statment to commands.*/
static void
case_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s, int rr, OX_BlockContentType ctype, int or)
{
    OX_VS_PUSH_7(ctxt, expr, items, item, conds, cond, blk, f)
    OX_VECTOR_TYPE_DECL(int) labels;
    size_t i, ilen, e, elen;
    int er, cr, fr, tr;
    int lall = -1;
    int lend = -1;
    int lnull = -1;
    OX_Location loc, lloc;

    ox_vector_init(&labels);

    AST_GET(s, expr, expr);
    er = add_reg(ctxt, c);
    expr_to_cmds(ctxt, c, expr, er);

    GET_LOC(s, &lloc);
    lloc.last_line = lloc.first_line;
    lloc.last_column = lloc.first_column + 4;

    AST_GET(s, items, items);
    ilen = ox_array_length(ctxt, items);
    for (i = 0; i < ilen; i ++) {
        int lab = add_label(ctxt, c);

        ox_not_error(ox_vector_append(ctxt, &labels, lab));
        ox_not_error(ox_array_get_item(ctxt, items, i, item));

        AST_GET(item, exprs, conds);
        elen = ox_array_length(ctxt, conds);

        for (e = 0; e < elen; e ++) {
            OX_AstType aty;

            ox_not_error(ox_array_get_item(ctxt, conds, e, cond));

            aty = ox_ast_get_type(ctxt, cond);
            if (aty == OX_AST_all) {
                lall = lab;
                GET_LOC(cond, &lloc);
            } else if (aty == OX_AST_case_func) {
                fr = add_reg(ctxt, c);

                AST_GET(cond, expr, f);
                expr_to_cmds(ctxt, c, f, fr);
                GET_LOC(f, &loc);
                set_loc(c, &loc);

                tr = add_reg(ctxt, c);
                cmd_load_null(ctxt, c, tr);
                cmd_call_start(ctxt, c, fr, tr);

                cmd_arg(ctxt, c, er);

                cr = add_reg(ctxt, c);
                cmd_call_end(ctxt, c, cr);
                cmd_jt(ctxt, c, cr, lab);
            } else {
                cr = add_reg(ctxt, c);
                expr_to_cmds(ctxt, c, cond, cr);
                tr = add_reg(ctxt, c);

                GET_LOC(cond, &loc);
                set_loc(c, &loc);
                cmd_eq(ctxt, c, er, cr, tr);
                cmd_jt(ctxt, c, tr, lab);
            }
        }
    }

    lend = add_label(ctxt, c);

    set_loc(c, &lloc);

    if (lall != -1) {
        cmd_jmp(ctxt, c, lall);
    } else if (rr != -1) {
        lnull = add_label(ctxt, c);
        cmd_jmp(ctxt, c, lnull);
    } else {
        cmd_jmp(ctxt, c, lend);
    }

    for (i = 0; i < ilen; i ++) {
        int lab = ox_vector_item(&labels, i);

        ox_not_error(ox_array_get_item(ctxt, items, i, item));
        AST_GET(item, block, blk);

        cmd_stub(ctxt, c, lab);
        block_to_cmds(ctxt, c, blk, rr, ctype, or);

        GET_LOC(blk, &loc);
        set_end_loc(c, &loc, 1);
        cmd_jmp(ctxt, c, lend);
    }

    GET_LOC(s, &loc);
    set_end_loc(c, &loc, 1);

    if (lnull != -1) {
        cmd_stub(ctxt, c, lnull);
        cmd_load_null(ctxt, c, rr);
    }

    cmd_stub(ctxt, c, lend);

    ox_vector_deinit(ctxt, &labels);
    OX_VS_POP(ctxt, expr)
}

/*Convert do while statement to commands.*/
static void
do_while_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, expr, blk)
    int old_bl = c->break_label;
    int old_cl = c->continue_label;
    int er;
    OX_Location loc;

    c->break_label = add_label(ctxt, c);
    c->continue_label = add_label(ctxt, c);

    cmd_stub(ctxt, c, c->continue_label);

    AST_GET(s, block, blk);
    block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

    AST_GET(s, expr, expr);
    er = add_reg(ctxt, c);
    expr_to_cmds(ctxt, c, expr, er);

    GET_LOC(expr, &loc);
    set_loc(c, &loc);
    cmd_jt(ctxt, c, er, c->continue_label);

    cmd_stub(ctxt, c, c->break_label);

    c->break_label = old_bl;
    c->continue_label = old_cl;
    OX_VS_POP(ctxt, expr)
}

/*Convert while statement to commands.*/
static void
while_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, expr, blk)
    int old_bl = c->break_label;
    int old_cl = c->continue_label;
    int er;
    OX_Location loc;

    c->break_label = add_label(ctxt, c);
    c->continue_label = add_label(ctxt, c);

    cmd_stub(ctxt, c, c->continue_label);

    AST_GET(s, expr, expr);
    er = add_reg(ctxt, c);
    expr_to_cmds(ctxt, c, expr, er);

    GET_LOC(expr, &loc);
    set_loc(c, &loc);
    cmd_jf(ctxt, c, er, c->break_label);

    AST_GET(s, block, blk);
    block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

    GET_LOC(blk, &loc);
    set_end_loc(c, &loc, 1);
    cmd_jmp(ctxt, c, c->continue_label);

    cmd_stub(ctxt, c, c->break_label);

    c->break_label = old_bl;
    c->continue_label = old_cl;
    OX_VS_POP(ctxt, expr)
}

/*Convert for statement to commands.*/
static void
for_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, expr, blk)
    int old_bl = c->break_label;
    int old_cl = c->continue_label;
    int sl;
    int er;
    OX_Location loc;

    AST_GET(s, init, expr);
    if (!ox_value_is_null(ctxt, expr)) {
        er = add_reg(ctxt, c);
        expr_to_cmds(ctxt, c, expr, er);
    }

    c->break_label = add_label(ctxt, c);
    c->continue_label = add_label(ctxt, c);

    sl = add_label(ctxt, c);
    cmd_stub(ctxt, c, sl);

    AST_GET(s, cond, expr);
    if (!ox_value_is_null(ctxt, expr)) {
        er = add_reg(ctxt, c);
        expr_to_cmds(ctxt, c, expr, er);

        GET_LOC(expr, &loc);
        set_loc(c, &loc);
        cmd_jf(ctxt, c, er, c->break_label);
    }

    AST_GET(s, block, blk);
    block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

    cmd_stub(ctxt, c, c->continue_label);

    AST_GET(s, step, expr);
    if (!ox_value_is_null(ctxt, expr)) {
        er = add_reg(ctxt, c);
        expr_to_cmds(ctxt, c, expr, er);
    }

    GET_LOC(blk, &loc);
    set_end_loc(c, &loc, 1);

    cmd_jmp(ctxt, c, sl);
    cmd_stub(ctxt, c, c->break_label);

    c->break_label = old_bl;
    c->continue_label = old_cl;
    OX_VS_POP(ctxt, expr)
}

/*Convert for...as statement to commands.*/
static void
for_as_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH_3(ctxt, expr, blk, op)
    int old_bl = c->break_label;
    int old_cl = c->continue_label;
    int er;
    OX_Location loc;

    AST_GET(s, right, expr);
    er = add_reg(ctxt, c);
    expr_to_cmds(ctxt, c, expr, er);

    c->break_label = add_label(ctxt, c);

    GET_LOC(s, &loc);
    set_start_loc(c, &loc, 3);
    cmd_iter_start(ctxt, c, er);

    c->continue_label = add_label(ctxt, c);

    cmd_stub(ctxt, c, c->continue_label);

    er = add_reg(ctxt, c);
    GET_LOC(expr, &loc);
    set_loc(c, &loc);
    cmd_iter_step(ctxt, c, er, c->break_label);

    AST_GET(s, left, expr);
    AST_GET(s, operator, op);
    GET_LOC(op, &loc);
    left_ast_assi(ctxt, c, expr, er, &loc);

    AST_GET(s, block, blk);
    block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);
    GET_LOC(blk, &loc);
    set_end_loc(c, &loc, 1);
    cmd_jmp(ctxt, c, c->continue_label);

    c->stack_level --;

    cmd_stub(ctxt, c, c->break_label);

    c->break_label = old_bl;
    c->continue_label = old_cl;
    OX_VS_POP(ctxt, expr)
}

/*Convert sched statement to commands.*/
static void
sched_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH(ctxt, blk)
    OX_Location loc;

    GET_LOC(s, &loc);
    AST_GET(s, block, blk);

    if (ox_value_is_null(ctxt, blk)) {
        set_loc(c, &loc);
        cmd_sched(ctxt, c);
    } else {
        set_start_loc(c, &loc, 5);
        cmd_sched_start(ctxt, c);

        block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

        set_end_loc(c, &loc, 1);
        cmd_s_pop(ctxt, c);
    }

    OX_VS_POP(ctxt, blk)
}

/*Convert try statement to commands.*/
static void
try_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH_3(ctxt, blk, cb, left)
    int cl, fl;
    OX_Location loc;

    cl = add_label_l(ctxt, c, 1);
    fl = add_label_l(ctxt, c, 1);

    GET_LOC(s, &loc);
    set_start_loc(c, &loc, 3);
    cmd_try_start(ctxt, c, cl, fl);

    AST_GET(s, block, blk);
    block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

    GET_LOC(blk, &loc);
    set_end_loc(c, &loc, 1);
    cmd_try_end(ctxt, c);

    cmd_stub(ctxt, c, cl);
    AST_GET(s, catch, cb);
    if (!ox_value_is_null(ctxt, cb)) {
        int er = add_reg(ctxt, c);

        GET_LOC(cb, &loc);
        set_loc(c, &loc);
        cmd_catch(ctxt, c, er);
        AST_GET(cb, expr, left);
        left_ast_assi(ctxt, c, left, er, &loc);
        AST_GET(cb, block, blk);
        block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);

        GET_LOC(blk, &loc);
        set_end_loc(c, &loc, 1);
        cmd_catch_end(ctxt, c);
    }

    cmd_stub(ctxt, c, fl);
    AST_GET(s, finally, blk);
    if (!ox_value_is_null(ctxt, blk)) {
        block_to_cmds(ctxt, c, blk, -1, OX_BLOCK_CONTENT_STMT, -1);
    }
    cmd_finally(ctxt, c);

    OX_VS_POP(ctxt, blk)
}

/*Convert return statement to commands.*/
static void
return_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH(ctxt, expr)
    int er = add_reg(ctxt, c);
    OX_Location loc;

    GET_LOC(s, &loc);

    AST_GET(s, expr, expr);

    if (ox_value_is_null(ctxt, expr)) {
        set_loc(c, &loc);
        cmd_load_null(ctxt, c, er);
    } else {
        expr_to_cmds(ctxt, c, expr, er);
        set_start_loc(c, &loc, 6);
    }

    cmd_ret(ctxt, c, er);

    OX_VS_POP(ctxt, expr)
}

/*Convert throw statement to commands.*/
static void
throw_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_VS_PUSH(ctxt, expr)
    int er = add_reg(ctxt, c);
    OX_Location loc;

    GET_LOC(s, &loc);

    AST_GET(s, expr, expr);
    expr_to_cmds(ctxt, c, expr, er);

    set_start_loc(c, &loc, 5);
    cmd_throw(ctxt, c, er);

    OX_VS_POP(ctxt, expr)
}

/*Convert break statement to commands.*/
static void
break_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_Location loc;
    OX_CompLabel *l;

    GET_LOC(s, &loc);
    set_loc(c, &loc);

    assert(c->break_label != -1);

    l = &ox_vector_item(&c->labels, c->break_label);

    if (l->stack_level == c->stack_level) {
        cmd_jmp(ctxt, c, c->break_label);
    } else {
        cmd_deep_jmp(ctxt, c, c->stack_level - l->stack_level, c->break_label);
    }
}

/*Convert continue statement to commands.*/
static void
continue_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s)
{
    OX_Location loc;
    OX_CompLabel *l;

    GET_LOC(s, &loc);
    set_loc(c, &loc);

    assert(c->break_label != -1);

    l = &ox_vector_item(&c->labels, c->continue_label);

    if (l->stack_level == c->stack_level) {
        cmd_jmp(ctxt, c, c->continue_label);
    } else {
        cmd_deep_jmp(ctxt, c, c->stack_level - l->stack_level, c->continue_label);
    }
}

/*Convert a statement to commands.*/
static void
stmt_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *s, int rr)
{
    OX_AstType aty = ox_ast_get_type(ctxt, s);
    OX_VS_PUSH(ctxt, v)
    OX_Bool is_expr = OX_FALSE;

    switch (aty) {
    case OX_AST_if:
        if_to_cmds(ctxt, c, s, rr, OX_BLOCK_CONTENT_STMT, -1);
        is_expr = OX_TRUE;
        break;
    case OX_AST_case:
        case_to_cmds(ctxt, c, s, rr, OX_BLOCK_CONTENT_STMT, -1);
        is_expr = OX_TRUE;
        break;
    case OX_AST_do_while:
        do_while_to_cmds(ctxt, c, s);
        break;
    case OX_AST_while:
        while_to_cmds(ctxt, c, s);
        break;
    case OX_AST_for:
        for_to_cmds(ctxt, c, s);
        break;
    case OX_AST_for_as:
        for_as_to_cmds(ctxt, c, s);
        break;
    case OX_AST_sched:
        sched_to_cmds(ctxt, c, s);
        break;
    case OX_AST_try:
        try_to_cmds(ctxt, c, s);
        break;
    case OX_AST_return:
        return_to_cmds(ctxt, c, s);
        break;
    case OX_AST_throw:
        throw_to_cmds(ctxt, c, s);
        break;
    case OX_AST_break:
        break_to_cmds(ctxt, c, s);
        break;
    case OX_AST_continue:
        continue_to_cmds(ctxt, c, s);
        break;
    default: {
        int er = rr;

        if (er == -1)
            er = add_reg(ctxt, c);

        expr_to_cmds(ctxt, c, s, er);
        is_expr = OX_TRUE;
        break;
    }
    }

    if ((rr != -1) && !is_expr) {
        cmd_load_null(ctxt, c, rr);
    }

    OX_VS_POP(ctxt, v)
}

/*Convert the statements to comands.*/
static void
stmts_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *stmts, int rr)
{
    OX_VS_PUSH(ctxt, stmt)
    size_t len;
    size_t i;

    if (ox_value_is_null(ctxt, stmts))
        goto end;

    len = ox_array_length(ctxt, stmts);
    if (len) {
        for (i = 0; i < len; i ++) {
            int sr = -1;

            ox_not_error(ox_array_get_item(ctxt, stmts, i, stmt));

            if (i == len - 1)
                sr = rr;

            stmt_to_cmds(ctxt, c, stmt, sr);
        }
    } else if (rr != -1) {
        cmd_load_null(ctxt, c, rr);
    }
end:
    OX_VS_POP(ctxt, stmt)
}

/*Convert parameters to commands.*/
static void
params_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *params)
{
    OX_VS_PUSH_4(ctxt, p, pat, defv, items)
    OX_Location loc, ploc;

    if (!ox_value_is_null(ctxt, params))
        AST_GET(params, items, items);

    if (!ox_value_is_null(ctxt, items)) {
        size_t i, len;

        len = ox_array_length(ctxt, items);
        if (len) {
            GET_LOC(params, &loc);
            set_start_loc(c, &loc, 1);

            cmd_p_start(ctxt, c);

            for (i = 0; i < len; i ++) {
                OX_AstType aty;
                int pr;

                ox_not_error(ox_array_get_item(ctxt, items, i, p));

                aty = ox_ast_get_type(ctxt, p);
                switch (aty) {
                case OX_AST_rest:
                    pr = add_reg(ctxt, c);
                    AST_GET(p, pattern, pat);

                    GET_LOC(p, &ploc);
                    set_loc(c, &ploc);
                    cmd_p_rest(ctxt, c, pr);
                    left_ast_assi(ctxt, c, pat, pr, &loc);
                    break;
                default: {
                    pr = add_reg(ctxt, c);
                    AST_GET(p, expr, defv);

                    GET_LOC(p, &ploc);
                    set_loc(c, &ploc);
                    cmd_p_get(ctxt, c, pr);

                    if (!ox_value_is_null(ctxt, defv)) {
                        OX_Location vloc;
                        int l = add_label(ctxt, c);

                        GET_LOC(defv, &vloc);
                        set_loc(c, &vloc);
                        cmd_jnn(ctxt, c, pr, l);

                        expr_to_cmds(ctxt, c, defv, pr);
                        cmd_stub(ctxt, c, l);
                    }

                    switch (aty) {
                    case OX_AST_array_pattern:
                    case OX_AST_object_pattern:
                    case OX_AST_id:
                        left_ast_assi(ctxt, c, p, pr, &ploc);
                        break;
                    default:
                        assert(0);
                    }
                }
                }

            }

            set_end_loc(c, &loc, 1);
            cmd_s_pop(ctxt, c);
        }
    }

    OX_VS_POP(ctxt, p)
}

/*Convert the block to commands.*/
static void
block_to_cmds (OX_Context *ctxt, OX_Compiler *c, OX_Value *blk, int rr, OX_BlockContentType ctype, int or)
{
    OX_VS_PUSH(ctxt, items)

    if (ox_value_is_null(ctxt, blk))
        goto end;

    switch (ctype) {
    case OX_BLOCK_CONTENT_STMT:
        AST_GET(blk, items, items);
        stmts_to_cmds(ctxt, c, items, rr);
        break;
    case OX_BLOCK_CONTENT_PROP:
        AST_GET(blk, props, items);
        object_props_to_cmds(ctxt, c, items, or);
        break;
    case OX_BLOCK_CONTENT_ITEM:
        AST_GET(blk, items, items);
        array_items_to_cmds(ctxt, c, items);
        break;
    }
end:
    OX_VS_POP(ctxt, items)
}

/*Get the bytecode's length.*/
static int
bytecode_len (OX_ByteCode bc)
{
    OX_BcModel m;

    if ((bc == OX_BC_stub) || (bc == OX_BC_nop))
        return 0;

    m = bytecode_models[bc];
    return bytecode_len_table[m];
}

/*Initialize the functions.*/
static void
init_func (OX_Context *ctxt, OX_Compiler *c)
{
    OX_VS_PUSH_2(ctxt, bot, id)

    /*Initialize the script function.*/
    c->sf->script = c->s;
    ox_size_hash_init(&c->sf->decl_hash);
    ox_list_init(&c->sf->decl_list);
    c->sf->frame_num = c->bot_frame_num;
    c->sf->reg_num = 0;
    c->sf->flags = 0;

    /*Get this argument flag.*/
    if (AST_GET_B(c->f, this))
        c->sf->flags |= OX_SCRIPT_FUNC_FL_THIS;

    /*Get the bottom frames' number.*/
    AST_GET(c->f, outer, bot);
    while (!ox_value_is_null(ctxt, bot)) {
        AST_GET(bot, id, id);
        if (!ox_value_is_null(ctxt, id))
            c->sf->frame_num ++;

        AST_GET(bot, outer, bot);
    }

    OX_VS_POP(ctxt, bot)
}

/*Initialize the functions.*/
static void
func_add_decls (OX_Context *ctxt, OX_Compiler *c)
{
    /*Generate the declarations.*/
    add_decls(ctxt, c);
}

/*Compile the function.*/
static OX_Result
compile_func (OX_Context *ctxt, OX_Compiler *c, int rr)
{
    OX_VS_PUSH_2(ctxt, blk, params)
    uint8_t *bc;
    size_t i;
    size_t bc_len;
    OX_Bool loop;
    OX_Result r;

    /*Index overflow check.*/
    if (c->sf->decl_hash.e_num > 0xffff) {
        r = ox_throw_range_error(ctxt, OX_TEXT("too many declarations in the function"));
        goto end;
    }

    if (c->sf->frame_num > 0xff) {
        r = ox_throw_range_error(ctxt, OX_TEXT("function stack is too deep"));
        goto end;
    }

    /*Clear the labels and commands buffer.*/
    c->labels.len = 0;
    c->cmds.len = 0;
    c->break_label = -1;
    c->continue_label = -1;
    c->ques_label = -1;
    c->ques_r = -1;
    c->stack_level = 0;

    /*Compile AST to commands.*/
    AST_GET(c->f, params, params);
    params_to_cmds(ctxt, c, params);

    AST_GET(c->f, block, blk);
    block_to_cmds(ctxt, c, blk, rr, OX_BLOCK_CONTENT_STMT, -1);

    if (rr != -1) {
        OX_Location loc;

        GET_LOC(c->f, &loc);
        set_loc(c, &loc);
        cmd_ret(ctxt, c, rr);
    }

    /*Calculate the bytecode buffer's length.*/
    c->sf->bc_start = c->bc.len;
    c->sf->loc_start = c->ltab.len;

    do {
        loop = OX_FALSE;
        bc_len = 0;

        for (i = 0; i < c->cmds.len; i ++) {
            OX_Command *cmd = &ox_vector_item(&c->cmds, i);
            OX_CompLabel *l;

            switch (cmd->bc) {
            case OX_BC_stub:
                /*Store the stub's offset.*/
                l = &ox_vector_item(&c->labels, cmd->l.l0);

                if (l->off != bc_len) {
                    l->off = bc_len;
                    loop = OX_TRUE;
                }
                break;
            case OX_BC_jmp:
                /*Remove unused jmp.*/
                l = &ox_vector_item(&c->labels, cmd->l.l0);

                if (l->off == bc_len + bytecode_len(cmd->bc)) {
                    cmd->bc = OX_BC_nop;
                    loop = OX_TRUE;
                }
                break;
            case OX_BC_jf:
            case OX_BC_jt:
            case OX_BC_jnn:
                /*Remove unused jt/jf/jn.*/
                l = &ox_vector_item(&c->labels, cmd->sl.l1);

                if (l->off == bc_len + bytecode_len(cmd->bc)) {
                    cmd->bc = OX_BC_nop;
                    loop = OX_TRUE;
                }
                break;
            default:
                break;
            }

            bc_len += bytecode_len(cmd->bc);
        }
    } while (loop);

    if (bc_len > 0xffff) {
        r = ox_throw_range_error(ctxt, OX_TEXT("byte code buffer is too big"));
        goto end;
    }

    c->sf->bc_len = bc_len;

    /*Allocate registers.*/
    for (i = 0; i < c->cmds.len; i ++) {
        OX_Command *cmd = &ox_vector_item(&c->cmds, i);

        bytecode_mark_regs(ctxt, c, i, cmd);
    }

    memset(c->reg_lifetimes, 0xff, sizeof(c->reg_lifetimes));
    for (i = 0; i < c->cmds.len; i ++) {
        OX_Command *cmd = &ox_vector_item(&c->cmds, i);

        if ((r = bytecode_alloc_regs(ctxt, c, i, cmd)) == OX_ERR)
            goto end;
    }

    /*Store the bytecodes.*/
    if (c->sf->bc_len) {
        int ip = 0;
        int top_ip = 0;
        OX_CompLabel *l;

        ox_not_error(ox_vector_expand(ctxt, &c->bc, c->sf->bc_start + c->sf->bc_len));

        bc = c->bc.items + c->sf->bc_start;
        for (i = 0; i < c->cmds.len; i ++) {
            OX_Command *cmd = &ox_vector_item(&c->cmds, i);
            int bc_len = bytecode_len(cmd->bc);
            OX_ScriptLoc sloc;

            if (bc_len) {
                /*Store location information.*/
                OX_Bool store_loc = OX_TRUE;

                if (c->ltab.len != c->sf->loc_start) {
                    OX_ScriptLoc *oloc = &ox_vector_item(&c->ltab, c->ltab.len - 1);

                    if (oloc->line == cmd->g.loc.first_line)
                        store_loc = OX_FALSE;
                }

                if (store_loc) {
                    sloc.line = cmd->g.loc.first_line;
                    sloc.ip = ip;

                    ox_not_error(ox_vector_append(ctxt, &c->ltab, sloc));
                }

                switch (cmd->bc) {
                case OX_BC_iter_step:
                    /*Store the top block's offset.*/
                    if (top_ip == 0) {
                        l = &ox_vector_item(&c->labels, cmd->dl.l1);
                        top_ip = l->off;
                    }
                    break;
                case OX_BC_try_start:
                    /*Store the top block's offset.*/
                    if (top_ip == 0) {
                        l = &ox_vector_item(&c->labels, cmd->ll.l1);
                    }
                    break;
                case OX_BC_call_end:
                    /*Tail recursion check.*/
                    if (top_ip == 0) {
                        OX_Bool is_tail = OX_FALSE;
                        OX_Command *ncmd = NULL;
                        int j;

                        /*Get the next real command.*/
                        for (j = i + 1; j < c->cmds.len; j ++) {
                            ncmd = &ox_vector_item(&c->cmds, j);

                            if (bytecode_len(ncmd->bc)
                                    && (bytecode_models[ncmd->bc] != OX_BC_MODEL_noarg))
                                break;
                        }

                        if (j >= c->cmds.len) {
                            is_tail = OX_TRUE;
                        } else if (ncmd->bc == OX_BC_ret) {
                            if (ncmd->s.s0 == cmd->d.d0)
                                is_tail = OX_TRUE;
                        } else if (ncmd->bc == OX_BC_load_null) {
                            if (j + 1 < c->cmds.len) {
                                OX_Command *nncmd = &ox_vector_item(&c->cmds, j + 1);

                                if ((nncmd->bc == OX_BC_ret) && (nncmd->s.s0 == ncmd->d.d0))
                                    is_tail = OX_TRUE;
                            }
                        }

                        if (is_tail)
                            cmd->bc = OX_BC_call_end_tail;
                    }
                    break;
                default:
                    break;
                }

                /*Store byte code.*/
                bytecode_store(ctxt, c, cmd, bc);
                bc += bc_len;
                ip += bc_len;
            } else {
                if (top_ip == ip)
                    top_ip = 0;
            }
        }
    }

    c->sf->loc_len = c->ltab.len - c->sf->loc_start;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, blk)
    return r;
}

/*Store the private property.*/
static void
script_store_pp (OX_Context *ctxt, OX_BcScript *s, int id, OX_Value *v)
{
    OX_CharBuffer cb;
    const char *n = ox_string_get_char_star(ctxt, v);

    ox_char_buffer_init(&cb);

    ox_not_error(ox_char_buffer_print(ctxt, &cb, "#%s@%p", n, s));
    ox_not_error(ox_char_buffer_get_string(ctxt, &cb, &s->pps[id]));
    ox_not_error(ox_string_singleton(ctxt, &s->pps[id]));

    ox_char_buffer_deinit(ctxt, &cb);
}

/**
 * Compile the abstract syntax tree to script.
 * @param ctxt The current running context.
 * @param input The input.
 * @param ast The abstract syntax tree.
 * @param[out] sv The result script.
 * @param flags The compile flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_compile (OX_Context *ctxt, OX_Value *input, OX_Value *ast, OX_Value *sv, int flags)
{
    OX_VS_PUSH_10(ctxt, funcs, func, refs, iter, ref, items, item, v, td, path)
    OX_Input *ip;
    OX_Compiler c;
    size_t i, len;
    OX_CompValue *cv, *pv;
    OX_BcScript *s;
    OX_Result  r;

    assert(ctxt && input && ast && sv);
    assert(ox_value_is_input(ctxt, input));

    ip = ox_value_get_gco(ctxt, input);

    compiler_init(ctxt, &c, ip, flags);

    if (!(s = ox_bc_script_new(ctxt, sv, ip,
            (flags & OX_COMPILE_FL_REGISTER) ? OX_TRUE : OX_FALSE))) {
        r = OX_ERR;
        goto end;
    }

    c.s = s;

    /*Allocate the functions.*/
    AST_GET(ast, funcs, funcs);
    len = ox_array_length(ctxt, funcs);
    if (len > 0xffff) {
        r = ox_throw_range_error(ctxt, OX_TEXT("too many functions defined"));
        goto end;
    }
    s->sfunc_num = len;

    ox_not_null(OX_NEW_N(ctxt, s->sfuncs, s->sfunc_num));

    /*Initialize the functions.*/
    for (i = 0; i < s->sfunc_num; i ++) {
        ox_not_error(ox_array_get_item(ctxt, funcs, i, func));

        c.sf = &s->sfuncs[i];
        c.f = func;

        init_func(ctxt, &c);
    }

    /*Add declarations.*/
    for (i = 0; i < s->sfunc_num; i ++) {
        ox_not_error(ox_array_get_item(ctxt, funcs, i, func));

        c.sf = &s->sfuncs[i];
        c.f = func;

        func_add_decls(ctxt, &c);
    }

    /*Store the functions.*/
    for (i = 0; i < s->sfunc_num; i ++) {
        int rr = -1;

        ox_not_error(ox_array_get_item(ctxt, funcs, i, func));

        c.sf = &s->sfuncs[i];
        c.f = func;
        c.regs.len = 0;

        if ((flags & OX_COMPILE_FL_EXPR) && (i == 0))
            rr = add_reg(ctxt, &c);

        if ((r = compile_func(ctxt, &c, rr)) == OX_ERR)
            goto end;
    }

    /*Store byte code array to script.*/
    s->bc_len = c.bc.len;
    if (s->bc_len) {
        ox_not_null(OX_NEW_N(ctxt, s->bc, s->bc_len));
        memcpy(s->bc, c.bc.items, c.bc.len);
    }

    /*Store location information to script.*/
    s->loc_tab_len = c.ltab.len;
    if (s->loc_tab_len) {
        ox_not_null(OX_NEW_N(ctxt, s->loc_tab, s->loc_tab_len));
        memcpy(s->loc_tab, c.ltab.items, s->loc_tab_len * sizeof(OX_ScriptLoc));
    }

    /*Store the constant values.*/
    len = c.cv_hash.e_num;
    if (len) {
        if (len > 0xffff) {
            r = ox_throw_range_error(ctxt, OX_TEXT("too many constant values used"));
            goto end;
        }

        s->cv_num = len;
        ox_not_null(OX_NEW_N(ctxt, s->cvs, s->cv_num));
        ox_hash_foreach_c(&c.cv_hash, i, cv, OX_CompValue, he) {
            ox_value_copy(ctxt, &s->cvs[cv->id], &cv->v);
        }
    }

    /*Store the private property names.*/
    len = c.pp_hash.e_num;
    if (len) {
        if (len > 0xffff) {
            r = ox_throw_range_error(ctxt, OX_TEXT("too many private properties used"));
            goto end;
        }

        s->pp_num = len;
        ox_not_null(OX_NEW_N(ctxt, s->pps, s->pp_num));
        ox_values_set_null(ctxt, s->pps, s->pp_num);
        ox_hash_foreach_c(&c.pp_hash, i, pv, OX_CompValue, he) {
            script_store_pp(ctxt, s, pv->id, &pv->v);
        }
    }

    /*Store the localized text strings.*/
    len = c.lt_hash.e_num;
    if (len) {
        if (len > 0xffff) {
            r = ox_throw_range_error(ctxt, OX_TEXT("too many localized text used"));
            goto end;
        }

        s->t_num = len;
        ox_not_null(OX_NEW_N(ctxt, s->ts, s->t_num));
        ox_not_null(OX_NEW_N(ctxt, s->lts, s->t_num));
        ox_values_set_null(ctxt, s->lts, s->t_num);
        ox_hash_foreach_c(&c.lt_hash, i, cv, OX_CompValue, he) {
            ox_value_copy(ctxt, &s->ts[cv->id], &cv->v);
        }
    }

    /*Store the localized text string templates.*/
    len = c.ltt_hash.e_num;
    if (len) {
        if (len > 0xffff) {
            r = ox_throw_range_error(ctxt, OX_TEXT("too many localized text templates used"));
            goto end;
        }

        s->tt_num = len;
        ox_not_null(OX_NEW_N(ctxt, s->tts, s->tt_num));
        ox_not_null(OX_NEW_N(ctxt, s->ltts, s->tt_num));
        ox_values_set_null(ctxt, s->ltts, s->tt_num);
        ox_hash_foreach_c(&c.ltt_hash, i, cv, OX_CompValue, he) {
            ox_value_copy(ctxt, &s->tts[cv->id], &cv->v);
        }
    }

    /*Store reference entries.*/
    AST_GET(ast, refs, refs);
    if (!ox_value_is_null(ctxt, refs)) {
        size_t ref_num = 0;
        size_t item_num = 0;

        ox_not_error(ox_object_iter_new(ctxt, iter, refs, OX_OBJECT_ITER_VALUE));
        while (!ox_iterator_end(ctxt, iter)) {
            ref_num ++;
            ox_not_error(ox_iterator_value(ctxt, iter, ref));
            AST_GET(ref, items, items);
            if (!ox_value_is_null(ctxt, items))
                item_num += ox_array_length(ctxt, items);
            ox_not_error(ox_iterator_next(ctxt, iter));
        }

        if (ref_num) {
            OX_ScriptRef *sr;
            OX_ScriptRefItem *si;

            ox_not_error(ox_script_alloc_refs(ctxt, &s->script, ref_num, item_num));

            sr = s->script.refs;
            si = s->script.ref_items;

            ox_not_error(ox_object_iter_new(ctxt, iter, refs, OX_OBJECT_ITER_VALUE));
            while (!ox_iterator_end(ctxt, iter)) {
                ox_not_error(ox_iterator_value(ctxt, iter, ref));

                AST_GET(ref, file, v);
                AST_GET(v, value, &sr->filename);

                ox_value_set_null(ctxt, &sr->script);

                sr->item_start = si - s->script.ref_items;
                sr->item_num = 0;

                AST_GET(ref, items, items);
                if (!ox_value_is_null(ctxt, items)) {
                    size_t len = ox_array_length(ctxt, items);
                    for (i = 0; i < len; i ++) {
                        ox_not_error(ox_array_get_item(ctxt, items, i, item));

                        AST_GET(item, orig, v);
                        if (ox_ast_get_type(ctxt, v) == OX_AST_all)
                            ox_value_copy(ctxt, &si->orig, OX_STRING(ctxt, star));
                        else
                            AST_GET(v, value, &si->orig);

                        AST_GET(item, name, v);
                        if (ox_value_is_null(ctxt, v)) {
                            OX_Bool pub = AST_GET_B(item, public);

                            ox_value_set_bool(ctxt, &si->name, pub);
                        } else {
                            AST_GET(v, value, &si->name);
                        }

                        si ++;
                        sr->item_num ++;
                    }
                }
                ox_not_error(ox_iterator_next(ctxt, iter));
                sr ++;
            }
        }
    }

    /*Store text domain.*/
    AST_GET(ast, textdomain, v);
    if (!ox_value_is_null(ctxt, v)) {
        const char *pcstr = NULL;

        AST_GET(v, value, td);
        AST_GET(v, path, path);

        if (!ox_value_is_null(ctxt, path))
            pcstr = ox_string_get_char_star(ctxt, path);

        ox_script_set_text_domain(ctxt, sv, td, pcstr);
    }

    if (c.flags & OX_COMPILE_FL_ERROR) {
        r = ox_throw_syntax_error(ctxt,
                OX_TEXT("error occurred while compiling file \"%s\""),
                ip->name);
        goto end;
    }

    r = OX_OK;
end:
    compiler_deinit(ctxt, &c);
    OX_VS_POP(ctxt, funcs)
    return r;
}

/**
 * Decompile the script to readable instructions.
 * @param ctxt The current running context.
 * @param script The script value.
 * @param fp The decompile output file.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_decompile (OX_Context *ctxt, OX_Value *script, FILE *fp)
{
    OX_BcScript *s;
    size_t i;
    uint8_t *bc, *bc_end;
    size_t off;

    assert(ctxt && script && fp);
    assert(ox_value_is_gco(ctxt, script, OX_GCO_BC_SCRIPT));

    s = ox_value_get_gco(ctxt, script);

    /*Reference.*/
    if (s->script.ref_num) {
        fprintf(fp, "reference:\n");

        for (i = 0; i < s->script.ref_num; i ++) {
            OX_ScriptRef *ref = &s->script.refs[i];
            size_t j;

            fprintf(fp, "  \"%s\":\n", ox_string_get_char_star(ctxt, &ref->filename));

            for (j = 0; j < ref->item_num; j ++) {
                OX_ScriptRefItem *item = &s->script.ref_items[ref->item_start + j];

                fprintf(fp, "    %s", ox_string_get_char_star(ctxt, &item->orig));

                if (ox_value_is_string(ctxt, &item->name))
                    fprintf(fp, " as %s", ox_string_get_char_star(ctxt, &item->name));

                fprintf(fp, "\n");
            }
        }
    }

    /*Functions.*/
    for (i = 0; i < s->sfunc_num; i ++) {
        OX_ScriptFunc *sf = &s->sfuncs[i];

        fprintf(fp, "function %"PRIdPTR":\n", i);
        fprintf(fp, "  bottom frames: %d\n", sf->frame_num);
        fprintf(fp, "  registers: %d\n", sf->reg_num);

        /*Local declaration.*/
        if (sf->decl_hash.e_num) {
            OX_ScriptDecl *decl;
            size_t j;
            OX_Bool first = OX_TRUE;

            fprintf(fp, "  declaration:");

            ox_hash_foreach_c(&sf->decl_hash, j, decl, OX_ScriptDecl, he) {
                if (first) {
                    fprintf(fp, " ");
                    first = OX_FALSE;
                } else {
                    fprintf(fp, ", ");
                }

                fprintf(fp, "%d: %s", decl->id, ((OX_String*)decl->he.key)->chars);
            }

            fprintf(fp, "\n");
        }

        /*Bytecode.*/
        bc = s->bc + sf->bc_start;
        bc_end = bc + sf->bc_len;
        off = 0;

        while (bc < bc_end) {
            int len;
            int line = ox_script_func_lookup_line(ctxt, sf, off);

            fprintf(fp, "  %05"PRIdPTR"|%05d: ", off, line);
            
            len = bytecode_decompile(ctxt, s, bc, fp);

            fprintf(fp, "\n");

            bc += len;
            off += len;
        }
    }

    return OX_OK;
}

/** Function running status.*/
typedef struct {
    OX_Function   *f;     /**< The function value.*/
    OX_Value      *thiz;  /**< This argument.*/
    OX_Value      *args;  /**< Arguments.*/
    size_t         argc;  /**< Arguments' count.*/
    OX_Value      *rv;    /**< Return value.*/
    OX_BcScript   *s;     /**< The byte code script.*/
    OX_ScriptFunc *sf;    /**< The script function.*/
    OX_Frame      *frame; /**< The current frame.*/
    OX_Value      *regs;  /**< Registers.*/
    OX_Value      *argv;  /**< Arguments vector.*/
    size_t         sp;    /**< Stack pointer.*/
    size_t         jmp_ip;/**< Deep jump's instruction pointer.*/
    size_t         jmp_sp;/**< Deep jump's stack pointer.*/
    OX_Fiber      *fiber; /**< The current fiber.*/
} OX_RunStatus;

/*Auto close the variable.*/
static OX_Result
auto_close (OX_Context *ctxt, OX_Value *v)
{
    OX_Result r;

    if (!ox_value_is_null(ctxt, v)) {
        OX_VS_PUSH(ctxt, rv)

        r = ox_try_call_method(ctxt, v, OX_STRING(ctxt, _close), NULL, 0, rv);

        OX_VS_POP(ctxt, rv)

        if (r == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Auto close all the variables.*/
static OX_Result
auto_close_all (OX_Context *ctxt, OX_ScriptFunc *sf, OX_Frame *f)
{
    OX_ScriptDecl *decl;
    OX_Result r;

    ox_list_foreach_c(&sf->decl_list, decl, OX_ScriptDecl, ln) {
        OX_Value *v;

        if (!(decl->type & OX_DECL_AUTO_CLOSE))
            break;

        v = &f->v[decl->id];

        if ((r = auto_close(ctxt, v)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Return from the function.*/
#define OX_RETURN    2
/*Jump to a label.*/
#define OX_JUMP      3
/*Deep jump.*/
#define OX_DEEP_JUMP 4
/*Yield.*/
#define OX_YIELD     5

/*Store run status to record.*/
static void
run_status_to_rec (OX_Context *ctxt, OX_RunStatus *rs, OX_RunStatusRec *rsr)
{
    rsr->frame = ctxt->frames;
    rsr->args = rs->args;
    rsr->argc = rs->argc;
    rsr->rv = rs->rv;
    rsr->vp = OX_VALUE_PTR2IDX(rs->regs);
    rsr->sp = rs->sp;
}

/*Load run status from record.*/
static void
run_status_from_rec (OX_Context *ctxt, OX_RunStatus *rs, OX_RunStatusRec *rsr)
{
    rs->frame = rsr->frame;
    rs->f = ox_value_get_gco(ctxt, &rsr->frame->func);
    rs->sf = rs->f->sfunc;
    rs->s = rs->sf->script;
    rs->thiz = ox_value_get_gco(ctxt, &rsr->frame->thiz);
    rs->args = rsr->args;
    rs->argc = rsr->argc;
    rs->rv = rsr->rv;
    rs->sp = rsr->sp;
    rs->regs = OX_VALUE_IDX2PTR(rsr->vp);
    rs->argv = ox_values_item(ctxt, rs->regs, rs->sf->reg_num);
}

#include "ox_run.h"

/**
 * Call the function.
 * @param ctxt The current running context.
 * @param f The function value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Count of arguments.
 * @param[out] rv Return value.
 * @param fiber The current fiber.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_function_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz,
        OX_Value *args, size_t argc, OX_Value *rv, OX_Fiber *fiber)
{
    OX_Result r = OX_OK;
    OX_StatusBuffer *old_s_stack = NULL;
    OX_ValueBuffer *old_v_stack = NULL;
    size_t sp;
    OX_RunStatus rs;
    
    if (fiber) {
        assert((fiber->state != OX_FIBER_STATE_END)
                && (fiber->state != OX_FIBER_STATE_ERROR));

        old_s_stack = ctxt->s_stack;
        old_v_stack = ctxt->base.v_stack;

        ctxt->s_stack = &fiber->s_stack;
        ctxt->base.v_stack = &fiber->v_stack;

        fiber->state = OX_FIBER_STATE_RUN;
        ctxt->main_frames = ctxt->frames;
        ctxt->frames = fiber->rsr.frame;
        sp = 0;

        run_status_from_rec(ctxt, &rs, &fiber->rsr);
    } else {
        rs.f = ox_value_get_gco(ctxt, f);
        rs.sf = rs.f->sfunc;
        rs.s = rs.sf->script;
        rs.thiz = thiz;
        rs.args = args;
        rs.argc = argc;
        rs.rv = rv;
        rs.frame = ctxt->frames;
        rs.frame->ip = 0;
        rs.regs = ox_value_stack_push_n(ctxt, rs.sf->reg_num + 1);
        rs.argv = ox_values_item(ctxt, rs.regs, rs.sf->reg_num);

        rs.sp = ctxt->s_stack->len;
        sp = rs.sp;
        ox_value_copy(ctxt, &rs.frame->thiz, thiz);
    }

    rs.fiber = fiber;

run:
    while (rs.frame->ip < rs.sf->bc_len) {
        uint8_t *bc = rs.s->bc + rs.sf->bc_start + rs.frame->ip;

#if 0
        fprintf(stderr, "  %05d: ", rs.frame->ip);
            
        bytecode_decompile(ctxt, rs.s, bc, stderr);

        fprintf(stderr, "\n");
#endif

        switch (bc[0]) {
#include "ox_bytecode_run.h"
        default:
            assert(0);
        }
    }

    ox_value_set_null(ctxt, rs.rv);
    r = OX_RETURN;
pop:
    if (r != OX_YIELD) {
        /*Solve the stack.*/
        while (1) {
            OX_Stack *se;

            if (r == OX_DEEP_JUMP) {
                if (rs.jmp_sp == ctxt->s_stack->len) {
                    rs.frame->ip = rs.jmp_ip;
                    goto run;
                }
            }

            if (ctxt->s_stack->len == sp)
                break;

            se = &ox_vector_item(ctxt->s_stack, ctxt->s_stack->len - 1);

            if (se->type == OX_STACK_TRY) {
                int ip;

                if (se->s.t.state == OX_TRY_STATE_TRY) {
                    if (r == OX_RETURN) {
                        ip = se->s.t.finally_label;
                        se->s.t.state = OX_TRY_STATE_FINALLY;
                    } else if (r == OX_DEEP_JUMP) {
                        ip = se->s.t.finally_label;
                        se->s.t.state = OX_TRY_STATE_FINALLY;
                        se->s.t.jmp_ip = rs.jmp_ip;
                        se->s.t.jmp_sp = rs.jmp_sp;
                    } else {
                        ip = se->s.t.catch_label;
                        se->s.t.state = OX_TRY_STATE_CATCH;
                    }

                    se->s.t.r = r;

                    if (ip < rs.sf->bc_len - 1) {
                        rs.frame->ip = ip;
                        goto run;
                    }
                } else if (se->s.t.state == OX_TRY_STATE_CATCH) {
                    if (r == OX_DEEP_JUMP) {
                        se->s.t.jmp_ip = rs.jmp_ip;
                        se->s.t.jmp_sp = rs.jmp_sp;
                    }
                    
                    ip = se->s.t.finally_label;
                    se->s.t.state = OX_TRY_STATE_FINALLY;
                    se->s.t.r = r;

                    if (ip < rs.sf->bc_len - 1) {
                        rs.frame->ip = ip;
                        goto run;
                    }
                }
            } else if (se->type == OX_STACK_RETURN) {
                int cr;

                cr = auto_close_all(ctxt, rs.sf, rs.frame);
                if (cr == OX_ERR)
                    r = OX_ERR;

                if (r == OX_RETURN)
                    ox_value_copy(ctxt, se->s.r.rv, rs.rv);

                run_status_from_rec(ctxt, &rs, &se->s.r);
                if (r != OX_ERR)
                    rs.frame->ip += 2;

                ox_stack_pop(ctxt);

                if (r == OX_RETURN) {
                    ox_stack_pop(ctxt);
                    goto run;
                }
            }

            ox_stack_pop(ctxt);
        }

        if (r == OX_RETURN)
            r = OX_OK;

        r |= auto_close_all(ctxt, rs.sf, rs.frame);

        ox_value_stack_pop(ctxt, rs.regs);
    }

    if (fiber) {
        if (r == OX_ERR) {
            fiber->state = OX_FIBER_STATE_ERROR;
            ox_value_copy(ctxt, &fiber->rv, &ctxt->error);
        } else if (r == OX_YIELD) {
            run_status_to_rec(ctxt, &rs, &fiber->rsr);
            r = OX_OK;
        } else {
            fiber->state = OX_FIBER_STATE_END;
        }

        ctxt->frames = ctxt->main_frames;
        ctxt->main_frames = NULL;
        ctxt->s_stack = old_s_stack;
        ctxt->base.v_stack = old_v_stack;
    }

    return r;
}
