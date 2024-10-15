ref "json/json_schema"

//Package's schema.
public package_schema: JsonSchema({
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
        system_package: {
            type: "boolean"
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
        architecture: {
            type: "string"
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
        executables: {
            type: "array"
            items: {
                type: "string"
            }
        }
        dependencies: {
            type: "object"
            additionalProperties: {
                type: "string"
                pattern: "[0-9]+(\.[0-9]+)*(-[0-9]+)?"
            }
        }
        files: {
            type: "array"
            items: {
                type: "string"
            }
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
        "architecture"
        "version"
    ]
})
