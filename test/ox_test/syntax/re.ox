ref "../test"
ref "std/log"

log: Log("re")

test("" ~ /\d/ == null)
test("0" ~ /\d/ == "0")
test("9" ~ /\d/ == "9")
test("a" ~ /\d/ == null)
test(" " ~ /\d/ == null)

test("" ~ /\D/ == null)
test("0" ~ /\D/ == null)
test("9" ~ /\D/ == null)
test("a" ~ /\D/ == "a")
test(" " ~ /\D/ == " ")

test("" ~ /\s/ == null)
test(" " ~ /\s/ == " ")
test("\t" ~ /\s/ == "\t")
test("\n" ~ /\s/ == "\n")
test("\r" ~ /\s/ == "\r")
test("\v" ~ /\s/ == "\v")
test("\f" ~ /\s/ == "\f")
test("a" ~ /\s/ == null)
test("0" ~ /\s/ == null)

test("" ~ /\S/ == null)
test(" " ~ /\S/ == null)
test("\t" ~ /\S/ == null)
test("\n" ~ /\S/ == null)
test("\r" ~ /\S/ == null)
test("\v" ~ /\S/ == null)
test("\f" ~ /\S/ == null)
test("a" ~ /\S/ == "a")
test("0" ~ /\S/ == "0")

test("" ~ /\w/ == null)
test("0" ~ /\w/ == "0")
test("9" ~ /\w/ == "9")
test("a" ~ /\w/ == "a")
test("z" ~ /\w/ == "z")
test("A" ~ /\w/ == "A")
test("Z" ~ /\w/ == "Z")
test("_" ~ /\w/ == "_")
test(" " ~ /\w/ == null)
test("-" ~ /\w/ == null)

test("" ~ /\W/ == null)
test("0" ~ /\W/ == null)
test("9" ~ /\W/ == null)
test("a" ~ /\W/ == null)
test("z" ~ /\W/ == null)
test("A" ~ /\W/ == null)
test("Z" ~ /\W/ == null)
test("_" ~ /\W/ == null)
test(" " ~ /\W/ == " ")
test("-" ~ /\W/ == "-")

test("a" ~ /[abc]/ == "a")
test("b" ~ /[abc]/ == "b")
test("c" ~ /[abc]/ == "c")
test("0" ~ /[abc]/ == null)
test(" " ~ /[abc]/ == null)

test("a" ~ /[a-c]/ == "a")
test("b" ~ /[a-c]/ == "b")
test("c" ~ /[a-c]/ == "c")
test("0" ~ /[a-c]/ == null)
test(" " ~ /[a-c]/ == null)

test("a" ~ /[^abc]/ == null)
test("b" ~ /[^abc]/ == null)
test("c" ~ /[^abc]/ == null)
test("0" ~ /[^abc]/ == "0")
test(" " ~ /[^abc]/ == " ")

test("a" ~ /[^a-c]/ == null)
test("b" ~ /[^a-c]/ == null)
test("c" ~ /[^a-c]/ == null)
test("0" ~ /[^a-c]/ == "0")
test(" " ~ /[^a-c]/ == " ")

test("" ~ /a?/ == "")
test("a" ~ /a?/ == "a")

test("" ~ /a+/ == null)
test("a" ~ /a+/ == "a")
test("aa" ~ /a+/ == "aa")

test("" ~ /a*/ == "")
test("a" ~ /a*/ == "a")
test("aa" ~ /a*/ == "aa")

test("" ~ /a{2}/ == null)
test("a" ~ /a{2}/ == null)
test("aa" ~ /a{2}/ == "aa")
test("aaa" ~ /a{2}/ == "aa")

test("" ~ /a{2,}/ == null)
test("a" ~ /a{2,}/ == null)
test("aa" ~ /a{2,}/ == "aa")
test("aaa" ~ /a{2,}/ == "aaa")

test("" ~ /a{,2}/ == "")
test("a" ~ /a{,2}/ == "a")
test("aa" ~ /a{,2}/ == "aa")
test("aaa" ~ /a{,2}/ == "aa")

test("" ~ /a{1,2}/ == null)
test("a" ~ /a{1,2}/ == "a")
test("aa" ~ /a{1,2}/ == "aa")
test("aaa" ~ /a{1,2}/ == "aa")

test("abc" ~ /[a-z]+/ == "abc")
test("abC" ~ /[a-z]+/ == "ab")
test("abC" ~ /[a-z]+/i == "abC")

test("0" ~ /./ == "0")
test("a" ~ /./ == "a")
test(" " ~ /./ == " ")
test("-" ~ /./ == "-")
test("" ~ /./ == null)
test("\n" ~ /./ == null)
test("\n" ~ /./d == "\n")

test("abc" ~ /b/ == "b")
test("abc" ~ /^b/ == null)
test("bc" ~ /^b/ == "b")

test("abc" ~ /b$/ == null)
test("ab" ~ /b$/ == "b")

test("yes" ~ /yes|no/ == "yes")
test("no" ~ /yes|no/ == "no")
test("yen" ~ /yes|no/ == null)

test("abc" ~ /a(?:bc)/ == "abc")

test("abc" ~ /a(?=bc)/ == "a")
test("abb" ~ /a(?=bc)/ == null)
test("abc" ~ /a(?!bc)/ == null)
test("abb" ~ /a(?!bc)/ == "a")

test("abc" ~ /(?<=ab)c/ == "c")
test("bbc" ~ /(?<=ab)c/ == null)
test("abc" ~ /(?<!ab)c/ == null)
test("bbc" ~ /(?<!ab)c/ == "c")

m = "abc=123".match(/([a-z]+)=([0-9]+)/)
test(m.$to_str() == "abc=123")
test(m.groups[0] == "abc=123")
test(m.groups[1] == "abc")
test(m.groups[2] == "123")

test("abcdef" ~ /[a-z]+f/ == "abcdef")

m = "abcdef".match(/([a-z]+)(f?)$/)
test(m.groups[0] == "abcdef")
test(m.groups[1] == "abcdef")
test(m.groups[2] == "")

m = "abcdef".match(/([a-z]+?)(f?)$/)
test(m.groups[0] == "abcdef")
test(m.groups[1] == "abcde")
test(m.groups[2] == "f")

test("abc" ~ /a/ == "a")
test("abc" ~ /a/p == null)
test("abc" ~ /b/ == "b")
test("abc" ~ /b/p == null)

test("abc\ndef" ~ /^def/ == null)
test("abc\ndef" ~ /^def/m == "def")

test("abc\ndef" ~ /abc$/ == null)
test("abc\ndef" ~ /abc$/m == "abc")

m = "abc def".match(/[a-z]+/)
test(m.$to_str() == "abc")
m = "abc def".match(/[a-z]+/, 3)
test(m.$to_str() == "def")
m = "abc def".match(/[a-z]+/, -4)
test(m.$to_str() == "def")
m = "abc def".match(/[a-z]+/, -5)
test(m.$to_str() == "c")

m = "abc def".match(/[a-z]+/, -8)
test(m.$to_str() == "abc")

i = 0
for "a1b2c3d4e5f6".match_iter(/[a-z]/) as s {
    test(s.length == 1)
    test(s.char_at(0) == 'a' + i)
    i += 1
}
test(i == 6)

i = 0;
for "a b  c\nd\te\ff".split(/\s+/) as s {
    test(s.length == 1)
    test(s.char_at(0) == 'a' + i)
    i += 1
}
test(i == 6)

s = "a1b2c3d4".replace(/[a-z]/, "$u&")
test(s == "A1B2C3D4")
s = "a1b2c3d4".replace(/[a-z]/, "$u&", 1)
test(s == "a1B2C3D4")
s = "a1b2c3d4".replace(/[a-z]/, "$u&", -2)
test(s == "a1b2c3D4")
s = "a1b2c3d4".replace(/[a-z]/, "$u&", 0, true)
test(s == "A1b2c3d4")

m = "20240101".match(/(\d+:)?(\d+)/)
test(m.$to_str() == "20240101")
test(m.groups[1] == null)
test(m.groups[2] == "20240101")

m = "12:20240101".match(/(\d+:)?(\d+)/)
test(m.$to_str() == "12:20240101")
test(m.groups[1] == "12:")
test(m.groups[2] == "20240101")
