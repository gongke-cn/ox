ref "../test"
ref "std/log"

log: Log("bool")

test(typeof true == Bool)
test(typeof false == Bool)
test(String(true) == "true")
test(String(false) == "false")
test(Number(true) == 1)
test(Number(false) == 0)
test(Bool() == false)
test(Bool("") == false)
test(Bool(1) == true)