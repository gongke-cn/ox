ref "../test"
ref "std/log"

log: Log("case")

a = 1
case1 = false
case2 = false
case3 = false
case4 = false
case a {
1 {
    case1 = true
}
2 {
    case2 = true
}
3 {
    case3 = true
}
* {
    case4 = true
}
}
test(case1 && !case2 && !case3 && !case4, "run case 1")

a = 2
case1 = false
case2 = false
case3 = false
case4 = false
case a {
1 {
    case1 = true
}
2 {
    case2 = true
}
3 {
    case3 = true
}
* {
    case4 = true
}
}
test(!case1 && case2 && !case3 && !case4, "run case 2")

a = 3
case1 = false
case2 = false
case3 = false
case4 = false
case a {
1 {
    case1 = true
}
2 {
    case2 = true
}
3 {
    case3 = true
}
* {
    case4 = true
}
}
test(!case1 && !case2 && case3 && !case4, "run case 3")

a = 0
case1 = false
case2 = false
case3 = false
case4 = false
case a {
1 {
    case1 = true
}
2 {
    case2 = true
}
3 {
    case3 = true
}
* {
    case4 = true
}
}
test(!case1 && !case2 && !case3 && case4, "run case *")

a = 1
case1 = false
case2 = false
case a {
1, 2 {
    case1 = true
}
3, * {
    case2 = true
}
}
test(case1 && !case2, "a=1, run case 1")

a = 2
case1 = false
case2 = false
case a {
1, 2 {
    case1 = true
}
3, * {
    case2 = true
}
}
test(case1 && !case2, "a=2, run case 1")

a = 3
case1 = false
case2 = false
case a {
1, 2 {
    case1 = true
}
3, * {
    case2 = true
}
}
test(!case1 && case2, "a=3, run case 2")

a = 0
case1 = false
case2 = false
case a {
1, 2 {
    case1 = true
}
3, * {
    case2 = true
}
}
test(!case1 && case2, "a=0, run case 2")

a = 0
case1 = false
case2 = false
case3 = false
case a {
($ > 0) {
    case1 = true
}
($ < 0) {
    case2 = true
}
0 {
    case3 = true
}
}
test(!case1 && !case2 && case3, "a=0")

a = 1
case1 = false
case2 = false
case3 = false
case a {
($ > 0) {
    case1 = true
}
($ < 0) {
    case2 = true
}
0 {
    case3 = true
}
}
test(case1 && !case2 && !case3, "a=1")

a = -1
case1 = false
case2 = false
case3 = false
case a {
($ > 0) {
    case1 = true
}
($ < 0) {
    case2 = true
}
0 {
    case3 = true
}
}
test(!case1 && case2 && !case3, "a=-1")