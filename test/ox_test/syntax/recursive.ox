ref "../test"
ref "std/log"

log: Log("recursive")

factorial: func(n) {
    case n {
    0, 1 {
        return 1
    }
    * {
        return n * factorial(n - 1)
    }
    }
}

test(factorial(0) == 1, "0!")
test(factorial(1) == 1, "1!")
test(factorial(2) == 2, "2!")
test(factorial(3) == 6, "3!")
test(factorial(4) == 24, "4!")
test(factorial(5) == 120, "5!")
test(factorial(6) == 720, "6!")