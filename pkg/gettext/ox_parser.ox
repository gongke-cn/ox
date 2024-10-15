ref "std/lang"
ref "std/ast"
ref "./log"

solve_ast: func(n, cb) {
    if !(n instof Ast) {
        return
    }

    case n.type {
    Ast.value {
        if n.local {
            cb(n.value, n.loc.first_line)
        }
    }
    Ast.string {
        if n.local {
            s = n.templ.$iter().map(func(item) {
                if item {
                    return item.replace(/[~$]/, func(m) {
                        case m.$to_str() {
                        "~" {
                            return "~0"
                        }
                        "$" {
                            return "~1"
                        }
                        }
                    })
                }
            }).$to_str("$")

            cb(s, n.loc.first_line)
        }
    }
    }

    for Object.entries(n) as [k, v] {
        if k != "outer" {
            if v instof Array {
                for v as item {
                    solve_ast(item, cb)
                }
            } else {
                solve_ast(v, cb)
            }
        }
    }
}

public ox_parse: func(fn, cb) {
    ast = OX.ast_from_file(fn)

    solve_ast(ast, cb)
}
