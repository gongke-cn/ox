ref "../test"
ref "std/log"

log: Log("try")

t_flag = false
c_flag = false
f_flag = false
try {
    t_flag = true
} catch e {
    c_flag = true
} finally {
    test(t_flag)
    f_flag = true
}
test(t_flag && !c_flag && f_flag, "no error")

t_flag = false
c_flag = false
f_flag = false
try {
    t_flag = true
    throw Error()
} catch e {
    test(t_flag)
    c_flag = true
} finally {
    test(t_flag && c_flag)
    f_flag = true
}
test(t_flag && c_flag && f_flag, "throw error")

t_flag = false
c_flag = false
f_flag = false
try {
    t_flag = true
    try {
        throw Error()
    }
} catch e {
    test(t_flag)
    c_flag = true
} finally {
    test(t_flag && c_flag)
    f_flag = true
}
test(t_flag && c_flag && f_flag, "inner throw error")

t_flag = false
c_flag = false
f_flag = false
outer_c_flag = false
try {
    try {
        t_flag = true
        throw Error()
    } catch e {
        test(t_flag)
        throw e
        c_flag = true
    } finally {
        test(t_flag)
        f_flag = true
    }
} catch e {
    outer_c_flag = true
}
test(t_flag && !c_flag && f_flag && outer_c_flag, "rethrow error in catch")

t_flag = false
c_flag = false
f_flag = false
outer_c_flag = false
try {
    try {
        t_flag = true
        throw Error()
    } catch e {
        test(t_flag)
        c_flag = true
    } finally {
        test(t_flag && c_flag)
        throw Error()
        f_flag = true
    }
} catch e {
    outer_c_flag = true
}
test(t_flag && c_flag && !f_flag && outer_c_flag, "throw error in finally")

f_flag = false
for [0,1,2,3,4] as item {
    try {
        break
    } finally {
        f_flag = true
    }
}
test(f_flag == true && item == 0)

c_flag = false
f_flag = false
for [0,1,2,3,4] as item {
    try {
        throw Error()
    } catch e {
        c_flag = true
        break
    } finally {
        f_flag = true
    }
}
test(c_flag == true && f_flag == true && item == 0)

c_flag = false
f_flag = false
for [0,1,2,3,4] as item {
    try {
        throw Error()
    } catch e {
        c_flag = true
    } finally {
        f_flag = true
        break
    }
}
test(c_flag == true && f_flag == true && item == 0)

f_inner = false
f_outer = false
for [0,1,2,3,4] as item {
    try {
        try {
            break
        } finally {
            f_inner = true
        }
    } finally {
        f_outer = true
    }
}
test(f_inner && f_outer && item == 0)