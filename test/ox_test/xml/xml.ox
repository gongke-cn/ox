ref "../test"
ref "std/log"
ref "json"
ref "xml"

log: Log("xml")

XML.from_str("<root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<root />").{
    (test($.root.tag == "root"))
    (test($.root.content == null))
}

XML.from_str("<root attr=\"1\"/>").{
    (test($.root.tag == "root"))
    (test($.root.attrs["attr"] == "1"))
}

XML.from_str("<root attr='1'/>").{
    (test($.root.tag == "root"))
    (test($.root.attrs["attr"] == "1"))
}

XML.from_str("<root attr='&lt;&gt;&amp;&apos;&quot;'/>").{
    (test($.root.tag == "root"))
    (test($.root.attrs["attr"] == "<>&'\""))
}

XML.from_str("<root a='1' b='2' />").{
    (test($.root.tag == "root"))
    (test($.root.attrs["a"] == "1"))
    (test($.root.attrs["b"] == "2"))
}

XML.from_str("<root></root>").{
    (test($.root.tag == "root"))
    (test($.root.content == null))
}

XML.from_str("<root><node/></root>").{
    (test($.root.tag == "root"))
    (test($.root.content[0].tag = "node"))
}

XML.from_str("<root> pcdata&lt;&gt;&amp;&apos;&quot; </root>").{
    (test($.root.tag == "root"))
    (test($.root.content[0] = "pcdata<>&'\""))
}

XML.from_str("<root>head<node/>tail</root>").{
    (test($.root.tag == "root"))
    (test($.root.content[0] = "head"))
    (test($.root.content[1].tag = "node"))
    (test($.root.content[2] = "tail"))
}

XML.from_str("<?xml?><root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<?xml version='1.123'?><root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<?xml version='1.123' encoding='zh_CN.UTF-8'?><root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<?xml version='1.123' encoding='zh_CN.UTF-8' standalone='yes'?><root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<?xml version='1.123' encoding='zh_CN.UTF-8' standalone='no'?><root/>").{
    (test($.root.tag == "root"))
}

XML.from_str("<?pi pi data ...?><root><?pi pi data ...?></root><?pi pi data ...?>").{
    (test($.root.tag == "root"))
}

XML.from_str("<!--comment--><root><!--comment --></root><!-- comment -->").{
    (test($.root.tag == "root"))
}

XML.from_str("<root> <![CDATA[cdata .......]]> </root>").{
    (test($.root.tag == "root"))
}

XML.from_str("<!DOCTYPE root><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
}

XML.from_str("<!DOCTYPE root SYSTEM 'mydoc'><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.system == "mydoc"))
}

XML.from_str("<!DOCTYPE root PUBLIC 'http://ox.org/mydoc.dtd' 'mydoc'><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.system == "mydoc"))
    (test($.doctype.public == "http://ox.org/mydoc.dtd"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root EMPTY>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].content == "EMPTY"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root ANY>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].content == "ANY"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (#PCDATA)>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].content == "MIXED"))
    (test($.doctype.internal.element["root"].children.items[0].data == "#PCDATA"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (#PCDATA|node)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].content == "MIXED"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "choice"))
    (test($.doctype.internal.element["root"].children.items[0].data == "#PCDATA"))
    (test($.doctype.internal.element["root"].children.items[1].data == "node"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1)>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == null))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1)?>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "?"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1)+>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "+"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1, n2)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
    (test($.doctype.internal.element["root"].children.items[1].data == "n2"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1 | n2)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "choice"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
    (test($.doctype.internal.element["root"].children.items[1].data == "n2"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root (n1?, n2)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].repeat == "?"))
    (test($.doctype.internal.element["root"].children.items[0].data == "n1"))
    (test($.doctype.internal.element["root"].children.items[1].data == "n2"))
}

XML.from_str("<!DOCTYPE root [<!ELEMENT root ((n1|n2)?, n3)*>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.element["root"].children.repeat == "*"))
    (test($.doctype.internal.element["root"].children.mode == "seq"))
    (test($.doctype.internal.element["root"].children.items[0].mode == "choice"))
    (test($.doctype.internal.element["root"].children.items[0].repeat == "?"))
    (test($.doctype.internal.element["root"].children.items[0].items[0].data == "n1"))
    (test($.doctype.internal.element["root"].children.items[0].items[1].data == "n2"))
    (test($.doctype.internal.element["root"].children.items[1].data == "n3"))
}

XML.from_str("<!DOCTYPE root [<!ATTLIST root>]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.attlist["root"] != null))
}

XML.from_str(
"<!DOCTYPE root [
    <!ATTLIST root
        a CDATA #REQUIRED
        b ID #IMPLIED
        c IDREF #FIXED 'default'
        d IDREFS 'd'>
]><root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.attlist["root"]["a"].type == "CDATA"))
    (test($.doctype.internal.attlist["root"]["a"].mode == "REQUIRED"))
    (test($.doctype.internal.attlist["root"]["b"].type == "ID"))
    (test($.doctype.internal.attlist["root"]["b"].mode == "IMPLIED"))
    (test($.doctype.internal.attlist["root"]["c"].type == "IDREF"))
    (test($.doctype.internal.attlist["root"]["c"].mode == "FIXED"))
    (test($.doctype.internal.attlist["root"]["c"].default == "default"))
    (test($.doctype.internal.attlist["root"]["d"].type == "IDREFS"))
    (test($.doctype.internal.attlist["root"]["d"].mode == null))
    (test($.doctype.internal.attlist["root"]["d"].default == "d"))
}

XML.from_str(
"<!DOCTYPE root [
    <!ENTITY a 'value of a'>
    <!ENTITY % b 'value of b'>
    <!ENTITY c SYSTEM 'entity1'>
    <!ENTITY d PUBLIC 'http://ox.org/entity2' 'entity2'>
    <!ENTITY e SYSTEM 'entity3' NDATA n1>
]>
<root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test(!$.doctype.internal.entity["a"].parsed))
    (test($.doctype.internal.entity["a"].def == "value of a"))
    (test($.doctype.internal.entity["b"].parsed))
    (test($.doctype.internal.entity["b"].def == "value of b"))
    (test(!$.doctype.internal.entity["c"].parsed))
    (test($.doctype.internal.entity["c"].def == null))
    (test($.doctype.internal.entity["c"].system == "entity1"))
    (test(!$.doctype.internal.entity["d"].parsed))
    (test($.doctype.internal.entity["d"].def == null))
    (test($.doctype.internal.entity["d"].public == "http://ox.org/entity2"))
    (test($.doctype.internal.entity["d"].system == "entity2"))
    (test(!$.doctype.internal.entity["e"].parsed))
    (test($.doctype.internal.entity["e"].def == null))
    (test($.doctype.internal.entity["e"].system == "entity3"))
    (test($.doctype.internal.entity["e"].ndata == "n1"))
}

XML.from_str(
"<!DOCTYPE root [
    <!NOTATION a>
    <!NOTATION b SYSTEM 'n1'>
    <!NOTATION c PUBLIC 'http://ox.org/n2'>
    <!NOTATION d PUBLIC 'http://ox.org/n3' 'n3'>
]>
<root/>").{
    (test($.root.tag == "root"))
    (test($.doctype.root == "root"))
    (test($.doctype.internal.notation["a"] != null))
    (test($.doctype.internal.notation["b"].system == "n1"))
    (test($.doctype.internal.notation["c"].public == "http://ox.org/n2"))
    (test($.doctype.internal.notation["d"].public == "http://ox.org/n3"))
    (test($.doctype.internal.notation["d"].system == "n3"))
}

xml_to_str: func(s) {
    xml = XML.from_str(s)
    test(xml.$to_str() == s, "\"{s}\" != \"{xml.$to_str()}\"", 2)
}

xml_to_str(''<?xml version="1.0"?><!DOCTYPE root PUBLIC "http://ox.org/mydoc" "mydoc" [<!ELEMENT root (#PCDATA|node)*><!ATTLIST root a CDATA #IMPLIED><!ENTITY e SYSTEM "e1" NDATA n1><!NOTATION n1 SYSTEM "n1">]><root>head<node/>tail</root>'')
xml_to_str(''<?xml version="1.0"?><!DOCTYPE root [<!ELEMENT root (#PCDATA)>]><root/>'')
xml_to_str(''<?xml version="1.0"?><!DOCTYPE root [<!ELEMENT root (n1?,n2+,n3*)+>]><root/>'')

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><!ATTLIST root a CDATA #IMPLIED>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist["root"]["a"].type == "CDATA"))
    (test($.doctype.internal.attlist["root"]["a"].mode == "IMPLIED"))
}

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><![INCLUDE[<!ATTLIST root a CDATA #IMPLIED>]]>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist["root"]["a"].type == "CDATA"))
    (test($.doctype.internal.attlist["root"]["a"].mode == "IMPLIED"))
}

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><![IGNORE[<!ATTLIST root a CDATA #IMPLIED>]]>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist == null))
}

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><![IGNORE[ <![ <!ATTLIST root a CDATA #IMPLIED> ]]> ]]>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist == null))
}

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><!ENTITY % include \"INCLUDE\"><![%include;[<!ATTLIST root a CDATA #IMPLIED>]]>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist["root"]["a"].type == "CDATA"))
    (test($.doctype.internal.attlist["root"]["a"].mode == "IMPLIED"))
}

DTD.from_str("<?xml version=\"1.0\"?><!ELEMENT root ANY><!ENTITY % ignore \"IGNORE\"><![%ignore;[ <![ <!ATTLIST root a CDATA #IMPLIED> ]]> ]]>").{
    (test($.doctype.internal.element["root"].content == "ANY"))
    (test($.doctype.internal.attlist == null))
}

dtd_to_str: func(s) {
    dtd = DTD.from_str(s)
    test(dtd.$to_str() == s, "\"{s}\" != \"{dtd.$to_str()}\"", 2)
}

dtd_to_str(''<?xml version="1.0"?><!ELEMENT root ANY><!ATTLIST root a CDATA #IMPLIED>'')

validate_ok: func(s) {
    ok = true

    try {
        xml = XML.from_str(s, true)
    } catch e {
        if e instof XmlValidateError {
            ok = false
        } else {
            throw e
        }
    }

    test(ok, "\"{s}\" failed", 2)
}

validate_fail: func(s) {
    ok = false

    try {
        xml = XML.from_str(s, true)
    } catch e {
        if e instof XmlValidateError {
            ok = true
        } else {
            throw e
        }
    }

    test(ok, "\"{s}\" ok", 2)
}

validate_ok("<!DOCTYPE root><root/>")
validate_fail("<!DOCTYPE root><root1/>")
validate_fail("<!DOCTYPE root [<!ELEMENT node ANY>]><root/>")
validate_ok("<!DOCTYPE root [<!ELEMENT root ANY>]><root/>")

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root a CDATA #REQUIRED>
]>
<root a="1"/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root a CDATA #REQUIRED>
]>
<root a=""/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root a CDATA #FIXED "1">
]>
<root a="1"/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root a CDATA #FIXED "1">
]>
<root a=""/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root id ID #REQUIRED>
]>
<root/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root id ID #REQUIRED>
    <!ATTLIST node id ID #REQUIRED>
]>
<root id='a'>
    <node id='b'/>
</root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root id ID #REQUIRED>
    <!ATTLIST node id ID #REQUIRED>
]>
<root id='a'>
    <node id='a'/>
</root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root
        id ID #REQUIRED
        ref IDREF #REQUIRED>
    <!ATTLIST node
        id ID #REQUIRED
        ref IDREF #REQUIRED>
]>
<root id='a' ref='b'>
    <node id='b' ref='a'/>
</root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root
        id ID #REQUIRED
        ref IDREF #REQUIRED>
    <!ATTLIST node
        id ID #REQUIRED
        ref IDREF #REQUIRED>
]>
<root id='a' ref='b'>
    <node id='b' ref='c'/>
</root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root
        id ID #REQUIRED
        refs IDREFS #REQUIRED>
    <!ATTLIST node
        id ID #REQUIRED
        refs IDREFS #REQUIRED>
]>
<root id='a' refs='a b'>
    <node id='b' refs=' b a '/>
</root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ELEMENT node ANY>
    <!ATTLIST root
        id ID #REQUIRED
        refs IDREFS #REQUIRED>
    <!ATTLIST node
        id ID #REQUIRED
        refs IDREFS #REQUIRED>
]>
<root id='a' refs='a b'>
    <node id='b' refs=' b c '/>
</root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        e ENTITY #REQUIRED>
    <!ENTITY entity1 "unparsed entity">
]>
<root e="entity1"/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        e ENTITY #REQUIRED>
]>
<root e="entity1"/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        e ENTITY #REQUIRED>
    <!ENTITY % entity1 "unparsed entity">
]>
<root e="entity1"/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        e ENTITIES #REQUIRED>
    <!ENTITY e1 "unparsed entity 1">
    <!ENTITY e2 "unparsed entity 2">
]>
<root e=" e1 e2 "/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        e ENTITIES #REQUIRED>
    <!ENTITY e1 "unparsed entity 1">
    <!ENTITY e2 "unparsed entity 2">
]>
<root e=" e1 e2 e3"/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        tok NMTOKEN #REQUIRED>
]>
<root tok='0a_01-x'/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        tok NMTOKEN #REQUIRED>
]>
<root tok='!'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        toks NMTOKENS #REQUIRED>
]>
<root toks=' a b1 c2 d.e '/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        toks NMTOKENS #REQUIRED>
]>
<root toks=' a b1 c2 d.e !'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a (yes|no) #REQUIRED>
]>
<root a='yes'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a (yes|no) #REQUIRED>
]>
<root a='no'/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a (yes|no) #REQUIRED>
]>
<root a='yes1'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a NOTATION (n1|n2) #REQUIRED>
    <!NOTATION n1>
    <!NOTATION n2>
]>
<root a='n1'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a NOTATION (n1|n2) #REQUIRED>
    <!NOTATION n1>
    <!NOTATION n2>
]>
<root a='n2'/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a NOTATION (n1|n2) #REQUIRED>
    <!NOTATION n1>
    <!NOTATION n2>
]>
<root a='n3'/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ANY>
    <!ATTLIST root
        a NOTATION (n1|n2) #REQUIRED>
    <!NOTATION n1>
]>
<root a='n2'/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root EMPTY>
]>
<root/>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root EMPTY>
]>
<root><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA)>
]>
<root/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA)>
]>
<root>pcdata</root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA)>
    <!ELEMENT node ANY>
]>
<root><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA|node)*>
    <!ELEMENT node ANY>
]>
<root>pcdata</root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA|node)*>
    <!ELEMENT node ANY>
]>
<root><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA|node)*>
    <!ELEMENT node ANY>
]>
<root>pcdata<node/>pcdata</root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (#PCDATA|n1)*>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
]>
<root>pcdata<n1/>pcdata<n2/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node?)>
    <!ELEMENT node ANY>
]>
<root/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node?)>
    <!ELEMENT node ANY>
]>
<root><node/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (node?)>
    <!ELEMENT node ANY>
]>
<root><node/><node/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (node+)>
    <!ELEMENT node ANY>
]>
<root/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node+)>
    <!ELEMENT node ANY>
]>
<root><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node+)>
    <!ELEMENT node ANY>
]>
<root><node/><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node*)>
    <!ELEMENT node ANY>
]>
<root/>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node*)>
    <!ELEMENT node ANY>
]>
<root><node/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (node*)>
    <!ELEMENT node ANY>
]>
<root><node/><node/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (node*)>
    <!ELEMENT node ANY>
]>
<root><node/><node/><root/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (n1,n2,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n2/><n3/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (n1,n2,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n3/><n2/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (n1,n2,n3)+>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n2/><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root (n1,n2,n3)+>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n2/><n3/><n1/><n2/><n3/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root (n1,n2,n3)+>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n2/><n3/><n1/><n2/><n2/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n2/><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n1/><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n2/><n2/><n3/></root>
'')

validate_ok(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n2/><n3/></root>
'')

validate_fail(''
<!DOCTYPE root [
    <!ELEMENT root ((n1|n2)*,n3)>
    <!ELEMENT n1 ANY>
    <!ELEMENT n2 ANY>
    <!ELEMENT n3 ANY>
]>
<root><n1/><n3/><n2/></root>
'')

XML.from_str(''
<root xmlns="http://ox.org"/>
'', false, true).{
    (log.debug(JSON.to_str($, "  ")))
    (test($.root.ns == "http://ox.org"))
}

XML.from_str(''
<root xmlns="http://ox.org">
    <node/>
</root>
'', false, true).{
    (test($.root.ns == "http://ox.org"))
    (test($.root.content[0].ns == "http://ox.org"))
}

XML.from_str(''
<root xmlns="http://ox.org" xmlns:ns="http://ox.org/test">
    <ns:node a="1" ns:b="2"/>
</root>
'', false, true).{
    (test($.root.ns == "http://ox.org"))
    (test($.root.content[0].ns == "http://ox.org/test"))
    (test($.root.content[0].short_tag == "node"))
    (test($.root.content[0].get_attr("a") == "1"))
    (test($.root.content[0].get_attr("b", "http://ox.org/test") == "2"))
}
