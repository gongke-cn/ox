ref "../test"
ref "std/log"

log: Log("array")

test_array([,,1,,], [null,null,1,null])

test_array([1,...[2,3,4,5]], [1,2,3,4,5])
test_array([1,...[2,3,4,5],...[6,7,8,9,10]], [1,2,3,4,5,6,7,8,9,10])
test_array([,2,,4], [null, 2, null, 4])

a=[1,2,3,4,5]
test_array(a, [1,2,3,4,5])

a.[6,7,8,9,10]
test_array(a, [1,2,3,4,5,6,7,8,9,10])

a.[...[11,12,13,14,15]]
test_array(a, [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15])

a=[]
a.push(1)
test_array(a, [1])
a.push(2,3)
test_array(a, [1,2,3])
a.push(4,5,6)
test_array(a, [1,2,3,4,5,6])

v=a.pop()
test(v == 6)
test_array(a, [1,2,3,4,5])
v=a.pop()
test(v == 5)
test_array(a, [1,2,3,4])
v=a.pop()
test(v == 4)
test_array(a, [1,2,3])
v=a.pop()
test(v == 3)
test_array(a, [1,2])
v=a.pop()
test(v == 2)
test_array(a, [1])
v=a.pop()
test(v == 1)
test_array(a, [])
v=a.pop()
test(v == null)
test_array(a, [])

i = 0
for [0,1,2,3,4,5] as v {
    test(v == i)
    i += 1
}
test(i == 6)

a = [0,1,2,3]
a.insert(0,6,7,8,9)
test_array(a, [6,7,8,9,0,1,2,3])
a = [0,1,2,3]
a.insert(-1,6,7,8,9)
test_array(a, [0,1,2,6,7,8,9,3])
a = [0,1,2,3]
a.remove(0,1)
test_array(a, [1,2,3])

a = [0,1,2,3]
a.remove(-1,1)
test_array(a, [0,1,2])

a = [0,1,2,3]
test_array(a.slice(0,1), [0])
test_array(a.slice(1,2), [1])
test_array(a.slice(1), [1,2,3])
test_array(a.slice(-1), [3])
test_array(a.slice(-3,-1), [1,2])

a = [0,1,2,3]
test(a.has(0))
test(a.has(1))
test(a.has(2))
test(a.has(3))
test(!a.has(4))
test(!a.has(-1))
test(a.find(0) == 0)
test(a.find(1) == 1)
test(a.find(2) == 2)
test(a.find(3) == 3)
test(a.find(4) == -1)
test(a.find(-1) == -1)

flag = 0
a = [
    1
    2
    3
    if flag {
        4
        5
    }
]
test(a.length == 3)

flag = 1
a = [
    1
    2
    3
    if flag {
        4
        5
    }
]
test(a.length == 5)
test(a[3] == 4)
test(a[4] == 5)

flag = 0
a = [
    1
    2
    3
    case flag {
    1 {
        1
        1
    }
    2 {
        2
        2
    }
    3 {
        3
        3
    }
    }
]
test(a.length == 3)

flag = 1
a = [
    1
    2
    3
    case flag {
    1 {
        1
        1
    }
    2 {
        2
        2
    }
    3 {
        3
        3
    }
    }
]
test(a.length == 5)
test(a[3]==1)
test(a[4]==1)

flag = 2
a = [
    1
    2
    3
    case flag {
    1 {
        1
        1
    }
    2 {
        2
        2
    }
    3 {
        3
        3
    }
    }
]
test(a.length == 5)
test(a[3]==2)
test(a[4]==2)

flag = 3
a = [
    1
    2
    3
    case flag {
    1 {
        1
        1
    }
    2 {
        2
        2
    }
    3 {
        3
        3
    }
    }
]
test(a.length == 5)
test(a[3]==3)
test(a[4]==3)