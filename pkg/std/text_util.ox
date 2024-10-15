/*?
 *? @lib Text process utilities.
 */

ref "./log"

log: Log("text_util")
log.level = Log.WARNING

/*?
 *? Replace the characters in string to C escape characters.
 *? @param text {String} The original string.
 *? @return {String} The string with C escape characters.
 */
public c_escape: func(text) {
    return text.replace(/[\\"'\x00-\x1f\x7f]/, func(s) {
        case s.$to_str() {
        "\n" {
            return "\\n"
        }
        "\r" {
            return "\\r"
        }
        "\v" {
            return "\\v"
        }
        "\f" {
            return "\\f"
        }
        "\a" {
            return "\\a"
        }
        "\b" {
            return "\\b"
        }
        "\"" {
            return "\\\""
        }
        "\\" {
            return "\\\\"
        }
        * {
            c = s.char_at(0)
            if ((c >= 0) && (c <= 0x06)) ||
                ((c >= 0x0e) && (c <= 0x1f)) ||
                (c == 0x7f) {
                return "\\x{c!02x}"
            } else {
                return s
            }
        }
        }
    })
}

/*?
 *? Replace the characters in string to HTML escape characters.
 *? @param text {String} The original string.
 *? @return {String} The string with HTML escape characters.
 */
public html_escape: func(text) {
    return text.replace(/['"><&]/, func(s) {
        case s.$to_str() {
        "'" {
            return "&apos;"
        }
        "\"" {
            return "&quot;"
        }
        "<" {
            return "&lt;"
        }
        ">" {
            return "&gt;"
        }
        "&" {
            return "&amp;"
        }
        * {
            return s;
        }
        }
    })
}

get_prefix: func(text, skip_first) {
    s = 0
    skip = skip_first

    while true {
        e = text.lookup_char('\n', s)
        if e == -1 {
            if skip {
                return null
            } else {
                m = text.match(/^(\s)*.*$/m, s)
                return m.groups[1]
            }
            break
        }

        skip = false
        s = e + 1
    }
}

append_with_prefix: func(sb, prefix, text) {
    s = 0
    first = true

    while true {
        e = text.lookup_char('\n', s)
        if e == -1 {
            line = text.slice(s)
        } else {
            line = text.slice(s, e + 1)
        }

        if first {
            sb.append(line)
        } else {
            sb.append(prefix)
            sb.append(line)
        }

        if e == -1 {
            break
        }

        s = e + 1
        first = false
    }
}

/*?
 *? Indent string generater.
 *? Use "indent" string generater, the multi line items will be replaced with prefixed indent.
 *? For example:
 *? @code{
 *? a = "line 1\nline 2"
 *? text = indent ''
 *?     {a}
 *? ''
 *? @code}
 *? The text is eqaul to
 *? @code{
 *? ''
 *?     line 1
 *?     line 2
 *? ''
 *? @code}
 *? @param templ The string template.
 */
public indent: func(templ) {
    sb = String.Builder()
    i = 1
    prefix = ""
    skip_first = false

    for templ as item {
        sb.append(item)

        if (nprefix = get_prefix(item, skip_first)) {
            prefix = nprefix
        }

        t = argv[i]
        if t {
            append_with_prefix(sb, prefix, t)

            if t.length && t[t.length - 1] == '\n' {
                skip_first = true
            }
        }

        i += 1
    }

    return sb.$to_str()
}
