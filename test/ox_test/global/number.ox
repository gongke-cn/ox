ref "../test"
ref "std/log"

log: Log("number")

test(Number(true) == 1)
test(Number(false) == 0)
test(Number() == 0)
test(Number("0") == 0)
test(Number("1") == 1)
test(Number(" 0xff") == 255)
test(Number(" - 1.2e5") == -1.2e5)
test(Number.NAN != Number.NAN)

test(Number.NAN.isnan())
test((Number.NAN + 1).isnan())
test(Number.INFINITY.isinf())
test((-Number.INFINITY).isinf())

test(1/0 == Number.INFINITY)
test(-1/0 == -Number.INFINITY)