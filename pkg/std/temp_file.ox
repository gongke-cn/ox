/*?
 *? @lib Temporary file.
 */

ref "./io"
ref "./fs"
ref "./path"
ref "./dir_r"
ref "./log"

log: Log("temp_file")
log.level = Log.WARNING

/*?
 *? Temporary file.
 *? When closing the temporary file, the file is removed from the file system.
 */
public TempFile: class {
    /*?
     *? Initialize the temporary file.
     *? @param name {String} The path name of the file.
     */
    $init(name) {
        this.name = name
    }

    /*?
     *? Convert the temporary file to string.
     *? @return {String} Return the filename string.
     */
    $to_str() {
        return this.name
    }

    /*?
     *? Open the temporary file.
     *? @param mode {Number} The file mode bits.
     *? @return {File} The file object.
     */
    open(mode) {
        return File(this.name, mode)
    }

    /*?
     *? Close and remove the temporary file.
     */
    $close() {
        if Path(this.name).exist() {
            unlink(this.name)
        }
    }
}

/*?
 *? Temporary directory.
 *? When closing the temporary directory, the directory and all the files in it are removed.
 */
public TempDir: class {
    /*?
     *? Intialize the temporary directory.
     *? @param name {String} The path name of the directory.
     */
    $init(name) {
        this.name = name
    }

    /*?
     *? Convert the temporary directory to string.
     *? @return {String} The directory's path name string.
     */
    $to_str() {
        return this.name
    }

    /*?
     *? Create the directory.
     *? @param mode {Number} The mode bits.
     */
    mkdir(mode=0o777) {
        mkdir(this.name, mode)
    }

    /*?
     *? Create the directory and its parents.
     */
    mkdir_p() {
        mkdir_p(this.name)
    }

    /*?
     *? Open the temporary directory.
     *? @return {Dir} The directory object.
     */
    open() {
        return DirR(this.name)
    }

    /*?
     *? Close and remove the directory and all the files in it.
     */
    $close() {
        if Path(this.name).exist {
            for DirR(this.name, DirR.DIR_LAST) as file {
                base = basename(file)

                if (base != ".") && (base != "..") {
                    if Path(file).format == Path.FMT_DIR {
                        rmdir(file)
                    } else {
                        unlink(file)
                    }
                }
            }

            rmdir(this.name)
        }
    }
}
