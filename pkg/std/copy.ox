/*?
 *? @lib Copy file functions.
 */

ref "std/log"
ref "std/io"
ref "std/fs"
ref "std/dir"
ref "std/path"
ref "std/math"

log: Log("copy")
log.level = Log.WARNING

/*?
 *? Copy a file.
 *? @param src {String} The source filename.
 *? @param dst {String} The destination filename.
 *? @param mode {?Number} If mode is not null, it is the destination file's mode.
 *? @param off {?Number} If off is not null, it is the copy begin offset in bytes.
 *? @param size {?Number} If size is not null, it is the copy size in bytes. 
 */
public copy: func(src, dst, mode, off, size) {
    #sf = File(src, "rb")
    #df = File(dst, "wb")

    if off != null {
        sf.seek(off)
    }

    buf_size = 512 * 1024

    if size != null {
        buf_size = min(buf_size, size)
    }

    buf = UInt8(buf_size)
    num = null

    while true {
        if size != null {
            num = min(size, buf_size)
        }

        n = sf.read(buf, 0, num)
        if n == 0 {
            break
        }

        df.write(buf, 0, n)

        if size != null {
            size -= n
        }
    }

    if mode {
        chmod(dst, mode)
    }
}

/*?
 *? Copy files in a directory to the destination directory.
 *? @param src_dir {String} The source directory.
 *? @param dst_dir {String} The destination directory.
 *? @param mode {?Number} If mode is not null, it is the destination files' mode.
 */
public copy_dir: func(src_dir, dst_dir, mode) {
    #dir = Dir(src_dir)

    for dir as file {
        if (file == ".") || (file == "..") {
            continue
        }

        src_file = "{src_dir}/{file}"

        if Path(src_file).format == Path.FMT_REG {
            dst_file = "{dst_dir}/{file}"

            copy(src_file, dst_file, mode)
        }
    }
}

/*?
 *? Copy all files and directories to the destination directory.
 *? @param src_dir {String} The source directory.
 *? @param dst_dir {String} The destination directory.
 *? @param mode {?Number} If mode is null, it is the destination files' mode.
 *? @param dir_mode {?Number} If dir_mode is not null, it is the sub directory's mode.
 */
public copy_dir_r: func(src_dir, dst_dir, mode, dir_mode) {
    #dir = Dir(src_dir)

    for dir as file {
        if (file == ".") || (file == "..") {
            continue
        }

        src_file = "{src_dir}/{file}"

        fmt = Path(src_file).format;

        if fmt == Path.FMT_REG {
            dst_file = "{dst_dir}/{file}"

            copy(src_file, dst_file, mode)
        } elif fmt == Path.FMT_DIR {
            dst_file = "{dst_dir}/{file}"

            mkdir_p(dst_file, dir_mode)

            copy_dir_r(src_file, dst_file, mode, dir_mode)
        }
    }
}
