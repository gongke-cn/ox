ref "std/option"
ref "std/io"
ref "std/path"
ref "std/path_conv"
ref "std/fs"
ref "std/system"
ref "std/temp_file"
ref "std/shell"
ref "std/dir"
ref "std/copy"
ref "std/lang"
ref "json"
ref "./prepare"
ref "./loader"
ref "./config"
ref "./tools"
ref "./build_oxngen"
ref "./build_oxn"
ref "./build_text"
ref "./build_doc"
ref "./build_package"
ref "./build_oxp"
ref "./log"

/*?
 *? @package pb OX package builder.
 *? @exe OX package builder.
 */

//Show usage message.
show_usage: func() {
    stdout.puts(L''
Usage: ox -r pb [OPTION]...
Build the OX package.
Option:
{{options.help()}}
    '')
}

//?
options: Option([
    {
        short: "C"
        arg: Option.STRING
        help: L"Change the work directory before other operation"
        on_option: func(opt, arg) {
            config.cwd = arg
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

             config.p[pn] = pv
         }
    }
    {
        short: "i"
        arg: Option.STRING
        help: L"Use the file as build description file"
        on_option: func(opt, arg) {
            config.input = arg
        }
    }
    {
        short: "f"
        help: L"Force to rebuild the target files"
        on_option: func() {
            config.force = true
        }
    }
    {
        short: "g"
        help: L"Compile in debug mode"
        on_option: func() {
            config.debug = true
        }
    }
    {
        short: "o"
        arg: Option.STRING
        help: L"Set the output directory"
        on_option: func(opt, arg) {
            config.outdir = arg
        }
    }
    {
        long: "gen-po"
        arg: Option.STRING
        help: L"Generate \"po\" file"
        on_option: func(opt, arg) {
            config.cmd = Command.GEN_PO
            config.lang = arg
        }
    }
    {
        long: "update-text"
        help: L"Update \"pot\" and \"po\" files"
        on_option: func(opt, arg) {
            config.cmd = Command.UPDATE_TEXT
        }
    }
    {
        long: "clean"
        help: L"Clean the output files"
        on_option: func() {
            config.cmd = Command.CLEAN
        }
    }
    {
        short: "p"
        help: L"Create the OX package file"
        on_option: func() {
            config.cmd = Command.PACKAGE
        }
    }
    {
        short: "q"
        help: L"Quiet mode"
        on_option: func() {
            config.quiet = true
        }
    }
    {
        short: "t"
        arg: Option.STRING
        help: L"Set the target architecture"
        on_option: func(opt, arg) {
            config.target_arch = arg
        }
    }
    {
        long: "cc"
        arg: Option.STRING
        help: L"Set the compiler"
        on_option: func(opt, arg) {
            config.cc = arg
        }
    }
    {
        long: "pc"
        arg: Option.STRING
        help: L"Set the proram \"pkg-config\""
        on_option: func(opt, arg) {
            config.pc = arg
        }
    }
    {
        long: "ox"
        arg: Option.STRING
        help: L"Set the ox program"
        on_option: func(opt, arg) {
            config.ox = arg
        }
    }
    {
        long: "cflags"
        arg: Option.STRING
        help: L"Set compile flags"
        on_option: func(opt, arg) {
            config.cflags = arg
        }
    }
    {
        long: "libs"
        arg: Option.STRING
        help: L"Set linker flags"
        on_option: func(opt, arg) {
            config.libs = arg
        }
    }
    {
        long: "json"
        help: L"Treat the \"build.ox\" as static JSON"
        on_option: func {
            config.json = true
        }
    }
    {
        long: "prep"
        help: L"Prepare the building environment"
        on_option: func {
            config.cmd = Command.PREPARE
        }
    }
    {
        long: "sys"
        help: L"Install system packages when prepare building environment"
        on_option: func {
            config.sys = true
        }
    }
    {
        long: "help"
        help: L"Show this help message"
        on_option: func() {
            config.cmd = Command.HELP
        }
    }
])

//Parse options.
if !options.parse(argv) {
    show_usage()
    return 1
}

//Show help.
if config.cmd == Command.HELP {
    show_usage()
    return 0
}

if config.outdir {
    mkdir_p(config.outdir)
    config.outdir = fullpath(config.outdir)
}

if config.cwd {
    chdir(config.cwd)
}

if !Path(config.input).exist {
    stderr.puts(L"cannot find input file \"{config.input}\"\n")
    return 1
}

config.input = fullpath(config.input)

if !config.outdir {
    config.outdir = fullpath(".")
}

//Get the build target.
config.target = Shell.output("{config.cc} -dumpmachine").trim()

case config.target {
($ ~ /x86_64-.*linux/) {
    if config.cflags && config.cflags ~ /-m32\b/ {
        config.target = "i686-pc-linux-gnu"
    } else {
        config.target = "x86_64-pc-linux-gnu"
    }
}
($ ~ /i686-.*linux/) {
    if config.cflags && config.cflags ~ /-m64\b/ {
        config.target = "x86_64-pc-linux-gnu"
    } else {
        config.target = "i686-pc-linux-gnu"
    }
}
($ ~ /x86_64-w64.*/) {
    if config.cflags && config.cflags ~ /-m32\b/ {
        config.target = "i686-w64-windows-gnu"
    } else {
        config.target = "x86_64-w64-windows-gnu"
    }
}
}

m = config.target.match(/(.+)-(.+)-(.+)-(.+)/)
if !m {
    throw TypeError("illegal target")
}

config.{
    arch: m.groups[1]
    vendor: m.groups[2]
    os: m.groups[3]
    abi: m.groups[4]
}

if config.os == "windows" {
    config.exe = ".exe"
} else {
    config.exe = ""
}

//Get OX program.
if !config.ox {
    config.ox = "ox"
}

if config.debug {
    config.cflags += " -g"
}

//Build the package files.
build: func() {
    //Run pre-build command.
    if desc.pre_build {
        Shell.run(desc.pre_build, Shell.OUTPUT|Shell.ERROR)
    }

    //Build oxngen OXN modules.
    if desc.oxngen_targets {
        build_oxngen(desc.oxngen_targets)
    }

    //Build OXN modules.
    if desc.oxn_modules {
        build_oxn(desc.oxn_modules)
    }

    //Build text message.
    build_text(desc)

    //Build document.
    build_doc(desc)

    //Build package.
    build_package(desc)

    //Run post-build command.
    if desc.post_build {
        Shell.run(desc.post_build, Shell.OUTPUT|Shell.ERROR)
    }
}

//Load the "build.ox".
desc = build_load(config.input)

case config.cmd {
Command.PREPARE {
    prepare(desc)
}
Command.GEN_PO {
    gen_po(desc)
}
Command.UPDATE_TEXT {
    update_text(desc)
}
Command.CLEAN {
    files = []

    remove: func(file) {
        st = Path(file)
        if st.exist {
            if st.format == Path.FMT_DIR {
                #dir = TempDir(file)
            } else {
                unlink(file)
                files.push(file)
            }
        }
    }

    if desc.oxn_modules {
        for Object.entries(desc.oxn_modules) as [mod, def] {
            for def.sources as src {
                o_name = object_name(mod, src)
                remove(o_name)

                d_name = o_name.replace(/\.o$/, ".d")
                remove(d_name)
            }
            remove("{config.outdir}/{mod}.oxn")
        }
    }

    if desc.oxngen_targets {
        for Object.entries(desc.oxngen_targets) as [mod, target] {
            remove("{config.outdir}/{mod}.oxn.c")

            o_name = "{config.outdir}/{mod}.o"
            remove(o_name)

            d_name = o_name.replace(/\.o$/, ".d")
            remove(d_name)

            remove("{config.outdir}/{mod}.oxn")
        }
    }

    if Path("locale").exist {
        for Dir("locale") as dent {
            if (m = dent.match(/(.+)\.po/ip)) {
                l = m.groups[1]
                file = "{config.outdir}/locale/{l}"
                remove(file)
            }
        }
    }

    remove("{config.outdir}/doc")

    oxp_name = "{config.outdir}/{desc.name}-{get_arch(desc)}-{desc.version}.oxp"
    if Path(oxp_name).exist {
        remove(oxp_name)
    }

    if Path("package.ox").exist {
        remove("package.ox")
    }

    if files.length {
        info(L"delete {files.$iter().$to_str(", ")}")
    }
}
Command.BUILD {
    //Build oxngen targets.
    build()
}
Command.PACKAGE {
    //Create the oxp file.
    build()

    build_oxp(desc)
}
}
