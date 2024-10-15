ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

{
    name: "http"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "HTTP protocol"
        zh: "HTTP超文本传输协议"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std", "ssl")
    executables: [
        "server"
    ]
    internal_libraries: [
        "http_server_listen_sock"
        "http_server_session"
        "https_server_sock"
        "server_log"
        "ssl_error"
        "http_reason"
        "http_server"
        "http_server_sock"
        "mime"
    ]
}
