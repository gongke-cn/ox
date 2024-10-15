ref "std/option"
ref "std/io"
ref "std/lang"
ref "std/path"
ref "std/dir_r"
ref "./log"
ref "./config"
ref "./server_list"
ref "./local_pm"
ref "./server_manager"
ref "./package_list"
ref "./tools"
ref "./delay_task"

/*?
 *? @package pm OX package manager.
 *? @exe OX package manager.
 */

//Show pm usage.
show_usage: func() {
    stdout.puts(L''
Usage: ox -r pm [OPTION]... [PACKAGE]...
OX package manager.
Option:
{{options.help()}}
    '')
}

//?
options: Option([
    {
        short: "d"
        arg: Option.STRING
        help: L"Add a package lookup directory"
        on_option: func(opt, arg) {
            config.pkg_dirs.push(arg)
        }
    }
    {
        long: "download"
        help: L"Only download software packages without installing them"
        on_option: func {
            config.cmd = Command.DOWNLOAD
        }
    }
    {
        long: "force"
        help: L"Force to install the packages even if the version is lower than the installed one"
        on_option: func {
            config.force = true
        }
    }
    {
        long: "gen-pl"
        help: L"Generate package list file"
        on_option: func {
            config.cmd = Command.GEN_PL
        }
    }
    {
        long: "split"
        arg: Option.NUMBER
        help: L"Split the oxp file the clips when generating package list. Set the clip size in bytes"
        on_option: func(opt, arg) {
            config.split = arg
        }
    }
    {
        shor: "i"
        long: "install"
        help: L"Install the package files. The PACKAGES are the package files, not the package names"
        on_option: func {
            config.cmd = Command.INSTALL
        }
    }
    {
        short: "I"
        arg: Option.STRING
        help: L"Set the installation directory"
        on_option: func(opt, arg) {
            config.install_dir = arg
        }
    }
    {
        short: "l"
        long: "list"
        help: L"List the packages installed"
        on_option: func {
            config.cmd = Command.LIST
        }
    }
    {
        long: "libdir"
        arg: Option.STRING
        help: L"Set the library directory"
        on_option: func(opt, arg) {
            config.libdir = arg
        }
    }
    {
        long: "list-files"
        help: L"List the files in the packages"
        on_option: func {
            config.cmd = Command.LIST_FILES
        }
    }
    {
        long: "list-server"
        help: L"List the packages on the server"
        on_option: func {
            config.cmd = Command.LIST_SERVER
        }
    }
    {
        long: "no-dep"
        help: L"Do not check dependencies"
        on_option: func {
            config.no_dep = true
        }
    }
    {
        short: "L"
        help: L"Update the local package list file"
        on_option: func {
            config.update_pl = true
        }
    }
    {
        long: "prep"
        help: L"Prepare packages for pb"
        on_option: func {
            config.cmd = Command.PREPARE
        }
    }
    {
        short: "q"
        long: "query"
        help: L"Query the package information"
        on_option: func {
            config.cmd = Command.QUERY
        }
    }
    {
        long: "query-server"
        help: L"Query the package information on the server"
        on_option: func {
            config.cmd = Command.QUERY_SERVER
        }
    }
    {
        short: "r"
        help: L"Remove the packages"
        on_option: func {
            config.cmd = Command.REMOVE
        }
    }
    {
        short: "s"
        help: L"Sync packages from the server"
        on_option: func {
            config.cmd = Command.SYNC
        }
    }
    {
        long: "repair"
        help: L"Repair the missing packages"
        on_option: func {
            config.cmd = Command.REPAIR
        }
    }
    {
        long: "clean"
        help: L"Cleaning the unused packages"
        on_option: func {
            config.cmd = Command.CLEAN
        }
    }
    {
        short: "S"
        arg: Option.STRING
        help: L"Add a package server"
        on_option: func(opt, arg) {
            config.servers.push(arg)
        }
    }
    {
        long: "sys"
        help: L"Synchronize system dependencies"
        on_option: func {
            config.sys = true
        }
    }
    {
        short: "t"
        arg: Option.STRING
        help: L"Set the target architecture name"
        on_option: func(opt, arg) {
            config.target = arg
        }
    }
    {
        short: "u"
        help: L"Update the local cached data"
        on_option: func {
            config.update = true
        }
    }
    {
        long: "help"
        help: L"Show this help message"
        on_option: func {
            show_usage()
            config.cmd = Command.HELP
        }
    }
])

if !options.parse(argv) {
    show_usage()
    return 1
}

for i = options.index; i < argv.length; i += 1 {
    config.packages.push(argv[i])
}

//Show help message
if config.cmd == Command.HELP {
    return 0
} elif !config.cmd {
    error(L"no command specified")
    return 1
}

//Get the default installation directory
if !config.install_dir {
    config.install_dir = OX.install_dir
}
log.debug("installation directory: {config.install_dir}")

//Load the servers.
if !config.servers.length {
    server_dir = "{config.install_dir}/share/ox/server"
    st = Path(server_dir)
    if st.exist && st.format == Path.FMT_DIR {
        #dir = DirR(server_dir, DirR.DIR_SKIP)
        for dir as filename {
            config.servers.push(...server_list_load(filename))
        }
    }
}
log.debug("servers: {config.servers.$iter().$to_str(", ")}")

if !config.target {
    //Get the architecture information
    config.target = OX.target
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

//Check the package list.
check_packages: func() {
    if config.packages.length == 0 {
        error(L"no package names specified")
        throw PmError()
    }
}

//Get the servers manager.
get_server_manager: func() {
    if !config.servers.length {
        error(L"no server specified")
        throw PmError()
    }

    return ServerManager()
}

//Get the local package manager.
get_local_pm: func() {
    if !config.install_dir {
        error(L"cannot get the OX installation directory")
        throw PmError()
    }

    pm = LocalPm()

    for config.pkg_dirs as dir {
        pm.add_package_dir(dir)
    }

    return pm
}

//Query the packages' information.
query_packages: func(pm) {
    for config.packages as pn {
        pi = pm.lookup_throw(pn)

        sb = String.Builder()

        if pi.system_package {
            name = "{pi.name} (system)"
        } else {
            name = pi.name
        }

        

        sb.append(''
name: {{name}}
architecture: {{pi.architecture}}
version: {{pi.version}}
description:
  {{get_desc(pi)}}

        '')

        if pi.dependencies {
            sb.append("dependencies:\n")

            for Object.entries(pi.dependencies) as [name, version] {
                sb.append("  {name} ({version})\n")
            }
        }

        if pi.executables {
            sb.append("executables:\n")

            for pi.executables as exe {
                sb.append("  {exe}\n")
            }
        }

        if pi.libraries {
            sb.append("libraries:\n")

            for pi.libraries as lib {
                sb.append("  {lib}\n")
            }
        }

        if pi.homepage {
            sb.append("homepage: {pi.homepage}\n")
        }

        if pi.maintainer {
            sb.append("maintainer: {pi.maintainer}\n")
        }

        stdout.puts(sb)
    }
}

//List the files in the packages.
list_files: func(pm) {
    for config.packages as pn {
        pi = pm.lookup_throw(pn)

        if pi.files {
            for pi.files as file {
                fn = file_path(config.install_dir, pi, file)
                stdout.puts("{fn}\n")
            }
        }
    }
}

//Run the command.
case config.cmd {
Command.PREPARE {
    check_packages()
    sm = get_server_manager()
    local_pm = get_local_pm()

    pkgs = []

    for config.packages as pn {
        pi = sm.lookup_throw(pn)
        if config.sys || !pi.system_package {
            pkgs.push(pn)
        }
    }

    sm.sync(local_pm, pkgs)
}
Command.REPAIR {
    sm = get_server_manager()
    local_pm = get_local_pm()

    local_pm.repair(sm)
}
Command.CLEAN {
    local_pm = get_local_pm()

    local_pm.clean()
}
Command.SYNC {
    sm = get_server_manager()
    local_pm = get_local_pm()

    //Sync all packages?
    if config.packages.length == 0 {
        for local_pm.packages.keys() as pn {
            if sm.lookup(pn) {
                config.packages.push(pn)
            }
        }
    }

    sm.sync(local_pm, config.packages)
}
Command.REMOVE {
    check_packages()
    local_pm = get_local_pm()
    local_pm.remove_packages(config.packages)
}
Command.LIST {
    local_pm = get_local_pm()
    local_pm.list()
}
Command.LIST_SERVER {
    sm = get_server_manager()
    local_pm = get_local_pm()
    sm.list(local_pm)
}
Command.LIST_FILES {
    local_pm = get_local_pm()
    list_files(local_pm)
}
Command.QUERY {
    check_packages()
    local_pm = get_local_pm()
    query_packages(local_pm)
}
Command.QUERY_SERVER {
    check_packages()
    sm = get_server_manager()
    query_packages(sm)
}
Command.DOWNLOAD {
    check_packages()
    sm = get_server_manager()

    for config.packages as pn {
        sm.download(pn)
    }
}
Command.INSTALL {
    check_packages()
    sm = get_server_manager()
    local_pm = get_local_pm()
    local_pm.install_packages(config.packages, sm)
}
Command.GEN_PL {
    check_packages()
    gen_package_list()
}
}

DelayTask.start()
