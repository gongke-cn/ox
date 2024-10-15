ref "std/socket"
ref "ssl/ssl"
ref "./http_server_sock"
ref "./ssl_error"
ref "./server_log"

//? HTTPS server session socket.
public HttpsServerSock: class HttpServerSock {
    //Accept a new session socket.
    static accept(server, sock, addr) {
        Object.set_inf(sock, HttpsServerSock.$inf)

        sock.$init(server, addr)
        
        if !(sock.ssl = SSL_new(server.ssl_ctx)) {
            throw SslError("SSL_new")
        }

        SSL_set_fd(sock.ssl, sock.fd)

        r = SSL_accept(sock.ssl)
        if r == 1 {
            sock.accepted = true
            sock.start_session()
        } else {
            e = SSL_get_error(sock.ssl, r)
            if e == SSL_ERROR_WANT_READ {
                sock.set_state(SockMan.EVENT_IN)
            } elif e == SSL_ERROR_WANT_WRITE {
                sock.set_state(SockMan.EVENT_OUT)
            } else {
                throw SslError("SSL_accept")
            }
        }
    }

    //Close the SSL socket.
    $close() {
        SSL_free(this.ssl)
        HttpServerSock.$inf.$close.call(this)
    }

    //Solve the socket event.
    on_event(event) {
        try {
            if event & SockMan.EVENT_ERR {
                this.$close()
                return
            }

            if (!this.accepted) {
                r = SSL_accept(this.ssl)
                if r == 1 {
                    this.accepted = true
                    this.start_session()
                } else {
                    e = SSL_get_error(this.ssl, r)
                    if e == SSL_ERROR_WANT_READ {
                        this.set_state(SockMan.EVENT_IN)
                    } elif e == SSL_ERROR_WANT_WRITE {
                        this.set_state(SockMan.EVENT_OUT)
                    } else {
                        throw SslError("SSL_accept")
                    }
                }
            } else {
                this.run_session()
            }
        } catch e {
            this.$close()
            throw e
        }
    }

    //Read data from the socket.
    read(buf, pos, len) {
        while true {
            n = SSL_read(this.ssl, C.get_ref(buf, pos, len), len)
            if n == -1 {
                e = SSL_get_error(this.ssl, r)
                if e == SSL_ERROR_WANT_READ {
                    this.set_state(SockMan.EVENT_IN)
                    yield
                } elif e == SSL_ERROR_WANT_WRITE {
                    this.set_state(SockMan.EVENT_OUT)
                    yield
                } else {
                    throw SslError("SSL_read")
                }
            } else {
                return n
            }
        }
    }

    //Write data to the socket.
    write(buf, pos, len) {
        left = len

        while left > 0 {
            n = SSL_write(this.ssl, C.get_ref(buf, pos, left), left)
            if n == -1 {
                e = SSL_get_error(this.ssl, r)
                if e == SSL_ERROR_WANT_READ {
                    this.set_state(SockMan.EVENT_IN)
                    yield
                } elif e == SSL_ERROR_WANT_WRITE {
                    this.set_state(SockMan.EVENT_OUT)
                    yield
                } else {
                    throw SslError("SSL_write")
                }
            } else {
                pos += n
                left -= n
            }
        }

        return len
    }
}
