#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("ast")

ast = [
    "script"
    "if"
    "do_while"
    "while"
    "sched"
    "yield"
    "for"
    "for_as"
    "case"
    "case_func"
    "try"
    "return"
    "throw"
    "break"
    "continue"
    "block"
    "value"
    "id"
    "string"
    "format"
    "unary_expr"
    "binary_expr"
    "assi"
    "rev_assi"
    "func"
    "class"
    "ref"
    "ref_item"
    "expr_block"
    "exprs_block"
    "all"
    "skip"
    "plus"
    "minus"
    "typeof"
    "bit_rev"
    "logic_not"
    "global"
    "owned"
    "parenthese"
    "spread"
    "get_ptr"
    "get_value"
    "get"
    "lookup"
    "exp"
    "add"
    "sub"
    "match"
    "mul"
    "div"
    "mod"
    "shl"
    "shr"
    "ushr"
    "lt"
    "gt"
    "le"
    "ge"
    "instof"
    "eq"
    "ne"
    "none"
    "bit_or"
    "bit_xor"
    "bit_and"
    "logic_and"
    "logic_or"
    "comma"
    "call"
    "args"
    "params"
    "arg"
    "rest"
    "item_pattern"
    "prop_pattern"
    "array_pattern"
    "object_pattern"
    "prop"
    "curr_object"
    "expr_name"
    "const"
    "var"
    "method"
    "accessor"
    "array"
    "object"
    "array_append"
    "object_set"
    "decl"
    "this"
    "argv"
    "enum"
    "bitfield",
    "doc"
]

gen_def: func() {
    stdout.puts(''
typedef enum {
{{ast.$iter().map(("    OX_AST_{$},")).$to_str("\n")}}
    OX_AST_MAX
} OX_AstType;
    '')
}

gen_tab: func() {
    stdout.puts(''
static const char*
ast_names[] = {
{{ast.$iter().map(("    \"{$}\",")).$to_str("\n")}}
    NULL
};
    '')
}

if argv[1] == "-d" {
    gen_def()
} else {
    gen_tab()
}