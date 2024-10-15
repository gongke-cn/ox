ref "../test"
ref "std/log"
ref "std/fiber"

log: Log("fiber")

test1: func() {
    for i=0; i < 100; i+=1 {
        yield i
    }

    return "end"
}

f = Fiber(test1)
for i=0; i<100; i+=1 {
    test(!f.end)
    test(f.value == i)
    f.next()
}
test(f.end)
test(f.value == "end")

test2: func() {
    for i=0; i<100; i+=1 {
        r = yield
        test(r == i)
    }
}

f = Fiber(test2)
for i=0; i<100; i+=1 {
    test(!f.end)
    test(f.value == null)
    f.next(i)
}
test(f.end)
test(f.value == null)

inner: func(n) {
    yield n
}

test3: func() {
    for i=0; i<100; i+=1 {
        inner(i)
    }
}

f = Fiber(test3)
for i=0; i<100; i+=1 {
    test(!f.end)
    test(f.value == i)
    f.next()
}
test(f.end)
test(f.value == null)

inner4: func(n) {
    yield
}

test4: func() {
    for i=0; i<100; i+=1 {
        inner()
    }
}

f = Fiber(test4)
for i=0; i<100; i+=1 {
    test(!f.end)
    test(f.value == null)
    f.next()
}
test(f.end)
test(f.value == null)
