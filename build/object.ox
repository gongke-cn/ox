#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("keyword")

objects = [
    "Global"
    "Bool"
    "Bool_inf"
    "String"
    "String_inf"
    "StringIterator_inf"
    "Number"
    "Number_inf"
    "Object"
    "ObjectIterator_inf"
    "Function"
    "Function_inf"
    "Array_inf"
    "ArrayIterator_inf"
    "Iterator"
    "Iterator_inf"
    "MapIterator_inf"
    "SelectIterator_inf"
    "MatchIterator_inf"
    "SplitIterator_inf"
    "Error"
    "NullError"
    "RangeError"
    "NoMemoryError"
    "TypeError"
    "SyntaxError"
    "SystemError"
    "ReferenceError"
    "AccessError"
    "Script_inf"
    "ScriptIterator_inf"
    "Enum_inf"
    "EnumIterator_inf"
    "Re_inf"
    "Match_inf"
    "SetIterator_inf"
    "Dict_inf"
    "DictIterator_inf"
    "Int8"
    "Int16"
    "Int32"
    "Int64"
    "UInt8"
    "UInt16"
    "UInt32"
    "UInt64"
    "Float32"
    "Float64"
    "Void"
    "CType_inf"
    "CNumber_inf"
    "CPointer_inf"
    "CArrayIterator_inf"
    "Ast"
    "Ast_inf"
]

gen_def: func() {
    stdout.puts(''
typedef enum {
{{objects.$iter().map(("    OX_OBJ_ID_{$},")).$to_str("\n")}}
    OX_OBJ_ID_MAX
} OX_ObjectID;
    '')
}

if argv[1] == "-d" {
    gen_def()
}