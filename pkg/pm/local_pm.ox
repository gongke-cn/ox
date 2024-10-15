ref "std/io"
ref "std/lang"
ref "std/dir"
ref "std/path"
ref "std/dir_r"
ref "std/lang"
ref "std/fs"
ref "std/shell"
ref "oxp/package_schema"
ref "oxp/oxp"
ref "json"
ref "./log"
ref "./config"
ref "./tools"
ref "./delay_task"

non_removable_pn_list = ["ox", "pm", "std"]

//Local package manager.
public LocalPm: class PackageManager {
    //Add a directory.
    #add_dir(pkg_dir, managed) {
        if !Path(pkg_dir).exist {
            return
        }

        #dir = Dir(pkg_dir)
        for dir as dent {
            if dent == "." || dent == ".." {
                continue
            }

            dn = "{pkg_dir}/{dent}"
            st = Path(dn)
            if st.exist && st.format == Path.FMT_DIR {
                pn = "{dn}/package.ox"

                st = Path(pn)
                if st.exist && st.format == Path.FMT_REG {
                    pi = JSON.from_file(pn)
                    package_schema.validate_throw(pi)

                    if !package_is_compatible(pi) {
                        continue
                    }

                    old = this.lookup(pi.name)
                    if old == null || version_compare(old.version, pi.version) < 0 {
                        this.packages.add(pi.name, pi)
                        pi.managed = managed
                    }

                    log.debug("find package \"{pi.name}\"")
                }
            }
        }
    }

    //Initialize the local package manager.
    $init() {
        this.#dir = config.install_dir
        this.#need_clear_dirs = false
        this.packages = Dict()

        this.#add_dir("{this.#dir}/share/ox/pkg/{config.target}", true)
        this.#add_dir("{this.#dir}/share/ox/pkg/all", true)
    }

    //Add a package lookup directory.
    add_package_dir(dir) {
        this.#add_dir(dir)
    }

    //List the packages.
    list() {
        pi_array = []

        for this.packages.values() as pi {
            pi_array.push(pi)
        }

        pi_array.sort(($0.name.compare($1.name)))

        for pi_array as pi {
            package_basic_info(pi)
        }
    }

    //Lookup a package's information.
    lookup(name) {
        return this.packages.get(name)
    }

    //Remove a package.
    remove(name, upgrade) {
        pi = this.lookup_throw(name)

        delay = DelayTask.need_delay(pi.name)

        if pi.pre_uninstall {
            if delay {
                DelayTask.pre_uninst(pi.pre_uninstall)
            } else {
                Shell.run(pi.pre_uninstall, Shell.OUTPUT|Shell.ERROR)
            }
        }

        for pi.files as file {
            path = file_path(this.#dir, pi, file)

            if Path(path).exist {
                if delay {
                    DelayTask.unlink(path)
                } else {
                    unlink(path)
                }

                log.debug("unlink \"{path}\"")
            }
        }

        dir = "{this.#dir}/share/ox/pkg/{pi.architecture}/{name}"

        if delay {
            DelayTask.rmdir(dir)
        } else {
            DirR.rmdir(dir)
            this.#need_clear_dirs = true
        }

        log.debug("rmdir \"{dir}\"")

        if pi.post_uninstall {
            if delay {
                DelayTask.post_uninst(pi.post_uninstall)
            } else {
                Shell.run(pi.post_uninstall, Shell.OUTPUT|Shell.ERROR)
            }
        }

        if upgrade {
            stdout.puts(L"remove package \"{name}\" ({pi.version})\n")
        } else {
            stdout.puts(L "package \"{name}\" removed\n")
        }
    }

    //Install a oxp file.
    install_oxp(oxp, upgrade) {
        pi = oxp.info
        new_dir = "{this.#dir}/share/ox/pkg/{pi.architecture}/{pi.name}"

        delay = DelayTask.need_delay(pi.name)

        if pi.pre_install {
            if delay {
                DelayTask.pre_inst(pi.pre_install)
            } else {
                Shell.run(pi.pre_install, Shell.OUTPUT|Shell.ERROR)
            }
        }

        if delay {
            delay_root = DelayTask.rootdir
            DelayTask.install(pi.name)
            oxp.unpack(delay_root, file_path)
        } else {
            oxp.unpack(this.#dir, file_path)
        }

        if pi.post_install {
            if delay {
                DelayTask.post_inst(pi.post_install)
            } else {
                Shell.run(pi.post_install, Shell.OUTPUT|Shell.ERROR)
            }
        }

        stdout.puts(L"package \"{pi.name}\" ({pi.version}) installed\n")
    }

    //Remove the packages.
    remove_packages(names) {
        p_dict = Dict()

        for names as name {
            if non_removable_pn_list.has(name) {
                error(L"package \"{name}\" cannot be removed")
                throw PmError()
            }

            pi = this.lookup(name)
            if pi {
                p_dict.add(name, {version: pi.version, dep_list: []})
            } else {
                warning(L"package \"{name}\" is not installed")
            }
        }

        for this.packages.values() as pi {
            if p_dict.get(pi.name) {
                continue
            }

            if pi.dependencies {
                for p_dict.entries() as [r_name, r_def] {
                    version = null

                    if pi.dependencies {
                        version = pi.dependencies[r_name]
                    }

                    if version /*&& version_compare(r_version, version) >= 0*/ {
                        r_def.dep_list.push(pi.name)
                        has_dep = true
                    }
                }
            }
        }

        if has_dep {
            if !config.no_dep {
                error(L"the following packages depend on the package to be removed")
            } else {
                warning(L"the following packages depend on the package to be removed")
            }

            for p_dict.entries() as [name, def] {
                if def.dep_list.length {
                    stderr.puts("  {name}: {def.dep_list.$iter().$to_str(", ")}\n")
                }
            }

            if !config.no_dep {
                throw PmError()
            }
        }

        for p_dict.keys() as name {
            this.remove(name)
        }

        this.clear_dirs()
    }

    //Install packages.
    install_packages(names, server_manager) {
        install_oxps = []
        oxp_dict = Dict()
        dep_dict = Dict()
        sys_dep_set = Set()

        //Load the oxp files.
        for names as name {
            oxp = OxpReader(name)
            oxp_dict.add(oxp.info.name, oxp)
        }

        //Check the version and dependencies.
        for oxp_dict.values() as oxp {
            pi = oxp.info
            if !package_is_compatible(pi) {
                error("\"{name}\" is not compatible with the current architecture")
                throw PmError()
            }

            need_install = false
            upgrade = false

            //Compare the version number.
            old = this.lookup(pi.name)
            if old {
                cr = version_compare(pi.version, old.version)
                if cr > 0 {
                    need_install = true
                } else {
                    if config.force {
                        need_install = true
                    }
                }

                if old.managed {
                    upgrade = true
                }
            } else {
                need_install = true
            }

            if need_install {
                install_oxps.push({
                    oxp
                    upgrade
                })

                //Add dependencies.
                add_deps: func(deps) {
                    if deps {
                        for Object.entries(deps) as [name, version] {
                            dep_oxp = oxp_dict.get(name)
                            if dep_oxp {
                                cr = version_compare(dep_oxp.info.version, version)
                                if cr >= 0 || config.force {
                                    continue
                                }
                            }

                            dep_pi = this.lookup(name)
                            if dep_pi {
                                cr = version_compare(dep_pi.version, version)
                                if cr >= 0 || config.force {
                                    continue
                                }
                            }

                            old_version = dep_dict.get(name)
                            if old_version == null || version_compare(version, old_version) > 0 {
                                srv_pi = server_manager.lookup_throw(name)

                                if config.sys || !srv_pi.system_package {
                                    dep_dict.add(name, version)
                                } else {
                                    sys_dep_set.add(name)
                                }
                            }
                        }
                    }
                }

                if !config.no_dep {
                    add_deps(oxp.info.dependencies)
                }
            } else {
                warning("package \"{pi.name}\" is already installed")
            }
        }

        //Sync dependencies.
        if !config.no_dep {
            if dep_dict.length {
                if server_manager {
                    packages = [...dep_dict.keys()]
                    oxp_list = [...oxp_dict.values().map(($.filename))]
                    server_manager.sync(this, packages, oxp_list)
                    return
                }

                error(L"the following dependent packages are not installed")
                for Object.entries(dep_dict) as [name, version] {
                    stderr.puts("  {name} ({version})\n")
                }
                throw PmError()
            }
        }

        //Files check. 
        rm_files = Set()
        exist_files = Dict()

        for install_oxps as rec {
            if rec.upgrade {
                for rec.oxp.info.files as file {
                    rm_files.add(file_path(this.#dir, rec.oxp.info, file))
                }
            }
        }

        for install_oxps as rec {
            check_file: func(file) {
                if rm_files.has(file) {
                    return
                }

                if Path(file).exist {
                    pkgs = exist_files.get(file)
                    if !pkgs {
                        pkgs = []
                        exist_files.add(file, pkgs)
                    }

                    pkgs.push(rec.oxp.info.name)
                }
            }

            for rec.oxp.info.files as file {
                check_file(file_path(this.#dir, rec.oxp.info, file))
            }
        }

        if exist_files.length {
            stderr.puts(L"The files in the following packages already exist\n")
            for exist_files.entries() as [file, pkgs] {
                stderr.puts("  \"{file}\": {pkgs}\n")
            }

            if !config.force {
                throw PmError()
            }

            stdout.puts(L"do you want to overwrite these files (y/n): ")
            stdout.flush()

            rep = stdin.gets()
            if !(rep ~ /\s*y/i) {
                return
            }
        }

        //Remove the old version packages.
        for install_oxps as rec {
            if rec.upgrade {
                this.remove(rec.oxp.info.name, true)
            }
        }

        //Install the new version packages.
        for install_oxps as rec {
            this.install_oxp(rec.oxp, rec.upgrade)
        }

        if sys_dep_set.length {
            stdout.puts(L"the installed packages depend on the following system packages:\n  ")
            for sys_dep_set as pn {
                stdout.puts("{pn} ")
            }
            stdout.puts("\n")
        }

        this.clear_dirs()
    }

    //Repair the missing package.
    repair(server_manager) {
        missing = {}

        for this.packages.values() as pi {
            if pi.dependencies {
                for Object.entries(pi.dependencies) as [pn, version] {
                    srv_pi = server_manager.lookup_throw(pn)
                    if !config.sys && srv_pi.system_package {
                        continue
                    }

                    local_pi = this.lookup(pn)
                    if !local_pi || version_compare(local_pi.version, version) < 0 {
                        old_version = missing[pn]

                        if old_version == null || version_compare(old_version, verion) < 0 {
                            missing[pn] = version
                        }
                    }
                }
            }
        }

        missing_pl = Object.keys(missing).to_array()

        if missing_pl.length {
            stdout.puts(L"the following packages are missing:\n")

            for Object.entries(missing) as [pn, version] {
                stdout.puts("  \"{pn}\" ({version})\n")
            }

            server_manager.sync(this, missing_pl)
        }

        this.clear_dirs()
    }

    //Cleaning the unused packages.
    clean() {
        needed_set = Set()
        unused_set = Set()

        mark_as_needed: func(pi) {
            if needed_set.has(pi) {
                return
            }

            unused_set.remove(pi)
            needed_set.add(pi)

            if pi.dependencies {
                for Object.keys(pi.dependencies) as dep_pn {
                    dep_pi = this.lookup(dep_pn)
                    if dep_pi {
                        mark_as_needed(dep_pi)
                    }
                }
            }
        }

        for this.packages.values() as pi {
            if pi.executables || non_removable_pn_list.has(pi.name) {
                mark_as_needed(pi)
            } elif !needed_set.has(pi) {
                unused_set.add(pi)
            }
        }

        rm_pn_list = []
        for unused_set as pi {
            rm_pn_list.push(pi.name)
        }

        if rm_pn_list.length > 0 {
            stdout.puts(L"the following packages can be removed:\n")

            for unused_set as pi {
                stdout.puts("  \"{pi.name}\" ({pi.version})\n")
            }

            stdout.puts(L"should these packages be removed (y/n): ")
            stdout.flush()

            rep = stdin.gets()
            if rep ~ /\s*y/i {
                this.remove_packages(rm_pn_list)
            }
        }

        this.clear_dirs()
    }

    //Clear empty directories.
    clear_dirs() {
        if !this.#need_clear_dirs {
            return
        }

        try_rmdir: func(path) {
            #dir = Dir(path)
            empty = true

            for dir as dent {
                if dent == "." || dent == ".." {
                    continue
                }

                cpath = "{path}/{dent}"
                if Path(cpath).format == Path.FMT_DIR {
                    if !try_rmdir(cpath) {
                        empty = false
                    }
                } else {
                    empty = false
                }
            }

            if empty {
                dir.$close()
                rmdir(path)
            }

            return empty
        }

        try_rmdir("{this.#dir}/share/ox")
    }
}
