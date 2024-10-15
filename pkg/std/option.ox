/*?
 *? @lib Command option parser.
 */

ref "./io"
ref "./log"

log: Log("option")
log.level = Log.WARNING

/*?
 *? @otype{ OptionDesc Option's description.
 *? @var short {String} The short name of the option.
 *? @var long {String} The long name of the option.
 *? @var arg {?Option.ArgType|[String]} The argument type of the option.
 *? @ul{
 *? @li If it is null, means the option has not any argument.
 *? @li If it is an array, it contains the valid values of the argument.
 *? @li Or it is the type of the argument.
 *? @ul}
 *? @var help {String} The help message of this option.
 *? @otype}
 */

//? Command option parser.
public Option: class {
    //? Option's argument types.
    enum ArgType {
        //? Boolean value.
        BOOL
        //? Number value.
        NUMBER
        //? Integer number value.
        INTEGER
        //? String value.
        STRING
    }

    /*?
     *? Initialize the command option parser.
     *? @param opts {[OptionDesc]} Options' description.
     */
    $init(opts) {
        this.#options = opts
        this.#options.sort(func(o1, o2) {
            n1 = if o1.short {o1.short} else {o1.long}
            n2 = if o2.short {o2.short} else {o2.long}

            return n1.compare(n2)
        })
    }

    #lookup_option(opt) {
        odef = null
        mlen = 0

        for this.#options as o {
            if o.short {
                if opt == o.short {
                    odef = o
                    break
                }
            }
            if o.long {
                if opt == o.long {
                    odef = o
                    break
                }
                if o.long.length >= opt.length {
                    if (o.long.slice(0, opt.length) == opt) && !odef {
                        odef = o
                    }
                }
            }
        }

        if !odef {
            stderr.puts(L"illegal option \"{opt}\"\n")
            return null
        }

        return odef
    }

    #check_option(odef, opt, arg) {    
        if (odef.arg == null) && (arg != null) {
            stderr.puts(L"option \"{opt}\" cannot have argument\n")
            return false
        } elif (odef.arg != null) && (arg == null) {
            stderr.puts(L"option \"{opt}\" should have argument\n")
            return false
        }

        case odef.arg {
        Option.BOOL {
            arg = arg.to_lower()

            if arg == "false" || arg == "no" || arg == "0" {
                arg = false
            } elif arg == "true" || arg == "yes" || arg == "1" {
                arg = true
            } else {
                stderr.puts(L"illegal boolean argument \"{arg}\"\n")
                return false
            }
        }
        Option.NUMBER {
            arg = Number(arg)
            if arg.isnan() {
                stderr.puts(L"argument of \"{opt}\" is not a number\n")
                return false
            }
        }
        Option.INTEGER {
            arg = Number(arg)
            if arg.isnan() {
                stderr.puts(L"argument of \"{opt}\" is not a number\n")
                return false
            }

            if arg.floor() != arg {
                stderr.puts(L"argument of \"{opt}\" is not an integer\n")
                return false
            }
        }
        * {
            if Array.is(odef.arg) {
                if !odef.arg.has(arg) {
                    stderr.puts(L"argument \"{arg}\" is not in \"{odef.arg.$iter().$to_str("|")}\"\n")
                    return false
                }
            }
        }
        }

        if odef.on_option {
            odef.on_option(opt, arg)
        } elif this.on_option {
            this.on_option(opt, arg)
        }

        return true
    }

    /*?
     *? Parse the command options.
     *? @param args {[String]} The command line options.
     *? @return {Bool} true on success and false on failed.
     */
    parse(args) {
        for i = 1; i < args.length; i += 1 {
            arg = args[i]
            oarg = null

            if arg[0] == "-" && arg[1] == "-" && arg.length > 2 {
                oname = arg.slice(2)

                match = oname.match(/(.+)=(.*)/)
                if match {
                    oname = match.groups[1]
                    oarg = match.groups[2]
                }

                match_long = true
            } elif arg[0] == "-" && arg.length > 1 {
                oname = arg[1]

                if arg.length > 2 {
                    if arg[2] == "=" {
                        oarg = arg.slice(3)
                    } else {
                        odef = this.#lookup_option(oname)
                        if !odef {
                            return false
                        }

                        if odef.arg == null {
                            if !this.#check_option(odef, oname) {
                                return false
                            }

                            for arg.slice(2) as c {
                                odef = this.#lookup_option(c)
                                if !odef {
                                    return false
                                }

                                if !this.#check_option(odef, c) {
                                    return false
                                }
                            }

                            continue
                        }

                        oarg = arg.slice(2)
                    }
                }

                match_long = false
            } else {
                break
            }

            odef = this.#lookup_option(oname)
            if !odef {
                return false
            }

            if match_long {
                opt = odef.long
            } else {
                opt = odef.short
            }

            if odef.arg != null && oarg == null {
                if i + 1 < args.length {
                    i += 1
                    oarg = args[i]
                }
            }

            if !this.#check_option(odef, opt, oarg) {
                return false
            }
        }

        this.index = i
        return true
    }

    /*?
     *? Get the help message of th options.
     *? @return {String} The help message.
     */
    help() {
        msg = ""
        for this.#options as opt {
            msg += "  "
            if opt.short {
                msg += "-{opt.short}"
                if opt.long {
                    msg += ","
                }
            }
            if opt.long {
                msg += "--{opt.long}"
            }
            if opt.arg != null {
                case opt.arg {
                Option.BOOL {
                    msg += " BOOL"
                }
                Option.NUMBER {
                    msg += " NUMBER"
                }
                Option.INTEGER {
                    msg += " INTEGER"
                }
                Option.STRING {
                    msg += " STRING"
                }
                * {
                    if Array.is(opt.arg) {
                        msg += " {opt.arg.$iter().$to_str("|")}"
                    } else {
                        msg += " {opt.arg}"
                    }
                }
                }
            }
            if opt.help {
                msg += "\n        "
                msg += opt.help
            }
            msg += "\n"
        }

        return msg
    }
}
