config = argv[0]

{
    name: "test"
    homepage: config.p.homepage
    maintainer: config.p.maintainer
    description: {
        "en": "test oxn"
    }
    version: "0.0.1"
    dependencies: {
    }
    development_dependencies: {
    }
    libraries: [
        "test"
    ]
    oxngen_targets: {
        test: {
            input_files: [
                "test.h"
            ]
        }
    }
}
