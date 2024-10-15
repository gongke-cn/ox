ref "std/option"
ref "std/io"
ref "std/time"
ref "std/text_util"
ref "./ox_parser"
ref "./c_parser"
ref "./log"

/*?
 *? @package gettext The executable program collects strings from the OX source code.
 *? @exe The executable program collects strings from the OX source code.
 *? In OX source code, the string literals with prefixed character 'L' can be collected.
 *? The generated file format is compatible with GNU gettext tools,
 *? and you can use GNU gettext tools for localization these strings.
 *? @br
 *? For example, you have written an OX package "test". There are some message
 *? outputs in the source code as follows:
 *? @code{
 *? stdout.puts(L"hello!\n")
 *? @code}
 *? You can run program "gettext" to collect the message:
 *? @code{
 *? ox -r doc -o test.pot test.ox
 *? @code}
 *? The generated "test.pot" contains the following information:
 *? @code{
 *? #: test.ox:1
 *? msgid "hello!\n"
 *? msgstr ""
 *? @code}
 *? Then you can use GNU gettext tools to generate "po" file, translate it, and build the "mo" file.
 *? Install the "mo" file to the directory "/usr/share/ox/locale/{LANG}/LC_MESSAGES/"
 *? ({LANG} is the language of the "mo" file). When the package "test" runs, it will load the
 *? corresponding "mo" file based on the current language settings.
 *? Run the program as follows:
 *? @code{
 *? ox -r gettext [OPTION]... [FILE]...
 *? @code}
 *? The files are the OX source files contains message strings.
 */

out_filename
show_help = false

show_usage: func() {
    stdout.puts(L''
Usage: ox -r gettext [OPTION]... FILE...
Collect messages in the OX source files.
Option:
{{options.help()}}
        '')
}

//?
options: Option([
    {
        short: "o"
        arg: Option.STRING
        help: L"Set the output filename"
        on_option: func(opt, arg) {
            @out_filename = arg
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

if !options.parse(argv) {
    show_usage()
    return 1
}

if options.index == argv.length {
    stderr.puts(L"no input file specified\n")
    return 1
}

if show_help && options.index == argv.length {
    return 0
}

msg_map = Dict()

for i = options.index; i < argv.length; i += 1 {
    filename = argv[i]

    add_message: func(msg, line) {
        ent = msg_map.get(msg)
        info = "{filename}:{line}"

        if !ent {
            ent = [info]
            msg_map.add(msg, ent)
        } else {
            ent.push(info)
        }
    }

    if filename ~ /\.ox$/i {
        ox_parse(filename, add_message)
    } elif filename ~ /\.(c|h|cc|cxx|cpp|hpp|hxx)/i {
        c_parse(filename, add_message)
    } else {
        stderr.puts(L"format of input file \"{filename}\" is unknown\n")
        return 1
    }
}

now = Time()
hoff = now.hour - now.gm_hour
moff = now.min - now.gm_min
date = "{now.year!04d}-{now.mon!02d}-{now.mday!02d} {now.hour!02d}:{now.min!02d}+{hoff!02d}{moff!02d}"

text = ''
# SOME DESCRIPTIVE TITLE.
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER
# This file is distributed under the same license as the PACKAGE package.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
#, fuzzy
msgid ""
msgstr ""
"Project-Id-Version: PACKAGE VERSION\n"
"Report-Msgid-Bugs-To: \n"
"POT-Creation-Date: {{date}}\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
"Language-Team: LANGUAGE <LL@li.org>\n"
"Language: \n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"

{{msg_map.entries().map(func([k, v]) {
    comment = v.$iter().$to_str(" ")
    msg = c_escape(k)

    return ''
#: {{comment}}
msgid "{{msg}}"
msgstr ""

    ''
}).$to_str("\n")}}
''

if out_filename {
    File.store_text(out_filename, text)
} else {
    stdout.puts(text)
}
