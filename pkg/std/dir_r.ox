/*?
 *? @lib Recursive directory.
 *? Recursive directory is used to traverse all files in all subdirectories.
 */

ref "./dir"
ref "./fs"
ref "./path"
ref "./log"

log: Log("dir_r")

/*?
 *? Recursive directory.
 *? Recursive directory inherts the properties of "Iterator".
 *? It is used to traverse all files in all subdirectories.
 */
public DirR: class Iterator {
    //? Traverse order.
    enum Order {
        //? Subdirectories are traversed before the files in it.
        DIR_FIRST,
        //? Subdirectories are traversed after the files in it.
        DIR_LAST,
        //? Skip the subdirectories.
        DIR_SKIP
    }

    /*?
     *? Remove the directory and all files in it.
     *? @param name The directory's name.
     */
    static rmdir(name) {
        #dir = DirR(name, DirR.DIR_LAST)
        for dir as file {
            base = basename(file)
            if base == "." || base == ".." {
                continue
            }

            if Path(file).format == Path.FMT_DIR {
                rmdir(file)
            } else {
                unlink(file)
            }
        }

        rmdir(name)
    }

    #push(dn, prefix) {
        ent = {
            path: dn
            prefix
            dir: Dir(dn)
        }

        ent.bottom = this.#stack
        this.#stack = ent
    }

    #pop() {
        bot = this.#stack.bottom
        this.#stack = bot
    }

    #move_next() {
        ent = this.#stack
        if ent {
            ent.dir.next()
        }
    }

    #store_curr() {
        ent = this.#stack
        if ent {
            this.#curr = "{ent.prefix}/{ent.dir.value}"
        }
    }

    #get_curr() {
        while true {
            if !this.#stack {
                this.#curr = null
                return
            }

            ent = this.#stack
            if ent.dir.end {
                this.#pop()

                ent = this.#stack
                if !ent {
                    continue
                }

                if this.#mode == DirR.DIR_LAST {
                    this.#store_curr()
                    break
                }

                this.#move_next()
                continue
            }

            v = ent.dir.value
            if (Path("{ent.path}/{v}").format == Path.FMT_DIR) {
                if (v == ".") || (v == "..") {
                    if this.#mode == DirR.DIR_SKIP {
                        this.#move_next()
                        continue
                    }

                    this.#store_curr()
                    break
                }

                if this.#mode == DirR.DIR_FIRST {
                    this.#store_curr()
                    this.#push("{ent.path}/{v}", "{ent.prefix}/{v}")
                    break
                }

                this.#push("{ent.path}/{v}", "{ent.prefix}/{v}")
                continue
            }

            this.#store_curr()
            break
        }
    }

    /*?
     *? Initialize the DirR object.
     *? @param dir {String} The root directory name.
     *? @param mode {DirR.Order} The traverse order.
     *? @param prefix {String} =dir The pathname prefixed string.
     */
    $init(dir, mode = DirR.DIR_FIRST, prefix = dir) {
        this.#mode = mode
        this.#push(dir, prefix)

        this.#get_curr()
    }

    //? {Bool} If the iterator is end.
    end {
        return this.#curr == null
    }

    //? {String} The current filename.
    value {
        return this.#curr
    }

    /*?
     *? Move the iterator to the next position.
     */
    next() {
        this.#move_next()
        this.#get_curr()
    }

    /*?
     *? Close the recursive directory object.
     */
    $close() {
        s = this.#stack
        while s {
            t = s.bottom
            s.dir.$close()
            s = t
        }
    }
}
