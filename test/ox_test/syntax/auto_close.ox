ref "../test"
ref "std/log"

log: Log("auto_close")

close_num = 0

Auto: class {
    $close() {
        @close_num += 1
    }
}

#var = Auto()

test(close_num == 0)

var = var
test(close_num == 0)

var=null

test(close_num == 1)

close_num = 0

func() {
    test(close_num == 0)
    #v = Auto()
    test(close_num == 0)
}()

test(close_num == 1)