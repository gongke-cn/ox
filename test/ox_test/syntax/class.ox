ref "../test"
ref "std/log"

log: Log("class")

c = class {
    static sm() {
        return "sm"
    }
    m() {
        return "m"
    }
    rd_acce {
        return "rd"
    }
    rw_acce {
        return this.v
    } (v) {
        this.v = String(v)
    }
}
test(c.sm() == "sm")
inst = c()
test(inst.sm == null)
test(inst.m() == "m")
test(inst.rd_acce == "rd")
test(inst.rw_acce == null)
inst.rw_acce = 12345
test(inst.rw_acce == "12345")

p = class {
    p() {
        return "from_p"
    }
}
c = class p {
    p() {
        return "from_c"
    }
}
test(c().p() == "from_c")

p = class {
    a() {
        return "a"
    }
}
c = class p {
    b() {
        return "b"
    }
}
inst = c()
test(inst.a() == "a")
test(inst.b() == "b")

p1 = class {
    a() {
        return "a"
    }
}
p2 = class {
    b() {
        return "b"
    }
}
c = class p1, p2 {
}
inst = c()
test(inst.a() == "a")
test(inst.b() == "b")

c = {
    enum {
        A
        B
        C
    }
}
test(c.A == 0)
test(c.B == 1)
test(c.C == 2)

c = {
    enum E {
        A
        B
        C
    }
}
test(c.A == 0)
test(c.B == 1)
test(c.C == 2)
test(c.E[0] == "A")
test(c.E[1] == "B")
test(c.E[2] == "C")

i = 0
for c.E.keys() as k {
    test(k == i)
    i += 1
}
test(i == 3)
i = 0
for c.E.values() as v {
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 3)
i = 0
for c.E.entries() as [k, v] {
    test(k == i)
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 3)
i = 0
for c.E as [k, v] {
    test(k == i)
    test(v.length == 1)
    test(v.char_at(0) == 'A' + i)
    i += 1
}
test(i == 3)

c = {
    bitfield b {
        A
        B
        C
    }
}
test(c.A == 1)
test(c.B == 2)
test(c.C == 4)
test_array(c.b[1], ["A"])
test_array(c.b[2], ["B"])
test_array(c.b[4], ["C"])
test_array(c.b[3], ["A", "B"])
test_array(c.b[7], ["A", "B", "C"])