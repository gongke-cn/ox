ref "std/option"
ref "std/io"
ref "std/log"
ref "../test"

log = Log("option")

OptFlag = class {
    bitfield {
        A
        B
        C
        D
        E
    }
}

flags = 0
barg = null
carg = null
darg = null
earg = null

options = Option([
    {
        short: "a"
        long: "along"
        help: "option a"
        on_option: func (opt, arg) {
            test(opt == "a" || opt == "along")
            @flags |= OptFlag.A
            test(arg == null)
        }
    }
    {
        short: "b"
        long: "blong"
        arg: Option.BOOL
        help: "option b"
        on_option: func (opt, arg) {
            test(opt == "b" || opt == "blong")
            @flags |= OptFlag.B
            test(typeof arg == Bool)
            @barg = arg
        }
    }
    {
        short: "c"
        long: "clong"
        arg: Option.INTEGER
        help: "option c"
        on_option: func (opt, arg) {
            test(opt == "c" || opt == "clong")
            @flags |= OptFlag.C
            test(typeof arg == Number)
            test(arg.floor() == arg)
            @carg = arg
        }
    }
    {
        short: "d"
        long: "dlong"
        arg: Option.NUMBER
        help: "option d"
        on_option: func (opt, arg) {
            test(opt == "d" || opt == "dlong")
            @flags |= OptFlag.D
            test(typeof arg == Number)
            @darg = arg
        }
    }
    {
        short: "e"
        long: "elong"
        arg: Option.STRING
        help: "option e"
        on_option: func (opt, arg) {
            test(opt == "e" || opt == "elong")
            @flags |= OptFlag.E
            test(typeof arg == String)
            @earg = arg
        }
    }
])

/*stdout.puts("\
Options:
{options.help()}\
")*/

flags = 0
options.parse(["cmd"])
test(flags == 0)

flags = 0
test(options.parse(["cmd", "-a"]))
test(flags == OptFlag.A)

flags = 0
test(options.parse(["cmd", "--along"]))
test(flags == OptFlag.A)

flags = 0
test(options.parse(["cmd", "-b", "1"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "-b1"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "-b=1"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "--blong", "yes"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "--blong", "true"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "--blong=true"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "--bl=true"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "--blo=true"]))
test(flags == OptFlag.B)
test(barg == true)

flags = 0
test(options.parse(["cmd", "-b", "0"]))
test(flags == OptFlag.B)
test(barg == false)

flags = 0
test(options.parse(["cmd", "-b", "no"]))
test(flags == OptFlag.B)
test(barg == false)

flags = 0
test(options.parse(["cmd", "-b", "false"]))
test(flags == OptFlag.B)
test(barg == false)

flags = 0
test(options.parse(["cmd", "-c", "12345"]))
test(flags == OptFlag.C)
test(carg == 12345)

flags = 0
test(options.parse(["cmd", "-c12345"]))
test(flags == OptFlag.C)
test(carg == 12345)

flags = 0
test(options.parse(["cmd", "--clong", "-12345"]))
test(flags == OptFlag.C)
test(carg == -12345)

flags = 0
test(options.parse(["cmd", "-d", "3.14"]))
test(flags == OptFlag.D)
test(darg == 3.14)

flags = 0
test(options.parse(["cmd", "--dlong", "-3.14"]))
test(flags == OptFlag.D)
test(darg == -3.14)

flags = 0
test(options.parse(["cmd", "-e", "abc"]))
test(flags == OptFlag.E)
test(earg == "abc")

flags = 0
test(options.parse(["cmd", "--elong", "abc"]))
test(flags == OptFlag.E)
test(earg == "abc")