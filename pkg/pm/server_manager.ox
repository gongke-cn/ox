ref "std/lang"
ref "std/uri"
ref "std/path"
ref "std/fs"
ref "std/copy"
ref "std/fs"
ref "std/io"
ref "oxp/package_list_schema"
ref "curl/curl"
ref "json/json"
ref "./log"
ref "./config"
ref "./tools"

curl_global_init(CURL_GLOBAL_DEFAULT)

//A package server.
Server: class {
    //Initialize a package server.
    $init(uri) {
        this.#uri = uri
    }

    //Download a file
    download(src, dst, total_size, clips) {
        downloaded = 0
        #dst_file = File(dst, "wb")

        if !clips {
            loop = 1
        } else {
            loop = clips
        }

        stdout.puts(L"downloading \"{src}\"\n")

        for i = 0; i < loop; i += 1 {
            if clips {
                src_uri = "{this.#uri}/{src}.{i}"
            } else {
                src_uri = "{this.#uri}/{src}"
            }

            curl = curl_easy_init()

            try {
                write_fn: func(buf, size, num) {
                    dst_file.write(buf, 0, num)
                    @downloaded += num
                    return num
                }.to_c(C.func_type(Size, UInt8.pointer, Size, Size))

                curl_easy_setopt(curl, CURLOPT_URL, src_uri)
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn)

                r = curl_easy_perform(curl)
                if r != CURLE_OK {
                    throw SystemError("download \"{src_uri}\" failed")
                }

                code = Int32()
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code)
                if code.$to_num() != 200 {
                    throw SystemError("download \"{src_uri}\" failed")
                }
            } catch e {
                dst_file.$close()
                unlink(dst)
                throw e
            } finally {
                curl_easy_cleanup(curl)
            }
        }
    }

    //Get the package list from the server.
    package_list {
        if !this.#package_list {
            mkdir_p("{config.install_dir}/share/ox/oxp")
            pl_fn = "{config.install_dir}/share/ox/oxp/{URI.encode(this.#uri, URI.ENC_COMP)}.ox"

            if !Path(pl_fn).exist || config.update_pl {
                this.download("package_list.ox", pl_fn)

                stdout.puts(L"sync packages list from \"{this.#uri}\"\n")
            }

            this.#package_list = JSON.from_file(pl_fn)

            package_list_schema.validate_throw(this.#package_list)
        }

        return this.#package_list
    }
}

//Local packages directory.
OxpDir: class {
    //Initialize the package directory.
    $init(dir) {
        this.#dir = dir
    }

    //Copy the package.
    download(src, dst, total_size, clips) {
        src_path = "{this.#dir}/{src}"

        if !clips {
            copy(src_path, dst)
        } else {
            #dst_file = File.open(dst, "wb")
            buf = UInt8(512*1024)

            for i = 0; i < clips; i += 1 {
                #src_file = File.open("{src}.{i}", "rb")

                while true {
                    n = src_file.read(buf)
                    dst_file.write(buf, 0, n)
                    if n == 0 {
                        break
                    }
                }
            }
        }
    }

    //Get the package list from the directory.
    package_list {
        if !this.#package_list {
            pl_fn = "{this.#dir}/package_list.ox"

            this.#package_list = JSON.from_file(pl_fn)

            package_list_schema.validate_throw(this.#package_list)
        }

        return this.#package_list
    }
}

//Package servers manager.
public ServerManager: class PackageManager {
    //Initialize the servers manager.
    $init() {
        this.#servers = []
        this.packages = Dict()

        for config.servers as uri {
            if uri ~ /^(.+):\/\// {
                srv = Server(uri)
            } else {
                srv = OxpDir(uri)
            }

            this.#servers.push(srv)
        }

        for this.#servers as srv {
            pl = srv.package_list

            for pl as pi {
                if !package_is_compatible(pi) {
                    continue
                }

                old = this.packages.get(pi.name)
                if old {
                    if version_compare(old.package.version, pi.version) >= 0 {
                        continue
                    }
                }

                this.packages.add(pi.name, {
                    package: pi
                    server: srv
                })
            }
        }
    }

    //List the packages.
    list(local_pm) {
        for this.packages.values() as ps {
            if local_pm {
                old = local_pm.lookup(ps.package.name)
            } else {
                old = null
            }

            package_basic_info(ps.package, old)
        }
    }

    //Lookup the package's information.
    lookup(name) {
        ps = this.packages.get(name)
        if !ps {
            return null
        }

        return ps.package
    }

    //Download the packages.
    download(name) {
        pi = this.lookup_throw(name)

        oxp_name = "{pi.name}-{pi.architecture}-{pi.version}.oxp"
        dst_path = "{config.install_dir}/share/ox/oxp/{oxp_name}"

        if !config.update {
            if Path(dst_path).exist {
                stdout.puts(L"package \"{name}\" is already downloaded\n")
                return
            }
        }

        ps = this.packages.get(name)

        mkdir_p("{config.install_dir}/share/ox/oxp")
        ps.server.download(oxp_name, dst_path, ps.package.size, pi.clips)
        stdout.puts(L"package \"{name}\" downloaded as \"{dst_path}\"\n")
    }

    //Sync packages from the server.
    sync(local_pm, packages, oxp_list = []) {
        install_dict = Dict()
        check_set = Set()

        add_sync: func(name, force, version) {
            need_install = false

            if check_set.has(name) {
                return
            }
            check_set.add(name)

            pi = this.lookup_throw(name)
            if version {
                if !config.force {
                    if version_compare(pi.version, version) < 0 {
                        error(L"minimum version required for \"{pi.name}\" is \"{version}\", but the actual version is \"{pi.verion}\"")
                        throw PmError()
                    }
                }
            }

            if !force {
                old = local_pm.lookup(name)
                if old {
                    cr = version_compare(pi.version, old.version)
                    if cr > 0 {
                        need_install = true
                    } else {
                        if force {
                            need_install = true
                        }
                    }
                } else {
                    need_install = true
                }

                if pi.system_package && !config.sys {
                    need_install = false
                }
            } else {
                need_install = true
            }

            if need_install {
                old = install_dict.get(pi.name)
                if !old {
                    install_dict.add(pi.name, pi)
                }
            }

            if !config.no_dep && pi.dependencies {
                for Object.entries(pi.dependencies) as [name, version] {
                    if packages.has(name) {
                        continue
                    }

                    dep_pi = this.lookup_throw(name)
                    if !dep_pi.system_package || config.sys {
                        add_sync(name, false, version)
                    }
                }
            }
        }

        //Check the version.
        for packages as pn {
            add_sync(pn, config.force)
        }

        if install_dict.length == 0 {
            stdout.puts(L"the packages are already the latest versions\n")
            return
        }

        stdout.puts(L"the following packages will be synced:\n")
        for install_dict.values() as pi {
            stdout.puts("  \"{pi.name}\" ({pi.version}) size: {pi.size}\n")
        }

        //Download the packages.
        for install_dict.entries() as [pn, pi] {
            this.download(pn)

            oxp_name = "{config.install_dir}/share/ox/oxp/{pi.name}-{pi.architecture}-{pi.version}.oxp"

            oxp_list.push(oxp_name)
        }

        //Install the packages.
        local_pm.install_packages(oxp_list, this)
    }
}
