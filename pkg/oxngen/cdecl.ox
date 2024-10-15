ref "std/fs"
ref "std/path"
ref "std/system"
ref "std/io"
ref "std/lang"
ref "std/temp_file"
ref "std/shell"
ref "json"
ref "./ctype"
ref "./tools"
ref "./log"

//Convert the pathname to unified format
pathname: func(n) {
    return n.replace("\\", "/")
}

//Get the file of the declaration
decl_get_file: func(decl) {
    if decl.loc {
        if decl.loc.file {
            return decl.loc.file
        }

        if decl.loc.expansionLoc?.file {
            return decl.loc.expansionLoc.file
        }
    }

    return null
}

//Solve the declarations
solve_decls: func(ctxt, decl)
{
    if !decl.id {
        return
    }

    if decl.loc {
        file = decl_get_file(decl)
        if file {
            ctxt.current_filename = file
        }
        if decl.loc.line {
            ctxt.current_line = decl.loc.line
        }
        if decl.loc.col {
            ctxt.current_col = decl.loc.col
        }
    }

    ctxt.id_decl_map[decl.id] = decl

    if decl.kind == "TypedefDecl" {
        ctxt.typedefs.push(decl)

        ctxt.name_typedef_map[decl.name] = decl
    } elif decl.kind == "RecordDecl" {
        if decl.name {
            if decl.tagUsed == "struct" {
                ctxt.name_struct_map[decl.name] = decl
            } elif decl.tagUsed == "union" {
                ctxt.name_union_map[decl.name] = decl
            }
        } else {
            name = "{ctxt.current_filename}:{ctxt.current_line}:{ctxt.current_col}"
            ctxt.noname_record_map[name] = decl
        }
    }

    if decl.inner {
        for decl.inner as idecl {
            solve_decls(ctxt, idecl)
        }
    }
}

//Sovle the typedefs
solve_typedefs: func(ctxt)
{
    for ctxt.typedefs as decl {
        def = decl.inner[0]

        if def.kind == "ElaboratedType" {
            rdecl = def.inner[0]
            if rdecl.kind == "RecordType" {
                rec = ctxt.id_decl_map[rdecl.decl.id]
                if rec {
                    if !ctxt.types.has(rec.name) {
                        rec.typedef = decl
                    }
                }
            }
        }
    }
}

//Add a name to the set.
add_name: func(ctxt, name) {
    if ctxt.cdecls.name_set.has(name) {
        warning(L"\"{name}\" is already declared")
        return false
    }

    ctxt.cdecls.name_set.add(name)
    return true
}

//Generate an enumeration declaration
gen_enum: func(ctxt, decl) {
    for decl.inner as idecl {
        if idecl.kind == "EnumConstantDecl" {
            if !decl_is_deprecated(idecl) && ctxt.decl_match(idecl.name) {
                if add_name(ctxt, idecl.name) {
                    ctxt.cdecls.enum_items.push(idecl.name)
                }
            }
        }
    }
}

//Generate a variable declaration
gen_var: func(ctxt, decl) {
    type = gen_type(ctxt, get_qual_type(decl))

    if add_name(ctxt, decl.name) {
        ctxt.cdecls.vars.push({
            name: decl.name
            type
        })
    }
}

//Get the record's name
record_get_name: func(decl) {
    if decl.typedef {
        return decl.typedef.name
    } else {
        return decl.name
    }
}

//Get the record's C type name
record_get_ctype: func(decl) {
    if decl.typedef {
        return decl.typedef.name
    } elif decl.tagUsed == "struct" {
        return "struct {decl.name}"
    } elif decl.tagUsed == "union" {
        return "union {decl.name}"
    } else {
        log.error("unknown record tag \"{decl.tagUsed}\"")
    }
}

//Generate type
gen_type: func(ctxt, type) {
    return parse_type(ctxt, type)
}

//Generate parameter type
gen_param_type: func(ctxt, type) {
    r = gen_type(ctxt, type)
    if r.kind == "array" {
        return {
            kind: "pointer"
            value: r.item
            length: r.length
        }
    }

    return r
}

//Generate parameter types
gen_param_types: func(ctxt, type, fn) {
    fn.parameters = []

    if type != "void" {
        for type.split(",") as param {
            param = param.trim()

            if param == "..." {
                fn.vaarg = true
                break
            }

            fn.parameters.push(gen_param_type(ctxt, param))
        }
    }
}

//Get record type's declaration
get_record_type_decl: func(ctxt, type)
{
    tdef = ctxt.name_typedef_map[type]
    if tdef {
        while true {
            if tdef.kind != "TypedefType" && tdef.kind != "ElaboratedType" {
                break
            }
            if !tdef.inner {
                break
            }
            tdef = tdef.inner[0]
        }

        if tdef.kind != "RecordType" {
            log.error("{type} is not a record type")
        } else {
            return ctxt.id_decl_map[tdef.decl.id]
        }
    } elif (match = type.match(/(struct|union).*\(.*at(.+)\)/)) {
        name = match.groups[2].trim()

        return ctxt.noname_record_map[name]
    } elif (match = type.match(/struct\s+(.+)/p)) {
        return ctxt.name_struct_map[match.groups[1].trim()]
    } elif (match = type.match(/union\s+(.+)/p)) {
        return ctxt.name_union_map[match.groups[1].trim()]
    }
}

//Generate fields
gen_fields: func(ctxt, decl, fields = [])
{
    if decl.inner {
        for decl.inner as idecl {
            if idecl.kind == "FieldDecl" {
                if idecl.isImplicit {
                    rdecl = get_record_type_decl(ctxt, get_qual_type(idecl))

                    gen_fields(ctxt, rdecl, fields)
                } else {
                    type = gen_type(ctxt, get_qual_type(idecl))
                    if type {
                        log.error("unknown type \"{get_qual_type(idecl)}\"")
                    }

                    fdef = {
                        name: idecl.name
                        type
                    }

                    if idecl.isBitfield {
                        fdef.bitfield = idecl.inner[0].value
                        if !fdef.bitfield {
                            fdef.bitfield = idecl.inner[0].inner[0].value
                        }
                    }

                    fields.push(fdef)
                }
            }
        }
    }

    return fields
}

//Generate a record declaration
gen_record: func(ctxt, name, decl) {
    fields = gen_fields(ctxt, decl)

    rec = {
        name
        ctype: record_get_ctype(decl)
        complete: decl.completeDefinition
        fields
    }

    ctxt.cdecls.records.push(rec)
}

//Get qualType
get_qual_type: func(decl) {
    if decl.type.desugaredQualType {
        return decl.type.desugaredQualType
    }

    return decl.type.qualType
}

//Generate a function declaration
gen_func: func(ctxt, decl) {
    if add_name(ctxt, decl.name) {
        type = gen_type(ctxt, get_qual_type(decl))

        fi = {
            name: decl.name
            type
        }

        ctxt.cdecls.functions.push(fi)
    }
}

//Check if the declaration is derecated
decl_is_deprecated: func(decl) {
    if decl.inner {
        for decl.inner as item {
            if item.kind == "DeprecatedAttr" {
                return true
            }
        }
    }

    return false
}

//Generate the C declarations
gen_cdecls: func(ctxt, json) {
    ctxt.cdecls = {
        enum_items: []
        records: []
        vars: []
        functions: []
        name_set: Set()
        record_map: {}
    }

    //Scan records.
    in_matched_file = false

    for json.inner as decl {
        //Match the input file
        file = decl_get_file(decl)
        if file {
            in_matched_file = ctxt.file_match(file)
        }

        if !in_matched_file {
            continue
        }

        if decl.typedef {
            name = decl.typedef.name
        } else {
            name = decl.name
        }

        if name {
            //Match the declaration's name
            if !ctxt.decl_match(name) {
                continue
            }
        }

        if decl.isImplicit {
            continue
        }

        if decl_is_deprecated(decl) {
            continue
        }

        if decl.kind == "RecordDecl" {
            name = record_get_name(decl)

            if add_name(ctxt, name) {
                ctxt.cdecls.record_map[name] = decl
            }
        }
    }

    //Scan other declarations.
    in_matched_file = false

    for json.inner as decl {
        //Match the input file
        file = decl_get_file(decl)
        if file {
            in_matched_file = ctxt.file_match(file)
        }

        if !in_matched_file {
            continue
        }

        if decl.typedef {
            name = decl.typedef.name
        } else {
            name = decl.name
        }

        if name {
            //Match the declaration's name
            if !ctxt.decl_match(name) {
                continue
            }
        }

        if decl.isImplicit {
            continue
        }

        if decl_is_deprecated(decl) {
            continue
        }

        case decl.kind {
        "EnumDecl" {
            gen_enum(ctxt, decl)
        }
        "VarDecl" {
            gen_var(ctxt, decl)
        }
        "FunctionDecl" {
            gen_func(ctxt, decl)
        }
        }
    }

    //Add records.
    for Object.entries(ctxt.cdecls.record_map) as [name, decl] {
        gen_record(ctxt, name, decl)
    }
}

//Parse the AST json file
parse: func(ctxt, fn)
{
    json = JSON.from_file(fn)

    ctxt.id_decl_map = {}
    ctxt.name_struct_map = {}
    ctxt.name_union_map = {}
    ctxt.name_typedef_map = {}
    ctxt.noname_record_map = {}
    ctxt.typedefs = []
    ctxt.loc_decl_map = {}
    ctxt.noname_countor = 0

    //Solve the declarations
    solve_decls(ctxt, json)

    //Sovle the typedefs
    solve_typedefs(ctxt)

    //Generate the C declarations
    gen_cdecls(ctxt, json)
}

//Get the C declarations
public get_cdecls: func(ctxt) {
    #outfile = TempFile("{ctxt.outdir}/{ctxt.target_name}.cdecls")
    cflags = ctxt.cflags()

    cmd = "clang {cflags} -Xclang -ast-dump=json {ctxt.input_file} > {outfile}"

    log.debug("exec \"{cmd}\"")
    r = Shell.run(cmd, Shell.ERROR)
    if r != 0 {
        throw Error(L"run \"{cmd}\" failed")
    }

    parse(ctxt, outfile)

    log.debug(JSON.to_str(ctxt.cdecls, "  "))
}

//Resolve the type
public resolve_type: func(ctxt, type) {
    return gen_param_type(ctxt, type)
}

//Add a noname record
public add_noname_record: func(ctxt, loc, decl) {
    old = ctxt.loc_decl_map[loc]
    if old {
        return old
    }

    name = "noname_{ctxt.noname_countor}"
    ctxt.noname_countor += 1

    rec = {
        is_noname: true
        name: name
        ctype: name
        fields: gen_fields(ctxt, decl)
    }

    ctxt.loc_decl_map[loc] = rec
    ctxt.cdecls.records.push(rec)

    return rec
}
