ref "std/io"
ref "./log"

//Escape characters conversion.
escape: func(text, in_html) {
    if in_html {
        return text.replace(/['"><&]/, func(s) {
            case s.$to_str() {
            "'" {
                return "&apos;"
            }
            "\"" {
                return "&quot;"
            }
            "<" {
                return "&lt;"
            }
            ">" {
                return "&gt;"
            }
            "&" {
                return "&amp;"
            }
            * {
                return s;
            }
        }
    })
    } else {
        return text.replace(/[\\`*_{}()\[\]+\-\.!]/, func(s) {
            return "\\{s}"
        })
    }
}

//Get brief text.
brief: func(n, in_html) {
    if !n?.length{
        return null
    }

    sb = String.Builder()

    for n as item {
        if String.is(item) {
            sb.append(escape(item, in_html))
        } else {
            case item.node_type {
            "br" {
                sb.append("<br/>")
            }
            }
        }
    }

    return sb.$to_str()
}

//Get brief description.
brief_section: func(pre, n) {
    if n?.brief.length {
        return ''
{{pre}} SYNOPSIS
{{brief(n.brief)}}
        ''
    }
}

//Get detail text.
detail: func(n, in_html) {
    if !n?.length{
        return null
    }

    sb = String.Builder()

    for n as item {
        if String.is(item) {
            sb.append(escape(item, in_html))
        } else {
            case item.node_type {
            "br" {
                sb.append("<br/>")
            }
            "ol" {
                add_ol(sb, item)
            }
            "ul" {
                add_ul(sb, item)
            }
            "code" {
                sb.append("<pre>")
                sb.append(item.code)
                sb.append("</pre>")
            }
            }
        }
    }

    return sb.$to_str()
}

//Add a list item.
add_li: func(sb, li) {
    sb.append("<li>")
    sb.append(brief(li.brief, true))
    sb.append("</li>")
}

//Add an unordered list.
add_ul: func(sb, ul) {
    sb.append("<ul>")

    for ul.li as li {
        case li.node_type {
        "li" {
            add_li(sb, li)
        }
        "ul" {
            add_ul(sb, li)
        }
        "ol" {
            add_ol(sb, li)
        }
        }
    }

    sb.append("</ul>")
}

//Add a ordered list.
add_ol: func(sb, ol) {
    sb.append("<ol>")

    for ol.li as li {
        case li.node_type {
        "li" {
            add_li(sb, li)
        }
        "ul" {
            add_ul(sb, li)
        }
        "ol" {
            add_ol(sb, li)
        }
        }
    }

    sb.append("</ol>")
}

//Get detail description.
detail_section: func(pre, n) {
    if n?.detail.length {
        return ''
{{pre}} DESCRIPTION
{{detail(n.detail)}}
        ''
    }
}

//Generate index of the document.
gen_index: func(doc, pn, dir) {
    log.debug("gen index")

    #file = File("{dir}/{pn}.md", "wb")

    sb = String.Builder()
    for Object.entries(doc.module) as [name, mod] {
        if mod.lib?.brief {
            desc = ": {brief(mod.lib.brief)}"
        } elif mod.exe?.brief {
            desc = ": {brief(mod.exe.brief)}"
        } else {
            desc = null
        }

        sb.append("+ [{name}]({pn}_{name}.md){desc}\n")
    }

    file.puts(''
# PACKAGE {{pn}}
{{brief_section("##", doc)}}
{{detail_section("##", doc)}}
## MODULES
{{sb.$to_str()}}
    '')
}

//Add an enumeration.
add_enum: func(pre, sb, detail_sb, scope, name, def) {
    if def.brief {
        desc = ": {brief(def.brief)}"
    }
    sb.append("+ [{name}](#{scope}{name}){desc}\n")

    if def.node_type == "bitfield" {
        type = "bitfield"
    } else {
        type = "enum"
    }

    npre = pre + "#"

    item_sb = String.Builder()
    for Object.entries(def.item) as [iname, item] {
        if item.brief?.length {
            desc = ": {brief(item.brief)}"
        }

        item_sb.append("+ **{iname}**{desc}\n")

        if item.detail?.length {
            item_sb.append("\n    ")
            item_sb.append(detail(item.detail))
            item_sb.append("\n")
        }
    }

    detail_sb.append(''
---
{{pre}} <a id="{{scope}}{{name}}">{{scope}}{{name}}</a>
{{type}} **{{name}}**
{{brief_section(npre, def)}}
{{detail_section(npre, def)}}
{{npre}} ITEMS
{{item_sb.$to_str()}}
    '')
}

//Add a class.
add_class: func(pre, sb, detail_sb, scope, name, cdef) {
    if cdef.brief {
        desc = ": {brief(cdef.brief)}"
    }
    sb.append("+ [{name}](#{scope}{name}){desc}\n")

    npre = pre + "#"

    if cdef.inherit?.type {
        inherit = "{npre} INHERIT\n{cdef.inherit.type}"
    }

    detail_sb.append(''
---
{{pre}} <a id="{{scope}}{{name}}">{{scope}}{{name}}</a>
class **{{name}}**
{{inherit}}
{{brief_section(npre, cdef)}}
{{detail_section(npre, cdef)}}

    '')

    item_sb = String.Builder()
    c_scope = "{scope}{name}."
    inf_scope = "{scope}{name}.$inf."

    if cdef.enum {
        detail_sb.append("{npre} ENUMERATIONS\n")

        for Object.entries(cdef.enum) as [en, edef] {
            add_enum(npre, detail_sb, item_sb, c_scope, en, edef)
        }
    }

    if cdef.bitfield {
        detail_sb.append("{npre} BITFIELDS\n")

        for Object.entries(cdef.bitfield) as [bn, bdef] {
            add_enum(npre, detail_sb, item_sb, c_scope, bn, bdef)
        }
    }

    if cdef.const {
        detail_sb.append("{npre} CONSTANT PROPERTIES\n")

        for Object.entries(cdef.const) as [cn, def] {
            add_var(detail_sb, cn, def)
        }
    }

    if cdef.svar {
        detail_sb.append("{npre} STATIC PROPERTIES\n")

        for Object.entries(cdef.svar) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
    }

    if cdef.sacc {
        detail_sb.append("{npre} STATIC ACCESSORS\n")

        for Object.entries(cdef.acc) as [an, adef] {
            add_acc(detail_sb, an, adef)
        }
    }

    if cdef.sfunc {
        detail_sb.append("{npre} STATIC METHODS\n")

        for Object.entries(cdef.sfunc) as [fname, fdef] {
            add_func(npre, detail_sb, item_sb, c_scope, fname, fdef)
        }
    }

    if cdef.class {
        detail_sb.append("{npre} CLASSES\n")

        for Object.entries(cdef.class) as [cn, def] {
            add_class(npre, detail_sb, item_sb, c_scope, cn, def)
        }
    }

    if cdef.object {
        detail_sb.append("{npre} OBJECTS\n")

        for Object.entries(cdef.object) as [on, odef] {
            add_object(npre, detail_sb, item_sb, c_scope, on, odef)
        }
    }

    if cdef.var {
        detail_sb.append("{npre} PROPERTIES\n")

        for Object.entries(cdef.var) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
    }

    if cdef.acc {
        detail_sb.append("{npre} ACCESSORS\n")

        for Object.entries(cdef.acc) as [an, adef] {
            add_acc(detail_sb, an, adef)
        }
    }

    if cdef.func {
        detail_sb.append("{npre} METHODS\n")

        for Object.entries(cdef.func) as [fname, fdef] {
            add_func(npre, detail_sb, item_sb, inf_scope, fname, fdef)
        }
    }
    detail_sb.append("\n")

    detail_sb.append(item_sb.$to_str())
    detail_sb.append("\n")
}

//Add an object.
add_object: func(pre, sb, detail_sb, scope, name, odef) {
    if odef.brief {
        desc = ": {brief(odef.brief)}"
    }
    sb.append("+ [{name}](#{scope}{name}){desc}\n")

    npre = pre + "#"

    detail_sb.append(''
---
{{pre}} <a id="{{scope}}{{name}}">{{scope}}{{name}}</a>
{{brief_section(npre, odef)}}
{{detail_section(npre, odef)}}

    '')

    item_sb = String.Builder()
    o_scope = "{scope}{name}."

    if odef.enum {
        detail_sb.append("{npre} ENUMERATIONS\n")

        for Object.entries(odef.enum) as [en, edef] {
            add_enum(npre, detail_sb, item_sb, o_scope, en, edef)
        }
    }

    if odef.bitfield {
        detail_sb.append("{npre} BITFIELDS\n")

        for Object.entries(odef.bitfield) as [bn, bdef] {
            add_enum(npre, detail_sb, item_sb, o_scope, bn, bdef)
        }
    }

    if odef.const {
        detail_sb.append("{npre} CONSTANT PROPERTIES\n")

        for Object.entries(odef.const) as [cn, cdef] {
            add_var(detail_sb, cn, cdef)
        }
    }

    if odef.var {
        detail_sb.append("{npre} PROPERTIES\n")

        for Object.entries(odef.var) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
    }

    if odef.func {
        detail_sb.append("{npre} METHODS\n")

        for Object.entries(odef.func) as [fname, fdef] {
            add_func(npre, detail_sb, item_sb, o_scope, fname, fdef)
        }
    }

    if odef.class {
        detail_sb.append("{npre} CLASSES\n")

        for Object.entries(odef.class) as [cn, def] {
            add_class(npre, detail_sb, item_sb, c_scope, cn, def)
        }
    }

    if odef.object {
        detail_sb.append("{npre} OBJECTS\n")

        for Object.entries(odef.object) as [on, def] {
            add_object(npre, detail_sb, item_sb, c_scope, on, def)
        }
    }

    detail_sb.append("\n")
    detail_sb.append(item_sb.$to_str())
    detail_sb.append("\n")
}

//Add function.
add_func: func(pre, sb, detail_sb, scope, name, fn) {
    if fn.brief {
        desc = ": {brief(fn.brief)}"
    }

    npre = pre + "#"

    args = fn.plist

    if fn.param {
        if !args {
            args = Object.keys(fn.param).$to_str(", ")
        }

        pdesc_sb = String.Builder()
        for Object.entries(fn.param) as [pname, param] {
            pdesc_sb.append("+ **{pname}**")

            if param.type {
                pdesc_sb.append(" \{**{param.type}**\}")
            }

            if param.value {
                pdesc_sb.append(" = {param.value}")
            }

            pdesc_sb.append(": {brief(param.brief)}\n")

            if param.detail?.length {
                pdesc_sb.append("\n    {detail(param.detail)}\n\n")
            }
        }

        params = ''
{{npre}} PARAMETERS
{{pdesc_sb.$to_str()}}
        ''
    }

    if fn.return {
        retval = "{npre} RETURN\n"

        if fn.return.type {
            retval += "**{fn.return.type}**: "
        }

        retval += "{brief(fn.return.brief)}\n"

        if fn.return.detail?.length {
            retval += "\n{detail(fn.return.detail)}\n\n"
        }
    }

    if fn.throw {
        err_sb = String.Builder()

        for fn.throw as err {
            err_sb.append("+ ")

            if err.type {
                err_sb.append("**{err.type}**: ")
            }

            err_sb.append(brief(err.brief))
            err_sb.append("\n")

            if err.detail?.length {
                err_sb.append("\n    ")
                err_sb.append(detail(err.detail))
                err_sb.append("\n")
            }
        }

        errors = ''
{{npre}} ERRORS
{{err_sb.$to_str()}}
        ''
    }

    sb.append("+ [{name}](#{scope}{name})({args}){desc}\n")

    if fn.node_type == "sfunc" {
        is_static = "static "
    }

    detail_sb.append(''
---
{{pre}} <a id="{{scope}}{{name}}">{{scope}}{{name}}</a>
{{is_static}}**{{name}}({{args}})**
{{brief_section(npre, fn)}}
{{detail_section(npre, fn)}}
{{params}}
{{retval}}
{{errors}}

    '')
}

//Add variable.
add_var: func(sb, name, vdef) {
    sb.append("+ **{name}**")

    if vdef.type {
        sb.append(" \{**{vdef.type}**\}")
    }

    if vdef.brief?.length {
        sb.append(": ")
        sb.append(brief(vdef.brief))
        sb.append("\n")
    }

    if vdef.detail?.length {
        sb.append("\n    ")
        sb.append(detail(vdef.detail))
        sb.append("\n")
    }

    sb.append("\n")
}

//Add an accessor
add_acc: func(sb, name, adef) {
    sb.append("+ ")

    if adef.node_type == "roacc" || adef.node_type == "sroacc" {
        sb.append("readonly ")
    }

    sb.append("**{name}**")

    if adef.type {
        sb.append(" \{**{adef.type}**\}")
    }

    if adef.brief?.length {
        sb.append(": ")
        sb.append(brief(adef.brief))
        sb.append("\n")
    }

    if adef.detail?.length {
        sb.append("\n    ")
        sb.append(detail(adef.detail))
        sb.append("\n")
    }

    sb.append("\n")
}

//Generate document file.
gen_doc: func(pn, name, doc, dir) {
    log.debug("gen {name} document")

    #file = File("{dir}/{pn}_{name}.md", "wb")

    if doc.exe {
        type = "EXECUTABLE"
        node = doc.exe
    } else {
        type = "LIBRARY"
        node = doc.lib
    }

    sb = String.Builder()
    detail_sb = String.Builder()

    if doc.option {
        sb.append("## OPTIONS\n")

        for Object.entries(doc.option.oi) as [optn, optd] {
            if optd.type {
                atype = " {optd.type}"
            }
            if optd.brief {
                bdesc = ": {brief(optd.brief)}"
            }
            sb.append("+ **{optn}{atype}**{bdesc}\n")
            
            if optd.detail {
                sb.append("\n{detail(opt.detail)}\n")
            }
        }
    }

    if doc.otype {
        sb.append("## OBJECT TYPES\n")

        for Object.entries(doc.otype) as [tn, ot] {
            add_object("##", sb, detail_sb, null, tn, ot)
        }
    }

    if doc.callback {
        sb.append("## CALLBACK TYPES\n")

        for Object.entries(doc.callback) as [cn, cb] {
            add_func("##", sb, detail_sb, null, cn, cb)
        }
    }

    if doc.const {
        sb.append("## CONSTANTS\n")

        for Object.entries(doc.const) as[cn, cdef] {
            add_var(sb, cn, cdef)
        }
    }

    if doc.var {
        sb.append("## VARIABLES\n")

        for Object.entries(doc.var) as[vn, vdef] {
            add_var(sb, vn, vdef)
        }
    }

    if doc.class {
        sb.append("## CLASSES\n")

        for Object.entries(doc.class) as [cn, cdef] {
            add_class("##", sb, detail_sb, null, cn, cdef)
        }
    }

    if doc.object {
        sb.append("## OBJECTS\n")

        for Object.entries(doc.object) as [on, odef] {
            add_object("##", sb, detail_sb, null, on, odef)
        }
    }

    if doc.func {
        sb.append("## FUNCTIONS\n")

        for Object.entries(doc.func) as [cn, fn] {
            add_func("##", sb, detail_sb, null, cn, fn)
        }
    }

    file.puts(''
# {{type}} {{pn}}:{{name}}
{{brief_section("##", node)}}
{{detail_section("##", node)}}
{{sb.$to_str()}}

{{detail_sb.$to_str()}}
    '')
}

//Generate markdown document.
public gen_markdown_doc: func(doc, pn, dir) {
    if doc.name {
        pn = doc.name
    }

    gen_index(doc, pn, dir)

    for Object.entries(doc.module) as [name, mod] {
        gen_doc(pn, name, mod, dir)
    }
}
