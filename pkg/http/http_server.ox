ref "std/socket"
ref "std/system"
ref "std/time"
ref "ssl"
ref "./http_server_listen_sock"
ref "./ssl_error"
ref "./server_log"

//? HTTP server.
public HttpServer: class {
    //?Initialize the HTTP server.
    $init() {
        this.port = 80
        this.backlog = 20
        this.root = getcwd()
        this.script_dirs = []
        this.socks = Set()
    }

    //? Startup the HTTPS server.
    startup_https() {
        SSL_load_error_strings()
        SSL_library_init()

        if (this.ssl_ctx = SSL_CTX_new(TLS_server_method())) {
            throw SslError("SSL_CTX_new")
        }

        if SSL_CTX_use_certificate_file(this.ssl_ctx, this.cert_file, SSL_FILETYPE_PEM) <= 0 {
            throw SslError("SSL_CTX_use_certificate_file \"{this.cert_file}\"")
        }

        if SSL_CTX_use_PrivateKey_file(this.ssl_ctx, this.key_file, SSL_FILETYPE_PEM) <= 0 {
            throw SslError("SSL_CTX_use_PrivateKey_file \"{this.key_file}\"")
        }

        this.is_https = true
    }

    //? Startup the HTTP/HTTPS server.
    startup() {
        log.info("startup on port {this.port}")

        try {
            if this.cert_file {
                this.startup_https()
            }

            this.sock_man = SockMan(func(sock, event) {
                sock.on_event(event)
            })

            sock = HttpServerListenSock(this)
            this.sock_man.add(sock, SockMan.EVENT_IN)
        } catch e {
            log.error("uncaught error: {e}")
            throw e
        }

        while true {
            try {
                this.sock_man.process(1000)

                if this.timeout {
                    now = Time()
                    for this.socks as sock {
                        if now - sock.start_time >= this.timeout * 1000 {
                            log.info("{sock.addr.addr}:{sock.addr.port} timeout")
                            this.remove(sock)
                            sock.$close()
                        } else {
                            break
                        }
                    }
                }
            } catch e {
                log.error("uncaught error: {e}")
            }
        }
    }

    //? Add a socket to the server.
    add(sock, event) {
        this.sock_man.add(sock, event)
        this.socks.add(sock)
    }

    //? Remove a socket to the server.
    remove(sock) {
        this.sock_man.remove(sock)
        this.socks.remove(sock)
    }
}
