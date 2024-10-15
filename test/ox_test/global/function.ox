ref "../test"
ref "std/log"

log: Log("function")

f = func (a) {
    return -a
}

test(f.call(null, 100) == -100)

MyClass = class {
    $init(v) {
        this.v = v
    }

    f() {
        return - this.v
    }
}

o = MyClass(100)
test(o.f() == -100)

o = {v: 1000}
test(MyClass.$inf.f.call(o) == -1000)