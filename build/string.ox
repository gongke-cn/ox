#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("string")

strings: [
    ["", "empty"]
    ["*", "star"]
    "true"
    "false"
    "$init"
    "$call"
    "$iter"
    "$close"
    "$name"
    "$scope"
    "$to_str"
    "$to_num"
    "$to_json"
    "$inf"
    "$class"
    "$owned"
    "$keys"
    "match"
    "message"
    "length"
    "next"
    "value"
    "end"
    "loc"
    "first_line"
    "first_column"
    "last_line"
    "last_column"
    "items"
    "args"
    "funcs"
    "params"
    "parents"
    "refs"
    "decls"
    "props"
    "models"
    "exprs"
    "decl"
    "expr"
    "cond"
    "block"
    "else"
    "catch"
    "finally"
    "init"
    "step"
    "left"
    "right"
    "operator"
    "operand1"
    "operand2"
    "file"
    "orig"
    "public"
    "private"
    "this"
    "name"
    "id"
    "pattern"
    "static"
    "get"
    "set"
    "keys"
    "has"
    "del"
    "call"
    "type"
    "decl_type"
    "outer"
    "hash"
    "path"
    "libraries"
    "executables"
    "format",
    "ques_src",
    "ques_dst",
    "script",
    "offset",
    "templ",
    "local"
    "textdomain"
    "doc"
    "modules"
]

gen_def: func() {
    for strings as s {
        if typeof s == Array {
            s = s[1]
        } elif s[0] == "$" {
            s = "_" + s.slice(1)
        }
        
        code += "    OX_STR_ID_{s},\n"
    }

    stdout.puts(''
typedef enum {
{{code}}    OX_STR_ID_MAX
} OX_StringID;
    '')
}

gen_tab: func() {
    for strings as s {
        if code {
            code += "\n"
        }

        if typeof s == Array {
            s = s[0]
        }

        code += "    \"{s}\","
    }

    stdout.puts(''
static const char*
string_table[] = {
{{code}}
};
    '')
}

if argv[1] == "-d" {
    gen_def()
} else {
    gen_tab()
}