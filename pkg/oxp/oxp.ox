ref "std/path"
ref "std/io"
ref "std/dir"
ref "std/fs"
ref "json"
ref "archive"
ref "./log"
ref "./package_schema"

/*?
 *? @package oxp OX package operation library.
 *? @lib OX package operation library.
 */

/*?
 *? @callback FilenameCallback Filename convertor callback.
 *? @param root {String} The root directory.
 *? @param pi {Object} The package information.
 *? @param file {String} The input filename.
 *? @return {String} The result filename.
 */

//Archive error.
ArchiveError: class Error {
    //Initialize an archive error.
    $init(archive, msg) {
        Error.$inf.$init.call(this, "{msg}: {archive_errno(archive)}: {archive_error_string(archive)}")
    }
}

//? Base class of OX package reader and writter.
OxpBase: class {
    /*?
     *? Throw an archive error.
     *? @param msg {String} The error message.
     */
    error(msg) {
        throw ArchiveError(this.#archive, msg)
    }
}

//? OX package writter.
public OxpWritter: class OxpBase {
    /*?
     *? Initialize an OX package writter.
     *? @param name {string} The filename of the oxp file.
     */
    $init(name) {
        this.#archive = archive_write_new()
        this.filename = name

        try {
            if archive_write_set_format_pax_restricted(this.#archive) != ARCHIVE_OK {
                this.error(L"\"{"archive_write_set_format_pax_restricted"}\" failed")
            }

            if archive_write_add_filter_gzip(this.#archive) != ARCHIVE_OK {
                this.error(L"\"{"archive_write_add_filter_gzip"}\" failed")
            }

            if archive_write_open_filename(this.#archive, name) != ARCHIVE_OK {
                this.error(L"\"{"archive_write_open_filename"}\" \"{name}\" failed")
            }
        } catch e {
            archive_write_free(this.#archive)
            throw e
        }
    }

    //Write data to archive.
    #write_data(buf, len) {
        off = 0
        while len > 0 {
            r = archive_write_data(this.#archive, C.get_ref(buf, off), len)
            if r < 0 {
                this.error(L"\"archive_write_data\" failed")
            }

            len -= r
            off += r
        }
    }

    /*?
     *? Add a symbol link to the package.
     *? @param lnk {String} The symbol link filename.
     *? @param target {String} The target of the symbol link.
     */
    add_symlink(lnk, target) {
        ent = archive_entry_new()

        try {
            archive_entry_set_pathname(ent, target)
            archive_entry_set_filetype(ent, AE_IFLNK)
            archive_entry_set_symlink(ent, lnk)

            while true {
                r = archive_write_header(this.#archive, ent)
                if r == ARCHIVE_OK {
                    break
                } elif (r == ARCHIVE_FATAL) || (r == ARCHIVE_FAILED) {
                    this.error(L"\"{"archive_write_header"}\" failed")
                }
            }
        } finally {
            archive_entry_free(ent)
        }
    }

    /*?
     *? Add a file to the package.
     *? @param src {String} The source filename.
     *? @param dst {String} The destination filename.
     */
    add_file(src, dst) {
        if !this.#archive {
            throw ReferenceError(L"the archive is closed")
        }

        st = Path(src)
        if !st.exist {
            throw NullError(L"cannot find \"{src}\"")
        }

        if st.format != Path.FMT_REG && st.format != Path.FMT_LNK {
            return
        }

        ent = archive_entry_new()

        try {
            archive_entry_set_pathname(ent, dst)

            if st.format == Path.FMT_LNK {
                archive_entry_set_filetype(ent, AE_IFLNK)
                lnk = readlink(src)
                archive_entry_set_symlink(ent, lnk)
            } else {
                archive_entry_set_filetype(ent, AE_IFREG)
                archive_entry_set_size(ent, st.size)
            }

            archive_entry_set_perm(ent, st.mode)
            archive_entry_set_mtime(ent, st.mtime/1000, 0)

            while true {
                r = archive_write_header(this.#archive, ent)
                if r == ARCHIVE_OK {
                    break
                } elif (r == ARCHIVE_FATAL) || (r == ARCHIVE_FAILED) {
                    this.error(L"\"{"archive_write_header"}\" failed")
                }
            }

            if st.format == Path.FMT_REG {
                if dst == "package.ox" {
                    pkg = JSON.from_file(src)

                    package_schema.validate_throw(pkg)

                    if pkg.files {
                        Object.del_prop(pkg, "files")
                    }

                    s = JSON.to_str(pkg, "  ")
                    this.#write_data(s.to_uint8_ptr(), s.length)

                    this.info = s
                } else {
                    buf = UInt8(512*1024)
                    #file = File(src, "rb")

                    while true {
                        n = file.read(buf)
                        if n == 0 {
                            break
                        }

                        this.#write_data(buf, n)
                    }

                    log.debug("pack \"{dst}\" to \"{this.filename}\"")
                }
            }
        } finally {
            archive_entry_free(ent)
        }
    }

    // Pack the files in a directory.
    #pack_dir(src, dst) {
        #dir = Dir(src)
        for dir as fn {
            if fn == "." || fn == ".." {
                continue
            }

            full_src = "{src}/{fn}"

            if dst {
                full_dst = "{dst}/{fn}"
            } else {
                full_dst = fn
            }

            fmt = Path(full_src, true).format
            if fmt == Path.FMT_DIR {
                this.#pack_dir(full_src, full_dst)
            } elif (fmt == Path.FMT_REG) || (fmt == Path.FMT_LNK) {
                this.add_file(full_src, full_dst)
            }
        }
    }

    /*?
     *? Pack the files in the directory.
     *? @param dn {String} The directory name.
     */ 
    pack(dn) {
        this.#pack_dir(dn)
    }

    //? Close the OX package writter.
    $close() {
        if this.#archive {
            archive_write_close(this.#archive)
            archive_write_free(this.#archive)
            this.#archive = null
        }
    }
}

//? OX package reader.
public OxpReader: class OxpBase {
    /*?
     *? Initialize an OX package reader.
     *? @param name {String} The package filename.
     */
    $init(name) {
        this.filename = name
    }

    //Open the archive.
    #open() {
        this.#archive = archive_read_new()

        try {
            if archive_read_support_filter_gzip(this.#archive) != ARCHIVE_OK {
                this.error(L"\"{"archive_read_support_filter_gzip"}\" failed")
            }

            if archive_read_support_format_tar(this.#archive) != ARCHIVE_OK {
                this.error(L"\"{"archive_read_support_format_tar"}\" failed")
            }

            if archive_read_open_filename(this.#archive, this.filename, 512*1024) != ARCHIVE_OK {
                this.error(L"\"{"archive_read_open_filename"}\" \"{this.filename}\" failed")
            }
        } catch e {
            this.#close()
            throw e
        }
    }

    //Close the archive.
    #close() {
        if this.#archive {
            archive_read_free(this.#archive)
            this.#archive = null
        }
    }

    //? The package information.
    info {
        if !this.#info {
            this.#open()

            try {
                files = []
                ent = archive_entry.pointer()
                while true {
                    r = archive_read_next_header(this.#archive, &ent)
                    if r == ARCHIVE_EOF {
                        break
                    } elif r == ARCHIVE_FATAL || r == ARCHIVE_FAILED {
                        this.error(L"\"{"archive_read_next_header"}\" failed")
                    } elif r == ARCHIVE_OK {
                        pn = archive_entry_pathname(ent)

                        if pn == "package.ox" {
                            sb = String.Builder()
                            buf = UInt8(512 * 1024)

                            while true {
                                n = archive_read_data(this.#archive, buf, C.get_length(buf))
                                if n > 0 {
                                    sb.append(buf.$to_str(0, n))
                                } elif n == 0 {
                                    break
                                } else {
                                    this.error(L"\"{"archive_read_data"}\" failed")
                                }
                            }

                            try {
                                this.#info = JSON.from_str(sb)
                                this.#info.files = files
                                package_schema.validate_throw(this.#info)
                            } catch e {
                                e = SyntaxError(L"\"package.ox\" of \"{this.filename}\" syntax error: {e}")
                                throw e
                            }

                            files.push("%pkg%/package.ox")
                        } else {
                            files.push(pn.$to_str())
                        }
                    }
                }
            } finally {
                this.#close()
            }
        }

        return this.#info
    }

    /*?
     *? Unpack the package to a directory.
     *? @param dir {String} The output directory.
     *? @param fn_cb {FilenameCallback} The filename convertor function.
     */
    unpack(dir, fn_cb) {
        st = Path(dir)
        if !st.exist || st.format != Path.FMT_DIR {
            throw NullError(L"directory \"{dir}\" does not exist")
        }

        info = this.info

        this.#open()

        try {
            ent = archive_entry.pointer()
            buf = UInt8(512*1024)

            while true {
                r = archive_read_next_header(this.#archive, &ent)
                if r == ARCHIVE_EOF {
                    break
                } elif r == ARCHIVE_FATAL || r == ARCHIVE_FAILED {
                    this.error(L"\"{"archive_read_next_header"}\" failed")
                } elif r == ARCHIVE_OK {
                    pn = archive_entry_pathname(ent).$to_str()
                    fmt = archive_entry_filetype(ent)
                    mode = archive_entry_perm(ent)

                    if fmt == AE_IFLNK {
                        dst = fn_cb(dir, info, pn)
                        target = archive_entry_symlink(ent)

                        symlink(target, dst)
                    } elif fmt == AE_IFREG {
                        if pn == "package.ox" {
                            dst = fn_cb(dir, info, "%pkg%/package.ox")
                        } else {
                            dst = fn_cb(dir, info, pn)
                        }

                        /*if Path(dst).exist {
                            throw ReferenceError(L"file \"{dst}\" already exists")
                        }*/

                        mkdir_p(dirname(dst))

                        if pn == "package.ox" {
                            File.store_text(dst, JSON.to_str(info, "  "))
                        } else {
                            #file = File(dst, "wb")
                            while true {
                                n = archive_read_data(this.#archive, buf, C.get_length(buf))
                                if n > 0 {
                                    file.write(buf, 0, n)
                                } elif n == 0 {
                                    break
                                } else {
                                    this.error(L"\"{"archive_read_data"}\" failed")
                                }
                            }
                        }

                        chmod(dst, mode)
                    }

                    log.debug("unpack \"{dst}\" from \"{this.filename}\"")
                }
            }
        } finally {
            this.#close()
        }
    }
}
