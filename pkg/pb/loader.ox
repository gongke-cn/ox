ref "std/io"
ref "std/lang"
ref "std/path"
ref "json"
ref "./config"
ref "./log"

//Build description's schema.
build_schema: JsonSchema({
    type: "object"
    properties: {
        name: {
            type: "string"
        }
        homepage: {
            type: "string"
        }
        maintainer: {
            type: "string"
        }
        description: {
            anyOf: [
                {
                    type: "string"
                }
                {
                    type: "object"
                    additionalProperties: {
                        type: "string"
                    }
                }
            ]
        }
        version: {
            type: "string"
            pattern: "[0-9]+(\.[0-9]+)*(-[0-9]+)?"
        }
        libraries: {
            type: "array"
            items: {
                type: "string"
            }
        }
        internal_libraries: {
            type: "array"
            items: {
                type: "string"
            }
        }
        executables: {
            type: "array"
            items: {
                type: "string"
            }
        }
        internal_files: {
            type: "array"
            items: {
                type: "string"
            }
        }
        system_files: {
            type: "object"
            additionalProperties: {
                type: "string"
            }
        }
        system_package: {
            type: "boolean"
        }
        dependencies: {
            type: "object"
            additionalProperties: {
                type: "string"
                pattern: "[0-9]+(\.[0-9]+)*(-[0-9]+)?"
            }
        }
        development_dependencies: {
            type: "object"
            additionalProperties: {
                type: "string"
                pattern: "[0-9]+(\.[0-9]+)*(-[0-9]+)?"
            }
        }
        oxn_modules: {
            type: "object"
            additionalProperties: {
                type: "object"
                properties: {
                    sources: {
                        type: "array"
                        items: {
                            type: "string"
                        }
                    }
                    cflags: {
                        type: "string"
                    }
                    libs: {
                        type: "string"
                    }
                }
            }
        }
        oxngen_targets: {
            type: "object"
            properties: {
                cflags: {
                    type: "string"
                }
                libs: {
                    type: "string"
                }
                input_files: {
                    type: "array"
                    items: {
                        type: "string"
                    }
                }
                references: {
                    type: "object"
                    additionalProperties: {
                        type: "array"
                        items: "string"
                    }
                }
                file_filters: {
                    type: "array"
                    items: {
                        type: "string"
                    }
                }
                rev_file_filters: {
                    type: "array"
                    items: {
                        type: "string"
                    }
                }
                decl_filters: {
                    type: "array"
                    items: {
                        type: "string"
                    }
                }
                rev_decl_filters: {
                    type: "array"
                    items: {
                        type: "string"
                    }
                }
            }
        }
        pre_build: {
            type: "string"
        }
        post_build: {
            type: "string"
        }
        pre_install: {
            type: "string"
        }
        post_install: {
            type: "string"
        }
        pre_uninstall: {
            type: "string"
        }
        post_uninstall: {
            type: "string"
        }
    }
    required: [
        "name"
        "description"
        "version"
    ]
})

//Load the "build.ox"
public build_load: func(fn) {
    //Load the file.
    try {
        if config.json {
            json = JSON.from_file(fn)
        } else {
            json = OX.file(fn)(config)
        }
    } catch e {
        if e.type instof AccessError {
            stderr.puts(L"cannot open \"{fn}\"\n")
        } elif e.type instof SystemError {
            stderr.puts(L"read \"{fn}\" failed\n")
        } elif e.type instof SyntaxError {
            stderr.puts(L"\"{fn}\" is not a valid build description file\n")
        }

        throw e
    }

    //Validate by schema
    try {
        build_schema.validate_throw(json)
    } catch e {
        throw SyntaxError(L"\"{fn}\" syntax error: {e}")
    }

    //Check if the module exists.
    module_exist: func(mod) {
        if Path("{mod}.ox").exist {
            return true
        }

        if json.oxn_modules?[mod] {
            return true
        }

        if json.oxngen_targets?[mod] {
            return true
        }
    }

    //Check module.
    check_module: func(mod) {
        if !module_exist(mod) {
            throw NullError(L"module \"{mod}\" is not defined")
        }    
    }

    //Check modules.
    if json.libraries {
        for json.libraries as lib {
            check_module(lib)
        }
    }

    if json.executables {
        for json.executables as exe {
            check_module(exe)
        }
    }

    if json.internal_libraries {
        for json.internal_libraries as lib {
            check_module(lib)
        }
    }

    //Check oxn module.
    check_oxn: func(name) {
        if !json.executables?.has(name) &&
                !json.libraries?.has(name) &&
                !json.internal_libraries?.has(name) {
            throw ReferenceError(L"oxn module \"{name}\" is not defined as library or executable")
        }
    }

    if json.oxn_modules {
        for Object.keys(json.oxn_modules) as name {
            check_oxn(name)
        }
    }

    if json.oxngen_targets {
        for Object.keys(json.oxngen_targets) as name {
            check_oxn(name)
        }
    }

    return json
}
