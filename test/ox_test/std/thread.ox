ref "std/thread"
ref "std/system"
ref "../test"

fn: func(arg) {
    msleep(100)
    return arg * 10
}

threads = []
for i = 0; i < 10; i += 1 {
    th = Thread(fn, null, i)
    threads.push(th)
}

i = 0
for threads as th {
    r = th.join()
    test(i * 10 == r)
    i += 1
}
test(i == 10)

result = []

detach_fn: func(arg) {
    msleep(100)
    result[arg] = true
}

for i = 0; i < 10; i += 1 {
    th = Thread(detach_fn, null, i)
    th.detach()
}

msleep(500)
test(result.length == 10)
for result as r {
    r == true
}