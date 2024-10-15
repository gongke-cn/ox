ref "std/io"
ref "std/lang"
ref "std/system"
ref "./config"
ref "./log"

//PM command.
public Command: class {
    enum {
        HELP
        SYNC
        REMOVE
        LIST
        LIST_SERVER
        LIST_FILES
        QUERY
        QUERY_SERVER
        DOWNLOAD
        INSTALL
        GEN_PL
        PREPARE
        REPAIR
        CLEAN
    }
}

//PM error.
public PmError: class Error {
    $init() {
        Error.$inf.$init.call(this, L"Package manager error")
    }
}

//Package manager.
public PackageManager: class {
    //Lookup the package by its name, and throw an error when cannot find it.
    lookup_throw(name) {
        pi = this.lookup(name)
        if !pi {
            error(L"cannot find package \"{name}\"")
            throw PmError()
        }

        return pi
    }
}

//Show package basic information.
public package_basic_info: func(pi, old_pi) {
    if old_pi {
        cr = version_compare(pi.version, old_pi.version)
        if cr == 0 {
            status = "({pi.version} installed)"
        } elif cr < 0 {
            status = "({pi.version} < {old_pi.version})"
        } else {
            status = "({pi.version} > {old_pi.version})"
        }
    } else {
        status = "({pi.version})"
    }

    desc = get_desc(pi)
    if (m = desc.match(/(.+)$/m)) {
        desc = m.groups[1]
    }

    stdout.puts("{pi.name!-20s} {status!-15s} {desc}\n")
}

//Checlk if the package compatible with the current platform.
public package_is_compatible: func(pi) {
    if pi.architecture == "all" {
        return true
    }

    if pi.architecture != config.target {
        return false
    }

    return true
}

//Compare 2 version strings.
public version_compare: func(s1, s2) {
    m = s1.match(/(.+)-(.+)/)
    if m {
        v1 = m.groups[1]
        b1 = Number(m.groups[2])
    } else {
        v1 = s1
        b1 = 0
    }

    m = s2.match(/(.+)-(.+)/)
    if m {
        v2 = m.groups[1]
        b2 = Number(m.groups[2])
    } else {
        v2 = s2
        b2 = 0
    }

    va1 = v1.split(".").map((Number($))).to_array()
    va2 = v2.split(".").map((Number($))).to_array()

    i = 0;
    while true {
        n1 = va1[i]
        n2 = va2[i]

        if n1 == null && n2 == null{
            return b1 - b2
        } elif n1 == null {
            return -1
        } elif n2 == null {
            return 1
        } elif n1 != n2 {
            return n1 - n2
        }

        i += 1
    }
}

//Get packages from their names.
public packages_from_names: func(pm, pns) {
    pkgs = []

    for pns as pn {
        pi = pm.lookup_throw(pn)
        pkgs.push(pi)
    }

    return pkgs
}

//Output error message.
public error: func(msg) {
    if stderr.is_tty {
        cb = "\x1b[31;1m"
        ce = "\x1b[0m"
    }
    stderr.puts(L"{cb}error{ce}: {msg}\n")
}

//Output warning message.
public warning: func(msg) {
    if stderr.is_tty {
        cb = "\x1b[35;1m"
        ce = "\x1b[0m"
    }
    stderr.puts(L"{cb}warning{ce}: {msg}\n")
}

//Convert the file path.
public file_path: func(inst, pi, file) {
    rf = file.replace(/%(\w+)%/, func(m) {
        case m.groups[1] {
        "bin" {
            s = "bin"
        }
        "lib" {
            s = config.libdir
        }
        "inc" {
            s = "include"
        }
        "pkg" {
            s = "share/ox/pkg/{pi.architecture}/{pi.name}"
        }
        "share" {
            s = "share/ox"
        }
        "locale" {
            s = "share/ox/pkg/{pi.architecture}/{pi.name}/locale"
        }
        "doc" {
            s = "share/ox/doc"
        }
        * {
            s = m.$to_str()
        }
        }

        return s
    })

    return "{inst}/{rf}"
}

//Get description
public get_desc: func(pi) {
    if pi.description instof String {
        desc = pi.description
    } else {
        lang = getenv("LANG")

        if lang {
            desc = pi.description[lang]

            if desc == null {
                pos = lang.lookup_char('.')
                if pos != -1 {
                    lang = lang.slice(0, pos)
                    desc = pi.description[lang]
                }
            }

            if desc == null {
                lang = lang.slice(0, 2)
                desc = pi.description[lang]
            }
        }

        if desc == null {
            #iter = Object.entries(pi.description)
            desc = iter.value[1]
        }
    }

    return desc
}
