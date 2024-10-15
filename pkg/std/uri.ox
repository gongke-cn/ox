/*?
 *? @lib URI parser.
 */

ref "./log"

log: Log("uri")
log.level = Log.WARNING

//Get the hexadecimal character's number value.
hex_char_val: func(c) {
    if (c >= '0') && (c <= '9') {
        return c - '0'
    } elif (c >= 'a') && (c <= 'f') {
        return c - 'a' + 10
    } else {
        return c - 'A' + 10
    }
}

/*?
 *? URI parser.
 */
public URI: class {
    /*?
     *? Encode mode.
     */
    enum EncMode {
        //? Encode the URI string.
        ENC_URI,
        //? Encode the URI compoment.
        ENC_COMP,
        //? Encode the path.
        ENC_PATH
    }

    /*?
     *? Convert the characters in URI to escape characters.
     *? @param s {String} The input string.
     *? @param mode {URI.EncMode} =URI.ENC_URI Encode mode.
     *? @return {String} The result string with escape characters.
     */
    static encode(s, mode = URI.ENC_URI) {
        rs = s.replace(/[^A-Za-z0-9\-_\.!~*'()]/, func(si) {
            c = si.$to_str()
            case si.$to_str() {
            "/" {
                if mode == URI.ENC_URI || mode == URI.ENC_PATH {
                    return c
                }
            }
            ";"
            "?"
            ":"
            "@"
            "&"
            "="
            "+"
            "$"
            ","
            "#" {
                if mode == URI.ENC_URI {
                    return c
                }
            }
            }

            c = si.char_at(0)
            return "%{c!02x}"
        })

        return rs
    }

    /*?
     *? Convert the escape characters in URI to origin characters.
     *? @param s {String} The string with escape characters.
     *? @return {String} The origin string.
     */
    static decode(s) {
        return s.replace(/%[0-9a-fA-F]{2}/, func(si) {
            c1 = si.char_at(1)
            c2 = si.char_at(2)
            return String.from_char((hex_char_val(c1) << 4) | hex_char_val(c2))
        })
    }

    /*?
     *? Initialize an URI object.
     *? @param s {String} The source string of the URI.
     */
    $init(s) {
        p = s.lookup_char(':')
        if p != -1 {
            this.scheme = URI.decode(s.slice(0, p))
            left = s.slice(p + 1)
        } else {
            left = s
        }

        m = left.match(/[?#]/)
        if m {
            hier = URI.decode(left.slice(0, m.start))
            left = left.slice(m.end)

            if m.$to_str() == "?" {
                m = left.match(/#/)

                if m {
                    this.query = left.slice(0, m.start)
                    this.fragment = URI.decode(left.slice(m.end))
                } else {
                    this.query = left
                }
            } elif m.$to_str() == "#" {
                this.fragment = URI.decode(left)
            }
        } else {
            hier = left
        }

        if hier.char_at(0) == '/' && hier.char_at(1) == '/' {
            left = hier.slice(2)

            p = left.lookup_char('/')
            if p != -1 {
                this.path = URI.decode(left.slice(p))
                left = left.slice(0, p)
            }

            p = left.lookup_char('@')
            if p != -1 {
                this.userinfo = URI.decode(left.slice(0, p))
                left = left.slice(p + 1)
            }

            p = left.lookup_char(':')
            if p != -1 {
                this.port = Number(left.slice(p + 1))
                if this.port.isnan() || this.port.floor() != this.port {
                    throw TypeError(L"illegal port")
                }
                left = left.slice(0, p)
            }

            if left.length {
                this.host = URI.decode(left)
            }
        } elif hier.length {
            this.path = URI.decode(hier)
        }
    }

    /*?
     *? Convert the URI to string.
     *? @return {String} The result string.
     */
    $to_str() {
        sb = String.Builder()

        if this.scheme {
            sb.append("{URI.encode(this.scheme, URI.ENC_COMP)}:")
        }

        if this.host {
            sb.append("//")

            if this.userinfo {
                sb.append("{URI.encode(this.userinfo, URI.ENC_COMP)}@")
            }

            sb.append(URI.encode(this.host, URI.ENC_COMP))

            if this.port {
                sb.append(":{this.port}")
            }
        }

        if this.path {
            sb.append(URI.encode(this.path, URI.ENC_PATH))
        }

        if this.query {
            sb.append("?{this.query}")
        }

        if this.fragment {
            sb.append("#{URI.encode(this.fragment, URI.ENC_COMP)}")
        }

        return sb.$to_str()
    }
}
