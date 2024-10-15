ref "../test"
ref "std/log"

log: Log("set")

SET_LENGTH: 1024

set = Set()
test(set.length == 0)

for i = 0; i < SET_LENGTH; i += 1 {
    v = String(i)
    test(!set.has(v))
    test(set.add(v))
    test(set.length == i + 1)
}

for i = 0; i < SET_LENGTH; i += 1 {
    v = String(i)
    test(set.has(v))
}

i = 0
for set as v {
    test(v == String(i))
    i += 1
}
test(i == SET_LENGTH)

for i = 0; i < SET_LENGTH; i += 1 {
    v = String(i)
    test(set.remove(v))
    test(!set.has(v))
    test(set.length == SET_LENGTH - i - 1)
}

set = Set()
a = 1978.to_int32()
set.add(a)
test(set.length == 1)
b = 1009.to_int32()
set.add(b)
test(set.length == 2)
c = 1978.to_int32()
set.add(c)
test(set.length == 2)

set = Set()
buf = UInt8(1024)
set.add(buf)
test(set.length == 1)
b2 = C.get_ref(buf, 1)
set.add(b2)
test(set.length == 2)
b3 = C.get_ref(buf, 2)
set.add(b3)
test(set.length == 3)
b4 = C.get_ref(buf, 1)
set.add(b4)
test(set.length == 3)

set = Set().[0,1,2,3,4,5,6,5,4,3,2,1,0]
test(set.length == 7)
for i = 0; i < set.length; i += 1 {
    test(set[i] == i)
}