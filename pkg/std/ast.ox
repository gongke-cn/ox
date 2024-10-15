ref "./log"
ref "./text_util"
ref "./char"
ref "./ast_types"

log: Log("Ast")
log.level = Log.WARNING

//Check if the value is an identifier.
is_id: func(s) {
    if !String.is(s) {
        return false
    }

    if s.length == 0 {
        return false
    }

    c = s.char_at(0)
    if !isalpha(c) && (c != '$') && (c != '_') {
        return false
    }

    for s.chars() as c {
        if !isalnum(c) && (c != '$') && (c != '_') {
            return false
        }
    }

    return true
}

//Add AST functions.
Ast.$inf.{
    //Convert the AST node to string.
    $to_str: func() {
        case this.type {
        Ast.if {
            stmt = "if {this.items.$iter().$to_str(" elif ")}"

            if this.else {
                stmt += "else {this.else}"
            }

            return stmt
        }
        Ast.do_while {
            return "do {this.block} {this.expr};"
        }
        Ast.while {
            return "while {this.expr} {this.block}"
        }
        Ast.sched {
            if this.block {
                return "sched {this.block}"
            } else {
                return "sched;"
            }
        }
        Ast.for {
            return "for {this.init}; {this.cond}; {this.step} {this.block}"
        }
        Ast.for_as {
            return "for {this.right} as {this.left} {this.block}"
        }
        Ast.case {
            return "case {this.expr} \{{this.items.$iter().$to_str("")}\}"
        }
        Ast.case_func {
            return "({this.expr})"
        }
        Ast.try {
            stmt = "try \{{this.block}\}"

            if this.catch {
                stmt += " catch {this.catch.expr} {this.catch.block}"
            }

            if this.finally {
                stmt += " finally {this.block}"
            }

            return stmt
        }
        Ast.return {
            if this.expr {
                return "return {this.expr};"
            } else {
                return "return;"
            }
        }
        Ast.throw {
            return "throw {this.expr};"
        }
        Ast.break {
            return "break;"
        }
        Ast.continue {
            return "continue;"
        }
        Ast.block {
            return "\{{this.items?.$iter().$to_str("; ")}\}"
        }
        Ast.value {
            if String.is(this.value) {
                return "\"{c_escape(this.value)}\""
            } else {
                return this.value.$to_str()
            }
        }
        Ast.id {
            if this.outer {
                return "@{this.value}"
            } elif this.hash {
                return "#{this.value}"
            } else {
                return this.value
            }
        }
        Ast.string {
            sb = String.Builder()

            if this.local {
                sb.append_char("L")
            }

            sb.append_char('\"')

            id = 0
            for this.templ as item {
                sb.append(c_escape(item))
                if id != this.templ.length - 1 {
                    sb.append("\{{this.items[id]}\}")
                }
                id += 1
            }

            sb.append_char('\"')

            return sb.$to_str()
        }
        Ast.format {
            s = "{this.expr}!"

            fmt = this.format
            if fmt & 0x1000000 {
                s += "-"
            } elif fmt & 0x2000000 {
                s += "0"
            }

            s += (fmt >> 8) & 0xff
            s += "."
            s += (fmt >> 16) & 0xff
            s += "sduoxfenc".slice(fmt & 0xff, 1)

            return s
        }
        Ast.unary_expr {
            return "{this.operator}{this.operand1}"
        }
        Ast.binary_expr {
            if this.operator.type == Ast.get && this.operand1.type == Ast.curr_object {
                return this.operand2.$to_str()
            }

            if this.operator.type == Ast.get {
                if this.operand2.type != Ast.value || !is_id(this.operand2.value) {
                    if this.ques_dst {
                        return "{this.operand1}?[{this.operand2}]"
                    } else {
                        return "{this.operand1}[{this.operand2}]"
                    }
                } else {
                    if this.ques_dst {
                        return "{this.operand1}?.{this.operand2.value}"
                    } else {
                        return "{this.operand1}.{this.operand2.value}"
                    }
                }
            }

            return "{this.operand1} {this.operator} {this.operand2}"
        }
        Ast.assi {
            return "{this.left} {this.operator}= {this.right}"
        }
        Ast.rev_assi {
            return "{this.right} => {this.left}"
        }
        Ast.func {
            return "func ({this.params}) {this.block}"
        }
        Ast.class {
            sb = String.Builder()
            sb.append("class")

            if c.parents {
                sb.append(" {c.parents.$iter().$to_str(", ")}")
            }

            sb.append(" \{{this.props.$iter().$to_str(" ")}\}")

            return sb.$to_str()
        }
        Ast.ref {
            return "ref \"{c_escape(this.file)}\" \{this.items.$iter().$to_str(", ")\}"
        }
        Ast.ref_item {
            if this.orig {
                r = this.orig.value
            }

            if this.name {
                r += this.name.value
            }

            return r
        }
        Ast.expr_block {
            return "{this.expr} {this.block}"
        }
        Ast.exprs_block {
            return "{this.conds.$iter().to_str(", ")} {this.block}"
        }
        Ast.all {
            return "*"
        }
        Ast.skip {
            return ""
        }
        Ast.plus {
            return "+"
        }
        Ast.minus {
            return "-"
        }
        Ast.typeof {
            return "typeof "
        }
        Ast.bit_rev {
            return "~"
        }
        Ast.logic_not {
            return "!"
        }
        Ast.global {
            return "global "
        }
        Ast.owned {
            return "owned "
        }
        Ast.parenthese {
            return "({this.expr})"
        }
        Ast.spread {
            return "...{this.expr}"
        }
        Ast.get_ptr {
            return "&"
        }
        Ast.get_value {
            return "*"
        }
        Ast.get {
            return "."
        }
        Ast.exp {
            return "**"
        }
        Ast.add {
            return "+"
        }
        Ast.sub {
            return "-"
        }
        Ast.match {
            return "~"
        }
        Ast.mul {
            return "*"
        }
        Ast.div {
            return "/"
        }
        Ast.mod {
            return "%"
        }
        Ast.shl {
            return "<<"
        }
        Ast.shr {
            return ">>"
        }
        Ast.ushr {
            return ">>>"
        }
        Ast.lt {
            return "<"
        }
        Ast.gt {
            return ">"
        }
        Ast.le {
            return "<="
        }
        Ast.ge {
            return ">="
        }
        Ast.instof {
            return "instof "
        }
        Ast.eq {
            return "=="
        }
        Ast.ne {
            return "!="
        }
        Ast.none {
            return ""
        }
        Ast.bit_or {
            return "|"
        }
        Ast.bit_xor {
            return "^"
        }
        Ast.bit_and {
            return "&"
        }
        Ast.logic_and {
            return "&&"
        }
        Ast.logic_or {
            return "||"
        }
        Ast.comma {
            return this.items?.$iter().$to_str(", ")
        }
        Ast.call {
            if this.args?.items[0].type == Ast.curr_object {
                return "{this.expr.block.items[0].expr}"
            }

            if this.ques_dst {
                return "{this.expr}?({this.args})"
            } else {
                return "{this.expr}({this.args})"
            }
        }
        Ast.args {
            return this.items.$iter().$to_str(", ")
        }
        Ast.params {
            if this.items {
                return this.items.$iter().$to_str(", ")
            } else {
                return ""
            }
        }
        Ast.arg {
            return "${this.id}"
        }
        Ast.rest {
            return "...{this.pattern}"
        }
        Ast.item_pattern {
            p = this.pattern.$to_str()

            if this.expr {
                p += " = {this.expr}"
            }

            return p
        }
        Ast.prop_pattern {
            p = "{this.name}: {this.pattern}"

            if this.expr {
                p += " = {this.expr}"
            }

            return p;
        }
        Ast.array_pattern {
            return "[{this.items?.$iter().$to_str(", ")}]"
        }
        Ast.object_pattern {
            return "\{{this.props?.$iter().$to_str(", ")}\}"
        }
        Ast.prop {
            return "{this.name}: {this.expr}"
        }
        Ast.curr_object {
            return ""
        }
        Ast.expr_name {
            return "[{this.expr}]"
        }
        Ast.const {
            c = this.name.$to_str()

            if this.expr {
                c += " : {this.expr}"
            }

            return c
        }
        Ast.var {
            v = this.name.$to_str()

            if this.expr {
                v += " = {this.expr}"
            }

            return v
        }
        Ast.method {
            return "{this.name}({this.expr.params}) {this.expr.block}"
        }
        Ast.accessor {
            a = "{this.name} {this.get.block}"

            if a.set {
                a += " {this.set.params} {this.set.block}"
            }

            return a
        }
        Ast.array {
            return "[{this.items?.$iter().$to_str(", ")}]"
        }
        Ast.object {
            return "\{{this.props?.$iter().$to_str(", ")}\}"
        }
        Ast.array_append {
            return "{this.expr}.[{this.items?.$iter().$to_str(", ")}]"
        }
        Ast.object_set {
            return "{this.expr}.\{{this.props?.$iter().$to_str(", ")}\}"
        }
        Ast.decl {
            return ""
        }
        Ast.this {
            return "this"
        }
        Ast.argv {
            return "argv"
        }
        Ast.enum {
            e = "enum"

            if this.name {
                e += " {this.name.value}"
            }

            e += " \{{this.items.$iter().map(($.value)).$to_str(", ")}\}"

            return e;
        }
        Ast.bitfield {
            e = "bitfield"

            if this.name {
                e += " {this.name.value}"
            }

            e += " \{{this.items.$iter().map(($.value)).$to_str(", ")}\}"

            return e;
        }
        Ast.doc {
            return ""
        }
        }
    }
}
