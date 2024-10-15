#!/usr/bin/ox

ref "test"
ref "std/log"
ref "std/io"
ref "std/lang"
ref "json"

log: Log("test")
log.level = Log.WARNING

total_test_num = 0
failed_test_num = 0

test: func (v, msg, level = 1) {
    @total_test_num += 1
    if !v {
        @failed_test_num += 1
        s = OX.stack(level + 1)[level]
        if !msg {
            msg = "failed"
        }
        name = Object.get_name(s.function)
        if !name {
            name = "noname"
        }
        stderr.puts("test failed: \"{s.filename}\": line {s.line}: {name}: {msg}\n")
    }
}

s = Struct()

s.s_field.a = 1
test(s.s_field.a == 1)

s.c_field = 1
test(s.c_field == 1)
s.c_field = 255
test(s.c_field == -1)
s.c_field += 1
test(s.c_field == 0)

s.i_field = 123
test(s.i_field == 123)

s.b_field_1 = 1
test(s.b_field_1 == 1)
s.b_field_1 = 2
test(s.b_field_1 == 0)

s.b_field_2 = 1
test(s.b_field_2 == 1)
s.b_field_2 = 2
test(s.b_field_2 == 2)
s.b_field_2 = 3
test(s.b_field_2 == 3)
s.b_field_2 = 4
test(s.b_field_2 == 0)

test(C.get_length(s.cb_field) == 16)
for i=0; i<16; i+=1 {
    s.cb_field[i] = i
}
for i=0; i<16; i+=1 {
    test(s.cb_field[i] == i)
}

cb=Int8(16)
for i=0; i<16; i+=1 {
    cb[i] = 'a' + i
}
C.copy(cb, 0, s.cb_field, 0)
for i=0; i<16; i+=1 {
    test(s.cb_field[i] == 'a' + i)
}

sa = Struct(4)
test(C.get_length(sa) == 4)
for i=0; i<4; i+=1 {
    sa[i] = s
}

for i=0; i<4; i+=1 {
    for j=0; j<16; j+=1 {
        test(sa[i].cb_field[j] == 'a' + j)
    }
}

n=Int32()

for i=0; i<100; i+=1 {
    test(increase(&n) == i)
    test(*n == i + 1)
}

test(variable.get() == 1982)
variable.set(1978)
test(variable.get() == 1978)

test(function_v(0) == 0)
test(function_v(1, 1) == 1)
test(function_v(2, 1, 2) == 3)
test(function_v(3, 1, 2, 3) == 6)

cb_arg = 0
my_cb: func(v) {
    @cb_arg = v
}

call_cb()
test(cb_arg == 0)
register_cb(my_cb)
call_cb()
test(cb_arg == 123456789)

stdout.puts("total: {total_test_num} failed: {failed_test_num}\n")
