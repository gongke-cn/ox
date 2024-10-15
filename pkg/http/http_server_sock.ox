ref "std/socket"
ref "std/fiber"
ref "std/time"
ref "./http_server_session"
ref "./server_log"

//? HTTP server session socket.
public HttpServerSock: class Socket {
    //Accept a new session socket.
    static accept(server, sock, addr) {
        Object.set_inf(sock, HttpServerSock.$inf)

        sock.$init(server, addr)

        sock.start_session()
    }

    //Initialize the socket.
    $init(server, addr) {
        this.server = server
        this.addr = addr
        this.start_time = Time()
    }

    //Close the socket.
    $close() {
        this.server.remove(this)
        Socket.$inf.$close.call(this)
        log.info("close {this.addr.addr}:{this.addr.port}")
    }

    //Set the socket's current state.
    set_state(state) {
        if this.state != state {
            this.state = state
            this.server.add(this, state)
        }
    }

    //Start the HTTP server session.
    start_session() {
        sess = HttpServerSession(this)

        this.session_fiber = Fiber(sess.process, sess)
        if this.session_fiber.end {
            this.$close()
        }
    }

    //Run the HTTP server session.
    run_session() {
        if this.session_fiber.end {
            this.$close()
        } else {
            this.session_fiber.next()

            if this.session_fiber.end {
                this.$close()
            }
        }
    }

    //Solve the socket event.
    on_event(event) {
        try {
            if event & SockMan.EVENT_ERR {
                this.$close()
                return
            }

            this.run_session()
        } catch e {
            this.$close()
            throw e
        }
    }

    //Read data from the socket.
    read(buf, pos, len) {
        this.set_state(SockMan.EVENT_IN)

        while true {
            n = this.recv(buf, Socket.MSG_DONTWAIT, pos, len)
            if n == 0 {
                yield
            } else {
                return n
            }
        }
    }

    //Write data to the socket.
    write(buf, pos, len) {
        this.set_state(SockMan.EVENT_OUT)

        left = len
        while left > 0 {
            n = this.send(buf, Socket.MSG_DONTWAIT|Socket.MSG_NOSIGNAL, pos, left)
            if n == 0 {
                yield
            }

            pos += n
            left -= n
        }

        return len
    }
}