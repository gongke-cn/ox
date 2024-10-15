ref "../../build/pinfo"

config = argv[0]

out = config.p.outdir
so_version = config.p.so_version
libdir = config.p.libdir

deps = ["libffi"]

if config.os == "windows" {
    deps.[
        "libiconv"
        "libintl"
    ]
} else {
    deps.[
        "libm"
        "libc"
    ]
}

{
    name: "ox"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX script language"
        zh: "OX脚本语言"
    }
    version: "0.0.1"
    dependencies: get_deps(...deps)
    system_files: {
        if config.os == "windows" {
            "bin/ox-cli.exe": "{out}/bin/ox-cli.exe"
            "bin/ox-win.exe": "{out}/bin/ox-win.exe"
            "bin/libox-{so_version}.dll": "{out}/bin/libox-{so_version}.dll"
        } else {
            "bin/ox-cli": "{out}/bin/ox-cli"
            "%lib%/libox.so.{so_version}": "{out}/{libdir}/libox.so.{so_version}"
        }

        "share/locale/zh_CN/LC_MESSAGES/ox.mo": "{out}/share/locale/zh_CN/LC_MESSAGES/ox.mo"
        "share/ox/doc/md/builtin": "{out}/share/ox/doc/md/builtin"
        "share/ox/doc/md/ox": "{out}/share/ox/doc/md/ox"
        "share/ox/server/main.ox": "{out}/share/ox/server/main.ox"
    }
}