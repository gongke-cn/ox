ref "json"
ref "./error"
ref "./log"

//Command
public Command: class {
    //Commands dictionary.
    static dict = Dict()

    //Initialize a command.
    $init(desc) {
        this.desc = desc

        Command.dict.add(desc.name, this)
    }

    //Lookup a command by its name.
    static lookup(name) {
        last = name.char_at(name.length - 1)

        case last {
        '{'
        '}' {
            name = name.slice(0, name.length - 1)
            flag = last
        }
        }

        cmd = this.dict.get(name)
        if !cmd {
            return null
        }

        return {
            cmd
            flag
        }
    }

    //Solve the brief description.
    solve_brief(dn, input) {
        text = []

        input.eatup_space_no_lt()

        sb = String.Builder()

        while true {
            c = input.getc()
            
            if c == '\\' {
                nc = input.getc()
                if nc == '\n' {
                    continue
                } else {
                    input.ungetc(nc)
                }
            }

            if c == '\n' || c == null {
                if sb.length {
                    text.push(sb.$to_str().trim())
                }
                break
            }

            if c == '@' {
                nc = input.getc()
                if nc == '@' {
                    sb.append_char('@')
                } else {
                    if sb.length {
                        text.push(sb.$to_str().trim())
                        sb = String.Builder()
                    }
                    input.ungetc(nc)
                    input.ungetc(c)
                    ct = input.get_cmd()

                    lr = Command.lookup(ct.text)
                    if !lr {
                        input.error(ct.loc, L"illegal command \"{ct.text}\"")
                    }

                    cmd = lr.cmd
                    has_end = lr.flag == '{'

                    if !cmd.desc.in_line || lr.flag == '}' {
                        input.unget_cmd(ct)
                        break
                    }

                    cn = dn(cmd.desc.name)
                    cmd.solve(cn, input, has_end)
                    text.push(cn)
                }
            } else {
                sb.append_char(c)
            }
        }

        return text
    }

    //Solve the detail description.
    solve_detail(dn, input) {
        text = []

        input.eatup_space()

        sb = String.Builder()

        while true {
            c = input.getc()
            if c == '@' {
                nc = input.getc()
                if nc == '@' {
                    sb.append_char('@')
                } else {
                    if sb.length {
                        text.push(sb.$to_str().trim())
                        sb = String.Builder()
                    }

                    input.ungetc(nc)
                    input.ungetc(c)
                    ct = input.get_cmd()

                    lr = Command.lookup(ct.text)
                    if !lr {
                        input.error(ct.loc, L"illegal command \"{ct.text}\"")
                    }

                    cmd = lr.cmd
                    has_end = lr.flag == '{'

                    if (!cmd.desc.in_text && !cmd.desc.in_line) || lr.flag == '}' {
                        input.unget_cmd(ct)
                        break
                    }

                    cn = dn(cmd.desc.name)

                    cmd.solve(cn, input, has_end)
                    text.push(cn)
                }
            } elif c == null {
                if sb.length {
                    text.push(sb.$to_str().trim())
                }
                break
            } else {
                sb.append_char(c)
            }
        }

        return text
    }

    //Solve the command.
    solve(n, input, has_end) {
        DocNode = typeof n

        //Get parameters list.
        if this.desc.has_plist {
            t = input.get_plist()
            if t {
                n.plist = t.text
            }
        }

        //Get type.
        if this.desc.has_type {
            t = input.get_type()
            if t {
                n.type = t.text
            }
        }

        //Get value.
        if this.desc.has_value {
            t = input.get_value()
            if t {
                n.value = t.text
            }
        }

        //Get brief description.
        if this.desc.has_brief {
            n.brief = this.solve_brief(DocNode, input)
        }

        //Get detail description.
        if this.desc.has_detail {
            n.detail = this.solve_detail(DocNode, input)
        }

        //Get code.
        if this.desc.has_code {
            n.code = input.get_code()

            ct = input.get_cmd()
            lr = Command.lookup(ct.text)
            if !lr {
                input.error(ct.loc, L"illegal command \"{ct.text}\"")
            } elif lr.flag != '}' || lr.cmd.desc.name != "code" {
                input.error(ct.loc, L"expect \"code\}\" here")
            }

            return
        }

        if !this.desc.children {
            return
        }

        //Get children commands.
        while true {
            input.eatup_space()

            c = input.getc()
            if c == null {
                break
            }

            input.ungetc(c)
            ct = input.get_cmd()

            lr = Command.lookup(ct.text)
            if !lr {
                input.error(ct.loc, L"illegal command \"{ct.text}\"")
            }

            cmd = lr.cmd
            c_has_end = lr.flag == '{'

            if has_end && cmd.desc.name == this.desc.name && lr.flag == '}' {
                break
            }

            if !this.desc.children?.has(cmd.desc.name) {
                if has_end {
                    input.error(ct.loc, L"expect \"{this.desc.name}\}\" here")
                }
                input.unget_cmd(ct)
                break
            }

            cn = DocNode(cmd.desc.name, ct.loc)

            if cmd.desc.group {
                nt = input.get_word()
                n.add_named_child(cmd.desc.group, nt.text, cn)
            } elif cmd.desc.array {
                n.add_child_item(cmd.desc.array, cn)
            } else {
                n.add_child(cmd.desc.name, cn)
            }

            cmd.solve(cn, input, c_has_end)
        }
    }
}

Command({
    name: "package"
    has_brief: true
    has_detail: true
})
Command({
    name: "module"
    children: [
        "lib"
        "exe"
        "otype"
        "callback"
        "func"
        "class"
        "object"
        "var"
        "const"
    ]
})
Command({
    name: "lib"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "exe"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "otype"
    group: "otype"
    has_brief: true
    has_detail: true
    children: [
        "func"
        "var"
        "acc"
        "roacc"
    ]
})
Command({
    name: "func"
    group: "func"
    has_plist: true
    has_brief: true
    has_detail: true
    children: [
        "param"
        "return"
        "throw"
    ]
})
Command({
    name: "class"
    group: "class"
    has_brief: true
    has_detail: true
    children: [
        "enum"
        "bitfield"
        "sfunc"
        "const"
        "svar"
        "sacc"
        "sroacc"
        "func"
        "var"
        "acc"
        "roacc"
        "inherit"
        "class"
        "object"
    ]
})
Command({
    name: "inherit"
    has_type: true
})
Command({
    name: "object"
    group: "object"
    has_brief: true
    has_detail: true
    children: [
        "enum"
        "bitfield"
        "func"
        "var"
        "const"
        "acc"
        "roacc"
        "class"
        "object"
    ]
})
Command({
    name: "callback"
    group: "callback"
    has_brief: true
    has_detail: true
    children: [
        "param"
        "return"
        "throw"
    ]
})
Command({
    name: "sfunc"
    group: "sfunc"
    has_plist: true
    has_brief: true
    has_detail: true
    children: [
        "param"
        "return"
        "throw"
    ]
})
Command({
    name: "var"
    group: "var"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "const"
    group: "const"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "svar"
    group: "svar"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "acc"
    group: "acc"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "roacc"
    group: "acc"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "sacc"
    group: "sacc"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "sroacc"
    group: "sacc"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "param"
    group: "param"
    has_type: true
    has_value: true
    has_brief: true
    has_detail: true
})
Command({
    name: "return"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "throw"
    array: "throw"
    has_type: true
    has_brief: true
    has_detail: true
})
Command({
    name: "br"
    in_line: true
})
Command({
    name: "ol"
    array: "li"
    in_text: true
    children: [
        "li"
        "ol"
        "ul"
    ]
})
Command({
    name: "ul"
    array: "li"
    in_text: true
    children: [
        "li"
        "ol"
        "ul"
    ]
})
Command({
    name: "li"
    array: "li"
    has_brief: true
})
Command({
    name: "code"
    in_text: true
    has_code: true
})
Command({
    name: "enum"
    group: "enum"
    has_brief: true
    has_detail: true
    children: [
        "item"
    ]
})
Command({
    name: "bitfield"
    group: "bitfield"
    has_brief: true
    has_detail: true
    children: [
        "item"
    ]
})
Command({
    name: "item"
    group: "item"
    has_brief: true
    has_detail: true
})
Command({
    name: "option"
    children: [
        "oi"
    ]
})
Command({
    name: "oi"
    group: "oi"
    has_brief: true
    has_detail: true
    has_type: true
})
