ref "json/json_schema"
ref "./package_schema"

//Package list's schema.
public package_list_schema: JsonSchema({
    type: "array"
    items: {
        allOf: [
            package_schema.json
            {
                properties: {
                    size: {
                        type: "integer"
                    }
                }
                required: [
                    "size"
                ]
            }
        ]
    }
})