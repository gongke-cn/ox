ref "ssl/err"

//SSL error.
public SslError: class Error {
    $init(msg) {
        n = ERR_get_error()
        buf = Int8(256)
        ERR_error_string_n(n, buf, C.get_length(buf))
        emsg = buf.$to_str()

        Error.$inf.$init.call(this, "{msg}: {emsg}")
    }
}