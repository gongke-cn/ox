ref "std/socket"
ref "./http_server_sock"
ref "./https_server_sock"
ref "./server_log"

//? HTTP server's listen socket.
public HttpServerListenSock: class Socket {
    //Initialize the listen socket.
    $init(server) {
        Socket.$inf.$init.call(this, Socket.AF_INET, Socket.SOCK_STREAM, 0)

        this.server = server

        addr = SockAddr(Socket.AF_INET)
        addr.addr = SockAddr.INADDR_ANY
        addr.port = server.port

        this.set_nonblock(true)
        this.bind(addr)
        this.listen(server.backlog)
    }

    //On socket event.
    on_event(event) {
        if event & SockMan.EVENT_IN {
            addr = SockAddr(Socket.AF_INET)

            sock = this.accept(addr)
            log.info("accept {addr.addr}:{addr.port}")

            try {
                if this.server.is_https {
                    HttpsServerSock.accept(this.server, sock, addr)
                } else {
                    HttpServerSock.accept(this.server, sock, addr)
                }
            } catch e {
                sock.$close()
                throw e
            }
        }
    }
}