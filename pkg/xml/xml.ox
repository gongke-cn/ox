ref "std/uri"
ref "std/path"
ref "./xml_input"
ref "./log"

/*?
 *? @package xml XML parser.
 *? @lib XML parser.
 */

/*?
 *? @callback DocLoader External document loader.
 *? @param type {Document.Type} The document type.
 *? @param sys {String} The system identifier.
 *? @param pub {String} The public identifier.
 *? @return The document.
 */

//XML error.
public XmlError: class {
    //Initialize a XML error.
    $init(loc, msg) {
        this.loc = loc
        this.message = msg
    }

    //Convert the XML error to string.
    $to_str() {
        if this.loc.first_line == this.loc.last_line {
            if this.loc.first_column == this.loc.last_column {
                l = "{this.loc.first_line}.{this.loc.first_column}"
            } else {
                l = "{this.loc.first_line}.{this.loc.first_column}-{this.loc.last_column}"
            }
        } else {
            l = "{this.loc.first_line}.{this.loc.first_column}-{this.loc.last_line}.{this.loc.last_column}"
        }

        return "XmlError: \"{this.loc.file}\": {l}: {this.message}"
    }
}

//XML validate error.
public XmlValidateError: class XmlError {
}

//Build the children element FSM.
build_children_fsm: func(edecl) {
    if edecl.fsm {
        return edecl.fsm
    }

    case edecl.content {
    "ANY" {
        return null
    }
    "EMPTY" {
        return {
            end: true
        }
    }
    }

    //Build the FSM.
    nfa_state_num = 0
    nfa_add_state: func() {
        s = {id:nfa_state_num, edges:[]}
        @nfa_state_num += 1
        return s
    }

    nfa_start = nfa_add_state()
    nfa_end = nfa_add_state()
    nfa_end.end = true

    nfa_add_edge: func(s, e, tok) {
        edge = {
            dest: e
            token: tok
        }

        s.edges.push(edge)
    }

    nfa_add_child_once: func(c, s, e) {
        if c.data {
            nfa_add_edge(s, e, c.data)
        } elif c.mode == "choice" {
            for c.items as item {
                nfa_add_child(item, s, e)
            }
        } else {
            for c.items as item {
                t = nfa_add_state()
                nfa_add_child(item, s, t)
                s = t
            }

            nfa_add_edge(t, e)
        }
    }

    nfa_add_child: func(c, s, e, mixed) {
        case c.repeat {
        "?" {
            nfa_add_child_once(c, s, e)
            nfa_add_edge(s, e)
        }
        "+" {
            t = nfa_add_state()
            nfa_add_child_once(c, s, t)
            nfa_add_child_once(c, t, e)
            nfa_add_edge(t, e)
            nfa_add_edge(e, t)
        }
        "*" {
            nfa_add_child_once(c, s, e)
            nfa_add_edge(s, e)
            nfa_add_edge(e, s)
        }
        * {
            nfa_add_child_once(c, s, e)

            if mixed {
                nfa_add_edge(s, e)
            }
        }
        }
    }

    nfa_add_child(edecl.children, nfa_start, nfa_end, edecl.content == "MIXED")

    //Make the NFA to DFA.
    dfa_states = []
    dfa_stack = []

    nfa_states_equal: func(s1, s2) {
        if s1.length != s2.length {
            return false
        }

        for i = 0; i < s1.length; i += 1 {
            if s1[i].id != s2[i].id {
                return false
            }
        }

        return true
    }

    dfa_add_state: func(ss) {
        for ss as s {
            if s.end {
                end = true
            }
            for s.edges as e {
                if !e.token {
                    ds = e.dest
                    if !ss.has(ds) {
                        ss.push(ds)
                    }
                }
            }
        }

        ss.sort(($0.id - $1.id))

        for i = 0; i < dfa_states.length; i += 1 {
            ds = dfa_states[i]
            if nfa_states_equal(ds.nfa, ss) {
                return ds.dfa
            }
        }

        s = {edges:{}}
        if end {
            s.end = true
        }

        ds = {id:dfa_states.length, dfa:s, nfa:ss}
        dfa_states.push(ds)
        dfa_stack.push(ds)

        return s
    }

    dfa_add_state([nfa_start])

    while dfa_stack.length {
        se = dfa_stack.pop()
        edge_tab = Dict()

        for se.nfa as s { 
            for s.edges as e {
                if e.token {
                    ent = edge_tab.get(e.token)

                    if ent {
                        ent.push(e.dest)
                    } else {
                        edge_tab.add(e.token, [e.dest])
                    }
                }
            }
        }

        for edge_tab.entries() as [tok, ss] {
            ds = dfa_add_state(ss)
            se.dfa.edges[tok] = ds
        }
    }

    edecl.fsm = dfa_states[0].dfa
    return edecl.fsm
}

//XML parser.
Parser: class {
    //Initialize a XML parser.
    $init(xml, input, validate, ns, loader, uri) {
        this.xml = xml
        this.input = input
        this.validate = validate
        this.namespace = ns
        this.loader = loader

        if uri {
            this.uri = URI(uri)
        }
    }

    //Build the referenced document URI.
    get_uri(sys) {
        uri = URI(sys)

        if uri.path && this.uri {
            if !uri.host {
                if !uri.scheme {
                    uri.scheme = this.uri.scheme
                }

                if !uri.userinfo {
                    uri.userinfo = this.uri.userinfo
                } 

                uri.host = this.uri.host

                if !uri.port {
                    uri.port = this.uri.port
                }

                if !uri.query {
                    uri.query = this.uri.query
                }
            }

            if uri.path[0] != "/" {
                dn = dirname(this.uri.path)

                if dn == "/" {
                    uri.path = "/{uri.path}"
                } else {
                    uri.path = "{dn}/{uri.path}"
                }
            }
        }

        return uri.$to_str()
    }

    //Throw an unexpected token error.
    unexpect_error(loc, t1, t2) {
        if t2 {
            throw XmlError(loc, L"expect `{t1}' or `{t2}' here")
        } else {
            throw XmlError(loc, L"expect `{t1}' here")
        }
    }

    //Get a character from the input.
    get_char(ec) {
        c = this.input.get_char()
        if ec != null {
            if c != ec {
                this.unexpect_error(this.input.get_loc(), String.from_uchar(ec))
            }
        }

        return c
    }

    //Get a character not space from the input.
    get_char_ns(ec) {
        c = this.input.get_char_ns()
        if ec {
            if c != ec {
                this.unexpect_error(this.input.get_loc(), String.from_uchar(ec))
            }
        }

        return c
    }

    //Push back a character to the input.
    unget_char(c) {
        this.input.unget_char(c)
    }

    //Parse prolog.
    parse_prolog() {
        c = this.get_char()
        if c == '<' {
            c = this.get_char()
            if c == '?' {
                this.parse_xml_decl_or_pi()
            } else {
                this.unget_char(c)
                this.unget_char('<')
            }
        }

        this.parse_miscs(true)

        if this.validate && this.xml.doctype {
            if this.xml.doctype.internal {
                this.dtd = this.xml.doctype.internal
            } elif this.xml.doctype.external {
                this.dtd = this.xml.doctype.external
            }

            this.id_map = {}
        }
    }

    //Parse XML declaration.
    parse_xml_decl() {
        while true {
            c = this.get_char_ns()
            if c == '?' {
                c = this.get_char()
                if c == '>' {
                    break
                }

                this.unget_char(c)
            } elif c == -1 {
                this.unexpect_error(this.input.get_loc(), "?>")
            } else {
                this.unget_char(c)

                name = this.input.get_name()
                nloc = this.input.get_text_loc()

                c = this.get_char('=')

                str = this.input.get_str()

                case name {
                "version" {
                    if !(str ~ /1\.[0-9]+/p) {
                        throw XmlError(this.input.get_text_loc(), L"illegal XML version \"{str}\"")
                    }
                }
                "encoding" {
                    if !(str ~ /[a-zA-Z][a-zA-Z0-9._\-]*/p) {
                        throw XmlError(this.input.get_text_loc(), L"illegal XML encoding \"{str}\"")
                    }
                }
                "standalone" {
                    if this.is_dtd {
                        throw XmlError(this.input.get_text_loc(), L"\"standalone\" cannot be used in DTD")
                    }

                    if str != "yes" && str != "no" {
                        throw XmlError(this.input.get_text_loc(), L"value of \"standalone\" must be \"yes\" or \"no\"")
                    }
                }
                * {
                    throw XmlError(nloc, L"unknown XML attribute \"{name}\"")
                }
                }

                log.debug("xml {name}=\"{str}\"")
            }
        }
    }

    //Parse PI body.
    parse_pi_body(target) {
        if target.to_lower() == "xml" {
            throw XmlError(this.input.get_text_loc(), L"\"{target}\" cannot be used as PI target")
        }

        data = this.input.get_pi_data()

        log.debug("PI: {target} \"{data}\"")
    }

    //Parse PI.
    parse_pi() {
        name = this.input.get_name()

        this.parse_pi_body(name)
    }

    //Parse XML declaration or PI.
    parse_xml_decl_or_pi() {
        name = this.input.get_name()

        if name == "xml" {
            this.parse_xml_decl()
        } elif !this.is_dtd {
            this.parse_pi_body(name)
        } else {
            this.unexpect_error(this.input.get_text_loc(), "xml")
        }
    }

    //Parse CDATA.
    parse_cdata() {
        tag = this.input.get_name()
        if tag != "CDATA" {
            this.unexpect_error(this.input.get_text_loc(), "CDATA")
        }

        c = this.get_char('[')

        this.input.eatup_chars("]]>")
    }

    //Validate the ID.
    validate_id(id, loc) {
        old = this.id_map[id]
        if old?.defined {
            throw XmlValidateError(loc, L"id \"{id}\" is already used")
        }

        this.id_map[id] = {
            loc
            defined: true
        }
    }

    //Validate the ID reference.
    validate_id_ref(id, loc) {
        def = this.id_map[id]
        if !def {
            this.id_map[id] = {
                loc
            }
        }
    }

    //Validate the enrity.
    validate_entity(e, loc) {
        def = this.dtd?.entity[e]
        if !def || def.parsed {
            throw XmlValidateError(loc, L"entity \"{e}\" is not defined")
        }
    }

    //Validate the name token.
    validate_nm_token(n, loc) {
        if !XmlInput.is_nm_token(n) {
            throw XmlValidateError(loc, L"\"{n}\" is not a name token")
        }
    }

    //Lookup the namespace.
    lookup_ns(node, def_ns) {
        m = node.tag.match(/(.+):(.+)/p)
        if m {
            n = m.groups[1]
            node.short_tag = m.groups[2]
            if n == "xml" {
                return
            }
        } else {
            node.short_tag = node.tag
            if def_ns {
                node.ns = def_ns
                return
            } else {
                n = null
            }
        }

        for i = this.ns_stack.length - 1; i >= 0; i -=1 {
            map = this.ns_stack[i]
            uri = map.get(n)
            if uri {
                node.ns = uri
            }
        }
    }

    //Parse an element.
    parse_element() {
        c = this.get_char_ns('<')

        tag = this.input.get_name()
        tloc = this.input.get_text_loc()

        e = XmlElement(tag)

        ns_map = null

        if this.e {
            if !this.e.content {
                this.e.content = []
            }

            this.e.content.push(e)
        } else {
            if this.validate && this.xml.doctype {
                if e.tag != this.xml.doctype.root {
                    throw XmlValidateError(this.input.get_text_loc(), L"tag of root element must be \"{this.xml.doctype.root}\"")
                }
            }

            this.xml.root = e
        }

        //Get the element and attlist declaration from DTD.
        if this.dtd {
            if !(edecl = this.dtd?.element[tag]) {
                throw XmlValidateError(this.input.get_text_loc(), L"element \"{tag}\" is not defined in DTD")
            }

            aldecl = this.dtd?.attlist[tag]
        }

        while true {
            c = this.get_char_ns()
            if c == '/' {
                c = this.get_char()
                if c == '>' {
                    empty = true
                    break
                } else {
                    this.unexpect_error(this.input.get_loc(), ">")
                }
            } elif c == '>' {
                break
            } else {
                this.unget_char(c)

                attr = this.input.get_name()
                nloc = this.input.get_text_loc()

                c = this.get_char('=')

                val = this.input.get_str()

                if !e.attrs {
                    e.attrs = {}
                    e.attlist = []
                }

                if e.attrs[attr] {
                    throw XmlError(nloc, L"attribute \"{attr}\" is already declared")
                }

                //Validate the attributes
                if aldecl {
                    adecl = aldecl[attr]
                    if adecl {
                        case adecl.type {
                        "CDATA" {
                            ok = true
                        }
                        "ID" {
                            this.validate_id(val, nloc)
                        }
                        "IDREF" {
                            this.validate_id_ref(val, nloc)
                        }
                        "IDREFS" {
                            for val.trim().split(/\s+/) as id {
                                this.validate_id_ref(id, nloc)
                            }
                        }
                        "ENTITY" {
                            this.validate_entity(val, nloc)
                        }
                        "ENTITIES" {
                            for val.trim().split(/\s+/) as entity {
                                this.validate_entity(entity, nloc)
                            }
                        }
                        "NMTOKEN" {
                            this.validate_nm_token(val, nloc)
                        }
                        "NMTOKENS" {
                            for val.trim().split(/\s+/) as tok {
                                this.validate_nm_token(tok, nloc)
                            }
                        }
                        "NOTATION" {
                            if !adecl.enumeration.has(val) {
                                throw XmlValidateError(nloc, L"\"{val}\" is not a valid value of \"{attr}\"")
                            }

                            if !this.dtd?.notation[val] {
                                throw XmlValidateError(nloc, L"notation \"{val}\" is not defined");
                            }
                        }
                        * {
                            if !adecl.enumeration.has(val) {
                                throw XmlValidateError(nloc, L"\"{val}\" is not a valid value of \"{attr}\"")
                            }
                        }
                        }

                        if adecl.mode == "FIXED" {
                            if val != adecl.default {
                                throw XmlValidateError(nloc, L"attribute \"{attr}\" is not the fixed value")
                            }
                        }
                    }
                }

                alitem = {tag:attr, value:val}
                e.attrs[attr] = val
                e.attlist.push(alitem)

                if this.namespace {
                    match = attr.match(/xmlns(:(.+))?/p)
                    if match {
                        ns = match.groups[2]

                        if !ns_map {
                            ns_map = Dict()
                            this.ns_stack.push(ns_map)
                        }

                        ns_map.add(ns, val)
                    }
                }
            }
        }

        //Lookup namespace.
        if this.namespace {
            this.lookup_ns(e)

            for e.attlist as alitem {
                this.lookup_ns(alitem, e.ns)
            }
        }

        //Validate the required attributes.
        if aldecl {
            for Object.entries(aldecl) as [n, def] {
                if def.mode == "REQUIRED" {
                    if e.attrs?[n] == null {
                        throw XmlValidateError(tloc, L"required attribute \"{n}\" is not defined")
                    }
                }
            }
        }

        if edecl {
            fsm = build_children_fsm(edecl)
        }

        if !empty {
            old_e = this.e
            this.e = e

            while true {
                this.input.eatup_space()
                pcd = this.input.get_chars()
                if pcd {
                    if fsm {
                        ns = fsm.edges["#PCDATA"]
                        if !ns {
                            throw XmlValidateError(this.input.get_loc(), L"#PCDATA cannot be here")
                        }

                        fsm = ns
                    }

                    if !e.content {
                        e.content = []
                    }
                    e.content.push(pcd)
                }

                c = this.get_char('<')

                c = this.get_char()
                case c {
                '?' {
                    this.parse_pi()
                }
                '!' {
                    c = this.get_char()
                    if c == '-' {
                        this.parse_comment()
                    } elif c == '[' {
                        this.parse_cdata()
                    } else {
                        this.unexpect_error(this.input.get_loc(), "-", "[")
                    }
                }
                '/' {
                    name = this.input.get_name()
                    if name != tag {
                        throw XmlError(this.input.get_text_loc(), L"start and end tags mismatch")
                    }

                    c = this.get_char_ns('>')
                    break
                }
                -1 {
                    throw XmlError(this.input.get_loc(), L"expect end tag here")
                }
                * {
                    this.unget_char(c)
                    this.unget_char('<')
                    ctag = this.parse_element()

                    if fsm {
                        ns = fsm.edges[ctag]
                        if !ns {
                            throw XmlValidateError(this.input.get_loc(), L"element \"{ctag}\" cannot be here")
                        }

                        fsm = ns
                    }
                }
                }
            }

            this.e = old_e
        }

        if fsm {
            if !fsm.end {
                throw XmlValidateError(this.input.get_loc(), L"element \"{tag}\" should not end here")
            }
        }

        if ns_map {
            this.ns_stack.pop()
        }

        return tag
    }

    //Parse comment.
    parse_comment() {
        c = this.get_char('-')
        this.input.eatup_chars("-->")
    }

    //Parse element children.
    parse_element_children(d) {
        while true {
            c = this.get_char_ns()
            if c == '(' {
                cd = {items: [], mode: "seq"}
                d.items.push(cd)
                this.parse_element_children(cd)
            } elif c == ')' {
                break
            } else {
                this.unget_char(c)
                name = this.input.get_name()
                cd = {data: name}
                d.items.push(cd)

                c = this.get_char_ns()
                case c {
                '?' {
                    cd.repeat = "?"
                }
                '+' {
                    cd.repeat = "+"
                }
                '*' {
                    cd.repeat = "*"
                }
                * {
                    this.unget_char(c)
                }
                }
            }

            c = this.get_char_ns()
            if c == ')' {
                break
            } elif mode == '|' && c != '|' {
                this.unexpect_error(this.input.get_loc(), "|", ")")
            } elif mode == ',' && c != ',' {
                this.unexpect_error(this.input.get_loc(), ",", ")")
            } elif !mode {
                mode = c

                if c == '|' {
                    d.mode = "choice"
                } else {
                    d.mode = "seq"
                }
            }
        }

        c = this.get_char_ns()
        case c {
        '?' {
            d.repeat = "?"
        }
        '+' {
            d.repeat = "+"
        }
        '*' {
            d.repeat = "*"
        }
        * {
            this.unget_char(c)
        }
        }
    }

    //Parse element content.
    parse_element_content(ed) {
        c = this.get_char_ns()

        if c == '#' {
            tag = this.input.get_name()
            if tag != "PCDATA" {
                this.unexpect_error(this.input.get_text_loc(), "PCDATA")
            }

            ed.content = "MIXED"
            ed.children = {
                items: [{data: "#PCDATA"}]
                mode: "choice"
            }

            while true {
                c = this.get_char_ns()
                if c == ')' {
                    break
                } elif c == '|' {
                    this.input.eatup_space()
                    name = this.input.get_name()
                    ed.children.items.push({data: name})
                } else {
                    this.unexpect_error(this.input.get_loc(), "|", ")")
                }
            }

            if ed.children.items.length > 1 {
                c = this.get_char('*')
                ed.children.repeat = "*"
            } else {
                c = this.get_char()
                if c != '*' {
                    this.unget_char(c)
                }
            }
        } else {
            ed.children = {items: [], mode: "seq"}

            this.unget_char(c)
            this.parse_element_children(ed.children)
        }
    }

    //Parse element declaration.
    parse_element_decl() {
        dt = this.xml.doctype.internal

        if !dt.element {
            dt.element = {}
        }

        this.input.eatup_space()
        name = this.input.get_name()
        if dt.element[name] {
            throw XmlError(this.input.get_text_loc(), L"element \"{name}\" is already declared")
        }

        ed = {}
        dt.element[name] = ed

        c = this.get_char_ns()
        if c == '(' {
            this.parse_element_content(ed)
        } else {
            this.unget_char(c)

            tag = this.input.get_name()

            case tag {
            "EMPTY" {
                ed.content = "EMPTY"
            }
            "ANY" {
                ed.content = "ANY"
            }
            * {
                throw XmlError(this.input.get_text_loc(), L"illegal element content \"{tag}\"")
            }
            }
        }

        c = this.get_char_ns('>')
    }

    //Parse enumeration.
    parse_enumeration(attr, is_tok) {
        attr.enumeration = []

        this.input.eatup_space()
        if is_tok {
            name = this.input.get_nm_token()
        } else {
            name = this.input.get_name()
        }
        attr.enumeration.push(name)

        while true {
            c = this.get_char_ns()
            if c == ')' {
                break
            } elif c == '|' {
                this.input.eatup_space()
                if is_tok {
                    name = this.input.get_nm_token()
                } else {
                    name = this.input.get_name()
                }
                attr.enumeration.push(name)
            } else {
                this.unexpect_error(this.input.get_loc(), "|", ")")
            }
        }
    }

    //Parse attribute list declaration.
    parse_attlist_decl() {
        dt = this.xml.doctype.internal

        if !dt.attlist {
            dt.attlist = {}
        }

        this.input.eatup_space()
        name = this.input.get_name()

        ad = dt.attlist[name]
        if !ad {
            ad = {}
            dt.attlist[name] = ad
        }

        while true {
            c = this.get_char_ns()
            if c == '>' {
                break
            } else {
                this.unget_char(c)
                name = this.input.get_name()

                attr = ad[name]
                if !attr {
                    attr = {}
                    ad[name] = attr
                }

                c = this.get_char_ns()
                if c == '(' {
                    this.parse_enumeration(attr, true)
                } else {
                    this.unget_char(c)
                    type = this.input.get_name()

                    case type {
                    "CDATA"
                    "ID"
                    "IDREF"
                    "IDREFS"
                    "ENTITY"
                    "ENTITIES"
                    "NMTOKEN"
                    "NMTOKENS" {
                        attr.type = type
                    }
                    "NOTATION" {
                        attr.type = type

                        c = this.get_char_ns('(')

                        this.parse_enumeration(attr, false)
                    }
                    * {
                        throw XmlError(this.input.get_text_loc(), L"illegal attribute type \"{type}\"")
                    }
                    }
                }

                c = this.get_char_ns()
                if c == '#' {
                    name = this.input.get_name()

                    case name {
                    "REQUIRED"
                    "IMPLIED" {
                        attr.mode = name
                    }
                    "FIXED" {
                        attr.mode = name

                        this.input.eatup_space()
                        str = this.input.get_str()
                        attr.default = str
                    }
                    * {
                        throw XmlError(this.input.get_text_loc(), L"illegal default value \"{name}\"")
                    }
                    }
                } else {
                    this.unget_char(c)
                    str = this.input.get_str()
                    attr.default = str
                }
            }
        }
    }

    //Parse entity declaration.
    parse_entity_decl() {
        dt = this.xml.doctype.internal

        if !dt.entity {
            dt.entity = {}
        }

        c = this.get_char_ns()
        if c == '%' {
            parsed = true
            this.input.eatup_space()
        } else {
            this.unget_char(c)
        }

        entity_name = this.input.get_name()
        ed = {}

        if parsed {
            ed.parsed = true
        }

        c = this.get_char_ns()
        if c == '\'' || c == '\"' {
            this.unget_char(c)
            str = this.input.get_str()
            ed.def = str
        } else {
            this.unget_char(c)

            if !this.parse_ext_id(ed) {
                this.unexpect_error(this.input.get_text_loc(), "SYSTEM", "PUBLIC")
            }
        }

        if !parsed {
            c = this.get_char_ns()
            if c == 'N' {
                this.unget_char(c)
                tag = this.input.get_name()
                if tag != "NDATA" {
                    this.unexpect_error(this.input.get_text_loc(), "NDATA")
                }

                this.input.eatup_space()
                name = this.input.get_name()
                ed.ndata = name

                c = this.get_char_ns()
            }
        } else {
            c = this.get_char_ns()
        }

        if c != '>' {
            this.unexpect_error(this.input.get_loc(), ">")
        }

        dt.entity[entity_name] = ed
    }

    //Parse notation declaration.
    parse_notation_decl() {
        dt = this.xml.doctype.internal

        if !dt.notation {
            dt.notation = {}
        }

        this.input.eatup_space()
        name = this.input.get_name()

        nd = dt.notation[name]
        if nd {
            throw XmlError(this.input.get_text_loc(), L"notation \"{name}\" is already declared")
        }

        nd = {}
        dt.notation[name] = nd

        c = this.get_char_ns()
        if c != '>' {
            this.unget_char(c)
            tag = this.input.get_name()

            if tag == "SYSTEM" {
                this.input.eatup_space()
                str = this.input.get_str()
                nd.system = str

                c = this.get_char_ns()
            } elif tag == "PUBLIC" {
                this.input.eatup_space()
                str = this.input.get_str()
                nd.public = str

                c = this.get_char_ns()
                if c != '>' {
                    this.unget_char(c)
                    str = this.input.get_str()
                    nd.system = str

                    c = this.get_char_ns()
                }
            }
        }

        if c != '>' {
            this.unexpect_error(this.input.get_loc(), ">")
        }
    }

    //Parse include section.
    parse_include_sect() {
        c = this.get_char_ns('[')

        this.parse_subset_decl()

        c = this.get_char_ns(']')
        c = this.get_char(']')
        c = this.get_char('>')
    }

    //Parse ignore section.
    parse_ignore_sect() {
        c = this.get_char_ns('[')

        level = 0

        while true {
            c = this.input.get_char_ns(true)
            if c == '<' {
                c = this.input.get_char(true)
                if c == '!' {
                    c = this.input.get_char(true)
                    if c == '[' {
                        level += 1
                    } else {
                        this.unget_char(c)
                        this.unget_char('!')
                    }
                } else {
                    this.unget_char(c)
                }
            } elif c == ']' {
                c = this.input.get_char(true)
                if c == ']' {
                    c = this.input.get_char(true)
                    if c == '>' {
                        if level == 0 {
                            break
                        } else {
                            level -= 1
                        }
                    } else {
                        this.unget_char(c)
                        this.unget_char(']')
                    }
                } else {
                    this.unget_char(c)
                }
            }
        }
    }

    //Parse conditional section.
    parse_cond_sect() {
        this.input.eatup_space()
        tag = this.input.get_name()
        case tag {
        "INCLUDE" {
            this.parse_include_sect()
        }
        "IGNORE" {
            this.parse_ignore_sect()
        }
        * {
            throw XmlError(this.input.get_text_loc(), L"illegal tag \"{tag}\"")
        }
        }
    }

    //Parse subset of the doctype.
    parse_subset_decl() {
        while true {
            c = this.get_char_ns()
            if c == '<' {
                c = this.get_char()
                case c {
                '!' {
                    c = this.get_char()
                    case c {
                    '-' {
                        this.parse_comment()
                    }
                    '[' {
                        if this.is_dtd {
                            this.parse_cond_sect()
                        } else {
                            throw XmlError(this.input.get_loc(), L"`[' cannot be used here")
                        }
                    }
                    * {
                        this.unget_char(c)
                        tag = this.input.get_name()
                        case tag {
                        "ELEMENT" {
                            this.parse_element_decl()
                        }
                        "ATTLIST" {
                            this.parse_attlist_decl()
                        }
                        "ENTITY" {
                            this.parse_entity_decl()
                        }
                        "NOTATION" {
                            this.parse_notation_decl()
                        }
                        * {
                            throw XmlError(this.input.get_loc(), L"illegal declaration type \"{tag}\"")
                        }
                        }
                    }
                    }
                }
                '?' {
                    this.parse_pi()
                }
                * {
                    this.unexpect_error(this.input.get_loc(), "!", "?")
                }
                }
            } else {
                this.unget_char(c)
                break
            }
        }
    }

    //Parse external ID.
    parse_ext_id(o) {
        c = this.get_char_ns()
        if c == 'S' {
            this.unget_char(c)
            tag = this.input.get_name()
            if tag != "SYSTEM" {
                this.unexpect_error(this.input.get_text_loc(), "SYSTEM")
            }

            this.input.eatup_space()
            id = this.input.get_str()
            o.system = id

            return true
        } elif c == 'P' {
            this.unget_char(c)
            tag = this.input.get_name()
            if tag != "PUBLIC" {
                this.unexpect_error(this.input.get_text_loc(), "PUBLIC")
            }

            this.input.eatup_space()
            id = this.input.get_str()
            o.public = id

            this.input.eatup_space()
            id = this.input.get_str()
            o.system = id

            return true
        } else {
            this.unget_char(c)
        }

        return false
    }

    //Parse document type.
    parse_doctype() {
        name = this.input.get_name()
        if name != "DOCTYPE" {
            this.unexpect_error(this.input.get_text_loc(), "DOCTYPE")
        }

        this.input.eatup_space()
        name = this.input.get_name()

        dt = {
            root: name
        }

        this.xml.doctype = dt

        this.parse_ext_id(dt)
        c = this.get_char_ns()

        if c == '[' {
            this.xml.doctype.internal = {}

            this.input.set_resolve_pe(true)
            this.parse_subset_decl()
            this.input.set_resolve_pe(false)

            c = this.get_char_ns(']')
            c = this.get_char_ns()
        } else {
            //Load the external DTD file.
            if this.loader && (dt.system || dt.public) {
                dtd = this.loader(Document.DTD, this.get_uri(dt.system), dt.public)
                if dtd {
                    dt.external = dtd.doctype.internal
                }
            }
        }

        if c != '>' {
            this.unexpect_error(this.input.get_loc(), ">")
        }
    }

    //Parse miscs.
    parse_miscs(has_doctype) {
        while true {
            c = this.get_char_ns()
            if c == '<' {
                c = this.get_char()
                if c == '!' {
                    c = this.get_char()
                    if c == '-' {
                        this.parse_comment()
                    } elif has_doctype {
                        this.unget_char(c)
                        this.parse_doctype()
                        has_doctype = false
                    } else {
                        this.unexpect_error(this.input.get_loc(), "-")
                    }
                } elif c == '?' {
                    this.parse_pi()
                } else {
                    this.unget_char(c)
                    this.unget_char('<')
                    break
                }
            } else {
                this.unget_char(c)
                break
            }
        }
    }

    //Parse the XML.
    parse() {
        try {
            this.parse_prolog()

            //Build namespace stack.
            if this.namespace {
                this.ns_stack = []
            }

            this.parse_element()
            this.parse_miscs()

            //Check if the ID references are all defined.
            if this.id_map {
                for Object.entries(this.id_map) as [n, def] {
                    if !def.defined {
                        throw XmlValidateError(def.loc, L"id \"{n}\" is not defined")
                    }
                }
            }
        } catch e {
            if e instof SyntaxError {
                throw XmlError(this.input.get_text_loc(), e.message)
            } else {
                throw e
            }
        }

        c = this.get_char()
        if c != -1 {
            throw XmlError(this.input.get_loc(), L"illegal character")
        }
    }

    //Parse the DTD.
    parse_dtd() {
        this.is_dtd = true

        try {
            c = this.get_char()
            if c == '<' {
                c = this.get_char()
                if c == '?' {
                    this.parse_xml_decl_or_pi()
                } else {
                    this.unget_char(c)
                    this.unget_char('<')
                }
            }

            this.xml.doctype = {}
            this.xml.doctype.internal = {}

            this.input.set_resolve_pe(true)
            this.parse_subset_decl()
            this.input.set_resolve_pe(false)
        } catch e {
            if e instof SyntaxError {
                throw XmlError(this.input.get_text_loc(), e.message)
            } else {
                throw e
            }
        }

        c = this.get_char()
        if c != -1 {
            throw XmlError(this.input.get_loc(), L"illegal character")
        }
    }

    //Resolve entity.
    resolve_entity(in) {
        case in {
        "amp" {
            return "&"
        }
        "lt" {
            return "<"
        }
        "gt" {
            return ">"
        }
        "apos" {
            return "'"
        }
        "quot" {
            return "\""
        }
        }

        if this.xml?.doctype.internal.entity {
            edef = this.xml.doctype.internal.entity[in]
            if edef?.parsed {
                if edef.def {
                    return edef.def
                } elif this.loader {
                    return this.loader(Document.TEXT, this.get_uri(edef.system), edef.public)
                }
            }
        }
    }
}

/*?
 *? Element of XML.
 */
public XmlElement: class {
    /*?
     *? Initialize an element.
     *? @param tag The tag of the element.
     */
    $init(tag) {
        this.tag = tag
    }

    /*?
     *? Get the attribute of the element.
     *? @param tag {String} The tag of the attribute.
     *? @param ns {?String} The namespace of the attribute.
     *? @return {?String} The attribute's value.
     */
    get_attr(tag, ns) {
        if ns {
            for this.attlist as attr {
                if attr.short_tag == tag && attr.ns == ns {
                    return attr.value
                }
            }
        } else {
            return this.attrs[tag]
        }
    }

    /*?
     *? Set the attribute value of the element.
     *? @param tag {String} The tag of the attribute.
     *? @param val The value of the attribute.
     *? @param ns {?String} The namespace of the attribute.
     */
    set_attr(tag, val, ns) {
        if ns {
            this.attrs[tag] = val
        } else {
            old

            for this.attlist as attr {
                if attr.short_tag == tag && attr.ns == ns {
                    old = attr
                    break
                }
            }

            if old {
                old.value = val
            } else {
                this.attlist.append({
                    short_tag: tag
                    ns: ns
                    value: val
                })
            }
        }
    }
}

/*?
 *? Base document.
 */
public Document: class {
    /*?
     *? Document type.
     */
    enum Type {
        //? Text document.
        TEXT
        //? Document type document.
        DTD
        //? XML document.
        XML
    }

    //Convert the document to string.
    $to_str(indent, is_dtd) {
        sb = String.Builder()

        sb.append("<?xml version=\"1.0\"?>")

        level = 0

        pr_newline: func() {
            if indent {
                sb.append("\n")
            }
        }

        pr_indent: func() {
            if indent {
                for i = 0; i < level; i += 1 {
                    sb.append(indent)
                }
            }
        }

        if this.doctype {
            pr_newline()

            dt = this.doctype

            if !is_dtd {
                sb.append("<!DOCTYPE {dt.root}")

                if dt.public {
                    sb.append(" PUBLIC \"{XML.escape(dt.public)}\" \"{XML.escape(dt.system)}\"")
                } elif dt.system {
                    sb.append(" SYSTEM \"{XML.escape(dt.system)}\"")
                }
            }

            if dt.internal {

                if !is_dtd {
                    sb.append(" [")
                    pr_newline()
                    level += 1
                }

                for Object.entries(dt.internal.element) as [n, def] {
                    pr_indent()
                    sb.append("<!ELEMENT {n}")

                    if def.content && def.content != "MIXED" {
                        sb.append(" {def.content}")
                    }

                    if def.children {
                        element_children_to_str: func(c) {
                            if c.mode == "choice" {
                                sep = "|"
                            } else {
                                sep = ","
                            }

                            children = c.items.$iter().map(func(item) {
                                if item.items {
                                    return element_children_to_str(item)
                                }

                                return "{item.data}{item.repeat}"
                            }).$to_str(sep)

                            return "({children}){c.repeat}"
                        }

                        sb.append(" {element_children_to_str(def.children)}")
                    }

                    sb.append(">")
                    pr_newline()
                }

                for Object.entries(dt.internal.attlist) as [n, def] {
                    pr_indent()
                    sb.append("<!ATTLIST {n}")
                    pr_newline()

                    level += 1

                    for Object.entries(def) as [an, adef] {
                        pr_indent()

                        sb.append(" {an}")

                        if adef.type {
                            sb.append(" {adef.type}")
                        }

                        if adef.enumeration {
                            sb.append(" ({adef.enumeration.$iter().$to_str("|")})")
                        }

                        if adef.mode {
                            sb.append(" #{adef.mode}")
                        }

                        if adef.default {
                            sb.append(" \"{XML.escape(adef.default)}\"")
                        }

                        pr_newline()
                    }

                    level -= 1

                    pr_indent()
                    sb.append(">")
                    pr_newline()
                }

                for Object.entries(dt.internal.entity) as [n, def] {
                    pr_indent()
                    sb.append("<!ENTITY ")

                    if def.parsed {
                        sb.append("% ")
                    }

                    sb.append(n)

                    if def.def {
                        sb.append(" \"{XML.escape(def.def)}\"")
                    } elif def.public {
                        sb.append(" PUBLIC \"{XML.escape(def.public)}\" \"{XML.escape(def.system)}\"")
                    } elif def.system {
                        sb.append(" SYSTEM \"{XML.escape(def.system)}\"")
                    }

                    if def.ndata {
                        sb.append(" NDATA {def.ndata}")
                    }

                    sb.append(">")
                    pr_newline()
                }

                for Object.entries(dt.internal.notation) as [n, def] {
                    pr_indent()
                    sb.append("<!NOTATION {n}")

                    if def.public && def.system {
                        sb.append(" PUBLIC \"{XML.escape(def.public)}\" \"{XML.escape(def.system)}\"")
                    } elif def.public {
                        sb.append(" PUBLIC \"{XML.escape(def.public)}\"")
                    } elif def.system {
                        sb.append(" SYSTEM \"{XML.escape(def.system)}\"")
                    }

                    sb.append(">")
                    pr_newline()
                }

                if !is_dtd {
                    level -= 1

                    pr_indent()
                    sb.append("]")
                }
            }

            if !is_dtd {
                sb.append(">")
            }
        }

        element_to_str: func(e) {
            pr_newline()
            pr_indent()

            sb.append("<{e.tag}")

            if e.attrs {
                for Object.entries(e.attrs) as [k, v] {
                    sb.append(" {k}=\"{XML.escape(v)}\"")
                }
            }

            if !e.content {
                sb.append("/>")
            } else {
                sb.append(">")

                @level += 1

                for e.content as child {
                    if String.is(child) {
                        pr_newline()
                        sb.append(XML.escape(child))
                    } else {
                        element_to_str(child)
                    }
                }

                @level -= 1

                pr_newline()
                pr_indent()
                sb.append("</{e.tag}>")
            }
        }

        if !is_dtd {
            element_to_str(this.root)
        }

        return sb.$to_str()
    }
}

/*?
 *? XML
 */
public XML: class Document {
    /*?
     *? Load XML from a file.
     *? @param fn {String} The filename.
     *? @param validate {?Bool} Validate the XML with the document type when it is true.
     *? @param ns {?Bool} Enable namespace.
     *? @param loader {?DocLoader} The external document loader.
     *? @param uri {String} The URI string of this document.
     *? @return {XML} The result XML.
     */
    static from_file(fn, validate, ns, loader, uri) {
        xml = XML()
        #input = XmlInput.from_file(fn, func(in) {
            return p.resolve_entity(in)
        })
        p = Parser(xml, input, validate, ns, loader, uri)
        p.parse()

        return xml
    }

    /*?
     *? Load XML from a string.
     *? @param s {String} The source string.
     *? @param validate {Bool} Validate the XML with the document type when it is true.
     *? @param ns {?Bool} Enable namespace.
     *? @param loader {?DocLoader} The external document loader.
     *? @param uri {String} The URI string of this document.
     *? @return {XML} The result XML.
     */
    static from_str(s, validate, ns, loader, uri) {
        xml = XML()
        #input = XmlInput.from_str(s, func(in) {
            return p.resolve_entity(in)
        })
        p = Parser(xml, input, validate, ns, loader, uri)
        p.parse()

        return xml
    }

    /*?
     *? Convert the escape characters.
     *? @param s {String} The input string.
     *? @return {String} The result string.
     */
    static escape(s) {
        return s.replace(/[<>&,;]/, func(in) {
            case in.$to_str() {
            "<" {
                return "&lt;"
            }
            ">" {
                return "&gt;"
            }
            "&" {
                return "&amp;"
            }
            "," {
                return "&apos;"
            }
            ";" {
                return "&quot;"
            }
            }
        })
    }

    /*?
     *? Convert the XML to string.
     *? @param indent {?String} The indent string.
     *? @return {String} The result string.
     */
     $to_str(indent) {
        return Document.$inf.$to_str.call(this, indent, false)
     }
}

/*?
 *? XML document type document.
 */
public DTD: class Document {
    /*?
     *? Load DTD from a file.
     *? @param fn {String} The DTD filename.
     *? @param loader {DocLoader} The external document loader.
     *? @param uri {String} The URI string of this document.
     *? @return {DTD} The DTD document.
     */
    static from_file(fn, loader, uri) {
        dtd = DTD()
        #input = XmlInput.from_file(fn, func(in) {
            return p.resolve_entity(in)
        })
        p = Parser(dtd, input, false, false, loader, uri)
        p.parse_dtd()

        return dtd
    }

    /*?
     *? Load DTD from a string.
     *? @param s {String} The source string.
     *? @param loader {DocLoader} The external document loader.
     *? @param uri {String} The URI string of this document.
     *? @return {DTD} The DTD document.
     */
    static from_str(s, loader, uri) {
        dtd = DTD()
        #input = XmlInput.from_str(s, func(in) {
            return p.resolve_entity(in)
        })
        p = Parser(dtd, input, false, false, loader, uri)
        p.parse_dtd()

        return dtd
    }

    /*?
     *? Convert the DTD to string.
     *? @param indent {?String} The indent string.
     *? @return {String} The result string.
     */
     $to_str(indent) {
        return Document.$inf.$to_str.call(this, indent, true)
     }
}
