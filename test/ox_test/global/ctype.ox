ref "../test"
ref "std/log"

log: Log("ctype")

a = Int8()
test(a == 0)
a.value = 1
test(a == 1)
a.value = 127
test(a == 127)
a.value = -128
test(a == -128)

p = &a
*p = 100
test(a.value == 100)
*p = -1
test(a.value == -1)

a = UInt8()
test(a == 0)
a.value = 1
test(a == 1)
a.value = 255
test(a == 255)

a = UInt8(256)
test(C.get_length(a) == 256)
for i = 0; i < C.get_length(a); i+=1 {
	test(a[i] == 0)
	a[i] = i
}
for i = 0; i < C.get_length(a); i+=1 {
	test(a[i] == i)
}

b = C.get_ref(a, 0, 128)
test(C.get_length(b) == 128)
C.fill(b, 0)
for i = 0; i < 128; i+=1 {
	test(a[i] == 0)
}
for ; i < 256; i+=1 {
	test(a[i] == i)
}

C.copy(b, 0, a, 128, 128)
for i = 0; i < 128; i+=1 {
	test(a[i] == i + 128)
}

