ref "std/fs"
ref "std/path"
ref "std/system"
ref "std/io"
ref "std/lang"
ref "std/temp_file"
ref "std/shell"
ref "json"
ref "./tools"
ref "./log"

macro_match: func(ctxt, line)
{
    match = line.match(/(\S+)(.*)/)
    if match {
        name = match.groups[1].trim()
        data = match.groups[2].trim()

        if data == "" {
            return null
        }

        match = name.match(/(\w+)\((.*)\)/)
        if match {
            name = match.groups[1]
            args = match.groups[2].split(",").map(($.trim())).to_array()
        }

        if !ctxt.macro_match(name) {
            return null
        }

        return {
            name
            args
            data
        }
    }

    return null
}

parse: func(ctxt, fn) {
    #file = fn.open()
    filter_matched = false

    macros = []
    ctxt.cmacros = macros
    macro_set = Set()

    while true {
        line = file.gets()
        if line == null {
            break
        }

        match = line.match(/^# \d+ \"(.+)\"/)
        if match {
            filter_matched = ctxt.file_match(match.groups[1])
        } elif filter_matched {
            match = line.match(/^#define\s+(.+)/)
            if match {
                data = match.groups[1]
                macro = macro_match(ctxt, data)

                if macro {
                    if !macro_set.has(macro.name) {
                        macros.push(macro)
                        macro_set.add(macro.name)
                    } else {
                        warning(L"macro \"{macro.name}\" is already defined")
                    }
                }
            }
        }
    }
}

public get_cmacros: func(ctxt) {
    #outfile = TempFile("{ctxt.outdir}/{ctxt.target_name}.cmacros")
    cflags = ctxt.cflags()

    cmd = "clang {cflags} -E -dD {ctxt.input_file} > {outfile}"

    log.debug("exec \"{cmd}\"")
    r = Shell.run(cmd, Shell.ERROR)
    if r != 0 {
        throw Error(L"run \"{cmd}\" failed")
    }

    parse(ctxt, outfile)

    log.debug(JSON.to_str(ctxt.cmacros, "  "))
}
