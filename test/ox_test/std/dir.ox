ref "../test"
ref "std/log"
ref "std/io"
ref "std/dir"
ref "std/path"
ref "std/fs"

log: Log("dir")

mkdir("tmp")

for i=0; i<1024; i+=1 {
    f=File("tmp/{i}.txt", "w")
    f.puts("{i}")
    f.$close()
}

i = 0
for Dir("tmp") as n {
    if n == "." || n == ".." {
        continue
    }

    path = "tmp/{n}"
    test(Path(path).exist)

    f = File(path, "r")
    s = f.gets()
    test(s == n.slice(0, n.length - 4))
    f.$close()

    i += 1

    unlink(path)
}

test(i == 1024)

rmdir("tmp")