ref "std/char"
ref "json"
ref "./log"

//C type parser
Parser: class {
    static {
        TOKEN_END: -1
        TOKEN_ID: 256
        TOKEN_NUMBER: 257
        TOKEN_ELLIPSIS: 258
    }

    //Initialize the parser
    $init(ctxt, s) {
        this.ctxt = ctxt
        this.s = s
        this.pos = 0
        this.token = null
    }

    //Get a character from the input
    get_char() {
        if this.pos < this.s.length {
            c = this.s.char_at(this.pos)
            this.pos += 1
        } else {
            c = null
        }

        return c
    }

    //Get a character not space
    get_char_ns () {
        while true {
            c = this.get_char()
            if c == null {
                return c
            }

            if !isspace(c) {
                return c
            }
        }
    }

    //Push back a character to the input
    unget_char(c) {
        if c != null {
            this.pos -= 1
        }
    }

    //Get a token from the input
    get_token() {
        if this.token {
            t = this.token
            this.token = null
            return t
        }

        c = this.get_char_ns()
        if c == null {
            return {
                type: Parser.TOKEN_END
            }
        }

        if isdigit(c) {
            s = String.from_uchar(c)

            while true {
                c = this.get_char()
                if !isdigit(c) {
                    this.unget_char(c)
                    break
                }
                s += String.from_uchar(c)
            }

            return {
                type: Parser.TOKEN_NUMBER
                s
            }
        } elif isalpha(c) || (c == '_') {
            s = String.from_uchar(c)

            while true {
                c = this.get_char()
                if !isalnum(c) && (c != '_') {
                    this.unget_char(c)
                    break
                }
                s += String.from_uchar(c)
            }

            return {
                type: Parser.TOKEN_ID
                s
            }
        } elif c == '.' {
            nc = this.get_char()
            if nc == '.' {
                nnc = this.get_char()
                if nnc == '.' {
                    return {
                        type: Parser.TOKEN_ELLIPSIS
                    }
                } else {
                    this.unget_char(nnc)
                    this.unget_char(nc)
                }
            } else {
                this.unget_char(nc)
            }
        }
        
        return {
            type: c
        }
    }

    //Push back a token to the input
    unget_token(t) {
        if t.type != Parser.TOKEN_END {
            this.token = t
        }
    }

    //Throw parse error
    error() {
        throw Error(L"unknown type \"{this.s}\"")
    }

    //Check if the token is an identifier
    is_id(t, id) {
        if t.type == Parser.TOKEN_ID && t.s == id {
            return true
        }

        return false
    }

    //Get builtin type
    builtin_type(type, ctype) {
        return {
            kind: "builtin"
            type
            ctype
        }
    }

    //Parse primitive type
    prim_type(signed) {
        t = this.get_token()
        if t.type == Parser.TOKEN_ID {
            case t.s {
            "void" {
                type = "void"
                ctype = "void"
            }
            "char", "bool", "_Bool" {
                if signed {
                    type = "I8"
                    ctype = "char"
                } else {
                    type = "U8"
                    ctype = "unsigned char"
                }
            }
            "short" {
                if signed {
                    type = "I16"
                    ctype = "short"
                } else {
                    type = "U16"
                    ctype = "unsigned short"
                }
            }
            "int" {
                if signed {
                    type = "I32"
                    ctype = "int"
                } else {
                    type = "U32"
                    ctype = "unsigned int"
                }
            }
            "long" {
                t = this.get_token()
                if this.is_id(t, "long") {
                    if signed {
                        type = "I64"
                        ctype = "long long"
                    } else {
                        type = "U64"
                        ctype = "unsigned long long"
                    }
                } else {
                    this.unget_token(t)
                    if signed {
                        type = "SSIZE"
                        ctype = "long"
                    } else {
                        type = "SIZE"
                        ctype = "unsigned long"
                    }
                }
            }
            }
        }

        if type {
            return this.builtin_type(type, ctype)
        } else {
            this.unget_token(t)
        }
    }

    //Parse pointer and array items
    ptr_array(ty) {
        while true {
            t = this.get_token()
            if t.type == '*' {
                if ty.is_proto {
                    ty.is_proto = false
                } else {
                    ty = {
                        kind: "pointer"
                        value: ty
                        length: -1
                    }
                }

                t = this.get_token()
                if !this.is_id(t, "const") {
                    this.unget_token(t)
                }
            } elif t.type == '[' {
                start = this.pos
                while true {
                    c = this.get_char()
                    if c == ']' {
                        if this.pos == start + 1 {
                            len = -1
                        } else {
                            len = this.s.slice(start, this.pos - 1)
                        }
                        ty = {
                            kind: "array"
                            item: ty
                            length: len
                        }
                        break
                    }
                }
            } else {
                this.unget_token(t)
                break
            }
        }

        return ty
    }

    //Parse signed/unsigned primitive type
    signed_prim_type() {
        t = this.get_token()
        if this.is_id(t, "signed") {
            ty = this.prim_type(true)
            if !t {
                this.error()
            }
        } elif this.is_id(t, "unsigned") {
            ty = this.prim_type(false)
            if !t {
                this.error()
            }
        } elif this.is_id(t, "float") {
            ty = this.builtin_type("F32", "float")
        } elif this.is_id(t, "double") {
            ty = this.builtin_type("F64", "double")
        } else {
            this.unget_token(t)
            ty = this.prim_type(true)
        }

        if ty {
            ty = this.ptr_array(ty)
        }

        return ty
    }

    //(unnamed)
    unnamed() {
        c = this.get_char()
        if c != '(' {
            this.error()
        }

        level = 0

        start = this.pos - 1
        while true {
            c = this.get_char()
            if c == '(' {
                level += 1
            } elif c == ')' {
                if level {
                    level -= 1
                } else {
                    break
                }
            }
        }

        return this.s.slice(start, this.pos)
    }

    //Parse type name
    type_name() {
        c = this.get_char_ns()
        if isalpha(c) || (c == '_') {
            this.unget_char(c)
            t = this.get_token()
            tn = t.s

            c = this.get_char()
            nc = this.get_char()

            if c == ':' && nc == ':' {
                tn += "::" + this.unnamed()
            } else {
                this.unget_char(nc)
                this.unget_char(c)
            }
        } elif c == '(' {
            this.unget_char(c)
            tn = this.unnamed()
        } else {
            this.error()
        }

        //log.debug("name: \"{tn}\"")
        return tn
    }

    //Get type of the record declaration
    record_decl_type(decl) {
        if decl {
            if decl.typedef {
                name = decl.typedef.name
                ctype = decl.typedef.name
            } elif decl.tagUsed == "struct" {
                name = decl.name
                ctype = "struct {decl.name}"
            } elif decl.tagUsed == "union" {
                name = decl.name
                ctype = "union {decl.name}"
            } else {
                throw Error(L"unknown record tag \"{decl.tagUsed}\"")
            }

            if !decl.completeDefinition {
                if !this.ctxt.cdecls.record_map[name] {
                    return {
                        kind: "builtin"
                        type: "void"
                    }
                }
            }

            return {
                kind: "record"
                name
                ctype
            }
        } else {
            return {
                kind: "builtin"
                type: "void"
            }
        }
    }

    //Parse record type
    record_type(name, kind) {
        if (match = name.match(/\(\w+\s*at\s+(.+)\)/)) {
            loc = match.groups[1]

            decl = this.ctxt.noname_record_map[loc]
            if decl {
                rec = this.ctxt.add_noname_record(loc, decl)
                return {
                    kind: "record"
                    name: rec.name
                    ctype: rec.ctype
                }
            } else {
                this.error()
            }
        } else {
            if kind == "struct" {
                decl = this.ctxt.name_struct_map[name]
            } else {
                decl = this.ctxt.name_union_map[name]
            }

            return this.record_decl_type(decl)
        }
    }

    //Parse typedef type
    typedef_type(name) {
        decl = this.ctxt.name_typedef_map[name]
        if !decl {
            this.error()
        }

        tdef = decl.inner[0]
        while true {
            if tdef.kind != "TypedefType" && tdef.kind != "ElaboratedType" {
                break
            }
            if !tdef.inner {
                break
            }
            if this.ctxt.types.has(tdef.name) {
                break
            }
            tdef = tdef.inner[0]
        }

        get_qual_type: func(decl) {
            if decl.type.desugaredQualType {
                return decl.type.desugaredQualType
            }

            return decl.type.qualType
        }

        case tdef.kind {
        "BuiltinType" {
            return parse_type(this.ctxt, get_qual_type(tdef))
        }
        "EnumType" {
            return {
                kind: "builtin"
                type: "I32"
                ctype: "int"
            }
        }
        "PointerType" {
            return parse_type(this.ctxt, get_qual_type(tdef))
        }
        "RecordType" {
            decl = this.ctxt.id_decl_map[tdef.decl.id]

            return this.record_decl_type(decl)
        }
        "FunctionProtoType" {
            ty = parse_type(this.ctxt, get_qual_type(tdef))

            ty.is_proto = true

            return ty
        }
        }
    }

    //Parse basic type
    basic_type() {
        ty = this.signed_prim_type()
        if !ty {
            t = this.get_token()
            if this.is_id(t, "enum") {
                tn = this.type_name()
                ty = this.builtin_type("I32", "int")
            } elif this.is_id(t, "union") {
                tn = this.type_name()
                ty = this.record_type(tn, "union")
            } elif this.is_id(t, "struct") {
                tn = this.type_name()
                ty = this.record_type(tn, "struct")
            } elif t.type == Parser.TOKEN_ID {
                ty = this.typedef_type(t.s)
            } else {
                this.error()
            }

            ty = this.ptr_array(ty)
        }

        return ty
    }

    //Get parameters list
    get_plist() {
        plist = {
            parameters: []
        }

        while true {
            t = this.get_token()

            if t.type == Parser.TOKEN_ELLIPSIS {
                plist.vaarg = true
                t = this.get_token()
                if t.type == ')' {
                    break;
                }
                this.error()
            } elif t.type == ')' {
                break
            }

            this.unget_token(t)
            aty = this.get_type()
            if aty.type != "void" {
                plist.parameters.push(aty)
            }

            t = this.get_token()

            if t.type == ')' {
                break
            } elif t.type != ',' {
                this.error()
            }
        }

        return plist
    }

    //Get a type
    get_type() {
        t = this.get_token()
        if this.is_id(t, "const") {
            ty = this.basic_type()
        } else {
            this.unget_token(t)
            ty = this.basic_type()
        }
        if !ty {
            this.error()
        }

        while true {
            t = this.get_token()
            if t.type == '(' {
                t = this.get_token()
                if t.type == '*' {
                    fty = {
                        kind: "function"
                        return: ty
                    }
                    ty = this.ptr_array(fty)

                    t = this.get_token()
                    if t.type == '(' {
                        plist = this.get_plist()
                        rty = {
                            kind: "function"
                            return: fty.return
                            parameters: plist.parameters
                            vaarg: plist.vaarg
                        }

                        fty.return = rty

                        t = this.get_token()
                    }

                    if t.type != ')' {
                        this.error()
                    }

                    t = this.get_token()
                    if t.type != '(' {
                        this.error()
                    }
                } else {
                    this.unget_token(t)

                    fty = {
                        kind: "function"
                        return: ty
                    }
                    ty = fty
                }

                plist = this.get_plist()

                fty.vaarg = plist.vaarg
                fty.parameters = plist.parameters

                t = this.get_token()
                if this.is_id(t, "__attribute__") {
                    t = this.get_token()
                    if t.type != '(' {
                        this.error()
                    } else {
                        level = 0
                        while true {
                            t = this.get_token()
                            if t.type == '(' {
                                level += 1
                            } elif t.type == ')' {
                                if level {
                                    level -= 1
                                } else {
                                    break
                                }
                            }
                        }
                    }
                } else {
                    this.unget_token(t)
                }
            } else {
                this.unget_token(t)
                break
            }
        }

        return ty
    }

    //Parse the C type
    parse() {
        log.debug("parse \"{this.s}\"")

        ty = this.get_type()
        
        t = this.get_token()
        if t.type != Parser.TOKEN_END {
            this.error()
        }

        //log.debug("ty: {this.s} => {JSON.to_str(ty)}")

        return ty
    }
}

//Parse the type string
public parse_type: func(ctxt, str) {
    p = Parser(ctxt, str)

    return p.parse()
}
