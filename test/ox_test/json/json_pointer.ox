ref "../test"
ref "std/log"
ref "json"

log: Log("json_pointer")

json_ptr_test: func(src, ptrs) {
    jp = JsonPointer(src)
    match = true

    if jp.pointers.length != ptrs.length {
        match = false
    } else {
        for i = 0; i < ptrs.length; i += 1 {
            if jp.pointers[i] != ptrs[i] {
                match = false
            }
        }
    }

    test(match, "JSON pointer \"{src}\" do not match {JSON.to_str(ptrs)}")

    jp = JsonPointer.build(...ptrs)
    test(jp.$to_str() == src)
}

json_ptr_test("/", [""])
json_ptr_test("/a", ["a"])
json_ptr_test("/a/b", ["a", "b"])
json_ptr_test("/a/b/c", ["a", "b", "c"])
json_ptr_test("/a/b/", ["a", "b", ""])
json_ptr_test("/abc/0/12/def", ["abc", "0", "12", "def"])
json_ptr_test("/~0/~1/a~0~1", ["~", "/", "a~/"])
