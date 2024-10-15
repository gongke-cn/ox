ref "std/io"

//Convert the location to string.
loc_str: func(loc) {
    if !loc {
        return ""
    }

    if loc.first_line == loc.last_line {
        if loc.first_column == loc.last_column {
            lstr = "{loc.first_line}.{loc.first_column}"
        } else {
            lstr = "{loc.first_line}.{loc.first_column}-{loc.last_column}"
        }
    } else {
        lstr = "{loc.first_line}.{loc.first_column}-{loc.last_line}.{loc.last_column}"
    }

    return "{lstr}: "
}

curr_filename

//Set the current filename.
public set_filename: func(filename) {
    @curr_filename = filename
}

//Show document error message.
public error: func(loc, msg) {
    if stderr.is_tty {
        cb = "\x1b[31;1m"
        ce = "\x1b[0m"
    }

    stderr.puts(L"\"{curr_filename}\": {loc_str(loc)}{cb}error{ce}: {msg}\n")
}

//Show document warning message.
public warning: func(loc, msg) {
    if stderr.is_tty {
        cb = "\x1b[35;1m"
        ce = "\x1b[0m"
    }

    stderr.puts(L"\"{curr_filename}\": {loc_str(loc)}{cb}warning{ce}: {msg}\n")
}

//Show note message.
public note: func(loc, msg) {
    if stderr.is_tty {
        cb = "\x1b[36;1m"
        ce = "\x1b[0m"
    }

    stderr.puts(L"\"{curr_filename}\": {loc_str(loc)}{cb}note{ce}: {msg}\n")
}
