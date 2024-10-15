ref "std/uri"
ref "std/path"
ref "std/io"
ref "std/lang"
ref "std/math"
ref "./http_reason"
ref "./mime"
ref "./server_log"

//HTTP error.
HttpError: class Error {
    //Initialize a HTTP error.
    $init(code, msg) {
        this.code = code
        Error.$inf.$init.call(this, msg)
    }
}

//HTTP server session.
public HttpServerSession: class {
    //Initialize a HTTP session.
    $init(sock) {
        this.sock = sock
        this.server = sock.server
        this.recv_buf = UInt8(4096)
        this.recv_start = 0
        this.recv_end = 0
        this.request = {
            headers: {}
        }
        this.response = {
            code: 200
            headers: {}
        }
    }

    //? {String} The address string of the session.
    addr {
        return "{this.sock.addr.addr}:{this.sock.addr.port}"
    }

    /*?
     *? {Object} The parameters object of the request.
     *? The properties' keys are the parameters' names.
     *? The properties' values are the parameters' values.
     */
    p {
        if this.#p {
            return this.#p
        }

        uri = URI(this.request.uri)
        query = uri.query
        this.#p = {}
        if query {
            for query.split("&") as pdef {
                if (m = pdef.match(/([^=]+)=(.*)/p)) {
                    pn = URI.decode(m.groups[1])
                    pv = URI.decode(m.groups[2])
                    this.#p[pn] = pv
                } else {
                    pn = URI.decode(pdef)
                    this.#p[pn] = true
                }
            }
        }

        data = this.request.data
        if data {
            s = data.$to_str()

            ct = this.request.headers["Content-Type"]

            case ct {
            "application/x-www-form-urlencoded" {
                for s.split("&") as pdef {
                    if (m = pdef.match(/([^=]+)=(.*)/p)) {
                        pn = URI.decode(m.groups[1])
                        pv = URI.decode(m.groups[2])
                        this.#p[pn] = pv
                    } else {
                        pn = URI.decode(pdef)
                        this.#p[pn] = true
                    }
                }
            }
            "application/json" {
                v = JSON.from_str(s)
                for Object.entries(v) as [pk, pv] {
                    this.#p[pk] = pv
                }
            }
            }
        }

        return this.#p
    }

    /*?
     *? The JSON value of the request.
     */
    json {
        if this.#json {
            return this.#json
        }

        data = this.request.data
        if !data {
            return null
        }

        ct = this.request.headers["Content-Type"]
        if ct != "application/json" {
            return null
        }
        
        s = data.$to_str()
        this.#json = JSON.from_str(s)

        return s
    }

    //Read a line from the socket.
    get_line() {
        sb = String.Builder()

        while true {
            for pos = this.recv_start; pos < this.recv_end; pos += 1 {
                c = this.recv_buf[pos]
                if c == '\n' {
                    len = pos - this.recv_start
                    if len > 0 && this.recv_buf[pos - 1] == '\r' {
                        len -= 1
                    }
                    sb.append(this.recv_buf.$to_str(this.recv_start, len))
                    line = sb.$to_str()

                    log.debug("{this.addr} => {line}")

                    this.recv_start = pos + 1
                    return line
                }
            }

            sb.append(this.recv_buf.$to_str(this.recv_start, this.recv_end))

            n = this.sock.read(this.recv_buf, 0, C.get_length(this.recv_buf))

            this.recv_start = 0
            this.recv_end = n
        }
    }

    //Parse the request line.
    request_line() {
        line = this.get_line()

        m = line.match(/\s*(\S+)\s+(\S+)\s+HTTP\/(\d+\.\d+)/i)
        if !m {
            throw HttpError(400, "illegal request line \"{line}\"")
        }

        this.request.method = m.groups[1].to_upper()
        this.request.uri = m.groups[2]
        this.request.version = m.groups[3]
    }

    //Parse the request headers.
    request_headers() {
        while true {
            line = this.get_line()
            if line.length == 0 {
                break
            }

            m = line.match(/\s*(.+)\s*:\s*(.+)\s*/)
            if !m {
                throw HttpError(400, "illegal request header \"{line}\"")
            }

            tag = m.groups[1]
            val = m.groups[2]

            old = this.request.headers[tag]
            if old {
                this.request.headers[tag] = "{old}, {val}"
            } else {
                this.request.headers[tag] = val
            }
        }
    }

    //Read entity.
    read_entity(len) {
        if !(entity = this.request.data) {
            entity = UInt8(len)
            this.request.data = entity
            off = 0
        } else {
            old_len = C.get_length(entity)
            nb = UInt8(old_len + len)
            C.copy(nb, 0, entity, 0, old_len)
            this.request.data = nb
            off = old_len
            entity = nb
        }

        left = len
        while left > 0 {
            if this.recv_start != this.recv_end {
                rn = min(this.recv_end - this.recv_start, left)
                C.copy(entity, off, this.recv_buf, this.recv_start, rn)
                this.recv_start += rn
            } else {
                rn = this.sock.read(entity, off, left)
                if rn == 0 {
                    yield
                }
            }

            left -= rn
            off += rn
        }
    }

    //Send response.
    send_response() {
        sb = String.Builder()
        sl = "HTTP/1.1 {this.response.code} {HttpReason.get(this.response.code)}"
        sb.append("{sl}\r\n")
        log.debug("{this.addr} <= {sl}")

        buf = this.response.data
        if buf && !this.response.headers["Content-Length"] {
            if buf instof String {
                len = buf.length
            } else {
                len = C.get_length(buf)
            }

            this.response_header("Content-Length", len.$to_str())
        }

        for Object.entries(this.response.headers) as [tag, val] {
            sb.append("{tag}: {val}\r\n")
            log.debug("{this.addr} <= {tag}: {val}")
        }

        sb.append("\r\n")
        resp = sb.$to_str()
        this.sock.write(resp.to_uint8_ptr(), 0, resp.length)

        if buf {
            if buf instof String {
                buf = buf.to_uint8_ptr()
            }

            this.sock.write(buf, 0, C.get_length(buf))
        }
    }

    //Add a response header.
    response_header(tag, val) {
        old = this.response.headers[tag]
        if old {
            this.response.headers[tag] = "{old}, {val}"
        } else {
            this.response.headers[tag] = val
        }
    }

    //Send data file.
    send_data_file(path) {
        st = Path(path)
        if !st.exist || st.format != Path.FMT_REG{
            throw HttpError(404, "cannot find \"{path}\"")
        }

        pos = path.lookup_char_r('.')
        if pos != -1 {
            suffix = path.slice(pos + 1)
            if (mime = MIME.get(suffix)) {
                if mime ~ /^text\// {
                    tval = "{mime}; charset=UTF-8"
                } else {
                    tval = mime
                }

                this.response_header("Content-Type", tval)
            }
        }

        this.response_header("Content-Length", st.size.$to_str())
        this.send_response()

        #file = File(path, "rb")
        buf = UInt8(512 * 1024)

        while true {
            rn = file.read(buf, 0, C.get_length(buf))
            if rn == 0 {
                break
            }

            this.sock.write(buf, 0, rn)
        }

        log.info("{this.addr} <= file \"{path}\"")
    }

    //Rnu the script.
    run_script(path) {
        s = OX.script(path)

        if s.html {
            r = s.html(this)
            this.response_header("Content-Type", "text/html; charset=UTF-8")
            this.response.data = r
            this.send_response()
            log.debug("{this.addr} <= HTML {r}")
        } elif s.json {
            this.response_header("Content-Type", "application/json; charset=UTF-8")
            r = s.json(this)
            this.response.data = r
            this.send_response()
            log.debug("{this.addr} <= JSON {r}")
        } elif s.xml {
            this.response_header("Content-Type", "application/xml; charset=UTF-8")
            r = s.xml(this)
            this.response.data = r
            this.send_response()
            log.debug("{this.addr} <= XML {r}")
        } else {
            s.response(this)
        }

        log.info("{this.addr} run script \"{path}\"")
    }

    //Build the response.
    build_response() {
        path = URI(this.request.uri).path
        if path == "/" {
            if Path("{this.server.root}/index.ox").exist {
                path = "/index.ox"
            } else {
                path = "/index.html"
            }
        }

        if path.split("/").to_array().has("..") {
            throw HttpError(400, "illegal path {path}")
        }

        if path[0] == "/" {
            rpath = "{this.server.root}{path}"
        } else {
            rpath = "{this.server.root}/{path}"
        }

        if (req = this.request.headers["Origin"]) {
            this.response_header("Access-Control-Allow-Origin", req)
        }

        //Is script or normal file?
        is_script = false
        if path ~ /\.ox$/i {
            dir = dirname(rpath)
            for this.server.script_dirs as sdir {
                if dir == sdir {
                    is_script = true
                    break
                }
            }
        }

        if is_script {
            this.run_script(rpath)
        } else {
            this.send_data_file(rpath)
        }
    }

    //Process the session.
    process() {
        log.info("{this.addr}: HTTP session begin")

        try {
            //Read status line.
            this.request_line()

            //Read headers.
            this.request_headers()

            //Read entity.
            if this.request.headers["Content-Length"] {
                //Entity.
                len = Number(this.request.headers["Content-Length"])

                this.read_entity(len)
            } elif this.request.headers["Transfer-Encoding"] {
                enc = this.request.headers["Transfer-Encoding"]
                if enc != "chunked" {
                    throw HttpError(411, "\"Content-Length\" is not set")
                }

                //Chunked entity.
                while true {
                    line = this.get_line()
                    m = line.match(/^([0-9a-fA-F]+)/)
                    if !m {
                        this.error(400)
                    }

                    len = Number("0x{m.groups[1]}")
                    if len == 0 {
                        break
                    }

                    this.read_entity(len)
                }

                this.request_headers()
            }

            case this.request.method {
            "OPTIONS" {
                if (req = this.request.headers["Origin"]) {
                    this.response_header("Access-Control-Allow-Origin", req)
                }

                if (req = this.request.headers["Access-Control-Request-Method"]) {
                    this.response_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
                }

                if (req = this.request.headers["Access-Control-Request-Headers"]) {
                    this.response_header("Access-Control-Allow-Headers", req)
                }

                this.send_response()
            }
            "HEAD"
            "GET"
            "POST" {
                this.build_response()
            }
            * {
                throw HttpError(405, "unsupported method \"{this.request.method}\"")
            }
            }
        } catch e {
            if e instof HttpError {
                this.response.code = e.code
                this.response.data = ''
<html><body>
Error {{e.code}}: {{HttpReason.get(e.code)}}: {{e.message}}
</body></html>
                ''
                this.send_response()
            } else {
                throw e
            }
        }

        log.info("{this.addr}: HTTP session end")
    }
}
