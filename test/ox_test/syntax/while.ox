ref "../test"
ref "std/log"

log: Log("while")

a = 0
times = 0
while a < 100 {
    times += 1
    a += 1
}
test(times==100, "loop 100")

a = 100
times = 0
while a < 100 {
    times += 1
    a += 1
}
test(times==0, "loop 0")

a = 0
while true {
    a += 1
    if a >= 100 {
        break
    }
}
test(a==100, "break 100")

a = 0
times=0
while a < 100 {
    a += 1
    if a > 50 {
        continue
    }
    times += 1
}
test(times==50, "continue 50")