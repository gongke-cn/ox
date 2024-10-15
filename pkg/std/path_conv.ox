/*?
 *? @lib Path format converter on Windows.
 */

ref "./log"
ref "./lang"
ref "./shell"

log: Log("path_conv")
log.level = Log.WARNING

//Mount table.
mount = null

//Get the mount table.
get_mount: func() {
    if mount {
        return
    }

    @mount = Dict()

    out = Shell.output("mount")
    for out.split("\n") as line {
        m = line.match(/(\S+)\s+on\s+(\S+)/)
        if m {
            dev = m.groups[1]
            dir = m.groups[2]

            if dir == "/" {
                root_dev = dev
            } else {
                mount.add(dir, dev)
            }
        }
    }

    if root_dev {
        mount.add("/", root_dev)
    }
}

/*?
 *? Convert the path to UNIX format path.
 *? On windows, "C:\dir" will be converted to "/c/dir".
 *? @param p {String} The input pathname.
 *? @return {String} The UNIX format pathname.
 */
public unix_path: func(p) {
    if OX.os == "windows" {
        if (m = p.match(/^\s*([a-z]):\s*$/i)) {
            return "/{m.groups[1].to_lower()}"
        } elif (m = p.match(/^\s*([a-z]):[\/\\]/i)) {
            return "/{m.groups[1].to_lower()}/{p.slice(m.end).replace("\\", "/")}"
        } else {
            return p.replace("\\", "/")
        }
    } else {
        return p
    }
}

/*?
 *? Convert the path to Windows format path.
 *? On windows, "/c/dir" will be converted to "C:\dir".
 *? @param p {String} The input pathname.
 *? @return {String} The Windows format pathname.
 */
public win_path: func(p) {
    if OX.os == "windows" {
        p = p.trim()

        if p[0] == "/" {
            get_mount()

            for mount.entries() as [unix, win] {
                if unix == "/" {
                    head = win.replace("/", "\\")

                    if p == "/" {
                        return head
                    }

                    return "{head}{p.replace("/", "\\")}"
                }

                if p.slice(0, unix.length) == unix {
                    nc = p[unix.length]

                    if (nc == "/") ||
                            (nc == "\\") ||
                            (nc == null) {
                        head = win.replace("/", "\\")

                        if p.length == unix.length {
                            return head
                        }

                        return "{head}{p.slice(unix.length).replace("/", "\\")}"
                    }
                }
            }
        }
        
        return p.replace("/", "\\")
    } else {
        return p
    }
}

/*?
 *? Convert the path to mixed format path.
 *? On windows, "/c/dir" will be converted to "C:/dir".
 *? @param p {String} The input pathname.
 *? @return {String} The mixed format pathname.
 */
public mixed_path: func(p) {
    if OX.os == "windows" {
        if p[0] == "/" {
            get_mount()

            for mount.entries() as [unix, win] {
                if unix == "/" {
                    if p == "/" {
                        return win
                    }

                    return "{win}{p.replace("\\", "/")}"
                }

                if p.slice(0, unix.length) == unix {
                    nc = p[unix.length]

                    if (nc == "/") ||
                            (nc == "\\") ||
                            (nc == null) {
                        head = win.replace("\\", "/")

                        if p.length == unix.length {
                            return head
                        }

                        return "{head}{p.slice(unix.length).replace("\\", "/")}"
                    }
                }
            }
        }
        
        return p.replace("\\", "/")
    } else {
        return p
    }
}
