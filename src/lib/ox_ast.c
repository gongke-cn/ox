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
 * Abstract syntax tree.
 */

#define OX_LOG_TAG "ox_ast"

#include "ox_internal.h"
#include "ox_ast_table.h"

/** Process status.*/
typedef enum {
    STATUS_SCRIPT,
    STATUS_FUNCS,
    STATUS_FUNC,
    STATUS_OTHER
} OX_AstToStrStatus;

/** AST to string context.*/
typedef struct {
    OX_CharBuffer     cb;     /**< The output character buffer.*/
    int               level;  /**< The current indent level.*/
    OX_AstToStrStatus status; /**< Status.*/
} OX_AstToStr;

/**
 * Create a new AST node.
 * @param ctxt The current running context.
 * @param[out] ast Return the new AST node.
 * @param type The node's type.
 */
void
ox_ast_new (OX_Context *ctxt, OX_Value *ast, OX_AstType type)
{
    OX_VS_PUSH(ctxt, t)

    ox_not_error(ox_object_new(ctxt, ast, OX_OBJECT(ctxt, Ast_inf)));
    ox_value_set_number(ctxt, t, type);
    ox_not_error(ox_set(ctxt, ast, OX_STRING(ctxt, type), t));

    OX_VS_POP(ctxt, t)
}

/**
 * Get the AST node's type.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @return The type of the node.
 */
OX_AstType
ox_ast_get_type (OX_Context *ctxt, OX_Value *ast)
{
    OX_VS_PUSH(ctxt, t)
    OX_AstType type;

    ox_not_error(ox_get_throw(ctxt, ast, OX_STRING(ctxt, type), t));
    assert(ox_value_is_number(ctxt, t));
    type = ox_value_get_number(ctxt, t);

    OX_VS_POP(ctxt, t)
    return type;
}

/**
 * Get the AST node's location.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @param[out] loc Return the location of the node.
 */
void
ox_ast_get_loc (OX_Context *ctxt, OX_Value *ast, OX_Location *loc)
{
    OX_VS_PUSH_2(ctxt, l, n)

    ox_not_error(ox_get_throw(ctxt, ast, OX_STRING(ctxt, loc), l));
    assert(ox_value_is_object(ctxt, l));

    ox_not_error(ox_get_throw(ctxt, l, OX_STRING(ctxt, first_line), n));
    loc->first_line = ox_value_get_number(ctxt, n);

    ox_not_error(ox_get_throw(ctxt, l, OX_STRING(ctxt, first_column), n));
    loc->first_column = ox_value_get_number(ctxt, n);

    ox_not_error(ox_get_throw(ctxt, l, OX_STRING(ctxt, last_line), n));
    loc->last_line = ox_value_get_number(ctxt, n);

    ox_not_error(ox_get_throw(ctxt, l, OX_STRING(ctxt, last_column), n));
    loc->last_column = ox_value_get_number(ctxt, n);

    OX_VS_POP(ctxt, l)
}

/**
 * Set the AST node's location.
 * @param ctxt The current running context.
 * @param ast The AST node.
 * @param loc The location of the node.
 */
void
ox_ast_set_loc (OX_Context *ctxt, OX_Value *ast, OX_Location *loc)
{
    OX_VS_PUSH_2(ctxt, l, n)

    ox_not_error(ox_object_new(ctxt, l, NULL));
    ox_not_error(ox_set(ctxt, ast, OX_STRING(ctxt, loc), l));

    ox_value_set_number(ctxt, n, loc->first_line);
    ox_not_error(ox_set(ctxt, l, OX_STRING(ctxt, first_line), n));

    ox_value_set_number(ctxt, n, loc->first_column);
    ox_not_error(ox_set(ctxt, l, OX_STRING(ctxt, first_column), n));

    ox_value_set_number(ctxt, n, loc->last_line);
    ox_not_error(ox_set(ctxt, l, OX_STRING(ctxt, last_line), n));

    ox_value_set_number(ctxt, n, loc->last_column);
    ox_not_error(ox_set(ctxt, l, OX_STRING(ctxt, last_column), n));

    OX_VS_POP(ctxt, l)
}

/*Check if the value is an AST node.*/
static OX_Bool
value_is_ast (OX_Context *ctxt, OX_Value *v)
{
    if (!ox_value_is_object(ctxt, v))
        return OX_FALSE;

    return ox_instance_of(ctxt, v, OX_OBJECT(ctxt, Ast));
}

/*AST JSON map function.*/
static OX_Result
ast_json_map (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *root = ox_argument(ctxt, args, argc, 0);
    OX_Value *key = ox_argument(ctxt, args, argc, 1);
    OX_Value *v = ox_argument(ctxt, args, argc, 2);
    OX_VS_PUSH(ctxt, pv)
    OX_Bool is_ast;
    OX_Result r;

    is_ast = value_is_ast(ctxt, root);

    if (is_ast && ox_equal(ctxt, key, OX_STRING(ctxt, loc))) {
        OX_Location loc;
        char buf[256];

        ox_ast_get_loc(ctxt, root, &loc);

        if (loc.first_line == loc.last_line) {
            if (loc.first_column == loc.last_column) {
                snprintf(buf, sizeof(buf), "%d.%d",
                        loc.first_line, loc.first_column);
            } else {
                snprintf(buf, sizeof(buf), "%d.%d-%d",
                        loc.first_line, loc.first_column, loc.last_column);
            }
        } else {
            snprintf(buf, sizeof(buf), "%d.%d-%d.%d",
                    loc.first_line, loc.first_column, loc.last_line, loc.last_column);
        }

        if ((r = ox_string_from_char_star(ctxt, rv, buf)) == OX_ERR)
            goto end;
    } else if (is_ast && ox_equal(ctxt, key, OX_STRING(ctxt, type))) {
        int32_t type;

        if ((r = ox_to_int32(ctxt, v, &type)) == OX_ERR)
            goto end;

        if ((r = ox_string_from_const_char_star(ctxt, rv, ast_names[type])) == OX_ERR)
            goto end;
    } else if (is_ast && ox_equal(ctxt, key, OX_STRING(ctxt, decl_type))) {
        int32_t type;
        char *tn;
        char buf[16];

        if ((r = ox_to_int32(ctxt, v, &type)) == OX_ERR)
            goto end;

        switch (type & OX_DECL_TYPE_MASK) {
        case OX_DECL_CONST:
            tn = "const";
            break;
        case OX_DECL_VAR:
            tn = "var";
            break;
        case OX_DECL_PARAM:
            tn = "param";
            break;
        case OX_DECL_REF:
            tn = "ref";
            break;
        case OX_DECL_OUTER:
            tn = "var";
            break;
        default:
            assert(0);
        }

        snprintf(buf, sizeof(buf), "%s%s", type & OX_DECL_AUTO_CLOSE ? "#" : "", tn);

        if ((r = ox_string_from_char_star(ctxt, rv, buf)) == OX_ERR)
            goto end;
    } else {
        ox_value_copy(ctxt, rv, v);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, pv)
    return r;
}

/*AST JSON filter function.*/
static OX_Result
ast_json_filter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *root = ox_argument(ctxt, args, argc, 0);
    OX_Value *key = ox_argument(ctxt, args, argc, 1);
    OX_Value *v = ox_argument(ctxt, args, argc, 2);
    OX_Bool b = OX_TRUE, is_ast;

    is_ast = value_is_ast(ctxt, root);

    if (ox_value_is_null(ctxt, v)) {
        b = OX_FALSE;
    } else if (is_ast && ox_equal(ctxt, key, OX_STRING(ctxt, outer))) {
        b = OX_FALSE;
    }

    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/**
 * Convert the abstract syntax tree to string.
 * @param ctxt The current running context.
 * @param ast The abstract syntax tree.
 * @param[out] s The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_ast_to_string (OX_Context *ctxt, OX_Value *ast, OX_Value *s)
{
    OX_VS_PUSH_3(ctxt, indent, filter, map)
    OX_Result r;

    if ((r = ox_string_from_const_char_star(ctxt, indent, "  ")) == OX_ERR)
        goto end;
    if ((r = ox_native_func_new(ctxt, filter, ast_json_filter)) == OX_ERR)
        goto end;
    if ((r = ox_native_func_new(ctxt, map, ast_json_map)) == OX_ERR)
        goto end;

    r = ox_json_to_str(ctxt, ast, indent, filter, map, s);
end:
    OX_VS_POP(ctxt, indent)
    return r;
}

/**
 * Initialize the AST classes.
 * @param ctxt The current running context.
 */
void
ox_ast_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, v)

    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, Ast), OX_OBJECT(ctxt, Ast_inf),
            NULL, "Ast"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Ast", OX_OBJECT(ctxt, Ast)));

    OX_VS_POP(ctxt, v)
}

/*?
 *? @lib {Ast} Abstract syntax tree.
 *?
 *? @class{ Ast Node of the abstract syntax tree.
 *?
 *? @enum{ Type The type of the node.
 *? @item script The script.
 *? @item if If...else statement.
 *? @item do_while Do...while loop statement.
 *? @item while While loop statement.
 *? @item sched Schedule statement.
 *? @item for For loop statement.
 *? @item for_as For...as iterator loop statemtnt.
 *? @item case Case statement.
 *? @item case_func Case function condition.
 *? @item try Try...catch statement.
 *? @item return Return statement.
 *? @item throw Throw statement.
 *? @item break Break statement.
 *? @item continue Continue statement.
 *? @item block Statement block.
 *? @item value Constant value.
 *? @item id Identifier.
 *? @item string Multi-part string.
 *? @item format To string format.
 *? @item unary_expr Unary expression.
 *? @item binary_expr Binary expression.
 *? @item assi Assignment.
 *? @item rev_assi Reverse assignment.
 *? @item func Function.
 *? @item class Class.
 *? @item ref Reference.
 *? @item ref_item Reference item.
 *? @item expr_block Condition expression and statement block.
 *? @item exprs_block Condition expression and statement block list.
 *? @item all Select all.
 *? @item skip Skip the item.
 *? @item plus Plus operator.
 *? @item minus Minus operator.
 *? @item typeof Type of operatior.
 *? @item not_null Not null operator.
 *? @item bit_rev Bitwise reverse operator.
 *? @item logic_not Logic not operator.
 *? @item global Make a global reference operator.
 *? @item owned Make a owned reference operator.
 *? @item parenthese "("xxx")".
 *? @item spread Spread items.
 *? @item get_ptr Get the pointer of C value.
 *? @item get_value Get value of the C pointer.
 *? @item get Get the property value.
 *? @item exp Exponent operator.
 *? @item add Add operator.
 *? @item sub Subtract operator. 
 *? @item match Match operator.
 *? @item mul Multiply operator.
 *? @item div Divide operator.
 *? @item mod Modulo operator.
 *? @item shl Shift to left operator.
 *? @item shr Shift to right operator.
 *? @item ushr Unsigned integer shift to right operator.
 *? @item lt Less than operator.
 *? @item gt Greater than operator.
 *? @item le Less than or equal to operator.
 *? @item ge Greater than operator.
 *? @item instof Instance of operator.
 *? @item eq Equal to operator.
 *? @item ne Not equal to operatior.
 *? @item none None.
 *? @item bit_or Bitwise or operator.
 *? @item bit_xor Bitwise exclusive or operator.
 *? @item bit_and Bitwise and operator.
 *? @item logic_and Logic and operator.
 *? @item logic_or Logic or operator.
 *? @item comma Comma expressions list.
 *? @item call Call a function.
 *? @item args Arguments.
 *? @item params Formal parameters.
 *? @item arg Argument.
 *? @item rest The rest arguments.
 *? @item item_pattern Array item pattern.
 *? @item prop_pattern Object property pattern.
 *? @item array_pattern Array pattern.
 *? @item object_pattern Object pattern.
 *? @item prop Property.
 *? @item curr_object Current object.
 *? @item expr_name Property name in the form of an expression.
 *? @item const Constant.
 *? @item var Variable.
 *? @item method Method.
 *? @item accessor Accessor.
 *? @item array Array.
 *? @item object Object.
 *? @item array_append Append items to array.
 *? @item object_set Set properties of an object.
 *? @item decl Declaration.
 *? @item this This argument.
 *? @item argv Arguments of current function.
 *? @item enum Enumeration.
 *? @item bitfield Bitfield.
 *? @item doc Document.
 *? @enum}
 *?
 *? @enum{ DeclType The declaration's type.
 *? @item DECL_CONST Constant.
 *? @item DECL_PARAM Parameter.
 *? @item DECL_VAR Variable.
 *? @item DECL_REF Reference.
 *? @enum}
 *?
 *? @func $to_str Convert the AST node to OX source code.
 *? @return {String} The source code of the AST node.
 *?
 *? @class}
 */

/**
 * Add AST enumerations.
 * @param ctxt The current running context.
 */
void
ox_ast_add_enums (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, e)
    const char **pn;
    size_t id;

    /*Add AST types.*/
    ox_not_error(ox_enum_new(ctxt, e, OX_ENUM_ENUM));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Ast), "Type", e));

    for (id = 0, pn = ast_names; *pn; id ++, pn ++) {
        ox_enum_add_item_s(ctxt, e, OX_OBJECT(ctxt, Ast), *pn, id);
    }

    /*Add declaration types.*/
    ox_not_error(ox_enum_new(ctxt, e, OX_ENUM_ENUM));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Ast), "DeclType", e));

    ox_enum_add_item_s(ctxt, e, OX_OBJECT(ctxt, Ast), "DECL_CONST", OX_DECL_CONST);
    ox_enum_add_item_s(ctxt, e, OX_OBJECT(ctxt, Ast), "DECL_PARAM", OX_DECL_PARAM);
    ox_enum_add_item_s(ctxt, e, OX_OBJECT(ctxt, Ast), "DECL_VAR", OX_DECL_VAR);
    ox_enum_add_item_s(ctxt, e, OX_OBJECT(ctxt, Ast), "DECL_REF", OX_DECL_REF);

    OX_VS_POP(ctxt, e)
}