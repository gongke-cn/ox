#!/usr/bin/ox

ref "std/io"
ref "std/log"
ref "std/dir"
ref "std/path"
ref "std/lang"
ref "./test"

log: Log("ox_test")

stdout.puts("OX test start\n")

run_tests: func(path) {
    for Dir(path) as ent {
        if ent == "." || ent == ".." {
            continue
        }

        if path == "test/ox_test" {
            if ent == "test.ox" || ent == "ox_test.ox" {
                continue
            }
        }

        test_path = Path(fullpath("{path}/{ent}"))

        case test_path.format {
            Path.FMT_DIR {
                run_tests(test_path)
            }
            Path.FMT_REG {
                stdout.puts("run {test_path}\n")
                s = OX.script(test_path)
            }
        }
    }
}

run_tests("test/ox_test")

stdout.puts("OX test end\n")
test_report()
