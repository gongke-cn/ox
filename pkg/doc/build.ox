ref "../../build/pinfo"

config = argv[0]

{
    name: "doc"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "Document generater"
        zh: "文档生成器"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std")
    executables: [
        "doc"
    ]
    internal_libraries: [
        "error"
        "input"
        "command"
        "document"
        "html"
        "markdown"
        "parser"
        "log"
    ]
}