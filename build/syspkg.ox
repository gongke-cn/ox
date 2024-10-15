#!/usr/bin/ox

//Collect system packages.

ref "std/lang"
ref "std/io"
ref "std/path"
ref "std/path_conv"
ref "std/option"
ref "std/shell"
ref "std/dir"
ref "std/dir_r"
ref "std/temp_file"
ref "std/fs"
ref "std/copy"
ref "std/log"
ref "std/system"
ref "json"

log: Log("syspkg")

config = {
    outdir: "out"
    os: OX.os
}

if config.os == "windows" {
    config.rootdir = "C:/msys64/ucrt64"
} else {
    config.rootdir = "/usr"
}

options: Option([
    {
        short: "o"
        arg: Option.STRING
        help: "Set the output directory"
        on_option: func(opt, arg) {
            config.outdir = arg
        }
    }
    {
        short: "r"
        arg: Option.STRING
        help: "Set the system root directory"
        on_option: func(opt, arg) {
            config.rootdir = arg
        }
    }
    {
        long: "help"
        help: "Show this help message"
        on_option: func(opt, arg) {
            stdout.puts(''
Usage: syspkg [OPTION]...
Option:
{{options.help()}}
            '')
            config.show_help = true
        }
    }
])

if !options.parse(argv) {
    return 1
}

if config.show_help {
    return 0
}

//Load package list.
plist_path = "{config.outdir}/plist.ox"
plist = JSON.from_file(plist_path)

//Generate a system package.
gen_sys_pkg: func(pd) {
    p_dir = "{config.outdir}/pb/{pd.name}"
    mkdir_p(p_dir)

    build = {
        name: pd.name
        architecture: OX.target
        description: pd.desc
        version: pd.version
        system_package: true
        system_files: {}
    }

    if pd.deps {
        build.dependencies = pd.deps
    }

    if pd.url {
        build.homepage = pd.url
    }

    if pd.maintainer {
        build.maintainer = pd.maintainer
    }

    for pd.files as file {
        if pd.is_lib {
            src = file
        } else {
            st = Path(file, true)
            if st.format == Path.FMT_LNK {
                src = "?{file}"
            } else {
                src = file
            }
        }

        if file.slice(0, config.rootdir.length) == config.rootdir {
            dst = file.slice(config.rootdir.length + 1)
        } else {
            dst = file.slice(1)
        }

        build.system_files[dst] = src
    }

    build_path = "{p_dir}/build.ox"
    File.store_text(build_path, JSON.to_str(build, "  "))

    Shell.run("ox -r pb -C {p_dir} --json -p", Shell.OUTPUT|Shell.ERROR)

    p_file = "{pd.name}-{OX.target}-{pd.version}.oxp"

    copy("{p_dir}/{p_file}", "{config.outdir}/oxp/{p_file}")

    log.debug("generate \"{p_file}\"")
}

//Generate the system packages.
mkdir_p("{config.outdir}/oxp")
for Object.values(plist) as pd {
    pkg_path = "pkg/{pd.name}"
    if Path(pkg_path).exist {
        continue
    }

    log.debug("generate package \"{pd.name}\"")
    gen_sys_pkg(pd)
}