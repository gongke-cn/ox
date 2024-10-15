# 函数
## 函数声明
通过关键字"func"可以声明一个函数。函数接收一组参数，并返回一个返回值。‵
```
my_func: func (a, b, op) {
    if op {
        return a + b
    } else {
        return a - b
    }
}
```
上面的程序定义了一个函数，并将函数赋值给常量"my_func"。
函数接收3个参数，"a", "b"和"op"。
函数中根据布尔型参数"op"的值，分别进行参数"a"和"b"的加运算或减运算。
最后通过"return"语句返回结果。

下面我们通过常量"my_func"调用设个函数。
```
stdout.puts("{my_func(1, 2, true)}\n") //打印3
stdout.puts("{my_func(1, 2, false)}\n") //打印-1
```
## 参数
函数参数为通过一对小括号包围的一组形式参数列表。如果函数不带形式参数，可以直接省略小括号：
```
my_func: func {
    return "function without parameters"
}
```
参数可以带一个缺省值表达式，当调用函数此参数没有设置或参数值为null时，使用缺省值作为参数值。
```
my_func: func(a=100) {
    return -a
}

stdout.puts("{my_func()}\n") //打印-100
stdout.puts("{my_func(null)\n}") //打印-100
```
如果函数传入参数数目是可变的，可以在函数内部通过关键字"argv"访问传入的参数数组：
```
my_func: func() {
    for argv as arg {
        stdout.puts("{arg} ")
    }
    stdout.puts("\n")
}

my_func(1, 2, 3) //打印 1 2 3
my_func(1, 2, 3, 4, 5) //打印 1 2 3 4 5
```
如果函数的前几个参数是固定的，后面跟一组可变长度的参数，可以用以下方式定义：
```
//参数a, b为固定参数，后面可变参数存储在数组中赋值给参数args
my_func: func(a, b, ...args) {
    stdout.puts("a:{a} b:{b} args:")
    for args as arg {
        stdout.puts("{arg} ")
    }
    stdout.puts("\n")
}

my_func(1,2) //参数a=1, b=2, args=[]
my_func(1,2,3,4,5) //参数a=1, b=2, args=[3,4,5]
```
如果想将一个数组的元素作为可变参数传递给函数，可以用"..."将数组元素展开为参数：
```
a = [1,2,3]
my_func(...a) //等价于my_func(1,2,3)
```
有时我们需要将一个数组作为参数传递给函数：
```
my_func = func(arg) {
    return arg[0] + arg[1]
}

my_func([123, 321])
```
这时我们可以将形式参数直接写为数组形式，这样我们可以在函数体中直接用参数变量名替换对数组元素的访问：
```
my_func = func([a, b]) {
    return a + b
}

my_func([123, 321])
```
有时我们需要将一个对象作为参数传递给函数：
```
my_func = func(arg) {
    return arg.p1 + arg.p2
}

my_func({p1:100, p2:200})
```
这时我们可以将形式参数直接写为对象形式，这样我们可以在函数体中直接用参数变量名替换对对象属性的访问：
```
my_func = func({p1:a, p2:b}) {
    return a + b
}

my_func({p1:100, p2:200})
```
如果对象的属性名和参数变量名一样，我们还可以将上面的函数进一步简化为：
```
my_func = func({p1, p2}) {
    return p1 + p2
}

my_func({p1:100, p2:200})
```
## this参数
如果函数是一个对象的属性值，通过访问对象属性值的方式调用函数时，在函数中可以通过this参数访问这个对象。
```
o = {
    p: ""

    f: func() {
        stdout.puts("{this.p}\n")
    }
}

o.f() //o作为this参数传递给函数，函数中打印o.p
```
## 回调函数
函数定义可以作为参数传递给另一个函数。代表用这个函数参数作为回调函数。如:
```
test_cb: func(cb) {
    for i = 0; i < 100; i += 1 {
        cb(i) //调用回调函数
    }
}

//调用test_cb, 在回调函数中打印
test_cb(func(i) {
    stdout.puts("{i}\n")
})
```
如果回调函数执行操作很简单，可以使用表达式表示一个回调函数。如：
```
//函数遍历数组元素，通过回调函数修改元素内容
map: func(a, cb) {
    for i = 0; i < a.length; i += 1 {
        a[i] = cb(a[i])
    }
}

a = [0, 1, 2, 3, 4, 5, 6, 7]

//遍历数组，元素取负值
map(a, (-$))
```
回调函数用一对小括号封装，小括号内表达式表示回调函数执行的内容。上面的调用等价于：
```
map(a, func(v) {
    return -v
})
```
表达式回调函数定义中，"$"代表函数的首个参数，如果访问其他参数，可以用"$N"表示，其中"N"为参数的索引号。如：
```
//冒泡排序
sort: func(a, cmp) {
    for i = 0; i < a.length; i += 1 {
        for j = i + 1; j < a.length; j += 1 {
            if cmp(a[i], a[j]) > 0 {
                t = a[i]
                a[i] = a[j]
                a[j] = t
            }
        }
    }
}

a = [9,1,7,6,3,5,0]

//从大到小排序
sort(a, ($1 - $0))
```
## 函数体
函数体为通过一对大括号包围的一组内部语句。如果函数需要执行的操作比较简单可以用一个表达式描述，则函数可表示为以下形式：
```
my_func: func(a, b) => a + b
```
"=>"右侧为函数体对应的表达式，以上定义等价于：
```
my_func: func(a, b) {
    return a + b
}
```
## 内部变量
在函数内部声明的变量或常量，这些变量或常量的作用域在这个函数内部:
```
my_func: func() {
   v1 = 1 //声明内部变量v1并赋值为1
   v2 //声明内部变量v2,值为null
   c: true //内部常量c值为true
}
```
当一个函数在另一个函数内部定义时，内层函数可以访问外层函数定义的变量:
```
outer: func() {
    outer_v = 1 //outer函数内部变量
    
    inner: func() {
        stdout.puts("{outer_v}\n") //访问外层函数outer定义的变量outer_v
    }

    inner()
}
```
如果内层函数想修改外层函数定义变量的值，注意要在这个变量标识符的前面加"@"，这样OX解释器知道这是一个外层函数定义的变量，否则会认为这是内层函数自己定义的变量：
```
outer: func() {
    outer_v = 1 //outer函数内部变量
    
    inner: func() {
        @outer_v = 2 //将外层函数中定义的变量值改为2
    }

    inner()

    inner2: func() {
        outer_v = 2 //inner2自定义内部变量outer_v,不会影响outer函数变量outer_v
    }

    inner2()
}
```
如何内层函数多次修改外层函数定义变量的值，只要有一处变量标识符的前面加"@"就可以了:
```
outer: func() {
    outer_v = 1 //outer函数内部变量
    
    inner: func() {
        @outer_v = 2 //将外层函数中定义的变量值改为2
        outer_v = 3 //将外层函数中定义的变量值改为3
    }

    inner()
}
```
## 函数调用
当一个函数被调用时，OX解释器会分配一个新的数据帧用来存放函数的参数和内部变量。
```
generator: func(a) {
    v = a

    return func => v
}

f1 = generator(1) //返回函数，其内部变量v为1
f2 = generator(2) //返回函数，其内部变量v为2

stdout.puts("{f1()}\n") //打印1
stdout.puts("{f2()}\n") //打印2
```
## call属性
每个函数包含一个名为"call"的函数属性，可以通过调用这个属性实现对函数的调用。
"call"的第一个参数会作为"this"参数传递给函数，后面的参数作为参数传递给函数。
```
o: {
    test: func(a, b) {
        stdout.puts("{this.p} {a} {b}\n")
    }
}

o.test.call({p:1}, 2, 3) //this = {p:1}, argv = [2, 3] 打印 1 2 3
```

## 语法描述
```
function_declaration: "func" formal_parameters? function_body

formal_parameters: "(" formal_parameter_list? ")"

formal_parameter_list: formal_parameter_list "," formal_parameter
    | formal_parameter

formal_parameter: formal_parameter_pattern ("=" expression)?
    | "..." formal_parameter_pattern

formal_parameter_pattern: identifier
    | "[" array_parameter_pattern_items? "]"
    | "{" object_parameter_pattern_properties? "}"

array_parameter_pattern_items: array_parameter_pattern_items "," array_parameter_pattern_item
    | array_parameter_pattern_item

array_parameter_pattern_item:
    | formal_parameter_pattern ("=" expression)?

object_parameter_pattern_properties: object_parameter_pattern_properties "," object_parameter_pattern_property
    | object_parameter_pattern_property

object_parameter_pattern_property: property_name ":" formal_parameter_pattern ("=" expression)?
    | identifier

function_body: "=>" expression
    | "{" statements? "}"
```