#!/usr/bin/ox

ref "std/io" *
ref "std/log" *

log: Log("punct")

puncts: [
    "<<"
    ">>"
    ">>>"
    "=="
    "!="
    ">="
    "<="
    "**"
    "||"
    "&&"
    "+="
    "-="
    "*="
    "**="
    "/="
    "%="
    "|="
    "&="
    "^="
    "<<="
    ">>="
    ">>>="
    "||="
    "&&="
    "~="
    ".+="
    ".-="
    ".*="
    ".**="
    "./="
    ".%="
    ".|="
    ".&="
    ".^="
    ".<<="
    ".>>="
    ".>>>="
    ".||="
    ".&&="
    ".~="
    "..."
    "=>"
    ":>"
]

char_names: {
    "<": "lt"
    ">": "gt"
    "=": "eq"
    "!": "bang"
    "|": "pipe"
    "&": "amp"
    "%": "percent"
    "^": "caret"
    "+": "plus"
    "-": "minus"
    "*": "star"
    "/": "slash"
    ".": "dot"
    ":": "colon"
    "~": "tilde"
}

punct_name: func(p)
{
    return p.$iter().map((char_names[$])).$to_str("_")
}

gen_def: func() {
    stdout.puts(''
    OX_TOKEN_PUNCT_BEGIN,
{{puncts.$iter().map(("    OX_TOKEN_{punct_name($)}")).$to_str(",\n")}}
    '')
}

gen_tab: func() {
    nodes = []
    edges = []

    add_node: func() {
        n = {id: nodes.length, edges: {}, punct: "OX_TOKEN_END"};
        nodes.push(n);
        return n;
    }

    add_edge: func(src, c, dst) {
        e = {char: c, node: dst};
        src.edges[c] = e;
        return e;
    }

    add_node();
    for puncts as punct {
        n = nodes[0]
        for punct as c {
            e = n.edges[c]
            if !e {
                dst = add_node()
                e = add_edge(n, c, dst)
            }
            n = e.node
        }
        n.punct = "OX_TOKEN_{punct_name(punct)}"
    }

    for nodes as n {
        nedges = Object.values(n.edges).to_array()

        n.first_edge = edges.length
        n.nedge = nedges.length

        for nedges as e {
            e.id = edges.length
            edges.push(e)
        }
    }

    node_code = nodes.$iter().map(("    \{{$.first_edge}, {$.nedge}, {$.punct}\}")).$to_str(",\n")
    edge_code = edges.$iter().map(("    \{'{$.char}', {$.node.id}\}")).$to_str(",\n")
    punct_names = puncts.$iter().map(("    \"{$}\"")).$to_str(",\n")

    stdout.puts(''
static const OX_PunctNode
punct_nodes[] = {
{{node_code}}
};

static const OX_PunctEdge
punct_edges[] = {
{{edge_code}}
};

static const char*
puncts[] = {
{{punct_names}}
};
    '')
}

if argv[1] == "-d" {
    gen_def()
} else {
    gen_tab()
}