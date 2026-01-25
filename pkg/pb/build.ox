ref "../../build/pinfo"

config = argv[0]

{
    name: "pb"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX package builder"
        zh: "OX脚本语言软件包构建工具"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1", "pm:0.0.1", "gettext", "doc", "oxngen", "json", "oxp", "ox_devel", "gcc", "gnu-gettext", "pkgconf")
    executables: [
        "pb"
    ]
    internal_libraries: [
        "prepare"
        "build_doc"
        "build_oxn"
        "build_oxngen"
        "build_package"
        "build_text"
        "build_oxp"
        "config"
        "loader"
        "tools"
        "log"
    ]
}
