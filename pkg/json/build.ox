ref "../../build/pinfo"

config = argv[0]

{
    name: "json"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        en: "JSON libraries"
        zh: "JSON库"
    }
    version: "0.0.1"
    dependencies: get_deps("ox", "std")
    development_dependencies: get_deps("ox_devel", "pb")
    libraries: [
        "json"
        "json_pointer"
        "json_schema"
        "log"
    ]
    oxn_modules: {
        json: {
            sources: [
                "json.oxn.c"
            ]
        }
    }
}