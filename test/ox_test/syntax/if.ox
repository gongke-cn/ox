ref "../test"
ref "std/log"

log: Log("if")

a = 1
case1 = false
case2 = false
case3 = false
if a == 1 {
    case1 = true
} elif a == 2 {
    case2 = true
} else {
    case3 = true
}
test(case1 && !case2 && !case3, "run if block")

a = 2
case1 = false
case2 = false
case3 = false
if a == 1 {
    case1 = true
} elif a == 2 {
    case2 = true
} else {
    case3 = true
}
test(!case1 && case2 && !case3, "run elif block")

a = 0
case1 = false
case2 = false
case3 = false
if a == 1 {
    case1 = true
} elif a == 2 {
    case2 = true
} else {
    case3 = true
}
test(!case1 && !case2 && case3, "run else block")

a = 1
b = 0
if a == 1 {
} elif b = 1, a == 2 {
} else {
}
test(b == 0, "elif condition do not run")