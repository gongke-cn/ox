ref "std/log"
ref "std/socket"

log: Log("socket")

addrs = SockAddr.lookup("gitee.com")
for addrs as addr {
    log.debug("address: {addr.addr}")
}
addr = addrs[0]

s = Socket(Socket.AF_INET, Socket.SOCK_STREAM, 0)
addr.port = 80

s.connect(addr)
log.debug("connected")

s.send("\
GET / HTTP/1.1
Host: gitee.com
User-Agent: curl/8.6.0
Accept: */*

")
log.debug("send")

buf = Int8(1024)
s.set_recv_timeout(1000)
while true {
    n = s.recv(buf, 0)
    if n == 0 {
        break
    }
    log.debug("recv: {buf.$to_str(0, n)}")
}

s.$close()

//Non-block mode.
s = Socket(Socket.AF_INET, Socket.SOCK_STREAM, 0)
s.set_nonblock(true)
s.connect(addr)

State: class {
    enum {
        CONNECT
        SEND
        RECV
        END
    }
}

state = State.CONNECT

send_buf = "\
GET / HTTP/1.1
Host: gitee.com
User-Agent: curl/8.6.0
Accept: */*

".to_uint8_ptr()
send_pos = 0
send_left = C.get_length(send_buf)

recv_buf = Int8(1024)

sm = SockMan(func(sock, event) {
    if event & SockMan.EVENT_IN {
        n = s.recv(recv_buf, Socket.MSG_DONTWAIT)

        log.debug("recv: {buf.$to_str(0, n)}")

        @state = State.END
    } elif event & SockMan.EVENT_OUT {
        if state == State.CONNECT {
            @state = State.SEND
        }

        n = s.send(send_buf, 0, send_pos, send_left)
        @send_pos += n
        @send_left -= n

        if send_left == 0 {
            @state = State.RECV
            sm.add(s, SockMan.EVENT_IN)
        }
    } else {
        log.error("socket error")
        @state = State.END
    }
})

sm.add(s, SockMan.EVENT_OUT)

while state != State.END {
    sm.process(5000)
}
