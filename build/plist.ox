#!/usr/bin/ox

ref "std/option"
ref "std/log"
ref "std/lang"
ref "std/shell"
ref "std/io"
ref "std/path"
ref "std/dir"
ref "std/system"
ref "std/path_conv"
ref "json"
ref "xml"

log: Log("plist")

config: {
    version: "0.0.1"
    os: OX.os
    libdir: OX.lib_dir
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

if config.os == "windows" {
    config.rootdir = "/ucrt64"
    config.pn_prefix = "mingw-w64-ucrt-x86_64-"
} else {
    config.rootdir = "/usr"
    config.pn_prefix = ""
}

usage: func() {
    stdout.puts(''
Usage: plist [OPTION]...
Option:
{{options.help()}}
    '')
}

options: Option([
    {
        short: "l"
        arg: Option.STRING
        help: "Set the library directory"
        on_option: func(opt, arg) {
            config.libdir = arg
        }
    }
    {
        short: "o"
        arg: Option.STRING
        help: "Set the output filename"
        on_option: func(opt, arg) {
            config.out = arg
        }
    }
    {
        short: "r"
        arg: Option.STRING
        help: "Set the root directory"
        on_option: func(opt, arg) {
            config.rootdir = arg
        }
    }
    {
        short: "i"
        help: "Generate internal packages only"
        on_option: func {
            config.internal = true
        }
    }
    {
        long: "help"
        help: "Show this help message"
        on_option: func {
            usage()
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

//Packages' description
p_descs: {
    "libffi_devel": {
        bid: 0
        libs: [
            "libffi"
        ]
        srcs: [
            if config.os == "windows" {
                "libffi!\.(dll|exe)$"
            } else {
                "libffi-dev"
            }
        ]
    }
    "libarchive_devel": {
        bid: 0
        libs: [
            "libarchive"
        ]
        srcs: [
            if config.os == "windows" {
                "libarchive!\.(dll|exe)$"
            } else {
                "libarchive-dev"
            }
        ]
    }
    "libcurl_devel": {
        bid: 0
        libs: [
            "libcurl"
        ]
        srcs: [
            if config.os == "windows" {
                "curl!\\.(dll|exe)$"
            } else {
                "libcurl4-openssl-dev"
            }
        ]
    }
    "libncurses_devel": {
        bid: 0
        libs: [
            "libncursesw"
            "libformw"
            "libmenuw"
            "libpanelw"
            if config.os == "linux" {
                "libtinfo"
            }
        ]
        srcs: [
            if config.os == "windows" {
                "ncurses!\\.(dll|exe)$"
            } else {
                "libncurses-dev"
                "ncurses-base"
            }
        ]
    }
    "openssl_devel": {
        bid: 0
        libs: [
            "libssl"
            "libcrypto"
        ]
        srcs: [
            if config.os == "windows" {
                "openssl!\\.(dll|exe)$"
            } else {
                "libssl-dev"
            }
        ]
    }
    "libsdl_devel": {
        bid: 0
        libs: [
            if config.os == "windows" {
                "SDL"
            } else {
                "libSDL"
            }
        ]
        srcs: [
            if config.os == "windows" {
                "SDL!\\.(dll|exe)$"
            } else {
                "libsdl1.2-dev"
            }
        ]
    }
    "libglib2.0_devel": {
        bid: 0
        libs: [
            "libglib-2.0"
            "libgio-2.0"
            "libgmodule-2.0"
            "libgobject-2.0"
            "libgthread-2.0"
            "libgirepository-2.0"
        ]
        srcs: [
            if config.os == "windows" {
                "glib2!\\.(dll|exe|typelib|hook)$"
            } else {
                "libglib2.0-dev"
                "libglib2.0-dev-bin"
                "libgirepository-2.0-dev"
            }
        ]
    }
    "libgtk-4": {
        bid: 0
        libs: [
            "libgtk-4"
        ]
    }
    "gcc": {
        bid: 0
        deps: [
            "binutils"
            "libc_devel"
        ]
        srcs: [
            if config.os == "windows" {
                "gcc"
            } else {
                "gcc"
                "gcc-13"
                "gcc-13-base"
                "gcc-x86-64-linux-gnu"
                "g++"
                "g++-13"
                "g++-x86-64-linux-gnu"
                "cpp"
                "cpp-13"
                "cpp-x86-64-linux-gnu"
                "libgcc-13-dev"
                "libstdc++-13-dev"
            }
        ]
    }
    "clang": {
        bid: 0
        deps: [
            "binutils"
            "libc_devel"
        ]
        srcs: [
            if config.os == "windows" {
                "clang"
            } else {
                "clang"
                "clang-18"
                "libclang-dev"
                "libclang-18-dev"
                "libclang1-18"
                "libclang-common-18-dev"
                "libclang-cpp18"
            }
        ]
    }
    "binutils": {
        bid: 0
        srcs: [
            "binutils"
        ]
    }
    "gnu-gettext": {
        bid: 0
        srcs: [
            if config.os == "windows" {
                "gettext-runtime!\.dll$"
                "gettext-tools!\.dll$"
            } else {
                "gettext"
                "gettext-base"
            }
        ]
    }
    "libc_devel": {
        bid: 0
        srcs: [
            if config.os == "windows" {
                "crt-git"
                "headers-git"
            } else {
                "libc6-dev"
            }
        ]
    }
    "ca-certificates": {
        bid: 0
        srcs: [
            "ca-certificates"
        ]
    }
    "pkgconf": {
        bid: 0
        srcs: [
            if config.os == "windows" {
                "pkg-config"
            } else {
                "pkgconf"
                "pkgconf-bin"
            }
        ]
    }
}

lib_pn_dict = Dict()
packages = {}

//Find dll file.
get_dll: func(name) {
    #dir = Dir(mixed_path("{config.rootdir}/bin"))
    for dir as dent {
        if !(dent ~ /\.dll/i) {
            continue
        }

        if dent.slice(0, name.length) == name {
            return dent
        }
    }

    return null
}

//Get the soname of the library.
get_so_name: func(name) {
    if config.os == "windows" {
        if name ~ /\.dll$/i {
            path = mixed_path("{config.rootdir}/bin/{name}")
            if Path(path).exist {
                return name
            }
        } else {
            if (file = get_dll(name)) {
                return file
            }

            m = name.match(/lib(.+)/)
            if m {
                name = m.groups[1]
                if (file = get_dll(name)) {
                    return file
                }
            }
        }
    } else {
        if name ~ /\.so\b/ {
            path = "{config.rootdir}/{config.libdir}/{name}"
        } else {
            path = "{config.rootdir}/{config.libdir}/{name}.so"
        }

        out = Shell.output("objdump -x {path}")
        for out.split("/n") as line {
            m = line.match(/SONAME\s+(\S+)/)
            if m {
                return m.groups[1]
            }
        }

        m = File.load_text(path).match(/lib\S+\.so(\.\S+)?/)
        if m {
            return basename(m.$to_str())
        }
    }

    throw NullError("cannot get so name of \"{name}\"")
}

//Create the package basic information from system package manager.
get_info: func(pn, prefix) {
    pi = {}

    if config.os == "windows" {
        m = pn.match(/(.+)!(.+)/)
        if m {
            pn = m.groups[1]
        }

        out = Shell.output("pacman -Qi {config.pn_prefix}{pn}")
    } else {
        out = Shell.output("dpkg-query --status {pn}")
    }

    for out.split("\n") as line {
        if config.os == "windows" {
            m = line.match(/Version\s*:\s*(\d+~)?(\d+(\.\d+)*(-\d+)?)/)
            if m {
                pi.version = m.groups[2]
            } elif (m = line.match(/Description\s*:\s*(.+)/)) {
                if prefix {
                    pi.desc = "{prefix} - {m.groups[1]}"
                } else {
                    pi.desc = m.groups[1]
                }
            } elif (m = line.match(/URL\s*:\s*(.+)/)) {
                pi.url = m.groups[1]
            } elif (m = line.match(/Packager\s*:\s*(.+)/)) {
                pi.maintainer = m.groups[1]
            }
        } else {
            m = line.match(/Version: (\d+:)?(\d+(\.(\d+))*(-\d+)?)/)
            if m {
                pi.version = m.groups[2]
            } elif (m = line.match(/Description: (.+)/)) {
                if prefix {
                    pi.desc = "{prefix} - {m.groups[1]}"
                } else {
                    pi.desc = m.groups[1]
                }
            } elif (m = line.match(/Homepage: (.+)/)) {
                pi.url = m.groups[1]
            } elif (m = line.match(/Maintainer: (.+)/)) {
                pi.maintainer = m.groups[1]
            }
        }

        if pi.version && pi.desc && pi.url && pi.maintainer {
            break
        }
    }

    if pi.version && pi.desc {
        return pi
    }

    throw NullError("cannot get information of \"{pn}\"")
}

//Get the package information of a file.
get_info_from_file: func(path) {
    if config.os == "windows" {
        out = Shell.output("pacman -Qo {path}")
        if out == "" {
            throw NullError("cannot find package contains \"{path}\"")
        }

        m = out.match(/is owned by (\S+)/)
        pn = m.groups[1].replace(config.pn_prefix, "")
    } else {
        out = Shell.output("dpkg-query --search {path}")

        if out == "" {
            path = path.replace(/^\/usr/, "")
            out = Shell.output("dpkg-query --search {path}")
            if out == "" {
                throw NullError("cannot find package contains \"{path}\"")
            }
        }
        m = out.match(/([^:]+):/)
        pn = m.groups[1]
    }

    pi = get_info(pn, basename(path))
    pi.{
        files: [
            mixed_path(path)
        ]
    }

    return pi
}

//Get the package information of a dynamic library.
get_info_from_dlib: func(file) {
    if config.os == "windows" {
        path = "{config.rootdir}/bin/{file}"
    } else {
        path = "{config.rootdir}/{config.libdir}/{file}"
    }

    return get_info_from_file(path)
}

//Add dependencies libraries.
add_dep_libs: func(pi, path, files) {
    out = Shell.output("ldd {path}")
    for out.split("\n") as line {
        m = line.match(/=>\s*(\S+)/)
        if m {
            lib_path = m.groups[1]
            if config.os == "windows" {
                if lib_path ~ /^\/c\/Windows\// {
                    continue
                }
            }

            dep_lib = basename(lib_path)

            //If the file is in the package?
            if files {
                in_pkg = false
                for files as pfile {
                    if basename(pfile) == dep_lib {
                        in_pkg = true
                        break
                    }
                }

                if in_pkg {
                    continue
                }
            }

            //Add a new library package.
            dep_pi = add_lib(dep_lib, true)

            //Add dependencies.
            if !pi.deps {
                pi.deps = {}
            }

            pi.deps[dep_pi.name] = dep_pi.version
        }
    }
}

//Add a library package.
add_lib: func(lib, from_ldd) {
    so_name = get_so_name(lib)

    if config.os == "windows" {
        m = so_name.match(/(.+)-(\d+)\.dll$/i)
        if m {
            lib_name = "{m.groups[1]}_{m.groups[2]}"
        } else {
            m = so_name.match(/(.+)\.dll$/i)
            lib_name = m.groups[1]
        }

        lib_path = "{config.rootdir}/bin/{so_name}"
    } else {
        m = so_name.match(/(\S+)\.so\.(\S+)/)
        if m {
            lib_name = "{m.groups[1]}_{m.groups[2]}"
        } else {
            m = so_name.match(/(\S+)\.so/)
            lib_name = m.groups[1]
        }

        lib_path = "{config.rootdir}/{config.libdir}/{so_name}"
    }

    if lib_name.slice(0, 3) != "lib" {
        lib_name = "lib{lib_name}"
    }

    lib_pi = packages[lib_name]
    if lib_pi {
        if !from_ldd {
            lib_pi.ref_name = lib
        }
        return lib_pi
    }

    lib_pi = get_info_from_dlib(so_name)
    lib_pi.name = lib_name
    lib_pi.is_lib = true

    if !from_ldd {
        lib_pi.ref_name = lib
    }

    packages[lib_pi.name] = lib_pi
    lib_pn_dict.add(so_name, lib_pi.name)
    log.debug("add package \"{lib_pi.name}\"")

    add_dep_libs(lib_pi, lib_path)

    return lib_pi
}

//Add files to the packages.
add_files: func(pi, src) {
    if config.os == "windows" {
        m = src.match(/(.+)!(.+)/)
        if m {
            src = m.groups[1]
            filter = Re(m.groups[2])
        }
        out = Shell.output("pacman -Ql {config.pn_prefix}{src}")
    } else {
        out = Shell.output("dpkg-query --listfiles {src}")
    }

    //Add files.
    for out.split("\n") as line {
        if line == "" {
            continue
        }

        if config.os == "windows" {
            m = line.match(/(\S+)\s+(\S+)/)
            path = m.groups[2]

            if filter {
                if path.match(filter) {
                    continue
                }
            }

            file = mixed_path(path)
        } else {
            file = line
        }

        st = Path(file, true)
        if st.format != Path.FMT_DIR {
            if pi.files == null {
                pi.files = []
            }

            pi.files.push(file)
        }
    }
}

target = getenv("TARGET")

//Load build description.
bcfg = {
    cc: getenv("CC")
    pc: getenv("PC")
    target
    p: {}
}

m = target.match(/(.+)-(.+)-(.+)-(.+)/)
if !m {
    throw TypeError("illegal target")
}

bcfg.{
    arch: m.groups[1]
    vendor: m.groups[2]
    os: m.groups[3]
    abi: m.groups[4] 
}

//Scan the packages directory.
#dir = Dir("pkg")
for dir as dent {
    if dent == "." || dent == ".." {
        continue
    }

    path = "pkg/{dent}/build.ox"
    if !Path(path).exist {
        continue
    }

    build = OX.file(path)(bcfg)
    desc = {
        version: build.version
        desc: build.description
        internal: true
    }

    if build.dependencies {
        libs = []
        for Object.keys(build.dependencies) as dep {
            m = dep.match(/lib.+/ip)
            if m {
                if !(dep ~ /\_devel$/) {
                    libs.push(dep)
                }
            }
        }

        if libs.length {
            desc.libs = libs
        }
    }

    p_descs[build.name] = desc
}

//Generate the packages' information.
for Object.entries(p_descs) as [pn, pd] {
    if !pd.internal && config.internal {
        continue
    }

    if pd.srcs {
        //Build from system packages.
        pi = null

        //Add files from source packages.
        for pd.srcs as src {
            if pi == null {
                pi = get_info(src)
                pi.name = pn

                packages[pi.name] = pi
                log.debug("add package \"{pi.name}\"")
            }

            add_files(pi, src)
        }

        //Add dependencies libraries.
        if !config.internal {
            for pi.files as file {
                is_dobj = false
                if config.os == "windows" {
                    if file ~ /\.(dll|exe)$/ {
                        is_dobj = true
                    }
                } else {
                    if file ~ /\/bin\// || file ~ /lib.+\.so/{
                        is_dobj = true
                    }
                }

                if is_dobj {
                    add_dep_libs(pi, file, pi.files)
                }
            }
        }

        if pd.libs && !config.internal {
            for pd.libs as lib {
                lib_pi = add_lib(lib)

                if !pi.deps {
                    pi.deps = {}
                }

                pi.deps[lib_pi.name] = lib_pi.version
            }
        }
    } elif pd?.libs[0] == pn {
        pi = add_lib(pd.libs[0])
    } else {
        //Build the package.
        pi = {
            name: pn
            desc: pd.desc
            version: pd.version
        }

        packages[pi.name] = pi
        log.debug("add package \"{pi.name}\"")

        if pd.libs && !config.internal {
            for pd.libs as lib {
                lib_pi = add_lib(lib)

                if !pi.deps {
                    pi.deps = {}
                }

                pi.deps[lib_pi.name] = lib_pi.version
            }
        }
    }

    if pd.bid != null {
        version = pi.version
        if (m = version.match(/(.+)-\d+/)) {
            version = m.groups[1]
        }

        pi.version = "{version}-{pd.bid}"
    }
}

if !config.internal {
    //Add dependencies.
    for Object.entries(p_descs) as [pn, pd] {
        if pd.deps {
            for pd.deps as dep {
                pi = packages[pn]
                if !pi {
                    throw NullError("cannot get package \"{pn}\"")
                }

                dep_pi = packages[dep]
                if !dep_pi {
                    throw NullError("cannot get package \"{dep}\"")
                }

                if !pi.deps {
                    pi.deps = {}
                }

                pi.deps[dep] = dep_pi.version
            }
        }
    }

    /*Add typelib files.*/
    gir_dir = mixed_path("{config.rootdir}/share/gir-1.0")
    gir_dir2 = mixed_path("{config.rootdir}/{config.libdir}/gir-1.0")

    add_typelib: func(name) {
        pn = "{name}-typelib"

        if (dep_pi = packages[pn]) {
            return dep_pi
        }

        pi = get_info_from_file("{config.rootdir}/{config.libdir}/girepository-1.0/{name}.typelib")

        pi.name = pn

        packages[pn] = pi
        log.debug("add package \"{pi.name}\"")

        gir = "{gir_dir}/{name}.gir"
        if !Path(gir).exist {
            gir = "{gir_dir2}/{name}.gir"
        }

        xml = XML.from_file(gir)
        for xml.root.content as e {
            if e.tag == "namespace" {
                libs = e.attrs["shared-library"]
                if libs {
                    for libs.split(",") as lib {
                        dep_pn = lib_pn_dict.get(lib)
                        if dep_pn {
                            if !pi.deps {
                                pi.deps = {}
                            }
                            pi.deps[dep_pn] = packages[dep_pn].version
                        }
                    }
                }
            } elif e.tag == "include" {
                name = e.attrs["name"]
                version = e.attrs["version"]

                dep_pi = add_typelib("{name}-{version}")

                if !pi.deps {
                    pi.deps = {}
                }
                pi.deps[dep_pi.name] = dep_pi.version
            }
        }

        return pi
    }

    add_typelib("Gtk-4.0")
}

json = JSON.to_str(packages, "  ")

if config.out {
    File.store_text(config.out, json)
} else {
    stdout.puts(json)
}