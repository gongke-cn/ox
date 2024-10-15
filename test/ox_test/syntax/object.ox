ref "../test"
ref "std/log"

log: Log("object")

test_object: func(o1, o2, msg) {
    ok = true

    for Object.entries(o1) as [k, v] {
        if o2[k] != v {
            ok = false
            break
        }
    }

    if ok {
        for Object.entries(o2) as [k, v] {
            if o1[k] != v {
                ok = false
                break
            }
        }
    }

    test(ok, msg, 2)
}

test_object({}, {})
test_object({a:1, b:2}, {b:2, a:1})
test_object({a:1,...{b:2, c:3}}, {a:1, b:2, c:3})

test_object({"a":1}, {a:1})
test_object({["a"+"b"]: 1}, {ab: 1})

a=1
test_object({a}, {a:1})

a={}
a.{
    a:1,
    b:2
}
test_object(a, {a:1, b:2})

a={a:{a:1}, b:[1,2]}
a.{
    a.{
        b:2
    }
    b.[
        3
        4
    ]
}
test(a.a.a == 1)
test(a.a.b == 2)
test(a.b.length == 4)
for i=0; i < 4; i += 1 {
    test(a.b[i] == i + 1)
}


test_object({enum {a,b,c}}, {a:0, b:1, c:2})
test_object({bitfield {a,b,c}}, {a:1, b:2, c:4})

a={
    set_value: func(v) {
        this.p = v
    }
}
a.{
    set_value(1234)
}
test(a.p == 1234)

i = 0
for Object.keys({a:0, b:1, c:2, d:3, e:4}) as k {
    test(k.length == 1)
    test(k.char_at(0) == 'a' + i)
    i += 1
}
test(i == 5)

i = 0
for Object.values({a:0, b:1, c:2, d:3, e:4}) as v {
    test(v == i)
    i += 1
}
test(i == 5)

i = 0
for Object.entries({a:0, b:1, c:2, d:3, e:4}) as [k, v] {
    test(k.length == 1)
    test(k.char_at(0) == 'a' + i)
    test(v == i)
    i += 1
}
test(i == 5)

a={
    enum e {
        A
        B
        C
        D
        E
    }
}
i = 0
for a.e.keys() as k {
    test(k == i)
    i += 1
}
test(i == 5)
i = 0
for a.e.values() as v {
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 5)
i = 0
for a.e.entries() as [k, v] {
    test(k == i)
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 5)

a={
    bitfield b {
        A
        B
        C
        D
        E
    }
}
i = 0
for a.b.keys() as k {
    test(k == 1 << i)
    i += 1
}
test(i == 5)
i = 0
for a.b.values() as v {
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 5)
i = 0
for a.b.entries() as [k, v] {
    test(k == 1 << i)
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 5)

flag=0
o={
    a:1
    if flag {
        b: 2
    } else {
        b: 3
    }
}
test(o.a==1)
test(o.b==3)

flag=1
o={
    a:1
    if flag {
        b: 2
    } else {
        b: 3
    }
}
test(o.a==1)
test(o.b==2)

flag=0
o={
    a:1
    case flag {
    1 {
        b:1
    }
    2 {
        b:2
    }
    * {
        b:3
    }
    }
}
test(o.a==1)
test(o.b==3)

flag=1
o={
    a:1
    case flag {
    1 {
        b:1
    }
    2 {
        b:2
    }
    * {
        b:3
    }
    }
}
test(o.a==1)
test(o.b==1)

flag=2
o={
    a:1
    case flag {
    1 {
        b:1
    }
    2 {
        b:2
    }
    * {
        b:3
    }
    }
}
test(o.a==1)
test(o.b==2)
