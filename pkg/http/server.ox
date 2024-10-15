ref "std/option"
ref "std/io"
ref "std/path"
ref "./http_server"
ref "./server_log"

/*?
 *? @package http HTTP protocal.
 *? @exe HTTP/HTTPS server.
 */

show_usage: func {
    stdout.puts(L''
Usage: ox -r http/server [OPTION]...
Option:
{{options.help()}}
    '')
}

server = HttpServer()

options: Option([
    {
        long: "backlog"
        arg: Option.INTEGER
        help: L"Set the maximum length to which the queue of pending connections"
        on_option: func(opt, arg) {
            server.backlog = arg
        }
    }
    {
        long: "cert"
        arg: Option.STRING
        help: L"Set the HTTPS cerification key file"
        on_option: func(opt, arg) {
            server.cert_file = arg
        }
    }
    {
        long: "key"
        arg: Option.STRING
        help: L"Set the HTTPS private key file"
        on_option: func(opt, arg) {
            server.key_file = arg
        }
    }
    {
        long: "port"
        arg: Option.INTEGER
        help: L"Set the port number"
        on_option: func(opt, arg) {
            server.port = arg
        }
    }
    {
        long: "root"
        arg: Option.STRING
        help: L"Set the root directory"
        on_option: func(opt, arg) {
            server.root = fullpath(arg)
        }
    }
    {
        short: "s"
        arg: Option.STRING
        help: L"Add script directory"
        on_option: func(opt, arg) {
            server.script_dirs.push(fullpath(arg))
        }
    }
    {
        long: "timeout"
        arg: Option.INTEGER
        help: L"Set the HTTP session timeout in seconds"
        on_option: func(opt, arg) {
            server.timeout = arg
        }
    }
    {
        long: "help"
        help: L"Show this help message"
        on_option: func {
            show_usage()
            server.show_help = true
        }
    }
])

if !options.parse(argv) {
    return 1
}

if server.show_help {
    return 0
}

if server.cert_file {
    if !server.key_file {
        stderr.puts(L"private key is not specified\n")
        return 1
    }
}

if server.key_file {
    if !server.cert_file {
        stderr.puts(L"certification key is not specified\n")
        return 1
    }
}

//Startup the server.
server.startup()
