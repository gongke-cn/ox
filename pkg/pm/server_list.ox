ref "json"
ref "./log"

sl_schema: JsonSchema({
    type: "array"
    items: {
        type: "string"
    }
})

//Load the server list from a file.
public server_list_load: func(filename) {
    sl = JSON.from_file(filename)

    sl_schema.validate_throw(sl)

    return sl
}