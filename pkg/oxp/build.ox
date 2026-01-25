ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

{
    name: "oxp"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX package operation library"
        zh: "OX脚本语言软件包操作库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1", "archive")
    libraries: [
        "oxp"
        "package_schema"
        "package_list_schema"
    ]
    internal_libraries: [
        "log"
    ]
}
