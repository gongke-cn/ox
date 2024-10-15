ref "std/char"
ref "./error"
ref "./log"

//Document input.
public Input: class {
    //Initialize the document input.
    $init(line, col, text) {
        this.line = line
        this.column = col - 1
        this.s = text
        this.pos = 0
    }

    //Get a character from the input.
    getc() {
        if this.pos >= this.s.length {
            return null
        }

        c = this.s.char_at(this.pos)
        this.pos += 1

        if this.last_is_lt {
            this.line += 1
            this.column = 0
            this.last_is_lt = false
        }

        this.column += 1

        if c == '\n' {
            this.last_is_lt = true
        }

        return c
    }

    //Push back a character to the input.
    ungetc(c) {
        if c != null {
            this.pos -= 1
            this.column -= 1
            this.last_is_lt = false
        }
    }

    //Eatup space.
    eatup_space() {
        while true {
            c = this.getc()
            if c == null || !isspace(c) {
                this.ungetc(c)
                break
            }
        }
    }

    //Eatup space without newline.
    eatup_space_no_lt() {
        while true {
            c = this.getc()
            if c == null || !isspace(c) {
                this.ungetc(c)
                break
            }
        }
    }

    //Output error message.
    error(loc, msg) {
        if !loc {
            loc = this.first_loc()
            this.last_loc(loc)
        }

        error(loc, msg)

        throw SyntaxError(msg)
    }

    //Get the first character's location.
    first_loc() {
        return {
            first_line: this.line
            first_column: this.column
        }
    }

    //Get the last character's location.
    last_loc(loc) {
        loc.last_line = this.line
        loc.last_column = this.column
    }

    //Get command from the input.
    get_cmd(can_be_end) {
        if this.cmd {
            cmd = this.cmd
            this.cmd = null
            return cmd
        }

        this.eatup_space()

        c = this.getc()
        if can_be_end && !c {
            return null
        }

        if c != '@' {
            this.error(null, L"expect `@' here")
        }

        loc = this.first_loc()
        sb = String.Builder()

        while true {
            c = this.getc()
            if c == null || isspace(c) {
                this.ungetc(c)
                break
            }

            sb.append_char(c)
        }

        this.last_loc(loc)

        return {
            loc
            text: sb.$to_str()
        }
    }

    //Push back a command to the input.
    unget_cmd(cmd) {
        this.cmd = cmd
    }

    //Get a word from the input.
    get_word() {
        this.eatup_space_no_lt()

        loc = this.first_loc()
        sb = String.Builder()

        while true {
            c = this.getc()
            if c == null || isspace(c) {
                this.ungetc(c)
                break
            }

            sb.append_char(c)
        }

        this.last_loc(loc)

        if sb.length == 0 {
            this.error(loc, L"expect a word here")
        }

        return {
            loc
            text: sb.$to_str()
        }
    }

    //Get parameters list from the input
    get_plist() {
        this.eatup_space_no_lt()

        loc = this.first_loc()
        sb = String.Builder()

        c = this.getc()
        if c != '(' {
            this.ungetc(c)
            return null
        }

        level = 0

        while true {
            c = this.getc()
            if c == ')' {
                if level == 0 {
                    break
                } else {
                    level += 1
                }
            } elif c == '(' {
                if level > 0 {
                    level -= 1
                }
            } elif c == null {
                this.last_loc(loc)
                this.error(loc, L"expect `)' at end of type")
            }

            sb.append_char(c)
        }

        return {
            loc
            text: sb.$to_str()
        }
    }

    //Get type definition from the input
    get_type() {
        this.eatup_space_no_lt()

        loc = this.first_loc()
        sb = String.Builder()

        c = this.getc()
        if c != '{' {
            this.ungetc(c)
            return null
        }

        while true {
            c = this.getc()
            if c == '}' {
                break
            }
            if c == null {
                this.last_loc(loc)
                this.error(loc, L"expect `}' at end of type")
            }

            sb.append_char(c)
        }

        return {
            loc
            text: sb.$to_str()
        }
    }

    //Get value definition from the input
    get_value() {
        this.eatup_space_no_lt()

        loc = this.first_loc()
        sb = String.Builder()

        c = this.getc()
        if c != '=' {
            this.ungetc(c)
            return null
        }

        c = this.getc()
        if c == '{' {
            while true {
                c = this.getc()
                if c == '}' {
                    break
                }
                if c == null {
                    this.last_loc(loc)
                    this.error(loc, L"expect `}' at end of value")
                }

                sb.append_char(c)
            }
        } else {
            this.ungetc(c)

            return this.get_word()
        }

        return {
            loc
            text: sb.$to_str()
        }
    }

    //Get code from input.
    get_code() {
        sb = String.Builder()

        while true {
            c = this.getc()
            if c == null {
                break
            } elif c == '@' {
                nc = this.getc()
                if nc == '@' {
                    sb.append_char('@')
                } else {
                    this.ungetc(nc)
                    this.ungetc(c)
                    break
                }
            } else {
                sb.append_char(c)
            }
        }

        lines = sb.$to_str().split("\n").to_array()

        while lines.length {
            line = lines[0]

            if line ~ /\s*/p {
                lines.remove(0, 1)
            } else {
                break
            }
        }

        while lines.length {
            line = lines[lines.length - 1]

            if line ~ /\s*/p {
                lines.remove(lines.length - 1)
            } else {
                break
            }
        }

        if lines.length {
            line = lines[0]
            sp = line ~ /^\s+/
            if sp {
                lines = lines.$iter().map(func(line) {
                    if line.slice(0, sp.length) == sp {
                        line = line.slice(sp.length)
                    }

                    return line
                }).to_array()
            }
        }

        return lines.$iter().$to_str("\n")
    }
}
