ref "../../build/pinfo"

config = argv[0]

{
    name: "gettext"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "Translate message tools"
        zh: "信息翻译工具"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1")
    executables: [
        "gettext"
    ]
    internal_libraries: [
        "c_parser"
        "ox_parser"
        "log"
    ]
    oxn_modules: {
        c_parser: {
            sources: [
                "c_parser.oxn.c"
            ]
        }
    }
}