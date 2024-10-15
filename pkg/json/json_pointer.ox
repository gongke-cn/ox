/*?
 *? @lib JSON pointer.
 */

ref "./log"

/*?
 *? JSON pointer.
 */
public JsonPointer: class {
    /*?
     *? Convert characters of the string to JSON pointer escape characters.
     *? @param s {String} Input string.
     *? @return {String} The result string.
     */
    static escape(s) {
        return s.replace(/[~\/]/, func(si) {
            if si.$to_str() == "~" {
                return "~0"
            } else {
                return "~1"
            }
        })
    }

    /*?
     *? Convert JSON pointer escape characters in the string to origin characters.
     *? @param s {String} Input string.
     *? @return {String} The result string.
     */
    static unescape(s) {
        return s.replace(/~0|~1/, func(si) {
            if si.$to_str() == "~0" {
                return "~"
            } else {
                return "/"
            }
        })
    }

    /*?
     *? Build the JSON pointer from the pointers.
     *? The argument list is the properties' keys of the pointer.
     *? For example, JsonPointer.build("a", "b", "0") builds the JSON pointer "/a/b/0".
     *? @return {JsonPointer} The result JSON pointer.
     */
    static build() {
        jp = JsonPointer()
        for argv as arg {
            jp.pointers.push(String(arg))
        }

        return jp
    }

    /*?
     *? Initialize the JSON pointer.
     *? @param s {String} The JSON pointer string.
     */
    $init(s) {
        this.pointers = []

        if s {
            if s.char_at(0) != '/' {
                throw SyntaxError(L"the first character of JSON pointer must be \"/\"")
            }

            pos = 1

            while true {
                npos = s.lookup_char('/', pos)
                if npos == -1 {
                    this.pointers.push(JsonPointer.unescape(s.slice(pos)))
                    break
                } else {
                    this.pointers.push(JsonPointer.unescape(s.slice(pos, npos)))
                    pos = npos + 1
                }
            }

            if this.pointers.length == 0 {
                this.pointers.push("")
            }
        }
    }

    /*?
     *? Convert the JSON pointer to string.
     *? @return {String} The result string.
     */
    $to_str() {
        return "/" + this.pointers.$iter().map((JsonPointer.escape($))).$to_str("/")
    }
}
