ref "std/ast"
ref "std/path"
ref "json"
ref "./input"
ref "./command"
ref "./error"
ref "./log"

//Get the property of an object.
object_get_prop: func(o, pn) {
    for o.props as prop {
        if prop.type == Ast.prop {
            if prop.name.value == pn {
                return prop.expr
            } 
        }
    }
}

//Get the string type property of an object.
object_get_str_prop: func(o, pn) {
    v = object_get_prop(o, pn)
    if v {
        if v.type == Ast.value && String.is(v.value) {
            return v.value
        }
    }
}

//Get the option argument type property of an object.
object_get_arg_type: func(o, pn) {
    e = object_get_prop(o, pn)
    if e {
        if e.type == Ast.binary_expr &&
                e.operator.type == Ast.get &&
                e.operand1.type == Ast.id &&
                e.operand1.value == "Option" {
            return e.operand2.value
        } elif e.type == Ast.array {
            return e.items.$iter().map(($.value)).$to_str("|")
        }
    }
}

//Document node.
DocNode: class {
    static enum_num = 0
    static bitfield_num = 0

    //Initialize the document node.
    $init(nty, loc) {
        this.node_type = nty
        this.loc = loc
    }

    //Output error message.
    error(loc, msg) {
        error(loc, msg)
        throw SyntaxError(msg)
    }

    //Add a child node.
    add_child(name, child) {
        old = this[name]

        if old {
            error(child.loc, L"\"{name}\" is already defined")
            note(old.loc, L"previous definition is here")

            throw SyntaxError(L"\"{name}\" is already defined")
        }

        this[name] = child
    }

    //Add a named child node.
    add_named_child(group, name, child) {
        o = this[group]
        if !o {
            o = {}
            this[group] = o
        }

        old = o[name]

        if old {
            error(child.loc, L"\"{name}\" is already defined")
            note(old.loc, L"previous definition is here")

            throw SyntaxError(L"\"{name}\" is already defined")
        }

        o[name] = child
    }

    //Add a child item.
    add_child_item(name, child) {
        items = this[name]

        if !items {
            items = []
            this[name] = items
        }

        items.push(child)
    }

    //Check the function AST node.
    check_func_ast(ast) {
        //Build the parameters list.
        if ast.type == Ast.method {
            ast = ast.expr
        }

        if !this.plist && ast.params {
            this.plist = ast.params.$to_str()
        }

        //Check the parameters.
        if ast.params {
            //Build the parameter map.
            pmap = {}

            add_param: func(p) {
                case p.type {
                Ast.rest {
                    add_param(p.pattern)
                }
                Ast.id {
                    pmap[p.value] = p
                }
                Ast.array_pattern {
                    for p.items as item {
                        if item.pattern {
                            add_param(item.pattern)
                        }
                    }
                }
                Ast.object_pattern {
                    for p.props as prop {
                        if prop.pattern {
                            add_param(prop.pattern)
                        }
                    }
                }
                }
            }

            for ast.params.items as param {
                add_param(param)
            }

            //Check the parameter map.
            for Object.entries(pmap) as [pn, pdef] {
                if !this.param?[pn] {
                    warning(pdef.loc, L"parameter \"{pn}\" have not corresponding documentation")
                } else {
                    //Set the default value.
                    pdoc = this.param[pn]

                    if !pdoc.value && pdef.expr {
                        pdoc.value = pdef.expr.$to_str()
                        log.debug("default {pn}, {pdoc.value}")
                    }
                }
            }

            for Object.entries(this.param) as [pn, pdef] {
                if !pmap[pn] {
                    warning(pdef.loc, L"parameter \"{pn}\" is not a valid parameter")
                }
            }
        }
    }

    //Add document of an AST node
    add_ast_doc(name, doc, ast) {
        case ast.type {
        Ast.id {
            type = "item"
        }
        Ast.enum {
            type = "enum"

            if !name {
                name = "enum{DocNode.enum_num.+=1}"
            }
        }
        Ast.bitfield {
            type = "bitfield"

            if !name {
                name = "enum{DocNode.bitfield_num.+=1}"
            }
        }
        Ast.const {
            east = ast.expr
            type = "const"

            if east {
                case east.type {
                Ast.class {
                    type = "class"
                    ast = east
                }
                Ast.object {
                    type = "object"
                    ast = east
                }
                }
            }
        }
        Ast.var {
            if ast.static {
                type = "svar"
            } else {
                type = "var"
            }

            east = ast.expr
            case east.type {
            Ast.class {
                type = "class"
                ast = east
            }
            Ast.object {
                type = "object"
                ast = east
            }
            }
        }
        Ast.prop {
            east = ast.expr
            case east.type {
            Ast.class {
                type = "class"
                ast = east
            }
            Ast.object {
                type = "object"
                ast = east
            }
            * {
                type = "var"
            }
            }
        }
        Ast.accessor {
            if ast.static {
                if ast.set {
                    type = "sacc"
                } else {
                    type = "sroacc"
                }
                group = "sacc"
            } else {
                if ast.set {
                    type = "acc"
                } else {
                    type = "roacc"
                }
                group = "acc"
            }
        }
        Ast.func
        Ast.method {
            if ast.static {
                type = "sfunc"
            } else {
                type = "func"
            }
        }
        Ast.class {
            type = "class"
        }
        * {
            if ast.type == Ast.call &&
                    ast.expr.type == Ast.id &&
                    ast.expr.value == "Option" {
                type = "option"
                name = null
            } elif ast.type == Ast.object {
                type = "object"
            } elif this.#ast {
                decl = this.#ast.funcs[0].decls[name]

                if decl.decl_type == Ast.DECL_CONST {
                    type = "const"
                } else {
                    type = "var"
                }
            } else {
                this.error(doc.loc, L"illegal command")
            }
        }
        }

        if !group {
            group = type
        }

        cn = DocNode(type, doc.loc)

        if name {
            this.add_named_child(group, name, cn)
        } else {
            this.add_child(type, cn)
        }

        input = Input(doc.loc.first_line, doc.loc.first_column, doc.value)

        Command.lookup(type) => {cmd}
        cmd.solve(cn, input)

        case type {
        "func"
        "sfunc" {
            cn.check_func_ast(ast)
        }
        "class" {
            for ast.props as prop {
                if !prop.doc {
                    continue
                }

                cn.add_ast_doc(prop.name?.value, prop.doc, prop)
            }
        }
        "object" {
            for ast.props as prop {
                if !prop.doc {
                    continue
                }

                cn.add_ast_doc(prop.name?.value, prop.doc, prop)
            }
        }
        "enum"
        "bitfield" {
            for ast.items as item {
                if !item.doc {
                    continue
                }

                cn.add_ast_doc(item.value, item.doc, item)
            }
        }
        "option" {
            if ast.args {
                opts = cn.oi
                if !opts {
                    opts = {}
                    cn.oi = opts
                }

                odef = ast.args.items[0]
                if odef && odef.type == Ast.array {
                    for odef.items as item {
                        sn = object_get_str_prop(item, "short")
                        ln = object_get_str_prop(item, "long")
                        if sn && ln {
                            name = "-{sn},--{ln}"
                        } elif sn {
                            name = "-{sn}"
                        } elif ln {
                            name = "--{ln}"
                        } else {
                            continue
                        }

                        desc = opts[name]
                        if !desc {
                            desc = DocNode(item.loc)

                            help = object_get_str_prop(item, "help")
                            arg = object_get_arg_type(item, "arg")

                            opts[name] = {
                                type: arg
                                brief: [
                                    help
                                ]
                            }
                        }
                    }
                }
            }
        }
        }
    }
}

//Module.
public Module: class DocNode {
    //Initialize the module.
    $init(filename) {
        DocNode.$inf.$init.call(this, "module")

        this.filename = filename

        base = basename(filename)

        m = base.match(/(.+)\.(c|cc|cpp|cxx|h|hpp|hxx|ox)/ip)
        if !m {
            throw TypeError(L"not supported file format \"{filename}\"")
        }

        fmt = m.groups[1].to_lower()
        if fmt == "ox" {
            this.name = m.groups[1]
        } else {
            this.name = m.groups[1].replace(/\.oxn$/i, "")
        }

        this.format = m.groups[2]

        log.debug("add module \"{this.name}\"")
    }

    //Set the root AST of the module.
    set_ast(ast) {
        this.#ast = ast
    }

    //Add document of an AST node
    add_ast_doc(name, doc, ast) {
        DocNode.$inf.add_ast_doc.call(this, name, doc, ast)

        this.valid = true
    }

    //Add document text.
    add_doc(pkg, line, col, text) {
        input = Input(line, col, text)

        while true {
            ct = input.get_cmd(true)
            if !ct {
                break
            }

            has_end = false

            if ct.text == "package" {
                Command.lookup("package") => {cmd}
                cn = pkg

                nt = input.get_word()

                if cn.name {
                    msg = L"package is already defined"
                    error(nt.loc, msg)
                    note(cn.loc, L"previous definition is here")
                    throw SyntaxError(msg)
                }

                cn.name = nt.text
                cn.loc = nt.loc
                pkg.valid = true
            } else {
                if !(lr = Command.lookup(ct.text)) {
                    this.error(ct.loc, L"illegal document command \"{ct.text}\"")
                }

                if lr.flag == '}' {
                    this.error(ct.loc, L"command \"{ct.text}\" cannot be used here")
                }

                cmd = lr.cmd

                if lr.flag == '{' {
                    has_end = true
                }

                Command.lookup("module") => {cmd:mod_cmd}
                if !mod_cmd.desc.children.has(cmd.desc.name) {
                    this.error(ct.loc, L"command \"{ct.text}\" cannot be used here")
                }

                cn = DocNode(cmd.desc.name, ct.loc)

                if cmd.desc.group {
                    nt = input.get_word()

                    this.add_named_child(cmd.desc.group, nt.text, cn)
                } else {
                    this.add_child(cmd.desc.name, cn)
                }

                this.valid = true
            }

            cmd.solve(cn, input, has_end)

            c = input.getc()
            if c == null {
                break
            }

            input.ungetc(c)
        }
    }
}

//Document.
public Document: class DocNode {
    //Initialize the document.
    $init() {
        DocNode.$inf.$init.call(this, "doc")
    }
}
