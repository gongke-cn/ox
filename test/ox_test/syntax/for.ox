ref "../test"
ref "std/log"

log: Log("for")

times=0
for a=0; a<100; a+=1 {
    times += 1
}
test(times==100, "loop 100")

a=0
times=0
for ; a<100; a+=1 {
    times += 1
}
test(times==100, "loop 100")

a=0
times=0
for ; a<100; {
    times += 1
    a += 1
}
test(times==100, "loop 100")

a=0
times=0
for ;; {
    if a >= 100 {
        break
    }
    times += 1
    a += 1
}
test(times==100, "loop break")

a=0
times=0
for ;a<100; {
    a += 1
    if a < 50 {
        continue
    }
    times += 1
}
test(a==100 && times==51, "loop continue")