ref "std/shell"
ref "../../build/pinfo"

config = argv[0]

{
    name: "pm"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "OX package manager"
        zh: "OX脚本语言包管理器"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std", "curl", "json", "oxp")
    executables: [
        "pm"
    ]
    internal_libraries: [
        "tools"
        "config"
        "log"
        "server_list"
        "server_manager"
        "package_list"
        "local_pm"
        "delay_task"
    ]
}
