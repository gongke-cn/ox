ref "std/option"
ref "std/io"
ref "std/path"
ref "std/path_conv"
ref "std/lang"
ref "std/fs"
ref "std/system"
ref "std/shell"
ref "json"
ref "./cmacro"
ref "./cdecl"
ref "./oxnsrc"
ref "./log"

/*?
 *? @package oxngen Native OX module source generator.
 *? @exe Native OX module source generator.
 *? This program can scan the C header files and generate the native
 *? ox module's source code.
 *? Run the program as follows:
 *? @code{
 *? ox -r oxngen [OPTION]...
 *? @code}
 *? By defult "oxngen" read "build.ox" as native OX module description file.
 *? Or you can specify the description file use option "-i".
 */

//Show usage message.
usage: func {
    stdout.puts(L"\
Usage: ox -r oxngen [OPTION]...
OX native module source generater.
Option:
{options.help()}\
"   )
}

//Generator context.
ctxt = class {
    file_match(file) {
        match = true

        if file ~ /<.+>/p {
            return false
        }

        try {
            file = fullpath(file)
        } catch e {
        }

        if this.file_filters.length {
            match = false
            for this.file_filters as f {
                if f == file {
                    match = true
                    break
                }
            }
        }

        for this.rev_file_filters as f {
            if f == file {
                match = false
                break
            }
        }

        return match
    }

    decl_match(name) {
        match = true
        target = this.target

        if target.decl_filters?.length {
            match = false
            for target.decl_filters as f {
                if name ~ f == name {
                    match = true
                    break
                }
            }
        }

        if target.rev_decl_filters?.length {
            for target.rev_decl_filters as f {
                if name ~ f == name {
                    match = false
                    break
                }
            }
        }

        return match
    }

    macro_match(name) {
        match = true
        target = this.target

        if target.decl_filters?.length {
            match = false
            for target.decl_filters as f {
                if name ~ f == name {
                    match = true
                    break
                }
            }
        }

        if target.rev_decl_filters?.length {
            for target.rev_decl_filters as f {
                if name ~ f == name {
                    match = false
                    break
                }
            }
        }

        if !match {
            if target.number_macros {
                for target.number_macros as f {
                    if name ~ f == name {
                        return true
                    }
                }
            }

            if target.string_macros {
                for target.string_macros as f {
                    if name ~ f == name {
                        return true
                    }
                }
            }
        }

        return match
    }

    $init() {
        this.inc_dirs = []
        this.macros = []
    }

    cflags() {
        target = this.target
        config = this.config

        flags = "{config.cflags} {target.cflags}"

        if this.target_arch {
            flags += " -target {this.target_arch}"
        }

        flags += " {this.inc_dirs.$iter().map(("-I{$}")).$to_str(" ")}"
        flags += " {this.macros.$iter().map(("-D{$}")).$to_str(" ")}"

        if target.inc_dirs {
            flags += " {target.inc_dirs.$iter().map(("-I{$}")).$to_str(" ")}"
        }

        return flags
    }

    add_noname_record(loc, decl) {
        return add_noname_record(ctxt, loc, decl)
    }
}()

input_file
show_help = false
settings = {
    pc: "pkg-config"
    p: {}
}

//?
options: Option([
    {
        short: "i"
        arg: Option.STRING
        help: L"Set input native module description filename"
        on_option: func(opt, arg) {
            @input_file = arg
        }
    }
    {
        short: "I"
        arg: Option.STRING
        help: L"Add an include directory"
        on_option: func(opt, arg) {
            ctxt.inc_dirs.push(arg)
        }
    }
    {
        short: "D"
        arg: Option.STRING
        help: L"Add a macro definition"
        on_option: func(opt, arg) {
            ctxt.macros.push(arg)
        }
    }
    {
        short: "l"
        help: L"List the oxn targets in the input file"
        on_option: func(opt, arg) {
            ctxt.list_targets = true
        }
    }
    {
        short: "o"
        arg: Option.STRING
        help: L"Set output directory"
        on_option: func(opt, arg) {
            ctxt.outdir = fullpath(arg)
        }
    }
    {
        short: "s"
        arg: Option.STRING
        help: L"Set a parameter"
        on_option: func(opt, arg) {
            m = arg.match(/(.+)=(.*)/p)
            if m {
                pn = m.groups[1]
                pv = m.groups[2]
            } else {
                pn = arg
                pv = true
            }

            settings.p[pn] = pv
        }
    }
    {
        short: "t"
        arg: Option.STRING
        help: L"Set the target architecture"
        on_option: func(opt, arg) {
            ctxt.target_arch = arg
        }
    }
    {
        long: "pc"
        arg: Option.STRING
        help: L"Set the program \"pkg-config\""
        on_option: func(opt, arg) {
            settings.pc = arg
        }
    }
    {
        long: "help"
        help: L"Show this help message"
        on_option: func(opt, arg) {
            usage()
            @show_help = true
        }
    }
])

if !options.parse(argv) {
    return 1
}

if show_help && !input_file {
    return 0
}

if !input_file {
    input_file = "build.ox"
}

//Load "oxngen.ox" file.
if !Path(input_file).exist {
    throw Error(L"\"{input_file}\" does not exist")
}

//Get the path of clang.
path_list = getenv("PATH")
if path_list {
    if OX.os == "windows" {
        exe = ".exe"
        sep = /;/
    } else {
        exe = ""
        sep = ":"
    }
    for path_list.split(sep) as path {
        clang_path = "{path}/clang{exe}"
        if Path(clang_path).exist {
            if basename(path) == "bin" {
                root_dir = dirname(path)
                ctxt.inc_dirs.push("{root_dir}/include")
            }
            break
        }
    }
}

//Get target.
cmd = "clang --dumpmachine"
if ctxt.target_arch {
    cmd = "{cmd} -target {ctxt.target_arch}"
}
settings.target = Shell.output(cmd).trim()

ctxt.config = OX.file(input_file)(settings)
log.debug(JSON.to_str(ctxt.config, "  "))

if !ctxt.outdir {
    ctxt.outdir = dirname(fullpath(input_file))
}

//Enter the directory
dir = dirname(input_file)
if dir != "." {
    chdir(dir)
}

//Lookup the input header file
lookup_input_file: func(ctxt, fn) {
    if Path(mixed_path(fn)).exist() {
        return fullpath(fn)
    }

    for ctxt.inc_dirs as dir {
        path = "{mixed_path(dir)}/{fn}"

        if Path(path).exist {
            return fullpath(path)
        }
    }

    target = ctxt.target
    if target.inc_dirs {
        for target.inc_dirs as dir {
            path = "{mixed_path(dir)}/{fn}"

            if Path(path).exist {
                return fullpath(path)
            }
        }
    }

    if target.cflags {
        for target.cflags.match_iter(/-I\s*(\S+)/) as m {
            dir = m.groups[1]
            path = "{mixed_path(dir)}/{fn}"

            if Path(path).exist {
                return fullpath(path)
            }
        }
    }

    path = mixed_path("/usr/include/{fn}")
    if Path(path).exist {
        return fullpath(path)
    }

    throw Error(L"cannot find input \"{fn}\"")
}

//Generate input file
gen_input_file: func(ctxt) {
    target = ctxt.target
    fn = "{ctxt.outdir}/{ctxt.target_name}.wrapper.h"
    file = File(fn, "wb")

    for target.input_files as f {
        file.puts("#include <{lookup_input_file(ctxt, f)}>\n")
    }

    file.$close()
    return fn
}

//Generate a OXN target.
gen_target: func(ctxt) {
    log.debug("generate {ctxt.target_name}")

    target = ctxt.target
    if target.input_files.length == 1 {
        ctxt.input_file = lookup_input_file(ctxt, target.input_files[0])
    } else {
        ctxt.input_file = gen_input_file(ctxt)
    }

    try {
        ctxt.cmacros = null
        ctxt.cdecls = null
        ctxt.file_filters = []
        ctxt.rev_file_filters = []
        ctxt.types = []
        ctxt.number_macros = []
        ctxt.string_macros = []

        if target.file_filters {
            for target.file_filters as f {
                ctxt.file_filters.push(lookup_input_file(ctxt, f))
            }
        }

        if target.rev_file_filters {
            for target.rev_file_filters as f {
                ctxt.rev_file_filters.push(lookup_input_file(ctxt, f))
            }
        }

        if ctxt.file_filters.length == 0 && ctxt.rev_file_filters.length == 0 {
            for target.input_files as f {
                ctxt.file_filters.push(lookup_input_file(ctxt, f))
            }
        }

        if target.references {
            for Object.values(target.references) as rfile {
                for rfile as r {
                    if String.is(r) {
                        ctxt.types.push(r)
                    } else {
                        ctxt.types.push(r.local)
                    }
                }
            }
        }

        if target.number_macros {
            for target.number_macros as m {
                ctxt.number_macros.push(m)
            }
        }

        if target.string_macros {
            for target.string_macros as m {
                ctxt.string_macros.push(m)
            }
        }

        get_cmacros(ctxt)
        get_cdecls(ctxt)
        gen_oxn_src(ctxt)
    } finally {
        if target.input_files.length != 1 {
            unlink(ctxt.input_file)
        }
    }
}

if ctxt.list_targets {
    //List the targets
    for Object.keys(ctxt.config.oxngen_targets) as name {
        stdout.puts("{name} ")
    }
} else {
    //Generate targets
    for Object.entries(ctxt.config.oxngen_targets) as [ctxt.target_name, ctxt.target] {
        gen_target(ctxt)
    }
}

return 0
