ref "../test"
ref "std/log"

log: Log("return")

v = func(){
    return
}()
test(v == null, "return without value")

v = func(){
    return 1
}()
test(v == 1, "return with value")