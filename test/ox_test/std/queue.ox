ref "std/thread"
ref "std/system"
ref "std/queue"
ref "std/log"
ref "../test"

q = Queue()
log = Log("queue")

th = Thread(func() {
    for i = 0; i < 100; i += 1 {
        q.send(i)
        msleep(1)
    }
})

for i = 0; i < 100; i += 1 {
    v = q.wait()
    test(v == i)
}

th.join()
