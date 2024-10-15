ref "../test"
ref "std/log"

log: Log("tail_recursion")

tail1: func(n) {
    if n > 0 {
        return tail1(n -1)
    }

    return n
}

tail1(1000000)

tail2: func(n) {
    if n == 0 {
        return
    }

    tail2(n - 1)
    return
}

tail2(1000000)

tail3: func(n) {
    if n == 0 {
        return
    }

    tail3(n - 1)
}

tail3(1000000)

tail4: func(n) {
    try {
    } finally {
        if n > 1 {
            tail4(n - 1)
        }
    }
}

tail4(1000000)