ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

cflags = Shell.output("{config.pc} ncursesw --cflags").trim()

if config.os != "windows" {
    cflags = "-DNCURSES_WIDECHAR=0 {cflags}"
}

{
    name: "ncurses"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "ncurses - character-cell terminal interface with optimized output"
        zh: "ncurses - 终端界面字符输出库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1", "libncursesw")
    development_dependencies: get_deps("ox_devel", "pb", "libncurses_devel")
    libraries: [
        "ncurses"
    ]
    oxngen_targets: {
        ncurses: {
            input_files: [
                "ncurses.h"
            ]
            cflags
            libs: Shell.output("{config.pc} ncursesw --libs").trim()
            references: {
                "std/io": [
                    "FILE"
                ]
            }
            rev_decl_filters: [
                /_.+/p
                "TRUE"
                "FALSE"
                "CURSES"
                "CURSES_H"
                /alloc_pair(_sp)?/p
                /reset_color_pairs(_sp)?/p
                /init_extended_pair(_sp)?/p
                /init_extended_color(_sp)?/p
                /extended_color_content(_sp)?/p
                /extended_pair_content(_sp)?/p
                /extended_slk_color(_sp)?/p
                /free_pair(_sp)?/p
                /find_pair(_sp)?/p
                /slk_attr_on(_sp)?/p
                /slk_attr_off(_sp)?/p
            ]
        }
    }
}
