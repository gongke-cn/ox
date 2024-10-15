ref "std/path"
ref "std/io"
ref "json"
ref "./cdecl"
ref "./log"

//Add public declaration index
add_pub_decl_id: func(ctxt, name) {
    ctxt.src.pub_decl_ids += "\
    OXN_ID_{name},
"
}

//Add local declaration index
add_local_decl_id: func(ctxt, name) {
    ctxt.src.local_decl_ids += "\
    OXN_ID_{name},
"
}

//Add public entry
add_pub_entry: func(ctxt, name) {
    ctxt.src.pub_entries += "\
    \"{name}\",
"
}

//Add constant
add_const: func(ctxt, name, type) {
    add_pub_decl_id(ctxt, name)
    add_pub_entry(ctxt, name)

    if type == "number" {
        set_val = "\
    ox_value_set_number(ctxt, v, {name});\
"
    } elif type == "bool" {
        set_val = "\
    ox_value_set_bool(ctxt, v, {name});\
"
    } else {
        set_val = "\
    if ((r = ox_string_from_const_char_star(ctxt, v, {name})) == OX_ERR)
        goto end;\
"
    }

    ctxt.src.exec_code += "\
    /*{name}*/
{set_val}
    if ((r = ox_script_set_value(ctxt, s, OXN_ID_{name}, v)) == OX_ERR)
        goto end;

"
}

//Add temporary variable
add_tmp_var: func(ctxt, type, name, value) {
    if ctxt.src.in_func {
        dict = ctxt.src.func_vars
    } else {
        dict = ctxt.src.exec_vars
    }

    if !dict.get(name) {
        dict.add(name, {
            type
            value
        })
    }
}

//Add temporary value
add_tmp_val: func(ctxt, name) {
    add_tmp_var(ctxt, "OX_Value *", name, "ox_value_stack_push(ctxt)");
}

//Get temporary variables
tmp_vars_code: func(dict) {
    return dict.entries().map(func([name, def]) {
        if def.value {
            return "    {def.type} {name} = {def.value};"
        } else {
            return "    {def.type} {name};"
        }
    }).$to_str("\n");
}

//Get a type
get_type: func(ctxt, type, script, ty, depth) {
    case type.kind {
    "builtin" {
        if type.type == "void" {
            name = "VOID"
        } else {
            name = type.type
        }
        return "    ox_value_copy(ctxt, {ty}, ox_ctype_get(ctxt, OX_CTYPE_{name}));\n"
    }
    "record" {
        return "    ox_value_copy(ctxt, {ty}, ox_script_get_value(ctxt, {script}, OXN_ID_{type.name}));\n"
    }
    "pointer" {
        tty = "tty{depth}"
        add_tmp_val(ctxt, tty)
        return "\
{get_type(ctxt, type.value, script, tty, depth + 1)}\
    if ((r = ox_ctype_pointer(ctxt, {ty}, {tty}, {type.length})) == OX_ERR)
        goto end;
"
    }
    "array" {
        tty = "tty{depth}"
        add_tmp_val(ctxt, tty)
        return "\
{get_type(ctxt, type.item, script, tty, depth + 1)}\
    if ((r = ox_ctype_array(ctxt, {ty}, {tty}, {type.length})) == OX_ERR)
        goto end;
"
    }
    "function" {
        rty = "rty{depth}"
        atys = "atys{depth}"
        add_tmp_val(ctxt, rty)
        add_tmp_val(ctxt, atys)

        if type.return.ctype == "void" {
            rcode = "\
    ox_value_set_null(ctxt, {rty});
"
        } else {
            rcode = get_type(ctxt, type.return, script, rty, depth + 1);
        }

        if type.parameters.length {
            aty = "aty{depth}"
            add_tmp_var(ctxt, "OX_Value *", aty)
            astart = "\
    {atys} = ox_value_stack_push_n(ctxt, {type.parameters.length});
"
            i = 0
            for type.parameters as param {
                abody += "\
    {aty} = ox_values_item(ctxt, {atys}, {i});
{get_type(ctxt, param, script, aty, depth + 1)}\
"
                i += 1
            }

            aend = "\
    ox_value_stack_pop(ctxt, {atys});
"
        }

        if type.vaarg {
            vaarg = "OX_TRUE"
        } else {
            vaarg = "OX_FALSE"
        }

        return "\
{rcode}\
{astart}\
{abody}\
    if ((r = ox_ctype_func(ctxt, {ty}, {rty}, {atys}, {type.parameters.length}, {vaarg})) == OX_ERR)
        goto end;
{aend}\
"
    }
    * {
        log.error("unknown type kind {type.kind}")
    }
    }
}

//Add function code
add_func_code: func(ctxt, name, fn) {
    ctxt.src.in_func = true
    ctxt.src.func_vars = Dict()

    code = fn()

    ctxt.src.func_code += "\
/*{name}*/
static OX_Result
{name} (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
\{
    OX_Value *top = ox_value_stack_top(ctxt);
    OX_Result r;
{tmp_vars_code(ctxt.src.func_vars)}

{code}
    r = OX_OK;
    goto end;
end:
    ox_value_stack_pop(ctxt, top);
    return r;
\}

"

    ctxt.src.in_func = false
}

//Add a bitfield
add_bitfield: func(ctxt, rec, field) {
    add_func_code(ctxt, "oxn_{rec.name}_{field.name}_get", func() {

        add_tmp_var(ctxt, "OX_Value*", "ty")
        add_tmp_var(ctxt, "{rec.ctype}*", "sp")

        return "\
    ty = ox_script_get_value(ctxt, f, OXN_ID_{rec.name});
    if (!ox_value_is_cptr(ctxt, thiz, ty)) \{
        r = ox_throw_type_error(ctxt, \"the value is not a \\\"{rec.name}\\\"\");
        goto end;
    \}

    if (!(sp = ox_cvalue_get_pointer(ctxt, thiz))) \{
        r = ox_throw_null_error(ctxt, \"the c pointer is null\");
        goto end;
    \}

    ox_value_set_number(ctxt, rv, sp->{field.name});
"
    })

    add_func_code(ctxt, "oxn_{rec.name}_{field.name}_set", func() {
        
        add_tmp_var(ctxt, "OX_Value*", "ty")
        add_tmp_var(ctxt, "{rec.ctype}*", "sp")
        add_tmp_var(ctxt, "int32_t", "i")
        add_tmp_var(ctxt, "OX_Value*", "arg", "ox_argument(ctxt, args, argc, 0)")

        return "\
    ty = ox_script_get_value(ctxt, f, OXN_ID_{rec.name});
    if (!ox_value_is_cptr(ctxt, thiz, ty)) \{
        r = ox_throw_type_error(ctxt, \"the value is not a \\\"{rec.name}\\\"\");
        goto end;
    \}

    if (!(sp = ox_cvalue_get_pointer(ctxt, thiz))) \{
        r = ox_throw_null_error(ctxt, \"the c pointer is null\");
        goto end;
    \}

    if ((r = ox_to_int32(ctxt, arg, &i)) == OX_ERR)
        goto end;

    sp->{field.name} = i;
"
    })

    add_tmp_val(ctxt, "ty")
    return "\
    /*{rec.name}.{field.name}*/
{get_type(ctxt, field.type, "s", "ty")}\
    if ((r = ox_object_add_n_accessor_s(ctxt, inf, \"{field.name}\", oxn_{rec.name}_{field.name}_get, oxn_{rec.name}_{field.name}_set)) == OX_ERR)
        goto end;
"
}

//Add a field
add_field: func(ctxt, rec, field) {
    if field.bitfield {
        return add_bitfield(ctxt, rec, field)
    } else {
        add_tmp_val(ctxt, "ty")
        return "\
    /*{rec.name}.{field.name}*/
{get_type(ctxt, field.type, "s", "ty")}\
    if ((r = ox_cstruct_add_field_s(ctxt, v, ty, \"{field.name}\", OX_OFFSET_OF({rec.ctype}, {field.name}))) == OX_ERR)
        goto end;
"
    }
}

//Generate variable declaration
var_decl: func(ctxt, type, name, bitfield)
{
    case type.kind {
    "builtin" {
        if bitfield {
            return "{type.ctype} {name}:{bitfield}"
        } else {
            return "{type.ctype} {name}"
        }
    }
    "record" {
        return "{type.ctype} {name}"
    }
    "array" {
        if type.length == -1 {
            len = ""
        } else {
            len = type.length
        }
        return var_decl(ctxt, type.item, "{name}[{len}]")
    }
    "pointer" {
        return var_decl(ctxt, type.value, "*{name}")
    }
    "function" {
        id = 0
        for type.parameters as p {
            if id {
                params += ", "
            }
            params += var_decl(ctxt, p, "p{id}")
            id += 1
        }

        return var_decl(ctxt, type.return, "(*{name})({params})")
    }
    }
}

//Add a record typedef
add_record_typedef: func(ctxt, rec)
{
    for rec.fields as field {
        fields += "\
    {var_decl(ctxt, field.type, field.name, field.bitfield)};
"
    }

    ctxt.src.typedefs += "\
/*{rec.name}*/
typedef struct \{
{fields}\
\} {rec.name};

"
}

//Add a record
add_record: func(ctxt, rec) {
    if rec.is_noname {
        add_record_typedef(ctxt, rec)
        add_local_decl_id(ctxt, rec.name)
    } else {
        add_pub_decl_id(ctxt, rec.name)
        add_pub_entry(ctxt, rec.name)
    }

    if rec.fields?.length {
        ctxt.src.field_exec_code += "\
    /*{rec.name} fields*/
    ox_value_copy(ctxt, v, ox_script_get_value(ctxt, s, OXN_ID_{rec.name}));
    if ((r = ox_get_s(ctxt, v, \"$inf\", inf)) == OX_ERR)
        goto end;
"
        for rec.fields as field {
            //Generate field code
            ctxt.src.field_exec_code += add_field(ctxt, rec, field)
        }
    }

    add_tmp_val(ctxt, "inf")

    if rec.complete {
        size = "sizeof({rec.ctype})"
    } else {
        size = "-1"
    }

    if rec.is_noname {
        create_code = "ox_ctype_struct(ctxt, v, inf, {size})"
    } else {
        create_code = "ox_named_ctype_struct_s(ctxt, v, inf, {size}, NULL, \"{rec.name}\")"
    }

    ctxt.src.func_code += ''
static const char *{{rec.name}}_keys[] = {
{{rec.fields.$iter().map(("    \"{$.name}\",\n")).$to_str("")}}    NULL
};

    ''

    ctxt.src.exec_code += ''
    /*{{rec.name}}*/
    if ((r = {{create_code}}) == OX_ERR)
        goto end;
    if ((r = ox_script_set_value(ctxt, s, OXN_ID_{{rec.name}}, v)) == OX_ERR)
        goto end;
    if ((r = ox_object_set_keys(ctxt, inf, {{rec.name}}_keys)) == OX_ERR)
        goto end;

    ''
}

//Add a variable
add_var: func(ctxt, var) {
    add_pub_decl_id(ctxt, var.name)
    add_pub_entry(ctxt, var.name)

    add_func_code(ctxt, "oxn_{var.name}_get", func() {
        add_tmp_val(ctxt, "ty");
        add_tmp_var(ctxt, "OX_CValueInfo", "cvi")

        return "\
{get_type(ctxt, var.type, "f", "ty")}
    cvi.v.p = (void*)&{var.name};
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    r = ox_cptr_get_value(ctxt, ty, &cvi, rv);
"
    })

    add_func_code(ctxt, "oxn_{var.name}_set", func() {
        add_tmp_var(ctxt, "OX_Value*", "arg", "ox_argument(ctxt, args, argc, 0)")
        add_tmp_val(ctxt, "ty");
        add_tmp_var(ctxt, "OX_CValueInfo", "cvi")

        return "\
{get_type(ctxt, var.type, "f", "ty")}
    cvi.v.p = (void*)&{var.name};
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    r = ox_cptr_set_value(ctxt, ty, &cvi, arg);
"
    })

    ctxt.src.exec_code += "\
    /*{var.name}*/
    if ((r = ox_object_new(ctxt, v, NULL)) == OX_ERR)
        goto end;
    if ((r = ox_object_set_name_s(ctxt, v, \"{var.name}\")) == OX_ERR)
        goto end;
    if ((r = ox_script_set_value(ctxt, s, OXN_ID_{var.name}, v)) == OX_ERR)
        goto end;
    if ((r = ox_object_add_n_method_s(ctxt, v, \"get\", oxn_{var.name}_get)) == OX_ERR)
        goto end;
    if ((r = ox_object_add_n_method_s(ctxt, v, \"set\", oxn_{var.name}_set)) == OX_ERR)
        goto end;
"
}

//Add function
add_function: func(ctxt, fn) {
    add_pub_decl_id(ctxt, fn.name)
    add_pub_entry(ctxt, fn.name)

    add_tmp_val(ctxt, "ty")

    ctxt.src.exec_code += "\
    /*{fn.name}*/
{get_type(ctxt, fn.type, "s", "ty")}\
    if ((r = ox_named_cfunc_new_s(ctxt, v, ty, {fn.name}, NULL, \"{fn.name}\")) == OX_ERR)
        goto end;
    if ((r = ox_script_set_value(ctxt, s, OXN_ID_{fn.name}, v)) == OX_ERR)
        goto end;

"
}

//Add function defined in configure file
add_config_func: func(ctxt, name, fn) {
    if fn.parameters {
        if Object.keys(fn.parameters).to_array().length == 0 {
            fn.parameters = null
        }
    }

    if fn.return {
        if fn.return == "void" {
            fn.return = null
        }
    }

    add_pub_decl_id(ctxt, name)
    add_pub_entry(ctxt, name)

    add_func_code(ctxt, "oxn_{name}", func() {
        if fn.parameters || fn.return {
            add_tmp_var(ctxt, "OX_CValueInfo", "cvi")

            param_code += "\
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

"
        }

        if fn.parameters {
            add_tmp_val(ctxt, "ty")

            id = 0
            for Object.entries(fn.parameters) as [pn, pt] {
                add_tmp_var(ctxt, "OX_Value*", "{pn}_v", "ox_argument(ctxt, args, argc, {id})")
                add_tmp_var(ctxt, pt, pn)

                if id {
                    arguments += ", "
                }
                arguments += pn

                ty = resolve_type(ctxt, pt)
                param_code += "\
{get_type(ctxt, ty, "f", "ty")}\
    cvi.v.p = &{pn};
    if ((r = ox_cptr_set_value(ctxt, ty, &cvi, {pn}_v)) == OX_ERR)
        goto end;
"
                id += 1
            }
        }

        if fn.return && fn.return != "void" {
            add_tmp_var(ctxt, fn.return, "oxn_ret")
            add_tmp_val(ctxt, "ty")

            ret_set = "oxn_ret = "

            ty = resolve_type(ctxt, fn.return)
            ret_code = "\
{get_type(ctxt, ty, "f", "ty")}\
    cvi.v.p = &oxn_ret;
    if ((r = ox_cptr_get_value(ctxt, ty, &cvi, rv)) == OX_ERR)
        goto end;
"
        }

        return "\
{param_code}
    {ret_set}{name}({arguments});

{ret_code}
"
    })

    ctxt.src.exec_code += "\
    /*{name}*/
    if ((r = ox_named_native_func_new_s(ctxt, v, oxn_{name}, NULL, \"{name}\")) == OX_ERR)
        goto end;
    if ((r = ox_script_set_value(ctxt, s, OXN_ID_{name}, v)) == OX_ERR)
        goto end;

"
}

//Get the type of the macro
get_macro_type: func(ctxt, macro) {
    if macro.args || !macro.data {
        return
    }

    for ctxt.number_macros as m {
        if m instof Re {
            if macro.name ~ m {
                return "number"
            }
        } elif m == macro.name {
            return "number"
        }
    }

    for ctxt.string_macros as m {
        if m instof Re {
            if macro.name ~ m {
                return "string"
            }
        } elif m == macro.name {
            return "string"
        }
    }

    if macro.data ~ /[-+]?(0[xX][0-9a-fA-F]+|\d*(\.\d*)?([eE][-+]?\d+)?|\.\d+([eE][-+]?\d+))?/p {
        return "number"
    }

    if macro.data ~ /".*"/p {
        return "string"
    }
}

//Scan the C macros
scan_macros: func(ctxt) {
    for ctxt.cmacros as macro {
        mt = get_macro_type(ctxt, macro)
        if mt {
            add_const(ctxt, macro.name, mt)
        }
    }
}

//Scan the C declarations
scan_decls: func(ctxt) {
    cdecls = ctxt.cdecls

    //Enumeration items
    for cdecls.enum_items as item {
        add_const(ctxt, item, "number")
    }

    //Records
    for cdecls.records as rec {
        add_record(ctxt, rec)
    }

    ctxt.src.exec_code += ctxt.src.field_exec_code

    //Variables
    for cdecls.vars as var {
        add_var(ctxt, var)
    }

    //Functions
    for cdecls.functions as fn {
        add_function(ctxt, fn)
    }
}

//Add reference entries
add_ref: func(ctxt, script, refs) {
    for refs as r {
        if String.is(r) {
            name = r
            ctxt.src.ref_entries += "\
    \{\"{script}\", \"{r}\", \"{r}\"\},
"
        } else {
            ctxt.src.ref_entries += "\
    \{\"{script}\", \"{r.original}\", \"{r.local}\"\},
"
            name = r.local
        }

        add_local_decl_id(ctxt, name)
    }
}

//Scan the target's configure file
scan_config: func(ctxt) {
    target = ctxt.target

    if target.references {
        for Object.entries(target.references) as [script, refs] {
            add_ref(ctxt, script, refs)
        }
    }

    if target.numbers {
        for target.numbers as n {
            add_const(ctxt, n, "number")
        }
    }

    if target.strings {
        for target.strings as n {
            add_const(ctxt, n, "string")
        }
    }

    if target.booleans {
        for target.booleans as n {
            add_const(ctxt, n, "booleans")
        }
    }

    if target.functions {
        for Object.entries(target.functions) as [name, fn] {
            add_config_func(ctxt, name, fn)
        }
    }
}

//Generate the source file
gen_src: func(ctxt) {
    target = ctxt.target

    includes = target.input_files.$iter().map(("#include <{$}>")).$to_str("\n")

    src = {
        pub_decl_ids: ""
        local_decl_ids: ""
        ref_entries: ""
        pub_entries: ""
        func_code: ""
        field_exec_code: ""
        exec_code: ""
        typedefs: ""
        func_vars: Dict()
        exec_vars: Dict()
        in_func: false
    }
    ctxt.src = src

    //Scan the C macros
    scan_macros(ctxt)
    //Scan configure files
    scan_config(ctxt)
    //Scan the C declarations
    scan_decls(ctxt)

    return "\
/*Source code of {ctxt.target_name}.oxn.*/
/*Generated by oxngen.*/

#define OX_LOG_TAG \"{ctxt.target_name}\"
#include <ox.h>
{includes}

{src.typedefs}\

/*Declaration index.*/
enum \{
{src.pub_decl_ids}\
{src.local_decl_ids}\
    OXN_ID_MAX
\};

/*Reference table.*/
static const OX_RefDesc
oxn_ref_desc[] = \{
{src.ref_entries}\
    \{NULL, NULL, NULL\}
\};

/*Public table.*/
static const char*
oxn_pub_tab[] = \{
{src.pub_entries}\
    NULL
\};

/*Script description.*/
static const OX_ScriptDesc
oxn_script_desc = \{
    oxn_ref_desc,
    oxn_pub_tab,
    OXN_ID_MAX
\};

{src.func_code}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
\{
    ox_not_error(ox_script_set_desc(ctxt, s, &oxn_script_desc));
    return OX_OK;
\}

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
\{
    OX_Value *v = ox_value_stack_push(ctxt);
    OX_Result r = OX_OK;
{tmp_vars_code(ctxt.src.exec_vars)}

{src.exec_code}
    r = OX_OK;
end:
    ox_value_stack_pop(ctxt, v);
    return r;
\}
"
}

public gen_oxn_src: func(ctxt) {
    target = ctxt.target
    outfile = "{ctxt.outdir}/{ctxt.target_name}.oxn.c"

    file = File(outfile, "wb")
    src = gen_src(ctxt)
    file.puts(src)
    file.$close()
}
