ref "../test"
ref "std/log"

log: Log("string")

test("a".char_at(0) == 0x61, "\"a\"")
test("\n".char_at(0) == 0x0a, "\"\\n\"")
test("\r".char_at(0) == 0x0d, "\"\\r\"")
test("\t".char_at(0) == 0x09, "\"\\t\"")
test("\v".char_at(0) == 0x0b, "\"\\v\"")
test("\f".char_at(0) == 0x0c, "\"\\f\"")
test("\a".char_at(0) == 0x07, "\"\\a\"")
test("\b".char_at(0) == 0x08, "\"\\b\"")
test("\\".char_at(0) == 0x5c, "\"\\\\\"")
test("\"".char_at(0) == 0x22, "\"\\\"\"")

a=15
test("{a}" == "15", "$to_str")
test("{a!x}" == "f", "hex")
test("{a!o}" == "17", "oct")
test("{a!03d}" == "015", "$to_str with 0")

i = 0
for "0123456789" as s {
    test(s.length == 1)
    test(s.char_at(0) == 0x30 + i)
    i += 1
}
test(i == 10)

i = 0
for "0123456789".chars() as c {
    test(c == '0' + i)
    i += 1
}
test(i == 10)