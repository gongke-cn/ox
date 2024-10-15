ref "std/io"
ref "std/lang"
ref "std/path"
ref "std/char"
ref "json"
ref "./document"
ref "./error"
ref "./log"

//C input.
CInput: class {
    //Initialize a C input.
    $init(filename) {
        this.file = File(filename)
        this.cached = []
        this.line = 1
        this.column = 0
        this.filename = filename
    }

    //Close the C input.
    $close() {
        this.file.$close()
    }

    //Get a character from the C input.
    getc() {
        if this.cached.length {
            c = this.cached.pop()
        } else {
            c = this.file.getc()
        }

        if c != null {
            if this.last_is_nl {
                this.line += 1
                this.column = 0
                this.last_is_nl = false
            }

            this.column += 1

            if c == '\n' {
                this.last_is_nl = true
            }
        }

        return c
    }

    //Push back a character to the input.
    ungetc(c) {
        if c != null {
            this.cached.push(c)

            this.column -= 1
            this.last_is_nl = false
        }
    }

    //Parse the comment block.
    comment_block(dr, mod) {
        c = this.getc()
        if c == '?' {
            is_doc = true
            sb = String.Builder()
            sb.append("   ")

            line = this.line
            col = this.column - 2
        } else {
            this.ungetc()
        }

        while true {
            c = this.getc()
            if c == '*' {
                nc = this.getc()
                if nc == '/' {
                    break
                } else {
                    this.ungetc(nc)
                }
            } elif c == null {
                break
            }

            if is_doc {
                if new_line {
                    if c == '*' {
                        nc = this.getc()
                        if nc != '?' {
                            this.ungetc(nc)
                        } else {
                            sb.append_char(' ')
                        }

                        new_line = false
                        sb.append_char(' ')
                    } elif !isspace(c) {
                        new_line = false
                        sb.append_char(c)
                    } else {
                        sb.append_char(' ')
                    }
                } else {
                    sb.append_char(c)
                }

                if c == '\n' {
                    new_line = true
                }
            }
        }

        if is_doc {
            mod.add_doc(dr, line, col, sb.$to_str())
        }
    }

    //Parse the comment line.
    comment_line(dr, mod) {
        c = this.getc()
        if c == '?' {
            is_doc = true
            sb = String.Builder()
            sb.append("   ")

            line = this.line
            col = this.column - 2
        } else {
            this.ungetc()
        }

        while true {
            c = this.getc()
            if c == '\n' || c == null {
                break
            }

            if is_doc {
                sb.append_char(c)
            }
        }

        if is_doc {
            mod.add_doc(dr, line, col, sb.$to_str())
        }
    }

    //Parse the input.
    parse(dr, mod) {
        while true {
            c = this.getc()
            if c == null {
                break
            }

            if c == '/' {
                nc = this.getc()
                if nc == '*' {
                    this.comment_block(dr, mod)
                } elif nc == '/' {
                    this.comment_line(dr, mod)
                } else {
                    this.ungetc(nc)
                }
            }
        }
    }
}

//Parse the C file.
parse_c: func(dr, mod, filename) {
    log.debug("parse c")

    #input = CInput(filename)

    input.parse(dr, mod)
}

//Parse the OX file.
parse_ox: func(dr, mod, filename) {
    log.debug("parse ox")

    //Parse the source to AST.
    ast = OX.ast_from_file(filename)

    mod.set_ast(ast)

    //Add global document items.
    if ast.doc {
        for ast.doc as item {
            mod.add_doc(dr, item.loc.first_line, item.loc.first_column, item.value)
        }
    }

    //Add AST related document items.
    for ast.funcs[0].block.items as stmt {
        doc = stmt.doc
        if doc {
            if stmt.type != Ast.assi {
                continue
            }

            op = stmt.operator
            left = stmt.left
            right = stmt.right

            if op.type != Ast.none {
                continue
            }

            if left.type != Ast.id {
                continue
            }

            name = left.value
            mod.add_ast_doc(name, doc, right)
        }
    }
}

//Parse the input file.
public parse: func(doc, filename) {
    mod = Module(filename)

    set_filename(filename)

    if mod.format == "ox" {
        parse_ox(doc, mod, filename)
    } else {
        parse_c(doc, mod, filename)
    }

    if mod.lib?.type {
        mod.name = mod.lib.type
    } elif mod.exe?.type {
        mod.name = mod.exe.type
    }

    if mod.valid {
        doc.add_named_child("module", mod.name, mod)
        doc.valid = true
    }
}
