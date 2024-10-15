/*?
 *? @lib JSON schema validator.
 */

ref "std/io"
ref "std/uri"
ref "std/path"
ref "./json"
ref "./json_pointer"
ref "./log"

/*?
 *? @callback JsonLoader JSON schema loader function.
 *? @param uri {String} The URI of the JSON schema file.
 *? @return The JSON schema object.
 */

//Get the value's type.
get_type: func(v) {
    if v == null {
        return "null"
    }

    t = typeof v
    case t {
    Bool {
        return "boolean"
    }
    String {
        return "string"
    }
    Int8
    UInt8
    Int16
    UInt16
    Int32
    UInt32
    Int64
    UInt64
    Float32
    Float64
    Number {
        return "number"
    }
    Array {
        return "array"
    }
    * {
        return "object"
    }
    }
}

//Check if 2 values are equal.
equal: func(v1, v2) {
    if v1 == null && v2 == null {
        return true
    } elif v1 == null || v2 == null {
        return false
    } else {
        t1 = typeof v1
        t2 = typeof v2
        if t1 != t2 {
            return false
        }

        case t1 {
        Bool
        Number
        String {
            return v1 == v2
        }
        Array {
            if v1.length != v2.length {
                return false
            }

            for i = 0; i < v1.length; i += 1 {
                if !equal(v1[i], v2[i]) {
                    return false
                }
            }

            return true
        }
        * {
            for Object.entries(v1) as [pk, pv] {
                if !equal(pv, v2[pk]) {
                    return false
                }
            }

            return true
        }
        }
    }
}

//JSON schema validate error.
public JsonSchemaError: class Error {
    //Initialize a JSON schema error.
    $init (v, msg) {
        this.v = v
        this.msg = msg
    }

    //Convert the error to string.
    $to_str() {
        if this.v {
            return "{JSON.to_str(this.v)}: {this.msg}"
        } else {
            return this.msg
        }
    }
}

/*?
 *? JSON schema validator.
 */
public JsonSchema: class {
    //Collect anchors.
    #collect_anchors(v) {
        if Object.is(v) && !Array.is(v) {
            if v.$anchor {
                anchor = v.$anchor
            } elif v.$dynamicAnchor {
                anchor = v.$dynamicAnchor
            } else {
                anchor = null
            }

            if anchor {
                if !this.#anchors[anchor] {
                    this.#anchors[anchor] = v
                }
            }

            for Object.values(v) as pv {
                this.#collect_anchors(pv)
            }
        }
    }

    //Lookup reference.
    lookup_ref(rn) {
        uri = URI(rn)

        if uri.path && this.#uri {
            if !uri.host {
                if !uri.scheme {
                    uri.scheme = this.#uri.scheme
                }

                if !uri.userinfo {
                    uri.userinfo = this.#uri.userinfo
                }

                uri.host = this.#uri.host

                if !uri.port {
                    uri.port = this.#uri.port
                }

                if !uri.query {
                    uri.query = this.#uri.query
                }
            }

            if uri.path[0] != "/" {
                dn = dirname(this.#uri.path)

                if dn == "/" {
                    uri.path = "/{uri.path}"
                } else {
                    uri.path = "{dn}/{uri.path}"
                }
            }
        }

        fragment = uri.fragment

        if uri.host || uri.path {
            uri.fragment = null

            if !this.#loader {
                throw NullError(L"JSON schema loader is not specified")
            }

            sch = this.#loader(uri.$to_str())
        } else {
            sch = this
        }

        if !fragment {
            return sch.#json
        } elif fragment[0] == "/" {
            jp = JsonPointer(fragment)
            pv = sch.#json

            for jp.pointers as link {
                if Array.is(pv) {
                    idx = link.$to_num()
                    if idx.isnan() {
                        throw TypeError(L"cannot find the reference \"{rn}\"")
                    }

                    child = pv[idx]
                } else {
                    child = Object.lookup(pv, link)
                }

                if !child {
                    throw NullError(L"cannot find the reference \"{rn}\"")
                }

                pv = child
            }

            return pv
        } else {
            ptr = JsonPointer.unescape(fragment)

            if sch.#anchors[ptr] {
                return sch.#anchors[ptr]
            } else {
                throw NullError(L"cannot find the reference \"{rn}\"")
            }
        }
    }

    //Get the property of a shema value.
    #get(v, pk) {
        pv = Object.lookup(v, pk)
        if !pv {
            r = v.$ref

            if !r {
                r = v.$dynamicRef
            }

            if r {
                rv = this.lookup_ref(r)

                if rv {
                    pv = Object.lookup(rv, pk)
                }
            }
        }

        return pv
    }

    //? JSON of the schema.
    json {
        return this.#json
    }

    /*?
     *? Initialize the JSON schema validator.
     *? @param json The JSON describe the validator.
     *? @param loader {?JsonLoader} The JSON schema loader function.
     *? @param uri {?String} The URI of this JSON schema file.
     */
    $init(json, loader, uri) {
        this.#loader = loader
        this.#anchors = {}
        this.#json = json
        this.#collect_anchors(json)

        if uri {
            this.#uri = URI(uri)
        }
    }

    //Validate
    #validate(v, sch) {
        //Failed.
        fail: func(msg) {
            throw JsonSchemaError(v, msg)
        }

        if sch == true {
            return
        } elif sch == false {
            fail(L"the schema is always false")
        }

        // "allOf"
        if (csch = this.#get(sch, "allOf")) {
            for csch as sub {
                this.#validate(v, sub)
            }
        }

        // "anyOf"
        if (csch = this.#get(sch, "anyOf")) {
            match = false
            for csch as sub {
                try {
                    this.#validate(v, sub)
                    match = true
                } catch err {
                }
            }

            if !match {
                return fail(L"the value does not match any of {JSON.to_str(csch)}")
            }
        }

        // "oneOf"
        if (csch = this.#get(sch, "oneOf")) {
            match = 0
            for csch as sub {
                try {
                    this.#validate(v, sub)
                    match += 1
                } catch err {
                }
            }

            if match == 0 {
                fail(L"the value does not match any of {JSON.to_str(csch)}")
            } elif match > 1 {
                fail(L"the value match many items of {JSON.to_str(csch)}")
            }
        }

        // "not"
        if (csch = this.#get(sch, "not")) {
            try {
                this.#validate(v, csch)
                match = false
            } catch err {
                match = true
            }

            if !match {
                fail(L"the value cannot match {JSON.to_str(csch)}")
            }
        }

        // if ... then ... else
        if (csch = this.#get(sch, "if")) {
            try {
                this.#validate(v, csch)
                match = true
            } catch err {
                match = false
            }

            if match {
                if (csch = this.#get(sch, "then")) {
                    this.#validate(v, csch)
                }
            } else {
                if (csch = this.#get(sch, "else")) {
                    this.#validate(v, csch)
                }
            }
        }

        // "dependentSchemas"
        if (csch = this.#get(sch, "dependentSchemas")) {
            for Object.entries(csch) as [pk, psub] {
                if Object.lookup(v, pk) {
                    this.#validate(v, psub)
                }
            }
        }

        // "type"
        if (csch = this.#get(sch, "type")) {
            type = get_type(v)

            if Array.is(csch) {
                if !csch.has(type) && (type != "number" || !csch.has("integer") || v.floor() != v) {
                    fail(L"the value is not in valid type {JSON.to_str(csch)}")
                }
            } else {
                if csch != type && (type != "number" || csch != "integer" || v.floor() != v) {
                    fail(L"the value's type is not \"{csch}\"")
                }
            }
        }

        // "enum"
        if (csch = this.#get(sch, "enum")) {
            match = false
            for csch as item {
                if equal(item, v) {
                    match = true
                    break
                }
            }
            if !match {
                fail(L"the value is not in enum {JSON.to_str(csch)}")
            }
        }

        // "const"
        if (csch = this.#get(sch, "const")) != null {
            if !equal(csch, v) {
                fail(L"the value is not equal to {JSON.to_str(csch)}")
            }
        }

        case typeof v {
        // Boolean
        Bool {
        }
        // Number/integer instance.
        Number {
            // "multipleOf"
            if (csch = this.#get(sch, "multipleOf")) != null {
                r = v / csch
                if r.floor() != r {
                    fail(L"the value is not multiple of {csch}")
                }
            }

            // "maximum"
            if (csch = this.#get(sch, "maximum")) != null {
                if v > csch {
                    fail(L"must <= maximum value {csch}")
                }
            }

            // "exclusiveMaximum"
            if (csch = this.#get(sch, "exclusiveMaximum")) != null {
                if v >= csch {
                    fail(L"must < maximum value {csch}")
                }
            }

            // "minimum"
            if (csch = this.#get(sch, "minimum")) != null {
                if v < csch {
                    fail(L"must >= minimum value {csch}")
                }
            }

            // "exclusiveMinimum"
            if (csch = this.#get(sch, "exclusiveMinimum")) != null {
                if v <= csch {
                    fail(L"must > minimum value {csch}")
                }
            }
        }
        //String instance.
        String {
            // "maxLength"
            if (csch = this.#get(sch, "maxLength")) != null {
                if v.length > csch {
                    fail(L"the length must <= {csch}")
                }
            }

            // "minLength"
            if (csch = this.#get(sch, "minLength")) != null {
                if v.length < csch {
                    fail(L"the length must >= {csch}")
                }
            }

            // "pattern"
            if (csch = this.#get(sch, "pattern")) {
                if typeof csch == Re {
                    re = csch
                } else {
                    re = Re(csch, "p")
                }

                if !v.match(re) {
                    fail(L"does not match pattern {re}")
                }
            }
        }
        //Array instance
        Array {
            // "maxItems"
            if (csch = this.#get(sch, "maxItems")) != null {
                if v.length > csch {
                    fail(L"the length must <= {csch}")
                }
            }

            // "minItems"
            if (csch = this.#get(sch, "minItems")) != null {
                if v.length < csch {
                    fail(L"the length must >= {csch}")
                }
            }

            // "uniqueItems"
            if this.#get(sch, "uniqueItems") {
                for i = 0; i < v.length - 1; i += 1 {
                    for j = i + 1; j < v.length; j += 1 {
                        if equal(v[i], v[j]) {
                            fail(L"all array items must be unique")
                        }
                    }
                }
            }

            idx = 0
            n_contains = 0

            csch = this.#get(sch, "contains")
            usch = this.#get(sch, "unevaluatedItems")

            // "prefixItems"
            if (psch = this.#get(sch, "prefixItems")) {
                for psch as isch {
                    if csch {
                        try {
                            this.#validate(v[idx], csch)
                            n_contains += 1
                        } catch err {
                        }
                    }

                    try {
                        try {
                            this.#validate(v[idx], isch)
                        } catch err {
                            if usch {
                                this.#validate(v[idx], usch)
                            } else {
                                throw err
                            }
                        }
                    } catch e {
                        throw JsonSchemaError(null, "[{idx}]: {e}")
                    }
                    idx += 1
                }
            }

            // "items"
            if (isch = this.#get(sch, "items")) {
                for ; idx < v.length; idx += 1 {
                    if csch {
                        try {
                            this.#validate(v[idx], csch)
                            n_contains += 1
                        } catch err {
                        }
                    }

                    try {
                        try {
                            this.#validate(v[idx], isch)
                        } catch err {
                            if usch {
                                this.#validate(v[idx], usch)
                            } else {
                                throw err
                            }
                        }
                    } catch e {
                        throw JsonSchemaError(null, "[{idx}]: {e}")
                    }
                }
            }

            // "unevaluatedItems"
            if usch {
                for ; idx < v.length; idx += 1 {
                    if csch {
                        try {
                            this.#validate(v[idx], csch)
                            n_contains += 1
                        } catch err {
                        }
                    }

                    try {
                        this.#validate(v[idx], usch)
                    } catch e {
                        throw JsonSchemaError(null, "[{idx}]: {e}")
                    }
                }
            }

            // "contains"
            if csch {
                for ; idx < v.length; idx += 1 {
                    try {
                        this.#validate(v[idx], csch)
                        n_contains += 1
                    } catch err {
                    }
                }

                // "minContains"
                if (msch = this.#get(sch, "minContains")) != null {
                    min = msch
                } else {
                    min = 1
                }

                if n_contains < min {
                    fail(L"contains {JSON.to_str(csch)} < {min} times")
                }

                // "maxContains"
                if (msch = this.#get(sch, "maxContains")) != null {
                    if n_contains > msch {
                        fail(L"contains {JSON.to_str(csch)} > {msch} times")
                    }
                }
            }
        }
        // Object instance
        * {
            //Properties number
            max_sch = this.#get(sch, "maxProperties")
            min_sch = this.#get(sch, "minProperties")
            if min_sch != null || max_sch != null {
                n = Object.keys(v).to_array().length

                if max_sch {
                    if n > max_sch {
                        fail(L"properties number must < {max_sch}")
                    }
                }

                if min_sch {
                    if n < min_sch {
                        fail(L"properties number must > {min_sch}")
                    }
                }
            }

            // "required"
            if (csch = this.#get(sch, "required")) {
                for csch as pk {
                    if !Object.lookup(v, pk) {
                        fail(L"property \"{pk}\" is required")
                    }
                }
            }

            // "dependentRequired"
            if (csch = this.#get(sch, "dependentRequired")) {
                for Object.entries(csch) as [depk, keys] {
                    if Object.lookup(v, depk) {
                        for keys as key {
                            if !Object.lookup(v, key) {
                                fail(L"property \"{key}\" is required if \"{depk}\" is present")
                            }
                        }
                    }
                }
            }

            //Properties
            p_sch = this.#get(sch, "properties")
            pn_sch = this.#get(sch, "propertyNames")
            pat_sch = this.#get(sch, "patternProperties")
            add_sch = this.#get(sch, "additionalProperties")
            uneval_sch = this.#get(sch, "unevaluatedProperties")
            if p_sch || pat_sch || add_sch || uneval_sch || pn_sch {
                for Object.entries(v) as [pk, pv] {
                    try {
                        if pn_sch {
                            this.#validate(pk, pn_sch)
                        }

                        if p_sch {
                            psch = p_sch[pk]
                            if psch {
                                try {
                                    this.#validate(pv, psch)
                                    continue
                                } catch err {
                                    if uneval_sch {
                                        this.#validate(pv, uneval_sch)
                                    } else {
                                        throw err
                                    }
                                }
                            }
                        }

                        if pat_sch {
                            for Object.entries(pat_sch) as [pat, psch] {
                                if pk ~ Re(pat, "p") {
                                    try {
                                        this.#validate(pv, psch)
                                        continue
                                    } catch err {
                                        if uneval_sch {
                                            this.#validate(pv, uneval_sch)
                                        } else {
                                            throw err
                                        }
                                    }
                                }
                            }
                        }

                        if add_sch {
                            try {
                                this.#validate(pv, add_sch)
                                continue
                            } catch err {
                                if uneval_sch {
                                    this.#validate(pv, uneval_sch)
                                } else {
                                    throw err
                                }
                            }
                        }

                        if uneval_sch {
                            this.#validate(pv, uneval_sch)
                        }
                    } catch e {
                        throw JsonSchemaError(null, "\"{pk}\": {e}")
                    }
                }
            }
        }
        }

        return true
    }

    /*?
     *? Validate the value.
     *? @param v The value to be validated.
     *? @throw {JsonSchemaError} Validate failed.
     */
    validate_throw(v) {
        this.#validate(v, this.#json)
    }

    /*?
     *? Validate the value.
     *? @param v The value to be validated.
     *? @return {Bool} Validate successful or failed.
     */
    validate(v) {
        try {
            this.#validate(v, this.#json)
        } catch e {
            if e instof JsonSchemaError {
                return false
            } else {
                throw e
            }
        }

        return true
    }
}
