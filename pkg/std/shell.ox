/*?
 *? @lib Rnu system shell.
 */

ref "./log"
ref "./lang"
ref "./system"
ref "./process"

log: Log("shell")
log.level = Log.WARNING

v = getenv("SHELL")
if v {
    shell_cmd = "{v} -c"
} elif OX.os == "windows" {
    v = getenv("COMSPEC")
    if v {
        shell_cmd = "{v} /C"
    } else {
        shell_cmd = "C:\\Windows\\System32\\cmd.exe /C"
    }
} else {
    shell_cmd = "/bin/sh -c"
}

/*?
 *? Shell process.
 *? @inherit {Process}
 */
public Shell: class Process {
    /*?
     *? Shell verbo mode.
     */
    bitfield verbo {
        //? Enable output.
        OUTPUT
        //? Enable error output.
        ERROR
    }

    /*?
     *? Initialize a new shell process.
     *? @param cmd {String} The shell command line.
     *? @param flags {Number} The process flags.
     */
    $init(cmd, flags) {
        cl = "{shell_cmd} \"{cmd.replace(/["\\]/, "\\$&")}\""

        Process.$inf.$init.call(this, cl, flags)
    }

    /*?
     *? Create a new shell process and wait until it return.
     *? @param cmd {String} The shell command line.
     *? @param verbo {Shell.verbo} The shell's verbo mode.
     *? @return {Number} The return code of the shell.
     */
    static run(cmd, verbo) {
        flags = 0

        if !(verbo & Shell.OUTPUT) {
            flags |= Process.NULLOUT
        }

        if !(verbo & Shell.ERROR) {
            flags |= Process.NULLERR
        }

        p = Shell(cmd, flags)

        return p.wait()
    }

    /*?
     *? Run a shell and return its output.
     *? @param cmd {String} The shell command line.
     *? @return {String} The output of the shell.
     */
    static output(cmd) {
        p = Shell(cmd, Process.STDOUT|Process.NULLIN|Process.NULLERR)
        sb = String.Builder()

        while true {
            line = p.stdout.gets()
            if line == null {
                break
            }

            sb.append(line)
        }

        p.wait()

        return sb.$to_str()
    }

    /*?
     *? Run a shell and return its error output.
     *? @param cmd {String} The shell command line.
     *? @return {String} The error output of the shell.
     */
    static error(cmd) {
        p = Shell(cmd, Process.STDERR|Process.NULLIN|Process.NULLOUT)
        sb = String.Builder()

        while true {
            line = p.stderr.gets()
            if line == null {
                break
            }

            sb.append(line)
        }

        p.wait()

        return sb.$to_str()
    }
}
