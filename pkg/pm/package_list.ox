ref "std/path"
ref "std/io"
ref "std/copy"
ref "std/math"
ref "std/fs"
ref "oxp/oxp"
ref "json/json"
ref "./log"
ref "./config"
ref "./tools"

//Generate the package list.
public gen_package_list: func() {
    pl = []

    for config.packages as pn {
        #oxp = OxpReader(pn)
        pi = oxp.info

        if !package_is_compatible(pi) {
            continue
        }

        Object.del_prop(pi, "files")
        Object.del_prop(pi, "libraries")
        Object.del_prop(pi, "executables")
        Object.del_prop(pi, "internal_libraries")
        Object.del_prop(pi, "oxn_modules")
        Object.del_prop(pi, "oxngen_targets")
        Object.del_prop(pi, "data")

        st = Path(pn)

        pi.size = st.size

        if config.split {
            if pi.size > config.split {
                pi.clips = ((pi.size + config.split - 1) / config.split).floor()

                off = 0
                left = pi.size

                for i = 0; i < pi.clips; i += 1 {
                    size = min(left, config.split)

                    copy(pn, "{pn}.{i}", null, off, size)

                    off += size
                    left -= size
                }

                unlink(pn)
            }
        }

        pl.push(pi)
    }

    s = JSON.to_str(pl, "  ")
    stdout.puts(s)
}