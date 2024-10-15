ref "../../build/pinfo"

config = argv[0]

{
    name: "oxngen"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX native module generator"
        zh: "OX脚本语言原生模块生成器"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std", "json", "clang")
    executables: [
        "oxngen"
    ]
    internal_libraries: [
        "cdecl"
        "cmacro"
        "ctype"
        "oxnsrc"
        "tools"
        "log"
    ]
}