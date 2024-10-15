ref "../test"
ref "std/log"

log: Log("do_while")

a = 0
times = 0
do {
    times += 1
    a += 1
} while a < 100
test(times==100, "loop 100")

a = 100
times = 0
do {
    times += 1
    a += 1
} while a < 100
test(times==1, "loop 1")

a = 0
do {
    a += 1
    if a >= 100 {
        break
    }
} while true
test(a == 100, "loop 100 break")

a = 0
times = 0
do {
    a += 1
    if a < 50 {
        continue
    }
    times += 1
} while a < 100
test(a == 100 && times == 51, "loop continue")