ref "../test"
ref "std/log"

log: Log("dict")

DICT_LENGTH: 1024

dict = Dict()
test(dict.length == 0)

for i = 0; i < DICT_LENGTH; i += 1 {
    k = String(i)

    test(dict.get(k) == null)
    dict.add(k, i)
    test(dict.length == i + 1)
}

for i = 0; i < DICT_LENGTH; i += 1 {
    k = String(i)
    test(dict.get(k) == i)
}

i = 0
for dict.keys() as k {
    test(k == String(i))
    i += 1
}
test(i == DICT_LENGTH)

i = 0
for dict.values() as v {
    test(v == i)
    i += 1
}
test(i == DICT_LENGTH)

i = 0
for dict.entries() as [k, v] {
    test(k == String(i))
    test(v == i)
    i += 1
}
test(i == DICT_LENGTH)

i = 0
for dict as [k, v] {
    test(k == String(i))
    test(v == i)
    i += 1
}
test(i == DICT_LENGTH)

for i = 0; i < DICT_LENGTH; i += 1 {
    k = String(i)
    test(dict.remove(k))
    test(dict.get(k) == null)
    test(dict.length == DICT_LENGTH - i - 1)
}

dict = Dict().{"a":0, "b":1, "c":2, "d":3, "a":1, "b":2, "c":3, "d":4}
test(dict.length == 4)
test(dict["a"] == 1)
test(dict["b"] == 2)
test(dict["c"] == 3)
test(dict["d"] == 4)
dict["a"] = 5
dict["b"] = 6
dict["c"] = 7
dict["d"] = 8
test(dict["a"] == 5)
test(dict["b"] == 6)
test(dict["c"] == 7)
test(dict["d"] == 8)