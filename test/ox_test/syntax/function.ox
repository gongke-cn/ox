ref "../test"
ref "std/log"

log: Log("function")

f = func() {
    return 1234
}
test(f() == 1234, "return value")

f = func(a) {
    return -a
}
test(f(1234) == -1234, "return parameter")

f = func(a, b) {
    return a + b
}
test(f(1234, 4321) == 5555, "return sum")

f = func(a, b) => a + b
test(f(1234, 4321) == 5555, "expression function")

f = func() {
    for argv as arg {
        sum += arg
    }
    return sum
}
test(f(1,2,3,4) == 10, "argv")

f = func(a,b,...rest) {
    r = a
    for rest as sub {
        r += sub
    }
    r += b
    return r
}
test(f("[", "]", "a", "b", "c") == "[abc]", "rest parameters")

f = func(a=1234, b=4321) => a + b
test(f() == 5555, "default value of parameter")
test(f(1) == 4322, "default value of parameter")
test(f(null, 1) == 1235, "default value of parameter")

f = func([a,b,c=3]) => a + b + c
test(f([1,2,3]) == 6, "array pattern parameters")
test(f([1,2]) == 6, "array pattern parameters")

f = func({a, b, c=3}) => a + b + c
test(f({a:1, b:2, c:3}) == 6, "object pattern parameters")
test(f({a:1, b:2}) == 6, "object pattern parameters")

f = func() {
    a = 1
    v = func () {
        a = 2
        return a
    }()
    test(a == 1)
    test(v == 2)
}

f = func() {
    a = 1
    v = func () {
        @a = 2
        return a
    }()
    test(a == 2)
    test(v == 2)
}

f = func(a, cb) {
    r = []
    for a as i {
        if cb(i) {
            r.push(i)
        }
    }
    return r;
}
test_array(f([0,1,2,3,4,5], ($%2==0)), [0,2,4])

f = func(a,b) => a+b
test(f(1, (2.5).floor()) == 3)