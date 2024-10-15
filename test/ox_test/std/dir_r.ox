ref "../test"
ref "std/log"
ref "std/io"
ref "std/dir_r"
ref "std/fs"

mkdir("tmp")

for i = 0; i < 10; i += 1 {
    dir = "tmp/tmp{i}"
    path = "{dir}/test.txt"
    mkdir(dir)
    f = File(path, "w")
    f.puts("{i}")
    f.$close()
}

i = 0
for DirR("tmp").select(($ ~ /.txt$/)) as n {
    m = n.match(/tmp\/tmp(.*)\/test\.txt/).groups[1]
    f = File(n, "r")
    s = f.gets()
    test(s == m)
    f.$close()
    i += 1
}
test(i == 10)

for i = 0; i < 10; i += 1 {
    dir = "tmp/tmp{i}"
    path = "{dir}/test.txt"
    unlink(path)
    rmdir(dir)
}

rmdir("tmp")