ref "std/io"
ref "std/text_util"

//Get brief text.
brief: func(n) {
    if !n?.length{
        return null
    }

    sb = String.Builder()

    for n as item {
        if String.is(item) {
            sb.append(html_escape(item))
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
brief_section: func(tag, n) {
    if n?.brief.length {
        return ''
<{{tag}}>SYNOPSIS</{{tag}}>
{{brief(n.brief)}}
        ''
    }
}

//Get detail text.
detail: func(n) {
    if !n?.length{
        return null
    }

    sb = String.Builder()

    for n as item {
        if String.is(item) {
            sb.append(html_escape(item))
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
    sb.append(brief(li.brief))
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
detail_section: func(tag, n) {
    if n?.detail.length {
        return ''
<{{tag}}>DESCRIPTION</{{tag}}>
{{detail(n.detail)}}
        ''
    }
}

//Generate index document.
gen_index: func(doc, pn, dir) {
    #file = File("{dir}/{pn}.html", "wb")

    sb = String.Builder()

    for Object.entries(doc.module) as [name, mod] {
        if mod.lib?.brief {
            desc = ": {brief(mod.lib.brief)}"
        } elif mod.exe?.brief {
            desc = ": {brief(mod.exe.brief)}"
        } else {
            desc = null
        }

        sb.append("<li><a href=\"{pn}_{name}.html\">{name}</a>{desc}</li>\n")
    }

    file.puts(''
<html>
<head>
    <meta charset="UTF-8">
    <title>PACKAGE {{pn}}</title>
</head>
<h1>PACKAGE {{pn}}</h1>
{{brief_section("h2", doc)}}
{{detail_section("h2", doc)}}
<h2>MODULES</h2>
<ul>
{{sb.$to_str()}}
</ul>
</html>
    '')
}

//Get the next header tag.
next_htag: func(tag) {
    case tag {
    "h1" {
        return "h2"
    }
    "h2" {
        return "h3"
    }
    "h3" {
        return "h4"
    }
    "h4" {
        return "h5"
    }
    * {
        return "h6"
    }
    }
}

//Add an enumeration.
add_enum: func(tag, sb, detail_sb, scope, name, def) {
    if def.brief {
        desc = ": {brief(def.brief)}"
    }
    sb.append("<li><a href=\"#{scope}{name}\">{name}</a>{desc}</li>\n")

    if def.node_type == "bitfield" {
        type = "bitfield"
    } else {
        type = "enum"
    }

    item_sb = String.Builder()
    for Object.entries(def.item) as [iname, item] {
        if item.brief?.length {
            desc = ": {brief(item.brief)}"
        }

        item_sb.append("<li><b>{iname}</b>{desc}")

        if item.detail?.length {
            item_sb.append("</br>")
            item_sb.append(detail(item.detail))
        }

        item_sb.append("</li>\n")
    }

    ntag = next_htag(tag)

    detail_sb.append(''
<hr/>
<{{tag}} id="{{scope}}{{name}}">{{scope}}{{name}}</{{tag}}>
{{type}} <b>{{name}}</b>
{{brief_section(ntag, def)}}
{{detail_section(ntag, def)}}
<{{ntag}}>ITEMS</{{ntag}}>
<ul>
{{item_sb.$to_str()}}
</ul>
    '')
}

//Add a class.
add_class: func(tag, sb, detail_sb, scope, name, cdef) {
    if cdef.brief {
        desc = ": {brief(cdef.brief)}"
    }
    sb.append("<li><a href=\"#{scope}{name}\">{name}</a>{desc}</li>\n")

    ntag = next_htag(tag)

    if cdef.inherit?.type {
        inherit = "<{ntag}>INHERIT</{ntag}>\n{cdef.inherit.type}\n"
    }

    detail_sb.append(''
<hr/>
<{{tag}} id="{{scope}}{{name}}">{{scope}}{{name}}</{{tag}}>
class <b>{{name}}</b>
{{inherit}}
{{brief_section(ntag, cdef)}}
{{detail_section(ntag, cdef)}}

    '')

    item_sb = String.Builder()
    c_scope = "{scope}{name}."
    inf_scope = "{scope}{name}.$inf."

    if cdef.enum {
        detail_sb.append("<{ntag}>ENUMERATIONS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.enum) as [en, edef] {
            add_enum(ntag, detail_sb, item_sb, c_scope, en, edef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.bitfield {
        detail_sb.append("<{ntag}>BITFIELDS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.bitfield) as [bn, bdef] {
            add_enum(ntag, detail_sb, item_sb, c_scope, bn, bdef)
        }
        detail_sb.append("</ul>\n")
    }

     if cdef.const {
        detail_sb.append("<{ntag}>CONSTANT PROPERTIES</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.const) as [cn, def] {
            add_var(detail_sb, cn, def)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.svar {
        detail_sb.append("<{ntag}>STATIC PROPERTIES</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.svar) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
        detail_sb.append("</ul>\n")
    }

     if cdef.sacc {
        detail_sb.append("<{ntag}>STATIC ACCESSORS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.acc) as [an, adef] {
            add_acc(detail_sb, an, adef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.sfunc {
        detail_sb.append("<{ntag}>STATIC METHODS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.sfunc) as [fname, fdef] {
            add_func(ntag, detail_sb, item_sb, c_scope, fname, fdef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.class {
        detail_sb.append("<{ntag}>CLASSES</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.class) as [cn, def] {
            add_class(ntag, detail_sb, item_sb, c_scope, cn, def)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.object {
        detail_sb.append("<{ntag}>OBJECTS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.object) as [on, odef] {
            add_object(ntag, detail_sb, item_sb, c_scope, on, odef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.var {
        detail_sb.append("<{ntag}>PROPERTIES</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.var) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.acc {
        detail_sb.append("<{ntag}>ACCESSORS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.acc) as [an, adef] {
            add_acc(detail_sb, an, adef)
        }
        detail_sb.append("</ul>\n")
    }

    if cdef.func {
        detail_sb.append("<{ntag}>METHODS</{ntag}>\n")
        detail_sb.append("<ul>\n")
        for Object.entries(cdef.func) as [fname, fdef] {
            add_func(ntag, detail_sb, item_sb, inf_scope, fname, fdef)
        }
        detail_sb.append("</ul>\n")
    }

    detail_sb.append(item_sb.$to_str())
}

//Add an object.
add_object: func(tag, sb, detail_sb, scope, name, odef) {
    if odef.brief {
        desc = ": {brief(odef.brief)}"
    }
    sb.append("<li><a href=\"#{scope}{name}\">{name}</a>{desc}</li>\n")

    ntag = next_htag(tag)

    detail_sb.append(''
<hr/>
<{{tag}} id="{{scope}}{{name}}">{{scope}}{{name}}</{{tag}}>
{{brief_section(ntag, odef)}}
{{detail_section(ntag, odef)}}

    '')

    item_sb = String.Builder()
    o_scope = "{scope}{name}."

    if odef.enum {
        detail_sb.append("<{ntag}>ENUMERATIONS</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.enum) as [en, edef] {
            add_enum(ntag, detail_sb, item_sb, o_scope, en, edef)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.bitfield {
        detail_sb.append("<{ntag}>BITFIELDS</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.bitfield) as [bn, bdef] {
            add_enum(ntag, detail_sb, item_sb, o_scope, bn, bdef)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.const {
        detail_sb.append("<{ntag}>CONSTANT PROPERTIES</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.const) as [cn, cdef] {
            add_var(detail_sb, cn, cdef)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.var {
        detail_sb.append("<{ntag}>PROPERTIES</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.var) as [vn, vdef] {
            add_var(detail_sb, vn, vdef)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.func {
        detail_sb.append("<{ntag}>METHODS</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.func) as [fname, fdef] {
            add_func(ntag, detail_sb, item_sb, o_scope, fname, fdef)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.class {
        detail_sb.append("<{ntag}>CLASSES</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.class) as [cn, def] {
            add_class(ntag, detail_sb, item_sb, c_scope, cn, def)
        }
        detail_sb.append("</ul\n>")
    }

    if odef.object {
        detail_sb.append("<{ntag}>OBJECTS</{ntag}>\n")
        detail_sb.append("<ul\n>")
        for Object.entries(odef.object) as [on, def] {
            add_object(ntag, detail_sb, item_sb, c_scope, on, def)
        }
        detail_sb.append("</ul\n>")
    }

    detail_sb.append(item_sb.$to_str())
}

//Add function.
add_func: func(tag, sb, detail_sb, scope, name, fn) {
    if fn.brief {
        desc = ": {brief(fn.brief)}"
    }

    ntag = next_htag(tag)

    args = fn.plist

    if fn.param {
        if !args {
            args = Object.keys(fn.param).$to_str(", ")
        }

        pdesc_sb = String.Builder()
        for Object.entries(fn.param) as [pname, param] {
            pdesc_sb.append("<li><b>{pname}</b>")

            if param.type {
                pdesc_sb.append(" \{<b>{param.type}</b>\}")
            }

            if param.value {
                pdesc_sb.append(" = {param.value}")
            }

            pdesc_sb.append(": {brief(param.brief)}\n")

            if param.detail?.length {
                pdesc_sb.append("<br/>{detail(param.detail)}\n")
            }

            pdesc_sb.append("</li>\n")
        }

        params = ''
<{{ntag}}>PARAMETERS</{{ntag}}>
<ul>
{{pdesc_sb.$to_str()}}
</ul>

        ''
    }

    if fn.return {
        retval = "<{ntag}>RETURN</{ntag}>\n"

        if fn.return.type {
            retval += "<b>{fn.return.type}</b>: "
        }

        retval += "{brief(fn.return.brief)}\n"

        if fn.return.detail?.length {
            retval += "<br/>{detail(fn.return.detail)}\n"
        }
    }

    if fn.throw {
        err_sb = String.Builder()

        for fn.throw as err {
            err_sb.append("<li>")

            if err.type {
                err_sb.append("<b>{err.type}</b>: ")
            }

            err_sb.append(brief(err.brief))
            err_sb.append("\n")

            if err.detail?.length {
                err_sb.append("<br/>")
                err_sb.append(detail(err.detail))
                err_sb.append("\n")
            }
        }

        errors = ''
<{{ntag}}>ERRORS</{{ntag}}>
<ul>
{{err_sb.$to_str()}}
</ul>

        ''
    }

    sb.append("<li><a href = \"#{scope}{name}\">{name}</a>({args}){desc}</li>\n")

    if fn.node_type == "sfunc" {
        is_static = "static "
    }

    detail_sb.append(''
<hr/>
<{{tag}} id="{{scope}}{{name}}">{{scope}}{{name}}</{{tag}}>
{{is_static}}<b>{{name}}({{args}})</b>
{{brief_section(ntag, fn)}}
{{detail_section(ntag, fn)}}
{{params}}
{{retval}}
{{errors}}

    '')
}

//Add variable.
add_var: func(sb, name, vdef) {
    sb.append("<li><b>{name}</b>")

    if vdef.type {
        sb.append(" \{<b>{vdef.type}</b>\}")
    }

    if vdef.brief?.length {
        sb.append(": ")
        sb.append(brief(vdef.brief))
        sb.append("\n")
    }

    if vdef.detail?.length {
        sb.append("</br>")
        sb.append(detail(vdef.detail))
        sb.append("\n")
    }

    sb.append("</li>\n")
}

//Add an accessor
add_acc: func(sb, name, adef) {
    sb.append("<li>")

    if adef.node_type == "roacc" || adef.node_type == "sroacc" {
        sb.append("readonly ")
    }

    sb.append("<b>{name}</b>")

    if adef.type {
        sb.append(" \{<b>{adef.type}</b>\}")
    }

    if adef.brief?.length {
        sb.append(": ")
        sb.append(brief(adef.brief))
        sb.append("\n")
    }

    if adef.detail?.length {
        sb.append("<br/>")
        sb.append(detail(adef.detail))
        sb.append("\n")
    }
}

//Generate document.
gen_doc: func(pn, name, doc, dir) {
    #file = File("{dir}/{pn}_{name}.html", "wb")

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
        sb.append("<h2>OPTIONS</h2>\n")
        sb.append("<ul>\n")

        for Object.entries(doc.option.oi) as [optn, optd] {
            if optd.type {
                atype = " {optd.type}"
            }
            if optd.brief {
                bdesc = ": {brief(optd.brief)}"
            }

            sb.append("<li><b>{optn}{atype}</b>{bdesc}")
            
            if optd.detail {
                sb.append("<br/>{detail(opt.detail)}\n")
            }

            sb.append("</li>\n")
        }
        sb.append("</ul>\n")
    }

    if doc.otype {
        sb.append("<h2>OBJECT TYPES</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.otype) as [tn, ot] {
            add_object("h2", sb, detail_sb, null, tn, ot)
        }
        sb.append("</ul>\n")
    }

    if doc.callback {
        sb.append("<h2>CALLBACK TYPES</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.callback) as [cn, cb] {
            add_func("h2", sb, detail_sb, null, cn, cb)
        }
        sb.append("</ul>\n")
    }

    if doc.const {
        sb.append("<h2>CONSTANTS</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.const) as[cn, cdef] {
            add_var(sb, cn, cdef)
        }
        sb.append("</ul>\n")
    }

    if doc.var {
        sb.append("<h2>VARIABLES</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.var) as[vn, vdef] {
            add_var(sb, vn, vdef)
        }
        sb.append("</ul>\n")
    }

    if doc.class {
        sb.append("<h2>CLASSES</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.class) as [cn, cdef] {
            add_class("h2", sb, detail_sb, null, cn, cdef)
        }
        sb.append("</ul>\n")
    }

    if doc.object {
        sb.append("<h2>OBJECTS</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.object) as [on, odef] {
            add_object("h2", sb, detail_sb, null, on, odef)
        }
        sb.append("</ul>\n")
    }

    if doc.func {
        sb.append("<h2>FUNCTIONS</h2>\n")
        sb.append("<ul>\n")
        for Object.entries(doc.func) as [cn, fn] {
            add_func("h2", sb, detail_sb, null, cn, fn)
        }
        sb.append("</ul>\n")
    }

    file.puts(''
<html>
<head>
    <meta charset="UTF-8">
    <title>{{type}} {{pn}}:{{name}}</title>
</head>
<h1>{{type}} {{pn}}:{{name}}</h1>
{{brief_section("h2", node)}}
{{detail_section("h2", node)}}
{{sb.$to_str()}}
{{detail_sb.$to_str()}}
</html>
    '')
}

//Generate HTML document.
public gen_html_doc: func(doc, pn, dir) {
    if doc.name {
        pn = doc.name
    }

    gen_index(doc, pn, dir)

    for Object.entries(doc.module) as [name, mod] {
        gen_doc(pn, name, mod, dir)
    }
}
