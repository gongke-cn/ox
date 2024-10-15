ref "../test"
ref "std/log"

log: Log("string")

test(String() == "")
test(String(1) == "1")
test(String(true) == "true")
test(String(false) == "false")

test("".length == 0)
test("abcdefg".length == 7)

test("  a  ".trim() == "a")
test("  a  ".trim_h() == "a  ")
test("  a  ".trim_t() == "  a")

test("a".pad_h(5) == "    a")
test("a".pad_h(5, "*-") == "*-*-a")

test("a".pad_t(5) == "a    ")
test("a".pad_t(5, "*-") == "a*-*-")

test("abcdefg".slice(0,0) == "")
test("abcdefg".slice(0,1) == "a")
test("abcdefg".slice(0,2) == "ab")
test("abcdefg".slice(1,2) == "b")
test("abcdefg".slice(1) == "bcdefg")
test("abcdefg".slice(-2) == "fg")
test("abcdefg".slice(-4,-2) == "de")
test("abcdefg".slice(-8,-2) == "abcde")

test("a123a123".lookup_char('a') == 0)
test("a123a123".lookup_char('3') == 3)
test("a123a123".lookup_char('a', 1) == 4)
test("a123a123".lookup_char('3', 4) == 7)
test("a123a123".lookup_char('a', -7) == 4)

test("a123a123".lookup_char_r('a') == 4)
test("a123a123".lookup_char_r('a', 3) == 0)
test("a123a123".lookup_char_r('a', -5) == 0)

test("abcdefg1234567".to_upper() == "ABCDEFG1234567")
test("ABCDEFG1234567".to_lower() == "abcdefg1234567")

i = 0
for "abcdefg" as s {
    test(s.length == 1)
    test(s.char_at(0) == 'a' + i)
    i += 1
}
test(i == 7)

i = 0
for "abcdefg".chars() as c {
    test(c == 'a' + i)
    i += 1
}
test(i == 7)

i = 0;
for "天地玄黄宇宙洪荒".uchars() as uc {
    case i {
    0 {
        test(uc == '天')
    }
    1 {
        test(uc == '地')
    }
    2 {
        test(uc == '玄')
    }
    3 {
        test(uc == '黄')
    }
    4 {
        test(uc == '宇')
    }
    5 {
        test(uc == '宙')
    }
    6 {
        test(uc == '洪')
    }
    7 {
        test(uc == '荒')
    }
    }
    i += 1
}
test(i == 8)
