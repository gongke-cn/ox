ref "std/system"
ref "std/shell"
ref "std/io"
ref "std/fs"
ref "std/path"
ref "std/dir"
ref "./config"
ref "./log"

//Run shell command.
public shell: func(cmd) {
    info(cmd)

    if Shell.run(cmd, Shell.ERROR) != 0 {
        throw SystemError(L"run \"{cmd}\" failed")
    }
}

//Show information.
public info: func(msg) {
    if !config.quiet {
        stdout.puts("{msg}\n")
    }
}

//Check if the object need to be updated.
public need_update: func(dst, srcs) {
    if config.force {
        return true
    }

    dst_st = Path(dst)

    if dst_st.exist {
        update = false

        for srcs as src {
            src_st = Path(src)
            
            if !src_st.exist {
                update = true
                break
            }

            if src_st.mtime > dst_st.mtime {
                update = true
                break
            }
        }
    } else {
        update = true
    }

    return update
}

//Lookup all the source files.
public lookup_sources: func(desc, internal) {
    set = Set()

    //Check if the module is an OXN.
    is_oxn: func(mod) {
        if desc.oxn_modules?[mod] {
            return true
        }

        if desc.oxngen_targets?[mod] {
            return true
        }

        return false
    }

    //Add a module to the set.
    add_mod: func(mod) {
        if !is_oxn(mod) {
            set.add("{mod}.ox")
        }
    }

    if desc.libraries {
        for desc.libraries as lib {
            add_mod(lib)
        }
    }

    if internal && desc.internal_libraries {
        for desc.internal_libraries as lib {
            add_mod(lib)
        }
    }

    if desc.executables {
        for desc.executables as exe {
            add_mod(exe)
        }
    }

    if desc.oxn_modules {
        for Object.values(desc.oxn_modules) as def {
            for def.sources as src {
                set.add(src)
            }
        }
    }

    return set.$iter().to_array()
}

//Get the architecture of the package.
public get_arch: func(desc) {
    if desc.architecture {
        arch = desc.architecture
    } elif desc.oxn_modules ||
            desc.oxngen_targets ||
            desc.system_files {
        arch = config.target
    } else {
        arch = "all"
    }

    return arch
}

//Get dependence files.
public get_dep_files: func(dep_file) {
    deps = [dep_file]

    if !Path(dep_file).exist {
        return deps
    }

    #file = File(dep_file, "rb")
    first = true

    while true {
        line = file.gets()
        if line == null {
            break
        }

        if line[line.length - 1] == "\n" {
            line = line.slice(0, line.length - 1)
        }

        if line[line.length - 1] == "\\" {
            line = line.slice(0, line.length - 1)
        }

        if first {
            m = line.match(/.+:(.*)/)
            if m {
                first = false
                line = m.groups[1]
            } else {
                continue
            }
        }

        for line.split(/\s+/) as item {
            if item != "" {
                deps.push(item)
            }
        }
    }

    return deps
}
