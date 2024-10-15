ref "../test"
ref "std/log"
ref "json"

log: Log("json_schema")

json_schema_test: func(sch_json, v, ret) {
    sch = JsonSchema(sch_json)
    r = sch.validate(v)
    test(r == ret, "{JSON.to_str(v)} validate with {JSON.to_str(sch_json)} failed", 3)
}

json_schema_ok: func(sch_json, v) {
    json_schema_test(sch_json, v, true)
}

json_schema_failed: func(sch_json, v) {
    json_schema_test(sch_json, v, false)
}

json_schema_ok({type: "null"}, null)
json_schema_failed({type: "null"}, 1)
json_schema_failed({type: "null"}, "a")
json_schema_failed({type: "null"}, [])
json_schema_failed({type: "null"}, {})
json_schema_failed({type: "null"}, true)
json_schema_failed({type: "null"}, false)

json_schema_ok({type: "boolean"}, true)
json_schema_ok({type: "boolean"}, false)
json_schema_failed({type: "boolean"}, null)
json_schema_failed({type: "boolean"}, 0)
json_schema_failed({type: "boolean"}, "a")
json_schema_failed({type: "boolean"}, [])
json_schema_failed({type: "boolean"}, {})

json_schema_ok({type: "number"}, 0)
json_schema_ok({type: "number"}, 1)
json_schema_ok({type: "number"}, -1)
json_schema_ok({type: "number"}, 3.1415926)
json_schema_failed({type: "number"}, null)
json_schema_failed({type: "number"}, true)
json_schema_failed({type: "number"}, false)
json_schema_failed({type: "number"}, "a")
json_schema_failed({type: "number"}, [])
json_schema_failed({type: "number"}, {})

json_schema_ok({type: "integer"}, 0)
json_schema_ok({type: "integer"}, 1)
json_schema_ok({type: "integer"}, -1)
json_schema_failed({type: "integer"}, 3.1415926)
json_schema_failed({type: "integer"}, null)
json_schema_failed({type: "integer"}, true)
json_schema_failed({type: "integer"}, false)
json_schema_failed({type: "integer"}, "a")
json_schema_failed({type: "integer"}, [])
json_schema_failed({type: "integer"}, {})

json_schema_ok({type: "array"}, [])
json_schema_ok({type: "array"}, [1])
json_schema_ok({type: "array"}, [1,2])
json_schema_failed({type: "array"}, null)
json_schema_failed({type: "array"}, true)
json_schema_failed({type: "array"}, false)
json_schema_failed({type: "array"}, 1)
json_schema_failed({type: "array"}, "a")
json_schema_failed({type: "array"}, {})

json_schema_ok({type: "object"}, {})
json_schema_ok({type: "object"}, {a:1})
json_schema_ok({type: "object"}, {a:1, b:2})
json_schema_failed({type: "object"}, null)
json_schema_failed({type: "object"}, true)
json_schema_failed({type: "object"}, false)
json_schema_failed({type: "object"}, 1)
json_schema_failed({type: "object"}, "a")
json_schema_failed({type: "object"}, [])

json_schema_ok({"enum": [1,2,3]}, 1)
json_schema_ok({"enum": [1,2,3]}, 2)
json_schema_ok({"enum": [1,2,3]}, 3)
json_schema_failed({"enum": [1,2,3]}, 0)
json_schema_failed({"enum": [1,2,3]}, 4)

json_schema_ok({const: "a"}, "a")
json_schema_failed({const: "a"}, "")
json_schema_failed({const: "a"}, "b")

json_schema_ok({multipleOf: 2}, 0)
json_schema_ok({multipleOf: 2}, 2)
json_schema_ok({multipleOf: 2}, 4)
json_schema_failed({multipleOf: 2}, 1)
json_schema_failed({multipleOf: 2}, 3)

json_schema_ok({maximum: 2}, 0)
json_schema_ok({maximum: 2}, -1)
json_schema_ok({maximum: 2}, 1)
json_schema_ok({maximum: 2}, 2)
json_schema_failed({maximum: 2}, 3)

json_schema_ok({exclusiveMaximum: 2}, 0)
json_schema_ok({exclusiveMaximum: 2}, -1)
json_schema_ok({exclusiveMaximum: 2}, 1)
json_schema_ok({exclusiveMaximum: 2}, 1.999)
json_schema_failed({exclusiveMaximum: 2}, 2)
json_schema_failed({exclusiveMaximum: 2}, 3)

json_schema_ok({minimum: 0}, 0)
json_schema_ok({minimum: 0}, 1)
json_schema_ok({minimum: 0}, 2)
json_schema_failed({minimum: 0}, -0.001)
json_schema_failed({minimum: 0}, -1)

json_schema_failed({exclusiveMinimum: 0}, 0)
json_schema_ok({exclusiveMinimum: 0}, 0.001)
json_schema_ok({exclusiveMinimum: 0}, 1)
json_schema_ok({exclusiveMinimum: 0}, 2)
json_schema_failed({exclusiveMinimum: 0}, -0.001)
json_schema_failed({exclusiveMinimum: 0}, -1)

json_schema_ok({maxLength: 2}, "")
json_schema_ok({maxLength: 2}, "a")
json_schema_ok({maxLength: 2}, "ab")
json_schema_failed({maxLength: 2}, "abc")

json_schema_failed({minLength: 2}, "")
json_schema_failed({minLength: 2}, "a")
json_schema_ok({minLength: 2}, "ab")
json_schema_ok({minLength: 2}, "abc")

json_schema_ok({pattern: "[a-c]*"}, "")
json_schema_ok({pattern: "[a-c]*"}, "a")
json_schema_ok({pattern: "[a-c]*"}, "c")
json_schema_ok({pattern: "[a-c]*"}, "abc")
json_schema_failed({pattern: "[a-c]*"}, "abc1")

json_schema_ok({maxItems: 2}, [])
json_schema_ok({maxItems: 2}, [1])
json_schema_ok({maxItems: 2}, [1, 2])
json_schema_failed({maxItems: 2}, [1, 2, 3])

json_schema_failed({minItems: 2}, [])
json_schema_failed({minItems: 2}, [1])
json_schema_ok({minItems: 2}, [1, 2])
json_schema_ok({minItems: 2}, [1, 2, 3])

json_schema_ok({uniqueItems: true}, [])
json_schema_ok({uniqueItems: true}, [1])
json_schema_ok({uniqueItems: true}, [1, 2])
json_schema_ok({uniqueItems: true}, [1, 2, 3])
json_schema_ok({uniqueItems: true}, [1, 2, 3, 4])
json_schema_failed({uniqueItems: true}, [1, 2, 3, 4, 1])

json_schema_ok({prefixItems: [{const: 1}, {const: 2}]}, [1, 2])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}]}, [1, 2, 3])
json_schema_failed({prefixItems: [{const: 1}, {const: 2}]}, [0, 2])
json_schema_failed({prefixItems: [{const: 1}, {const: 2}]}, [1, 1])

json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}}, [1, 2])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}}, [1, 2, "a"])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}}, [1, 2, "a", "b"])
json_schema_failed({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}}, [1, 2, "a", "b", 1])

json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}, unevaluatedItems: {const: false}}, [false, 2, "a", "b"])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}, unevaluatedItems: {const: false}}, [1, false, "a", "b"])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}, unevaluatedItems: {const: false}}, [1, 2, false, "b"])
json_schema_ok({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}, unevaluatedItems: {const: false}}, [1, 2, "a", false])
json_schema_failed({prefixItems: [{const: 1}, {const: 2}], items: {type: "string"}, unevaluatedItems: {const: false}}, [1, 2, "a", true])

json_schema_ok({contains: {const: 1}}, [1])
json_schema_ok({contains: {const: 1}}, [1, 2])
json_schema_ok({contains: {const: 1}}, [2,3,4,5,6,7,1])
json_schema_failed({contains: {const: 1}}, [2,3,4,5,6,7])

json_schema_ok({contains: {const: 1}, minContains: 2}, [1, 2, 3, 4, 1])
json_schema_failed({contains: {const: 1}, minContains: 2}, [1, 2, 3, 4])
json_schema_ok({contains: {const: 1}, minContains: 0}, [2, 3, 4])

json_schema_ok({contains: {const: 1}, maxContains: 2}, [1, 2, 3, 4])
json_schema_ok({contains: {const: 1}, maxContains: 2}, [1, 2, 3, 4, 1])
json_schema_failed({contains: {const: 1}, maxContains: 2}, [1, 2, 3, 4, 1, 1])

json_schema_ok({maxProperties: 2}, {})
json_schema_ok({maxProperties: 2}, {a:1})
json_schema_ok({maxProperties: 2}, {a:1, b:2})
json_schema_failed({maxProperties: 2}, {a:1, b:2, c:3})

json_schema_failed({minProperties: 2}, {})
json_schema_failed({minProperties: 2}, {a:1})
json_schema_ok({minProperties: 2}, {a:1, b:2})
json_schema_ok({minProperties: 2}, {a:1, b:2, c:3})

json_schema_ok({required: ["a", "b"]}, {a:1, b:2, c:3})
json_schema_failed({required: ["a", "b"]}, {b:2, c:3})
json_schema_failed({required: ["a", "b"]}, {a:1, c:3})

json_schema_ok({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {a:1, a1:2, a2:3})
json_schema_ok({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {b:1, b1:2, b2:3})
json_schema_failed({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {a:1, a1:2})
json_schema_failed({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {a:1, a2:2})
json_schema_failed({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {b:1, b1:2})
json_schema_failed({dependentRequired: {a:["a1", "a2"], b:["b1", "b2"]}}, {b:1, b2:2})

json_schema_ok({propertyNames: {maxLength: 2}}, {a:1})
json_schema_ok({propertyNames: {maxLength: 2}}, {ab:1})
json_schema_failed({propertyNames: {maxLength: 2}}, {abc:1})
json_schema_ok({propertyNames: {maxLength: 2}}, {a:1, ab:1})
json_schema_failed({propertyNames: {maxLength: 2}}, {a:1, ab:1, abc:1})

json_schema_ok({properties: {a: {type: "integer"}, b: {type: "boolean"}}}, {a:1})
json_schema_ok({properties: {a: {type: "integer"}, b: {type: "boolean"}}}, {b:true})
json_schema_ok({properties: {a: {type: "integer"}, b: {type: "boolean"}}}, {a:0, b:false})
json_schema_failed({properties: {a: {type: "integer"}, b: {type: "boolean"}}}, {a:true, b:false})
json_schema_failed({properties: {a: {type: "integer"}, b: {type: "boolean"}}}, {a:1, b:1})

json_schema_ok({patternProperties: {"[a-z]+": {type: "integer"}, "[A-Z]+": {type: "boolean"}}}, {a:1, A:true})
json_schema_failed({patternProperties: {"[a-z]+": {type: "integer"}, "[A-Z]+": {type: "boolean"}}}, {a:true, A:true})
json_schema_failed({patternProperties: {"[a-z]+": {type: "integer"}, "[A-Z]+": {type: "boolean"}}}, {a:1, A:"a"})

json_schema_ok({properties: {a: {type: "integer"}}, additionalProperties: {type: "string"}}, {a:1})
json_schema_failed({properties: {a: {type: "integer"}}, additionalProperties: {type: "string"}}, {a:"a"})
json_schema_ok({properties: {a: {type: "integer"}}, additionalProperties: {type: "string"}}, {b:"a"})

json_schema_ok({properties: {a: {type: "integer"}}, additionalProperties: {type: "string"}, unevaluatedProperties: {type: "boolean"}}, {a:true})
json_schema_ok({properties: {a: {type: "integer"}}, additionalProperties: {type: "string"}, unevaluatedProperties: {type: "boolean"}}, {b:true})
json_schema_ok({properties: {a: {type: "integer"}}, unevaluatedProperties: {type: "boolean"}}, {b:true})
json_schema_failed({properties: {a: {type: "integer"}}, unevaluatedProperties: {type: "boolean"}}, {b:"a"})

json_schema_ok({not: {type: "null"}}, true)
json_schema_ok({not: {type: "null"}}, 1)
json_schema_ok({not: {type: "null"}}, "a")
json_schema_ok({not: {type: "null"}}, [])
json_schema_ok({not: {type: "null"}}, {})
json_schema_failed({not: {type: "null"}}, null)

json_schema_ok({allOf: [{type: "integer"}, {multipleOf: 2}]}, 0)
json_schema_ok({allOf: [{type: "integer"}, {multipleOf: 2}]}, 2)
json_schema_ok({allOf: [{type: "integer"}, {multipleOf: 2}]}, 4)
json_schema_failed({allOf: [{type: "integer"}, {multipleOf: 2}]}, 1)

json_schema_ok({anyOf: [{type: "integer"}, {type: "boolean"}]}, 0)
json_schema_ok({anyOf: [{type: "integer"}, {type: "boolean"}]}, true)
json_schema_failed({anyOf: [{type: "integer"}, {type: "boolean"}]}, null)
json_schema_ok({anyOf: [{type: "integer"}, {multipleOf: 2}]}, 0)
json_schema_ok({anyOf: [{type: "integer"}, {multipleOf: 2}]}, 1)
json_schema_ok({anyOf: [{type: "integer"}, {multipleOf: 2}]}, 2)

json_schema_ok({oneOf: [{type: "integer"}, {type: "boolean"}]}, 0)
json_schema_ok({oneOf: [{type: "integer"}, {type: "boolean"}]}, true)
json_schema_failed({oneOf: [{type: "integer"}, {type: "boolean"}]}, null)
json_schema_failed({oneOf: [{type: "integer"}, {multipleOf: 2}]}, 0)
json_schema_ok({oneOf: [{type: "integer"}, {multipleOf: 2}]}, 1)
json_schema_failed({oneOf: [{type: "integer"}, {multipleOf: 2}]}, 2)

json_schema_ok({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, 0)
json_schema_ok({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, 2)
json_schema_ok({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, true)
json_schema_ok({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, false)
json_schema_failed({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, 1)
json_schema_failed({"if": {type: "integer"}, then: {multipleOf: 2}, else: {type: "boolean"}}, "a")

sch = {
    $defs:{
        integer: {
            type: "integer"
            $anchor: "integer"
        }
    }
    properties: {
        a: {
            $ref: "#/$defs/integer"
        }
        b: {
            $ref: "#/$defs/integer"
        }
        c: {
            $ref: "#integer"
        }
    }
}

json_schema_ok(sch, {a:1, b:2, c:3})
json_schema_failed(sch, {a:true, b:2, c:3})
json_schema_failed(sch, {a:1, b:true, c:3})
json_schema_failed(sch, {a:1, b:2, c:true})

JsonSchemaLoader: func(uri) {
    if uri == "http://ox.org/boolean.json" {
        return JsonSchema({type: "boolean"})
    } elif uri == "http://ox.org/dir/integer.json" {
        return JsonSchema({type: "integer"})
    } elif uri == "http://ox.org/defs.json" {
        return JsonSchema({string: {type: "string"}})
    }

    test(false, "load illegal json {uri}")
}

sch = JsonSchema({$ref: "boolean.json"}, JsonSchemaLoader, "http://ox.org/test.json")
test(sch.validate(true), "loader test failed")

sch = JsonSchema({$ref: "integer.json"}, JsonSchemaLoader, "http://ox.org/dir/test.json")
test(sch.validate(1), "loader test failed")

sch = JsonSchema({$ref: "defs.json#/string"}, JsonSchemaLoader, "http://ox.org/test.json")
test(sch.validate(""), "loader test failed")
