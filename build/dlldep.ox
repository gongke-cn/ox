#!/ucrt64/bin/ox

ref "std/option"
ref "std/io"
ref "std/shell"
ref "std/copy"
ref "std/path_conv"

//Show usage.
show_usage: func() {
    stdout.puts(''
Usage: dlldep.ox [OPTION]... FILE
Option:
{{options.help()}}
    '')
}

copy_dir = null

options: Option([
    {
        short: "c"
        arg: Option.STRING
        help: "Copy the dependent dll files to the directory"
        on_option: func(opt, arg) {
            @copy_dir = arg
        }
    }
    {
        long: "help"
        help: "Show this help message"
        on_option: func {
            show_usage()
        }
    }
])

if !options.parse(argv) {
    return 1
}

if options.index >= argv.length {
    stderr.puts("no filename specified\n")
    return 1
}

filename = argv[options.index]
cmd = "ldd {filename}"
out = Shell.output(cmd)

for out.split("\n") as line {
    m = line.match(/\s*(.+)\s*=>\s*(.+)\s*\(.+\)/)
    if !m {
        continue
    }

    dll_name = m.groups[1]
    dll_file = m.groups[2]

    if dll_file ~ /^\/c\/Windows\// {
        continue
    }

    if copy_dir {
        copy(mixed_path(dll_file), mixed_path("{copy_dir}/{dll_name}"))
    } else {
        stdout.puts("{dll_file}\n")
    }
}

return 0
