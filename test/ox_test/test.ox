ref "std/io"
ref "std/lang"
ref "std/log"

log: Log("test")

total_test_num = 0
failed_test_num = 0

public test: func (v, msg, level = 1) {
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

public test_array: func (a1, a2, msg) {
    ok = true

    if a1.length != a2.length {
        ok = false
    } else {
        for i = 0; i < a1.length; i += 1 {
            if a1[i] != a2[i] {
                ok = false
                break;
            }
        }
    }

    if !ok {
        test(ok, "{msg} [{a1}] != [{a2}]", 2)
    }
}

public test_report: func() {
    stdout.puts("total: {total_test_num} failed: {failed_test_num}\n")
}
