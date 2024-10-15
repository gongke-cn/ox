ref "std/system"
ref "std/path"
ref "json"
ref "std/io"

plist_path = getenv("PLIST")
if plist_path && plist_path != "" {
    if Path(plist_path).exist {
        plist = JSON.from_file(plist_path)
    }
}

//Get the package information.
get_pi: func(n) {
    pi = plist[n]
    if pi {
        return pi
    }

    for Object.entries(plist) as [pn, pi] {
        if pi.ref_name == n {
            return pi
        }
    }

    //throw NullError("cannot find package \"{n}\"")
    return null
}

//Get dependencies.
public get_deps: func() {
    deps = {}

    for argv as p {
        if plist {
            pi = get_pi(p)
            if pi {
                deps[pi.name] = pi.version
            } else {
                deps[p] = "0"
            }
        } else {
            deps[p] = "0"
        }
    }

    return deps
}

//Get the package's version.
public get_version: func(p) {
    if plist {
        pi = get_pi(p)

        if pi {
            return pi.version
        } else {
            return "0"
        }
    } else {
        return "0"
    }
}