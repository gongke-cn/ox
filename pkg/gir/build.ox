ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

{
    name: "gir"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "GObject introspection library"
        zh: "GObject 反射库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std", "libgirepository-2.0")
    development_dependencies: get_deps("ox_devel", "pb", "libglib2.0_devel")
    libraries: [
        "gir"
    ]
    oxn_modules: {
        gir: {
            cflags: Shell.output("{config.pc} girepository-2.0 --cflags").trim()
            libs: Shell.output("{config.pc} girepository-2.0 --libs").trim()
            sources: [
                "gir.oxn.c"
            ]
        }
    }
}
