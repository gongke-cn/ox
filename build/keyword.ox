#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("keyword")

keywords: [
    "null"
    "true"
    "false"
    "func"
    "class"
    "static"
    "enum"
    "bitfield"
    "if"
    "elif"
    "else"
    "while"
    "do"
    "for"
    "as"
    "case"
    "break"
    "continue"
    "return"
    "try"
    "catch"
    "finally"
    "throw"
    "typeof"
    "instof"
    "ref"
    "public"
    "this"
    "argv"
    "sched"
    "yield"
    "global"
    "owned"
    "textdomain"
]

gen_def: func() {
    stdout.puts("\
typedef enum \{
    OX_KEYWORD_NONE  = -1,
    OX_KEYWORD_BEGIN = 512,
{keywords.$iter().map(("    OX_KEYWORD_{$}")).$to_str(",\n")}
\} OX_Keyword;
"   )
}

gen_tab: func() {
    nodes = []
    edges = []

    add_node: func() {
        n = {id: nodes.length, edges: {}, token: "OX_KEYWORD_NONE"}
        nodes.push(n)
        return n
    }

    add_edge: func(src, c, dst) {
        e = {char:c, node: dst}
        src.edges[c] = e
        return e
    }

    add_node()
    for keywords as keyword {
        n = nodes[0]
        for keyword as c {
            e = n.edges[c]
            if !e {
                d = add_node()
                e = add_edge(n, c, d)
            }
            n = e.node
        }
        n.token = "OX_KEYWORD_{keyword}"
    }

    for nodes as n {
        n.nedge = Object.keys(n.edges).to_array().length
        n.first_edge = edges.length
        for Object.values(n.edges) as e {
            e.id = edges.length
            edges.push(e)
        }
    }

    node_code = nodes.$iter().map(("    \{{$.first_edge}, {$.nedge}, {$.token}\}")).$to_str(",\n");
    edge_code = edges.$iter().map(("    \{'{$.char}', {$.node.id}\}")).$to_str(",\n");
    name_code = keywords.$iter().map(("    \"{$}\"")).$to_str(",\n")

    stdout.puts(''
static const OX_KeywordNode
keyword_nodes[] = {
{{node_code}}
};

static const OX_KeywordEdge
keyword_edges[] = {
{{edge_code}}
};

static const char*
keywords[] = {
{{name_code}}
};
    '')
}

if argv[1] == "-d" {
    gen_def()
} else {
    gen_tab()
}
