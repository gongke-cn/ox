ref "../test"
ref "std/log"

log: Log("for_as")

times = 0
sum = 0
for [0,1,2,3,4,5] as i {
    times += 1
    sum += i
}
test(times==6 && sum == 15, "for [0~5] as")

iter_num = 0
end_num = 0
value_num = 0
next_num = 0
close_num = 0
c: class {
    static Iterator: class Iterator {
        $init(a) {
            this.a = a
            this.p = 0
        }
        end {
            @end_num += 1
            if this.p >= this.a.length {
                return true
            } else {
                return false
            }
        }
        value {
            @value_num += 1
            if this.p >= this.a.length {
                return null
            } else {
                return this.a[this.p]
            }
        }
        next() {
            @next_num += 1
            if this.p < this.a.length {
                this.p += 1
            }
        }
        $close() {
            @close_num += 1
        }
    }
    $iter() {
        @iter_num += 1
        return c.Iterator([0,1,2,3,4,5,6,7,8,9])
    }
}
n = 0
for c() as i {
    test(n == i, "iterator item value check")
    n += 1
}
test(iter_num==1 && end_num == 11 && value_num==10 && next_num == 10 && close_num == 1)

for [0,1,2,3,4] as item {
    if item == 2 {
        break
    }
}

test(item == 2)