ref "../../build/pinfo"

config = argv[0]

out = config.p.outdir
so_version = config.p.so_version
libdir = config.p.libdir

{
    name: "ox_devel"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX script language development libraries and headers"
        zh: "OX脚本语言开发库和头文件"
    }
    version: "0.0.1"
    dependencies: get_deps("libffi_devel")
    system_files: {
        if config.os == "windows" {
            "%lib%/libox.dll.a": "{out}/{libdir}/libox.dll.a"
        } else {
            "%lib%/libox.so": "?libox.so.{so_version}"
        }

        "%inc%": "../../include"
        "%lib%/pkgconfig/libox.pc": "{out}/{libdir}/pkgconfig/libox.pc"
        "%lib%/libox.a": "{out}/{libdir}/libox.a"
    }
}