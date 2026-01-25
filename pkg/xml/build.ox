ref "../../build/pinfo"

config = argv[0]

{
    name: "xml"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "XML parse libraries"
        zh: "XML解析库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std:0.0.1")
    development_dependencies: get_deps("ox_devel", "pb")
    libraries: [
        "xml"
    ]
    oxn_modules: {
        xml_input: {
            sources: [
                "xml_input.oxn.c"
            ]
        }
    }
    internal_libraries: [
        "log"
        "xml_input"
    ]
}