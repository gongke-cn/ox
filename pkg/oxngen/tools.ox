ref "std/io"
ref "./log"

//Output warning message.
public warning: func(msg) {
    if stderr.is_tty {
        cb = "\x1b[35;1m"
        ce = "\x1b[0m"
    } else {
        cb = ""
        ce = ""
    }

    stderr.puts(L"{cb}warning{ce}: {msg}\n")
}
