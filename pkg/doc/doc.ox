/*?
 *? @package doc OX document generator.
 *? OX document generator can scan OX source files and collect
 *? document data from the comments. And generate the document
 *? files for the OX packages.
 *? @exe Document generator.
 *? Run the program as follows:
 *? @code{
 *? ox -r doc [OPTION]... [FILE]...
 *? @code}
 *? The files are the OX source files contains document data.
 */

ref "std/io"
ref "std/option"
ref "std/fs"
ref "json"
ref "./parser"
ref "./document"
ref "./markdown"
ref "./html"
ref "./log"

//Show usage message.
show_usage: func() {
    stdout.puts(L''
Usage: ox -r doc [OPTION]... FILE...
Generate document of OX package.
Option:
{{options.help()}}
    '')
}

out_dir = "."
out_format = "md"
package_name = "my_package"
show_help = false

//?
options: Option([
    {
        short: "f"
        arg: ["md", "html"]
        help: L"Set the output document's format"
        on_option: func(opt, arg) {
            @out_format = arg
        }
    }
    {
        short: "o"
        arg: Option.STRING
        help: L"Set the output directory's name"
        on_option: func(opt, arg) {
            @out_dir = arg
        }
    }
    {
        short: "p"
        arg: Option.STRING
        help: L"Set the package name"
        on_option: func(opt, arg) {
            @package_name = arg
        }
    }
    {
        long: "help"
        help: L"Show this help message"
        on_option: func(opt, arg) {
            show_usage()
            @show_help = true
        }
    }
])

//Parse options
if !options.parse(argv) {
    show_usage()
    return 1
}

if options.index == argv.length {
    stderr.puts(L"no input file specified\n")
    return 1
}

//Parse the input files.
doc = Document()

for i = options.index; i < argv.length; i += 1 {
    filename = argv[i]
    
    ast = parse(doc, filename)
}

if doc.valid {
    mkdir_p(out_dir)

    case out_format {
    "md" {
        gen_markdown_doc(doc, package_name, out_dir)
    }
    "html" {
        gen_html_doc(doc, package_name, out_dir)
    }
    }
} else {
    stdout.puts(L"no document data found in the files\n")
}
