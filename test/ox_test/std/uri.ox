ref "../test"
ref "std/log"
ref "std/uri"

log: Log("uri")

uri_test: func(s, sch, ui, host, port, path, query, frag) {
    uri = URI(s)

    //log.debug("scheme: {uri.scheme} userinfo: {uri.userinfo} host: {uri.host} port: {uri.port} path: {uri.path} query: {uri.query} fragment: {uri.fragment}")

    if uri.scheme != sch ||
            uri.userinfo != ui ||
            uri.host != host ||
            uri.port != port ||
            uri.path != path ||
            uri.query != query ||
            uri.fragment != frag {
        test(false, "URI {s} test failed")
    }

    if uri.$to_str() != s {
        log.debug("uri: {uri.$to_str()}")
        test(false, "URI {s} $to_str() test failed")
    }
}

uri_test("http://a.b.c", "http", null, "a.b.c")
uri_test("http://username@a.b.c:8080?query#f1", "http", "username", "a.b.c", 8080, null, "query", "f1")
uri_test("file:/root/dir/file?query#f1", "file", null, null, null, "/root/dir/file", "query", "f1")
uri_test("file:/%23path", "file", null, null, null, "/#path")
uri_test("file:path", "file", null, null, null, "path")
uri_test("path#fragment", null, null, null, null, "path", null, "fragment")