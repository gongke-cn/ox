#!/usr/bin/ox

ref "std/lang"
ref "std/io"
ref "std/path"
ref "std/text_util"
ref "std/system"
ref "json"

target = getenv("TARGET")

//Load build description.
config = {
    cc: getenv("CC")
    pc: getenv("PC")
    target
    p: {}
}

v = getenv("HOMEPAGE")
if v {
    config.p.homepage = v
}

v = getenv("MAINTAINER")
if v {
    config.p.maintainer = v
}

m = target.match(/(.+)-(.+)-(.+)-(.+)/)
if !m {
    throw TypeError("illegal target")
}

config. {
    arch: m.groups[1]
    vendor: m.groups[2]
    os: m.groups[3]
    abi: m.groups[4] 
}

build = OX.file(argv[1])(config)

gen_oxp_name: func() {
    if build.architecture {
        arch = build.architecture
    } elif build.oxngen_targets ||
            build.oxn_modules ||
            build.system_files {
        arch = config.target
    } else {
        arch = "all"
    }

    stdout.puts("{build.name}-{arch}-{build.version}.oxp")
}

gen_oxngen_list: func() {
    if build.oxngen_targets {
        stdout.puts(Object.keys(build.oxngen_targets).$to_str(" "))
    }
}

gen_package_ox: func() {
    pkg = {
        name: build.name
        description: build.description
        version: "0"
        homepage: build.homepage
        maintainer: build.maintainer
    }

    if build.architecture {
        pkg.architecture = build.architecture
    } elif build.oxngen_targets || build.oxn_modules || build.system_files {
        pkg.architecture = "%ARCH%"
    } else {
        pkg.architecture = "all"
    }

    files = ["%pkg%/package.ox"]

    add_file: func(file) {
        if !files.has(file) {
            files.push(file)
        }
    }

    add_mod: func(mod) {
        if build.oxn_modules {
            if build.oxn_modules[mod] {
                add_file("%pkg%/{mod}.oxn")
                return
            }
        }

        if build.oxngen_targets {
            if build.oxngen_targets[mod] {
                add_file("%pkg%/{mod}.oxn")
                return
            }
        }

        add_file("%pkg%/{mod}.ox")
    }

    if build.libraries {
        pkg.libraries = build.libraries

        for build.libraries as lib {
            add_mod(lib)
        }
    }

    if build.executables {
        pkg.executables = build.executables

        for build.executables as exe {
            add_mod(exe)
        }
    }

    if build.internal_libraries {
        pkg.internal_libraries = build.internal_libraries

        for build.internal_libraries as lib {
            add_mod(lib)
        }
    }

    if build.oxn_modules {
        for Object.keys(build.oxn_modules) as oxn {
            add_mod(oxn)
        }
    }

    if build.oxngen_targets {
        for Object.keys(build.oxngen_targets) as oxn {
            add_mod(oxn)
        }
    }

    if build.dependencies {
        pkg.dependencies = build.dependencies
    }

    pkg.files = files

    stdout.puts(JSON.to_str(pkg, "  "))
}

case argv[2] {
"-n" {
    gen_oxp_name()
}
"-o" {
    gen_oxngen_list()
}
* {
    gen_package_ox()
}
}
