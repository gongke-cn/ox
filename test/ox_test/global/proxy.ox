ref "../test"
ref "std/log"

log: Log("proxy")

get_thiz
get_name
set_thiz
set_name
set_val

templ: {
    get: func(name) {
        @get_thiz = this
        @get_name = name
        return "getv"
    }
    set: func(name, v) {
        @set_thiz = this
        @set_name = name
        @set_val = v
    }
}

thiz = {}
p: Proxy(templ, thiz)

test(p.test == "getv")
test(get_thiz == thiz)
test(get_name == "test")

p.test = 1234
test(set_thiz == thiz)
test(set_name == "test")
test(set_val == 1234)