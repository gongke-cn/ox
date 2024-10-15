ref "std/temp_file"
ref "std/path"
ref "std/fs"
ref "std/io"
ref "std/log"
ref "../test"

log: Log("temp_file")


filename = "test.txt"

file_test: func() {
    #tf = TempFile(filename)

    file = tf.open("w")
    file.puts("test pattern")

    test(Path(filename).exist())
    file.$close()
}

test(!Path(filename).exist())
file_test()
test(!Path(filename).exist())

dn = "test_dir"

dir_test: func() {
    #td = TempDir(dn)

    td.mkdir()

    mkdir("{dn}/sub_dir")

    file = File("{dn}/sub_dir/test.txt", "w")
    file.puts("test pattern")
    file.$close()

    file = File("{dn}/test.txt", "w")
    file.puts("test pattern")
    file.$close()
}

test(!Path(dn).exist())
dir_test()
test(!Path(dn).exist())
