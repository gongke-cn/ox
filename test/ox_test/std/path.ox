ref "../test"
ref "std/log"
ref "std/io"
ref "std/path"
ref "std/fs"

log: Log("path")

p=Path("Makefile")
test(p.exist())
test(p.format == Path.FMT_REG)

p=Path("tmp.txt")
test(!p.exist())

f=File(p, "w")
f.puts("hello")
f.$close()

p=Path("tmp.txt")
test(p.exist())
test(p.size == 5)
unlink(p)

mkdir("tmp")
p=Path("tmp")
test(p.exist())
test(p.format == Path.FMT_DIR)
rmdir("tmp")
