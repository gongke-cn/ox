ref "../test"
ref "std/log"

log: Log("iterator")

closed = true
MyIter: class Iterator {
    $init(max) {
        this.curr = 0
        this.max = max
    }

    end {
        return this.curr >= this.max
    }

    value {
        return this.curr
    }

    next() {
        this.curr += 1
    }

    $close() {
        @closed = true
    }
}

closed = false
i = 0
iter = MyIter(100)
for iter as v {
    test(i == v)
    i += 1
}
test(i == 100)
test(closed)

closed = false
i = 0
iter = MyIter(100).map(($ * 10))
for iter as v {
    test(i == v / 10)
    i += 1
}
test(i == 100)
test(closed)

closed = false
i = 0
iter = MyIter(100).select(($ % 2 == 0))
for iter as v {
    test(i * 2 == v)
    i += 1
}
test(i == 50)
test(closed)